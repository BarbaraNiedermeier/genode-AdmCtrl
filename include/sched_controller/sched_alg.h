/*
 * \brief  superclass of different scheduling algorithms.
 * \author Paul Nieleck
 * \date   2016/09/22
 */
 
#ifndef _INCLUDE__SCHED_CONTROLLER__SCHED_ALG_H_
#define _INCLUDE__SCHED_CONTROLLER__SCHED_ALG_H_

#include "sched_controller/rq_buffer.h"

namespace Sched_controller
{
	class Sched_alg
	{
	private:
		int _response_time_old;
		int _response_time;
		int _compute_repsonse_time(Rq_task::Rq_task *new_task, Rq_buffer<Rq_task::Rq_task> *rq_buf, int num_elements, float deadline);
		Rq_task::Rq_task *_curr_task;

	public:
		int RTA(Rq_task::Rq_task *new_task, Rq_buffer<Rq_task::Rq_task> *rq_buf);
		int fp_sufficient_test(Rq_task::Rq_task *new_task, Rq_buffer<Rq_task::Rq_task> *rq_buf);
	};
}

#endif /* _INCLUDE__SCHED_CONTROLLER__SCHED_ALG_H_ */