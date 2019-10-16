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

#ifndef TIMERTREE_H
#define TIMERTREE_H
#include <vector>
#include <string>
#include "timerdata.hpp"
#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef _NVTX
#include "nvToolsExt.h"
#endif

class TimerTree {
public:
   /**
   * Initialize the timertree. 
   *
   * @return
   *   Returns true if the timertree started successfully.
   */
   bool initialize();
   
  /**
   * Start a profiling timer.
   *
   * This function starts a timer with a certain label (name). If
   * timer does not exist, then it is created. The timer is
   * automatically started in the current active location in the tree
   * of timers. Thus the same start command in the code can start
   * different timers, if the current active timer is different.
   *
   * @param label
   *   Name for the timer to be start. 
   * @return
   *   Returns true if timer started successfully.
   */
   bool start(const std::string &label);
   /*
    * \overload bool phiprof::start(const std::string &label)
   */
   //start timer, with id
   bool start(int id){   
      bool success=true;
#ifdef DEBUG_PHIPROF_TIMERS         
      if(id > timers.size() ) {
#pragma omp critical
         std::cerr << "PHIPROF-ERROR: id is invalid, timer does not exist "<< std::endl;
         
         return false;
      }
      std::vector<int> childIds = timers[currentId[thread]].getChildIds(); 
      if ( std::find(childIds.begin(), childIds.end(), id) == childIds.end() ) {
#pragma omp critical 
         std::cerr << "PHIPROF-ERROR for thread "<< thread<< ": id "<< id << 
            " is invalid, timer is not child of current timer "<< currentId[thread] << 
            ":" << timers[currentId[thread]].getLabel() << std::endl;
         return false;
      }
      
#endif
      //start timer (currentId = id)
      int newId = timers[id].start();
      setCurrentId(newId);

#ifdef _NVTX
      nvtxRangePush(timers[id].getLabel().c_str());
#endif

      return true;
   }


  /**
   * Stop a profiling timer.
   *
   * This function stops a timer with a certain label (name). The
   * label has to match the currently last opened timer. One can also
   * (optionally) report how many workunits was done during this
   * start-stop timed segment, e.g. GB for IO routines, Cells for
   * grid-based solvers. Note, all stops for a particular timer has to
   * report workunits, otherwise the workunits will not be reported.
   *
   * @param label 
   *   Name for the timer to be stopped.     
   * @param workunits 
   *   (optional) Default is for no workunits to be
   *   collected.Amount of workunits that was done during this timer
   *   segment. If value is negative, then no workunit statistics will
   *   be collected.
   * @param workUnitLabel
   *   (optional) Name describing the unit of the workunits, e.g. "GB", "Flop", "Cells",...
   * @return
   *   Returns true if timer stopped successfully.
   */
  bool stop (const std::string &label,
              double workUnits=-1.0,
              const std::string &workUnitLabel="");

   bool stop (const std::string &label);
   
   /**
    * 
    */
   bool stop (int id,
              double workUnits,
              const std::string &workUnitLabel);


   bool stop (int id,
              double workUnits);
   
   /**
   * Fastest stop routine for cases when no workunits are defined.
   */


   bool stop (const int id){
#ifdef DEBUG_PHIPROF_TIMERS         
      if(id != currentId[thread] ){
         std::cerr << "PHIPROF-ERROR: id missmatch in profile::stop Stopping "<< id <<" at level " << timers[currentId[thread]].getLevel() << std::endl;
         return false;
      }
#endif            

#ifdef _NVTX
      nvtxRangePop();
#endif

      int newId = timers[id].stop();
      setCurrentId(newId);

      return true;
   }
   
   



  /**
   * Initialize a timer, with a particular label   
   *
   * Initialize a timer. This enables one to define groups, and to use
   * the return id value for more efficient starts/stops in tight
   * loops. If this function is called for an existing timer it will
   * simply just return the id.
   *
   *
   * @param label
   *   Name for the timer to be created. This will not yet start the timer, that has to be done separately with a start. 
   * @param groups
   *   The groups to which this timer belongs. Groups can be used combine times for different timers to logical groups, e.g., MPI, io, compute...
   * @return
   *   The id of the timer
   */
   int initializeTimer(const std::string &label, const std::vector<std::string> &groups, std::string workUnit = "");
   void resetTime(double endPrintTime, int id=0);
   void shiftActiveStartTime(double shiftTime, int id = 0);
   

   const TimerData& operator[](std::size_t id) const{
      return timers[id];
   }
   
   std::size_t size() const{
      return timers.size();
   }
   
   double getTime(int id) const;
   int getChildId(const std::string &label) const;
   double getGroupTime(std::string group, int id) const;
   int getHash() const;
   std::string getFullLabel(int id,bool reverse=false) const;  



protected:
   std::string getHashString(int id) const;
   

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

   void setCurrentId(int newId);
   static bool initialized;
   static int numThreads;
   static int thread;
#pragma omp threadprivate(thread)

   std::vector<int> currentId;
   std::vector<TimerData> timers;

};




#endif
