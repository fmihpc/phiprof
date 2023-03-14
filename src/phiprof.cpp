/*
This file is part of the phiprof library

Copyright 2012, 2013, 2014, 2015 Finnish Meteorological Institute
Copyright 2015, 2016 CSC - IT Center for Science 

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
      ParallelTimerTree parallelTimerTree; 
   }

   bool initialize(){
      return parallelTimerTree.initialize();
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
   bool print(MPI_Comm comm, std::string fileNamePrefix){
      return parallelTimerTree.print(comm, fileNamePrefix);
   }
   

   int getChildId(const string &label){
      return parallelTimerTree.getChildId(label);
   }

   Timer::Timer(const int id) : id {id} {
      this->start();
   }

   Timer::Timer(const string& label, const vector<string>& groups) : Timer(initializeTimer(label, groups)) {}

   Timer::~Timer() {
      this->stop();
   }

   bool Timer::start() {
      return active ? false : (active = phiprof::start(this->id));
   }

   bool Timer::stop(const double workUnits, const string& workUnitLabel) {
      if (!active)
         return false;

      active = false;
      return phiprof::stop(id, workUnits, workUnitLabel);
   }
   
}
