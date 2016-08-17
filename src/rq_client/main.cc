/*
 * \brief  simple client to test shared mem with Rq_manager
 * \author Paul Nieleck
 * \date   2016/08/17
 */

#include <base/env.h>
#include <base/printf.h>
#include "rq_manager_session/client.h"
#include "rq_manager_session/connection.h"
#include "rq_manager/rq_buffer.h"

using namespace Genode;

int main()
{
	Rq_manager::Connection rqm;
	Rq_manager::Rq_task *buf;
	Dataspace_capability dsc;

	dsc = rqm.get_core_rq_ds(0);
	PINF("Got Dataspace_capability :)");

	buf = env()->rm_session()->attach(dsc);
	PINF("Dataspace_capability successfully attached :D");

	return 0;
}