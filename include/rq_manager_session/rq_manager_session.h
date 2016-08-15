/*
 * \brief  Interface definition of the Rq_manager
 * \author Paul Nieleck
 * \date   2016/08/15
 *
 */

#ifndef _INCLUDE__RQ_MANAGER_SESSION__RQ_MANAGER_SESSION_H_
#define _INCLUDE__RQ_MANAGER_SESSION__RQ_MANAGER_SESSION_H_

#include <session/session.h>
#include <base/rpc.h>

#include "rq_manager/rq_task.h"

namespace Rq_manager {

	struct Session : Genode::Session
	{

		static const char *service_name() { return "Rq_manager"; }

		virtual int enq(int, Rq_task) = 0;
		virtual int deq(int, Rq_task**) = 0;
		virtual Genode::Dataspace_capability dataspace() = 0;


		/*********************
		 ** RPC declaration **
		 *********************/

		GENODE_RPC(Rpc_enq, int, enq, int, Rq_task);
		GENODE_RPC(Rpc_deq, int, deq, int, Rq_task**);
		GENODE_RPC(Rpc_dataspace, Genode::Dataspace_capability, dataspace);

		GENODE_RPC_INTERFACE(Rpc_enq,
		                     Rpc_deq,
		                     Rpc_dataspace);

	};

}

#endif /* _INCLUDE__RQ_MANAGER_SESSION__RQ_MANAGER_SESSION_H_ */
