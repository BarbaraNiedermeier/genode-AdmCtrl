/*
 * \brief  Interface definition of the Sched_controller
 * \author Paul Nieleck
 * \date   2016/08/02
 *
 */

#ifndef _INCLUDE__SCHED_CONTROLLER_SESSION__SCHED_CONTROLLER_SESSION_H_
#define _INCLUDE__SCHED_CONTROLLER_SESSION__SCHED_CONTROLLER_SESSION_H_

#include <session/session.h>
#include <base/rpc.h>

#include "rq_task/rq_task.h"

namespace Sched_controller {

	struct Session : Genode::Session
	{
		static const char *service_name() { return "Sched_controller"; }

		virtual void get_init_status() = 0;
		virtual int new_task(Rq_task::Rq_task, int core) = 0;
		virtual void set_sync_ds(Genode::Dataspace_capability) = 0;
		virtual int are_you_ready() = 0;
		virtual int update_rq_buffer(int core) = 0;

		GENODE_RPC(Rpc_get_init_status, void, get_init_status);
		GENODE_RPC(Rpc_new_task, int, new_task, Rq_task::Rq_task, int);
		GENODE_RPC(Rpc_set_sync_ds, void, set_sync_ds, Genode::Dataspace_capability);
		GENODE_RPC(Rpc_are_you_ready, int, are_you_ready);
		GENODE_RPC(Rpc_update_rq_buffer, int, update_rq_buffer, int);
		GENODE_RPC_INTERFACE(Rpc_get_init_status, Rpc_new_task, Rpc_set_sync_ds, Rpc_are_you_ready, Rpc_update_rq_buffer);
	};
}

#endif /* _INCLUDE__SCHED_CONTROLLER_SESSION__SCHED_CONTROLLER_SESSION_H_ */
