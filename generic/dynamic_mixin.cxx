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
#include <boost/mpl/apply.hpp>

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
  volatile uint16_t mode = 0x1 | 0x4;
  std::shared_ptr<Interface> dyninterface1(create( mode ));
  std::cout << "======== dynamic interface 1 ======================" << std::endl;
  dyninterface1->print();
  mode = 0x2;
  std::shared_ptr<Interface> dyninterface2(create( mode ));
  std::cout << "======== dynamic interface 2 ======================" << std::endl;
  dyninterface2->print();

  std::cout << "======== mixin with boost mpl apply  ==============" << std::endl;
  // define a data type for dec on top of base funtionality
  typedef dec<BaseFunctionality<int>> dec_t;

  // define an application of hex on top of dec_t as new data type
  // the Nth occurrence of the unnamed placeholder '_' is replaced
  // by the Nth argument, in this example '_1' could also be used
  using namespace boost::mpl::placeholders;
  typedef boost::mpl::apply1< hex<_>, dec_t >::type hexdec_t;

  // make objects from the data types and print the structure
  dec_t deco;
  hexdec_t hexdeco;
  std::cout << "one-level mixin (explicitely declared):" << std::endl;
  deco.print();
  std::cout << "two-level mixin (boost mpl apply):" << std::endl;
  hexdeco.print();

}
