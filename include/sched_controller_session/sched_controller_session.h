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

namespace Sched_controller {

	struct Session : Genode::Session
	{
		static const char *service_name() { return "Sched_controller"; }

		virtual void get_init_status() = 0;

		GENODE_RPC(Rpc_get_init_status, void, get_init_status);
		GENODE_RPC_INTERFACE(Rpc_get_init_status);
	};
}

#endif /* _INCLUDE__SCHED_CONTROLLER_SESSION__SCHED_CONTROLLER_SESSION_H_ */
