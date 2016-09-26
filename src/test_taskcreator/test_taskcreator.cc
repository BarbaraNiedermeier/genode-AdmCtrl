/*
 * \brief  Creates rq_task objects for the sched_controller
 * \author Paul Nieleck
 * \date   2016/09/15
 */

#include <random>
#include <timer_session/connection.h>
#include <base/printf.h>
#include <base/sleep.h>

#include "rq_task/rq_task.h"
#include "sched_controller_session/connection.h"

int main()
{

	static Timer::Connection _timer;
	static Sched_controller::Connection _schedcontrlr;

	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution(1000,9999);

	while (true) {

		int rand = distribution(generator);

		Rq_task::Rq_task task;

		task.task_id = rand;
		if (rand % 2 == 0) {
			task.task_class = Rq_task::Task_class::hi;
		} else {
			task.task_class = Rq_task::Task_class::lo;
		}
		if (rand % 3 == 0) {
			task.task_strategy = Rq_task::Task_strategy::priority;
		} else if (rand % 3 == 1) {
			task.task_strategy = Rq_task::Task_strategy::deadline;
		} else {
			task.task_strategy = Rq_task::Task_strategy::deadprio;
		}
		task.deadline = (float) rand;
		task.wcet = (float) rand;
		task.inter_arrival = (int) rand / 1000;
		task.prio = rand % 128;
		task.valid = true;

		_timer.msleep(rand);

		PINF("Task created with id: %d", task.task_id);

		_schedcontrlr.new_task(task);

	}

	return 0;

}