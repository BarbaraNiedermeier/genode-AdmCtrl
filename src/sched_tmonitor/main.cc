/*
 * \brief  testing monitor to call the sched_controller
 * \author Paul Nieleck
 * \date   2016/08/03
 *
 */

#include <base/env.h>
#include <base/printf.h>
#include <sched_controller_session/sched_controller_session.h>
#include <sched_tmonitor/sched_tmonitor.h>

#include <timer_session/connection.h>

using namespace Genode;

int main(void)
{
	Capability<Sched_controller::Session> sc_cap = env()->parent()->session<Sched_controller::Session>("foo, ram_quota=4K");

	Sched_tmonitor::Session_client sc(sc_cap);

	Timer::Connection timer;

	while(1) {
		sc.get_status();
		timer.msleep(2000);
	}

	return 0;
};
