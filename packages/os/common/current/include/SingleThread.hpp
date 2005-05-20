/***************************************************************************
  tag: Peter Soetens  Thu Apr 22 20:40:53 CEST 2004  SingleThread.hpp 

                        SingleThread.hpp -  description
                           -------------------
    begin                : Thu April 22 2004
    copyright            : (C) 2004 Peter Soetens
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
 

#ifndef SINGLE_THREAD_HPP
#define SINGLE_THREAD_HPP

// Our own package config headers.
#include "pkgconf/os.h"
#include <os/fosi.h>

#include <os/RunnableInterface.hpp>
#include <os/ThreadInterface.hpp>

#include <string>

namespace ORO_OS
{
    /**
     * This Thread abstraction class represents
     * a single-shot thread which can be started many times
     * and stops each time the loop() function returns.
     * @see RunnableInterface
     */
    class SingleThread 
        : public ThreadInterface
    {
        friend void* singleThread_f( void* t );

    public:
        /**
         * Create a single-shot Thread with priority \a priority, a \a name and optionally,
         * an object to execute.
         */
        SingleThread(int priority, const std::string& name, RunnableInterface* r=0);
    
        virtual ~SingleThread();

        virtual bool run( RunnableInterface* r);

        /**
         * Start the thread
         *
         * @return true if successfull.
         */
        virtual bool start();

        /**
         * Stop the thread. The return value of stop, is the
         * same as the return value of RunnableInterface::breakLoop().
         *
         * @return true if successfull.
         */
        virtual bool stop();

        /**
         * Returns whether the thread is running
         */
        virtual bool isRunning() const;

        /**
         * Set the name of this task
         */
        void setName(const char*);

        /**
         * Read the name of this task
         */
        virtual const char* getName() const;

        virtual bool makeHardRealtime() 
        { 
            // This construct is so because
            // the thread itself must call the proper RTAI function.
            if ( !running ) 
                {
                    goRealtime = true; 
                }
            return goRealtime; 
        }

        virtual bool makeSoftRealtime()
        { 
            if ( !running ) 
                {
                    goRealtime = false; 
                }
            return !goRealtime; 
        }

        virtual bool isHardRealtime() const;

        virtual Seconds getPeriod() const { return 0.0; }
        virtual nsecs getPeriodNS() const { return 0; }

        virtual int getPriority() const { return priority; }
    protected:

        virtual bool breakLoop();

        virtual void loop();
    
        virtual bool initialize();

        virtual void finalize();

    private:
        /**
         * When set to 1, the thread will run, when set to 0
         * the thread will stop
         */
        bool running;

        /**
         * True when the thread should go realtime.
         */
        bool goRealtime;

        /**
         * Signal the thread that it should exit.
         */
        bool prepareForExit;

        /**
         * The realtime task structure created by this object.
         */
        RTOS_TASK* rtos_task;

        /**
         * The userspace thread carying the rtos_task.
         */
        pthread_t thread;

        /**
         * The priority of the created thread.
         */
        int priority;

        /**
         * The unique name of this task.
         */
        std::string taskName;

        /**
         * The semaphore used for starting the thread.
         */
        rt_sem_t sem;

        /**
         * The semaphore used for communicating between
         * the thread and the constructor/destructor.
         */
        rt_sem_t confDone;

        /**
         * The possible Runnable to run in this Component
         */
        RunnableInterface* runComp;
    };

}

#endif