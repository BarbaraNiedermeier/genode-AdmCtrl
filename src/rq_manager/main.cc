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
		int success = _rqs[core].enq(task);
		PINF("Inserted task to core %d", core);
		return success;
	}

	return 1;

}

int Rq_manager::deq(int core, Ctr_task *task_ptr)
{

	if (core < _num_cores) {
		int success = _rqs[core].deq(task_ptr);
		PINF("Removed tast from core %d", core);
		return success;
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

	int *deqelem = 0;
	int val1 { 11 };
	int val2 = 22;
	int val3 = 33;
	Ctr_task task1 = {666, 1000, true};
	Ctr_task task2 = {777, 100, false};
	Ctr_task *deq_task;

	Rq_buffer<int> buf(10);
	PINF("New Buffer created");
	printf("Enq variable with value %d at pointer %p\n", val1, &val1);
	buf.enq(val1);
	buf.enq(val2);
	buf.enq(val3);
	PINF("Elements enqueued");
	int result = buf.deq(deqelem);
	PINF("result is: %d", result);
	PINF("address of the first dequeued element is: %p", deqelem);
	PINF("content is: %f", *deqelem);

	PINF("Now we will create several rqs to work with!");
	Rq_manager mgmt (2);
	mgmt.enq(0, task1);
	mgmt.enq(0, task2);

	PINF("Starting to dequeue some task");
	mgmt.deq(0, deq_task);
	PINF("Got task with task_id: %d, wcet: %d, valid: %d", deq_task->task_id, deq_task->wcet, deq_task->valid);

	return 0;
}
