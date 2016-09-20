/*
 * \brief  Pcore - Platform core definition
 * \author Paul Nieleck
 * \date   2016/09/07
 */

#include <base/printf.h>
#include <vector>
#include <forward_list>

#include "sched_controller/pcore.h"
#include "rq_manager/rq_buffer.h"
#include "rq_manager/rq_task.h"

namespace Sched_controller
{

	/*
	 * This static vector contains the pointers to all
	 * created Pcore objects.
	 */
	std::forward_list<Pcore*> Pcore::_cores;

	/**
	 * Get the list of current pcores at the system.
	 *
	 * \return forward list of pointers to the pcores
	 */
	std::forward_list<Pcore*> Pcore::get_pcores()
	{
		return Pcore::_cores;
	}

	/**
	 * Allocate a run queue to this pcore.
	 *
	 * \param Number of the run queue.
	 *
	 * \return Returns 0 on success.
	 */
	int Pcore::allocate_rq(int rq)
	{

		rqs.push_back(rq);
		return 0;

	}

	int Pcore::deallocate_rq(int rq)
	{
		for (unsigned int i = 0; i < rqs.size(); i++) {
			if (rqs[i] == rq) {
				rqs.erase(rqs.begin()+i);
				return rq;
			}
		}

		return -1;
	}

	std::vector<int> Pcore::get_rqs()
	{

		return rqs;

	}

	int Pcore::set_id(int core_id)
	{
		id = core_id;
		return 0;
	}

	int Pcore::get_id()
	{
		return id;
	}

	/**
	 * Get the Task_class of the pcore.
	 *
	 * \return Returns the Task_class of the pcore.
	 */
	Rq_manager::Task_class Pcore::get_class()
	{
		return _pcore_class;
	}

	
	/**
	 * Set the task_class of the pcore. This is only possible
	 * if the pcore hosts no run queues;
	 *
	 * \param task class
	 *
	 * \return Returns 0 on success. Returns 1 if the pcore
	 *         hosts run queues and therefore can not change
	 *         the Task_class.
	 */
	int Pcore::set_class(Rq_manager::Task_class task_class)
	{
		if (rqs.empty()) {
			_pcore_class = task_class;
			return 0;
		} else {
			return 1;
		}
	}


	/******************
	 ** Constructors **
	 ******************/

	Pcore::Pcore()
	{
		//id = core_id;
		//rqs = new std::vector<int>;
		set_class(Rq_manager::Task_class::lo);
		pcore_state = Pcore_state::active;
		Pcore::_cores.push_front(this);
	}

	Pcore::~Pcore()
	{
		Pcore::_cores.remove(this);
	}


}