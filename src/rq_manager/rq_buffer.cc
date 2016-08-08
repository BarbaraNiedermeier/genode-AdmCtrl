/*
 * \brief  circular buffer implementation for threads
 * \author Paul Nieleck
 * \date   2016/08/07
 *
 * This class implements a circular ring buffer.
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
 */

#include "rq_manager/rq_buffer.h"
#include <base/printf.h>

//using namespace Rq_manager;

template <typename T>
int Rq_buffer<T>::enq(T t)
{

	if (_window < 1) {

		PERR("The buffer is currently full. Can't insert further elements.");

	} else {

		if (_tail >= _buf_size - 1) {

			_tail = 0; /* wrap around if end of array reached */

		} else {

			_tail++; /* move tail to the right, if element is inserted */

		}

		_window--;
		_buf[_tail] = t;

		PINF("New element inserted to buffer");

		return 0;
	}

	return -1; /* buffer overflow */
}

template <typename T>
T* Rq_buffer<T>::deq()
{

	if (_window >= _buf_size) {

		PINF("The buffer is currently empty. Nothing to dequeue.");

	} else {

		int _current_head = _head;

		if (_head >= _buf_size - 1) {

			_head = 0; /* wrap around if end of array reached */

		} else {

			_head++; /* move head to the right if element is removed */

		}

		_window++;
		return _buf[_current_head];

	}

// Caution, this is wrong!!!:
	return _buf[0]; /* buffer empty */
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
