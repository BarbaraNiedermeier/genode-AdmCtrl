/*
 * \brief  This component provides an interface between Controller and Sync
 * \author Paul Nieleck
 * \date   2016/08/07
 *
 */

#include <base/env.h> /* not sure if needed */
#include <base/printf.h>
#include "rq_manager/rq_manager.h"
#include "rq_manager/rq_buffer.h"

int Rq_manager::_init_rqs()
{

	PDBG("Initialize the array of Rq_buffers");
	_rqs = new Rq_buffer<Ctr_task>[_num_cores];

	return 0;

}

int Rq_manager::_set_ncores(int n)
{
	_num_cores = n;
	
	return 0;
}

int Rq_manager::enq(int core, Ctr_task task)
{

	if (core < _num_cores) {
		_rqs[core].enq(task);
		PINF("Inserted task to core %d", core);
		return 0;
	}

	return 1;

}

Rq_manager::Rq_manager()
{
	PINF("Value of available system cores not provided -> set to 2.");

	_set_ncores(2);
	_init_rqs();
}

Rq_manager::Rq_manager(int num_cores)
{
	_set_ncores(num_cores);
	_init_rqs();
}


using namespace Genode;

int main()
{

	double *deqelem;
	Ctr_task task1 = {666, 1000, true};
	Ctr_task task2 = {777, 100, false};

	Rq_buffer<double> buf(10);
	PINF("New Buffer created");
	buf.enq(11.0);
	buf.enq(22.0);
	buf.enq(33.0);
	PINF("Elements enqueued");
	int result = buf.deq(deqelem);
	PINF("result is: %d", result);
	PINF("address of the first dequeued element is: %p", deqelem);

	PINF("Now we will create several rqs to work with!");
	Rq_manager mgmt (2);
	mgmt.enq(0, task1);
	mgmt.enq(0, task2);

	return 0;
}
