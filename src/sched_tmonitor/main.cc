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
#include <ram_session/connection.h>
#include <cpu_session/connection.h>
#include <trace_session/connection.h>
#include <timer_session/connection.h>

using namespace Genode;

int main(void)
{

	Sched_controller::Connection sc;

	Timer::Connection timer;

	static Genode::Trace::Connection trace(1024*4096, 64*4096, 0); 

	Trace::Subject_id subjects[32]; 
	size_t num_subjects = trace.subjects(subjects, 32);

	while(1) {
		for (size_t i = 0; i < num_subjects; i++) { 
			Trace::CPU_info info = trace.cpu_info(subjects[i]); 
			Trace::RAM_info ram_info = trace.ram_info(subjects[i]); 

			printf("ID:%d %lld prio:%d %s name:%s %d %d", 
			       info.id(), 
			       info.execution_time(), 
			       info.prio(), 
			       info.session_label().string(), 
			       info.thread_name().string(), 
			       ram_info.ram_quota(), 
			       ram_info.ram_used()); 
		}

		sc.get_init_status();
		timer.msleep(2000);

	}

	return 0;
};
