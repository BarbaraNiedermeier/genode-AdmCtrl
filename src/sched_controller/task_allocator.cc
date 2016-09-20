/*
 * \brief  allocate tasks to pcores
 * \author Paul Nieleck
 * \date   2016/09/16
 */

#include <forward_list>
#include <vector>
#include <base/printf.h>

#include "sched_controller/task_allocator.h"
#include "rq_manager/rq_task.h"

namespace Sched_controller {

	void Task_allocator::allocate_task(Rq_manager::Rq_task *task)
	{
		PINF("Task allocator got the Task with id: %d", task->task_id);

		/* now that we have the task, we need to put it to the right run_queue */
		/* how to find out which rqs are available??? */
		if (task->task_class == Rq_manager::Task_class::hi) {
			PINF("This is a high task and we do nothing!");
		} else {

			PINF("This is a lo task, going to allocate it.");

//			std::forward_list<Pcore*> _cores = Pcore::get_pcores();
//
//			/* we go through all cores to check if we can schedule the task on one
//			 * of them.
//			 */
//			for (auto it = _cores.begin(); it != _cores.end(); ++it) {
//				PINF("Task allocator is aware of pcore %d", (*it)->get_id());
//				
//				if ((*it)->get_class() != task->task_class) {
//
//					PINF("This pcore contains the wrong task class");
//					continue;
//
//				} 
//
//				PINF("This pcore contains rqs of task class %d", (*it)->get_class());
//				std::vector<int> pcore_rqs = (*it)->get_rqs();
//				
//				/* If the pcore currently has no run queue attached, we can use it,
//				 * if there is no other run queue/pcore that can host the task.
//				 */
//				if (pcore_rqs.empty()) {
//					continue; //currently we just skip this and go on with non-empty pcores.
//				}
//
//				/* On all other run queues we have to check if they are using the
//				 * scheduling algorithm strategy that the task is intended for.
//				 */

		}

	}

}
