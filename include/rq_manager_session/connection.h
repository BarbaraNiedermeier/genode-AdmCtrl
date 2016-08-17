/*
 * \brief  connection wrapper to access Rq_manager
 * \author Paul Nieleck
 * \date   2016/08/15
 *
 */

#ifndef _INCLUDE__RQ_MANAGER_SESSION__CONNECTION_H_
#define _INCLUDE__RQ_MANAGER_SESSION__CONNECTION_H_

#include <base/connection.h>
#include "rq_manager_session/client.h"

namespace Rq_manager {

	struct Connection : Genode::Connection<Session>, Session_client
	{
		Connection() : Genode::Connection<Rq_manager::Session>(session("rq_manager, ram_quota=4096")),
		               Session_client(cap()) { }
	};


}

#endif /* _INCLUDE__RQ_MANAGER_SESSION__CONNECTION_H_ */
