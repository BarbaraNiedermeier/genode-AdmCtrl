/*
 * \brief  Core definition
 * \author Paul Nieleck
 * \date   2016/09/07
 *
 * Each CPU core of the system that is available
 * to the scheduler is represented in its own class.
 */

#ifndef _INCLUDE__SCHED_CONTROLLER__CORE_H_
#define _INCLUDE__SCHED_CONTROLLER__CORE_H_

namespace Sched_controller
{

	enum class Core_state { active, suspended, off };

	class Core
	{

		protected:
			int id;
			Task_class task_class;
			Core_state core_state;
			Sched_alg sched_alg;

		public:
			float get_utilization();

			virtual Core_state get_core_state = 0;
			virtual float get_clockspead() = 0;

			virtual int set_core_state(Core_state) = 0;
			virtual float set_clockspeed(float) = 0;

	};

	class Arm_core : Core
	{

	};

	class Intel_core : Core
	{

	};

}

#endif /* _INCLUDE__SCHED_CONTROLLER__CORE_H_ */