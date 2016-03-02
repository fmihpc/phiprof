#include <utility>      // std::pair, std::make_pair
#include <iostream>    
#include <sstream>    
#include <iomanip>    
#include <vector>
#include <string>
#include <algorithm>
#include "prettyprinttable.hpp"

void PrettyPrintTable::addTitle(std::string const& newTitle){
   title = newTitle;
}


void PrettyPrintTable::addElement(std::string const& element, uint span, uint indentLevel ){
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


void PrettyPrintTable::addElement(float element, uint span){
   addElement((double)element, span);
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
   addRow();
   addElement("---"); //mark horizontal line
   addRow();
}




void PrettyPrintTable::addRow(){
   table.push_back(std::vector<std::pair<std::string, uint> >());
}


void PrettyPrintTable::print(std::ofstream& output, std::string const& delimeter){
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

   //Compute width of each column. Spanned cells are divided over its
   //columns equally 
   columnWidths.resize(nColumns);
   for(auto &row: table){
      uint countedColumns = 0;
      for(int j = 0; j <  row.size(); j++){
         uint span = row[j].second;
         uint requiredWidth = row[j].first.length(); //how much space
                                                     //we need for cell
         //compute how much is there in the merged columns
         uint availableWidth = (span - 1 ) * delimeter.length(); 
         for(uint col = countedColumns; col  < countedColumns + span; col++ ){
            availableWidth += columnWidths[col];
         }
         if(availableWidth < requiredWidth) {
            //not space for merged cell, add evenly width to the
            //columns, with 1 more to the firsts if it cannot be
            //divided equally
            for(uint col = countedColumns; col  < countedColumns + span; col ++ ){
               columnWidths[col] += (requiredWidth - availableWidth) / span;
               if (col - countedColumns < (requiredWidth - availableWidth) % span){
                  columnWidths[col]++; 
               }
            }
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
      if(row.size() == 0) {
         continue; //skip empty rows
      }
      
      if(row.size() == 1 && row[0].first=="---"){
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


      
      
      
