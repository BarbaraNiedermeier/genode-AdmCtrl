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
		//public:
			unsigned int		max_value;
			std::unordered_set<std::string> tasks;
			
		/*
			Related_tasks();
		*/
		
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
		bool* 			overload; // use this later to change cores depending on the overload on each core
		

		std::vector<unsigned int> competitor;
		
		// Attributes for fairness optimization
		// this is needed for every core
		
		bool* 			overload;
		unsigned int*		value;
		
		// attributes for fairness optimization
		unsigned int*		value; // value is needed for every core
		
		// attributes for utilization optimization
		double			utilization;
		unsigned int		execution_time;
		
		
	};	
	
	class Sched_opt {
		
		private:
			Mon_manager::Connection *_mon_manager;
			Mon_manager::Monitoring_object* _threads;
			Genode::Dataspace_capability _mon_ds_cap;
			
			// Attributes needed for analyzing rip list correctly
			long long unsigned *rip;
			Genode::Dataspace_capability _dead_ds_cap;
			
			Optimization_goal _opt_goal;
			std::vector<Optimization_task> _tasks;
			int num_cores;
			bool* overload_at_core;
			
			Timer::Connection timer;
			int query_intervall;
			
			
			// Attributes needed for fairness optimization
			int accept; // Acceptance niveau for fairness optimization
			
			
			void _query_monitor(std::string task_str, unsigned long long current_time);
			
			void _task_executed(std::string task_str, unsigned int thread_nr, bool set_to_schedules);
			void _task_not_executed(std::string task_str);
			void _deadline_reached(std::string task_str);
			std::string _get_cause_task(std::string task_str);
			
			void _remove_task(std::string task_str, unsigned int foc_id, Cause_of_death cause);
			void _set_arrival_time(std::string task_str, unsigned int thread_nr, bool deadline_time_reached);
			void _set_to_schedule(std::string task_str);
			
			
			// Function needed to determine task competitors
			std::string _get_cause_task(std::string task_str, unsigned int thread_nr);
			
			
		public:
			void set_goal(Genode::Ram_dataspace_capability);
<<<<<<< b0c003af0f0e95a6617a232e2c577477b2c1a464
			void start_optimizing();
			
			void add_task(unsigned int core, Rq_task::Rq_task task); // add task to task array (info from sched_controller that this task has been enqueued)
			
			// these functions are called by the taskloader
			bool scheduling_allowed(std::string task_name);
			void last_job_started(std::string task_name);
			bool change_core(std::string task_name, unsigned int core);
			
			
			Sched_opt(int sched_num_cores, Mon_manager::Connection *mon_manager, Mon_manager::Monitoring_object *sched_threads, Genode::Dataspace_capability mon_ds_cap, Genode::Dataspace_capability dead_ds_cap);
=======
			
			void add_task(int core, Rq_task::Rq_task task); // add task to task array (info from sched_controller that this task has been enqueued)
			void task_removed(int core, Rq_task::Rq_task **task_ptr); // info from sched_controller that this task has been dequeued
			
			bool change_core(int core, std::string task_name);
			bool scheduling_allowed(); // add task as call parameter
			
			void start_optimizing();
			
			Sched_opt(int sched_num_cores, Mon_manager::Connection *mon_manager, Mon_manager::Monitoring_object *sched_threads, Genode::Dataspace_capability mon_ds_cap);
>>>>>>> Remove changes to runqueue
			~Sched_opt();

	};
	

}

#endif /* _INCLUDE__SCHED_CONTROLLER__SCHED_OPT_H_ */
