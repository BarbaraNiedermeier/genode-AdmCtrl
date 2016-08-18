/*
 * \brief  simple client to test shared mem with Rq_manager
 * \author Paul Nieleck
 * \date   2016/08/17
 *
 * This component creates a new connection to the Rq_manager.
 * It then requests a dataspace capability from the Rq_manager
 * where the shared dataspace for one run queue can be found.
 */

#include <base/env.h>
#include <base/printf.h>
#include "rq_manager_session/client.h"
#include "rq_manager_session/connection.h"
#include "rq_manager/rq_buffer.h"

using namespace Genode;

int main()
{
	Rq_manager::Connection rqm;
	char *_rqbufp = nullptr;    /* char* because char* is only one byte, making addressing easier */
	int *head = nullptr;
	int *tail = nullptr;
	int *window = nullptr;
	Rq_manager::Rq_task *buf = nullptr;
	Dataspace_capability dsc;

	/* get dataspace capability for shared dataspace of run queue of core 0 */
	dsc = rqm.get_core_rq_ds(0);
	PINF("Got Dataspace_capability :)");

	/* 
	 * Attach dataspace capability to the _rqbufp pointer, i.e.
	 * the shared memory starts at the pointer position _rqbufp.
	 * Every shared memory area consists of the following parts,
	 * starting at the lowest address:
	 * - int head: the array position of the oldest element of
	 *             the rq_buffer - see rq_buffer.h
	 * - int tail: the array position of the next free position
	 *             in the rq_buffer - see rq_buffer.h
	 * - int window: the number of free array positions - see
	 *               rq_buffer.h
	 * - Rq_task: the actual array of tasks
	 */
	_rqbufp = env()->rm_session()->attach(dsc);
	PINF("Dataspace_capability successfully attached :D");
	PINF("Address of the buffer is %p", &buf);

	/* 
	 * need to set the pointers according to the correct
	 * positions. Must be void pointers, since _rqbufp is
	 * of type char*
	 */
	void *_headp = _rqbufp + (0 * sizeof(int));
	void *_tailp = _rqbufp + (1 * sizeof(int));
	void *_windowp = _rqbufp + (2 * sizeof(int));
	void *_bufp = _rqbufp + (3 * sizeof(int));

	/* cast the pointers to the correct type */
	head = (int*) _headp;
	tail = (int*) _tailp;
	window = (int*) _windowp;
	buf = (Rq_manager::Rq_task*) _bufp;

	PINF("The tail pointer points to %d", *tail);
	PINF("The head pointer points to %d", *head);
	PINF("The window size is %d", *window);

	/* copy content of buf[3] to variable task */
	Rq_manager::Rq_task task = buf[3];
	PINF("Got task with task_id: %d, wcet: %d, valid: %d", task.task_id, task.wcet, task.valid);

	/* access content of buf[4] directly */
	PINF("Got task with task_id: %d, wcet: %d, valid: %d", buf[4].task_id, buf[4].wcet, buf[4].valid);

	return 0;
}