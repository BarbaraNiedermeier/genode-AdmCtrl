/*
 * \brief  Rq_manager class
 * \author Paul Nieleck
 * \date   2016/08/17
 *
 * This class is manages several run queues
 * of type Rq_buffer.
 */

#include <base/printf.h>

/* local includes */
#include "rq_manager/rq_manager.h"
#include "rq_manager/rq_buffer.h"
#include "rq_manager/rq_task.h"

namespace Rq_manager
{

	/**
	 * Initialize the run queues that are used.
	 *
	 * \param rq_size: size of the run queues,
	 *        determines how many task can be
	 *        held in the queue at once.
	 *
	 * \return 0 if finished
	 */
	int Rq_manager::_init_rqs(int rq_size)
	{

		_rqs = new Rq_buffer<Rq_task>[_num_cores];

		for (int i = 0; i < _num_cores; i++) {
			_rqs[i].init_w_shared_ds(rq_size);
		}
		Genode::printf("New Rq_buffer created. Starting address is: %p.\n", _rqs);

		return 0;

	}

	/**
	 * Set the number of system cores that are used
	 * for scheduling, i.e. set the number of run
	 * queues to be available.
	 *
	 * \param n: number of cores
	 *
	 * \return 0 if finished
	 */
	int Rq_manager::_set_ncores(int n)
	{
		_num_cores = n;
		
		return 0;
	}

	/**
	 * Enqueue a new Task in the buffer
	 *
	 * \param core: which core/run queue the
	 *        task should be added to
	 * \param task: the task that should be added
	 *
	 * \return  0 if successful
	 *         >0 in any other case
	 */
	int Rq_manager::enq(int core, Rq_task task)
	{
		PDBG("Task is enqueued now.");

		if (core < _num_cores) {
			int success = _rqs[core].enq(task);
			//return success;
			return 1234;
		}


		return 1;

	}

	/**
	 * Dequeue a task from a given run queue
	 *
	 * \param core: specify the run queue from which
	 *        the element should be dequeued
	 * \param **task_ptr: pointer that will be set
	 *        to the location where the task is stored
	 */
	int Rq_manager::deq(int core, Rq_task **task_ptr)
	{

		if (core < _num_cores) {
			int success = _rqs[core].deq(task_ptr);
			PINF("Removed task from core %d, pointer is %p", core, *task_ptr);
			return success;
		}

		return 1;
	}

	/**
	 * Return the number of run queues
	 */
	int Rq_manager::get_num_rqs()
	{
		return _num_cores;
	}

	/**
	 *
	 */
	Genode::Dataspace_capability Rq_manager::get_core_rq_ds(int core)
	{
		return _rqs[core].get_ds_cap();
	}


	/******************
	 ** Constructors **
	 ******************/

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