// Demonstrator for the composite factory for a mixin class creator.
//
// The composite_factory is a compile time factory for creating combinations
// of the mixin objects depending on the property flag.
//
// compilation:
// Note: because of the include file hierarchy of the boost libraries,
// BOOST_INCLUDE_DIR points to include directory containing the 'boost'
// include folder
// g++ -o dynamic_mixin -I$BOOST_INCLUDE_DIR -std=c++11 dynamic_mixin.cxx

#include <iostream>
#include <boost/mpl/map.hpp>
#include <boost/mpl/pair.hpp>
#include <boost/mpl/int.hpp>

#include "composite_factory.h"
#include "mixinclass.h"

std::shared_ptr< Interface > create( int property )
{
  using namespace boost::mpl::placeholders;

  typedef boost::mpl::map<
    boost::mpl::pair< boost::mpl::int_< 0x01 >, oct<_> >,
    boost::mpl::pair< boost::mpl::int_< 0x02 >, dec<_> >,
    boost::mpl::pair< boost::mpl::int_< 0x04 >, hex<_> >
  > flag_to_wrapper;

  return CompositeCreator< Interface, BaseFunctionality<int>, int, flag_to_wrapper >::apply( property );
}

int main()
{
  std::shared_ptr<Interface> dyninterface1(create( 0x1 | 0x4 ));
  std::cout << "======== dynamic interface 1 ======================" << std::endl;
  dyninterface1->print();
  std::shared_ptr<Interface> dyninterface2(create( 0x2 ));
  std::cout << "======== dynamic interface 2 ======================" << std::endl;
  dyninterface2->print();
}
