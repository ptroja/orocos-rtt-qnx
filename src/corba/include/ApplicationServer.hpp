/***************************************************************************
  tag: Peter Soetens  Mon Jun 26 13:25:58 CEST 2006  ApplicationServer.hpp 

                        ApplicationServer.hpp -  description
                           -------------------
    begin                : Mon June 26 2006
    copyright            : (C) 2006 Peter Soetens
    email                : peter.soetens@fmtc.be
 
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
 
 
#ifndef ORO_APPLICATION_SERVER_HPP
#define ORO_APPLICATION_SERVER_HPP

#include <tao/corba.h>
#include <tao/PortableServer/PortableServer.h>

namespace RTT
{namespace Corba
{
    /**
     * A class which an provides ORB to
     * the application process.
     */
    struct ApplicationServer
    {
        /**
         * The orb of this process.
         */
        static CORBA::ORB_var orb;

        /**
         * The root POA of this process.
         */
        static PortableServer::POA_var rootPOA;

    };
}}

#endif