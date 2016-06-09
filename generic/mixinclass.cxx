// Demonstrator for a mixin class to combine functionality from
// orthogonal functional modules.
//
// In this example, functionality is accumulated rather then replaced
// by overloading.
//
// compilation:
// g++ -o mixinclass -std=c++11 mixinclass.cxx

#include <iostream>

template<typename T>
struct BaseFunctionality
{
  typedef T value_type;
  value_type value;
  void set(value_type v) { value = v; }
  value_type get() const { return value; }
  void print() {
    std::cout << "The base functionality" << std::endl;
  }
};

template <typename BASE, typename T = typename BASE::value_type>
struct dec : public BASE
{
  typedef T value_type;
  void print() {
    std::cout << "The decimal functionality" << std::endl;
    BASE::print();
  }
};

template <typename BASE, typename T = typename BASE::value_type>
struct hex : public BASE
{
  typedef T value_type;
  void print() {
    std::cout << "The hexadecimal functionality" << std::endl;
    BASE::print();
  }
};

template <typename BASE, typename T = typename BASE::value_type>
struct oct : public BASE
{
  typedef T value_type;
  void print() {
    std::cout << "The octal functionality" << std::endl;
    BASE::print();
  }
};


int main()
{
  hex<BaseFunctionality<int> > plainhex;
  oct< hex<BaseFunctionality<int> > > octhex;
  oct< dec< hex<BaseFunctionality<int> > > > octdechex;
  oct< dec< oct<BaseFunctionality<int> > > > octdecoct;

  std::cout << "==============================" << std::endl;
  plainhex.print();
  std::cout << "==============================" << std::endl;
  octhex.print();
  std::cout << "==============================" << std::endl;
  octdechex.print();
  std::cout << "==============================" << std::endl;
  octdecoct.print();
}
