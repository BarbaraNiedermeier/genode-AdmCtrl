/*
 * \brief  This component provides an interface between Controller and Sync
 * \author Paul Nieleck
 * \date   2016/08/07
 *
 */

/* global includes */
#include <base/env.h>
#include <base/printf.h>
#include <base/sleep.h>
#include <base/rpc_server.h>
#include <cap_session/connection.h>
#include <root/component.h>

/* local includes */
#include "rq_manager/rq_manager.h"
#include "rq_manager/rq_buffer.h"
#include "rq_manager/rq_task.h"
#include "rq_manager_session/rq_manager_session.h"


namespace Rq_manager
{

	struct Session_component : Genode::Rpc_object<Session>
	{

		private:

			Rq_manager *_rqmanager = nullptr;

		public:

			int enq(int core, Rq_task task)
			{
				// here must go some real function
				// return Rq_manager::enq(core, task)
				return 0;
			}

//			int deq(int core, Rq_task **task)
//			{
//				// here must go some real function
//				// return Rq_manager::deq(core, taskp)
//				return 0;
//			}

			Genode::Dataspace_capability get_core_rq_ds(int core)
			{
				return _rqmanager->get_core_rq_ds(core);
			}

			/* Standard Constructor enhanced by Rq_manager object */
			Session_component(Rq_manager *rqmgmt)
			: Genode::Rpc_object<Session>()
			{
				_rqmanager = rqmgmt;
			}

	};

	class Root_component : public Genode::Root_component<Session_component>
	{

		private:

			Rq_manager *_rqmanager = nullptr;

		protected:

			Session_component *_create_session(const char *args)
			{
				return new (md_alloc()) Session_component(_rqmanager);
			}

		public:

			/* Enhanced constructor to take Rq_manager object */
			Root_component(Genode::Rpc_entrypoint *ep,
			               Genode::Allocator *allocator,
			               Rq_manager *rqmgmt)
			: Genode::Root_component<Session_component>(ep, allocator)
			{
				_rqmanager = rqmgmt;
			}
	};

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


	/****************
     ** Connection **
	 ****************/

	/*
	 * Get a session for the parent's capability service, so that we
	 * are able to create capabilities.
	 */
	Cap_connection cap;

	/*
	 * A sliced heap is used for allocating session objects - thereby we
	 * can release objects separately.
	 */
	static Sliced_heap sliced_heap(env()->ram_session(),
	                               env()->rm_session());

	/*
	 * Create objects for use by the framework.
	 *
	 * An 'Rpc_entrypoint' is created to announce our service's root
	 * capability to our parent, manage incoming session creation
	 * requests, and dispatch the session interface. The incoming RPC
	 * requests are dispatched via a dedicated thread. The 'STACK_SIZE'
	 * argument defines the size of the thread's stack. The additional
	 * string argument is the name of the entry point, used for
	 * debugging purposes only.
	 */
	enum { STACK_SIZE = 4096 };
	static Rpc_entrypoint ep(&cap, STACK_SIZE, "rq_manager_ep");

	static Rq_manager::Root_component rq_manager_root(&ep, &sliced_heap, &mgmt);
	env()->parent()->announce(ep.manage(&rq_manager_root));

	/* We are done with this and only act upon client requests now. */
	sleep_forever();

	return 0;
}
