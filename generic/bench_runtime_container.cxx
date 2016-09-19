// g++ --std=c++11 -g -ggdb -I$BOOST_ROOT/include -o bench_runtime_container bench_runtime_container.cxx

#include <iostream>
#include <iomanip>
#include <memory>
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
#include <chrono>

typedef std::chrono::system_clock system_clock;
typedef std::chrono::nanoseconds TimeScale;

/////////////////////////////////////////////////////////////////////
/// unrolled apply function
/// implements an unrolled version of the apply function of
/// RuntimeContainer for the sake of benchmarking
template<typename ContainerType, typename F>
typename F::return_type apply_unroll(ContainerType& c, int index, F f) {
  typedef typename ContainerType::types types;
  switch (index) {
  case 0: {
    typedef typename boost::mpl::at_c< types, 0 >::type stagetype;
    stagetype& stage = static_cast<stagetype&>(c);
    return f(stage);
  }
  case 1: {
    typedef typename boost::mpl::at_c< types, 1 >::type stagetype;
    stagetype& stage = static_cast<stagetype&>(c);
    return f(stage);
  }
  case 2: {
    typedef typename boost::mpl::at_c< types, 2 >::type stagetype;
    stagetype& stage = static_cast<stagetype&>(c);
    return f(stage);
  }
  case 3: {
    typedef typename boost::mpl::at_c< types, 3 >::type stagetype;
    stagetype& stage = static_cast<stagetype&>(c);
    return f(stage);
  }
    /* here we need a type trait to compile the code even if there
       are not enough elements in the mpl vector
      case 4: {
      typedef typename boost::mpl::at_c< types, 4 >::type stagetype;
      stagetype& stage = static_cast<stagetype&>(c);
      return f(stage);
      }
    */
  }
}

/////////////////////////////////////////////////////////////////////
/// helper function for the unrolled adder function
template<typename StageType, typename ContainerType, typename ValueType>
void stage_add(StageType*, ContainerType& c, ValueType v) {
  StageType& stage = static_cast<StageType&>(c);
  return stage += v;
}

/////////////////////////////////////////////////////////////////////
/// unrolled add function
/// avoid the creation of the functor object
template<typename ContainerType, typename ValueType>
void add_unroll(ContainerType c, int index, ValueType v) {
  typedef typename ContainerType::types types;
  switch (index) {
  case 0: {
    typedef typename boost::mpl::at_c< types, 0 >::type stagetype;
    stage_add((stagetype*)(0), c, v);
  }
  case 1: {
    typedef typename boost::mpl::at_c< types, 1 >::type stagetype;
    stagetype& stage = static_cast<stagetype&>(c);
    stage += v;
  }
  case 2: {
    typedef typename boost::mpl::at_c< types, 2 >::type stagetype;
    stagetype& stage = static_cast<stagetype&>(c);
    stage += v;
  }
  case 3: {
    typedef typename boost::mpl::at_c< types, 3 >::type stagetype;
    stagetype& stage = static_cast<stagetype&>(c);
    stage += v;
  }
    /*
      case 4: {
      typedef typename boost::mpl::at_c< types, 4 >::type stagetype;
      stagetype& stage = static_cast<stagetype&>(c);
      stage += v;
      }
    */
  }
}

template<typename SetType>
struct check_set {
  static int apply(int nrolls) {
    typedef RuntimeContainer<DefaultInterface, funny_initializer, verbose_printer> ContainerBase_t;
    typedef typename create_rtc< SetType, ContainerBase_t >::type Container_t;
    Container_t container;

    system_clock::time_point refTimeApply = system_clock::now();
    for (int roll=0; roll < nrolls; roll++) {
      for (int i = 0; i < boost::mpl::size<SetType>::value; i++) {
	container.apply(i, add_value<float>(1));
      }
    }
    auto durationApply = std::chrono::duration_cast<TimeScale>(std::chrono::system_clock::now() - refTimeApply);

    system_clock::time_point refTimeApplyUnrolled = system_clock::now();
    for (int roll=0; roll < nrolls; roll++) {
      for (int i = 0; i < boost::mpl::size<SetType>::value; i++) {
	apply_unroll(container, i, add_value<float>(1));
      }
    }
    auto durationApplyUnrolled = std::chrono::duration_cast<TimeScale>(std::chrono::system_clock::now() - refTimeApplyUnrolled);

    system_clock::time_point refTimeApplyDirect = system_clock::now();
    for (int roll=0; roll < nrolls; roll++) {
      for (int i = 0; i < boost::mpl::size<SetType>::value; i++) {
	apply_unroll(container, i, add_value<float>(1));
      }
    }
    auto durationApplyDirect = std::chrono::duration_cast<TimeScale>(std::chrono::system_clock::now() - refTimeApplyDirect);

    std::cout << "Type set with " << boost::mpl::size<SetType>::value
	      << " element(s), "
	      << std::setw(10) << nrolls << " iteration(s): "
	      << std::setw(10) << durationApply.count() << " ns, "
	      << std::setw(10) << durationApplyUnrolled.count() << " ns "
	      << std::setw(10) << durationApplyDirect.count() << " ns "
	      << (float)(durationApplyUnrolled.count())/durationApply.count()
	      << "/"
	      << (float)(durationApplyDirect.count())/durationApplyUnrolled.count()
	      << std::endl;

    container.print();
  }
};

int main() {
  typedef boost::mpl::vector<
    int
    , char
    , unsigned int
    , float
    > types;

  for (auto nrolls : {100, 1000, 10000, 100000, 1000000, 10000000, 100000000}) {
    for (auto factor : { 1, 2, 4, 6, 8}) {
      check_set<types>::apply(nrolls*factor);
    }
  }

  return 0;
}
