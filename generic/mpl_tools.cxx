// g++ --std=c++11 -g -ggdb -I$BOOST_ROOT/include -o mpl_tools mpl_tools.cxx

#include <iostream>
#include <boost/mpl/apply.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/map.hpp>
#include "runtime_container.h"

template< typename N1, typename N2 >
struct int_plus : boost::mpl::int_<( N1::value + N2::value )> {};
template< typename N1, typename N2 >
struct int_minus : boost::mpl::int_<( N1::value - N2::value )> {};

using boost::mpl::placeholders::_1;
using boost::mpl::placeholders::_2;

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

  typedef boost::mpl::map<
    boost::mpl::pair<rc_mixin<_1, _2>, int>,
    boost::mpl::pair<rc_mixin<_1, _2>, char>,
    boost::mpl::pair<rc_mixin<_1, _2>, unsigned int>,
    boost::mpl::pair<rc_mixin<_1, _2>, float>
    > mixinmap;

  typedef rc_base<int> Base_t;
  typedef typename
    boost::mpl::fold<
    mixinmap,
    Base_t,
    boost::mpl::apply2< boost::mpl::first<_2>, _1, boost::mpl::second<_2> >
    >::type mixin_t;

  std::cout << "testing composite mixing: " << std::endl;
  mixin_t mixin;
  mixin.print();

  std::cout << std::endl << "checking rtc_less meta function" << std::endl;
  typedef boost::mpl::int_<2> condition2;
  typedef boost::mpl::int_<3> condition3;
  typedef boost::mpl::int_<5> condition5;
  std::cout << "has less then " << condition3::value << " entries? " << (rtc_less<mixin_t, condition3>::value?"yes":"no") << std::endl;
  std::cout << "has less then " << condition5::value << " entries? " << (rtc_less<mixin_t, condition5>::value?"yes":"no") << std::endl;

  std::cout << std::endl << "reduced mixin, reduced to " << condition2::value << " elements" << std::endl;
  // the mixin base is labeled -1, the mixin stages are labeled from 0 to n-1
  // rtc_less checks for 'level less than condition_level-1', if 2 is given
  // as condition here, fold applies the mixin first on the base which has level
  // -1 and then on the first mixin stage which has level 0
  typedef typename
    boost::mpl::fold<
    mixinmap
    , Base_t
    , boost::mpl::if_<
      rtc_less<_1, condition2 >
      , boost::mpl::apply2< boost::mpl::first<_2>, _1, boost::mpl::second<_2> >
      , boost::mpl::identity<_1>
      >
    >::type mixin2_t;

  mixin2_t mixin2;
  mixin2.print();

  // the cast to a certain level in the mixing works only as desired as
  // long as there is no virtual inheritence
  std::cout << std::endl << "checking cast of full mixin to reduced type" << std::endl;
  mixin2_t& ref=static_cast<mixin2_t&>(mixin);
  ref.print();
}
