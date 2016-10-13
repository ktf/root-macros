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

/// @file   compare_polymorphism.h
/// @author Matthias Richter
/// @since  2015-10-13
/// @brief  Light-weight test program for runtime and static polymorphism
///
/// For the runtime polymorphic case the virtual interface @ref Interface is
/// the base class for the type wrapper class @ref TypeWrapper. A std vector
/// holding pointers is used to hold the runtime structure of the array of
/// multiple instances of this wrapper with different data types.
/// The static polymorphic case uses the runtime container for the runtime
/// state of all types from the list. This example uses the generic apply
/// method for dispatch to the individual object.

/// Compilation:
/// g++ --std=c++11 -O3 -I$BOOST_ROOT/include -o compare_polymorphism compare_polymorphism.cxx
/// Options: -DNROLLS=number  default is 1000000000
///          -DSTATIC_POLY    select static polymorphism (default runtime)
/// debug options: replace -O3 by e.g. '-g -ggdb'

#include "runtime_container.h"
#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <type_traits>
#include <chrono>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/at.hpp>

typedef std::chrono::system_clock system_clock;
typedef std::chrono::nanoseconds TimeScale;

#ifdef STATIC_POLY
#define VIRTUAL_ATTRIBUTE
#define PURE_VIRTUAL_ATTRIBUTE {}
#else
#define VIRTUAL_ATTRIBUTE virtual
#define PURE_VIRTUAL_ATTRIBUTE = 0
#endif

/**
 * @brief Common interface for type wrappers
 * This is a pure virtual interface class for the runtime polymorphic
 * case and not needed for the static polymorphic case.
 */
class Interface {
public:
  typedef Interface self_type;

  Interface() {}
  VIRTUAL_ATTRIBUTE ~Interface() {}

  VIRTUAL_ATTRIBUTE void print() const PURE_VIRTUAL_ATTRIBUTE;
  VIRTUAL_ATTRIBUTE self_type& operator+=(int value) PURE_VIRTUAL_ATTRIBUTE;

private:
};

/**
 * @brief Wrapper class to test the virtual interface
 * The class can wrap an arbitrary type which implements operator++ and
 * the output stream operator.
 */
template<typename T>
class TypeWrapper : public Interface {
public:
  typedef TypeWrapper self_type;
  typedef T wrapped_type;

  TypeWrapper() : mMember(0), mCount(0) {}
  VIRTUAL_ATTRIBUTE ~TypeWrapper() {}

  VIRTUAL_ATTRIBUTE void print() const {
    const char* typeName = "unknown";
    if (std::is_floating_point<wrapped_type>::value)
      typeName = "float";
    if (std::is_integral<wrapped_type>::value) {
      if (std::is_signed<wrapped_type>::value)
        typeName = "signed int";
      else
        typeName = "unsigned int";
    }

    std::cout << "I'm a "
              << std::setw(12) << typeName << " type,"
              << " of size " << sizeof(wrapped_type)
              << " value " << std::setw(10) << std::setprecision(4) << mMember
              << "  - called " << std::setw(10) << mCount << " time(s)"
              << std::endl;
  }

  VIRTUAL_ATTRIBUTE self_type& operator+=(int value) {
    mMember += value;
    ++mCount;
  }

private:
  T mMember;
  int mCount;
};

/**
 * @brief Definition of the data types to be used in the test
 */
typedef boost::mpl::vector<
  TypeWrapper<int>
  ,TypeWrapper<float>
  ,TypeWrapper<short>
  ,TypeWrapper<double>
  ,TypeWrapper<char>
  ,TypeWrapper<uint64_t>
  > types;

// TODO:
// - this should be in general included in runtime_container.h
// - better explaination of what needs to be defined in the uniary function
//   object
// - the parameter in the operator should be const, but this does not compile
//   for the moment, check this
struct printer
{
  typedef void return_type;
  template<typename T>
  return_type operator()(T& me) {return (*me).print();}
};

/**
 * @brief The test loop
 */
template<typename ContainerT>
int test_loop(ContainerT& container, int nrolls) {

  system_clock::time_point refTime = system_clock::now();
  for (auto roll = 0; roll < nrolls; roll++) {
    for (auto index = 0; index < container.size(); index++) {
#ifdef STATIC_POLY
      container.apply(index, add_value<int>(1));
#else
      *(container[index])+=1;
#endif
    }
  }
  auto duration = std::chrono::duration_cast<TimeScale>(system_clock::now() - refTime);

  std::cout << "testing "
            << std::setw(10) << nrolls << " iteration(s): "
            << std::setw(10) << duration.count() << " ns"
            << std::endl;

#ifdef STATIC_POLY
  for (auto index = 0; index < container.size(); index++) {
    container.apply(index, printer());
  }
#else
  for (auto& element : container) {
    element->print();
  }
#endif

  return 0;
}

/**
 * @brief Unary function for mpl for_each loop to add pointer of new object
 * Create an object of the corresponding type. The type comes from the list
 * of types while for_each is going through the list.
 *
 * TODO:that would be probably suited for a templated lambda function, but
 * this is only supported from C++14, look into this at some time
 */
template<typename ContainerT>
struct add_object {
  add_object(ContainerT& c) : _c(c) {};
  template<typename T>
  void operator()(T&) {
    _c.push_back(new T);
  }

  ContainerT& _c;
};

int main()
{
#ifdef STATIC_POLY
  typedef create_rtc< types, RuntimeContainer<> >::type Container_t;
  Container_t container;
#else
  std::vector<Interface*> container;
  boost::mpl::for_each<types>(add_object<std::vector<Interface*>>(container));
#endif

  int nrolls = 1000000000;
#ifdef NROLLS
  nrolls = NROLLS
#endif

  test_loop(container, nrolls);

  return 0;
}
