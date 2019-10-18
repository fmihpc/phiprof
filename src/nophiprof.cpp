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

   bool start([[maybe_unused]] int id){return true;}
   bool start([[maybe_unused]] const string &label){return true;}

   bool stop ([[maybe_unused]] int id) {return true;}
   bool stop ([[maybe_unused]] int id, [[maybe_unused]] double workUnits, [[maybe_unused]] const string &workUnitLabel){return true;}
   bool stop ([[maybe_unused]] const string &label, [[maybe_unused]] double workUnits, [[maybe_unused]] const string &workUnitLabel){return true;}

   int getChildId([[maybe_unused]] const std::string &label) {return 0;}

   int initializeTimer([[maybe_unused]] const string &label, [[maybe_unused]] const vector<string> &groups) { return 0;}
   int initializeTimer([[maybe_unused]] const string &label){return 0;}
   int initializeTimer([[maybe_unused]] const string &label, [[maybe_unused]] const string &group1){return 0;}
   int initializeTimer([[maybe_unused]] const string &label, [[maybe_unused]] const string &group1, [[maybe_unused]] const string &group2){return 0;}
   int initializeTimer([[maybe_unused]] const string &label, [[maybe_unused]] const string &group1, [[maybe_unused]] const string &group2, [[maybe_unused]]const string &group3){return 0;}

   bool print([[maybe_unused]] MPI_Comm comm, [[maybe_unused]] std::string fileNamePrefix){return true;}
   

}

