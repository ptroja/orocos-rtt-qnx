/***************************************************************************
  tag: Peter Soetens  Thu Oct 10 16:16:58 CEST 2002  WriteInterface.hpp

                        WriteInterface.hpp -  description
                           -------------------
    begin                : Thu October 10 2002
    copyright            : (C) 2002 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be

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

#ifndef ORO_CORELIB_WRITEINTERFACE_HPP
#define ORO_CORELIB_WRITEINTERFACE_HPP

#include <boost/call_traits.hpp>
#include <vector>
#include "BufferBase.hpp"

namespace RTT
{ namespace base {

    /**
     * This interface describes a write interface for value types.
     * @param T The value type write into this buffer.
     */
    template<class T>
    class WriteInterface
        : public virtual BufferBase
    {
    public:
        typedef T value_t;
        typedef typename boost::call_traits<T>::param_type param_t;

        typedef BufferBase::size_type size_type;

        /**
         * Write a single value to the buffer.
         * @param item the value to write
         * @return false if the buffer is full.
         */
        virtual bool Push( param_t item) = 0;

        /**
         * Write a sequence of values to the buffer.
         * @param items the values to write
         * @return the number of values written (may be less than items.size())
         */
        virtual size_type Push( const std::vector<value_t>& items ) = 0;

        virtual ~WriteInterface()
        {}

    };

}}

#endif // WRITEINTERFACE_HPP

