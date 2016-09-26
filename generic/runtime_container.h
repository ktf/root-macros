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

//  @file   runtime_container.h
//  @author Matthias Richter
//  @since  2015-09-11
//  @brief  A general runtime container for a compile time sequence

// A general runtime container for a compile time sequence
// of types. A mixin class is used to represent a member of each data
// type. Every data type in the sequence describes a mixin on top of
// the previous one. The runtime container accumulates the type
// properties.

#include <iostream>
#include <iomanip>
#include <boost/mpl/equal.hpp>
#include <boost/mpl/minus.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/lambda.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/protect.hpp>
#include <boost/mpl/begin.hpp>
#include <boost/mpl/end.hpp>
#include <boost/mpl/next.hpp>
#include <boost/mpl/deref.hpp>

using namespace boost::mpl::placeholders;

/**
 * @class DefaultInterface
 * @brief The default interface for the RuntimeContainer
 *
 * The common interface for the mixin class. In order to allow entry
 * points to the different levels of the mixin, none of the interface
 * functions has to be declared virtual. The function implementation of
 * the top most mixin would be called otherwise.
 */
class DefaultInterface
{
public:
  DefaultInterface() {}
  ~DefaultInterface() {}

  void print() const {}
};

/**
 * @brief Default initializer does nothing
 */
struct default_initializer
{
  template<typename T>
  void operator()(T&) {}
};

/**
 * @brief An initializer for simple types
 * The initializer makes use of truncation for non-float types, and
 * over- and underflow to produce different values in the member
 * of the individual stages in the container.
 * - float types keep the fraction
 * - integral types truncate the fraction
 * - unsigned types undergo an underflow and produce big numbers
 * - 8 bit char produces the '*' character
 */
struct funny_initializer
{
  template<typename T>
  void operator()(T& v) {v=0; v-=214.5;}
};

/**
 * @brief Default printer prints nothing
 */
struct default_printer
{
  template<typename T>
  void operator()(const T& v, int level = -1) {}
};

/**
 * @brief Verbose printer prints level and content
 */
struct verbose_printer
{
  template<typename T>
  void operator()(const T& v, int level = -1) {
    std::cout << "RC mixin level "
              << std::setw(2)
              << level << ": " << v << std::endl;
  }
};

/**
 * @brief Setter functor, forwards to the container mixin's set function
 */
template<typename U>
class set_value {
public:
  typedef void return_type;
  typedef U value_type;

  set_value(U u) : mValue(u) {}
  template<typename T>
  return_type operator()(T& t) {
    t = mValue;
  }

private:
  set_value(); // forbidden
  U mValue;
};

/**
 * @brief Adder functor
 */
template<typename U>
class add_value {
public:
  typedef void return_type;
  typedef U value_type;

  add_value(U u) : mValue(u) {}
  template<typename T>
  return_type operator()(T& t) {
    t += mValue;
  }

private:
  add_value(); // forbidden
  U mValue;
};

/**
 * @brief Getter functor, forwards to the container mixin's get function
 *
 * TODO: make a type trait to either return t.get() if its a container
 * instance or t directly if it is the member object
 */
template<typename U>
class get_value {
public:
  typedef U return_type;
  typedef U value_type;
  class NullType {};
private:
  /* could not solve the problem that one has to instantiate Traits
     with a fixed number of template arguments where wrapped_type
     would need to be provided already to go into the specialization
  template<typename InstanceType, typename Dummy = InstanceType>
  struct Traits {
    typedef NullType container_type;
    typedef InstanceType type;
    static return_type apply(InstanceType& c) {
      std::cout << "Traits";
      return c;
    }
  };
  // specialization for container instances
  template<typename InstanceType>
  struct Traits<InstanceType, typename InstanceType::wrapped_type> {
    typedef InstanceType container_type;
    typedef typename InstanceType::wrapped_type type;
    static return_type apply(InstanceType& c) {
      std::cout << "specialized Traits";
      return c.get();
    }
  };
  */

public:
  template<typename T>
  return_type operator()(T& t) {
    return t.get();
    //return (typename Traits<T>::type)(t);
  }
};


/******************************************************************************
 * @brief apply functor to the wrapped member object in the runtime container
 * This meta function recurses through the list while incrementing the index
 * and calls the functor at the required position
 *
 * @note internal meta function for the RuntimeContainers' apply function
 */
template <
  typename _ContainerT  // container type
  , typename _IndexT    // data type of position index
  , typename _Iterator  // current iterator position
  , typename _End       // end iterator position
  , _IndexT  _Index     // current index
  , typename F          // functor
  >
struct rc_apply_at
{
  static typename F::return_type apply( _ContainerT& c, _IndexT position, F f )
  {
    if ( position == _Index ) {
      // this is the queried position, make the type cast to the current
      // stage of the runtime container and execute function for it.
      // Terminate loop by forwarding _End as _Iterator and thus
      // calling the specialization
      typedef typename boost::mpl::deref< _Iterator >::type stagetype;
      stagetype& stage = static_cast<stagetype&>(c);
      return f(stage);
    } else {
      // go to next element
      return rc_apply_at<
	_ContainerT
        , _IndexT
        , typename boost::mpl::next< _Iterator >::type
        , _End
        , _Index + 1
        , F
        >::apply( c, position, f );
    }
  }
};
// specialization: end of recursive loop, kicks in if _Iterator matches
// _End.
// here we end up if the position parameter is out of bounds
template <
  typename _ContainerT  // container type
  , typename _IndexT    // data type of position index
  , typename _End       // end iterator position
  , _IndexT  _Index     // current index
  , typename F          // functor
  >
struct rc_apply_at<_ContainerT
		   , _IndexT
		   , _End
		   , _End
		   , _Index
		   , F
		   >
{
  static typename F::return_type apply( _ContainerT& c, _IndexT position, F f )
  {
    // TODO: this is probably the place to through an exeption because
    // we are out of bound
    return typename F::return_type(0);
  }
};

/**
 * @class RuntimeContainer The base for the mixin class
 * @brief the technical base of the mixin class
 *
 * The class is necessary to provide the innermost functionality of the
 * mixin.
 *
 * The level of the mixin is encoded in the type 'level' which is
 * incremented in each mixin stage.
 */
template<typename InterfacePolicy = DefaultInterface
  , typename InitializerPolicy = default_initializer
  , typename PrinterPolicy = default_printer>
struct RuntimeContainer : public InterfacePolicy
{
  InitializerPolicy _initializer;
  PrinterPolicy     _printer;
  typedef boost::mpl::int_<-1> level;
  typedef boost::mpl::vector<>::type  types;

  void print() {
    const char* string = "base";
    _printer(string, level::value);
  }

  // not yet clear if we need the setter and getter in the base class
  // at least wrapped_type is not defined in the base
  //void set(wrapped_type) {mMember = v;}
  //wrapped_type get() const {return mMember;}

};

/**
 * @class rc_mixin Components for the mixin class
 * @brief Mixin component is used with different data types
 *
 * Each mixin component has a member of the specified type, as a simple
 * trick this member is initialized with 0 and then a negative float number
 * is subtracted. The fraction will be cut off for integral types, for
 * unsigned numbers there will be range wrap. For an 8 bit char it gives
 * the character '*'
 */
template <typename BASE, typename T>
class rc_mixin : public BASE
{
public:
  rc_mixin() : mMember() {BASE::_initializer(mMember);}

  /// each stage of the mixin class wraps one type
  typedef T wrapped_type;
  /// this is the self type
  typedef rc_mixin<BASE, wrapped_type> mixin_type;
  /// a vector of all mixin stage types so far
  typedef typename boost::mpl::push_back<typename BASE::types, mixin_type>::type types;
  /// increment the level counter
  typedef typename boost::mpl::plus< typename BASE::level, boost::mpl::int_<1> >::type level;
  void print() {
    BASE::_printer(mMember, level::value);
    BASE::print();
  }

  /// set member wrapped object
  void set(wrapped_type v) {mMember = v;}
  /// get wrapped object
  wrapped_type get() const {return mMember;}
  /// get wrapped object reference
  wrapped_type& operator*() {return mMember;}
  /// assignment operator to wrapped type
  wrapped_type& operator=(const wrapped_type& v) {mMember = v; return mMember;}
  /// type conversion to wrapped type
  operator wrapped_type() const {return mMember;}
  /// operator
  wrapped_type& operator+=(const wrapped_type& v) {mMember += v; return mMember;}
  /// operator
  wrapped_type operator+(const wrapped_type& v) {return mMember + v;}

  /// a functor wrapper dereferencing the RC container instance
  template<typename F>
  class member_apply_at {
  public:
    member_apply_at(F& f) : mFunctor(f) {}
    typedef typename F::return_type return_type;
    template<typename _T>
    typename F::return_type operator()(_T& me) {
      return mFunctor(*me);
    }
  private:
    member_apply_at(); //forbidden
    F& mFunctor;
  };

  /// apply functor to the runtime object at index
  /// TODO: there is a performance issue with this solution, introducing another
  /// level of functors makes the access much slower compared with applying to
  /// container instance and using container member functions, tested with the
  /// add_value functor and bench_runtime_container, also the actual operation
  /// needs to be checked, the result is not correct for the last check of
  /// 100000000 iterations
  /*
  template<typename F>
  typename F::return_type applyToMember(int index, F f) {
    return apply(index, member_apply_at<F>(f));
  }
  */

  /// apply functor to the runtime container at index
  template<typename F>
  typename F::return_type apply(int index, F f) {
    return rc_apply_at<
      mixin_type
      , int
      , typename boost::mpl::begin<types>::type
      , typename boost::mpl::end<types>::type
      , 0
      , F
      >::apply(*this, index, f);
  }

private:
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
