#include "ServiceRequester.hpp"
#include "ServiceProvider.hpp"
#include "../internal/mystd.hpp"
#include "../Logger.hpp"
#include "../TaskContext.hpp"

#include <utility>

namespace RTT
{
    using namespace detail;
    using namespace std;

    ServiceRequester::ServiceRequester(const std::string& name, TaskContext* tc) :
        mrname(name), mrowner(tc)
    {
    }

    ServiceRequester::~ServiceRequester()
    {
    }

    bool ServiceRequester::addMethod(MethodBaseInvoker& isb)
    {
        if (mmethods.find(isb.getName()) != mmethods.end())
        {
            log(Error) << "Method with name '" + isb.getName() + "' already present." << endlog();
            return false;
        }
        mmethods.insert(make_pair<std::string, MethodBaseInvoker&> (isb.getName(), isb));
        return true;
    }

    std::vector<std::string> ServiceRequester::getMethodNames() const
    {
        return keys(mmethods);
    }

    std::vector<std::string> ServiceRequester::getRequestNames() const
    {
        return keys(mrequests);
    }

    MethodBaseInvoker& ServiceRequester::getMethod(const std::string& name)
    {
        return mmethods.find(name)->second;
    }

    bool ServiceRequester::connectTo( ServiceProvider* sp) {
        for (Methods::iterator it = mmethods.begin(); it != mmethods.end(); ++it) {
            if ( !it->second.ready() ) {
                if (sp->hasOperation( it->first )) {
                    it->second.setImplementation( sp->getLocalOperation( it->first ), mrowner ? mrowner->engine() : 0 );
                    if ( it->second.ready() ) {
                        if (mrowner)
                            log(Debug) << "Successfully set up Method " << it->first <<endlog();
                        else
                            log(Warning) << "Method "<< it->first << " only setup of synchronous calls (no owner was set)."<<endlog();
                    }
                }
            }
        }
        if (ready())
            log(Info) << "Found complete interface of requested service '" << mrname <<"'"<< endlog();

        return ready();
    }

    bool ServiceRequester::ready() const
    {
        for (Methods::const_iterator it = mmethods.begin(); it != mmethods.end(); ++it)
            if ( !it->second.ready() )
                return false;
        return true;
    }
}