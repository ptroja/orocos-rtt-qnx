/***************************************************************************
  tag: Peter Soetens  Wed Jan 18 14:11:39 CET 2006  Handle.cxx 

                        Handle.cxx -  description
                           -------------------
    begin                : Wed January 18 2006
    copyright            : (C) 2006 Peter Soetens
    email                : peter.soetens@mech.kuleuven.be
 
 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/
 
 
#include "rtt/Handle.hpp"


namespace RTT
{

    Handle::Handle()
        : m_conn(0)
    {
    }

    Handle::Handle(connection_t conn)
        : m_conn(conn)
    {
    }

    Handle::Handle(const Handle& hs)
        : m_conn( hs.m_conn )
    {
    } 

    Handle::~Handle()
    {
    }

    bool Handle::connect()
    {
        if ( connected() )
            return true;
        return m_conn && m_conn->connect();
    }

    bool Handle::disconnect()
    {
        if ( !connected() )
            return true;
        return m_conn->disconnect();
    }

    bool Handle::connected() const {
        return m_conn && m_conn->connected();
    }

    Handle::operator bool() const {
        return m_conn;
    }

    bool Handle::ready() const {
        return m_conn;
    }


    ScopedHandle::ScopedHandle(const Handle& hs)
        : Handle( hs )
    {
    } 


    ScopedHandle::~ScopedHandle()
    {
        this->disconnect();
    }
    CleanupHandle::CleanupHandle(const Handle& hs)
        : Handle( hs )
    {
    } 

    CleanupHandle::~CleanupHandle()
    {
        if (m_conn)
            m_conn->destroy();
    }
}

#if 0
namespace RTT
{
    Handle::Handle( const sigslot::Handle & c,
                    const sigslot::Handle & c2 )
        : _c(c), _c2(c2) {}
    Handle::Handle( const sigslot::Handle & c ) : _c(c), _c2(c) {}
    Handle::Handle( const Handle& h ) : _c(h._c), _c2(h._c2) {}
    Handle::Handle() {}
    Handle::~Handle() {}

#if 0
    Handle& Handle::operator=(const Handle& h) {
        _c = h._c;
        _c2 = h._c2;
        return *this;
    }

    bool Handle::operator==(const Handle& h) const {
        return (_c == h._c) && (_c2 == h._c2);
    }

    bool Handle::operator<(const Handle& h) const {
        return (_c < h._c) && (_c2 < h._c2);
    }
#endif

    bool Handle::connected() const {
        return _c.connected() && _c2.connected();
    }

    bool Handle::disconnect() {
        return _c.disconnect() &&
            _c2.disconnect();
    }

    bool Handle::connect() {
        return _c.connect() &&
            _c2.connect();
    }

    Handle::operator bool() const {
        return _c && _c2;
    }

    bool Handle::ready() const {
        return _c && _c2;
    }


}
#endif