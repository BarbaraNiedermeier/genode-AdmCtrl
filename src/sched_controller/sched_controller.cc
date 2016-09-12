/*
 * \brief  sched_controller
 * \author Paul Nieleck
 * \date   2016/09/09
 *
 */

#include "sched_controller/sched_controller.h"
#include "rq_manager_session/client.h"
#include "rq_manager_session/connection.h"

#include <base/printf.h>

namespace Sched_controller {

	/******************
	 ** Constructors **
	 ******************/

	Sched_controller::Sched_controller()
	{

		/* On initialization check how many run queues (Rq_buffer) are created */
		num_rqs = rq_manager.get_num_rqs();
		PDBG("Number of supplied run queues is: %d", num_rqs);
	}

	Sched_controller::~Sched_controller()
	{

	}

}