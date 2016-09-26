/*
 * \brief  Task-description for Tasks in the run queue
 * \author Paul Nieleck
 * \date   2016/08/15
 *
 */

#ifndef _INCLUDE__RQ_TASK__RQ_TASK_H_
#define _INCLUDE__RQ_TASK__RQ_TASK_H_

namespace Rq_task
{

	enum class Task_class { hi, lo };

	enum class Task_strategy { priority, deadline, deadprio };

	struct Rq_task
	{

			int task_id;
			Task_class task_class;
			Task_strategy task_strategy;
			float deadline;
			float wcet;
			float inter_arrival;
			int prio;
			bool valid;

	};
}

#endif /* _INCLUDE__RQ_TASK__RQ_TASK_H_ */
