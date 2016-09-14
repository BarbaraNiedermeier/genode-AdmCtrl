/*
 * \brief  Scheduler Controller main class
 * \author Paul Nieleck
 * \date   2016\09\08
 *
 * This class is the main class of the scheduling
 * controller. It instantiates the cores, provides
 * interfaces, initiates task allocations and
 * is responsible for the controlling of cores.
 */

#ifndef _INCLUDE__SCHED_CONTROLLER__SCHED_CONTROLLER_H_
#define _INCLUDE__SCHED_CONTROLLER__SCHED_CONTROLLER_H_

#include <vector>

#include "rq_manager_session/client.h"
#include "rq_manager_session/connection.h"
#include "sched_controller/pcore.h"

namespace Sched_controller
{

	class Sched_controller
	{

		private:

			Rq_manager::Connection _rq_manager;
			int _num_rqs = 0;
			int _num_pcores = 0;
			Pcore *_pcore;

			int _init_pcores();
			int _set_num_pcores();

		public:

			Sched_controller();
			~Sched_controller();

	};

}

#endif /* _INCLUDE__SCHED_CONTROLLER__SCHED_CONTROLLER_H_ */