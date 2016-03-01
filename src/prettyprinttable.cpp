#include <utility>      // std::pair, std::make_pair
#include <iostream>    
#include <sstream>    
#include <iomanip>    
#include <vector>
#include <string>
#include <algorithm>
#include "prettyprinttable.hpp"

void PrettyPrintTable::addTitle(std::string& newTitle){
   title = newTitle;
}


void PrettyPrintTable::addElement(std::string element, uint span, uint indentLevel ){
   std::stringstream buffer;
   buffer << std::left;
   for(int i = 0; i < _indentWidth * indentLevel; i++)
      buffer << " ";
   buffer << element;
   if(table.size() == 0) {
      addRow();
   }
   table.back().push_back(std::make_pair(buffer.str(), span));
}

void PrettyPrintTable::addElement(int element, uint span){
   std::stringstream buffer;
   buffer << element;
   if(table.size() == 0) {
      addRow();
   }
   table.back().push_back(std::make_pair(buffer.str(), span));
}

void PrettyPrintTable::addElement(double element, uint span){
   std::stringstream buffer;
   //make sure we use default floats, and not fixed or other format
   buffer << std::setiosflags( std::ios::floatfield );
   buffer << std::setprecision(_floatPrecision); 
   buffer << element;
   if(table.size() == 0) {
      addRow();
   }
   table.back().push_back(std::make_pair(buffer.str(), span));
}


void PrettyPrintTable::addHorizontalLine(){
   //we mark a line via empty row
   addRow();
   addRow();
}




void PrettyPrintTable::addRow(){
   table.push_back(std::vector<std::pair<std::string, uint> >());
}


void PrettyPrintTable::print(std::ofstream& output, std::string& delimeter){
   uint nColumns;
   std::vector<uint> columnWidths;
   
   //How many columns
   nColumns = 0;   
   for(auto &row: table){
      uint rowColumns = 0;
      for(auto &cell: row){
         rowColumns +=  cell.second;
      }
      nColumns = std::max(nColumns, rowColumns);
   }

   //get width of each column. Spanned cells are divided over its
   //columns equally (might not always give the tightest fit, TODO)
   columnWidths.resize(nColumns);
   for(auto &row: table){
      uint countedColumns = 0;
      for(int j = 0; j <  row.size(); j++){
         uint span = row[j].second;
         for(uint col = countedColumns; col  < countedColumns + span; col ++ ){
            uint columnWidth = (row[j].first.length() + (span - 1) * delimeter.length()) / span;
            columnWidths[col] = std::max(columnWidths[col], columnWidth);
         }
         countedColumns += span;
      }

      
   }
   
   //get total width of the table
   uint totWidth = delimeter.length();
   for(auto &width: columnWidths) {
      totWidth += width + delimeter.length();
   }

   //Output title
   if(title.length() > 0) {
      output << std::right;
      output << "\n";
      output << std::setw( (totWidth+ title.length())/2);
      output << title;
      output << "\n";
      
   }
   //Output the table
   output << std::left;
   for(auto &row: table){
      if(row.size() == 0){
         //empty row => print line
         output << std::setw(1);
         for (int j = 0; j < totWidth; j++) {
            output << "-";
         }
      }
      else{
         output << delimeter;
         uint printedColumns = 0;
         for(int j = 0; j < row.size(); j++){
            uint span = row[j].second;
            uint width = 0;
            for(uint col = printedColumns; col < printedColumns + span; col++) {
               width += columnWidths[col];
            }
            width += (span - 1) * delimeter.length();
            if( span > 1) {
               //print merged cells as centered
               uint indent = (width - row[j].first.length())/2;
               for(uint ind = 0; ind < indent; ind++){
                  output << std::setw(1) << " ";
               }
               output << std::setw(width - indent) << row[j].first;
            }
            else {
               output << std::setw(width) << row[j].first;
            }
            
            output << delimeter;
            printedColumns += span;
         }

         for(int j = printedColumns; j < nColumns; j++){
            output << std::setw( columnWidths[j] ) << "";
            output << delimeter;
         }
      }
      output<< "\n";
   }
   
}


      
      
      
