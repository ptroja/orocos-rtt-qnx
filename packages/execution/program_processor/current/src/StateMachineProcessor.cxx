/***************************************************************************
  tag: Peter Soetens  Wed Jan 18 14:11:40 CET 2006  StateMachineProcessor.cxx 

                        StateMachineProcessor.cxx -  description
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
 
 

#include "execution/StateMachineProcessor.hpp"
#include <corelib/Logger.hpp>

#include <boost/bind.hpp>
#include <os/Semaphore.hpp>
#include <iostream>

namespace ORO_Execution
{

    using namespace boost;
    using namespace std;
    using namespace ORO_CoreLib;


    StateMachineProcessor::StateMachineProcessor(ORO_OS::Semaphore* work_sem)
        : states( new StateMap(4) ),
          queuesem( work_sem )
    {
    }

    StateMachineProcessor::~StateMachineProcessor()
    {
        // first deactivate (hard way) all state machines :
        states->apply( bind( &StateMachine::deactivate, _1));
        states->apply( bind( &StateMachine::deactivate, _1));
    
        while ( !states->empty() ) {
            // try to unload all
            try {
                Logger::log() << Logger::Info << "StateMachineProcessor unloads StateMachine "<< states->front()->getName() << "..."<<Logger::endl;
                this->unloadStateMachine( states->front()->getName() );
            } catch ( program_load_exception& ple) {
                Logger::log() << Logger::Error << ple.what() <<Logger::endl;
                states->erase( states->front() ); // plainly remove it to avoid endless loop.
            }
        }
            
        delete states;
    }

     StateMachine::Status::StateMachineStatus StateMachineProcessor::getStateMachineStatus(const std::string& name) const
     {
         StateMachinePtr pip = states->find_if( bind(equal_to<string>(), name, bind(&StateMachine::getName, _1)) );
         if ( pip )
             return pip->getStatus();
         return Status::unloaded;
     }

     std::string StateMachineProcessor::getStateMachineStatusStr(const std::string& name) const
     {
        switch ( getStateMachineStatus( name ))
            {
            case Status::inactive:
                return "inactive";
                break;
            case Status::stopping:
                return "stopping";
                break;
            case Status::stopped:
                return "stopped";
                break;
            case Status::requesting:
                return "requesting";
                break;
            case Status::running:
                return "running";
                break;
            case Status::paused:
                return "paused";
                break;
            case Status::active:
                return "active";
                break;
            case Status::activating:
                return "activating";
                break;
            case Status::deactivating:
                return "deactivating";
                break;
            case Status::resetting:
                return "resetting";
                break;
            case Status::error:
                return "error";
                break;
            case Status::unloaded:
                return "unloaded";
                break;
            }
        return "na";
     }

	bool StateMachineProcessor::loadStateMachine( StateMachinePtr sc )
    {
        // test if parent ...
        if ( sc->getParent() ) {
            std::string error(
                "Could not register StateMachine \"" + sc->getName() +
                "\" with the processor. It is not a root StateMachine." );
            throw program_load_exception( error );
        }

        this->recursiveCheckLoadStateMachine( sc ); // throws load_exception
        this->recursiveLoadStateMachine( sc );
        return true;
    }
    
    void StateMachineProcessor::recursiveCheckLoadStateMachine( StateMachinePtr sc )
    {
        // test if already present..., this cannot detect corrupt
        // trees with double names...
        StateMachinePtr pip = states->find_if( bind(equal_to<string>(), sc->getName(), bind(&StateMachine::getName, _1)) );

        if ( pip ) {
            std::string error(
                "Could not register StateMachine \"" + sc->getName() +
                "\" with the processor. A StateMachine with that name is already present." );
            throw program_load_exception( error );

            std::vector<StateMachinePtr>::const_iterator it2;
            for (it2 = sc->getChildren().begin(); it2 != sc->getChildren().end(); ++it2)
                {
                    this->recursiveCheckLoadStateMachine( *it2 );
                }
        }
    }

    void StateMachineProcessor::recursiveLoadStateMachine( StateMachinePtr sc )
    {
        std::vector<StateMachinePtr>::const_iterator it;

        // first load parent.
        states->grow();
        states->append(sc);
        sc->setStateMachineProcessor(this);

        // then load children.
        for (it = sc->getChildren().begin(); it != sc->getChildren().end(); ++it)
            {
                this->recursiveLoadStateMachine( *it );
            }
        
    }

    bool StateMachineProcessor::unloadStateMachine( const std::string& name )
    {
        StateMachinePtr pip = states->find_if( bind(equal_to<string>(), name, bind(&StateMachine::getName, _1)) );

        if ( pip ) {
            // test if parent ...
            if ( pip->getParent() ) {
                std::string error(
                                  "Could not unload StateMachine \"" + pip->getName() +
                                  "\" with the processor. It is not a root StateMachine." );
                throw program_unload_exception( error );
            }
            recursiveCheckUnloadStateMachine( pip );
            recursiveUnloadStateMachine( pip );
            return true;
        }
        return false;
    }

    void StateMachineProcessor::recursiveCheckUnloadStateMachine(StateMachinePtr si)
    {
        // check this state
        if ( si->isActive() ) {
            std::string error(
                              "Could not unload StateMachine \"" + si->getName() +
                              "\" with the processor. It is still active, status is "+
                    this->getStateMachineStatusStr( si->getName() ) );
            throw program_unload_exception( error );
        }

        // check children
        std::vector<StateMachinePtr>::const_iterator it2;
        for (it2 = si->getChildren().begin();
             it2 != si->getChildren().end();
             ++it2)
            {
                StateMachinePtr pip = states->find_if( bind(equal_to<string>(), (*it2)->getName(), bind(&StateMachine::getName, _1)) );
                if ( !pip ) {
                    std::string error(
                              "Could not unload StateMachine \"" + si->getName() +
                              "\" with the processor. It contains not loaded child "+ (*it2)->getName() );
                    throw program_unload_exception( error );
                }
                // all is ok, check child :
                this->recursiveCheckUnloadStateMachine( pip );
            }
    }

    void StateMachineProcessor::recursiveUnloadStateMachine(StateMachinePtr sc) {
        std::vector<StateMachinePtr>::const_iterator it;

        // first erase children
        for (it = sc->getChildren().begin(); it != sc->getChildren().end(); ++it)
            {
                this->recursiveUnloadStateMachine( *it );
            }
        
        // erase this sc :
        StateMachinePtr pip = states->find_if( bind(equal_to<string>(), sc->getName(), bind(&StateMachine::getName, _1)) );

        assert( pip ); // we checked that this is possible

        // lastly, unload the parent.
        states->erase(pip);
        states->shrink();
        sc->setStateMachineProcessor( 0 );

    }

	bool StateMachineProcessor::deleteStateMachine(const std::string& name)
    {
        return this->unloadStateMachine(name);
    }

    bool StateMachineProcessor::initialize()
    {
        return true;
    }

    static void shutdown_hard(StateMachinePtr sc)  {
        if ( sc->isActive() == false)
            return;
        if ( sc->isReactive() )
            sc->requestFinalState();
        if ( sc->isAutomatic() )
            sc->stop();
        if (sc->currentState() != sc->getFinalState() )
            sc->execute(); // try one last time
        if (sc->currentState() != sc->getFinalState() )
            Logger::log() << Logger::Critical << "StateMachineProcessor failed to bring StateMachine "<< sc->getName()
                          << " into the final state. Program stalled in state '"
                          << sc->currentState()->getName()<<"' line number "
                          << sc->getLineNumber()<<Logger::endl; // critical failure !
    }

    void StateMachineProcessor::finalize()
    {
        // stop all SCs the 'hard' way.
        states->apply(bind(&shutdown_hard, _1));
    }

	void StateMachineProcessor::step()
    {
        // Evaluate all states->
        states->apply(bind(&StateMachine::execute, _1));
    }

    const StateMachinePtr StateMachineProcessor::getStateMachine(const std::string& name) const
    {
        return states->find_if(bind(equal_to<string>(), name, bind(&StateMachine::getName, _1)));
    }

    StateMachinePtr StateMachineProcessor::getStateMachine(const std::string& name)
    {
        return states->find_if(bind(equal_to<string>(), name, bind(&StateMachine::getName, _1)));
    }

    std::vector<std::string> StateMachineProcessor::getStateMachineList() const
    {
        std::vector<string> sret;
        states->apply( bind( &vector<string>::push_back, ref(sret), bind( &StateMachine::getName, _1) ) );
        return sret;
    }
}
