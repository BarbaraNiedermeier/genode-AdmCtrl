/*
 * \brief  testing monitor to call the sched_controller
 * \author Paul Nieleck
 * \date   2016/08/03
 *
 */

#include <base/env.h>
#include <base/printf.h>
#include <sched_controller_session/client.h>
#include <sched_controller_session/connection.h>

#include <timer_session/connection.h>

using namespace Genode;

int main(void)
{

	Sched_controller::Connection sc;

	Timer::Connection timer;

	while(1) {
		sc.get_init_status();
		timer.msleep(2000);
	}

	return 0;
};
