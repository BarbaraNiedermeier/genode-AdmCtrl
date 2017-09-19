/*
 * \brief  Client interface to the Sched_controller
 * \author Paul Nieleck
 * \date   2016/08/04
 *
 */

#ifndef _INCLUDE__SCHED_CONTROLLER_SESSION__CLIENT_H_
#define _INCLUDE__SCHED_CONTROLLER_SESSION__CLIENT_H_

#include <sched_controller_session/sched_controller_session.h>
#include <base/rpc_client.h>
#include <base/printf.h>

#include "rq_task/rq_task.h"

namespace Sched_controller {

	struct Session_client : Genode::Rpc_client<Session>
	{
		Session_client(Genode::Capability<Session> cap)
		: Genode::Rpc_client<Session>(cap) { }

		void get_init_status()
		{
			PDBG("Calling Rpc-Interface function to get init status of sched_monitor.");
			call<Rpc_get_init_status>();
		}

		int new_task(Rq_task::Rq_task task, int core)
		{
			return call<Rpc_new_task>(task, core);
		}

		void set_sync_ds(Genode::Dataspace_capability ds_cap)
		{
			call<Rpc_set_sync_ds>(ds_cap);
		}

		int are_you_ready()
		{
			return call<Rpc_are_you_ready>();
		}

		int update_rq_buffer(int core)
		{
			return call<Rpc_update_rq_buffer>(core);
		}

	};
}

#endif /* _INCLUDE__SCHED_CONTROLLER_SESSION__CLIENT_H_ */
