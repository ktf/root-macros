//-*- Mode: C++ -*-

#include <memory>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/pair.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/next.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/apply.hpp>

// recursive accumulation of creation wrappers
template < typename _Interface,
	   typename _Base,
	   typename _Iterator,
	   typename _End,
	   typename _Property,
	   typename _WrappersToApply >
struct WrapperAccumulation
{
  static std::shared_ptr< _Interface > apply( _Property property )
  {
    typedef typename boost::mpl::deref< _Iterator >::type        flag_to_wrapper;
    typedef typename boost::mpl::first< flag_to_wrapper >::type  flag;
    typedef typename boost::mpl::second< flag_to_wrapper >::type wrapper;

    if ( property & flag::value ) {
      // add current wrapper
      return WrapperAccumulation<
        _Interface, 
        _Base,
	typename boost::mpl::next< _Iterator >::type,
        _End,
	_Property,
	typename boost::mpl::push_back<
          _WrappersToApply,
          wrapper
	  >::type
	>::apply( property );
    } else {
      // don't add current wrapper
      return WrapperAccumulation<
        _Interface,
        _Base,
	typename boost::mpl::next< _Iterator >::type,
        _End,
	_Property,
        _WrappersToApply
      >::apply( property );
    }
  }
};


//base case through partial template specialization
template < typename _Interface,
	   typename _Base, 
           typename _End,
	   typename _Property,
	   typename _WrappersToApply >
struct WrapperAccumulation< _Interface, _Base, 
			    _End, _End, _Property, _WrappersToApply >
{
  static std::shared_ptr< _Interface > apply( _Property property )
  {
    using boost::mpl::placeholders::_1;
    using boost::mpl::placeholders::_2;

    typedef typename
      boost::mpl::fold<
        _WrappersToApply,
        _Base,
        boost::mpl::apply1< _2, _1 >
      >::type mixin;

    return std::make_shared< mixin >();
  }
};

template < typename _Interface, typename _Base, typename _Property, typename WrapperMap >
struct CompositeCreator
{
  static std::shared_ptr< _Interface > apply( _Property property )
  {
    return WrapperAccumulation<
      _Interface,
      _Base,
      typename boost::mpl::begin< WrapperMap >::type,
      typename boost::mpl::end< WrapperMap >::type,
      _Property,
      boost::mpl::vector< >
    >::apply( property );
  }
};
