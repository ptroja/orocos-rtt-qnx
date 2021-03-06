#ifndef ORO_OPERATION_REPOSITORY_PART_HPP
#define ORO_OPERATION_REPOSITORY_PART_HPP


#include <boost/shared_ptr.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/fusion/include/make_unfused_generic.hpp>

#include <vector>
#include <string>

#include "DataSource.hpp"
#include "CreateSequence.hpp"
#include "FusedFunctorDataSource.hpp"
#include "../interface/OperationRepository.hpp"
#include "../interface/FactoryExceptions.hpp"
#include "../Operation.hpp"
#include "../base/MethodBase.hpp"

/**
 * @file OperationRepositoryPart.hpp This file contains the code
 * required to register operations as scriptable entities.
 */

namespace RTT
{
    namespace internal {

        /**
         * OperationRepositoryPart implementation that uses boost::fusion
         * to produce items.
         */
        template<typename Signature>
        class OperationRepositoryPartFused
            : public interface::OperationRepositoryPart
        {
            typedef typename boost::function_traits<Signature>::result_type result_type;
            //! The factory for converting data sources to C++ types in call()
            typedef create_sequence<typename boost::function_types::parameter_types<Signature>::type> SequenceFactory;
            //! The factory for converting data sources to C++ types in collect(). This includes SendHandle.
            typedef create_sequence<typename boost::function_types::parameter_types<typename CollectType<Signature>::type>::type > CollectSequenceFactory;
            Operation<Signature>* op;
        public:
            OperationRepositoryPartFused( Operation<Signature>* o)
                : op(o)
            {
            }

            virtual std::string description() const {
                return op->getDescriptions().front();
            }

            virtual std::vector<interface::ArgumentDescription> getArgumentList() const {
                std::vector<std::string> const& descr = op->getDescriptions();
                std::vector<interface::ArgumentDescription> ret;
                for (unsigned int i =1; i < descr.size(); i +=2 )
                    ret.push_back(interface::ArgumentDescription(descr[i],descr[i+1], SequenceFactory::GetType((i-1)/2+1) ));
                return ret;
            }

            std::string resultType() const
            {
                return DataSource<result_type>::GetType();
            }

            unsigned int arity() const { return boost::function_traits<Signature>::arity; }

            const types::TypeInfo* getArgumentType(unsigned int arg) const
            {
                if (arg == 0 )
                    return internal::DataSourceTypeInfo<result_type>::getTypeInfo();
                return SequenceFactory::GetTypeInfo(arg);
            }

            unsigned int collectArity() const { return boost::function_traits< typename CollectType<Signature>::type >::arity; }

            const types::TypeInfo* getCollectType(unsigned int arg) const
            {
                return CollectSequenceFactory::GetTypeInfo(arg);
            }

            base::DataSourceBase::shared_ptr produce(
                            const std::vector<base::DataSourceBase::shared_ptr>& args, ExecutionEngine* caller) const
            {
                // convert our args and signature into a boost::fusion Sequence.
                if ( args.size() != arity() ) throw interface::wrong_number_of_args_exception(arity(), args.size() );
                return new FusedMCallDataSource<Signature>(typename base::MethodBase<Signature>::shared_ptr(op->getMethod()->cloneI(caller)), SequenceFactory()(args) );
            }

            virtual base::DataSourceBase::shared_ptr produceSend( const std::vector<base::DataSourceBase::shared_ptr>& args, ExecutionEngine* caller ) const {
                // convert our args and signature into a boost::fusion Sequence.
                if ( args.size() != arity() ) throw interface::wrong_number_of_args_exception(arity(), args.size() );
                return new FusedMSendDataSource<Signature>(typename base::MethodBase<Signature>::shared_ptr(op->getMethod()->cloneI(caller)), SequenceFactory()(args) );
            }

            virtual base::DataSourceBase::shared_ptr produceHandle() const {
                // Because of copy/clone,program script variables ('var') must begin unbound.
                return new internal::UnboundDataSource<ValueDataSource<SendHandle<Signature> > >();
            }

            virtual base::DataSourceBase::shared_ptr produceCollect( const std::vector<base::DataSourceBase::shared_ptr>& args, DataSource<bool>::shared_ptr blocking ) const {
                const unsigned int carity = boost::mpl::size<typename FusedMCollectDataSource<Signature>::handle_and_arg_types>::value;
                assert( carity == collectArity() + 1 ); // check for arity functions. (this is actually a compile time assert).
                if ( args.size() != carity ) throw interface::wrong_number_of_args_exception(carity, args.size() );
                // we need to ask FusedMCollectDataSource what the arg types are, based on the collect signature.
                return new FusedMCollectDataSource<Signature>( create_sequence<typename FusedMCollectDataSource<Signature>::handle_and_arg_types >()(args), blocking );
            }

            virtual Handle produceSignal( base::ActionInterface* func, const std::vector<base::DataSourceBase::shared_ptr>& args) const {
                // convert our args and signature into a boost::fusion Sequence.
                if ( args.size() != arity() ) throw interface::wrong_number_of_args_exception(arity(), args.size() );
                // note: in boost 1.41.0+ the function make_unfused() is available.
                return op->signals( boost::fusion::make_unfused_generic(boost::bind(&FusedMSignal<Signature>::invoke,
                                                                            boost::make_shared<FusedMSignal<Signature> >(func, SequenceFactory::assignable(args)),
                                                                            _1
                                                                            )
                                                                )
                                   );
            }

            boost::shared_ptr<base::DisposableInterface> getLocalOperation() const {
                return op->getImplementation();
            }
        };

            /**
             * OperationRepositoryPart implementation that uses boost::fusion
             * to produce items. The methods invoked get their object pointer
             * from the first data source, which contains a shared_ptr.
             * @param Signature The full signature, with the object pointer as
             * the first argument.
             * @param ObjT The object pointer type used in Signature.
             */
            template<typename Signature,typename ObjT>
            class OperationRepositoryPartFusedDS
                : public interface::OperationRepositoryPart
            {
                typedef create_sequence<typename boost::function_types::parameter_types<Signature>::type> SequenceFactory;
                typedef create_sequence<typename boost::function_types::parameter_types<typename CollectType<Signature>::type>::type > CollectSequenceFactory;
                typedef typename boost::function_traits<Signature>::result_type result_type;
                Operation<Signature>* op;
                // the datasource that stores a weak pointer is itself stored by a shared_ptr.
                typename DataSource<boost::shared_ptr<ObjT> >::shared_ptr mwp;
            public:
                OperationRepositoryPartFusedDS( DataSource< boost::shared_ptr<ObjT> >* wp, Operation<Signature>* o)
                    : op( o ), mwp(wp)
                {
                }

                typedef std::vector<base::DataSourceBase::shared_ptr> ArgList;

                std::string resultType() const
                {
                    return DataSource<result_type>::GetType();
                }

                unsigned int arity() const { return boost::function_traits<Signature>::arity - 1;/*lie about the hidden member pointer */ }

                const types::TypeInfo* getArgumentType(unsigned int arg) const
                {
                    if (arg == 0 )
                        return internal::DataSourceTypeInfo<result_type>::getTypeInfo();
                    return SequenceFactory::GetTypeInfo(arg);
                }

                unsigned int collectArity() const { return boost::function_traits< typename CollectType<Signature>::type >::arity; }

                const types::TypeInfo* getCollectType(unsigned int arg) const
                {
                    return CollectSequenceFactory::GetTypeInfo(arg);
                }

                virtual std::string description() const {
                    return op->getDescriptions().front();
                }

                virtual std::vector<interface::ArgumentDescription> getArgumentList() const {
                    std::vector<std::string> const& descr = op->getDescriptions();
                    std::vector<interface::ArgumentDescription> ret;
                    for (unsigned int i =1; i < descr.size(); i +=2 )
                        ret.push_back(interface::ArgumentDescription(descr[i],descr[i+1], SequenceFactory::GetType((i-1)/2+1)) );
                    return ret;
                }

                base::DataSourceBase::shared_ptr produce(ArgList const& args, ExecutionEngine* caller) const
                {
                    if ( args.size() != arity() ) throw interface::wrong_number_of_args_exception(arity(), args.size() );
                    // the user won't give the necessary object argument, so we glue it in front.
                    ArgList a2;
                    a2.reserve(args.size()+1);
                    a2.push_back(mwp);
                    a2.insert(a2.end(), args.begin(), args.end());
                    // convert our args and signature into a boost::fusion Sequence.
                    return new FusedMCallDataSource<Signature>(typename base::MethodBase<Signature>::shared_ptr(op->getMethod()->cloneI(caller)), SequenceFactory()(a2) );
                }

                virtual base::DataSourceBase::shared_ptr produceSend( const std::vector<base::DataSourceBase::shared_ptr>& args, ExecutionEngine* caller ) const {
                    if ( args.size() != arity() ) throw interface::wrong_number_of_args_exception(arity(), args.size() );
                    // the user won't give the necessary object argument, so we glue it in front.
                    ArgList a2;
                    a2.reserve(args.size()+1);
                    a2.push_back(mwp);
                    a2.insert(a2.end(), args.begin(), args.end());
                    // convert our args and signature into a boost::fusion Sequence.
                    return new FusedMSendDataSource<Signature>(typename base::MethodBase<Signature>::shared_ptr(op->getMethod()->cloneI(caller)), SequenceFactory()(a2) );
                }

                virtual base::DataSourceBase::shared_ptr produceHandle() const {
                    // Because of copy/clone,value objects must begin unbound.
                    return new internal::UnboundDataSource<ValueDataSource<SendHandle<Signature>& > >();
                }

                virtual base::DataSourceBase::shared_ptr produceCollect( const std::vector<base::DataSourceBase::shared_ptr>& args, DataSource<bool>::shared_ptr blocking ) const {
                    const unsigned int carity = boost::mpl::size<typename FusedMCollectDataSource<Signature>::handle_and_arg_types>::value;
                    assert( carity == collectArity() + 1 ); // check for arity functions. (this is actually a compile time assert).
                    if ( args.size() != carity ) throw interface::wrong_number_of_args_exception(carity, args.size() );
                    // we need to ask FusedMCollectDataSource what the arg types are, based on the collect signature.
                    return new FusedMCollectDataSource<Signature>( create_sequence<typename FusedMCollectDataSource<Signature>::handle_and_arg_types >()(args), blocking );
                }

                boost::shared_ptr<base::DisposableInterface> getLocalOperation() const {
                    return op->getImplementation();
                }
            };
    }
}

#endif
