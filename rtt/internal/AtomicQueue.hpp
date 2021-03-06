#ifndef ORO_CORELIB_ATOMIC_QUEUE_HPP
#define ORO_CORELIB_ATOMIC_QUEUE_HPP

#include "../os/CAS.hpp"
#include "../base/BufferPolicy.hpp"
#include <utility>

namespace RTT
{
    namespace internal {
    /**
     * Create an atomic, non-blocking single ended queue (FIFO) for storing
     * a \b pointer to \a T. It is a
     * Many Readers, Many Writers implementation
     * based on the atomic Compare And Swap instruction. Any number of threads
     * may access the queue concurrently.
     *
     * This queue tries to obey strict ordering, but under high contention
     * of reads interfering writes, one or more elements may be dequeued out of order.
     * For this reason, size() is expensive to accurately calculate the size.
     *
     * Due to the same limitations, it is possible that the full capacity of the
     * queue is not used (simulations show seldomly an off by one element if capacity==10) and
     * that isFull() returns true, while size() < capacity().
     *
     * @warning You can not store null pointers.
     * @param T The pointer type to be stored in the Queue.
     * Example : AtomicQueue< A* > is a queue of pointers to A.
     *
     * @ingroup CoreLibBuffers
     */
    template<class T, class ReadPolicy = base::NonBlockingPolicy, class WritePolicy = base::NonBlockingPolicy>
    class AtomicQueue
    {
        const int _size;
        typedef T C;
        typedef volatile C* CachePtrType;
        typedef C* volatile CacheObjType;
        typedef C  ValueType;
        typedef C* PtrType;

        union SIndexes
        {
        	unsigned long _value;
        	unsigned short _index[2];
        };

        /**
         * The pointer to the buffer can be cached,
         * the contents are volatile.
         */
        CachePtrType  _buf;

        /**
         * The indexes are packed into one double word.
         * Therefore the read and write index can be read and written atomically.
         */
        volatile SIndexes _indxes;

        WritePolicy write_policy;
        ReadPolicy read_policy;

        /**
         * The loose ordering may cause missed items in our
         * queue which are not pointed at by the read pointer.
         * This function recovers such items from _buf.
         * @return zero if nothing to recover, the location of
         * a lost item otherwise.
         */
        CachePtrType recover_r() const
        {
            // The implementation starts from the read pointer,
            // and wraps around until all fields were scanned.
            // As such, the out-of-order elements will at least
            // be returned in their relative order.
            SIndexes start;
            start._value = _indxes._value;
            unsigned short r = start._index[1];
            while( r != _size) {
                if (_buf[r])
                    return &_buf[r];
                ++r;
            }
            r = 0;
            while( r != start._index[1]) {
                if (_buf[r])
                    return &_buf[r];
                ++r;
            }
            return 0;
        }

        /**
         * Atomic advance and wrap of the Write pointer.
         * Return the old position or zero if queue is full.
         */
        CachePtrType propose_w()
        {
        	SIndexes oldval, newval;
            do {
            	oldval._value = _indxes._value; /*Points to a free writable pointer.*/
                newval._value = oldval._value; /*Points to the next writable pointer.*/
                // check for full on a *Copy* of oldval:
                if ( (newval._index[0] == newval._index[1] - 1) || (newval._index[0] == newval._index[1] + _size - 1) )
                {
                    // note: in case of high contention, there might be existing empty fields
                    // in _buf that aren't used.
                    return 0;
                }
                newval._index[0]++;
                if ( newval._index[0] >= _size )
                	newval._index[0] = 0;
                // if ptr is unchanged, replace it with newval.
            } while ( !os::CAS( &_indxes._value, oldval._value, newval._value) );

            // the returned field may contain data, in that case, the caller needs to retry.
            return &_buf[ oldval._index[0] ];
        }
        /**
         * Atomic advance and wrap of the Read pointer.
         * Return the data position or zero if queue is empty.
         */
        CachePtrType propose_r()
        {
        	SIndexes oldval, newval;
            do {
            	oldval._value = _indxes._value;
            	newval._value = oldval._value;
                // check for empty on a *Copy* of oldval:
                if ( newval._index[0] == newval._index[1] )
                {
                    // seldom: R and W are indicating empty, but 'lost' fields
                    // are to be picked up. Return these
                    // that would have been read eventually after some writes.
                    return recover_r();
                }
                newval._index[1]++;
                if ( newval._index[1] >= _size )
                	newval._index[1] = 0;

            } while ( !os::CAS( &_indxes._value, oldval._value, newval._value) );
            // the returned field may contain *no* data, in that case, the caller needs to retry.
            // as such r will advance until it hits a data sample or write pointer.
            return &_buf[oldval._index[1] ];
        }

        // non-copyable !
        AtomicQueue( const AtomicQueue<T>& );
    public:
        typedef unsigned int size_type;

        /**
         * Create an AtomicQueue with queue size \a size.
         * @param size The size of the queue, should be 1 or greater.
         */
        AtomicQueue( unsigned int size )
            : _size(size+1), write_policy(size), read_policy(0)
        {
            _buf= new C[_size];
            this->clear();
        }

        ~AtomicQueue()
        {
            delete[] _buf;
        }

        /**
         * Inspect if the Queue is full.
         * @return true if full, false otherwise.
         */
        bool isFull() const
        {
            // two cases where the queue is full :
            // if wptr is one behind rptr or if wptr is at end
            // and rptr at beginning.
            return _indxes._index[0] == _indxes._index[1] - 1 || _indxes._index[0] == _indxes._index[1] + _size - 1;
        }

        /**
         * Inspect if the Queue is empty.
         * @return true if empty, false otherwise.
         */
        bool isEmpty() const
        {
            // empty if nothing to read.
            return _indxes._index[0] == _indxes._index[1] && recover_r() == 0;
        }

        /**
         * Return the maximum number of items this queue can contain.
         */
        size_type capacity() const
        {
            return _size -1;
        }

        /**
         * Return the exact number of elements in the queue.
         * This is slow because it scans the whole
         * queue.
         */
        size_type size() const
        {
            int c = 0, ret = 0;
            while (c != _size ) {
                if (_buf[c++] )
                    ++ret;
            }
            return ret;
            //int c = (_indxes._index[0] - _indxes._index[1]);
            //return c >= 0 ? c : c + _size;
        }

        /**
         * Enqueue an item.
         * @param value The value to enqueue, not zero.
         * @return false if queue is full or value is zero, true if queued.
         */
        bool enqueue(const T& value)
        {
            if ( value == 0 )
                return false;
            write_policy.pop();
            CachePtrType loc;
            C null = 0;
            do {
                loc = propose_w();
                if ( loc == 0 )
                    return false; //full
                // if loc contains a zero, write it, otherwise, re-try.
            } while( !os::CAS(loc, null, value));
            read_policy.push();
            return true;
        }

        /**
         * Dequeue an item.
         * @param value The value dequeued.
         * @return false if queue is empty, true if dequeued.
         */
        bool dequeue( T& result )
        {
            read_policy.pop();
            CachePtrType loc;
            C null = 0;
            do {
                loc = propose_r();
                if ( loc == 0 )
                    return false; // empty
                result = *loc;
                // if loc still contains result, clear it, otherwise, re-try.
            } while( result == 0 || !os::CAS(loc, result, null) );
            write_policy.push();
            assert(result);
            return true;
        }

        /**
         * Return the next to be read value.
         */
        const T front() const
        {
            return _buf[_indxes._index[1] ];
        }

        /**
         * Return the next to be read value and lock
         * it in a MemoryPool, such that it is not freed.
         * The returned pointer must be unlock()'ed by the
         * user's code.
         */
        template<class MPoolType>
        T lockfront(MPoolType& mp) const
        {
            CachePtrType loc=0;
            bool was_locked = false;
            do {
                if (was_locked)
                    mp.unlock(*loc);
                loc = &_buf[_indxes._index[1] ];
                if (*loc == 0)
                    return 0;
                was_locked = mp.lock(*loc);
                // retry if lock failed or read moved.
            } while( !was_locked || loc != &_buf[_indxes._index[1] ] ); // obstruction detection.
            return *loc;
        }

        /**
         * Clear all contents of the Queue and thus make it empty.
         */
        void clear()
        {
            for(int i = 0 ; i != _size; ++i) {
                _buf[i] = 0;
            }
            _indxes._value = 0;
            write_policy.reset( _size - 1 );
            read_policy.reset(0);
        }

    };

}}

#endif
