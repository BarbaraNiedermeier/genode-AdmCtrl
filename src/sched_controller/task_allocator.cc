/*
 * \brief  allocate tasks to pcores
 * \author Paul Nieleck
 * \date   2016/09/16
 */

#include <forward_list>
#include <vector>
#include <base/printf.h>

#include "sched_controller/task_allocator.h"
#include "sched_controller/sched_controller.h"
#include "rq_task/rq_task.h"

namespace Sched_controller {

	/**
	 * Allocate the given task to a suitable run queue of the calling Sched_controller
	 *
	 * \param *sc: The Sched_controller that is calling this function, i.e. "this"
	 * \param *task: Pointer to the task that should be initially allocated to a run queue
	 */
	void Task_allocator::allocate_task(Sched_controller *sc, Rq_task::Rq_task *task)
	{
		PINF("Task allocator got the Task with id: %d prio: %d\n", task->task_id, task->prio);

		/* First we need to check for the Task_class of the task */
		if (task->task_class == Rq_task::Task_class::hi) {
			PINF("This is a high task => can currently not be allocated!");
			return;
		}

		/* 
		 * Now we need to see if there exists any run queue that
		 * already supports this kind of task
		 */
		std::vector<Runqueue> rqs;
		sc->which_runqueues(&rqs, task->task_class, task->task_strategy);

		if (rqs.size() == 0) {
			/* check for empty pcore and put the task there. */
			std::forward_list<Pcore*> empty_pcore = sc->get_unused_cores();

			if (empty_pcore.empty() == true) {
				PINF("No empty pcore available");
				/* TODO: Now we need to reject the task somehow. */
			} else {
				PINF("There are empty pcores on the system.");
				/* 
				 * TODO: Need to create new runqueue, associate it with the pcore that consumes the
				 *       lowest energy and enqueue the task into that run queue
				 */
 
			}

		} else {
			/* 
			 * Check which runque has the lowest utilization and put it there.
			 * TODO: If the RQ is a priority based rq, we can't check for the
			 *       utilization by means of execution times. Instead we should
			 *       probably go for the load of the CPU or simply put it some-
			 *       where.
			 */
			int lowest_util_rq = 0;
			int util = 100;
			for(int i=0;i<sc->get_num_cores();i++)
			{
				int new_util=sc->get_utilization(i);
				Genode::printf("util: %d\n",new_util);
				if(new_util<util)
				{
					util=new_util;
					lowest_util_rq=i;
				}
			}
			
			
			

			for (auto it = rqs.begin(); it != rqs.end(); it++) {
				int util_comp = sc->get_utilization((*it).rq_buffer);
				//PINF("The utilization of run queue %d is %d and the lowest utilization is currently %d", (*it).rq_buffer, (int) (100 * util_comp), (int) (100 * util));
				if (util_comp < util) {
					lowest_util_rq = (*it).rq_buffer;
					util = util_comp;
					//PINF("Setting lowest_util_rq to %d", lowest_util_rq);
				}

			}

			PINF("The Runqueue with the lowest utilization is: %d", lowest_util_rq);
			
			sc->task_to_rq(lowest_util_rq, task);

		}

	}

}
