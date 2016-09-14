/*
 * \brief  Pcore - Platform core definition
 * \author Paul Nieleck
 * \date   2016/09/07
 */

#include <base/printf.h>
#include <vector>

#include "sched_controller/pcore.h"
#include "rq_manager/rq_buffer.h"
#include "rq_manager/rq_task.h"

namespace Sched_controller
{

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

	/******************
	 ** Constructors **
	 ******************/

	Pcore::Pcore()
	{
		//id = core_id;
		//rqs = new std::vector<int>;
		pcore_state = Pcore_state::active;
	}

}