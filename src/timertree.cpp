#include <vector>
#include <string>
#include <limits>
#include <algorithm>
#include <time.h>
#include "timerdata.h"
#include "timertree.hpp"

//time struct to get wall time
struct timespec t;

//initialize profiler, called by first start/initializeTimer. This adds the root timer
TimerTree::TimerTree(){
   std::vector<std::string> group;
   group.push_back("Total");
   //no timer yet
   currentId=-1;
   //mainId will be 0, parent is -1 (does not exist)
   int id=constructTimer("total",-1,group);
   //start root timer, is stopped in print.      
   start(id);
}



//djb2 hash function copied from
//http://www.cse.yorku.ca/~oz/hash.html
unsigned long TimerTree::hash(const char *str)
{
   unsigned long hash = 5381;
   int c;
   while ( (c = *str++) )
      hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
   return hash;
}




//initialize a timer, with a particular label belonging to some groups
//returns id of new timer. If timer exists, then that id is returned.
int TimerTree::initializeTimer(const std::string &label,const std::vector<std::string> &groups){
   //check if the global profiler is initialized

   int id=getId(label); //check if it exists
   if(id>=0)
      //do nothing if it exists      
      return id; 
   else
      //create new timer if it did not exist
      return constructTimer(label,currentId,groups);
}




double TimerTree::getTime(int id){
   double time;
   time=timers[id].time;
   //add time uptill print for active timers
   if(timers[id].active)
      time+=wTime()-timers[id].startTime;
   return time;
}

   
//start timer, with id
bool TimerTree::start(int id){   
   bool success=true;
#ifdef DEBUG_PHIPROF_TIMERS
   if(currentId!=timers[id].parentId){
      cerr << "PHIPROF-ERROR: Starting timer that is not a child of the current profiling region" <<endl;
      success= false;
      return success;        
   }
#endif
   currentId=id;      
   //start timer
   timers[currentId].startTime=wTime();
   timers[currentId].active=true;
   return success;        
}

//start timer, with label
bool TimerTree::start(const std::string &label){
   //If the timer exists, then initializeTimer just returns its id, otherwise it is constructed.
   //Make the timer the current one
   currentId=initializeTimer(label, std::vector<std::string>() );
   //start timer
   timers[currentId].startTime=wTime();
   timers[currentId].active=true;
   return true;        
}

//stop with workunits
bool TimerTree::stop (const std::string &label,
                      const double workUnits,
                      const std::string &workUnitLabel){
   bool success=true;
#ifdef DEBUG_PHIPROF_TIMERS         
   if(label != timers[currentId].label ){
      cerr << "PHIPROF-ERROR: label missmatch in profile::stop  when stopping "<< label <<
         ". The started timer is "<< timers[currentId].label<< " at level " << timers[currentId].level << endl;
      success=false;
      return success;
   }
#endif
   success=stop(currentId, workUnits, workUnitLabel);
   return success;
}

//stop a timer defined by id
bool TimerTree::stop (int id,
                      double workUnits,
                      const std::string &workUnitLabel){
   bool success=true;
   double stopTime=wTime();
#ifdef DEBUG_PHIPROF_TIMERS         
   if(id != currentId ){
      cerr << "PHIPROF-ERROR: id missmatch in profile::stop Stopping "<< id <<" at level " << timers[currentId].level << endl;
      success=false;
      return success;
   }
#endif
   //handle workUnits for timers               
   if(timers[currentId].count!=0){
      //if this, or a previous, stop did not include work units then do not add them
      //work units have to be defined for all stops with a certain (full)label
      if(workUnits<0 || timers[currentId].workUnits<0){
         timers[currentId].workUnits=-1;
      }
      else{
         timers[currentId].workUnits+=workUnits;
      }
   }

   else{
      //firsttime, initialize workUnit stuff here
      if(workUnits>=0.0 ){
         //we have workUnits for this counter
         timers[currentId].workUnits=workUnits;
         timers[currentId].workUnitLabel=workUnitLabel;
      }
      else{
         // no workUnits for this counter
         timers[currentId].workUnits=-1.0;
      }
   }
   //stop timers 
   timers[currentId].time+=(stopTime-timers[currentId].startTime);
   timers[currentId].count++;
   timers[currentId].active=false;
   //go down in hierarchy    
   currentId=timers[currentId].parentId;
   return success;
}
   


//stop a timer defined by id
bool TimerTree::stop (int id) {
   bool success=true;
   double stopTime=wTime();
#ifdef DEBUG_PHIPROF_TIMERS         
   if(id != currentId ){
      cerr << "PHIPROF-ERROR: id missmatch in profile::stop Stopping "<< id <<" at level " << timers[currentId].level << endl;
      success=false;
      return success;
   }
#endif            
   //stop timers 
   timers[currentId].time+=(stopTime-timers[currentId].startTime);
   timers[currentId].count++;
   timers[currentId].active=false;
   //go down in hierarchy    
   currentId=timers[currentId].parentId;

   return success;
}
      
//get id number of a timer, return -1 if it does not exist
int TimerTree::getId(const std::string &label){
   //find child with this id
   int childId=-1;
   for(unsigned int i=0;i<timers[currentId].childIds.size();i++) {
      if (timers[timers[currentId].childIds[i]].label==label){
         childId=timers[currentId].childIds[i];
         break;
      }
   }
   return childId;
}

double TimerTree::getGroupTime(std::string group, int id){
   double groupTime=0.0;
   for (std::vector<std::string>::const_iterator g = timers[id].groups.begin(); g != timers[id].groups.end(); ++g ) {
      if(group==*g){
         groupTime=getTime(id);
         return groupTime; // do not collect for children when this is already in group.Avoid double counting
      }
   }
   //recursively collect time data   
   for(unsigned int i=0;i<timers[id].childIds.size();i++){
      groupTime+=getGroupTime( group, timers[id].childIds[i]);
   }
   return groupTime;
}
      


         
//Hash value identifying all labels, groups and workunitlabels.
//If any std::strings differ, hash should differ. Computed recursively in the same way as prints
int TimerTree::getHash(int id){
   unsigned long hashValue;
   //add hash values from label, workunitlabel and groups. Everything has to match.
   hashValue=hash(timers[id].label.c_str());
   hashValue+=hash(timers[id].workUnitLabel.c_str());
   for (std::vector<std::string>::const_iterator g = timers[id].groups.begin();g != timers[id].groups.end(); ++g ) {
      hashValue+=hash((*g).c_str());
   }
         
   for(unsigned int i=0;i<timers[id].childIds.size();i++){
      hashValue+=getHash(timers[id].childIds[i]);
   }
         
         
   // MPI_Comm_split needs a non-zero value
   if (hashValue == 0) {
      return 1;
   } else {
      //we return an integer value
      return hashValue%std::numeric_limits<int>::max();
   }
}

//get full hierarchical name for a timer
//can have either the timer-label first (reverse order), or last.
std::string TimerTree::getFullLabel(int id,bool reverse){
   //create a label with all hierarchical levels     
   std::vector<std::string> labels;
   while(id>0){
      labels.push_back(timers[id].label);
      id=timers[id].parentId;
   }
         
   std::string fullLabel;
   if(reverse){
      std::vector<std::string>::iterator it;
      for ( it=labels.begin() ; it != labels.end(); ++it ){
         fullLabel+=*it;
         fullLabel+="\\";   
      }
   }
   else{
      std::vector<std::string>::reverse_iterator rit;
      for ( rit=labels.rbegin() ; rit != labels.rend(); ++rit ){
         fullLabel+="/";
         fullLabel+=*rit;
      }
   }
   return fullLabel;
}

//reset logtimes in timers to zero.
void TimerTree::resetTime(double endPrintTime, int id=0){
   timers[id].time=0;
   timers[id].count=0;
   timers[id].workUnits=0;
                     
   if(timers[id].active){
      timers[id].startTime=endPrintTime;
   }

   for(unsigned int i=0;i<timers[id].childIds.size();i++){
      resetTime(endPrintTime,timers,timers[id].childIds[i]);
   }
}            
      

         



//Private



int TimerTree::constructTimer(const std::string &label,int parentId,const std::vector<std::string> groups){
   TimerData timerData;
   timerData.label=label;
   timerData.groups=groups;
   timerData.id=timers.size(); //will be added last to timers vector
   timerData.parentId=parentId;	 
   timerData.workUnits=-1;
   timerData.workUnitLabel="";
   if(parentId!=-1) 
      timerData.level=timers[parentId].level+1;
   else //this is the special case when one adds the root timer
      timerData.level=0;
   timerData.time=0;
   timerData.startTime=-1;
   timerData.count=0;
   timerData.active=false;
   timerData.threads=1;
#ifdef _OPENMP
   timerData.threads=omp_get_num_threads();
#endif
   //timerData.work  UnitCount initialized in stop
   //add timer, to both vectors   
   timers.push_back(timerData);
   //add timer to tree, both in timers
   if(parentId!=-1){
      timers[parentId].childIds.push_back(timerData.id);
   }
   return timerData.id;
}


//this function returns the time in seconds . 
inline double TimerTree::wTime() {
   clock_gettime(CLOCK_ID,&t);
   return t.tv_sec + 1.0e-9 * t.tv_nsec;
}
//this function returns the accuracy of the timer     
inline double TimerTree::wTick() {
   clock_getres(CLOCK_ID,&t);
   return t.tv_sec + 1.0e-9 * t.tv_nsec;
} 
  
