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
#include <rq_manager/rq_buffer.h>

namespace Rq_manager {

	class Rq_manager
	{

		private:

			int _num_cores = 0;
			
			int _set_ncores(int n);
		//	int _set_rqs(Rq_buffer::Rq_buffer<int> *r);

		public:

			int enc(int rq, int num);
			int deq(int rq);
			int deq_n(int rq, int n);

			Rq_manager();
			Rq_manager(int num_cores);
			~Rq_manager();

	};

}

#endif /* _INCLUDE__RQ_MANAGER__RQ_MANAGER_H_ */
