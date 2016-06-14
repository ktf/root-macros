// Demonstrator for a mixin class to combine functionality from
// orthogonal functional modules. This is a pure theoretical example,
// the module names (hexa)decimal and octal are chosen arbitrarily.
// But this can illustrate orthogonal groups of functionality.
//
// In this example, functionality is accumulated rather then replaced
// by overloading.

#include <iostream>

class Interface {
public:
Interface() {}
~Interface() {}

virtual void print() = 0;
};

template<typename T>
struct BaseFunctionality : public Interface
{
  typedef T value_type;
  value_type value;
  void set(value_type v) { value = v; }
  value_type get() const { return value; }
  void print() {
    std::cout << "The base functionality" << std::endl;
  }
};

template <typename BASE>
struct dec : public BASE
{
  typedef typename BASE::value_type value_type;
  void print() {
    std::cout << "The decimal functionality" << std::endl;
    BASE::print();
  }
};

template <typename BASE>
struct hex : public BASE
{
  typedef typename BASE::value_type value_type;
  void print() {
    std::cout << "The hexadecimal functionality" << std::endl;
    BASE::print();
  }
};

template <typename BASE>
struct oct : public BASE
{
  typedef typename BASE::value_type value_type;
  void print() {
    std::cout << "The octal functionality" << std::endl;
    BASE::print();
  }
};
