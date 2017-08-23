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

namespace Sched_controller {

	
	enum Optimization_goal {
		NONE,
		FAIRNESS,
		UTILIZATION
	};


	struct Optimization_task
	{
		// general task attributes
		//char			name[24]; // This is used to identify the task
		std::string		name; // This is used to identify the task
		
		Rq_task::Rq_task	rq_task; // needed for update of _rqs and to access inter_arrival and deadline
		
		
		// dynamische Info
		int			core;
		unsigned long long 	start_time;
		bool			to_schedule;
		
		// Pointer zu task
		Rq_task::Rq_task**	task_ptr;
		
		std::vector<unsigned int> competitor;
		
		// Attributes for fairness optimization
		// this is needed for every core
		
		bool* 			overload;
		unsigned int*		value;
		
		
		bool* 			overload;
		unsigned int*		value;
		
		
		
	};	
	
	class Sched_opt {
		
		private:
			Mon_manager::Connection *_mon_manager;
			Mon_manager::Monitoring_object* threads;
			int* rqs;
			Genode::Dataspace_capability _mon_ds_cap;
			Genode::Dataspace_capability _rq_ds_cap;
			Genode::Dataspace_capability _sync_ds_cap;
			
			Optimization_goal _opt_goal;
			std::vector<Optimization_task> _tasks;
			Rq_buffer<Rq_task::Rq_task> *_rqs;
			int num_cores;
			bool* overload_at_core;
			
			Timer::Connection timer;
			int query_intervall;
			
			
			// Attributes needed for fairness optimization
			int accept; // Acceptance niveau for fairness optimization
			
			
			void _job_finished(int task_nr, unsigned long long current_time);
			
			void _task_executed(unsigned int task_nr, unsigned int thread_nr, bool set_to_schedules);
			void _task_not_executed(unsigned int task_nr);
			
			void _remove_task(unsigned int task_nr);
			void _set_start_time(unsigned int task_nr, unsigned int thread_nr, bool deadline_time_reached);
			void _set_to_schedule(unsigned int task_nr);
			
			
			// Functions needed for fairness optimization
			void _optimize_fairness();
			int _get_cause_task(unsigned int task_nr, unsigned int thread_nr);
			
			
		public:
			void set_goal(Genode::Ram_dataspace_capability);
			int add_task(int core, Rq_task::Rq_task task); // add task to task array
			bool change_core(std::string task_name, int core);
			void run_job();
			
			void start_optimizing();
			
			Sched_opt(int, Rq_buffer<Rq_task::Rq_task>*, Mon_manager::Connection*, Mon_manager::Monitoring_object*, Genode::Dataspace_capability, int*, Genode::Dataspace_capability);
			
			
			//Sched_opt(int, Rq_buffer<Rq_task::Rq_task>*, Mon_manager::Connection*, Mon_manager::Monitoring_object*, Genode::Dataspace_capability, int*, Genode::Dataspace_capability, Genode::Dataspace_capability);
			~Sched_opt();

	};
	

}

#endif /* _INCLUDE__SCHED_CONTROLLER__SCHED_OPT_H_ */
