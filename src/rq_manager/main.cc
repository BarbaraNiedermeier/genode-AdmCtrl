/*
 * \brief  This component provides an interface between Controller and Sync
 * \author Paul Nieleck
 * \date   2016/08/07
 *
 */

#include <base/printf.h>
#include <rq_manager/rq_manager.h>

namespace Rq_manager{

	int _set_ncores(int n)
	{
		_num_cores = n;
		
		return 0;
	}

//	int _set_rqs(std::vector<Genode::fifo<int>> *r)
//	{
//		
//	}

	Rq_manager()
	{
		PINF("Value of available system cores not provided -> set to 2.");

		_set_ncores(2);
	}

	Rq_manager(int num_cores)
	{
		_set_ncores(num_cores);
	}

}

using namespace Genode;

int main(void)
{

	Rq_buffer::Rq_buffer<int> buf (10);
	buf.enq(11);
	buf.enq(22);
	buf.enq(33);
	buf.deq();
	buf.deq();
	buf.deq();
	buf.deq();

	return 0;
}
