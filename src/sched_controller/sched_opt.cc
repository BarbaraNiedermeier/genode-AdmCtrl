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
	
	

	void Sched_opt::add_task(int core, Rq_task::Rq_task task)
	{
		// Es kommt immer nur ein neuer Task hinzu. Dieser Muss identifiziert werden und in das Task-Array eingefügt werden.
		PDBG("Optimizer - Add task to _tasks array.");
		
		// convert newly arriving task to optimization task
		Optimization_task _task;
		

		_task.deadline = task.deadline;
		_task.inter_arrival = task.inter_arrival;
		_task.name = std::string(task.name); // das geht erst, wenn Masterarebit von Steffan fertig ist
		
		_task.start_time = 0;
		_task.to_schedule = true;
		_task.core = core;
		_task.newest_job.foc_id = 0;
		_task.newest_job.start_time = 0;
		
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
		
	}
	

	void Sched_opt::task_removed(int core, Rq_task::Rq_task **task_ptr)
	{
		// this function is called by the sched_controller when dequeueing the task with given name.
		
		std::string task_name = std::string((*task_ptr)->name);
		
		for (unsigned int task=0; task < _tasks.size(); ++task)
		{
			if(!_tasks[task].name.compare(task_name))
			{
				// the task was found in task array -> remove it an all its occurances in the competitor lists
				_remove_task(task);
			}
		}
	}
	
	void Sched_opt::_remove_task(unsigned int task_nr)
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
	

	bool Sched_opt::change_core(int core, std::string task_name)
	{
		// This function handles the situation, when a task has changed its core.
		// It referes to the following global variables:
		//	_tasks -> name, deadline
		

		for (unsigned int i=0; i<_tasks.size(); ++i)
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
	
	

	bool Sched_opt::scheduling_allowed()
	{
		
		// This should be called by Taskloader_session_component::start(), before starting a task
		
		// make an Task as input-parameter
		// get its task_name
		// look in _tasks for task_name
		// return its value of to_schedule
		
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

				if (current_time >= _tasks[i].start_time + _tasks[i].deadline)
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
	
	
	void Sched_opt::_query_monitor(int task_nr, unsigned long long current_time)
	{
		// This function query monitoring information and analyzes it. Then it reacts correspondingly by adjusting the value, reacting on deadline misses and setting the to_schedule flags.
		// Although it sets the arrival_time if there was a job.
		// It referes to the following global variables:
		//	_tasks -> name, start_time, deadline
		//	_threads -> thread_name, start_time, foc_id
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
		PINF("Optimizer: Searching in monitoring list for jobs with name %s", _tasks[task_nr].name.c_str());
		for(int j=0; j<100; ++j)
		{
			// determine unknown (new) jobs of given task
			if( !_tasks[task_nr].name.compare(_threads[j].thread_name.string()))
			{
				// matching task found -> check if this thread is a new job
				if(_threads[j].start_time >= _tasks[task_nr].start_time)
				{
					PINF("Optimizer: Another new job with name %s", _threads[j].thread_name.string());
					new_threads_nr.push_back(j);
				}
			}
			
			// store foc_id to the correct task
			for(unsigned int task = 0; task<_tasks.size(); ++task)
			{
				if(!_tasks[task].name.compare(_threads[j].thread_name.string()))
				{
					// task for this thread found...
					if(_threads[j].start_time > _tasks[task].newest_job.start_time)
						_tasks[task].newest_job.foc_id = _threads[j].foc_id;
				}
			}
				
			
			
			
			// end of threads-array reached?
			if(_threads[j].foc_id == 0)
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
				// there are no new tasks
				// job_executed stays false
				PINF("Optimizer: No new job for task %s", _tasks[task_nr].name.c_str());
				break;
			}
			case 1:
			{
				// there is only one new thread with _threads[j].start_time >= _tasks[task_nr].start_time
				job_executed = true;
				
				bool deadline_time_reached = (current_time >= _threads[new_threads_nr[0]].start_time + _tasks[task_nr].deadline);
				
				if (deadline_time_reached) // the job has no time left to be executed
				{
					// the task should now have been executed
					
					// determine if it had a deadline miss or correct execution
					// -> update value correspondingly and react to deadline miss
					// set the to_schedules values
					
					_task_executed(task_nr, new_threads_nr[0], true);
					
					// set the tasks execution time to the one of this job
					_tasks[task_nr].execution_time = _threads[new_threads_nr[0]].execution_time.value;
					
				}
				// else: the job has still some time left for execution 
				
				// set start_time for current/next iteration
				_set_start_time(task_nr, new_threads_nr[0], deadline_time_reached);
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
					if ( (most_recent_thread < 0) || (_threads[new_threads_nr[i]].start_time > _threads[most_recent_thread].start_time) )
						most_recent_thread = new_threads_nr[i];
					else if ( ((second_recent_thread < 0) || (_threads[new_threads_nr[i]].start_time > _threads[second_recent_thread].start_time)) )
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
				bool recent_deadline_time_reached = (current_time >= _threads[most_recent_thread].start_time + _tasks[task_nr].deadline);
					
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
					bool to_schedule =( ((i == most_recent_thread) && recent_deadline_time_reached) || ((i == second_recent_thread) && !recent_deadline_time_reached) );
					
					_task_executed(task_nr, i, to_schedule);
					
					if (to_schedule)
					{
						// set the tasks execution time to the one of this job
						_tasks[task_nr].execution_time = _threads[i].execution_time.value;
					}
				}
				
				// set start_time for next iteration
				_set_start_time(task_nr, most_recent_thread, recent_deadline_time_reached);
				
			}
		}
		
		
		
		if(!job_executed)
		{
			// this task has no new job in threads array although it would be time to
			
		
			if(_tasks[task_nr].start_time > 0)
			{
				PINF("Optimizer: task %s had already had some tasks before, but no new one.", _tasks[task_nr].name.c_str());
				_task_not_executed(task_nr);
			}
			// If there is no thread in monitoring data that matches this task and the start_time is not set until now
			// -> query again later...
		}
		
	}
	
	
	void Sched_opt::_set_arrival_time(std::string task_str, unsigned int thread_nr, bool deadline_time_reached)
	{
		// This function sets the arrival_time of the given task
		// It referes to the following global variables:
		//	_tasks -> start_time, inter_arrival
		//	_threads -> start_time
		
		_tasks[task_nr].start_time = _threads[thread_nr].start_time;
		if (deadline_time_reached)
			_tasks[task_nr].start_time += _tasks[task_nr].inter_arrival;
	}
	
	void Sched_opt::_task_executed(std::string task_str, unsigned int thread_nr, bool set_to_schedules)
	{
		// This function handles the situation, when a job of a task should have been executed and its deadline time has already reched.
		// Depending on the exit_time of the thread, the tasks value is in-/decreased and the deadline miss is handled
		// It referes to the following global variables:
		//	_tasks -> value, deadline, competitor (name), core
		//	_threads -> start_time, exit_time
	
		unsigned int core = _tasks[task_nr].core;
		
		
		// determine if there was an soft-exit before reaching the deadline time
		if((_threads[thread_nr].exit_time > 0) && (_threads[thread_nr].exit_time <= _threads[thread_nr].start_time + _tasks[task_nr].deadline))
		{
			// job was executed before reaching its deadline
			
			// reduce value
			if(_tasks.at(task_str).value[core] > 0)
				_tasks.at(task_str).value[core] --;
		}
		else
		{
			// job reached its deadline before finishing its execution (= deadline miss)
			// toDo: this call is related since the threads which reached their deadlines ar not in monitoring threads any more
			_deadline_reached(task_str);
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
		
		// calculate the utilization
		if(_threads[thread_nr].exit_time > 0)
		{
			unsigned int exec_diff = _threads[thread_nr].exit_time - _tasks[task_nr].execution_time;
			double new_util = exec_diff / _tasks[task_nr].inter_arrival;
			_tasks[task_nr].utilization = new_util;
		}
		
		// if the to_schedules shall be set, ...
		if(set_to_schedules)
			_set_to_schedule(task_nr);
	}
	
	void Sched_opt::_task_not_executed(std::string task_str)
	{
		// This function handles the situation, when the task has elapsed its inter_arrival time, but the job was not executed
		// It referes to the following global variables:
		//	_tasks -> to_schedule, value, core
		
		
		// should this task be executed?
		if(!_tasks.at(task_str).to_schedule)
		
		{
			// The task should be executed but didn't start.
			// The task might have an hard exit or the last job finished its exeution. In both cases it has to be deleted from _tasks
			
			// toDo: Query rip list
			// if it had an hard exit and is now in RIP list, ...
			if (_query_rip_list(task_nr))
			{
				//... delete it from _tasks
				_remove_task(task_nr);
			}
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
				
					// thus the matching of foc_id and task wouldn't work for rip list
				}
				else
				{
					// check the RIP list if task was killed by user <-> job had deadline miss <-> newest job is neißer in rip nor in monitoring list
					bool task_in_rip = false;
				
					// fill rip list with data
					_mon_manager->update_dead(_dead_ds_cap);
		
					// rip is a 'list of tuples (foc_id, time)' similar to RQ list of Monitor
					// RIP table size is shown in rip[0]
					for (unsigned int i=1; i<rip[0]*2+1; i+=2)
					{
						// foc_id is rip[i], time is rip[i+1]/1000
						if(_tasks.at(task_str).newest_job.foc_id == rip[i])
						{
							// the task was found in rip list
							task_in_rip = true;
				
							// check if this task was killed by user <-> job reached its deadline
				
							//check if deadline was reached
							if(rip[i+1] >= _tasks.at(task_str).newest_job.arrival_time + _tasks.at(task_str).deadline)
							{
								
								// check for core change
								if(_tasks.at(task_str).core != _tasks.at(task_str).newest_job.core)
								{
									PWRN("Optimizer (_task_not_executed): The task %s has changed its core from core-%d to core-%d.", task_str.c_str(), _tasks.at(task_str).core, _tasks.at(task_str).newest_job.core);
									_tasks.at(task_str).core = _tasks.at(task_str).newest_job.core;
								}
								
								// the thread has reached its deadline
								_deadline_reached(task_str);
							}
							else // the task was killed by the user
							{
								// remove it from the _tasks list
								_remove_task(task_str, rip[i], KILLED);
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
	
	void Sched_opt::_set_to_schedule(unsigned int task_nr)
	{	
		// This function sets the to_schedule of the thask with max value/max utilization to true.
		// It referes to the following global variables:
		//	_tasks -> competitor, core, value, to_schedule, utilization
		
		if(!_tasks.at(task_str).competitor.empty())
		{
			// find the task with max value and the task with max utilization
			unsigned int max_value_nr = -1;
			unsigned int max_util_nr = -1;
			for(unsigned int i=0; i<_tasks[task_nr].competitor.size(); ++i)
			{
				if (_tasks[_tasks[task_nr].competitor[i]].core == _tasks[task_nr].core)
				{
					// find the task with max value
					if(_tasks[_tasks[task_nr].competitor[i]].value[_tasks[_tasks[task_nr].competitor[i]].core] > _tasks[task_nr].value[_tasks[task_nr].core])
					{
						if((max_value_nr < 0) || (_tasks[_tasks[task_nr].competitor[i]].value[_tasks[_tasks[task_nr].competitor[i]].core] > _tasks[max_value_nr].value[_tasks[max_value_nr].core]))
							max_value_nr = _tasks[task_nr].competitor[i];
					}
					// find the task with max utilization
					if(_tasks[_tasks[task_nr].competitor[i]].utilization > _tasks[task_nr].utilization)
					{
						if((max_util_nr < 0) || (_tasks[_tasks[task_nr].competitor[i]].utilization > _tasks[max_util_nr].utilization))
							max_util_nr = _tasks[task_nr].competitor[i];
					}
				}
			}
			
			
			switch( _opt_goal )
			{
				case FAIRNESS:
				{
					PDBG("The optimization goal 'fairness' will be used.");
					
					if(max_value_nr < 0)
					{
						// this task is the one with max value
						_tasks[task_nr].to_schedule = true;
						for (unsigned int i=0; i<_tasks[task_nr].competitor.size(); ++i)
						{
							_tasks[_tasks[task_nr].competitor[i]].to_schedule = false;
						}
					}
					else
					{
						// the task _tasks[max_value_nr] is the one with max value
						_tasks[task_nr].to_schedule = false;
						_tasks[max_value_nr].to_schedule = true;
					}
					break;
				}
				case UTILIZATION:
				{
					PDBG("The optimization goal 'utilization' will be used.");
					
					if(max_util_nr < 0)
					{
						// this task is the one with max utilization
						_tasks[task_nr].to_schedule = true;
						for (unsigned int i=0; i<_tasks[task_nr].competitor.size(); ++i)
						{
							_tasks[_tasks[task_nr].competitor[i]].to_schedule = false;
						}
					}
					else
					{
						// the task _tasks[max_util_nr] is the one with max utilization
						_tasks[task_nr].to_schedule = false;
						_tasks[max_util_nr].to_schedule = true;
					}
					break;
				}
				default:
					PDBG("No optimization goal is set, hence the task scheduling is not influenced.");
				
			}
		}
		
		
		unsigned int core = _tasks.at(task_str).core;
		
		//indicate an overload at this core
		_tasks.at(task_str).overload[core] = true;
		
		// increase value and check max_value
		_tasks.at(task_str).value[core] ++;
		if(_tasks.at(task_str).id_related <= 0)
			PWRN("Optimizer: The task %s has no id_related althought it should have been updated priorly.", task_str.c_str());
		else if(_tasks.at(task_str).value[core] >= _related_tasks.at(_tasks.at(task_str).id_related).max_value)
		{
			// check if all tasks at the list have reached the max_value
			bool reduce_values = true;
			unsigned int list_id = _tasks.at(task_str).id_related;
			for(const std::string& task: _related_tasks.at(list_id).tasks)
			{
				// only consider related tasks, which are currently working on the same core (core of the considered task)
				if ((_tasks.at(task).core == core) && (_tasks.at(task).value[core] < _related_tasks.at(list_id).max_value))
				{
					reduce_values = false;
					break;
				}
			}
			
			if(reduce_values)
			{
				for(const std::string& task: _related_tasks.at(list_id).tasks)
				{
					// also consider related tasks, which are not currently working on the same core (core of the considered task)
					// Reason: Avoid a task waiting to reduce its value due to a other task which cannot update its value of this core since it isn't executed there
					if(_tasks.at(task).value[core] >= _related_tasks.at(list_id).max_value)
						_tasks.at(task).value[core] -= _related_tasks.at(list_id).max_value;
					else
						_tasks.at(task).value[core] = 0;
				}
			}
			
		}
	}
	
	std::string Sched_opt::_get_cause_task(std::string task_str)
	{
		// This function queries the monitoring objects (threads) to find the thread which caused the deadline miss for the task
		// It referes to the following global variables:
		//	_threads -> start_time, exit_time, foc_id, thread_name
		//	_tasks -> deadline, name
		
		int cause_thread_nr = -1;
		
		// time interval in which the threads exit time hast to be in: exit_time shall be after threads start_time and before threads deadline
		unsigned long long thread_start = _threads[thread_nr].start_time;
		unsigned long long thread_deadline = _threads[thread_nr].start_time + _tasks[task_nr].deadline;
		
		// query monitoring objects to find a matching thread
		bool threads_array_ended = false;
		
		// loop through threads array
		for(unsigned int i=0; i<100; ++i)
		{
			// find thread which exit-time is the most recent in the interval [thread_start, thread_deadline], so it is the causation thread
			// matching task found -> check if exit_time is in considered time interval
			if((_threads[i].exit_time >= thread_start) && (_threads[i].exit_time <= thread_deadline))
			{
				// this is a possible cause task. I need the most recent one
				if((cause_thread_nr < 0) || (_threads[i].exit_time > _threads[cause_thread_nr].exit_time))
				{
					cause_thread_nr = i;
				}
			}
			
			
			// end of threads-array reached?
			if(_threads[i].foc_id == 0)
			{
				if (threads_array_ended)
				{
					// Reached end of monitoring list without finding a matching thread to _tasks[task_nr]
					PINF("No entry found in monitoring list for task with name %s", _threads[i].thread_name.string());
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
			return _threads[cause_thread_nr].thread_name.string();
		
		// else : maybe cause task also reached its deadline before the job of the considered task
		// check rip list for cause task whith exit_time in desired interval
		long long unsigned job_foc_id;
		long long unsigned latest_rip_time = 0;
	
		// fill rip list with data
		_mon_manager->update_dead(_dead_ds_cap);

		// query rip list
		for (unsigned int i=1; i<rip[0]*2+1; i+=2)
		{
			//check if thread had rip-time in interval
			if((rip[i+1] >= thread_start) && (rip[i+1] <= thread_deadline))
			{
				if(!_tasks[i].name.compare(_threads[cause_thread_nr].thread_name.string()))
					return i;
			}
		}
	
	bool Sched_opt::_query_rip_list(unsigned int task_nr)
	{
		// This function queries the monitoring objects (threads) to find the thread which caused the deadline miss for thread with thread_nr
		// It referes to the following global variables:
		//	_threads -> newest_job.foc_id
		// 	_mon_manager -> update_dead
		//	_dead_cs_cap
		
		
		
		long long unsigned *rip = Genode::env()->rm_session()->attach(_dead_ds_cap);
		
		// fill rip list with data
		_mon_manager->update_dead(_dead_ds_cap);
		
		
		// rip is a 'list of tuples (foc_id, time)' similar to RQ list of Monitor
		// RIP table size is shown in rip[0]
		for (unsigned int i=1; i<rip[0]; ++i)
		{
			// foc_id is rip[2*i-1], time is rip[2*i]/1000
			if(_tasks[task_nr].newest_job.foc_id == rip[2*i-1])
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	Sched_opt::Sched_opt(int sched_num_cores, Mon_manager::Connection *mon_manager, Mon_manager::Monitoring_object *sched_threads, Genode::Dataspace_capability mon_ds_cap, Genode::Dataspace_capability dead_ds_cap)
	{
		
		// set variables for querying monitor data
		_mon_manager = mon_manager;
		_threads = sched_threads;
		_mon_ds_cap = mon_ds_cap;
		
		// set variables for querying rip list
		_dead_ds_cap = dead_ds_cap;
		
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
