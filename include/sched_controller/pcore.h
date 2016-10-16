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

#include <vector>
#include <forward_list>

#include "rq_manager/rq_buffer.h"
#include "rq_task/rq_task.h"

namespace Sched_controller
{

	enum class Pcore_state { active, standby, off };

	class Pcore
	{

		private:
			static std::forward_list<Pcore*> _cores;

		protected:

			int id;
			Pcore_state pcore_state;
			Rq_task::Task_class _pcore_class;

		public:

			int set_id(int);
			int get_id();

			static std::forward_list<Pcore*> get_pcores();

			//virtual Pcore_state get_pcore_state = 0;
			//virtual float get_clockspead() = 0;

			//virtual int set_pcore_state(pcore_state) = 0;
			//virtual float set_clockspeed(float) = 0;

			Pcore();
			~Pcore();

	};

	class Arm_core : Pcore
	{

	};

	class Intel_core : Pcore
	{

	};

}

#endif /* _INCLUDE__SCHED_CONTROLLER__PCORE_H_ */