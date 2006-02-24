/***************************************************************************
  tag: Peter Soetens  Wed Jan 18 14:11:40 CET 2006  EventC.cxx 

                        EventC.cxx -  description
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
 
 
#include "execution/EventService.hpp"
#include "execution/EventC.hpp"
#include "execution/FactoryExceptions.hpp"
#include "corelib/Logger.hpp"
#include <vector>

namespace ORO_Execution
{
    using namespace ORO_CoreLib;
    
    class EventC::D
    {
    public:
        const EventService* mgcf;
        std::string mname;
        std::vector<DataSourceBase::shared_ptr> args;
        DataSourceBase::shared_ptr m;

        void checkAndCreate() {
            Logger::In in("EventC");
            if ( mgcf == 0 || mgcf->hasEvent(mname) == false ) {
                Logger::log() <<Logger::Error << "No '"<<mname<<"' event found in the EventService."<<Logger::endl;
                throw name_not_found_exception(mname);
            }
            size_t sz = mgcf->arity(mname);
            if ( sz == args.size() ) {
                // may throw.
                m = mgcf->setupEmit(mname, args );
                args.clear();
            }
        }

        void newarg(DataSourceBase::shared_ptr na)
        {
            this->args.push_back( na );
            this->checkAndCreate();
        }

        D( const EventService* gcf, const std::string& name)
            : mgcf(gcf), mname(name), m()
        {
            this->checkAndCreate();
        }

        D(const D& other)
            : mgcf( other.mgcf), mname(other.mname),
              args( other.args ), m( other.m )
        {
        }

        ~D()
        {
        }

        void emit() {
            if (m)
                m->evaluate();
        }

    };

    EventC::EventC(const EventService* gcf, const std::string& name)
        : d( new D( gcf, name) ), m()
    {
        if (d->m) {
            this->m = d->m;
            delete d;
            d = 0;
        }
    }

    EventC::EventC()
        : d( 0 ), m()
    {
    }

    EventC::EventC(const EventC& other)
        : d( other.d ? new D(*other.d) : 0 ), m(other.m)
    {
    }

    EventC& EventC::operator=(const EventC& other)
    {
        delete d;
        d = ( other.d ? new D(*other.d) : 0 );
        m = other.m;
        return *this;
    }

    EventC::~EventC()
    {
        delete d;
    }

    EventC& EventC::arg( DataSourceBase::shared_ptr a )
    {
        if (d)
            d->newarg( a );
        else {
            Logger::log() <<Logger::Warning << "Extra argument discarded for EventC."<<Logger::endl;
        }

        if (d && d->m) {
            this->m = d->m;
            delete d;
            d = 0;
        }
            
        return *this;
    }

    void EventC::emit() {
        if (m)
            m->evaluate();
        else {
            Logger::log() <<Logger::Error << "emit() called on incomplete EventC."<<Logger::endl;
            if (d) {
                size_t sz = d->mgcf->arity(d->mname);
                if ( m ) {
                    Logger::log() <<Logger::Error << "Wrong number of arguments provided for event '"+d->mname+"'"<<Logger::nl;
                    Logger::log() <<Logger::Error << "Expected "<< sz << ", got: " << d->args.size() <<Logger::endl;
                }
            }
        }
    }
}