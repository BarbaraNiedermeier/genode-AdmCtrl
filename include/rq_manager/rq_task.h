/*
 * \brief  Task-description for Tasks in the run queue
 * \author Paul Nieleck
 * \date   2016/08/15
 *
 */

#ifndef _INCLUDE__RQ_MANAGER__RQ_TASK_H_
#define _INCLUDE__RQ_MANAGER__RQ_TASK_H_

namespace Rq_manager
{

	struct Rq_task
	{

			int task_id;
			int wcet;
			bool valid;

	};
}

#endif /* _INCLUDE__RQ_MANAGER__RQ_TASK_H_ */
