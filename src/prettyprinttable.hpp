#include <iostream>    
#include <fstream>    
#include <vector>
#include <string>


class PrettyPrintTable {
public:
   void addHorizontalLine();
   void addTitle(std::string& title);
   void addElement(std::string element, uint indentLevel = 0);
   void addElement(int element);
   void addElement(double element);
   void addRow();
   void print(std::ofstream& file, std::string& delimeter);
   

private:
   const int _indentWidth=2; //how many spaces each level is indented
   const int _floatWidth=11; //width of float fields;
   const int _intWidth=6;   //width of int fields;
   const int _unitWidth=4;  //width of workunit label
   const int _levelWidth=5; //width of level label
   
   std::string title;
   std::vector<std::vector<std::string>> table;
};

   
