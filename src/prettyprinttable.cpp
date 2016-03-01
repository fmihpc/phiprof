#include <iostream>    
#include <sstream>    
#include <iomanip>    
#include <vector>
#include <string>
#include <algorithm>
#include "prettyprinttable.hpp"

void PrettyPrintTable::addTitle(std::string& newTitle)
{
   title = newTitle;
}


void PrettyPrintTable::addElement(std::string element, uint indentLevel ){
   std::stringstream buffer;
   buffer << std::left;
   for(int i = 0; i < _indentWidth * indentLevel; i++)
      buffer <<" ";
   buffer << element;
   table.back().push_back(buffer.str());
}

void PrettyPrintTable::addElement(int element){
   std::stringstream buffer;
   // buffer <<  std::setw(_intWidth);
   buffer << element;
   table.back().push_back(buffer.str());
}

void PrettyPrintTable::addElement(double element){
   std::stringstream buffer;
   //make sure we use default floats, and not fixed or other format
   buffer << std::setiosflags( std::ios::floatfield );
   //set float precision, 6 is needed for ".", "e+xx" and a space
   buffer << std::setprecision(_floatWidth-6); 
//   buffer << std::setw(_floatWidth);
   buffer << element;
   table.back().push_back(buffer.str());
}


void PrettyPrintTable::addHorizontalLine(){
   //we mark a line via empty row
   addRow();
   addRow();
}




void PrettyPrintTable::addRow(){
   table.push_back(std::vector<std::string>());
}

void PrettyPrintTable::print(std::ofstream& output, std::string& delimeter){
   int nColumns;
   std::vector<int> columnWidths;
   
   nColumns=0;   
   for(int i=0; i <  table.size();i++){
      nColumns = std::max(nColumns, (int) (table[i].size()));
   }
   columnWidths.resize(nColumns);
   for(int i = 0; i <  table.size(); i++){
      for(int j = 0; j <  table[i].size(); j++){
         columnWidths[j] = std::max(columnWidths[j], (int)(table[i][j].length()) );
      }
      
   }
   int totWidth = 0;
   for(int j = 0; j <  nColumns; j++){
      totWidth += columnWidths[j] + delimeter.length();
   }
   
   
   output << std::right;

   if(title.length() > 0) {
      output << "\n";
      output << std::setw( (totWidth+ title.length())/2);
      output << title;
      output << "\n";
      
   }
   output << std::left;
   for(int i = 0; i <  table.size(); i++){
      if(table[i].size() == 0){
         output << std::setw(1);
         for (int j = 0; j < totWidth; j++) {
            output << "-";
         }
      }
      else{
         
         for(int j = 0; j < nColumns; j++){
            if( j < table[i].size() )
               output << std::setw( columnWidths[j] ) << table[i][j];
            else
               output << std::setw( columnWidths[j] ) << "";
            output << delimeter;
         }
      }
      
      output<< "\n";
      
   }
   
}


      
      
      
