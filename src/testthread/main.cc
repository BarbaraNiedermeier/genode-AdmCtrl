 /*
  * \brief  Create threads and check their scheduling options
  * \author Paul Nieleck <paul.nieleck@tum.de>
  * \date   2016-06-16
  *
  * This program creates some threads and checks for their
  * specific scheduling values.
  * Purpose is to find out how genode handels the creation
  * of new threads and how we can access this facilities.*
  */


/* Genode includes */
#include <base/printf.h>
#include <base/thread.h>
//#include <base/env.h>
//#include <base/rpc_client.h>
//#include <cpu_session/cpu_session.h>
//#include <cpu_session/client.h>
//#include <parent/parent.h>


using namespace Genode;


class Mythread : public Genode::Thread<2*4096>
{
	public:

		Mythread() : Thread("My Thread") { }

		void func()
		{
			PINF("I am a thread!\n");
		}

		void entry()
		{
			func();
		}

};

class Thread_creator
{
	public:

		int create_thread(int);
};


int Thread_creator::create_thread(int thread_num)
{
	printf("Now we will create thread %i.\n", thread_num);
	Mythread myt;
	myt.start();

	Genode::Thread_capability mycap = myt.cap();
	PINF("Got Thread capability information.\n");

	myt.join();
	return 0;
}


int main()
{
	PDBG("Hello World! I am a process\n");
	printf("And this is how normal printf looks like...\n");
	Thread_creator t;
	for (int i = 0; i < 5; i++) {
		t.create_thread(i);
	}
	return 0;
}
