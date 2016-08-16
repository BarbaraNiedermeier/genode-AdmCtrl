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

namespace Rq_manager
{

	template <typename T>
	class Rq_buffer
	{

		private:
			
			static const int _DEFAULT_SIZE = 100;
			int _buf_size;                    /* size of the buffer */
			int _head;                        /* points to the element that has been enqueued first */
			int _tail;                        /* points to the next free array index */
			int _window;                      /* number of unallocated objects between tail and head */
			T *_buf = nullptr;                /* buffer of type T - it's an array */
			Genode::Dataspace_capability _ds; /* dataspace capability of the shared object */

			void _init_rq_buf(int);  /* helper function for enabling different constructors */

			bool _lock = false;                   /* mutual exclusion state */
			bool _check_lock() { return _lock; }; /* returns the lock state */
			void _set_lock() { _lock = true; };
			void _unset_lock() { _lock = false; };

		public:

			int enq(T);      /* insert an element at the tail */
			int deq(T**);    /* deque the element at the head */
	//		T* deq_blk(int n); /* deque n elements beginning at the head */

			void init_w_shared_ds(int);                                /* helpere function for createing the Rq_buffer within a shared memory */
			Genode::Dataspace_capability get_ds_cap() { return _ds; }; /* return the dataspace capability */

			Rq_buffer();
			Rq_buffer(int);

	};

	/**
	 * Init variables according to constructor.
	 * Only executed if constructor with arguments
	 * had been called.
	 *
	 * \param n size of the buffer
	 */
	template <typename T>
	void Rq_buffer<T>::_init_rq_buf(int size)
	{

		_buf_size = size;
		_buf = new T[_buf_size]; /* create a new array of size _buf_size */

		_head = 0;
		_tail = 0;
		_window = _buf_size;
	}

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

		_buf_size = size;
		int ds_size = sizeof(T) * _buf_size;

		/* 
		 * create dataspace capability, i.e. mem is allocated,
         * and attach the dataspace (the first address of the
		 * allocated mem) to _buf
		 */
		_ds = Genode::env()->ram_session()->alloc(ds_size);
		_buf = Genode::env()->rm_session()->attach(_ds);

		_head = 0;
		_tail = 0;
		_window = _buf_size;

		Genode::printf("New dataspace capability created and attached to address %p.\n", _buf);
		Genode::printf("The last element of this dataspace is located at address %p.\n", &_buf[_buf_size - 1]);

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

		if (_check_lock()) {

			PWRN("Buffer locked");
			return 2;

		} else {

			_set_lock();

			if (_window < 1) {

				PERR("The buffer is currently full. Can't insert further elements.");
				_unset_lock();

			} else {

				_buf[_tail] = t; /* insert element at the current free position */
				PINF("New element inserted to buffer at position %d with pointer %p", _tail, &_buf[_tail]);
				_tail++; /* move the free position one to the right or wrap around */

				/* check if end of array has been reached */
				if (_tail >= _buf_size) {
					_tail = 0; /* wrap around if end of array reached */
				}

				_window--; /* window of free space got smaller, decrease it */
				_unset_lock();
				return 0;

			}

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

		if (_check_lock()) {

			PWRN("Buffer locked");
			return 2;

		} else {

			_set_lock();
		
			if (_window >= _buf_size) {

				PINF("The buffer is currently empty. Nothing to dequeue.");
				*t = nullptr; /* returning null pointer so no old data is used by anyone */
				_unset_lock();
				return 1;

			} else {

				*t = &_buf[_head]; /* save address of the element located at the head */
				_head++;           /* move head one position to the right */

				/* check if end of array has been reached */
				if (_head >= _buf_size) {
					_head = 0; /* wrap around if end of array reached */
				}

				_window++;
				_unset_lock();
				return 0;
			}
		}

		return 1; /* buffer empty */
	}

	template <typename T>
	Rq_buffer<T>::Rq_buffer()
	{

		PINF("Constructor called without arguments. Please allocate dataspace by calling init_w_shared_ds(int size).");

	}

	template <typename T>
	Rq_buffer<T>::Rq_buffer(int size)
	{

		_init_rq_buf(size);

		PINF("Buffer created, size is %d", _buf_size);
	}
}

#endif /* _INCLUDE__RQ_MANAGER__RQ_BUFFER_H_ */
