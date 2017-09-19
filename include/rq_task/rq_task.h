/*
 * \brief  Task-description for Tasks in the run queue
 * \author Paul Nieleck
 * \date   2016/08/15
 *
 */

#ifndef _INCLUDE__RQ_TASK__RQ_TASK_H_
#define _INCLUDE__RQ_TASK__RQ_TASK_H_

#include <string>

namespace Rq_task
{

	enum class Task_class { hi, lo };

	enum class Task_strategy { priority, deadline };

	struct Rq_task
	{

			int task_id;
			Task_class task_class;
			Task_strategy task_strategy;
			unsigned long long deadline;
			unsigned long long wcet;
			unsigned long long inter_arrival;
			int prio;
			bool valid;
			char name[24];

	};
}

#endif /* _INCLUDE__RQ_TASK__RQ_TASK_H_ */
