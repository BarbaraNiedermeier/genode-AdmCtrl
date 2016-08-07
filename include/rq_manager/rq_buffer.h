/*
 * \brief  circular buffer to store tasks
 * \author Paul Nieleck
 * \date   2016/08/07
 *
 */

#ifndef _INCLUDE__RQ_MANAGER__RQ_BUFFER_H_
#define _INCLUDE__RQ_MANAGER__RQ_BUFFER_H_

namespace Rq_buffer {

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
        template<typename T>
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
        
                        int enq(T t);      /* insert an element at the tail */
                        T* deq();          /* deque the head */
//                        T* deq_blk(int n); /* deque n elements beginning at the head */
        
                        Rq_buffer();
                        Rq_buffer(int size);
        
        };
}

#endif /* _INCLUDE__RQ_MANAGER__RQ_BUFFER_H_ */
