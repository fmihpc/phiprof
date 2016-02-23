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
#include <iostream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <fstream>
#include <sstream>
#include "paralleltimertree.hpp"
#include "phiprof.hpp"

#include "mpi.h"
#ifdef _OPENMP
#include "omp.h"
#endif


using namespace std;

namespace phiprof
{
   namespace 
   {
      ParallelTimerTree parallelTimerTree; //TODO make threadprivate / one per thread or something
   }

   int initializeTimer(const string &label,const vector<string> &groups){
      return parallelTimerTree.initializeTimer(label, groups);
   }
   
//initialize a timer, with a particular label belonging to a group
//returns id of new timer. If timer exists, then that id is returned.
   int initializeTimer(const string &label,const string &group){
      //check if the global profiler is initialized
      vector<string> groups;
      groups.push_back(group);
      return parallelTimerTree.initializeTimer(label,groups);
   }

//initialize a timer, with a particular label belonging to two groups
//returns id of new timer. If timer exists, then that id is returned.
   int initializeTimer(const string &label,
                       const string &group1,
                       const string &group2
      ){
      vector<string> groups;
      groups.push_back(group1);
      groups.push_back(group2);
      return parallelTimerTree.initializeTimer(label,groups);
   }

//initialize a timer, with a particular label belonging to three groups
//returns id of new  timer. If timer exists, then that id is returned.
   int initializeTimer(const string &label,
                       const string &group1,
                       const string &group2,
                       const string &group3
      ){
      vector<string> groups;
      groups.push_back(group1);
      groups.push_back(group2);
      groups.push_back(group3);
      return parallelTimerTree.initializeTimer(label,groups);
   }

   //initialize a timer, with a particular label belonging to no group
   //returns id of new timer. If timer exists, then that id is returned.
   int initializeTimer(const string &label){
      vector<string> groups; //empty vector
      return parallelTimerTree.initializeTimer(label,groups);
   }

   bool start(int id){   
      return parallelTimerTree.start(id);
   }
   bool start(const string &label){   
      return parallelTimerTree.start(label);
   }
   
   bool stop (const string &label,
              const double workUnits,
              const string &workUnitLabel){
      return parallelTimerTree.stop(label, workUnits, workUnitLabel);
   }
   bool stop (int id,
              const double workUnits,
              const string &workUnitLabel){
      return parallelTimerTree.stop(id, workUnits, workUnitLabel);
   }

   bool stop (int id){
      return parallelTimerTree.stop(id);
   }
   bool print(MPI_Comm comm, std::string fileNamePrefix, double minFraction){
      return parallelTimerTree.print(comm, fileNamePrefix, minFraction);
   }
   

   int getId(const string &label){
      return parallelTimerTree.getId(label);
   }
   
      
   void phiprofAssert(bool condition, const string error_message, const string  file, int line ) {
#ifndef NDEBUG
      if(!condition) {
#pragma omp critical
         {
            int rank;
            MPI_Comm_rank(MPI_COMM_WORLD, &rank);
            cerr << "ASSERT ERROR on process "<< rank << ": " << error_message      
                 << ", File: " << file << ", Line: " << line
                 << endl;       
            exit(1);
         }
      }
#endif
      return;
   }
   
}


   
   
