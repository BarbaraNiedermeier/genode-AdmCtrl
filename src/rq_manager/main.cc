/*
 * \brief  This component provides an interface between Controller and Sync
 * \author Paul Nieleck
 * \date   2016/08/07
 *
 */

/* global includes */
#include <base/env.h> /* not sure if needed */
#include <base/printf.h>

/* local includes */
#include "rq_manager/rq_manager.h"
#include "rq_manager/rq_buffer.h"
#include "rq_manager/rq_task.h"

namespace Rq_manager
{

	int Rq_manager::_init_rqs(int rq_size)
	{

		_rqs = new Rq_buffer<Rq_task>[_num_cores];

		for (int i = 0; i < _num_cores; i++) {
			_rqs[i].init_w_shared_ds(rq_size);
		}
		Genode::printf("New Rq_buffer created. Starting address is: %p.\n", _rqs);

		return 0;

	}

	int Rq_manager::_set_ncores(int n)
	{
		_num_cores = n;
		
		return 0;
	}

	int Rq_manager::enq(int core, Rq_task task)
	{

		if (core < _num_cores) {
			int success = _rqs[core].enq(task);
			//PINF("Inserted task to core %d", core);
			return success;
		}

		return 1;

	}

	int Rq_manager::deq(int core, Rq_task **task_ptr)
	{

		if (core < _num_cores) {
			int success = _rqs[core].deq(task_ptr);
			PINF("Removed task from core %d, pointer is %p", core, *task_ptr);
			return success;
		}

		return 1;
	}

	Genode::Dataspace_capability Rq_manager::get_core_rq_ds(int core)
	{
		return _rqs[core].get_ds_cap();
	}

	Rq_manager::Rq_manager()
	{
		PINF("Value of available system cores not provided -> set to 2.");

		_set_ncores(2);
		_init_rqs(100);
	}

	Rq_manager::Rq_manager(int num_cores)
	{
		_set_ncores(num_cores);
		_init_rqs(100);
	}
}

using namespace Genode;

int main()
{

	/* testing the Rq_buffer */
	Rq_manager::Rq_task task1 = {111, 1000, true};
	Rq_manager::Rq_task task2 = {222, 100, false};
	Rq_manager::Rq_task task3 = {333, 1234, true};
	Rq_manager::Rq_task task4 = {444, 4321, false};
	Rq_manager::Rq_task task5 = {555, 4524, true};
	Rq_manager::Rq_task task6 = {666, 5875, false};
	Rq_manager::Rq_task *deq_task;

	PINF("Now we will create several rqs to work with!");
	Rq_manager::Rq_manager mgmt (4);
	mgmt.enq(0, task1);
	mgmt.enq(0, task2);
	mgmt.enq(0, task3);
	mgmt.enq(0, task4);
	mgmt.enq(0, task5);

	for (int i = 0; i < 50; i++) {
		mgmt.enq(1, task6);
	}

	PINF("Starting to dequeue some task");
	mgmt.deq(0, &deq_task);
	PINF("Got task with task_id: %d, wcet: %d, valid: %d", deq_task->task_id, deq_task->wcet, deq_task->valid);
	mgmt.deq(0, &deq_task);
    PINF("Got task with task_id: %d, wcet: %d, valid: %d", deq_task->task_id, deq_task->wcet, deq_task->valid);

	return 0;
}
