/*
 * \brief  Pcore - Platform core definition
 * \author Paul Nieleck
 * \date   2016/09/07
 */

#include <base/printf.h>
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

	int Pcore::set_id(int core_id)
	{
		id = core_id;
		return 0;
	}

	int Pcore::get_id()
	{
		return id;
	}


	/******************
	 ** Constructors **
	 ******************/

	Pcore::Pcore()
	{
		pcore_state = Pcore_state::active;
		Pcore::_cores.push_front(this);
	}

	Pcore::~Pcore()
	{
		Pcore::_cores.remove(this);
	}


}