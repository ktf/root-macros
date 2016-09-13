// g++ --std=c++11 -g -ggdb -I$BOOST_ROOT/include -o test_runtime_container test_runtime_container.cxx

#include <iostream>
#include <boost/mpl/size.hpp>
#include <boost/mpl/apply.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/map.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/plus.hpp>
#include <boost/mpl/at.hpp>
#include "runtime_container.h"

template< typename N1, typename N2 >
struct int_plus : boost::mpl::int_<( N1::value + N2::value )> {};
template< typename N1, typename N2 >
struct int_minus : boost::mpl::int_<( N1::value - N2::value )> {};

using boost::mpl::placeholders::_1;
using boost::mpl::placeholders::_2;

struct print_container {
  template<typename T>
  void operator()(T t) {
    std::cout << "--------------------------" << std::endl;
    t.print();
  }
};

int main()
{
  ////////////////////////////////////////////////////////////////////////////////
  // small example for mpl apply, the first template argument defined the meta
  // function class, the following arguments are passed as template arguments
  // to that meta function. Each placeholder is replace by the Nth argument
  typedef boost::mpl::apply< int_plus<_1,_2>, boost::mpl::int_<2>, boost::mpl::int_<3> >::type t1;
  typedef boost::mpl::apply< int_plus<_1,_2>, t1, boost::mpl::int_<3> >::type t2;
  typedef boost::mpl::apply< int_minus<_1,_2>, t1, t2 >::type t3;
  typedef boost::mpl::apply< int_minus<_2,_1>, t1, t2 >::type t4;

  // example how to use types and objects at runtime
  t1 v1; t2 v2; t3 v3; t4 v4;

  std::cout << "example for boost::mpl::apply:" << std::endl;
  std::cout << "int_minus meta fct:   " << t1::value << " - " << t2::value << " = " << v3.value << std::endl;
  std::cout << "reverse placeholders: " << t2::value << " - " << t1::value << " = " << v4.value << std::endl;
  std::cout << std::endl;

  ////////////////////////////////////////////////////////////////////////////////

  typedef boost::mpl::vector<
    int
    , char
    , unsigned int
    , float
    > types;

  ////////////////////////////////////////////////////////////////////////////////
  std::cout << "testing composite container: " << std::endl;
  typedef RuntimeContainer<DefaultInterface, funny_initializer, verbose_printer> ContainerBase_t;
  typedef create_rtc< types, ContainerBase_t >::type Container_t;
  Container_t container;
  container.print();

  std::cout << std::endl << "checking rtc_less meta function" << std::endl;
  typedef boost::mpl::int_<2> condition2;
  typedef boost::mpl::int_<3> condition3;
  typedef boost::mpl::int_<5> condition5;
  std::cout << "has less then " << condition3::value << " entries? " << (rtc_less<Container_t, condition3>::value?"yes":"no") << std::endl;
  std::cout << "has less then " << condition5::value << " entries? " << (rtc_less<Container_t, condition5>::value?"yes":"no") << std::endl;

  ////////////////////////////////////////////////////////////////////////////////
  std::cout << std::endl << "checking vector of container types" << std::endl;


  boost::mpl::for_each<Container_t::types>(print_container());

  ////////////////////////////////////////////////////////////////////////////////
  std::cout << std::endl << "reduced container, reduced to " << condition2::value << " elements" << std::endl;
  // the mixin base is labeled -1, the mixin stages are labeled from 0 to n-1
  // rtc_less checks for 'level less than condition_level-1', if 2 is given
  // as condition here, fold applies the mixin first on the base which has level
  // -1 and then on the first mixin stage which has level 0
  typedef create_rtc< types, ContainerBase_t, condition2 >::type container2_t;

  container2_t container2;
  container2.print();

  ////////////////////////////////////////////////////////////////////////////////
  // the cast to a certain level in the mixin works only as desired as
  // long as there is no virtual inheritance
  std::cout << std::endl << "checking cast of full container to reduced type" << std::endl;
  auto& ref=static_cast<boost::mpl::at_c<Container_t::types, 1>::type&>(container);
  ref.print();

  std::cout << std::endl << "testing runtime container setter (float, set to 42.5 + i)" << std::endl;
  for (int i = 0; i < boost::mpl::size<types>::value; i++) {
    container.apply(i, set_value<float>(42.5 + i));
  }
  container.print();
  std::cout << std::endl << "testing runtime container getter (float)" << std::endl;
  for (int i = 0; i < boost::mpl::size<types>::value; i++) {
    std::cout << "reading container at level " << i << ": "
	      <<container.apply(i, get_value<float>()) << std::endl;
  }
}
