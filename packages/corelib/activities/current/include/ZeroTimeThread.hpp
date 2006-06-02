/***************************************************************************
 tag: Peter Soetens  Wed Apr 17 16:01:36 CEST 2002  ZeroTimeThread.h 

                       ZeroTimeThread.h -  description
                          -------------------
   begin                : Wed April 17 2002
   copyright            : (C) 2002 Peter Soetens
   email                : peter.soetens@mech.kuleuven.ac.be

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


#ifndef ZEROTIMETHREAD_HPP
#define ZEROTIMETHREAD_HPP

#include "TimerThread.hpp"


namespace ORO_CoreLib
{

    class NonPreemptibleActivity;
    /**
     * @brief This thread is the fastest, non preemptible thread in the
     * Orocos realtime system.
     *
     * It can not be interrupted by another thread
     * and executes its functionality as fast as possible on the current
     * CPU, in a short (zero) timespan.
     *
     * It Uses the Singleton pattern, since there will be only one EVER.
     *
     * @see PeriodicThread
     */
    class ZeroTimeThread
        : public TimerThread
    {
    public:
        static TimerThreadPtr Instance();

        /**
         * Releases the ZeroTimeThread
         * Reference counting might aid in making this call safe
         *
         * @return true on success, false on failure
         */
        static bool Release();

        /**
         * Destructor
         */
        virtual ~ZeroTimeThread();

    protected:

        /**
         * Constructor
         */
        ZeroTimeThread();

    private:

        /**
         * Our only instance of the ZeroTimeThread
         */
        static TimerThreadPtr _instance;

    };
} // namespace ORO_CoreLib

#endif