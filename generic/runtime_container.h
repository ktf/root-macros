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

/**
 * @class RuntimeContainer
 * @brief A runtime container for a compile time sequence of data types
 */
class RuntimeContainer {
public:
RuntimeContainer() {}
~RuntimeContainer() {}

 virtual void print() = 0;
};

/**
 * @class rc_base The base for the mixin class
 * @brief The main purpose is the provide the interface to the mixin
 */
template<typename T>
struct rc_base : public RuntimeContainer
{
  typedef T value_type;
  value_type value;
  void set(value_type v) { value = v; }
  value_type get() const { return value; }
  void print() {
    std::cout << "RCBase" << std::endl;
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
 * the charavter '*'
 */
template <typename BASE, typename T>
struct rc_mixin : public BASE
{
  rc_mixin() : mMember(0) {mMember-=214.5;}
  typedef typename BASE::value_type value_type;
  typedef T wrapped_type;
  typedef rc_mixin<BASE, wrapped_type> mixin_type;
  void print() {
    std::cout << "RC mixin  " << mMember << std::endl;
    BASE::print();
  }
  T mMember;
};

#endif
