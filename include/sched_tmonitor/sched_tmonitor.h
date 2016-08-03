/*
 * \brief  header for the sched_tmonitor component
 * \author Paul Nieleck
 * \date   2016/08/03
 *
 */

#ifndef _INCLUDE__SCHED_TMONITOR__SCHED_TMONITOR_H_
#define _INCLUDE__SCHED_TMONITOR__SCHED_TMONITOR_H_

#include <sched_controller_session/sched_controller_session.h>
#include <base/rpc_client.h>
#include <base/printf.h>

namespace Sched_tmonitor {

	struct Session_client : Genode::Rpc_client<Session>
	{
		Session_client(Genode::Capability<Session> cap)
		: Genode::Rpc_client<Session>(cap) { }

		void get_status()
		{
			PDBG("Calling Rpc-Interface function to get init status of sched_monitor.");
			call<Rpc_get_init_status>();
		}
	};
}

#endif /* _INCLUDE__SCHED_TMONITOR__SCHED_TMONITOR_H_ */
