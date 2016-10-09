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

/// @file   importCSV.C
/// @author Matthias Richter
/// @since  2016-09-30
/// @brief  Converter from csv text file to root tree

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <exception>
#include "TTree.h"
#include "TFile.h"

enum {
  kUndefinedValue = 0,
  kIntValue,
  kFloatValue,
};

/**
 *
 * Reads csv format from standard input line by line and fills values
 * into tree. The branch configuration can be specified in the corresponding
 * argument or first line if argument NULL.
 *
 * Branch configuration format: type is either 'I' or 'F'
 * branchname1/type;branchname2/type;...;branchnameN/type
 *
 * Value line format: value is either integer or float
 * value1;value2;...;valueN
 *
 * @param treename     name of the tree
 * @param treetitle    title string of tree
 * @param branchnames  branch names and types in csv format
 * @param outputfile   name of output root file
 */
int importCSV(const char* treename = "tree"
               , const char* treetitle = "tree"
               , const char* branchnames = NULL
               , const char* outputfile = "csvtree.root"
               )
{
  std::string line;
  if (branchnames)
    // configured by parameter
    line = branchnames;
  else
    // configured from first line
    std::getline(std::cin, line);

  TTree* tree=new TTree(treename, treetitle);

  // helper struct to combine branch information in one object
  struct branchconfiguration {
    branchconfiguration(std::string& _configuration, std::string& _name, int _type)
      : configuration(_configuration), name(_name), type(_type), intval(0) {}

    std::string configuration;
    std::string name;
    int         type;
    union {
      int intval;
      float floatval;
    };
  };
  std::vector<branchconfiguration> branchconfigurations;

  {
    // scan the branch configuration line and extract properties
    // branches are created in a separate loop afterwards to make sure
    // the the pointers used as branch addresses are contant.
    std::string token;
    std::stringstream linestream(line);
    while ((std::getline(linestream, token, ';')) && !token.empty()) {
      cout << "scanning branch configuration: " << token << std::endl;
      std::stringstream tokenstream(token);
      std::string name, typestr;
      int type = kUndefinedValue;
      if ((std::getline(tokenstream, name, '/')) && (tokenstream >> typestr)
          && ((typestr == "I" && (type = kIntValue)) || (typestr == "F" && (type = kFloatValue)))) {
        //std::cout << "  name: " << name << std::endl;
        //std::cout << "  type: " << typestr << " " << type << std::endl;
      } else {
        std::cerr << "format error in branch definition '" << token << "' (expected format 'branchname/[I,F]')" << std::endl;
        return -1;
      }
      branchconfigurations.push_back(branchconfiguration(token, name, type));
    }
  }

  // now create and add all branches
  for (auto& bc : branchconfigurations) {
    cout << "adding branch: " << bc.configuration << " type " << bc.type << " " << ((void*)&bc.intval) << std::endl;
    switch (bc.type) {
    case kIntValue:
      tree->Branch(bc.name.c_str(), &bc.intval, bc.configuration.c_str());
      break;
    case kFloatValue:
      tree->Branch(bc.name.c_str(), &bc.floatval, bc.configuration.c_str());
      break;
    }
  }

  // read data lines from std input
  int index = 0;
  try {
    while ((std::getline(std::cin, line))) {
      std::string token;
      std::stringstream linestream(line);
      index = 0;
      //std::cout << "scanning: " << linestream.str() << std::endl;
      auto bc = branchconfigurations.begin();
      std::getline(linestream, token, ';');
      while (!token.empty()) {
        switch (bc->type) {
        case kIntValue:
          bc->intval = std::stoi(token);
          //std::cout << "  branch " << bc->name << " value " << bc->intval << " " << ((void*)&bc->intval) << std::endl;
          break;
        case kFloatValue:
          bc->floatval = std::stof(token);
          //std::cout << "  branch " << bc->name << " value " << bc->floatval << " " << ((void*)&bc->floatval) << std::endl;
          break;
        }
        std::getline(linestream, token, ';');
        if (++bc == branchconfigurations.end()) break;
        ++index;
      }
      tree->Fill();
    }
  }
  catch (std::exception& e) {
    std::cout << e.what()
              << "format error in line '" << line << "'" << " at position " << index << std::endl;
    return -1;
  }

  // write file
  TFile* of=TFile::Open(outputfile, "RECREATE");
  if (!of || of->IsZombie()) {
    cerr << "can not open file " << outputfile << endl;
    return -1;
  }

  of->cd();
  if (tree) {
    tree->Print();
    tree->Write();
  }

  of->Close();

  return 0;
}
