/*
This file is part of the phiprof library

Copyright 2015, 2016 CSC - IT Center for Science 

Phiprof is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <iostream>    
#include <fstream>    
#include <sstream>    
#include <vector>
#include <string>


class PrettyPrintTable {
public:
   void addTitle(std::string const& title);
   void addHorizontalLine();
   void addRow();
   void addElement(std::string const& element, uint span = 1, uint indentLevel = 0);
   template<typename T> void addElement(T element, uint span = 1){
      std::stringstream buffer;
      buffer << element;
      if(table.size() == 0) {
         addRow();
      }
      table.back().push_back(std::make_pair(buffer.str(), span));
   }
   

   void addElement(double element, uint span = 1);
   void addElement(float element, uint span = 1);
   void print(std::ofstream& file, std::string const& delimeter = " | ");

private:
   const int _indentWidth=2; //how many spaces each level is indented
   const int _floatPrecision = 4; //precision of float fields;
   
   std::string title;
   std::vector<std::vector<std::pair<std::string, uint> > > table;
};

   
