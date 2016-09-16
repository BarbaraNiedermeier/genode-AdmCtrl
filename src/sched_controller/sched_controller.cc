/*
 * \brief  sched_controller
 * \author Paul Nieleck
 * \date   2016/09/09
 *
 */

#include "sched_controller/sched_controller.h"
#include "sched_controller/task_allocator.h"
#include "rq_manager_session/client.h"
#include "rq_manager_session/connection.h"

#include <base/printf.h>

namespace Sched_controller {

	/**
	 * Initialize the pcores, i.e. create new
	 * instances of the pcore class
	 *
	 * \return success status
	 */
	int Sched_controller::_init_pcores()
	{

		_pcore = new Pcore[_num_pcores];

		for (int i = 0; i < _num_pcores; i++) {
			_pcore[i].set_id(i);
		}

		return 0;
	}

	/**
	 * Get and set the number of available physically
	 * available CPU cores of the system.
	 * This function will not change the number of
	 * _pcore objects.
	 *
	 * \return success status
	 */
	int Sched_controller::_set_num_pcores()
	{
		/* 
		 * From the monitor we will request the number of physical
		 * cores available at the system. Currently this feature is
		 * not implemented, therefore we will set the number of cores
		 * to be 4.
		 */
		_num_pcores = 4;

		return 0;
	}

	/**
	 * Call the Task_allocator to allocate newly arriving tasks
	 * (comming in via the respective RPC-call) to a sufficient
	 * pcore/rq_buffer.
	 *
	 * \param newly arriving task
	 */
	void Sched_controller::allocate_task(Rq_manager::Rq_task task)
	{

		PINF("Now we'll allocate Task with id %d", task.task_id);
		Task_allocator::allocate_task(&task);

	}

	/******************
	 ** Constructors **
	 ******************/

	Sched_controller::Sched_controller()
	{

		/* We then need to figure out how many CPU cores are available at the system */
		_set_num_pcores();

		/* And finally we will create instances of _pcore */
		_init_pcores();

		/* 
		 * Check how many run queues (Rq_buffer) are created
		 * For the final implementation it is actually planned
		 * to initialize the run queues dynamically, but therefore
		 * the rq_manager has to be changed accordingly. At the
		 * moment the rq_manager is configured to provide a fixed
		 * number of run queues.
		 */
		_num_rqs = _rq_manager.get_num_rqs();
		PDBG("Number of supplied run queues is: %d", _num_rqs);

		/*
		 * After we know about our run queues, we will assign them to the pcores.
		 * Currently we have 4 run queues and 4 pcores. Hence we can make a fixed
		 * assignement.
		 */
		for (int i = 0; i < _num_rqs; i++) {
			_pcore[i].allocate_rq(i);
			PINF("Allocated rq_buffer %d to _pcore %d", i, i);
		}

	}

	Sched_controller::~Sched_controller()
	{

	}

}