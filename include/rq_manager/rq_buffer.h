/*
 * \brief  circular buffer to store tasks
 * \author Paul Nieleck
 * \date   2016/08/07
 *
 */

#ifndef _INCLUDE__RQ_MANAGER__RQ_BUFFER_H_
#define _INCLUDE__RQ_MANAGER__RQ_BUFFER_H_

#include <base/printf.h>

//namespace Rq_manager {
//
//	class Rq_buffer;
//
//}

/*
 * The Rq_buffer is an implementation
 * of a circular buffer of fixed size.
 * Using a circular buffer writing to and
 * reading from the buffer only costs
 * O(1) time. The buffer allows bulk
 * reading of data, only the header
 * needs to be repositioned.
 *
 */
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

	public:

		int enq(T);      /* insert an element at the tail */
		T* deq();          /* deque the head */
//		T* deq_blk(int n); /* deque n elements beginning at the head */

		Rq_buffer();
		Rq_buffer(int);

};

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

	/* Caution, this is wrong!!!: */
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

#endif /* _INCLUDE__RQ_MANAGER__RQ_BUFFER_H_ */
