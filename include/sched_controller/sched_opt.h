/*
 * \brief  superclass of different scheduling optimization algorithms.
 * \author Barbara Niedermeier
 * \date   2017/07/25
 */
 
#ifndef _INCLUDE__SCHED_CONTROLLER__SCHED_OPT_H_
#define _INCLUDE__SCHED_CONTROLLER__SCHED_OPT_H_


#include <ram_session/ram_session.h>
#include <os/attached_ram_dataspace.h>
#include "sched_controller/rq_buffer.h"
#include "rq_task/rq_task.h"

#include <timer_session/connection.h>
#include "mon_manager/mon_manager.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace Sched_controller {

	
	enum Optimization_goal {
		NONE,
		FAIRNESS,
		UTILIZATION
	};
	enum Cause_of_death {
		KILLED, // task was killed by user (hard exit)
		FINISHED // task finished its last job (soft exit)
	};
	// this struct is used to represent the tasks which are no Optimization_tasks any more (finished execution <-> killed)
	struct Ended_task
	{
		std::string		name;
		unsigned int		last_foc_id; // foc_id of last job
		Cause_of_death		cause_of_death;
	};
	
	// this struct is used to determine the job corresponding to the thread at the rip list
	struct Newest_job
	{
		unsigned int		foc_id;
		unsigned long long 	arrival_time;
		unsigned int		core;
		bool			dispatched;
		
	};

	//class Related_tasks
	struct Related_tasks
	{
		unsigned int		max_value;
		std::unordered_set<std::string> tasks;
	};
	
	struct Optimization_task
	{
		// static task attributes
		std::string		name; // This is also used to identify the task
		
		unsigned long long	inter_arrival;
		unsigned long long	deadline;
		
		// dynamic task attributes
		unsigned int		core;
		unsigned long long 	arrival_time; // this is the jobs earliest possible start time
		bool			to_schedule;
		bool			last_job_started; // used to indicate thelast execution of a job belonging to this task
		std::vector<std::string> competitor;
		unsigned int 		id_related;
		Newest_job		newest_job;// used for rip list
		
		
		// attributes for optimization
		unsigned int*		value; // value is needed for every core
		double			utilization;
		
		
	};	
	
	class Sched_opt {
		
		private:
			Mon_manager::Connection*				_mon_manager;
			Mon_manager::Monitoring_object*				_threads;
			Genode::Dataspace_capability				_mon_ds_cap;
			
			// Attributes needed for analyzing rip list correctly
			long long unsigned*					rip;
			Genode::Dataspace_capability				_dead_ds_cap;
			
			Optimization_goal					_opt_goal;
			std::unordered_map<std::string, Optimization_task>	_tasks;
			std::unordered_map<std::string, Ended_task>		_ended_tasks;
			std::unordered_map<unsigned int, Related_tasks>		_related_tasks;
			
			int							num_cores;
			bool*							overload_at_core;
			
			Timer::Connection					timer;
			int							query_intervall;
			
			
			
			void _query_monitor(std::string task_str, unsigned long long current_time);
			void _task_executed(std::string task_str, unsigned int thread_nr, bool set_to_schedules);
			void _task_not_executed(std::string task_str);
			void _deadline_reached(std::string task_str);
			void _remove_task(std::string task_str, unsigned int foc_id, Cause_of_death cause);
			
			// private setter
			void _set_newest_job(std::string task_str, unsigned int thread_nr);
			void _set_arrival_time(std::string task_str, unsigned int thread_nr, bool deadline_time_reached);
			void _set_to_schedule(std::string task_str);
			void _reset_values(std::string task_str);
			
			// private getter
			std::string _get_cause_task(std::string task_str);
			
			
		public:
			void set_goal(Genode::Ram_dataspace_capability);
			void start_optimizing(std::string task_name);
			
			void add_task(unsigned int core, Rq_task::Rq_task task); // add task to task array (info from sched_controller that this task has been enqueued)
			
			// these functions are called by the taskloader
			int scheduling_allowed(std::string task_name);
			void last_job_started(std::string task_name);
			
			
			Sched_opt(int sched_num_cores, Mon_manager::Connection *mon_manager, Mon_manager::Monitoring_object *sched_threads, Genode::Dataspace_capability mon_ds_cap, Genode::Dataspace_capability dead_ds_cap);
			~Sched_opt();

	};
	

}

#endif /* _INCLUDE__SCHED_CONTROLLER__SCHED_OPT_H_ */
