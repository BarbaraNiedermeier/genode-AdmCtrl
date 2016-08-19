/*
 * \brief  circular buffer to store tasks
 * \author Paul Nieleck
 * \date   2016/08/07
 *
 * This template class implements a circular ring buffer.
 * The buffer is an array of fixed size. There are
 * two pointers which define the position of the
 * head and the position of the tail.
 * Elements are always inserted at the tail, i.e.
 * they are enqued. Inserting an element means
 * the tail moves one position to the right.
 * Elements are always removed at the head, i.e.
 * they are enqueued. Removing an element means
 * the head moves one position to the right.
 * If one of the pointers reaches the end of the
 * array it is wrapped around.
 * The available free space in the buffer is called
 * window. The initial window size is equal to the
 * size of the array.
 * 
 *        tail      head
 *          |         |
 * ---------------------------
 * |x|x|x|x| | | | | |x|x|x|x|
 * ---------------------------
 *          ^ ^ ^ ^ ^
 *          window
 *
 * inserting (enqueue) an element y:
 *
 *          tail    head
 *            |       |
 * ---------------------------
 * |x|x|x|x|y| | | | |x|x|x|x|
 * ---------------------------
 *            ^ ^ ^ ^
 *            window
 *
 * removing (dequeue) an element y:
 *
 *          tail      head
 *            |         |
 * ---------------------------
 * |x|x|x|x|y| | | | | |x|x|x|
 * ---------------------------
 *            ^ ^ ^ ^ ^
 *            window
 *
 * Using a circular buffer writing to and
 * reading from the buffer only costs
 * O(1) time. The buffer allows bulk
 * reading of data, only the header
 * needs to be repositioned.
 */

#ifndef _INCLUDE__RQ_MANAGER__RQ_BUFFER_H_
#define _INCLUDE__RQ_MANAGER__RQ_BUFFER_H_

#include <base/printf.h>
#include <spec/arm/cpu/atomic.h> /* atomic access to int values on arm CPUs */

namespace Rq_manager
{

	template <typename T>
	class Rq_buffer
	{

		private:

			int _buf_size;                    /* size of the buffer */
			int *_lock = nullptr;             /* points to the lock for the Rq_buffer */
			int *_head = nullptr;             /* points to the element that has been enqueued first */
			int *_tail = nullptr;             /* points to the next free array index */
			int *_window = nullptr;           /* number of unallocated objects between tail and head */
			T *_buf = nullptr;                /* buffer of type T - it's an array */
			Genode::Dataspace_capability _ds; /* dataspace capability of the shared object */
			char *_ds_begin = nullptr;        /* pointer to the beginning of the shared dataspace */

			void _init_rq_buf(int);  /* helper function for enabling different constructors */

		public:

			int enq(T);      /* insert an element at the tail */
			int deq(T**);    /* deque the element at the head */
	//		T* deq_blk(int n); /* deque n elements beginning at the head */

			void init_w_shared_ds(int);                                /* helpere function for createing the Rq_buffer within a shared memory */
			Genode::Dataspace_capability get_ds_cap() { return _ds; }; /* return the dataspace capability */

			Rq_buffer();

	};

	/**************************
	 ** Function definitions **
	 **************************/

	/**
	 * Init new Rq_buffer in a shared dataspace.
	 * Must be called separately, if constructor
	 * with no arguments has been called.
	 *
	 * \param n size of the buffer
	 */
	template <typename T>
	void Rq_buffer<T>::init_w_shared_ds(int size)
	{

		/*
		 * Set the size of the buffer and calculate the memory
		 * that needs to be aquired for the shared dataspace.
		 * The shared dataspace will consist of one lock, three
		 * element-pointers for buffer positions and the actual
		 * circular buffer of type T.
		 */
		_buf_size = size;
		int ds_size = (4 * sizeof(int)) + (_buf_size * sizeof(T));

		/* 
		 * create dataspace capability, i.e. mem is allocated,
         * and attach the dataspace (the first address of the
		 * allocated mem) to _ds_begin. Then all the variables
		 * are set to the respective pointers in memory.
		 */
		_ds = Genode::env()->ram_session()->alloc(ds_size);
		_ds_begin = Genode::env()->rm_session()->attach(_ds);

		char *_lockp = _ds_begin + (0 * sizeof(int));
		char *_headp = _ds_begin + (1 * sizeof(int));
		char *_tailp = _ds_begin + (2 * sizeof(int));
		char *_windowp = _ds_begin + (3 * sizeof(int));
		char *_bufp = _ds_begin + (4 * sizeof(int));

		_lock = (int*) _lockp;
		_head = (int*) _headp;
		_tail = (int*) _tailp;
		_window = (int*) _windowp;
		_buf = (T*) _bufp;

		/* 
		 * set initial values for the lock and the element-pointers
		 * in an so far empty Rq_buffer.
		 */
		*_lock = false;
		*_head = 0;
		*_tail = 0;
		*_window = _buf_size;

	}

	/**
	 * Enque a new element at the tail pointer
	 * of the buffer.
	 *
	 * \param t any element that should be enqueued
	 *
	 * \return 0 enqueue operation successful
	 *         1 buffer full
	 *         2 buffer locked
	 */
	template <typename T>
	int Rq_buffer<T>::enq(T t)
	{

		if ( Genode::cmpxchg(_lock, false, true) ) {

			if (*_window < 1) {

				PERR("The buffer is currently full. Can't insert further elements.");
				*_lock = false;

			} else {

				_buf[*_tail] = t; /* insert element at the current free position */
				PINF("New element inserted to buffer at position %d with pointer %p", *_tail, &_buf[*_tail]);
				*_tail += 1; /* move the free position one to the right or wrap around */

				/* check if end of array has been reached */
				if (*_tail >= _buf_size) {
					*_tail = 0; /* wrap around if end of array reached */
				}

				*_window -= 1; /* window of free space got smaller, decrease it */
				*_lock = false;
				return 0;
			}

		} else {

			PWRN("Buffer locked");
			return 2;

		}

		return 1; /* buffer overflow */
	}

	/**
	 * Dequeue an element at the head pointer.
	 *
	 * \param **t pointer to the list element pointer
	 *            that is to be returned
	 *
	 * \return 0 dequeue operation successful
	 *         1 buffer empty
	 *         2 buffer locked
	 *         
	 * The function takes a pointer to a pointer
	 * as input argument. The inner pointer represents
	 * the memory address to which we want to store
	 * the memory address of the list element that
	 * is at position of _head.
	 * The head is moved one position to the right,
	 * i.e. to the next element in the list and the
	 * counter for the window-size is incremented
	 * since the space of one element was freed.
	 */
	template <typename T>
	int Rq_buffer<T>::deq(T **t)
	{

		if ( Genode::cmpxchg(_lock, false, true) ) {

			if (*_window >= _buf_size) {

				PINF("The buffer is currently empty. Nothing to dequeue.");
				*t = nullptr; /* returning null pointer so no old data is used by anyone */
				*_lock = false;
				return 1;

			} else {

				*t = &_buf[*_head]; /* save address of the element located at the head */
				*_head += 1;           /* move head one position to the right */

				/* check if end of array has been reached */
				if (*_head >= _buf_size) {
					*_head = 0; /* wrap around if end of array reached */
				}

				*_window += 1;
				*_lock = false;
				return 0;
			}
		} else {
			PWRN("Buffer locked");
			return 2;
		}

		return 1; /* buffer empty */
	}

	/*****************
	 ** Constructor **
	 *****************/

	template <typename T>
	Rq_buffer<T>::Rq_buffer()
	{

		PINF("This class must be instantiated explicitly by calling the function 'init_w_shared_ds(int size)'.");
		PINF("If this function is not executed the class will stay in an undefined state.");

	}
}

#endif /* _INCLUDE__RQ_MANAGER__RQ_BUFFER_H_ */
