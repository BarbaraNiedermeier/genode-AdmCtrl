/*
 * \brief  Pcore - Platform core definition
 * \author Paul Nieleck
 * \date   2016/09/07
 */

#include <base/printf.h>

#include "sched_controller/pcore.h"
#include "rq_manager/rq_buffer.h"
#include "rq_manager/rq_task.h"

namespace Sched_controller
{

	int allocate_rq(int rq)
	{

		rqs.push_back(rq);
		return 0;

	}

	int deallocate_rq(int rq)
	{
		for (int i = 0; i < rqs.size; i++) {
			if (rqs[i] == rq) {
				rqs.erase(i);
				return rq;
			}
		}

		return -1;
	}

	std::vector<int> get_rqs()
	{

		return rqs;

	}

	/******************
	 ** Constructors **
	 ******************/

	Pcore::Pcore(int core_id)
	{
		id = core_id
		rqs = new vector<int>;
		//pcore_state = Pcore_state::active;
	}

}