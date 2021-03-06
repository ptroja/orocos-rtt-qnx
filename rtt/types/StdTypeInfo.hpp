
#include "../rtt-config.h"
#include "Types.hpp"
#include "TemplateTypeInfo.hpp"

namespace RTT
{
    namespace types {

        /**
         * Standard types that don't need decomposition.
         */
        template<class T>
        struct StdTypeInfo
        : public TemplateTypeInfo<T,true>
        {
            StdTypeInfo(const char* type)
            : TemplateTypeInfo<T,true>(type)
              {}
            virtual bool decomposeType( base::DataSourceBase::shared_ptr source, PropertyBag& targetbag ) const {
                return false;
            }

            virtual bool composeType( base::DataSourceBase::shared_ptr source, base::DataSourceBase::shared_ptr result) const {
                // First, try a plain update.
                if ( result->update( source.get() ) )
                    return true;
                return false;
            }
        };
    }
}
