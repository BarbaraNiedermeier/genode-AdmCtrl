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

namespace Sched_controller {

	
	enum Optimization_goal {
		NONE,
		FAIRNESS,
		UTILIZATION
	};
	
	// this struct is used to determine the task corresponding to the job at the rip list
	struct Newest_job
	{
		unsigned int		foc_id;
		unsigned long long 	start_time;
		
	};

	struct Optimization_task
	{
		// general task attributes
		std::string		name; // This is used to identify the task
		
		Rq_task::Rq_task	rq_task; // needed for update of _rqs and to access inter_arrival and deadline
		
		
		// dynamische Info
		int			core;
		unsigned long long 	start_time;
		
		bool			to_schedule;
		
		std::vector<std::string> competitor;
		// used for rip list
		Newest_job		newest_job;
		
		// Attributes for fairness optimization
		// this is needed for every core
		
		unsigned int*		value;
		
		double			utilization;
		unsigned int		execution_time;
		
		// use this later to change cores depending on the overload on each core
		bool* 			overload;
		
		
		
	};	
	
	class Sched_opt {
		
		private:
			Mon_manager::Connection *_mon_manager;
			Mon_manager::Monitoring_object* threads;
			int* rqs;
			Genode::Dataspace_capability _mon_ds_cap;
			Genode::Dataspace_capability _rq_ds_cap;
			Genode::Dataspace_capability _sync_ds_cap;
			
			// Attributes needed for analyzing rip list correctly
			Genode::Dataspace_capability _dead_ds_cap;
			
			Optimization_goal _opt_goal;
			std::vector<Optimization_task> _old_task;
			std::unordered_map<std::string, Optimization_task> _tasks;
			int num_cores;
			bool* overload_at_core;
			
			Timer::Connection timer;
			int query_intervall;
			
			
			// Attributes needed for fairness optimization
			int accept; // Acceptance niveau for fairness optimization
			
			
			void _query_monitor(std::string task_str, unsigned long long current_time);
			
			void _task_executed(std::string task_str, unsigned int thread_nr, bool set_to_schedules);
			void _task_not_executed(std::string task_str);
			
			void _remove_task(std::string task_str);
			void _set_start_time(std::string task_str, unsigned int thread_nr, bool deadline_time_reached);
			void _set_to_schedule(std::string task_str);
			bool _query_rip_list(std::string task_str);
			
			
			// Function needed to determine task competitors
			std::string _get_cause_task(std::string task_str, unsigned int thread_nr);
			
			
		public:
			void set_goal(Genode::Ram_dataspace_capability);
			
			void add_task(int core, Rq_task::Rq_task task); // add task to task array (info from sched_controller that this task has been enqueued)
			void task_removed(int core, Rq_task::Rq_task **task_ptr); // info from sched_controller that this task has been dequeued
			
			bool change_core(int core, std::string task_name);
			bool scheduling_allowed(std::string task_name); // add task as call parameter
			
			void start_optimizing();
			
			Sched_opt(int sched_num_cores, Mon_manager::Connection *mon_manager, Mon_manager::Monitoring_object *sched_threads, Genode::Dataspace_capability mon_ds_cap, Genode::Dataspace_capability dead_ds_cap);
			~Sched_opt();

	};
	

}

#endif /* _INCLUDE__SCHED_CONTROLLER__SCHED_OPT_H_ */
