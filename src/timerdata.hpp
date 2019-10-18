/*
This file is part of the phiprof library

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

#ifndef TIMERDATA_H
#define TIMERDATA_H
#include <iostream>
#include <vector>
#include <string>
#include <stdint.h>
#include <omp.h>
#include <limits>
#include "common.hpp"



class TimerData {
public:
   //threadcounts should be set before creating any objects
   TimerData(TimerData* parentTimer,
             const int &id, 
             const std::string &label, 
             const std::vector<std::string> &groups, 
             const std::string &workUnitLabel) : id(id), label(label), groups(groups), workUnitLabel(workUnitLabel) {
      if(parentTimer != NULL) {
         parentId = parentTimer->id;
         level = parentTimer->level + 1;
         //add timer to parentTimer
         parentTimer->childIds.push_back(id);
      }
      else { //this is the special case when one adds a root timer
         parentId = -1;
         level = 0;
      }
      count.assign(numThreads, 0);
      time.assign(numThreads, 0.0);
      startTime.assign(numThreads, -1);
      workUnits.assign(numThreads, 0.0 );
      active.assign(numThreads, false);
   }


   static void setThreadCounts(){
#ifdef _OPENMP
#pragma omp single
      numThreads = omp_get_max_threads();
      if(omp_in_parallel()) {
         thread = omp_get_thread_num();
      }
      else{
#pragma omp parallel
         thread = omp_get_thread_num();
      }
#else
      numThreads = 1;
      thread = 0;
#endif
   }

   int start() {
      startTime[thread] = wTime();
      active[thread] = true;
      return id;
   }

   int stop(){
      time[thread] += (wTime() - startTime[thread]);
      count[thread]++;
      active[thread]=false;
      return parentId;
   }

   int stop(double addWorkUnits){
      double stopTime = wTime();
      workUnits[thread] += addWorkUnits;
      time[thread] += (stopTime - startTime[thread]);
      count[thread]++;
      active[thread]=false;
      return parentId;
   }

   int stop(double addWorkUnits, const std::string &addWorkUnitLabel){
      double stopTime=wTime();
      
      if(count[thread]==0){ //set workUnitLabel the first time, the
                            //rest of the time adding it has no
                            //impact. This has a data race
                            //vs. threads, so if many set it the end
                            //value is undefined (the last one)
         workUnitLabel = addWorkUnitLabel;
      }
      workUnits[thread] += addWorkUnits;
      time[thread] += (stopTime - startTime[thread]);
      count[thread]++;
      active[thread]=false;
      return parentId;
   }


   void getTimeStatistics(double &ave, double &max, double &min, int &nThreads) const {
      max = 0;
      min = std::numeric_limits<double>::max() ;
      double sum = 0;
      nThreads = 0;
      
      for(uint i = 0; i < time.size(); i++){
         if(count[i] > 0 || active[i]) {
            nThreads++;
            double timerTime = time[i] + (active[i] ? wTime()- startTime[i] : 0.0);
            max = std::max(timerTime, max);
            min = std::min(timerTime, min);
            sum += timerTime;
         }
      }

      if(nThreads == 0)
         ave = 0.0;
      else
         ave =  sum / nThreads;
   }
   

   double getTimeImbalance() const {
      double max, min, ave;
      int nThreads;
      getTimeStatistics(ave, max, min, nThreads);
      if ( nThreads > 1) {
         return  (max- ave)/max * nThreads / (nThreads - 1) ;
      }
      else {
         return 1.0;
      }
   }
   
   double getAverageTime() const {
      double max, min, ave;
      int nThreads;
      getTimeStatistics(ave, max, min, nThreads);
      return ave;
   }
      

   int64_t getAverageCount() const {
      int64_t sumCount = 0.0;
      int timedThreads = 0;
      for(uint i = 0; i < time.size(); i++){
         if(count[i] > 0 || active[i]) {
            timedThreads++;
            sumCount += count[i];
         }
      }
      //TODO, should return double
      if (timedThreads > 0)
         return sumCount / timedThreads;
      else
         return 0;
   }

   double getAverageWorkUnits() const{
      double sumWorkUnits = 0.0;
      int timedThreads = 0;
      for(uint i = 0; i < time.size(); i++){
         if(count[i] > 0 || active[i]) {
            timedThreads++;
            sumWorkUnits += workUnits[i];
         }
      }
      if (timedThreads > 0)
         return sumWorkUnits / timedThreads;
      else
         return 0;
   }


   double getThreads() const {
      int timedThreads = 0;
      for(uint i = 0; i < time.size(); i++){
         if(count[i] > 0 || active[i]) {
            timedThreads++;
         }
      }
      return timedThreads;
   }

   

//String with label, groups and workunitlabel.
   std::string getStringForHash() const{
      std::string hashString = label;
      hashString += workUnitLabel;
      for (const auto& g: groups) {
         hashString += g;
      }
      return hashString;
   }
   
   
   void resetTime(double resetWallTime){
      count.assign(count.size(), 0);
      time.assign(time.size(), 0.0);
      workUnits.assign(workUnits.size(), 0.0 );
      
      for(uint thread = 0; thread < active.size(); thread++) {
         if(active[thread]){
            startTime[thread] = resetWallTime;
         }
      }
   }
   
   void shiftActiveStartTime(double shiftTime){
      for(uint thread = 0; thread < active.size(); thread++) {
         if(active[thread]){
            startTime[thread] += shiftTime;
         }
      }
   }

   const std::string& getLabel() const { return label;}
   const int& getId() const { return id;}
   const int& getLevel() const { return level;}
   const int& getParentId() const { return parentId;}
   const std::vector<int>& getChildIds() const { return childIds;}
   const std::string& getWorkUnitLabel() const { return workUnitLabel;}
   const std::vector<std::string>& getGroups() const { return groups;}
   
private:
   const int id; // unique id identifying this timer (index for timers)
   const std::string label;          //print label 
   static int numThreads;
   static int thread;
#pragma omp threadprivate(thread)
   
   int level;  //what hierarchy level
   int parentId;  //key of parent (id)
   std::vector<int> childIds; //children of this timer
   const std::vector<std::string> groups; // What user-defined groups does this timer belong to, e.g., "MPI", "IO", etc..
   std::string workUnitLabel;   //unit for the counter workUnitCount
                                //(can be changed in stop)

   std::vector<int64_t> count; //how many times have this been accumulated per thread
   std::vector<double> time; // total time accumulated per thread
   std::vector<double> startTime; //Starting time of previous start() call per thread
   std::vector<double> workUnits;        // how many units of work have we done. If -1 it is not counted, or printed
   std::vector<bool> active;
};


#endif
