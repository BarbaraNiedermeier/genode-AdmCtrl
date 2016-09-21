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
#include "rq_manager/rq_task.h"

/* for testing */
#include "rq_manager_session/client.h"
#include "rq_manager_session/connection.h"
/* */

namespace Sched_controller {

	void Task_allocator::allocate_task(Sched_controller *sc, Rq_manager::Rq_task *task)
	{
		PINF("Task allocator got the Task with id: %d", task->task_id);

		/* now that we have the task, we need to put it to the right run_queue */
		/* how to find out which rqs are available??? */
		if (task->task_class == Rq_manager::Task_class::hi) {
			PINF("This is a high task and we do nothing!");
			return;
		}

		PINF("This is a lo task, going to allocate it.");

		/* Now we need to see if there exists any run queue that
		 * already supports this kind of task
		 */
		std::vector<Runqueue> rq;
		sc->which_runqueues(&rq, task->task_class, task->task_strategy);

		int num_elements_in_rq = rq.size();
		PINF("So now we know that there are %d runqueus that we can allocate the task to", num_elements_in_rq);

		double util = 1.0;
		int lowest_util_rq = -1;

		if (rq.size() == 0) {
			//check for empty pcore and put the task there.
		} else {
			//check which runque has the lowest utilization and put it there.
			for (auto it = rq.begin(); it != rq.end(); it++) {
				double util_comp = sc->get_utilization((*it).rq_buffer);
				PINF("The utilization of run queue %d is %d and the lowest utilization is currently %d", (*it).rq_buffer, (int) (100 * util_comp), (int) (100 * util));
				if (util_comp < util) {
					lowest_util_rq = (*it).rq_buffer;
					util = util_comp;
					PINF("Setting lowest_util_rq to %d", lowest_util_rq);
				}

			}

			PINF("The Runqueue with the lowest utilization is: %d", lowest_util_rq);
			
			sc->task_to_rq(lowest_util_rq, task);
			//Rq_manager::Connection _rq_manager;
			//int status = -666;
			//status = _rq_manager.enq(lowest_util_rq, *task);
			//PINF("Enqueued status of _rq_manager.enq returned: %d", status);

		}

	}

}
