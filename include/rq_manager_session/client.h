/*
 * \brief  Client interface to the Rq_manager
 * \author Paul Nieleck
 * \date   2016/08/15
 *
 */

#ifndef _INCLUDE__RQ_MANAGER_SESSION__CLIENT_H_
#define _INCLUDE__RQ_MANAGER_SESSION__CLIENT_H_

/* global includes */
#include <base/rpc_client.h>

/* local includes */
#include "rq_manager_session/rq_manager_session.h"
#include "rq_manager/rq_task.h"

namespace Rq_manager {

	struct Session_client : Genode::Rpc_client<Session>
	{
		Session_client(Genode::Capability<Session> cap) : Genode::Rpc_client<Session>(cap) { }

		int enq(int core, Rq_task task)
		{
			return call<Rpc_enq>(core, task);
		}

		/* 
		 * This will probably not work!
		 * Why? Because the pointer we
		 * give here as an argument points
		 * to an address in the address-
		 * space of the client, which is
		 * not the same as the address
		 * space of the server. Both are
		 * only virtual address spaces,
		 * i.e. the pointer points to
		 * completely different locations.
		 * :(
		 */
//		int deq(int core, Rq_task **task)
//		{
//			return call<Rpc_deq>(core, task);
//		}

		/* 
		 * return the capability to access the
		 * dataspace of the Rq_manager
		 */
		Genode::Dataspace_capability get_core_rq_ds(int core)
		{
			return call<Rpc_get_core_rq_ds>(core);
		}

	};

}

#endif /* _INCLUDE__RQ_MANAGER_SESSION__CLIENT_H_ */
