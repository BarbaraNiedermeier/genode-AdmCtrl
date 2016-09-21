/*
 * \brief  Task_allocator header
 * \author Paul Nieleck
 * \date   2016/09/07
 *
 * The Task_allocator is responsible for
 * allocating newly arriving tasks to a
 * core, according to the class and scheduling
 * type of the task.
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
			static void allocate_task(Sched_controller*, Rq_manager::Rq_task*);

	};
//
//	static class Task_allocator_lo : Task_allocator
//	{
//
//		private:
//			int get_available_cores(Pcore*);
//			int get_inactive_cores(Pcore*);
//
//		public:
//			int allocate_task(Rq_manager::Rq_task, Pcore*);
//
//	};
//
//	static class Task_allocator_hi : Task_allocator
//	{
//
//		public:
//			int allocate_task(Rq_manager::Rq_task, Pcore) {return 1;};
//
//	};

}

#endif /* _INCLUDE__SCHED_CONTROLLER__TASK_ALLOCATOR_H_ */