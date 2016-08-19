/*
 * \brief  Rq_manager class definitions
 * \author Paul Nieleck
 * \date   2016/08/06
 *
 * The Rq_manager provides a data structure for the
 * scheduling controller and the scheduling synchronization
 */

#ifndef _INCLUDE__RQ_MANAGER__RQ_MANAGER_H_
#define _INCLUDE__RQ_MANAGER__RQ_MANAGER_H_

/* global includes */
#include <base/printf.h>
#include <base/env.h>
#include <dataspace/client.h>

/* local includes */
#include "rq_manager/rq_buffer.h"
#include "rq_manager/rq_task.h"
#include "rq_manager_session/rq_manager_session.h"

namespace Rq_manager {

	class Rq_manager
	{

		private:

			int _num_cores = 0;
			Rq_buffer<Rq_task> *_rqs; /* array of ring buffers (Rq_buffer with fixed size) */
			
			int _init_rqs(int);
			int _set_ncores(int);

		public:

			int enq(int, Rq_task);
			int deq(int, Rq_task**);
			Genode::Dataspace_capability get_core_rq_ds(int);
//			int deq_n(int, int);

			Rq_manager();
			Rq_manager(int);
//			~Rq_manager();

	};

}

#endif /* _INCLUDE__RQ_MANAGER__RQ_MANAGER_H_ */
