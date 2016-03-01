#include <iostream>    
#include <fstream>    
#include <vector>
#include <string>


class PrettyPrintTable {
public:
   void addHorizontalLine();
   void addTitle(std::string& title);
   void addElement(std::string element, uint span = 1, uint indentLevel = 0);
   void addElement(int element, uint span = 1);
   void addElement(double element, uint span = 1);
   void addRow();
   void print(std::ofstream& file, std::string& delimeter);
   

private:
   const int _indentWidth=2; //how many spaces each level is indented
   const int _floatPrecision = 5; //precision of float fields;
   
   std::string title;
   std::vector<std::vector<std::pair<std::string, uint> > > table;
};

   
