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

#include <base/printf.h>
#include "rq_manager/rq_buffer.h"

struct Ctr_task
{

	int task_id;
	int wcet;
	bool valid;

};

class Rq_manager
{

	private:

		int _num_cores = 0;
		Rq_buffer<Ctr_task> *_rqs;  /* array of ring buffers (Rq_buffer with fixed size) */
		
		int _init_rqs();
		int _set_ncores(int);

	public:

		int enq(int, Ctr_task);
		int deq(int, Ctr_task**);
//		int deq_n(int, int);

		Rq_manager();
		Rq_manager(int);
//		~Rq_manager();

};

#endif /* _INCLUDE__RQ_MANAGER__RQ_MANAGER_H_ */
