/*
 * \brief  
 * \author Paul Nieleck
 * \date   2016/09/22
 */

#include <base/printf.h>
#include "rq_task/rq_task.h"
#include "sched_controller/sched_alg.h"

namespace Sched_controller {

	int RTA(int core, Rq_task::Rq_task new_task)
	{
		int num_elements = _rqs[core].get_num_elements();
		PDBG("There are currently %d elements in the rq_buffer", num_elements);

		/*
		 * Assuming that each task for schedulable if it is alone,
		 * the task is acceptet if the rq_buffer is empty
		 */
		if (num_elements == 0)
		{
			return 1;
		}

		/*
		 * RTA-Algorithm
		 * We assume that the existing Task-Set is schedulable without
		 * the new task. The response time of the task with the smallest
		 * priority has to be computed.
		 * If the priority of the new task is smaller then all existing tasks,
		 * the Respone-Time of the new task is computed, otherwise the last
		 * element of the rq_buffer is taken (rq_buffer assumed to be sorted).
		 */

		Rq_task::Rq_task *curr_task = _rqs[core].get_first_element();
		int response_time_old, response_time = 0;
		float deadline;
		if (new_task->prio < _rqs[core].get_last_element()->prio)
		{
			response_time_old = new_task->wcet;
			deadline = new_task->deadline;
			PDBG("New task has lower prio then all other tasks");
		}
		else
		{
			response_time_old = _rqs[core].get_last_element()->wcet);
			deadline = _rqs[core].get_tail_ptr()->deadline;
			PDBG("New task has higher prio then lowest existing task");
		}
		while (true){
			response_time = 0;
			respone_time += (response_time_old / new_task->period) * new_task->wcet;
			for (int i=0; i<num_elements; ++i)
			{
				response_time += (response_time_old / curr_task->period) * curr_task->wcet;
				curr->task = _rqs[core].get_next_element(curr_task);
			}

			/*Since the response_time is increasing with each iteration, it has to be always
			 * smaller then the deadline
			 */
			if (respone_time > deadline)
			{
				//Task-Set is NOT schedulable
				PWRN("Task-Set is NOT schedulable!");
				return 0;
			}
			if (response_time_old >= response_time)
			{
				if (response_time <= deadline)
				{
					//Task-Set is schedulable
					PDBG("Task-Set is schedulable!");
					return 1;
				}
			}
		} // while(true)
	}//RTA

}