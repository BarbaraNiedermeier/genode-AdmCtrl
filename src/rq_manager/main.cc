/*
 * \brief  This component provides an interface between Controller and Sync
 * \author Paul Nieleck
 * \date   2016/08/07
 *
 */

#include <base/env.h> /* not sure if needed */
#include <base/printf.h>
//#include "rq_manager/rq_manager.h"
#include "rq_manager/rq_buffer.h"

// using namespace Rq_manager;

//int Rq_manager::_set_ncores(int n)
//{
//	_num_cores = n;
//	
//	return 0;
//}
//
//Rq_manager::Rq_manager()
//{
//	PINF("Value of available system cores not provided -> set to 2.");
//
//	_set_ncores(2);
//}
//
//Rq_manager::Rq_manager(int num_cores)
//{
//	_set_ncores(num_cores);
//}


using namespace Genode;

int main()
{

	double *deqelem;

	Rq_buffer<double> buf(10);
	PINF("New Buffer created");
	buf.enq(11.0);
	buf.enq(22.0);
	buf.enq(33.0);
	PINF("Elements enqueued");
	int result = buf.deq(deqelem);
	PINF("result is: %d", result);
	PINF("address of the first dequeued element is: %p", deqelem);

	return 0;
}
