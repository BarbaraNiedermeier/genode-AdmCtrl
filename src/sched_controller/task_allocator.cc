/*
 * \brief  allocate tasks to pcores
 * \author Paul Nieleck
 * \date   2016/09/16
 */

#include <base/printf.h>

#include "sched_controller/task_allocator.h"
#include "rq_manager/rq_task.h"

namespace Sched_controller {

	void Task_allocator::allocate_task(Rq_manager::Rq_task *task)
	{
		PINF("Task allocator got the Task with id: %d", task->task_id);
	}

}