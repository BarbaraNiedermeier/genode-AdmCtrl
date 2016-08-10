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
 *      tail        head
 *        |           |
 * ---------------------------
 * |x|x|x|x| | | | | |x|x|x|x|
 * ---------------------------
 *          ^ ^ ^ ^ ^
 *          window
 *
 * inserting (enqueue) an element y:
 *
 *        tail      head
 *          |         |
 * ---------------------------
 * |x|x|x|x|y| | | | |x|x|x|x|
 * ---------------------------
 *            ^ ^ ^ ^
 *            window
 *
 * removing (dequeue) an element y:
 *
 *        tail        head
 *          |           |
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

template <typename T>
class Rq_buffer
{

	private:
        
		static const int _DEFAULT_SIZE = 100;
		int _buf_size;
		int _head;
		int _tail;
		int _window; /* number of unallocated objects between tail and head */
		T *_buf;

		bool _lock = false;
		bool _check_lock() { return _lock; };
		void _set_lock() { _lock = true; };
		void _unset_lock() { _lock = false; };

	public:

		int enq(T);      /* insert an element at the tail */
		int deq(T*);      /* deque the head */
//		T* deq_blk(int n); /* deque n elements beginning at the head */

		Rq_buffer();
		Rq_buffer(int);

};

/**
 * Enque a new element at the tail pointer
 * of the buffer.
 *
 * \param t any element that should be enqueued
 *
 * \return  0 enqueue operation successful
 *         -1 buffer full
 *         -2 buffer locked
 */
template <typename T>
int Rq_buffer<T>::enq(T t)
{

	if (_check_lock()) {

		PWRN("Buffer locked");
		return -2;

	} else {

		_set_lock();

		if (_window < 1) {

			PERR("The buffer is currently full. Can't insert further elements.");
			_unset_lock();

		} else {

			if (_tail >= _buf_size - 1) {

				_tail = 0; /* wrap around if end of array reached */

			} else {

				_tail++; /* move tail to the right, if element is inserted */

			}

			_window--;
			_buf[_tail] = t;

			PINF("New element inserted to buffer");

			_unset_lock();
			return 0;

		}

	}

	return -1; /* buffer overflow */
}

/**
 * Dequeue an element at the head pointer
 * of the buffer.
 *
 * \param *t pointer to the list element
 *           that is to be returned
 *
 * \return  0 dequeue operation successful
 *         -1 buffer empty
 *         -2 buffer locked
 *         
 */
template <typename T>
int Rq_buffer<T>::deq(T *t)
{

	if (_check_lock()) {

		PWRN("Buffer locked");
		return -2;

	} else {

		_set_lock();
	
		if (_window >= _buf_size) {

			PINF("The buffer is currently empty. Nothing to dequeue.");
			_unset_lock();
			return -1;

		} else {

			int _current_head = _head;

			if (_head >= _buf_size - 1) {

				_head = 0; /* wrap around if end of array reached */

			} else {

				_head++; /* move head to the right if element is removed */	

			}

			_window++;
			t =  &_buf[_current_head];

			_unset_lock();
			return 0;
		}
	}

	return -1; /* buffer empty */
}

template <typename T>
Rq_buffer<T>::Rq_buffer()
{
	PINF("Constructor called without arguments. Creating buffer of default size.");

	Rq_buffer<T>::Rq_buffer(_DEFAULT_SIZE);
}

template <typename T>
Rq_buffer<T>::Rq_buffer(int size)
{
	_buf_size = size;
	_buf = new T[_buf_size]; /* create a new array of size _buf_size */

	_head = 0;
	_tail = 0;
	_window = _buf_size;

	PINF("Buffer created, size is %d", _buf_size);
}

#endif /* _INCLUDE__RQ_MANAGER__RQ_BUFFER_H_ */
