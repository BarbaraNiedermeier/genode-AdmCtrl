/*
 * \brief  
 * \author Barbara Niedermeier
 * \date   2017/07/25
 */

#include <base/printf.h>

/* for optimize function */
#include <util/xml_node.h>
#include <util/xml_generator.h>
#include <typeinfo>
/* ******************************** */

#include "sched_controller/sched_opt.h"

#include <cmath>
#include <string>
#include <cstring>


namespace Sched_controller {

	void Sched_opt::set_goal(Genode::Ram_dataspace_capability xml_ds_cap)
	{
		// Definition of the optimization goal via xml file
		Genode::Rm_session* rm = Genode::env()->rm_session();
		const char* xml = rm->attach(xml_ds_cap);
		PDBG("Optimizer - Start parsing XML file.");
		Genode::Xml_node root(xml);

		const auto fn = [this] (const Genode::Xml_node& node)
		{
			int max_len = 32;
			std::vector<char> fair_goal(max_len);
			std::vector<char> util_goal(max_len);
			
			// store xml node value
			node.sub_node("fairness").sub_node("apply").value(fair_goal.data(), fair_goal.size());
			node.sub_node("utilization").sub_node("apply").value(util_goal.data(), util_goal.size());
			
			
			
			// analyze xml node value and set _opt_goal
			if (std::stoi(fair_goal.data())) 
			{
				// optimization goal is set to fairness
				_opt_goal = FAIRNESS;
				PDBG("Optimizer - Set optimization goal to fairness.");
				
				// set acceptance niveau. This is a magic number and can be adjusted to finalize the optimization.
				std::vector<char> fair_acceptance(max_len);
				node.sub_node("fairness").sub_node("acceptance").value(fair_acceptance.data(), fair_acceptance.size());
				accept = std::stoi(fair_acceptance.data());
				PDBG("the acceptance interval is: %d", accept);
			}
			else if (std::stoi(util_goal.data()))
			{
				// optimization goal is set to utilization
				_opt_goal = UTILIZATION;
				PDBG("Optimizer - Set optimization goal to utilization.");
			}
			else
			{
				// no optimization goal is set
				_opt_goal = NONE;
				PDBG("Optimizer - Set optimization goal to none.");
			}
			
			
		};
		root.for_each_sub_node("goal", fn);
		
		// set query interval
		std::vector<char> interval(32);
		root.sub_node("query_interval").value(interval.data(), interval.size());
		query_intervall = std::stoi(interval.data());
		PDBG("Optimizer: The query interval is set to %d.", query_intervall);
		
		rm->detach(xml);
		
		PDBG("Optimizer - Finish parsing XML file.");
	}
	
	
	int Sched_opt::add_task(int core, Rq_task::Rq_task task)
	{
		PDBG("Optimizer - Add task %s to task list.", std::string(task.name).c_str());
		
		// convert newly arriving task to optimization task
		Optimization_task _task;
		
		_task.name = std::string(task.name); // das geht erst, wenn Masterarebit von Steffan fertig ist
		_task.inter_arrival = task.inter_arrival;
		_task.deadline = task.deadline;
		_task.core = core;
		_task.arrival_time = 0;
		_task.to_schedule = true;
		_task.last_job_started = false;
		
		_task.newest_job.foc_id = 0;
		_task.newest_job.arrival_time = 0;
		_task.newest_job.dispatched = true;
		
		// used to do utilization optimisation
		_task.utilization = 0;
		_task.execution_time = 0;
		
		// for all cores the value is initially 0
		for (int i=0; i < num_cores; ++i)
		{
			_task.value[i] = 0;
			_task.overload[core] = false;
		}
		
		
		_tasks.insert({_task.name, _task});
		
		// this return value is needed for call from enq
		return -1;
	}
	
	void Sched_opt::last_job_started(std::string task_name)
	{
		// This function is called by the taskloader as soon as the last job was started for this task.
		
		if(_tasks.count(task_name))
		{
			// the task was found in task list
			_tasks.at(task_name).last_job_started = true;
		}
		else
		{
			// the task was not found in task list
			PWRN("Optimizer (last_job_started): The requested task %s was not in task list any more.", task_name.c_str());
		}
	}
	
	void Sched_opt::_remove_task(std::string task_str, unsigned int foc_id, Cause_of_death cause)
	{
		// remove task from _tasks list
		_tasks.erase(task_str);
		
		
		// remove task from competitor list of all tasks
		for (auto &it: _tasks)
		{
			// look in competitor list ...
			for(unsigned int i=0; i<it.second.competitor.size(); ++i)
			{
				// ... search task to be deleted ...
				if(!it.second.competitor[i].compare(task_str))
				{
					// ... delete the task ...
					it.second.competitor.erase(it.second.competitor.begin() + i);
					
					// ... and update to_schedule to avoid this task to wait for the deleted task
					if (it.second.competitor.empty())
						it.second.to_schedule = true;
					else
						_set_to_schedule(it.first);
					break;
				}
			}
		}
		
		// insert it to the list of ended tasks
		if(_ended_tasks.count(task_str)<1)
		{
			Ended_task task;
			task.name = task_str;
			task.last_foc_id = foc_id;
			task.cause_of_death = cause;
			
			_ended_tasks.insert({task_str, task});
		}
		
	}
	
	bool Sched_opt::change_core(std::string task_name, int core)
	{
		// This function handles the situation, when a task has changed its core.
		// It referes to the following global variables:
		//	_tasks -> name, deadline
		
		
		std::unordered_map<std::string, Optimization_task>::iterator it = _tasks.find(task_name);
		if(it != _tasks.end())
		{
			PINF("Optimizer: Change core of task %s from %d to %d.", task_name.c_str(), it->second.core, core);
			it->second.core = core;
			return true;
		}
		// look in _ended_tasks for requested task
		std::unordered_map<std::string, Ended_task>::iterator it_end = _ended_tasks.find(task_name);
		if(it_end != _ended_tasks.end())
		{
			
			// the requested task is in list of ended tasks
			std::string reason = (it_end->second.cause_of_death == FINISHED)? "the task has finished its last job" : "the task was killed";
			PINF("Optimizer: Requested task for changing core (%s) already ended (cause: %s).", task_name.c_str(), reason.c_str());
			return false;
			
		}
		PINF("Optimizer: Requested task for changing core (%s) was not found in task lisk of actual or ended tasks.", task_name.c_str());
		return false;
	}
	
	
	bool Sched_opt::scheduling_allowed(std::string task_name)
	{
		// This function looks up the to_schedule value of the requested task.
		// It should be called by Taskloader before starting a task (some where in _session_component::start()).
		// It referes to the following global variables:
		//	_tasks -> to_schedule
		//	_ended_tasks
		
		
		// look in _tasks for requested task
		std::unordered_map<std::string, Optimization_task>::iterator it = _tasks.find(task_name);
		if(it != _tasks.end())
		{
			PINF("Optimizer: Query scheduling allowance for task %s: %d", task_name.c_str(), it->second.to_schedule);
			return it->second.to_schedule;
		}
		
		// look in _ended_tasks for requested task
		std::unordered_map<std::string, Ended_task>::iterator it_end = _ended_tasks.find(task_name);
		if(it_end != _ended_tasks.end())
		{
			
			// the requested task is in list of ended tasks
			std::string reason = (it_end->second.cause_of_death == FINISHED)? "the task has finished its last job" : "the task was killed";
			PINF("Optimizer: Requested task for scheduling allowance (%s) already ended (cause: %s).", task_name.c_str(), reason.c_str());
			return false;
			
		}
		PINF("Optimizer: Requested task for scheduling allowance (%s) was not found in task lisk of actual or ended tasks.", task_name.c_str());
		return false;
	}
	
	
	void Sched_opt::start_optimizing()
	{
		// This function determines if any thask has a job which reached its time to have a deadline
		PDBG("Optimizer: ------------------------------- start optimizing.");
		
		/*
		while (_opt_goal != NONE)
		{
			unsigned long long current_time = timer.elapsed_ms();
		
			// check for all Tasks, whether inter_arrival time has elapsed
			for(auto &it: _tasks)
			{
				// if it's time to see what happend, ...
				if (current_time >= it.second.arrival_time + it.second.deadline)
				{
					//... query monitor-info about current task (was there any deadline miss?)
					_query_monitor(it.first, current_time);
				}
			}
			
			// wait some time to query the next monitor data
			timer.msleep(query_intervall);
		}
		
		// opt_goal was changed to none -> dont' query monitor any more
		PDBG("The Optimization goal was set to none. No Optimization is done until it is set again.");
		return;
		
		
		*/
		PDBG("BN ------------------------------- No optimization is done until now.");
		
	}
	
	
	void Sched_opt::_query_monitor(std::string task_str, unsigned long long current_time)
	{
		// This function query monitoring information and analyzes it. Then it reacts correspondingly by adjusting the value, reacting on deadline misses and setting the to_schedule flags.
		// Although it sets the arrival_time if there was a job.
		// It referes to the following global variables:
		//	_tasks -> name, arrival_time, deadline
		//	_threads -> thread_name, arrival_time, foc_id
		// 	_mon_manager -> update_info
		
		
		/*
		 *
		 * Step 1: query monitoring information
		 * 
		*/
		
		
		bool threads_array_ended = false;
		std::vector<unsigned int> new_threads_nr;
		
		// fill _threads with data
		_mon_manager->update_info(_mon_ds_cap);
		
		// loop through _threads array
		PINF("Optimizer: Searching in monitoring list for jobs with name %s", task_str.c_str());
		for(int j=0; j<100; ++j)
		{
			// determine unknown (new) jobs of given task
			if( !task_str.compare(_threads[j].thread_name.string()))
			{
				// matching task found -> check if this thread is a new job
				if(_threads[j].arrival_time >= _tasks.at(task_str).arrival_time)
				{
					PINF("Optimizer: Another new job with name %s", _threads[j].thread_name.string());
					new_threads_nr.push_back(j);
				}
			}
			
			// store foc_id to the correct task
			std::unordered_map<std::string, Optimization_task>::iterator it = _tasks.find(_threads[j].thread_name.string());
			if(it != _tasks.end())
			{
				// task for this thread found...
				if(_threads[j].arrival_time > it->second.newest_job.arrival_time)
				{
					PINF("Optimizer: Task %s has a new job with foc_id %d.", _threads[j].thread_name.string(), _threads[j].foc_id);
					it->second.newest_job.foc_id = _threads[j].foc_id;
					it->second.newest_job.dispatched = false;
				}
			}
			
			
			// end of threads-array reached?
			if(threads[j].foc_id == 0)
			{
				if (threads_array_ended)
				{
					// Reached end of monitoring list
					PINF("Optimizer: End of monitoring list reached.");
					break;
				}
				else
				{
					// this is the first element of the array. Next time, when id==0, the end of threads-array has reached
					PINF("Optimizer: First element in monitoring list detected.");
					threads_array_ended = true;
				}
			}
		}
		// end of threads loop
		
		
		
		
		/*
		 *
		 * Step 2: analyze monitoring info
		 *
		*/
		
		
		// determine if the recent job of the requested task started its execution
		// and react correspondingly (call _task_not_executed(...) or _task_executed(...))
		bool job_executed = false;
		
		switch (new_threads_nr.size())
		{
			case 0:
			{
				// there are no new tasks => job_executed remains false
				PINF("Optimizer: No new job for task %s", task_str.c_str());
				break;
			}
			case 1:
			{
				// there is only one new thread with _threads[j].arrival_time >= _tasks.at(task_str).arrival_time
				job_executed = true;
				
				bool deadline_time_reached = (current_time >= _threads[new_threads_nr[0]].arrival_time + _tasks.at(task_str).deadline);
				
				if (deadline_time_reached) // the job should have been executed
				{
					// toDo: Bei deadline miss ist task evtl. nicht mehr in monitoring list
					// determine if the job had a deadline miss or correct execution and set the to_schedules values
					_task_executed(task_str, new_threads_nr[0], true);
					
					// set the tasks execution time to the one of this job
					_tasks.at(task_str).execution_time = _threads[new_threads_nr[0]].execution_time.value;
				}
				// else: the job has still some time left for execution 
				
				// set arrival_time for current/next iteration
				_set_arrival_time(task_str, new_threads_nr[0], deadline_time_reached);
				break;
			}
			default:
			{
				// at least two threads are in new_threads_nr
				unsigned int most_recent_thread = -1;
				unsigned int second_recent_thread = -1;
				
				// find the two threads, which are most recent
				for(unsigned int i=0; i<new_threads_nr.size(); ++i)
				{

					if ( (most_recent_thread < 0) || (_threads[new_threads_nr[i]].arrival_time > _threads[most_recent_thread].arrival_time) )
						most_recent_thread = new_threads_nr[i];
					else if ( ((second_recent_thread < 0) || (_threads[new_threads_nr[i]].arrival_time > _threads[second_recent_thread].arrival_time)) )
						second_recent_thread = new_threads_nr[i];
				}
				
				if ( (most_recent_thread <0) || (second_recent_thread <0) )
				{
					PWRN("Optimizer: Although there are at least two threads, the recent threads weren't found in new_threads list.");
					// job_executed stays false
					break;
				}
				else
					job_executed = true;
				
				// determine if most recent thread has reached its deadline time
				bool recent_deadline_time_reached = (current_time >= _threads[most_recent_thread].arrival_time + _tasks.at(task_str).deadline);
					
				// change value of _tasks.at(task_str), react to deadline-misses and set to_schedule
				for(unsigned int i=0; i<new_threads_nr.size(); ++i)
				{
					// change value and update to_schedule
					if((i == most_recent_thread) && !recent_deadline_time_reached)
					{
						// the most recent thread has still some time to finish its execution -> don't change values or update to_schedule
						continue;
					}
					
					// only set to_schedule if the thread is the most recent thread which reached its deadline time
					bool consider_this_thread =( ((i == most_recent_thread) && recent_deadline_time_reached) || ((i == second_recent_thread) && !recent_deadline_time_reached) );
					
					_task_executed(task_str, i, consider_this_thread);
					
					if (consider_this_thread)
					{
						// set the tasks execution time to the one of this job
						_tasks.at(task_str).execution_time = _threads[i].execution_time.value;
						PINF("Optimizer: Set exec_time for task %s", task_str.c_str());
					}
				}
				
				// set arrival_time for next iteration
				_set_arrival_time(task_str, most_recent_thread, recent_deadline_time_reached);
			}
		}
		
		
		
		if(!job_executed)
		{
			// this task has no new job in threads array although it would be time to
			
			// if this task already started ...
			if(_tasks.at(task_str).arrival_time > 0)
			{
				// ... determine why it's not in monitoring list
				PINF("Optimizer: task %s had already had some tasks before, but no new one.", task_str.c_str());
				_task_not_executed(task_str);
			}
			// else: the task did not start until now -> query again later...
		}
		
	}
	
	
	void Sched_opt::_set_arrival_time(std::string task_str, unsigned int thread_nr, bool deadline_time_reached)
	{
		// This function sets the arrival_time of the given task
		// It referes to the following global variables:
		//	_tasks -> arrival_time, inter_arrival
		//	_threads -> arrival_time
		
		_tasks.at(task_str).arrival_time = _threads[thread_nr].arrival_time;
		if (deadline_time_reached)
			_tasks.at(task_str).arrival_time += _tasks.at(task_str).inter_arrival;
	}
	
	void Sched_opt::_task_executed(std::string task_str, unsigned int thread_nr, bool set_to_schedules)
	{
		// This function handles the situation, when a job of a task should have been executed and its deadline time has already reched.
		// Depending on the exit_time of the thread, the tasks value is in-/decreased and the deadline miss is handled
		// It referes to the following global variables:
		//	_tasks -> value, deadline, competitor (name), core
		//	_threads -> arrival_time, exit_time
	
		unsigned int core = _tasks.at(task_str).core;
		
		
		// determine if there was an soft-exit before reaching the deadline time
		if((_threads[thread_nr].exit_time > 0) && (_threads[thread_nr].exit_time <= _threads[thread_nr].arrival_time + _tasks.at(task_str).deadline))
		{
			// job was executed before reaching its deadline
			
			// reduce value
			if(_tasks.at(task_str).value[core] > 0)
				_tasks.at(task_str).value[core] --;
		}
		else
		{
			// job reached its deadline before finishing its execution (= deadline miss)
			_job_reached_deadline(task_str);
		}
		
		// calculate the utilization
		if(_threads[thread_nr].exit_time > 0)
		{
			unsigned int exec_diff = _threads[thread_nr].exit_time - _tasks.at(task_str).execution_time;
			double new_util = exec_diff / _tasks.at(task_str).inter_arrival;
			_tasks.at(task_str).utilization = new_util;
		}
		
		// if the to_schedules shall be set, ...
		if(set_to_schedules)
		{
			_set_to_schedule(task_str);
			
			// this task is the newest one known to monitoring list, which reached its deadline
			
			// indicate that it was handled by the optimizer
			if(_threads[thread_nr].foc_id == _tasks.at(task_str).newest_job.foc_id)
				_tasks.at(task_str).newest_job.dispatched = true; 
			else
			{
				if(_threads[thread_nr].arrival_time <= _tasks.at(task_str).newest_job.arrival_time)
				{
					// This thread is not the real newest thread. There is another thread which was known to the monitoring list, but isn't there any more.
					// the newest_job might had a deadline_miss, and only an older one was found in the list which wasn't handled jet
					PDBG("Optimizer(_query_monitoring): Thread %d was dispatched and its deadline_time was reached, but it is not the newest thread (since thread with foc_id %d is the newest one), and the optimizer did'nt found it in monitoring list.", _threads[thread_nr].foc_id, _tasks.at(task_str).newest_job.foc_id);
				}
				else
					PWRN("Optimizer (_query_monitoring): Thread %d was dispatched, but its start time is newer than the newest_job (with foc_id %d). How can this be?", _threads[thread_nr].foc_id, _tasks.at(task_str).newest_job.foc_id);
			}
			
		}
	}
	
	void Sched_opt::_task_not_executed(std::string task_str)
	{
		// This function handles the situation, when the task has elapsed its inter_arrival time, but the job was not executed
		// It referes to the following global variables:
		//	_tasks
		
		
		
		// check if its newest_job has the desired arrival_time
		// => job has reached deadline an thread is not in Monitoring list
		// -> check rip list for this foc_id: if the exit_time ~ deadline => job reached its deadline
		
		
		// should this task be executed?
		if(!_tasks.at(task_str).to_schedule)
		
		{
			// this task should not be executed an wasn't -> increase value ...
			_tasks.at(task_str).value[_tasks.at(task_str).core] ++;
		
			// ... and update the to_schedule flags
			_set_to_schedule(task_str);
		}
		else
		{
			// The task should be executed but is not in monitoring list.
			
			// check if there was a newer thread in monitoring list before (which is not there any more) or if the last job of the task was executed
			
			if(! _tasks.at(task_str).newest_job.dispatched) // if the newest job was not already handled before
			{
				// the newest task was not already handled by the optimizer
				if(_tasks.at(task_str).arrival_time > _tasks.at(task_str).newest_job.arrival_time)
				{
					// The newest_job was not detected correctly
					//	=> there is no other task (which could set the dispatched-value)
					//	or the deadline of the other task is in between the deadline of this task and the actual start time of next thread of this task
					PWRN("Optimizer (_task_not_executed): The newest_job was not detected correcly.");
				
					// thus the matching of foc_id and task won't worke for rip list
				}
				else
				{
					// check the RIP list if task was killed by user <-> job had deadline miss <-> newest job is neißer in rip nor in monitoring list
					bool task_in_rip = false;
				
					// fill rip list with data
					_mon_manager->update_dead(_dead_ds_cap);
		
					// rip is a 'list of tuples (foc_id, time)' similar to RQ list of Monitor
					// RIP table size is shown in rip[0]
					for (unsigned int i=1; i<rip[0]; ++i)
					{
						// foc_id is rip[2*i-1], time is rip[2*i]/1000
						if(_tasks.at(task_str).newest_job.foc_id == rip[2*i-1])
						{
							// the task was found in rip list
							task_in_rip = true;
				
							// check if this task was killed by user <-> job reached its deadline
				
							//check if deadline was reached
							if(rip[2*i] >= _tasks.at(task_str).newest_job.arrival_time + _tasks.at(task_str).deadline)
							{
								// the thread has reached its deadline
								_job_reached_deadline(task_str);
							}
							else // the task was killed by the user
							{
								// remove it from the _tasks list
								_remove_task(task_str, rip[2*i-1], KILLED);
							}
						}
					}
				
					if(!task_in_rip)
					{
						// the task was not in RIP list and also not in monitoring list
						PWRN("The task %s was neither in monitoring nor in rip list.", task_str.c_str());
					}
				}
			}
			else
			{	//the newest job was already handled by the optimizer		
			
				// check if the task has probably finished its last job
				if(_tasks.at(task_str).last_job_started)
				{
					// the taskloader already told the optimizer about the starting of the last job
					// -> remove this task from task list
					_remove_task(task_str, _tasks.at(task_str).newest_job.foc_id, FINISHED);
				}
				// check if the newest_job wasn't set correctly
				else if(_tasks.at(task_str).arrival_time > _tasks.at(task_str).newest_job.arrival_time)
				{
					// The newest_job was not detected correctly
					//	=> there is no other task (which could set the dispatched-value)
					//	or the deadline of the other task is in between the deadline of this task and the actual start time of next thread of this task
					PWRN("Optimizer (_task_not_executed): The newest_job was not detected correcly.");
				}
				else
				{
					// Current Situation:
					// The deadline time for the newest job has reached, the job was allowed to run.
					// The newest_job was already handled by the optimizer and no new thread was found in monitoring list.
					// The last job of this task will still occur later so this is not the last job.
					PWRN("Optimizer (_task_not_executed): The task should be executed but wasn't.");
				}
			}
		}
	}
	
	void Sched_opt::_set_to_schedule(std::string task_str)
	{	
		// This function sets the to_schedule of the thask with max value/max utilization to true.
		// It referes to the following global variables:
		//	_tasks -> competitor, core, value, to_schedule, utilization
		
		if(!_tasks.at(task_str).competitor.empty())
		{
			// find the task with max value and the task with max utilization
			std::string max_value_str = std::string();
			std::string max_util_str = std::string();
			for(unsigned int i=0; i<_tasks.at(task_str).competitor.size(); ++i)
			{
				std::string comp_str = _tasks.at(task_str).competitor[i];
				if (_tasks.at(comp_str).core == _tasks.at(task_str).core)
				{
					// find the task with max value
					if(_tasks.at(comp_str).value[_tasks.at(comp_str).core] > _tasks.at(task_str).value[_tasks.at(task_str).core])
					{
						if((max_value_str.empty()) || (_tasks.at(comp_str).value[_tasks.at(comp_str).core] > _tasks.at(max_value_str).value[_tasks.at(max_value_str).core]))
							max_value_str = comp_str;
					}
					// find the task with max utilization
					if(_tasks.at(comp_str).utilization > _tasks.at(task_str).utilization)
					{
						if((max_util_str.empty()) || (_tasks.at(comp_str).utilization > _tasks.at(max_util_str).utilization))
							max_util_str = comp_str;
					}
				}
			}
			
			
			switch( _opt_goal )
			{
				case FAIRNESS:
				{
					PDBG("The optimization goal 'fairness' is used.");
					
					if(max_value_str.empty()) // this task is the one with max value
					{
						// allow this task to be scheduled and all competitors not
						_tasks.at(task_str).to_schedule = true;
						for (unsigned int i=0; i<_tasks.at(task_str).competitor.size(); ++i)
						{
							_tasks.at(_tasks.at(task_str).competitor[i]).to_schedule = false;
						}
					}
					else // the task _tasks.at(max_val_str) is the one with max value
					{
						// don't allow this task to be scheduled, but the one with max value
						_tasks.at(task_str).to_schedule = false;
						_tasks.at(max_value_str).to_schedule = true;
					}
					break;
				}
				case UTILIZATION:
				{
					PDBG("The optimization goal 'utilization' is used.");
					
					if(max_util_str.empty()) // this task is the one with max utilization
					{
						// allow this task to be scheduled and all competitors not
						_tasks.at(task_str).to_schedule = true;
						for (unsigned int i=0; i<_tasks.at(task_str).competitor.size(); ++i)
						{
							_tasks.at(_tasks.at(task_str).competitor[i]).to_schedule = false;
						}
					}
					else // the task _tasks.at(max_util_str) is the one with max utilization
					{
						// don't allow this task to be scheduled, but the one with max utilization
						_tasks.at(task_str).to_schedule = false;
						_tasks.at(max_util_str).to_schedule = true;
					}
					break;
				}
				default:
					PDBG("No optimization goal is set, hence the task scheduling is not influenced.");
				
			}
		}
		
	}
	
	void Sched_opt::_job_reached_deadline(std::string task_str)
	{
	
		unsigned int core = _tasks.at(task_str).core;
		
		// increase value and indicate an overload at this core
		_tasks.at(task_str).value[core] ++;
		_tasks.at(task_str).overload[core] = true;
		
		// find causation task
		std::string cause_task_str = _get_cause_task(task_str);
		if(cause_task_str.empty())
		{
			// The task reached its deadline an no task in monitoring list caused this ???
			PWRN("Optimizer: The current job of task %s reached its deadline although there is no cause thread in monitoring data.", task_str.c_str());
			
		}
		else
		{
			// check, if causation task is already in list of competitors
			bool cause_already_at_competitors = false;
			for(unsigned int i = 0; i<_tasks.at(task_str).competitor.size(); ++i)
			{
				if (!_tasks.at(task_str).competitor[i].compare(cause_task_str))
					cause_already_at_competitors = true;
			}
			if (cause_already_at_competitors)
			{
				// This situation may happen if a task doesn't know about this task to be its cometitor
				// and allowed a competitor of this task to be executed too 
				PWRN("Optimizer: The Task %s is already in competitors list of task %s, but %s had a deadline miss because of it.", cause_task_str.c_str(), task_str.c_str(), task_str.c_str());
			}
			else
			{
				// add causation task to competitor list
				_tasks.at(task_str).competitor.emplace_back(cause_task_str);
			}
		}
	}
	std::string Sched_opt::_get_cause_task(std::string task_str)
	{
		// This function queries the monitoring objects (threads) to find the thread which caused the deadline miss for the task
		// It referes to the following global variables:
		//	_threads -> arrival_time, exit_time, foc_id, thread_name
		//	_tasks -> deadline, name
		
		int cause_thread_nr = -1;
		
		// time interval in which the threads exit time hast to be in: exit_time shall be after threads arrival_time and before threads deadline
		unsigned long long thread_start = _tasks.at(task_str).arrival_time;
		unsigned long long thread_deadline = _tasks.at(task_str).arrival_time + _tasks.at(task_str).deadline;
		
		// query monitoring objects to find a matching thread
		bool threads_array_ended = false;
		
		// loop through threads array
		for(unsigned int i=0; i<100; ++i)
		{
			// find thread which exit-time is the most recent in the interval [thread_start, thread_deadline], so it is the causation thread
			// matching task found -> check if exit_time is in considered time interval
			if((threads[i].exit_time >= thread_start) && (threads[i].exit_time <= thread_deadline))
			{
				// this is a possible cause task. I need the most recent one
				if((cause_thread_nr < 0) || (threads[i].exit_time > threads[cause_thread_nr].exit_time))
				{
					cause_thread_nr = i;
				}
			}
			
			
			// end of threads-array reached?
			if(threads[i].foc_id == 0)
			{
				if (threads_array_ended)
				{
					// Reached end of monitoring list
					break;
				}
				// else: this is the first element of the array.
				// the next time, when id==0, the end of threads-array has reached
				threads_array_ended = true;
			}
		}
		// end of threads loop
		
		// return thread name, if cause thread was found in monitoring list
		if((cause_thread_nr >= 0) && (_tasks.count(_threads[cause_thread_nr].thread_name.string()) > 0))
		{
			return _threads[cause_thread_nr].thread_name.string();
		}
		else
		{
			// maybe a task, which reached its deadline caused this deadline miss
			// toDo: check rip list for task which exit_time is in desired interval
			return std::string();
		}
	}
	
	void Sched_opt::run_job()
	{
		// This should be called by Taskloader_session_component::start(), before starting a task
		
		
		// goal: start job of task via adding it to rqs
		// Step 1: update rqs via monitor
		// Step 2: only add job if allowed
		
		
		// remove old entries from RunQueue
		//_rqs.init_w_shared_ds(_sync_ds_cap);
		
		
		
		// fill rqs with data
		_mon_manager->update_rqs(_rq_ds_cap);
		
		// fill threads with data
		_mon_manager->update_info(_mon_ds_cap);
		
		
		
		// Loop through rqs-Array
		for (int i = 1; i<= rqs[0]; ++i)
		{
			unsigned int rqs_foc_id = rqs[2*i-1];
			
			// Loop through threads-Array
			for(int j=0; j<100; ++j)
			{
				if(threads[j].foc_id == rqs_foc_id)
				{
					// found matching of monitoring_threads <-> monitoring_rqs
					
					// look for task label of thread in _tasks
					
					// toDo: do this via map
					
					bool tasks_found = false;
					for(unsigned int k=0; k<_tasks.size(); ++k)
					{
						if(!_tasks[k].name.compare(threads[j].thread_name.string()))
						{
							//Task found in map (threads[j] ~ _tasks[k])
							tasks_found = true;
							PINF("Job with foc-id %d and label %s was found in _tasks", rqs_foc_id, _tasks[k].name.c_str());
						
							// Add tasks to RunQueue
							if (_tasks[k].to_schedule)
							{
				
								_rqs->enq(_tasks[k].rq_task);
				
								/*
								// erstelle neuen Rq_Task für enq
								Rq_task::Rq_task new_task;
								
								new_task.task_id = _tasks[k].rq_task.task_id; //rqs_foc_id;
								new_task.task_class = _tasks[k].rq_task.task_class;
								new_task.task_strategy = _tasks[k].rq_task.task_strategy;
								new_task.deadline = _tasks[k].rq_task.deadline;
								new_task.wcet = _tasks[k].rq_task.wcet;
								new_task.inter_arrival = _tasks[k].rq_task.inter_arrival;
								new_task.prio = _tasks[k].rq_task.prio;
								new_task.valid = _tasks[k].rq_task.valid;
								strcpy(new_task.name, _tasks[k].rq_task.name);
				
								//_rqs.enq(new_task);
				
								*/
							}
							
						}
					}
					if(!tasks_found)
					{
						//label not found in map --> task is not managed and will be ignored for runqueue
						PINF("Job with foc-id %d and label %s was not found in _tasks", rqs_foc_id, threads[j].thread_name.string());
					}
					
					// Beende die Schleife durch threads-Array, da kein zweiter job zur foc_id passen wird
					break;
				}

				if(threads[j].foc_id == 0)
				{
					//Reached end of monitoring list without finding a match
					PINF("No entry found in monitoring list for task with foc-id %d", rqs_foc_id);
					break;
				}
			}
			// Ende von Schleife durch threads-Array
			
		}
		// Ende von Schleife durch rqs-Array
		
		
	}
	
	
	
	Sched_opt::Sched_opt(int sched_num_cores, Mon_manager::Connection *mon_manager, Mon_manager::Monitoring_object *sched_threads, Genode::Dataspace_capability mon_ds_cap, Genode::Dataspace_capability dead_ds_cap)

	{
		// set runqueue, which shall be influenced
		_rqs = sched__rqs;
		
		// set variables for querying monitor data
		_mon_manager = sched_mon_manager;
		threads = sched_threads;
		_mon_ds_cap = sched_mon_ds_cap;
		
		rqs = sched_rqs;
		_rq_ds_cap = sched_rq_ds_cap;
		_sync_ds_cap = Genode::env()->ram_session()->alloc(100*sizeof(int));
		
		_sync_ds_cap = schred_sync_ds_cap;
		
		// set variables for querying rip list
		_dead_ds_cap = dead_ds_cap;
		rip = Genode::env()->rm_session()->attach(_dead_ds_cap);
		
		// set the number of cores to handle multicore optimization
		num_cores = sched_num_cores;
		
		// default optimization goal = no optimization is done
		_opt_goal = NONE;
		
		
		// default value for the acceptance interval
		accept = 0;
		
		// number of ms to sleep between two intervalls to query the monitor data
		query_intervall = 100;
	}
	
	
	Sched_opt::~Sched_opt()
	{
	}


}
