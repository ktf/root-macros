//****************************************************************************
//* This file is free software: you can redistribute it and/or modify        *
//* it under the terms of the GNU General Public License as published by     *
//* the Free Software Foundation, either version 3 of the License, or        *
//* (at your option) any later version.                                      *
//*                                                                          *
//* Primary Authors: Matthias Richter <richterm@scieq.net>                   *
//*                                                                          *
//* The authors make no claims about the suitability of this software for    *
//* any purpose. It is provided "as is" without express or implied warranty. *
//****************************************************************************

//  @file   multiple_distributions.cxx
//  @author Matthias Richter
//  @since  2015-09-20
//  @brief  Type safe container for multiple statistics distributions

// Compilation: make sure variable BOOST_ROOT points to your boost installation
/*
   g++ --std=c++11 -g -ggdb -I$BOOST_ROOT/include -o multiple_distributions multiple_distributions.cxx
*/

#include <iostream>
#include <iomanip> // std::setw
#include <vector>
#include <random> // random distribution
#include <exception>
#include <stdexcept>
#include <cmath>  // std::exp
#include <boost/mpl/vector.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/size.hpp>
#include "runtime_container.h"

template<typename ReturnType, typename RandomEngine>
class randval {
public:
  randval(RandomEngine& engine) : mEngine(engine) {}
  typedef ReturnType return_type;
  template<typename ContainerType>
  return_type operator()(ContainerType& c) {
    return (*c)(mEngine);
  }
private:
  randval(); // forbidden
  RandomEngine& mEngine;
};

template<typename ParamType>
class setparam {
public:
  typedef void return_type;

private:
  template<typename U, typename V>
  struct ParamTypeTraits {
    template<typename DistributionType>
    static return_type apply(DistributionType& distribution, ParamType& param) {
      throw std::runtime_error("parameter type mismatch");
    }
  };
  template<typename U>
  struct ParamTypeTraits<U, U> {
    template<typename DistributionType>
    static return_type apply(DistributionType& distribution, ParamType& param) {
      return distribution.param(param);
    }
  };
  setparam(); // forbidden
  ParamType mParam;

public:
  setparam(ParamType param) : mParam(param) {}
  template<typename ContainerType>
  return_type operator()(ContainerType& c) {
    return ParamTypeTraits<ParamType, typename ContainerType::wrapped_type::param_type>::apply(*c, mParam);
  }
};

template<typename RandomEngine>
class print_graph {
public:
  typedef void return_type;

  print_graph(RandomEngine& engine, int nIterations, int min = 0, int max = 10)
    : mEngine(engine)
    , mIterations(nIterations)
    , mMin(min)
    , mMax(max)
  {}
  template<typename ContainerType>
  void operator()(ContainerType& c) {
    std::map<int, int> counts;
    for (int i=mMin; i<mMax; ++i) {
      counts[i] = 0;
    }

    const int nstars=100;

    for (int i=0; i<mIterations; ++i) {
      auto number = (*c)(mEngine);
      if ((number>=mMin)&&(number<mMax)) ++counts[int(number)];
    }
    for (int i=mMin; i<mMax; ++i) {
      std::cout << std::setw(2) << i << "-" << std::setw(2) << (i+1) << ": ";
      std::cout << std::string(counts[i]*nstars/mIterations,'*') << std::endl;
    }
  }

private:
  print_graph(); // forbidden
  RandomEngine& mEngine;
  int mIterations;
  float mMin;
  float mMax;
};

int main()
{
  typedef std::default_random_engine RandomEngine_t;
  RandomEngine_t generator;

  typedef boost::mpl::vector<
    std::exponential_distribution<double>
    , std::geometric_distribution<int>
    , std::normal_distribution<double>
    > distributions_t;

  typedef RuntimeContainer<DefaultInterface, default_initializer, verbose_printer> ContainerBase_t;
  typedef typename create_rtc< distributions_t, ContainerBase_t >::type DistributionContainer_t;
  DistributionContainer_t distributions;

  // TODO: generalized method to set parameters, e.g. use a mpl vector to define
  // distribution type in the first element and parameters in sebsequent elements
  distributions.apply(0, setparam<boost::mpl::at_c<distributions_t, 0>::type::param_type >(boost::mpl::at_c<distributions_t, 0>::type::param_type(0.5)));
  distributions.apply(1, setparam<boost::mpl::at_c<distributions_t, 1>::type::param_type >(boost::mpl::at_c<distributions_t, 1>::type::param_type(0.3)));
  distributions.apply(2, setparam<boost::mpl::at_c<distributions_t, 2>::type::param_type >(boost::mpl::at_c<distributions_t, 2>::type::param_type(5.0, 2.)));

  for (int i=0; i < boost::mpl::size<distributions_t>::value; i++) {
    distributions.apply(i, print_graph<RandomEngine_t>(generator, 10000, 0, 10));
  }

}
