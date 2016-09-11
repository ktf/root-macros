//-*- Mode: C++ -*-

#ifndef RUNTIME_CONTAINER_H
#define RUNTIME_CONTAINER_H
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

//  @file   Runtime_Container.h
//  @author Matthias Richter
//  @since  2015-09-11
//  @brief  Demonstrator for a runtime container for a compile time sequence

// Demonstrator for a runtime container for a compile time sequence
// of types. A mixin class is used to represent a member of each data
// type. Every data type in the sequence describes a mixin on top of
// the previous one. The runtime container accumulates the type
// properties.

#include <iostream>
#include <boost/mpl/equal.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/lambda.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/protect.hpp>

using namespace boost::mpl::placeholders;

/**
 * @class RuntimeContainer
 * @brief A runtime container for a compile time sequence of data types
 *
 * The common interface for the mixin class. In order to allow entry
 * points to the different levels of the mixin, none of the interface
 * functions has to be declared virtual. The function implementation of
 * the top most mixin would be called otherwise.
 */
class RuntimeContainer
{
public:
  RuntimeContainer() {}
  ~RuntimeContainer() {}

  void print() const {}
};

/**
 * @class rc_base The base for the mixin class
 * @brief the technical base of the mixin class
 *
 * The class is necessary to provide the innermost functionality of the
 * mixin. The template parameter is purely theoretically and not used
 * at all for the functionality. Its a placeholder if a templated
 * version is needed at some point.
 *
 * The level of the mixin is encoded in the type 'level' which is
 * incremented in each mixin stage.
 */
template<typename T>
struct rc_base : public RuntimeContainer
{
  typedef T value_type;
  typedef boost::mpl::int_<-1> level;
  typedef boost::mpl::vector<>::type  mixin_types;
  value_type value;
  void set(value_type v) { value = v; }
  value_type get() const { return value; }
  void print() {
    std::cout << "RC base" << std::endl;
  }
};

/**
 * @class rc_mixin Components for the mixin class
 * @brief Mixin component is used with different data types
 *
 * Each mixin component has a member of the specified type, as a simple
 * trick this member is initialised with 0 and then a negative float number
 * is subtracted. The fraction will be cut off for integral types, for
 * unsigned numbers there will be range wrap. For an 8 bit char it gives
 * the character '*'
 */
template <typename BASE, typename T>
struct rc_mixin : public BASE
{
  rc_mixin() : mMember(0) {mMember-=214.5;}
  typedef typename BASE::value_type value_type;
  /// each stage of the mixin class wraps one type
  typedef T wrapped_type;
  /// this is the self type
  typedef rc_mixin<BASE, wrapped_type> mixin_type;
  /// a vector of all mixin stage types so far
  typedef typename boost::mpl::push_back<typename BASE::mixin_types, mixin_type>::type mixin_types;
  /// increment the level counter
  typedef typename boost::mpl::plus< typename BASE::level, boost::mpl::int_<1> >::type level;
  void print() {
    std::cout << "RC mixin level " << level::value << ": " << mMember << std::endl;
    BASE::print();
  }
  T mMember;
};

typedef typename boost::mpl::lambda< rc_mixin<_1, _2> >::type apply_rc_mixin;

/**
 * @brief check the mixin level to be below specified level
 *
 * @note: the number is specified as a type, e.g. boost::mpl:int_<3>
 */
template< typename T, typename N > struct rtc_less
: boost::mpl::bool_<(T::level::value < boost::mpl::minus<N, boost::mpl::int_<1>>::value) > {};

template< typename T, typename N > struct rtc_equal
: boost::mpl::bool_<boost::mpl::equal<typename T::wrapped_type, N>::type> {};

/**
 * @brief create the runtime container
 *
 * Usage: typedef create_rtc<types, base>::type container;
 */
template<typename Types, typename Base, typename N = boost::mpl::size<Types>>
struct  create_rtc
{
  typedef typename boost::mpl::lambda<
  typename boost::mpl::fold<
  Types
  , Base
  , boost::mpl::if_<
      rtc_less<_1, N >
      , boost::mpl::apply2< boost::mpl::protect<apply_rc_mixin>::type, _1, _2 >
      , boost::mpl::identity<_1>
      >
    >::type
  >::type type;
};

/**
 * @brief create a vector of mixin types
 *
 * Usage: typedef create_rtc_types<types, base>::type container_types;
 */
template<typename Types, typename Base, typename N = boost::mpl::size<Types>>
struct create_rtc_types
{
  typedef typename boost::mpl::fold<
    boost::mpl::range_c<int, 0, N::value>
    , boost::mpl::vector< >
    , boost::mpl::push_back<_1, create_rtc<Types , Base , boost::mpl::plus<_2, boost::mpl::int_<1>>>>
    >::type type;
};

#endif
