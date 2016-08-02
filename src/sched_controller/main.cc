/*
 * \brief  sched_controller is the user land controller for the kernel schedulers
 * \author Paul Nieleck
 * \date   2016/08/02
 *
 */

#include <base/printf.h>
#include <sched_controller_session/sched_controller_session.h>
#include <base/rpc_server.h>

namespace Sched_controller {

	struct Session_component : Genode::Rpc_object<Session>
	{
		void get_init_status() {
			PDBG("sched_controller is initialized");
		}
	};

}
