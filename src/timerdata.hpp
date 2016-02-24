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

   //
   static int setThreadCounts(){
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

   double getAverageTime() const {
      double sumTime = 0.0;
      int timedThreads = 0;
      for(int i = 0; i < time.size(); i++){
         if(count[i] > 0 || active[i]) {
            timedThreads++;
            sumTime += time[i];
            //add time uptill now for active timers
            if(active[i]) {
               sumTime += wTime()- startTime[i];
               
            }
         }
      }
      return sumTime / timedThreads;
   }

   double getAverageWorkUnits() const {
      double sumWorkUnits = 0.0;
      int timedThreads = 0;
      for(int i = 0; i < time.size(); i++){
         if(count[i] > 0 || active[i]) {
            timedThreads++;
            sumWorkUnits += workUnits[i];
         }
      }
      return sumWorkUnits / timedThreads;
   }

   int getAverageCount() const {
      int sumCount = 0.0;
      int timedThreads = 0;
      for(int i = 0; i < time.size(); i++){
         if(count[i] > 0 || active[i]) {
            timedThreads++;
            sumCount += count[i];
         }
      }
      //TODO, should return double
      return sumCount / timedThreads;
   }

   double getThreads() const {
      int timedThreads = 0;
      for(int i = 0; i < time.size(); i++){
         if(count[i] > 0 || active[i]) {
            timedThreads++;
         }
      }
      return timedThreads;
   }

   

//Hash value identifying all labels, groups and workunitlabels.
//If any std::strings differ, hash should differ. Computed recursively in the same way as prints
   int getHash() const{
      unsigned long hashValue;
      //add hash values from label, workunitlabel and groups. Everything has to match.
      hashValue=hash(label.c_str());
      hashValue+=hash(workUnitLabel.c_str());
      for (std::vector<std::string>::const_iterator g = groups.begin();g != groups.end(); ++g ) {
         hashValue+=hash((*g).c_str());
      }
      
      // MPI_Comm_split needs a non-zero value
      if (hashValue == 0) {
         return 1;
      } else {
         //we return an integer value
         return hashValue%std::numeric_limits<int>::max();
      }
   }
   
   void resetTime(double resetWallTime){
      count.assign(count.size(), 0);
      time.assign(time.size(), 0.0);
      workUnits.assign(workUnits.size(), 0.0 );
      
      for(int thread = 0; thread < active.size(); thread++) {
         if(active[thread]){
            startTime[thread] = resetWallTime;
         }
      }
   }
   
   void shiftActiveStartTime(double shiftTime){
      for(int thread = 0; thread < active.size(); thread++) {
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
   std::string workUnitLabel;   //unit for the counter workUnitCount
                                //(can be changed in stop)
   const std::vector<std::string> groups; // What user-defined groups does this timer belong to, e.g., "MPI", "IO", etc..
   std::vector<int64_t> count; //how many times have this been accumulated per thread
   std::vector<double> time; // total time accumulated per thread
   std::vector<double> startTime; //Starting time of previous start() call per thread
   std::vector<double> workUnits;        // how many units of work have we done. If -1 it is not counted, or printed
   std::vector<bool> active;
};


#endif
