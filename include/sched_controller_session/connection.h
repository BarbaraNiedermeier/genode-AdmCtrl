/*
 * \brief  connection wrapper to access the sched_controller
 * \author Paul Nieleck
 * \date   2016/08/04
 *
 */

#ifndef _INCLUDE__SCHED_CONROLLER_SESSION__CONNECTION_H_
#define _INCLUDE__SCHED_CONROLLER_SESSION__CONNECTION_H_

#include <sched_controller_session/client.h>
#include <base/connection.h>

namespace Sched_controller {

	struct Connection : Genode::Connection<Session>, Session_client
	{
		Connection()
		:

			Genode::Connection<Sched_controller::Session>(session("foo, ram_quota=4096")),

			Session_client(cap()) {}
	};

}

#endif /* _INCLUDE__SCHED_CONROLLER_SESSION__CONNECTION_H_ */
