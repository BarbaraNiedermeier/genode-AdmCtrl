/*
 * \brief  superclass of different scheduling algorithms.
 * \author Paul Nieleck
 * \date   2016/09/22
 */
 
#ifndef _INCLUDE__SCHED_CONTROLLER__SCHED_ALG_H_
#define _INCLUDE__SCHED_CONTROLLER__SCHED_ALG_H_

#include "sched_controller/rq_buffer.h"

namespace Sched_controller {

	int RTA(int core, Rq_task::Rq_task *new_task, Rq_buffer<Rq_task::Rq_task> *rq_buf);

	class Sched_alg {

	};

}

#endif /* _INCLUDE__SCHED_CONTROLLER__SCHED_ALG_H_ */