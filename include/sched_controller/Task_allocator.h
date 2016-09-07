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

namespace Sched_controller {

	static class Task_allocator
	{

		private:
			int check_task_consistency(Rq_task);
			int get_dependent_core(Rq_task, Core*);

		public:
			virtual int enqueue_task(Rq_task, Core*) = 0;

	};

	static class Task_allocator_LO : Task_allocator
	{

		private:
			int get_available_cores(Core*);
			int get_inactive_cores(Core*);

		public:
			int enqueue_task(Rq_task, Core*);

	};

	static class Task_allocator_HI : Task_allocator
	{

		public:
			int enqueue_task(Rq_task, Core) {return 1;};

	};

}

#endif /* _INCLUDE__SCHED_CONTROLLER__TASK_ALLOCATOR_H_ */