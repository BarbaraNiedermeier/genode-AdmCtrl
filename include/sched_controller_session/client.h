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

#include "rq_manager/rq_task.h"

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

		void new_task(Rq_manager::Rq_task task)
		{
			call<Rpc_new_task>(task);
		}

	};
}

#endif /* _INCLUDE__SCHED_CONTROLLER_SESSION__CLIENT_H_ */
