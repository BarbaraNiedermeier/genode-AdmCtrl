/*
 * \brief  Task_allocator header
 * \author Paul Nieleck
 * \date   2016/09/07
 *
 * The Task_allocator is responsible for
 * allocating newly arriving tasks to a
 * run queue, according to the class and 
 * scheduling type of the task.
 */

#ifndef _INCLUDE__SCHED_CONTROLLER__TASK_ALLOCATOR_H_
#define _INCLUDE__SCHED_CONTROLLER__TASK_ALLOCATOR_H_

#include "sched_controller/pcore.h"
#include "sched_controller/sched_controller.h"

namespace Sched_controller {

	class Task_allocator
	{

		private:
			//int check_task_consistency(Rq_manager::Rq_task);
			//int get_dependent_core(Rq_manager::Rq_task, Pcore*);

		public:
			//virtual int allocate_task(Rq_manager::Rq_task, Pcore*) = 0;
			static void allocate_task(Sched_controller*, Rq_task::Rq_task*);

	};

/*
 * TODO: Implement the functionality with subclasses instead of
 *       doing everything in the Task_allocator
 */

//	class Task_allocator_lo : Task_allocator
//	{
//
//		private:
//			int get_available_cores(Pcore*);
//			int get_inactive_cores(Pcore*);
//
//		public:
//			static int allocate_task(Rq_manager::Rq_task, Pcore*);
//
//	};
//
//	class Task_allocator_hi : Task_allocator
//	{
//
//		public:
//			static int allocate_task(Rq_manager::Rq_task, Pcore) {return 1;};
//
//	};

}

#endif /* _INCLUDE__SCHED_CONTROLLER__TASK_ALLOCATOR_H_ */