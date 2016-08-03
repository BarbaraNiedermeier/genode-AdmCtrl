/*
 * \brief  sched_controller is the user land controller for the kernel schedulers
 * \author Paul Nieleck
 * \date   2016/08/02
 *
 */

#include <base/printf.h>
#include <sched_controller_session/sched_controller_session.h>
#include <base/rpc_server.h>
#include <root/component.h>
#include <cap_session/connection.h>
#include <base/sleep.h>

namespace Sched_controller {

	struct Session_component : Genode::Rpc_object<Session>
	{
		void get_init_status() {
			PDBG("sched_controller is initialized");
		}
	};

	class Root_component : public Genode::Root_component<Session_component>
	{
		protected:

			Sched_controller::SessionComponent *_create_session(const char *args)
			{
				return new(md_alloc()) Session_component();
			}

		public:

			Root_component(Genode::Rpc_entrypoint *ep,
			               Genode::Allocator *allocator)
			: Genode::Root_component<Session_component(ep, allocator)
			{
				PDBG("Creating root component");
			}

	};

}

using namespace Genode;

int main(void)
{
	Cap_connection cap;

	static Sliced_heap sliced_heap(env()->ram_session(),
	                               env()->rm_session());

	enum { STACK_SIZE = 4096 };
	static Rpc_entrypoint ep(&cap, STACK_SIZE, "sched_controller_ep");

	static Sched_controller::Root_component sched_controller_root(&ep, &sliced_heap);
	env()->parent()->announce(ep.manage(&sched_controller_root));

	sleep_forever();

	return 0;
}	
