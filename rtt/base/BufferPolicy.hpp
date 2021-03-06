/***************************************************************************
  tag: Peter Soetens  Wed Jan 18 14:11:39 CET 2006  BufferPolicy.hpp

                        BufferPolicy.hpp -  description
                           -------------------
    begin                : Wed January 18 2006
    copyright            : (C) 2006 Peter Soetens
    email                : peter.soetens@mech.kuleuven.be

 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public                   *
 *   License as published by the Free Software Foundation;                 *
 *   version 2 of the License.                                             *
 *                                                                         *
 *   As a special exception, you may use this file as part of a free       *
 *   software library without restriction.  Specifically, if other files   *
 *   instantiate templates or use macros or inline functions from this     *
 *   file, or you compile this file and link it with other files to        *
 *   produce an executable, this file does not by itself cause the         *
 *   resulting executable to be covered by the GNU General Public          *
 *   License.  This exception does not however invalidate any other        *
 *   reasons why the executable file might be covered by the GNU General   *
 *   Public License.                                                       *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public             *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/


#ifndef ORO_CORELIB_BUFFER_POLICY_HPP
#define ORO_CORELIB_BUFFER_POLICY_HPP
#include "../os/Semaphore.hpp"

namespace RTT
{ namespace base {


    /**
     * Use this policy to indicate that you \b do \b not want
     * to block on an \a empty or \a full buffer, queue, fifo,...
     */
    struct NonBlockingPolicy
    {
        NonBlockingPolicy(unsigned int ) {}
        void push(int /*i*/ = 1 ) {
        }
        void pop(int /*i*/ = 1 ) {
        }
        void reset( int /*i*/ ) {
        }
    };

    /**
     * Use this policy to indicate that you \b do want
     * to block on an \a empty or \a full buffer, queue, fifo,...
     */
    struct BlockingPolicy
    {
        BlockingPolicy(unsigned int c) : count(c) {}
        void push(int add = 1) {
            while (add-- > 0) {
                count.signal();
            }
        }
        void pop(int sub = 1) {
            while (sub-- > 0) {
                count.wait();
            }
        }
#ifndef OROPKG_OS_MACOSX
        void reset( int c ) {
            while( c > count.value() )
                count.signal();
            while( c < count.value() )
                count.wait();
        }
#endif
    private:
        os::Semaphore count;
    };
}}
#endif
