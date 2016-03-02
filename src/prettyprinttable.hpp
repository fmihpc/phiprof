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

   
