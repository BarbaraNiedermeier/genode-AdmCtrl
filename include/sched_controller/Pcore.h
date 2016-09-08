/*
 * \brief  Pcore - Platform core definition
 * \author Paul Nieleck
 * \date   2016/09/07
 *
 * Each CPU core of the system that is available
 * is represented in its own class. Each Pcore
 * is assigned 0..n logical cores Lcore.
 */

#ifndef _INCLUDE__SCHED_CONTROLLER__PCORE_H_
#define _INCLUDE__SCHED_CONTROLLER__PCORE_H_

#include "rq_manager/rq_buffer.h"
#include "rq_manager/rq_task.h"

namespace Sched_controller
{

	enum class Pcore_state { active, standby, off };

	class Pcore
	{

		protected:

			int id;
			Core_state core_state;

		public:

			float get_utilization();

			virtual Core_state get_core_state = 0;
			virtual float get_clockspead() = 0;

			virtual int set_core_state(Core_state) = 0;
			virtual float set_clockspeed(float) = 0;

	};

	class Arm_core : Pcore
	{

	};

	class Intel_core : Pcore
	{

	};

}

#endif /* _INCLUDE__SCHED_CONTROLLER__PCORE_H_ */