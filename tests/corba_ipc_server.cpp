/***************************************************************************
  tag: Peter Soetens  Mon Jun 26 13:26:02 CEST 2006  generictask_test.cpp

                        generictask_test.cpp -  description
                           -------------------
    begin                : Mon June 26 2006
    copyright            : (C) 2006 Peter Soetens
    email                : peter.soetens@fmtc.be

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#include <iostream>
#include <rtt/Method.hpp>
#include <rtt/interface/ServiceProvider.hpp>
#include <transports/corba/DataFlowI.h>
#include <rtt/transports/corba/RemotePorts.hpp>
#include <transports/corba/ServiceProviderC.h>
#include <transports/corba/corba.h>
#include <rtt/InputPort.hpp>
#include <rtt/OutputPort.hpp>
#include <rtt/TaskContext.hpp>
#include <transports/corba/TaskContextServer.hpp>
#include <transports/corba/TaskContextProxy.hpp>
#include <string>
#include <os/main.h>
#include "operations_fixture.hpp"

using namespace RTT;
using namespace RTT::detail;
using namespace std;

class TheServer : public TaskContext, public OperationsFixture
{
public:
    // Ports
    InputPort<double>  mi1;
    OutputPort<double> mo1;

    TheServer(string name) : TaskContext(name), mi1("mi"), mo1("mo") {
        ports()->addEventPort( mi1 );
        ports()->addPort( mo1 );
        this->createMethodFactories( this );
        ts = corba::TaskContextServer::Create( this, true ); //use-naming
        this->start();
    }
    ~TheServer() {
        this->stop();
    }

    void updateHook(){
        double d = 123456.789;
        mi1.read(d);
        mo1.write(d);
    }

    corba::TaskContextServer* ts;

};

int ORO_main(int argc, char** argv)
{
    corba::TaskContextProxy::InitOrb(argc,argv);

    {
        TheServer ctest1("peerRMC");
        TheServer ctest2("peerRM");
        TheServer ctest3("peerAM");
        TheServer ctest4("peerDFI");
        TheServer ctest5("peerPC");
        TheServer ctest6("peerPP");
        TheServer ctest7("peerDH");
        TheServer ctest8("peerBH");

        // wait for shutdown.
        corba::TaskContextServer::RunOrb();
    }

    corba::TaskContextProxy::DestroyOrb();

    return 0;
}
