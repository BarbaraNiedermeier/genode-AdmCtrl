/*
 * \brief  Monitor the scheduled data
 * \author Paul Nieleck
 * \date   2016/09/23
 */

#include <base/env.h>
#include <base/printf.h>
#include <sched_controller_session/client.h>
#include <sched_controller_session/connection.h>
#include <ram_session/connection.h>
#include <cpu_session/connection.h>
#include <trace_session/connection.h>
#include <timer_session/connection.h>

#include <sched_controller/monitor.h>

namespace Sched_controller
{

	void Monitor::monitor_data() {

		Timer::Connection timer;

		static Genode::Trace::Connection trace(1024*4096, 64*4096, 0); 

		Genode::Trace::Subject_id subjects[32]; 
		size_t num_subjects = trace.subjects(subjects, 32);

		while(1) {
			for (size_t i = 0; i < num_subjects; i++) { 
				Genode::Trace::CPU_info info = trace.cpu_info(subjects[i]); 
				Genode::Trace::RAM_info ram_info = trace.ram_info(subjects[i]); 

				Genode::printf("ID:%d name:%s exec_time:%lld prio:%d RAM:%d\n", 
					   subjects[i].id, 
					   info.thread_name().string(), 
					   info.execution_time().value, 
					   info.prio(), 
					   ram_info.ram_used()); 
			}

			timer.msleep(2000);

		}


	}

}