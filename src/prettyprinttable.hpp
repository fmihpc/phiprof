#include <iostream>    
#include <fstream>    
#include <vector>
#include <string>


class PrettyPrintTable {
public:
   void addTitle(std::string const& title);
   void addHorizontalLine();
   void addRow();
   void addElement(std::string const& element, uint span = 1, uint indentLevel = 0);
   void addElement(int element, uint span = 1);
   void addElement(double element, uint span = 1);
   void print(std::ofstream& file, std::string const& delimeter = " | ");

private:
   const int _indentWidth=2; //how many spaces each level is indented
   const int _floatPrecision = 5; //precision of float fields;
   
   std::string title;
   std::vector<std::vector<std::pair<std::string, uint> > > table;
};

   
