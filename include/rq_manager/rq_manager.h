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

//namespace Rq_manager {
//
//	class Rq_manager;
//
//}

class Rq_manager
{

	private:

		int _num_cores = 0;
		
		int _set_ncores(int);
	//	int _set_rqs(Rq_buffer::Rq_buffer<int> *r);

	public:

		int enc(int, int);
		int deq(int);
		int deq_n(int, int);

		Rq_manager();
		Rq_manager(int);
		~Rq_manager();

};

#endif /* _INCLUDE__RQ_MANAGER__RQ_MANAGER_H_ */
