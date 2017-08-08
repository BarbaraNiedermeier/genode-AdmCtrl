/*
 * \brief  superclass of different scheduling optimization algorithms.
 * \author Barbara Niedermeier
 * \date   2017/07/25
 */
 
#ifndef _INCLUDE__SCHED_CONTROLLER__SCHED_OPT_H_
#define _INCLUDE__SCHED_CONTROLLER__SCHED_OPT_H_


#include <ram_session/ram_session.h>
#include <os/attached_ram_dataspace.h>
#include "rq_task/rq_task.h"

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
		char		name[24]; // This is used to identify the task
		
		unsigned long long deadline; //		|
		//					  => bestimme Zeitpunkt, wann Task spätestens fertig sein müsste
		unsigned long long inter_arrival; //	|
		
		// Pointer zu task
		Rq_task::Rq_task** task_ptr;
		
		int		value;
		
		
	};	
	
	class Sched_opt {
		
		private:
			Optimization_goal _opt_goal;
			//Optimization_task *_tasks; // Task array, oder ist list besser?
			std::vector<Optimization_task> _tasks;
			Mon_manager::Connection *_mon_manager;
			Genode::Dataspace_capability _mon_ds_cap;
			
			// Attributes needed for fairness optimization
			int _accept; // Acceptance niveau for fairness optimization
			int _min_task; // Array number of task with min value
			int _max_task; // Array number of task with max value 
			
			
			
			void _optimize_fairness();
			void _query_monitor();
			bool _update_value(int, bool);
			
			void _deactivate_task();
			
		public:
			void set_goal(Genode::Ram_dataspace_capability);
			void add_task(int core, Rq_task::Rq_task task); // add task to task array
			
			void start_optimizing();
			
			Sched_opt(Mon_manager::Connection*, Genode::Dataspace_capability);
			~Sched_opt();

	};
	

}

#endif /* _INCLUDE__SCHED_CONTROLLER__SCHED_OPT_H_ */
