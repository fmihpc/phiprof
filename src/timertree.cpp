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

#include <vector>
#include <unordered_map> //for hasher
#include <string>
#include <limits>
#include <algorithm>
#include <iostream>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "timerdata.hpp"
#include "timertree.hpp"
#include "common.hpp"

int TimerTree::numThreads = 1;
int TimerTree::thread = 0;
bool TimerTree::initialized = false;
const int maxTimers = 10000;

/*
  initialize timertree. 
  
  Sets the static thread id and count values.  Also adds the root timer.
*/
  
bool TimerTree::initialize(){
   if(!initialized) {
      std::vector<std::string> group;
      group.push_back("Total");
      //set static variables with threadcounts
      TimerData::setThreadCounts();
      TimerTree::setThreadCounts();
      currentId.resize(numThreads);
      setCurrentId(-1);
   
#pragma omp master
      {
         timers.clear();
         //mainId will be 0, parent is -1 (does not exist)
         timers.push_back(TimerData(NULL, 0, "total", group,""));
         timers[0].start();
         timers.reserve(maxTimers); //up to maxtimers we are guaranteed not to
      }
      setCurrentId(0);
      initialized=true;
   }
   return initialized;
}

void TimerTree::setCurrentId(int id){
#ifdef _OPENMP
   if(omp_in_parallel()) {
      currentId[thread] = id;
   }
   
   else {
      for(int i=0; i< numThreads;i++) {
         currentId[i] = id;
      }
   }
#else
   currentId[thread] = id;
#endif

}

//initialize a timer, with a particular label belonging to some groups
//returns id of new timer. If timer exists, then that id is returned.
//this function needs to be called by all (active) threads
int TimerTree::initializeTimer(const std::string &label, const std::vector<std::string> &groups, std::string workUnit){
   //check if the global profiler is initialized
   //master + barrier and not single to make sure one at a time is created
   int id;
#pragma omp critical(phiprof)
   {
      id = getChildId(label); //check if label exists as childtimer
      if(id < 0) {
         //does not exist, let's create it
         id = timers.size(); //id for new timer
         timers.push_back(TimerData(&(timers[currentId[thread]]), id, label, groups, workUnit));
         
#ifdef DEBUG_PHIPROF_TIMERS         
         if(timers[id].getLevel() > 10) {
            std::string label = getFullLabel(id, false);
            std::cerr << "Warning creating deep timer level " << timers[id].getLevel() << " with full label " << label<<std::endl;
         }
#endif
      }
   }
   return id;
}
   

//start timer, with label.
bool TimerTree::start(const std::string &label){
   //If the timer exists, then initializeTimer just returns its id, otherwise it is constructed.
   int newId = initializeTimer(label, std::vector<std::string>(), "");
   start(newId);
   return true;
   
}

//stop a timer defined by id
bool TimerTree::stop (int id,
                      double workUnits){
#ifdef DEBUG_PHIPROF_TIMERS         
   if(id != currentId[thread] ){
      std::cerr << "PHIPROF-ERROR: id missmatch in profile::stop Stopping "<< id <<" at level " << timers[currentId[thread]].getLevel() << std::endl;
      return false;      
   }
#endif
   int newId = timers[id].stop(workUnits);
   setCurrentId(newId);   
   return true;
}
//stop a timer defined by id
bool TimerTree::stop (int id,
                      double workUnits,
                      const std::string &workUnitLabel){
#ifdef DEBUG_PHIPROF_TIMERS         
   if(id != currentId[thread] ){
      std::cerr << "PHIPROF-ERROR: id missmatch in profile::stop Stopping "<< id <<" at level " << timers[currentId[thread]].getLevel() << std::endl;
      return false;
   }
#endif
   int newId = timers[id].stop(workUnits, workUnitLabel);
   setCurrentId(newId);
   return true;
}


bool TimerTree::stop (const std::string &label)
{
#ifdef DEBUG_PHIPROF_TIMERS         
   if(label != timers[currentId[thread]].getLabel()){
      std::cerr << "PHIPROF-ERROR: label missmatch in profile::stop Stopping "<< label << " but " << timers[currentId[thread]].getLabel()<<" or full" <<  getFullLabel(currentId[thread], true) << " is active. thread:" << thread << "currentid" << currentId[thread] <<std::endl;
//      std::cerr << "PHIPROF-ERROR: id missmatch in profile::stop Stopping "<< label << " but " << getFullLabel(currentId[thread], true) << " is active. thread:" << thread << "currentid" << currentId[thread] <<std::endl;
      return false;
   }
#endif
   setCurrentId(timers[currentId[thread]].stop());
   return true;
}


//stop with workunits
bool TimerTree::stop (const std::string &label,
                      const double workUnits,
                      const std::string &workUnitLabel){
#ifdef DEBUG_PHIPROF_TIMERS         
   if(label != timers[currentId[thread]].getLabel()){
      std::cerr << "PHIPROF-ERROR: label missmatch in profile::stop Stopping "<< label << " but " << timers[currentId[thread]].getLabel()<<"/" <<  getFullLabel(currentId[thread], true) << " is active. thread:" << thread << "currentid" << currentId[thread] <<std::endl;

      return false;
   }
#endif
   setCurrentId(timers[currentId[thread]].stop(workUnits, workUnitLabel));
   return true;
}
      
//get id number of a timer, return -1 if it does not exist
int TimerTree::getChildId(const std::string &label) const{
   //find child with this id
   for(auto &childId : timers[currentId[thread]].getChildIds() ){  
      if (timers[childId].getLabel() == label){
         return childId;
      } 
   } 
   //nothing found
   return -1;
}

double TimerTree::getTime(int id) const{
   return timers[id].getAverageTime();
}



double TimerTree::getGroupTime(std::string group, int id) const{
   double groupTime=0.0;
   for(auto &timerGroup : timers[id].getGroups()){
      if(group == timerGroup){
         groupTime = timers[id].getAverageTime();
         return groupTime; // do not collect for children when this is already in group.Avoid double counting
      }
   }
   //recursively collect time data if possibly some children are in
   //group 
   for(auto &childId : timers[id].getChildIds()){
      groupTime += getGroupTime(group, childId);
   }
   return groupTime;
}
         
//Hash value identifying all labels, groups and workunitlabels.
int TimerTree::getHash() const{
   std::string hashString = getHashString(0); //starting from root, construct a
                                         //string representing the tree.
   std::hash< std::string> hasher;
   int hashValue = (int)(hasher(hashString) % std::numeric_limits<int>::max());
   // MPI_Comm_split needs a non-zero value
   if (hashValue == 0) {
      return 1;
   } else {
      return hashValue;
   }
}
std::string TimerTree::getHashString(int id) const{
   std::string hashString;  
   //add hash values from label, workunitlabel and groups. Everything has to match.
   hashString = timers[id].getStringForHash();
   //add hash values from descendants 
   for(const auto& childId : timers[id].getChildIds() ){
      hashString += getHashString(childId);
   }
   return hashString;
}

//get full hierarchical name for a timer
//can have either the timer-label first (reverse order), or last.
std::string TimerTree::getFullLabel(int id,bool reverse) const{
   //create a label with all hierarchical levels     
   std::vector<std::string> labels;
   while(id>0){
      labels.push_back(timers[id].getLabel());
      id = timers[id].getParentId();
   }
         
   std::string fullLabel;
   if(reverse){
    
      for (auto it=labels.begin() ; it != labels.end(); ++it ){
         fullLabel += *it;
         fullLabel += "\\";   
      }
   }
   else{
      for (auto rit = labels.rbegin() ; rit != labels.rend(); ++rit ){
         fullLabel += "/";
         fullLabel += *rit;
      }
   }
   return fullLabel;
}

//reset timers to zero recursively for all descendants
void TimerTree::resetTime(double resetWallTime, int id){
   for(auto &childId : timers[id].getChildIds() ){
      resetTime(resetWallTime, childId);
   }
   timers[id].resetTime(resetWallTime);
}

//remove, e.g., printtime from timings by pushing forward start_time,
//recursive function
void TimerTree::shiftActiveStartTime(double shiftTime, int id){

   for(auto &childId : timers[id].getChildIds() ){
      shiftActiveStartTime(shiftTime, childId);
   }

   timers[id].shiftActiveStartTime(shiftTime);
}


      
