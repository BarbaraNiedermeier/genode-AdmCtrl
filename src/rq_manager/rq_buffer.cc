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