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
		// Es kommt immer nur ein neuer Task hinzu. Dieser Muss identifiziert werden und in das Task-Array eingefügt werden.
		PDBG("Optimizer - Add task to _tasks array.");
		// convert newly arriving task to optimization task
		Optimization_task _task;
		
		_task.rq_task = task;
		_task.name = std::string(task.name);// das geht erst, wenn Masterarebit von Steffan fertig ist
		_task.start_time = 0;
		_task.task_ptr = nullptr;
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
		
		
		_tasks.push_back(_task);
		
		// this return value is needed for call from enq
		return -1;
	}
	
	
	void Sched_opt::_remove_task(unsigned int task_nr)
	{
		//int core = _tasks[task_nr].core;
		
		// remove task from _tasks vector
		_tasks.erase(_tasks.begin() + task_nr);
		
		
		// remove task from competitor list of all tasks
		for (unsigned int task=0; task<_tasks.size(); ++task)
		{
			// look in competitor list ...
			for(unsigned int i=0; i<_tasks[task].competitor.size(); ++i)
			{
				// ... search task to be deleted ...
				if(_tasks[task].competitor[i] == task_nr)
				{
					// ... delete the task ...
					_tasks[task].competitor.erase(_tasks[task].competitor.begin() + i);
					
					// ... and update to_schedule to avoid this task to wait for the deleted task
					if (_tasks[task].competitor.empty())
						_tasks[task].to_schedule = true;
					else
						_set_to_schedule(task);
					break;
				}
			}
		}
		
		
	}
	
	bool Sched_opt::change_core(std::string task_name, int core)
	{
		for (unsigned int i=0; i<_tasks.size(); ++i)
		{
			if(_tasks[i].name.compare(task_name) == 0)
			{
				PINF("Optimizer: Change core of task %s from %d to %d.", _tasks[i].name.c_str(), _tasks[i].core, core);
				_tasks[i].core = core;
				return true;
			}
		}
		PINF("Optimizer: Requested task for changing core (%s) was not found in task lisk of optimizer.", task_name.c_str());
		return false;
	}
	
	
	bool Sched_opt::scheduling_allowed(std::string task_name)
	{
		// This function looks up the to_schedule value of the requested task.
		// It should be called by Taskloader before starting a task (some where in _session_component::start()).
		// It referes to the following global variables:
		//	_tasks -> name, to_schedule
		
		
		// look in _tasks for task_name
		for (unsigned int task=0; task<_tasks.size(); ++task)
		{
			if(!_tasks[task].name.compare(task_name))
			{
				// task was found
				PINF("Optimizer: Query scheduling allowance for task %s: %d", task_name.c_str(), _tasks[task].to_schedule);
				return _tasks[task].to_schedule;
			}
		}
		
		PINF("Optimizer: Requested task for scheduling allowance (%s) was not found in task lisk of optimizer.", task_name.c_str());
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
			for (unsigned int i=0; i<_tasks.size(); ++i)
			{
				
				// if it's time to see what happend, ...
				if (current_time >= _tasks[i].start_time + _tasks[i].rq_task.deadline)
				{
					//... query monitor-info about _tasks[i] (was there any deadline miss?)
					_job_finished(i, current_time);
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
		// Although it sets the start_time if there was a job.
		// It referes to the following global variables:
		//	_tasks -> name, start_time, rq_task
		//	threads -> thread_name, start_time, foc_id
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
			if( !_tasks[task_nr].name.compare(threads[j].thread_name.string()))
			{
				// matching task found -> check if this thread is a new job
				if(threads[j].start_time >= _tasks[task_nr].start_time)
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
					{
						PINF("Optimizer: Task %s has a new job with foc_id %d.", _threads[j].thread_name.string(), _threads[j].foc_id);
						_tasks[task].newest_job.foc_id = _threads[j].foc_id;
					}
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
		
		
		// determine if the recent job of task _tasks[task_nr] started its execution and react correspondingly (call _task_not_executed(...) or _task_executed(...))
		bool job_executed = false;
		
		switch (new_threads_nr.size())
		{
			case 0:
			{
				// there are no new tasks => job_executed remains false
				PINF("Optimizer: No new job for task %s", _tasks[task_nr].name.c_str());
				break;
			}
			case 1:
			{
				// there is only one new thread with threads[j].start_time >= _tasks[task_nr].start_time
				job_executed = true;
				
				bool deadline_time_reached = (current_time >= threads[new_threads_nr[0]].start_time + _tasks[task_nr].rq_task.deadline);
				
				if (deadline_time_reached) // the job should have been executed
				{
					// determine if the job had a deadline miss or correct execution and set the to_schedules values
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
					if ( (most_recent_thread < 0) || (threads[new_threads_nr[i]].start_time > threads[most_recent_thread].start_time) )
						most_recent_thread = new_threads_nr[i];
					else if ( ((second_recent_thread < 0) || (threads[new_threads_nr[i]].start_time > threads[second_recent_thread].start_time)) )
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
				bool recent_deadline_time_reached = (current_time >= threads[most_recent_thread].start_time + _tasks[task_nr].rq_task.deadline);
					
				// change value of _tasks[task_nr], react to deadline-misses and set to_schedule
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
					
					_task_executed(task_nr, i, consider_this_thread);
					
					if (consider_this_thread)
					{
						// set the tasks execution time to the one of this job
						_tasks[task_nr].execution_time = _threads[i].execution_time.value;
						PINF("Optimizer: Set exec_time for task %s", _tasks[task_nr].name.c_str());
					}
				}
				
				// set start_time for next iteration
				_set_start_time(task_nr, most_recent_thread, recent_deadline_time_reached);
			}
		}
		
		
		
		if(!job_executed)
		{
			// this task has no new job in threads array although it would be time to
			
			// if this task already started ...
			if(_tasks[task_nr].start_time > 0)
			{
				// ... determine why it didn't start
				PINF("Optimizer: task %s had already had some tasks before, but no new one.", _tasks[task_nr].name.c_str());
				_task_not_executed(task_nr);
			}
			// else: the task did not start until now -> query again later...
		}
		
	}
	
	
	void Sched_opt::_set_start_time(unsigned int task_nr, unsigned int thread_nr, bool deadline_time_reached)
	{
		// This function sets the start_time of the given task
		// It referes to the following global variables:
		//	_tasks -> start_time, rq_task
		//	threads -> start_time
		
		_tasks[task_nr].start_time = threads[thread_nr].start_time;
		if (deadline_time_reached)
			_tasks[task_nr].start_time += _tasks[task_nr].rq_task.inter_arrival;
	}
	
	void Sched_opt::_task_executed(unsigned int task_nr, unsigned int thread_nr, bool set_to_schedules)
	{
		// This function handles the situation, when a job of a task should have been executed and its deadline time has already reched.
		// Depending on the exit_time of the thread, the tasks value is in-/decreased and the deadline miss is handled
		// It referes to the following global variables:
		//	_tasks -> value, rq_task, competitor (name), core
		//	threads -> start_time, exit_time
	
		unsigned int core = _tasks[task_nr].core;
		
		
		// determine if there was an soft-exit before reaching the deadline time
		if((_threads[thread_nr].exit_time > 0) && (_threads[thread_nr].exit_time <= _threads[thread_nr].start_time + _tasks[task_nr].deadline))
		{
			// job was executed before reaching its deadline
			
			// reduce value
			if(_tasks[task_nr].value[core] > 0)
				_tasks[task_nr].value[core] --;
		}
		else
		{
			// job reached its deadline before finishing its execution (= deadline miss)
			
			// increase value and indicate an overload at this core
			_tasks[task_nr].value[core] ++;
			_tasks[task_nr].overload[core] = true;
			
			// find causation task
			int cause_task_nr = _get_cause_task(task_nr, thread_nr);
			if(cause_task_nr < 0)
				PWRN("Optimizer: The task %s reached its deadline although there is no cause thread in monitoring data.", _tasks[task_nr].name.c_str());
			// check, if causation task is already in list of competitors
			bool cause_already_at_competitors = false;
			for(unsigned int i = 0; i<_tasks[task_nr].competitor.size(); ++i)
			{
				if (_tasks[task_nr].competitor[i] == (unsigned int)cause_task_nr)
					cause_already_at_competitors = true;
			}
			if (cause_already_at_competitors)
			{
				PWRN("Optimizer: The Task %s is already in competitors list of task %s, but %s had a deadline miss because of it.", _tasks[cause_task_nr].name.c_str(), _tasks[task_nr].name.c_str(), _tasks[task_nr].name.c_str());
				
			}
			
			// add causation task to competitor list
			_tasks[task_nr].competitor.push_back(cause_task_nr);
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
	
	
	void Sched_opt::_task_not_executed(unsigned int task_nr)
	{
		// This function handles the situation, when the task has elapsed its inter_arrival time, but the job was not executed
		// It referes to the following global variables:
		//	_tasks
		
		
		// should this task be executed?
		if(_tasks[task_nr].to_schedule)
		{
			// The task should be executed but didn't start.
			
			// if it had an hard exit and is now in RIP list, ...
			if (_query_rip_list(task_nr))
			{
				//... delete it from _tasks
				_remove_task(task_nr);
			}
			//else: the task may just finished its last job
		}
		else
		{
			// this task should not be executed an wasn't -> increase value ...
			_tasks[task_nr].value[_tasks[task_nr].core] ++;
		
			// ... and update the to_schedule flags
			_set_to_schedule(task_nr);
		}
	}
	
	void Sched_opt::_set_to_schedule(unsigned int task_nr)
	{	
		// This function sets the to_schedule of the thask with max value/max utilization to true.
		// It referes to the following global variables:
		//	_tasks -> competitor, core, value, to_schedule, utilization
		
		if(!_tasks[task_nr].competitor.empty())
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
					PDBG("The optimization goal 'fairness' is used.");
					
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
					PDBG("The optimization goal 'utilization' is used.");
					
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
		
	}
	
	int Sched_opt::_get_cause_task(unsigned int task_nr, unsigned int thread_nr)
	{
		// This function queries the monitoring objects (threads) to find the thread which caused the deadline miss for thread with thread_nr
		// It referes to the following global variables:
		//	threads
		//	_tasks
		
		int cause_thread_nr = -1;
		
		// time interval in which the threads exit time hast to be in: exit_time shall be after threads start_time and before threads deadline
		unsigned long long thread_start = threads[thread_nr].start_time;
		unsigned long long thread_deadline = threads[thread_nr].start_time + _tasks[task_nr].rq_task.deadline;
		
		// query monitoring objects to find a matching thread
		bool threads_array_ended = false;
		
		// loop through threads array
		for(unsigned int i=0; i<100; ++i)
		{
			// find thread which exit-time is the most recent the interval of [thread_start, thread_deadline], so it is the causation thread
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
					// Reached end of monitoring list without finding a matching thread to _tasks[task_nr]
					PINF("No entry found in monitoring list for task with name %s", threads[i].thread_name.string());
					break;
				}
				else
				{
					// this is the first element of the array.
					// the next time, when id==0, the end of threads-array has reached
					threads_array_ended = true;
				}
			}
		}
		// end of threads loop
		
		if(cause_thread_nr >= 0)
		{
			// look for the task in _tasks list which is matching to the cause thread
			for (unsigned int i=0; i<_tasks.size(); ++i)
			{
				if(!_tasks[i].name.compare(threads[cause_thread_nr].thread_name.string()))
					return i;
			}
		}
		
		return -1;
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
