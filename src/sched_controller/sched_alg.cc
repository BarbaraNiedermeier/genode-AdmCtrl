/*
 * \brief  
 * \author Paul Nieleck
 * \date   2016/09/22
 */

#include <base/printf.h>
#include "rq_task/rq_task.h"
#include "sched_controller/sched_alg.h"

namespace Sched_controller {

	int Sched_alg::_compute_repsonse_time(Rq_task::Rq_task *new_task, Rq_buffer<Rq_task::Rq_task> *rq_buf, int num_elements, float deadline)
	{
		_curr_task = rq_buf->get_first_element();
		while (true)
		{
			_response_time = 0;
			_response_time += (_response_time_old / new_task->inter_arrival) * new_task->wcet;
			for (int i=0; i<num_elements; ++i)
			{
				_response_time += (_response_time_old / _curr_task->inter_arrival) * _curr_task->wcet;
				_curr_task++;
			}

			/*Since the response_time is increasing with each iteration, it has to be always
			 * smaller then the deadline
			 */
			if (_response_time > deadline)
			{
				//Task-Set is NOT schedulable
				PWRN("Task-Set is NOT schedulable!");
				return 0;
			}
			if (_response_time_old >= _response_time)
			{
				if (_response_time <= deadline)
				{
					//Task-Set is schedulable
					PDBG("Task-Set is schedulable!");
					return 1;
				}
			}
		} // while(true)
	}

	int Sched_alg::RTA(Rq_task::Rq_task *new_task, Rq_buffer<Rq_task::Rq_task> *rq_buf)
	{
		int num_elements = rq_buf->get_num_elements();
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
		 * the new task. Therefore the response time has to be computed
		 * for the new task and all tasks having a smaller priority then
		 * the new task and for the new task. The tasks in the rq_buffer
		 * are assumed to be sorted by priorities.
		 */

		_curr_task = rq_buf->get_first_element();
		if (new_task->prio < rq_buf->get_last_element()->prio)
		{
			PDBG("New task has lower prio then all other tasks");
			if (_compute_repsonse_time(new_task, rq_buf, num_elements, new_task->deadline) <= 0)
			{
				//Task Set not schedulable
				PDBG("Task set is not schedulable!");
				return 0;
			}

		}
		else
		{
			/*
			 * Compute response time for all tasks with priority smaller
			 * then the priority of the new task
			 */
			PDBG("New task has higher prio then lowest existing task");
			for (int i=0; i<num_elements; ++i)
			{
				_response_time_old = _curr_task->wcet;
				if(_curr_task->prio < new_task->prio){
					if (_compute_repsonse_time(new_task, rq_buf, i+1, _curr_task->deadline) <= 0)
					{
						//Task Set not schedulable
						return 0;
					}
				}
				++_curr_task;
			}
		}
		PDBG("All Task-Sets passed the RTA Algorithm -> Task-Set schedulable!");
		return 1;

	}//RTA


	int Sched_alg::fp_sufficient_test(Rq_task::Rq_task *new_task, Rq_buffer<Rq_task::Rq_task> *rq_buf)
	{
		int num_elements = rq_buf->get_num_elements();
		if (num_elements == 0)
		{
			//Rq is empty --> Task set is schedulable
			PDBG("Rq is empty, Task set is schedulable!");
			return 1;
		}

		double R_ub, sum_utilisation;
		_curr_task = rq_buf->get_first_element();

		for (int i=0; i<num_elements; ++i)
		{
			R_ub = _curr_task->wcet;
			sum_utilisation = 0.0;
			//for each task with smaller prio
			for (int j=i; j>0; --j)
			{
				sum_utilisation += (_curr_task - j)->wcet / (_curr_task - j)->inter_arrival;
				R_ub +=  ((_curr_task - j)->wcet * (1 - (_curr_task - j)->wcet / (_curr_task - j)->inter_arrival));
			}

			R_ub /= (1 - sum_utilisation);
			if (R_ub > _curr_task->wcet)
			{
				//Deadline hit for task i
				PWRN("Deadline hit for task %d, Task set might be not schedulable! Maybe try an exact test.", _curr_task->task_id);
			}
		}

		return 1;
	}
}