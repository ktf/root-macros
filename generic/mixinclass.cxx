// Demonstrator for usage of mixin classes as defined in mixinclass.h
// See header files for additional comments
//
// compilation:
// g++ -o mixinclass -std=c++11 mixinclass.cxx
//
// run:
// ./mixinclass

#include "mixinclass.h"

typedef BaseFunctionality<int> IntFunctionality;

int main()
{
  hex<IntFunctionality > plainhex;
  oct< hex<IntFunctionality > > octhex;
  oct< dec< hex<IntFunctionality > > > octdechex;
  oct< dec< oct<IntFunctionality > > > octdecoct;

  std::cout << "==============================" << std::endl;
  plainhex.print();
  std::cout << "==============================" << std::endl;
  octhex.print();
  std::cout << "==============================" << std::endl;
  octdechex.print();
  std::cout << "==============================" << std::endl;
  octdecoct.print();
}
