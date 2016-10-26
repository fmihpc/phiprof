/*
This file is part of the phiprof library

Copyright 2011, 2012 Finnish Meteorological Institute

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

#include <string>
#include <vector>
#include <map>
#include "mpi.h"
using namespace std;

namespace phiprof
{
   bool initialize(){return true;}

   bool start(int id){return true;}
   bool start(const string &label){return true;}

   bool stop (int id) {return true;}
   bool stop (int id,double workUnits, const string &workUnitLabel){return true;}
   bool stop (const string &label,double workUnits, const string &workUnitLabel){return true;}

   int getChildId(const std::string &label) {return 0;}

   int initializeTimer(const string &label,const vector<string> &groups) { return 0;}
   int initializeTimer(const string &label){return 0;}
   int initializeTimer(const string &label,const string &group1){return 0;}
   int initializeTimer(const string &label,const string &group1,const string &group2){return 0;}
   int initializeTimer(const string &label,const string &group1,const string &group2,const string &group3){return 0;}

   bool print(MPI_Comm comm,std::string fileNamePrefix){return true;}
   

}

