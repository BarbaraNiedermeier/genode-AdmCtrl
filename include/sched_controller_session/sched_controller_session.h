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
#include <string>
#include <util/string.h>

#include "rq_task/rq_task.h"

namespace Sched_controller {

	//typedef Genode::String<32> Task_name;
	
	struct Session : Genode::Session
	{
		static const char *service_name() { return "Sched_controller"; }

		virtual void get_init_status() = 0;
		virtual int new_task(Rq_task::Rq_task, int core) = 0;
		virtual void set_sync_ds(Genode::Dataspace_capability) = 0;
		virtual int are_you_ready() = 0;
		virtual int update_rq_buffer(int core) = 0;
		virtual void optimize (Genode::String<32> task_name) = 0;
		virtual void set_opt_goal (Genode::Ram_dataspace_capability) = 0;
		virtual int scheduling_allowed(Genode::String<32>) = 0;
		virtual void last_job_started(Genode::String<32>) = 0;

		GENODE_RPC(Rpc_get_init_status, void, get_init_status);
		GENODE_RPC(Rpc_new_task, int, new_task, Rq_task::Rq_task, int);
		GENODE_RPC(Rpc_set_sync_ds, void, set_sync_ds, Genode::Dataspace_capability);
		GENODE_RPC(Rpc_are_you_ready, int, are_you_ready);
		GENODE_RPC(Rpc_update_rq_buffer, int, update_rq_buffer, int);
		GENODE_RPC(Rpc_optimize, void, optimize, Genode::String<32>);
		GENODE_RPC(Rpc_set_opt_goal, void, set_opt_goal, Genode::Ram_dataspace_capability);
		GENODE_RPC(Rpc_scheduling_allowed, int, scheduling_allowed, Genode::String<32>);
		GENODE_RPC(Rpc_last_job_started, void, last_job_started, Genode::String<32>);
		
		
		GENODE_RPC_INTERFACE(Rpc_get_init_status, Rpc_new_task, Rpc_set_sync_ds, Rpc_are_you_ready, Rpc_update_rq_buffer, Rpc_optimize, Rpc_set_opt_goal, Rpc_scheduling_allowed, Rpc_last_job_started);
	};
}

#endif /* _INCLUDE__SCHED_CONTROLLER_SESSION__SCHED_CONTROLLER_SESSION_H_ */
