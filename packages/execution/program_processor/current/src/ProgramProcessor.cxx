/***************************************************************************
  tag: Peter Soetens  Wed Jan 18 14:11:40 CET 2006  ProgramProcessor.cxx 

                        ProgramProcessor.cxx -  description
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
 
 

#include "execution/ProgramProcessor.hpp"
#include "execution/ProgramInterface.hpp"
#include <corelib/AtomicQueue.hpp>
#include <corelib/Logger.hpp>

#include <boost/bind.hpp>
#include <functional>
#include <os/Semaphore.hpp>
#include <iostream>

namespace ORO_Execution
{
    using namespace boost;
    using namespace std;
    using namespace ORO_CoreLib;

    ProgramProcessor::ProgramProcessor(int f_queue_size, ORO_OS::Semaphore* work_sem)
        : programs( new ProgMap(4) ),
          funcs( f_queue_size ),
          f_queue( new ORO_CoreLib::AtomicQueue<ProgramInterface*>(f_queue_size) ),
          f_queue_sem( work_sem )
    {
    }

    ProgramProcessor::~ProgramProcessor()
    {
        while ( !programs->empty() ) {
            Logger::log() << Logger::Info << "ProgramProcessor deletes Program "<< programs->front()->getName() << "..."<<Logger::endl;
            programs->front()->stop();
            this->unloadProgram( programs->front()->getName() );
        }

        delete programs;
        delete f_queue;
    }

     ProgramInterface::Status::ProgramStatus ProgramProcessor::getProgramStatus(const std::string& name) const
     {
         ProgramInterfacePtr pip = programs->find_if( bind(equal_to<string>(), name, bind(&ProgramInterface::getName, _1)) );

         if ( pip )
             return pip->getStatus();
         return Status::unloaded;
     }

     std::string ProgramProcessor::getProgramStatusStr(const std::string& name) const
     {
        switch ( getProgramStatus( name ))
            {
            case Status::unloaded:
                return "unloaded";
                break;
            case Status::stopped:
                return "stopped";
                break;
            case Status::running:
                return "running";
                break;
            case Status::paused:
                return "paused";
                break;
            case Status::error:
                return "error";
                break;
            }
        return "na";
     }

	bool ProgramProcessor::loadProgram(ProgramInterfacePtr pi)
    {
        ProgramInterfacePtr pip = programs->find_if( boost::bind(equal_to<string>(), pi->getName(), boost::bind(&ProgramInterface::getName, _1)) );
        if ( pip )
            return false;
        programs->grow();
        programs->append( pi );
        pi->setProgramProcessor(this);
        pi->reset();
        return true;
    }

	bool ProgramProcessor::deleteProgram(const std::string& name)
    {
        return this->unloadProgram(name);
    }

	bool ProgramProcessor::unloadProgram(const std::string& name)
    {
        ProgramInterfacePtr pip = programs->find_if( bind(equal_to<string>(), name, bind(&ProgramInterface::getName, _1)) );

        if ( pip && pip->isStopped() )
            {
                pip->setProgramProcessor(0);
                programs->erase(pip);
                programs->shrink();
                return true;
            }
        if ( !pip ) {
                std::string error(
                                  "Could not unload Program \"" + name +
                                  "\" with the processor. It does not exist." );
                throw program_unload_exception( error );
            }
        if ( !pip->isStopped() ) {
                std::string error(
                                  "Could not unload Program \"" + name +
                                  "\" with the processor. It is still running." );
                throw program_unload_exception( error );
            }
        return false;
    }

    bool ProgramProcessor::initialize()
    {
        f_queue->clear();
        return true;
    }

    void ProgramProcessor::finalize()
    {
        // stop all programs.
        programs->apply( boost::bind(&ProgramInterface::stop, _1));
    }

	void ProgramProcessor::step()
    {

        //Execute all normal programs->
        programs->apply( boost::bind(&ProgramInterface::execute, _1 ) );

        // Execute any Function :
        {
            // 0. find an empty spot to store :
            ProgramInterface* foo = 0;
            std::vector<ProgramInterface*>::iterator f_it = find(funcs.begin(), funcs.end(), foo );
            // 1. Fetch new ones from queue.
            while ( !f_queue->isEmpty() && f_it != funcs.end() ) {
                f_queue->dequeue( foo );
                // decrement semaphore if looping, but be sure not to block in any case
                if ( f_queue_sem )
                    f_queue_sem->trywait();
                *f_it = foo;
                // stored successfully, now reset for next item from queue.
                foo = 0;
                f_it = find(f_it, funcs.end(), foo );
            }
            // 2. execute all present
            for(std::vector<ProgramInterface*>::iterator it = funcs.begin();
                it != funcs.end(); ++it )
                if ( *it )
                    if ( (*it)->isStopped() || (*it)->inError() ){
                        (*it)->setProgramProcessor(0);
                        (*it) = 0;
                    } else
                        (*it)->execute();
        }
    }

    bool ProgramProcessor::runFunction( ProgramInterface* f )
    {
        if (this->getTask() && this->getTask()->isRunning() && f) {
            f->setProgramProcessor(this);
            if ( f->start() == false)
                return false;
            int result = f_queue->enqueue( f );
            if ( f_queue_sem )
                f_queue_sem->signal();
            return result != 0;
        }
        return false;
    }

    bool ProgramProcessor::removeFunction( ProgramInterface* f )
    {
        std::vector<ProgramInterface*>::iterator f_it = find(funcs.begin(), funcs.end(), f );
        if ( f_it != funcs.end() ) {
            f->setProgramProcessor(0);
            *f_it = 0;
            return true;
        }
        return false;
    }

    std::vector<std::string> ProgramProcessor::getProgramList() const
    {
        std::vector<string> sret;
        programs->apply( bind( &vector<string>::push_back, ref(sret), bind( &ProgramInterface::getName, _1) ) );
        return sret;
    }

    const ProgramInterfacePtr ProgramProcessor::getProgram(const std::string& name) const
    {
        return programs->find_if(bind(equal_to<string>(), name, bind(&ProgramInterface::getName, _1)));
    }

    ProgramInterfacePtr ProgramProcessor::getProgram(const std::string& name)
    {
        return programs->find_if(bind(equal_to<string>(), name, bind(&ProgramInterface::getName, _1)));
    }
}
