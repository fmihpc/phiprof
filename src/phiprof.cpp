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

#include "mpi.h"
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
#include "phiprof.hpp"
//include craypat api headers if compiled with craypat on Cray XT/XE
#ifdef CRAYPAT
#include "pat_api.h"
#endif

#ifdef _OPENMP
#include "omp.h"
#endif


using namespace std;

namespace phiprof
{
   namespace 
   {
      struct TimerData{
	 string label;          //print label 
	 string workUnitLabel;   //unit for the counter workUnitCount
	 double workUnits;        // how many units of work have we done. If -1 it is not counted, or printed
	 vector<string> groups; // What user-defined groups does this timer belong to, e.g., "MPI", "IO", etc..
	
	 int id; // unique id identifying this timer (index for timers)
	 int parentId;  //key of parent (id)
	 int threads; //threads active when this timer was called
  	 vector<int> childIds; //children of this timer
	
	 int level;  //what hierarchy level
	 int count; //how many times have this been accumulated
	 double time; // total time accumulated
	 double startTime; //Starting time of previous start() call

         bool active;
      };
      //used with MPI reduction operator
      struct doubleRankPair {
	 double val;
	 int rank;
      };

      // Updated in collectStats, only valid on root rank
      struct TimerStatistics {
         vector<int> id; //id of the timer at this index in the statistics vectors
         vector<int> level;
         vector<double> timeSum;
         vector<doubleRankPair> timeMax;
         vector<doubleRankPair> timeMin;
         vector<double> timeTotalFraction;
         vector<double> timeParentFraction;
         vector<bool> hasWorkUnits;
         vector<double> workUnitsSum;
         vector<int> countSum;
     	 vector<int> threadsSum;
      };

      struct GroupStatistics {
         vector<string> name; 
         vector<double> timeSum;
         vector<doubleRankPair> timeMax;
         vector<doubleRankPair> timeMin;
         vector<double> timeTotalFraction;
      };

      //vector with timers, cumulative and for log print
      vector<TimerData> _cumulativeTimers;
      vector<TimerData> _logTimers;
      
      //current position in timer hierarchy 
      int _currentId=-1;

      //is this profiler initialized
      bool _initialized=false;

      //used to store time when we start printing. This is used to correctly asses active timer time
      double _printStartTime;

      //defines print-area widths for print() output
      const int _indentWidth=2; //how many spaces each level is indented
      const int _floatWidth=10; //width of float fields;
      const int _intWidth=6;   //width of int fields;
      const int _unitWidth=4;  //width of workunit label
      const int _levelWidth=5; //width of level label

////-------------------------------------------------------------------------
///  Hash functions
////-------------------------------------------------------------------------      
      
      //djb2 hash function copied from
      //http://www.cse.yorku.ca/~oz/hash.html
      unsigned long hash(const char *str)
      {
	 unsigned long hash = 5381;
	 int c;
	 while ( (c = *str++) )
	       hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	 return hash;
      }

      
      //Hash value identifying all labels, groups and workunitlabels.
      //If any strings differ, hash should differ. Computed recursively in the same way as prints
      int getTimersHash(const vector<TimerData> &timers,int id=0){
         unsigned long hashValue=(int)hash(timers[id].label.c_str());
         hashValue+=hash(timers[id].workUnitLabel.c_str());
         for(unsigned int i=0;i<timers[id].childIds.size();i++){
            hashValue+=getTimersHash(timers,timers[id].childIds[i]);
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
      string getFullLabel(const vector<TimerData> &timers,int id,bool reverse=false){
         //create a label with all hierarchical levels     
         std::vector<string> labels;
         while(id>0){
            labels.push_back(_cumulativeTimers[id].label);
            id=_cumulativeTimers[id].parentId;
         }
         
         string fullLabel;
         if(reverse){
            vector<string>::iterator it;
            for ( it=labels.begin() ; it != labels.end(); ++it ){
               fullLabel+=*it;
               fullLabel+="\\";   
            }
         }
         else{
            vector<string>::reverse_iterator rit;
            for ( rit=labels.rbegin() ; rit != labels.rend(); ++rit ){
               fullLabel+="/";
               fullLabel+=*rit;
            }
         }

         
         return fullLabel;
      }
      
////-------------------------------------------------------------------------
///  Timer handling functions functions
////-------------------------------------------------------------------------            
      
//construct a new timer for the current level.
      int constructTimer(const string &label,int parentId,const vector<string> groups){
	 TimerData timerData;
	 timerData.label=label;
	 timerData.groups=groups;
	 timerData.id=_cumulativeTimers.size(); //will be added last to timers vector
	 timerData.parentId=parentId;
	 
	 timerData.workUnits=-1;
	 timerData.workUnitLabel="";
	 
	 if(parentId!=-1) 
	    timerData.level=_cumulativeTimers[parentId].level+1;
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
	 _cumulativeTimers.push_back(timerData);
         _logTimers.push_back(timerData);
	 //add timer to tree, both in _cumulativeTimers, and _logTimers
	 if(parentId!=-1){
	    _cumulativeTimers[parentId].childIds.push_back(timerData.id);
            _logTimers[parentId].childIds.push_back(timerData.id);
         }
         return timerData.id;
      }
      
      //initialize profiler, called by first start/initializeTimer. This adds the root timer
      bool init(){
	 if(!_initialized){
	    _initialized=true;
	    vector<string> group;
	    group.push_back("Total");
	    //no timer yet        
	    _currentId=-1;
	    //mainId will be 0, parent is -1 (does not exist)
	    int id=constructTimer("total",-1,group);
	    //start root timer, is stopped in print.      
	    start(id);
	 }
	 return true;
      }
      
      //this function returns the time in seconds 
      double getTime() {
	 return MPI_Wtime();
      }
      //this function returns the accuracy of the timer     
      double getTick() {
	 return MPI_Wtick();
      }


      
      


   

      
////-------------------------------------------------------------------------
///  Collect statistics functions
////-------------------------------------------------------------------------            


      double getTimerTime(int id,const vector<TimerData> &timers){
         double time;
         time=timers[id].time;
         //add time uptill print for active timers
         if(timers[id].active)
            time+=_printStartTime-timers[id].startTime;
         return time;
      }
   
     
      
      double getGroupInfo(string group,int id,const vector<TimerData> &timers , MPI_Comm comm){
         double groupTime=0.0;
	 for (vector<string>::const_iterator g = timers[id].groups.begin();
              g != timers[id].groups.end(); ++g ) {
	    if(group==*g){
               groupTime=getTimerTime(id,timers);
               return groupTime; // do not collect for children when this is already in group.Avoid double counting
	    }
	 }
      
         //recursively collect time data   
         for(unsigned int i=0;i<timers[id].childIds.size();i++){
            groupTime+=getGroupInfo(group,timers[id].childIds[i],timers,comm);
	 }
	 return groupTime;
      }
      

      
      void collectGroupStats(GroupStatistics &stats,const vector<TimerData> &timers,MPI_Comm &comm){
	 int rank,nProcesses;
         //per process info. updated in collectStats
         vector<double> time;
         vector<doubleRankPair> timeRank;
         map<string,vector<int> > groups;
         doubleRankPair in;
         int totalIndex=0; //where we store the group for total time (called Total, in timer id=0)
         
	 MPI_Comm_rank(comm,&rank);
	 MPI_Comm_size(comm,&nProcesses);

         
         //construct map from groups to timers in group 
	 for(unsigned int id=0;id<timers.size();id++){
	    for (vector<string>::const_iterator group = timers[id].groups.begin();
                 group != timers[id].groups.end(); ++group ) {
	       groups[*group].push_back(id);
	    }
	 }
         int nGroups=groups.size();
         stats.name.clear(); // we will use push_back to add names to this vector
         
         //collect data for groups
         for(map<string, vector<int> >::iterator group=groups.begin();
	    group!=groups.end();++group){
	    double groupTime=0.0;
            if(group->first=="Total")
               totalIndex=stats.name.size();

	    groupTime=getGroupInfo(group->first,0,timers,comm);
            stats.name.push_back(group->first);
            time.push_back(groupTime);
            in.val=groupTime;
            in.rank=rank;
            timeRank.push_back(in);
         }

         //Compute statistics using reduce operations
         if(rank==0){
            //reserve space for reduce operations output
            stats.timeSum.resize(nGroups);
            stats.timeMax.resize(nGroups);
            stats.timeMin.resize(nGroups);
            stats.timeTotalFraction.resize(nGroups);

            MPI_Reduce(&(time[0]),&(stats.timeSum[0]),nGroups,MPI_DOUBLE,MPI_SUM,0,comm);
            MPI_Reduce(&(timeRank[0]),&(stats.timeMax[0]),nGroups,MPI_DOUBLE_INT,MPI_MAXLOC,0,comm);
            MPI_Reduce(&(timeRank[0]),&(stats.timeMin[0]),nGroups,MPI_DOUBLE_INT,MPI_MINLOC,0,comm);

            for(int i=0;i<nGroups;i++){
               if(stats.timeSum[totalIndex]>0)
                  stats.timeTotalFraction[i]=stats.timeSum[i]/stats.timeSum[totalIndex];
               else
                  stats.timeTotalFraction[i]=0.0;
            }
         }
         else{
            //not masterank, we do not resize and use stats vectors
            MPI_Reduce(&(time[0]),NULL,nGroups,MPI_DOUBLE,MPI_SUM,0,comm);
            MPI_Reduce(&(timeRank[0]),NULL,nGroups,MPI_DOUBLE_INT,MPI_MAXLOC,0,comm);
            MPI_Reduce(&(timeRank[0]),NULL,nGroups,MPI_DOUBLE_INT,MPI_MINLOC,0,comm);            
         }

      }

      
//collect timer stats, call children recursively. In original code this should be called for the first id=0
      void collectTimerStats(TimerStatistics &stats,const vector<TimerData> &timers,MPI_Comm &comm,int id=0,int parentIndex=0){
	 int rank,nProcesses;
         //per process info. updated in collectStats
         static vector<double> time;
         static vector<doubleRankPair> timeRank;
         static vector<double> workUnits;
         static vector<int> count;
         static vector<int> threads;
         static vector<int> parentIndices;
         int currentIndex;
         doubleRankPair in;
            
	 MPI_Comm_rank(comm,&rank);
	 MPI_Comm_size(comm,&nProcesses);

         //first time we call  this function
         if(id==0){
            time.clear();
            timeRank.clear();
            count.clear();
            threads.clear();
	    workUnits.clear();
            parentIndices.clear();
            stats.id.clear();
            stats.level.clear();
         }
         
         //collect statistics

         currentIndex=stats.id.size();
         
         double currentTime=getTimerTime(id,timers);
         
         stats.id.push_back(id);
         stats.level.push_back(timers[id].level);
         time.push_back(currentTime);
         in.val=currentTime;
         in.rank=rank;
         timeRank.push_back(in);
         count.push_back(timers[id].count);
	 threads.push_back(timers[id].threads);
         workUnits.push_back(timers[id].workUnits);
         parentIndices.push_back(parentIndex);
         
         double childTime=0;
         //collect data for children. Also compute total time spent in children
         for(unsigned int i=0;i<timers[id].childIds.size();i++){
            childTime+=getTimerTime(timers[id].childIds[i],timers);
            collectTimerStats(stats,timers,comm,timers[id].childIds[i],currentIndex);
         }
         
         if(timers[id].childIds.size()>0){
            //Added timings for other time. These are assigned id=-1
            stats.id.push_back(-1);
            stats.level.push_back(timers[timers[id].childIds[0]].level); //same level as children
            time.push_back(currentTime-childTime);
            in.val=currentTime-childTime;
            in.rank=rank;
            timeRank.push_back(in);
            count.push_back(timers[id].count);
	    threads.push_back(timers[id].threads);
            workUnits.push_back(-1);
            parentIndices.push_back(currentIndex);
         }

         
         //End of function for id=0, we have now collected all timer data.
         //compute statistics now
         if(id==0){
            int nTimers=time.size();
            if(rank==0){
               stats.timeSum.resize(nTimers);
               stats.timeMax.resize(nTimers);
               stats.timeMin.resize(nTimers);
               stats.workUnitsSum.resize(nTimers);
               stats.hasWorkUnits.resize(nTimers);
               stats.countSum.resize(nTimers);
               stats.threadsSum.resize(nTimers);

               stats.timeTotalFraction.resize(nTimers);
               stats.timeParentFraction.resize(nTimers);
               vector<double> workUnitsMin;
               workUnitsMin.resize(nTimers);
               
               MPI_Reduce(&(time[0]),&(stats.timeSum[0]),nTimers,MPI_DOUBLE,MPI_SUM,0,comm);
               MPI_Reduce(&(timeRank[0]),&(stats.timeMax[0]),nTimers,MPI_DOUBLE_INT,MPI_MAXLOC,0,comm);
               MPI_Reduce(&(timeRank[0]),&(stats.timeMin[0]),nTimers,MPI_DOUBLE_INT,MPI_MINLOC,0,comm);
               
               MPI_Reduce(&(workUnits[0]),&(stats.workUnitsSum[0]),nTimers,MPI_DOUBLE,MPI_SUM,0,comm);
               MPI_Reduce(&(workUnits[0]),&(workUnitsMin[0]),nTimers,MPI_DOUBLE,MPI_MIN,0,comm);
               MPI_Reduce(&(count[0]),&(stats.countSum[0]),nTimers,MPI_INT,MPI_SUM,0,comm);
               MPI_Reduce(&(threads[0]),&(stats.threadsSum[0]),nTimers,MPI_INT,MPI_SUM,0,comm);
               
               for(int i=0;i<nTimers;i++){
                  if(workUnitsMin[i]<0)
                     stats.hasWorkUnits[i]=false;
                  else
                     stats.hasWorkUnits[i]=true;
                  
                  if(stats.timeSum[0]>0)
                     stats.timeTotalFraction[i]=stats.timeSum[i]/stats.timeSum[0];
                  else
                     stats.timeTotalFraction[i]=0.0;
                  
                  if(stats.timeSum[parentIndices[i]]>0)
                     stats.timeParentFraction[i]=stats.timeSum[i]/stats.timeSum[parentIndices[i]];
                  else
                     stats.timeParentFraction[i]=0.0;
               }
            }
            else{
               //not masterank, we do not resize and use stats vectors
               MPI_Reduce(&(time[0]),NULL,nTimers,MPI_DOUBLE,MPI_SUM,0,comm);
               MPI_Reduce(&(timeRank[0]),NULL,nTimers,MPI_DOUBLE_INT,MPI_MAXLOC,0,comm);
               MPI_Reduce(&(timeRank[0]),NULL,nTimers,MPI_DOUBLE_INT,MPI_MINLOC,0,comm);
               
               MPI_Reduce(&(workUnits[0]),NULL,nTimers,MPI_DOUBLE,MPI_SUM,0,comm);
               MPI_Reduce(&(workUnits[0]),NULL,nTimers,MPI_DOUBLE,MPI_MIN,0,comm);
               MPI_Reduce(&(count[0]),NULL,nTimers,MPI_INT,MPI_SUM,0,comm);               
	       MPI_Reduce(&(threads[0]),NULL,nTimers,MPI_INT,MPI_SUM,0,comm);
            }
            //clear temporary data structures
            time.clear();
            timeRank.clear();
            count.clear();
	    threads.clear();
            workUnits.clear();
            parentIndices.clear();
         }
      }
      

//remove print time from timings by pushing forward start_time
      void removePrintTime(double endPrintTime,vector<TimerData> &timers,int id=0){
         if(timers[id].active){
            //push start time so that ew push it forward
            timers[id].startTime+=endPrintTime-_printStartTime;
            for(unsigned int i=0;i<timers[id].childIds.size();i++){
               removePrintTime(endPrintTime,timers,timers[id].childIds[i]);
            }
         }

      }

      //reset logtimes in timers to zero.

      void resetTime(double endPrintTime,vector<TimerData> &timers,int id=0){
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
      



////-------------------------------------------------------------------------
///  PRINT functions

////-------------------------------------------------------------------------      

      // Creating map of group names to group one-letter ID
      // We assume same timers exists in all timer vectors, so can use just one here
      void getGroupIds(map<string, string> &groupIds,size_t &groupWidth,const vector<TimerData> &timers){
         groupIds.clear();
         groupWidth=6;
         //add groups to map
         for(unsigned int id=0;id<timers.size();id++) {
            size_t width = timers[id].groups.size();
            groupWidth = max(width, groupWidth);               
            for (vector<string>::const_iterator group = timers[id].groups.begin();
                 group != timers[id].groups.end(); ++group ) {
               groupIds[*group] = *group;
            }
         }
         //assing letters
         int character = 65; // ASCII A, TODO skip after Z to a, see ascii table
         for(map<string, string>::const_iterator group = groupIds.begin();
             group != groupIds.end(); ++group) {
            groupIds[group->first] = character++;
         }
      
      }


      //print all timers in stats
      bool printTreeTimerStatistics(const TimerStatistics &stats,const vector<TimerData> &timers,double minFraction,size_t labelWidth,size_t groupWidth,int totalWidth,
                                    const map<string,string> &groupIds,int nProcesses,fstream &output){

         for(int i=0;i<totalWidth/2-5;i++) output <<"-";
         output << " Profile ";
         for(int i=0;i<totalWidth/2-5;i++) output <<"-";
         output<<endl;

         for(unsigned int i=1;i<stats.id.size();i++){
            int id=stats.id[i];
            if(stats.timeTotalFraction[i]>=minFraction){
               //print timer if enough time is spent in it
               bool hasNoGroups = true;
               int indent=(stats.level[i]-1)*_indentWidth;
               output << setw(_levelWidth+1) << stats.level[i];
               
               if(id!=-1){
		 //other label has no groups
                  for (vector<string>::const_iterator group = timers[id].groups.begin();
                       group != timers[id].groups.end(); ++group) {
		    string groupId=groupIds.count(*group) ? groupIds.find(*group)->second : std::string();
		    output << setw(1) << groupId;
		    hasNoGroups = false;
                  }
               }
               
               if(hasNoGroups) output << setw(groupWidth+1) << "";
               else output << setw(groupWidth-timers[id].groups.size()+1) << "";
	       
               output << setw(indent) << "";
               if(id!=-1){
                  output << setw(labelWidth+1-indent) << setiosflags(ios::left) << timers[id].label;
               }
               else{
                  output << setw(labelWidth+1-indent) << setiosflags(ios::left) << "Other";
               }

               output << setw(_floatWidth+1) << stats.threadsSum[i]/nProcesses;        	       
               output << setw(_floatWidth) << stats.timeSum[i]/nProcesses;
               output << setw(_floatWidth) << 100.0*stats.timeParentFraction[i];
               output << setw(_floatWidth) << stats.timeMax[i].val;
               output << setw(_intWidth)   << stats.timeMax[i].rank;
               output << setw(_floatWidth) << stats.timeMin[i].val;
               output << setw(_intWidth)   << stats.timeMin[i].rank;
               output << setw(_floatWidth) << stats.countSum[i]/nProcesses;

               if(stats.hasWorkUnits[i]){
                     //print if units defined for all processes
                     //note how the total rate is computed. This is to avoid one process with little data to     
                     //skew results one way or the other                        
                  if(stats.timeSum[i]>0){
                     output << setw(_floatWidth) << nProcesses*(stats.workUnitsSum[i]/stats.timeSum[i]);
                     output << setw(_floatWidth) << stats.workUnitsSum[i]/stats.timeSum[i];
                  }
                  else if (stats.workUnitsSum[i]>0){
                     //time is zero
                     output << setw(_floatWidth) << "inf";
                     output << setw(_floatWidth) << "inf";
                  }
                  else {
                     //zero time zero units
                     output << setw(_floatWidth) << 0;
                     output << setw(_floatWidth) << 0;
                  }
                  output << timers[id].workUnitLabel<<"/s";                     
               }
               output<<endl;
            }
            
	 }
	 return true;
      }
      
      
      bool printTreeFooter(int totalWidth,fstream &output){
         for(int i=0;i<totalWidth;i++) output <<"-";
         output<<endl;
	 return true;
      }
      
      bool printTreeHeader(double minFraction,size_t labelWidth,size_t groupWidth,int totalWidth,int nProcs,fstream &output){
         for(int i=0;i<totalWidth;i++) output <<"-";
         output<<endl;
         output << "Phiprof results with time fraction of total time larger than " << minFraction;
         output<<endl;
         output << "Processes in set of timers " << nProcs;
#ifdef _OPENMP
	 output << " with (up to) " << omp_get_max_threads() << " threads ";
#endif 
         output<<endl;
         for(int i=0;i<totalWidth;i++) output <<"-";
         output<<endl;
         output<<setw(_levelWidth+1+groupWidth+1+labelWidth+1)<< setiosflags(ios::left) << "";
         output<<setw(_floatWidth)<< "Threads";
         output<<setw(4*_floatWidth+2*_intWidth) <<"Time(s)";
         output<<setw(_floatWidth)<<"Calls";
         output<<setw(2*_floatWidth)<<"Workunit-rate";
         output<<endl;
         output<<setw(_levelWidth+1)<< "Level";	    
         output<<setw(groupWidth+1)<< "Groups";
//         output << setw(1) << "|";
         output<<setw(labelWidth+1)<< "Label";
//         output << setw(1) << "|";
	    //  time
         output<<setw(_floatWidth) <<"Average";
         output<<setw(_floatWidth) <<"Average";
         output<<setw(_floatWidth) <<"parent %";
         output<<setw(_floatWidth) <<"Maximum";
         output<<setw(_intWidth) << "Rank";
         output<<setw(_floatWidth)<< "Minimum";
         output<<setw(_intWidth) << "Rank";
         //call count
         output<<setw(_floatWidth) << "Average";
         // workunit rate    
         output<<setw(_floatWidth) << "Average";
         output<<setw(_floatWidth) << "Per process";
         output<<endl;
      
	 return true;
      }

            //print groups
      bool printTreeGroupStatistics(const GroupStatistics &groupStats,
                                double minFraction,
                                size_t labelWidth,
                                size_t groupWidth,
                                int totalWidth,
                                const map<string, string> &groupIds,
                                int nProcesses,
                                fstream &output){

         for(int i=0;i<totalWidth/2 -4;i++) output <<"-";
         output <<" Groups ";
         for(int i=0;i<totalWidth/2 -3;i++) output <<"-";
         output<<endl;

         for(unsigned int i=0;i<groupStats.name.size();i++){            
            if(minFraction<=groupStats.timeTotalFraction[i]){
	      string groupId= groupIds.count(groupStats.name[i]) ? groupIds.find(groupStats.name[i])->second : std::string();
	      output << setw(_levelWidth+1) << " ";
	      output << setw(groupWidth+1) << groupId;
	      output << setw(labelWidth+1) << groupStats.name[i];
	      output << setw(_floatWidth) << " ";
	      output << setw(_floatWidth) << groupStats.timeSum[i]/nProcesses;
	      output << setw(_floatWidth) << 100.0*groupStats.timeTotalFraction[i];
	      output << setw(_floatWidth) << groupStats.timeMax[i].val;
	      output << setw(_intWidth)   << groupStats.timeMax[i].rank;
	      output << setw(_floatWidth) << groupStats.timeMin[i].val;
	      output << setw(_intWidth)   << groupStats.timeMin[i].rank;
	      output << endl;
	    }
         }

         return true;
      }
         
      
   
      //print out global timers
      //If any labels differ, then this print will deadlock. Only call it with a communicator that is guaranteed to be consistent on all processes.
      bool printTree(const TimerStatistics &stats,const GroupStatistics &groupStats,const vector<TimerData> &timers,double minFraction,string fileName, MPI_Comm comm){
         int rank,nProcesses;
         map<string, string> groupIds;
         size_t labelWidth=0;    //width of column with timer labels
         size_t groupWidth=0;    //width of column with group letters
         int totalWidth=6;       //total width of the table
      
         MPI_Comm_rank(comm,&rank);
         MPI_Comm_size(comm,&nProcesses);
         
         //compute labelWidth
         if(rank==0){
            fstream output;
            output.open(fileName.c_str(), fstream::out);
            if (output.good() == false)
               return false;

            
            for (unsigned int i=0;i<timers.size();i++){
               size_t width=timers[i].label.length()+(timers[i].level-1)*_indentWidth;
               labelWidth=max(labelWidth,width);
            }

            getGroupIds(groupIds,groupWidth,timers);
	    
            //make sure we use default floats, and not fixed or other format
            output <<resetiosflags( ios::floatfield );
            //set       float p  rec  ision
            output <<setprecision(_floatWidth-6); //6 is needed for ".", "e+xx" and a space
            
            unsigned int labelDelimeterWidth=2;
            totalWidth=_levelWidth+1+groupWidth+1+labelWidth+1+labelDelimeterWidth+_floatWidth*7+_intWidth*2+_unitWidth;
         
            //print header 
            printTreeHeader(minFraction,labelWidth,groupWidth,totalWidth,nProcesses,output);
            //print out all labels recursively
            printTreeTimerStatistics(stats,timers,minFraction,labelWidth,groupWidth,totalWidth,groupIds,nProcesses,output);
            //print groups
            printTreeGroupStatistics(groupStats,minFraction,labelWidth,groupWidth,totalWidth,groupIds,nProcesses,output);
            //print footer  
            printTreeFooter(totalWidth,output);
            // start root timer again in case we continue and call print several times
            output.close();
            
         }
         return true;
      }
      
      
      //print out global timers and groups in log format

      bool printLog(const TimerStatistics &stats,const GroupStatistics &groupStats,const vector<TimerData> &timers,string fileName, string separator,double simulationTime,int maxLevel,MPI_Comm comm){
         int rank,nProcesses;
         MPI_Comm_rank(comm,&rank);
         MPI_Comm_size(comm,&nProcesses);
         
         //compute labelWidth
         if(rank==0){
            fstream output;
            bool fileExists;
            int lineWidth=80;
            //test if file exists
            output.open(fileName.c_str(), fstream::in);
            fileExists=output.good();
            output.close();

            //FIXME, we should instead check if this profiler haswritten out anything and overwrite old files
            
            if(fileExists)
               output.open(fileName.c_str(), fstream::app|fstream::out);
            else
               output.open(fileName.c_str(), fstream::out);

            if(!fileExists){
               int column=1;
               size_t groupWidth;
               map<string, string> groupIds;               
               getGroupIds(groupIds,groupWidth,timers);
               
               //write header
               output << "#";
               for(int i=0;i<lineWidth;i++) output << "-";
               output << endl;
               output << "# Profile with "<< nProcesses <<" number of processes"<<endl;
               output << "# In first column of header, the first column for that timer/group is printed"<<endl;
               output << "# Each timer has data for count, time and workunits, 9 columns in total."<<endl;
               output << "#       +0  Count average"<<endl;
               output << "#       +1  Time  average"<<endl;
               output << "#       +2        percent of parent time"<<endl;
               output << "#       +3        max value"<<endl;
               output << "#       +4        max rank"<<endl;
               output << "#       +5        min value"<<endl;
               output << "#       +6        min rank"<<endl;
               output << "#       +7  Workunits (/s) average total"<<endl;
               output << "#       +8                 average per process"<<endl;
               output << "# Each group has data for time, 6 columns in total"<<endl;
               output << "#       +0  Time  average"<<endl;
               output << "#       +1        percent of total time"<<endl;
               output << "#       +2        max value"<<endl;
               output << "#       +3        max rank"<<endl;
               output << "#       +4        min value"<<endl;
               output << "#       +5        min rank"<<endl;
               for(int i=0;i<lineWidth/2 -4;i++) output <<"-";
               output <<" User data ";
               for(int i=0;i<lineWidth/2 -3;i++) output <<"-";
               output<<endl;
               output <<"#";
               output << setw(6) << column;
               output << setw(groupWidth+1) << "";
               output << "Simulation time";
               output << endl;
               column+=1;    

               
               output << "#";               
               for(int i=0;i<lineWidth/2 -4;i++) output <<"-";
               output <<" Groups ";
               for(int i=0;i<lineWidth/2 -3;i++) output <<"-";
               output<<endl;
               
               for(unsigned int i=0;i<groupStats.name.size();i++){            
		 string groupId=groupIds.count(groupStats.name[i]) ? groupIds.find(groupStats.name[i])->second : std::string();
                  output <<"#";
                  output << setw(6) << column;
                  output << setw(2) << groupId<<" ";
                  output << groupStats.name[i];
                  output << endl;
                  column+=6;
               }

               output << "#";               
               for(int i=0;i<lineWidth/2 -4;i++) output <<"-";
               output <<" Timers ";
               for(int i=0;i<lineWidth/2 -3;i++) output <<"-";
               output<<endl;

               
               for(unsigned int i=1;i<stats.id.size();i++){
                  int id=stats.id[i];
                  bool hasNoGroups = true;
                  int indent=(stats.level[i]-1)*_indentWidth;
                  if(stats.level[i]<=maxLevel || maxLevel<=0){
                     output <<"#";
                     output << setw(6) << column <<" ";
                     if(id!=-1){
                        //other label has no groups
                        for (vector<string>::const_iterator group = timers[id].groups.begin();
                             group != timers[id].groups.end(); ++group) {
			  string groupId=groupIds.count(*group) ? groupIds.find(*group)->second : std::string();
			  output << setw(2) << groupId;
			  hasNoGroups = false;
                        }
                     }
                     
                     if(hasNoGroups) output << setw(groupWidth+1) << "";
                     else output << setw(groupWidth-timers[id].groups.size()+1) << "";
                     
                     output << setw(indent) << "";
                     if(id!=-1){
                        output   << timers[id].label;
                     }
                     else{
                        output  << "Other";
                     }
                     output<<endl;
                     column+=9;
                  }
               }
               
               output << "#";
               for(int i=0;i<lineWidth;i++) output <<"-";
               output << endl;

               //end header write
            }

            output << simulationTime<<separator;
            
            for(unsigned int i=0;i<groupStats.name.size();i++){            
               output << groupStats.timeSum[i]/nProcesses<<separator;
               output << 100.0*groupStats.timeTotalFraction[i]<<separator;
               output << groupStats.timeMax[i].val<<separator;
               output << groupStats.timeMax[i].rank<<separator;
               output << groupStats.timeMin[i].val<<separator;
               output << groupStats.timeMin[i].rank<<separator;
             }

            for(unsigned int i=1;i<stats.id.size();i++){
               if(stats.level[i]<=maxLevel || maxLevel<=0){
                  output <<  stats.countSum[i]/nProcesses<<separator;
                  output <<  stats.timeSum[i]/nProcesses<<separator;
                  output <<  100.0*stats.timeParentFraction[i]<<separator;
                  output <<  stats.timeMax[i].val<<separator;
                  output <<  stats.timeMax[i].rank<<separator;
                  output <<  stats.timeMin[i].val<<separator;
                  output <<  stats.timeMin[i].rank<<separator;
                  
                  
                  if(stats.hasWorkUnits[i]){
                     //print if units defined for all processes
                     //note how the total rate is computed. This is to avoid one process with little data to     
                     //skew results one way or the other                        
                     if(stats.timeSum[i]>0){
                        output <<nProcesses*(stats.workUnitsSum[i]/stats.timeSum[i])<<separator;
                        output << stats.workUnitsSum[i]/stats.timeSum[i]<<separator;
                     }
                     
                     else if (stats.workUnitsSum[i]>0){
                        //time is zero
                        output << "inf"<<separator;
                        output << "inf"<<separator;
                     }
                     else{
                        //zero time zero units, or no time un
                        output << 0<<separator;
                        output << 0<<separator;
                     }
                  }
                  else{
                     //no workunits;
                     output << "nan" <<separator<< "nan" <<separator;
                  }
               } //if level<=maxlevel
            }
            output<<endl;
            output.close();

         }
            
         return true;
      }

      
      bool getPrintCommunicator(int &printIndex,int &timersHash,MPI_Comm &printComm, MPI_Comm comm){
         int mySuccess=1;
         int success;
         int myRank;
         MPI_Comm_rank(comm,&myRank);

//hash should be the same for _cumulativeTimers, _logTimers

         timersHash=getTimersHash(_cumulativeTimers);

         
         int result = MPI_Comm_split(comm, timersHash, 0, &printComm);
         
         if (result != MPI_SUCCESS) {
            int error_string_len = MPI_MAX_ERROR_STRING;
            char error_string[MPI_MAX_ERROR_STRING + 1];
            MPI_Error_string(result, error_string, &error_string_len);
            error_string[MPI_MAX_ERROR_STRING] = 0;
            if(myRank==0)
               cerr << "PHIPROF-ERROR: Error splitting communicator for printing: " << error_string << endl;
            mySuccess=0;
         }
         
         //Now compute the id for the print comms. First communicatr is given index 0, next 1 and so on.
         int printCommRank;
         int printCommSize;
         //get rank in printComm
         MPI_Comm_rank(printComm,&printCommRank);
         MPI_Comm_size(printComm,&printCommSize);
         //communicator with printComm masters(rank=0), this will be used to number the printComm's
         MPI_Comm printCommMasters;
         MPI_Comm_split(comm,printCommRank==0,-printCommSize,&printCommMasters);
         MPI_Comm_rank(printCommMasters,&printIndex);
         MPI_Comm_free(&printCommMasters);
         MPI_Bcast(&printIndex,1,MPI_INT,0,printComm);

         //check that the hashes at least have the same number of timers, just to be sure and to avoid crashes...

         int nTimers=_cumulativeTimers.size();
         int maxTimers;
         int minTimers;
         
         MPI_Reduce(&nTimers,&maxTimers,1,MPI_INT,MPI_MAX,0,printComm);
         MPI_Reduce(&nTimers,&minTimers,1,MPI_INT,MPI_MIN,0,printComm);

         if(printCommRank==0) { 
            if(minTimers !=  maxTimers) {
               cerr << "PHIPROF-ERROR: Missmatch in number of timers, hash conflict?  maxTimers = " << maxTimers << " minTimers = " << minTimers << endl;
               mySuccess=0;
            }
         }
         
         //if any process failed, the whole routine failed
         MPI_Allreduce(&mySuccess,&success,1,MPI_INT,MPI_MIN,comm);
         
         return success;

      }
   }
   
// end unnamed namespace
//----------------------------------------------------------------------------------------------
// public functions begin    


      
   //initialize a timer, with a particular label belonging to some groups
   //returns id of new timer. If timer exists, then that id is returned.
   int initializeTimer(const string &label,const vector<string> &groups){
      //check if the global profiler is initialized
      if(!_initialized)
	 init();
      int id=getId(label); //check if it exists
      if(id>=0)
	 //do nothing if it exists      
	 return id; 
      else
	 //create new timer if it did not exist
	 return constructTimer(label,_currentId,groups);
   }


   //initialize a timer, with a particular label belonging to a group
   //returns id of new timer. If timer exists, then that id is returned.
   int initializeTimer(const string &label,const string &group){
      //check if the global profiler is initialized
      vector<string> groups;
      groups.push_back(group);
      return initializeTimer(label,groups);
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
      return initializeTimer(label,groups);
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
      return initializeTimer(label,groups);
   }


   //initialize a timer, with a particular label belonging to no group
   //returns id of new timer. If timer exists, then that id is returned.
   int initializeTimer(const string &label){
      vector<string> groups; //empty vector
      return initializeTimer(label,groups);
   }

   
   //start timer, with id
   bool start(int id){
      bool success=true;
#pragma omp master
      {
         if(_currentId!=_cumulativeTimers[id].parentId){
            cerr << "PHIPROF-ERROR: Starting timer that is not a child of the current profiling region" <<endl;
            success= false;
         }
         else{
            _currentId=id;      
            //start timer
            _cumulativeTimers[_currentId].startTime=getTime();
            _cumulativeTimers[_currentId].active=true;

            //start log timer
            _logTimers[_currentId].startTime=_cumulativeTimers[_currentId].startTime;
            _logTimers[_currentId].active=true;

         
#ifdef CRAYPAT
            PAT_region_begin(_currentId+1,getFullLabel(_cumulativeTimers,_currentId,true).c_str());
#endif
         }
      }
      return success;        
   }

   //start timer, with label
   bool start(const string &label){
#pragma omp master
      {
         //If the timer exists, then initializeTimer just returns its id, otherwise it is constructed.
         //Make the timer the current one
         _currentId=initializeTimer(label);
         //start timer
         _cumulativeTimers[_currentId].startTime=getTime();
         _cumulativeTimers[_currentId].active=true;

         //start log timer   
         _logTimers[_currentId].startTime=_cumulativeTimers[_currentId].startTime;
         _logTimers[_currentId].active=true;
      
#ifdef CRAYPAT
         PAT_region_begin(_currentId+1,getFullLabel(_cumulativeTimers,_currentId,true).c_str());
#endif
      }
      return true;        
   }
   
   //stop with workunits
   bool stop (const string &label,
              const double workUnits,
              const string &workUnitLabel){
      bool success=true;
#pragma omp master
      {
         if(label != _cumulativeTimers[_currentId].label ){
            cerr << "PHIPROF-ERROR: label missmatch in profile::stop  when stopping "<< label <<
               ". The started timer is "<< _cumulativeTimers[_currentId].label<< " at level " << _cumulativeTimers[_currentId].level << endl;
            success=false;
         }
         if(success)
            success=stop(_currentId,workUnits,workUnitLabel);
      }
      return success;
   }

   //stop a timer defined by id
   bool stop (int id,
              double workUnits,
              const string &workUnitLabel){
      bool success=true;
#pragma omp master
      {
         double stopTime=getTime();
         if(id != _currentId ){
            cerr << "PHIPROF-ERROR: id missmatch in profile::stop Stopping "<< id <<" at level " << _cumulativeTimers[_currentId].level << endl;
            success=false;
         }

         else {
         
#ifdef CRAYPAT
            PAT_region_end(_currentId+1);
#endif  

         
            //handle workUnits for _cumulativeTimers               
            if(_cumulativeTimers[_currentId].count!=0){
               //if this, or a previous, stop did not include work units then do not add them
               //work units have to be defined for all stops with a certain (full)label
               if(workUnits<0 || _cumulativeTimers[_currentId].workUnits<0){
                  _cumulativeTimers[_currentId].workUnits=-1;
                  _logTimers[_currentId].workUnits=-1;
               }
               else{
                  _cumulativeTimers[_currentId].workUnits+=workUnits;
               }
            }
            else{
               //firsttime, initialize workUnit stuff here
               if(workUnits>=0.0 ){
                  //we have workUnits for this counter
                  _cumulativeTimers[_currentId].workUnits=workUnits;
                  _cumulativeTimers[_currentId].workUnitLabel=workUnitLabel;
               }
               else{
                  // no workUnits for this counter
                  _cumulativeTimers[_currentId].workUnits=-1.0;
               }
            }
            
            //handle workUnits for _logTimers
            if(_logTimers[_currentId].count!=0){
               //if this, or a previous, stop did not include work units then do not add t hem
               //work units have to be defined for all stops with a certain (full)label
               if(workUnits<0 || _logTimers[_currentId].workUnits<0){
                  _logTimers[_currentId].workUnits=-1;
               }
               else{
                  _logTimers[_currentId].workUnits+=workUnits;
               }
            }
            else{
               //firsttime, initialize workUnit stuff here
               if(workUnits>=0.0 ){
                  //we  have workUnits for this counter
                  _logTimers[_currentId].workUnits=workUnits;
                  _logTimers[_currentId].workUnitLabel=workUnitLabel;
               }
               else{
                  //  no workUnits for this counter
                  _logTimers[_currentId].workUnits=-1.0;
               }
            }
            
         //stop _cumulativeTimers & _logTimers timer              
            _cumulativeTimers[_currentId].time+=(stopTime-_cumulativeTimers[_currentId].startTime);
            _cumulativeTimers[_currentId].count++;
            _cumulativeTimers[_currentId].active=false;
            _logTimers[_currentId].time+=(stopTime-_logTimers[_currentId].startTime);
            _logTimers[_currentId].count++;
            _logTimers[_currentId].active=false;
            
      
            //go down in hierarchy    
            _currentId=_cumulativeTimers[_currentId].parentId;
         }
      }
      return success;
   }

      
   
   
   //get id number of a timer, return -1 if it does not exist
   int getId(const string &label){
      //find child with this id
      int childId=-1;
#pragma omp master
      {
         for(unsigned int i=0;i<_cumulativeTimers[_currentId].childIds.size();i++)
            if (_cumulativeTimers[_cumulativeTimers[_currentId].childIds[i]].label==label){
               childId=_cumulativeTimers[_currentId].childIds[i];
               break;
            }
      }
      return childId;
   }
   
//print a tree profile (will overwrite)
   
   bool print(MPI_Comm comm,string fileNamePrefix,double minFraction){
#pragma omp master
      {
         int timersHash,printIndex;
         int rank,nProcesses;
         MPI_Comm printComm;
         TimerStatistics stats;
         GroupStatistics groupStats;
      
         //_printStartTime defined in namespace, used to correct timings for open timers
         _printStartTime=getTime();
         MPI_Barrier(comm);
         
         MPI_Comm_rank(comm,&rank);
         MPI_Comm_size(comm,&nProcesses);
         
         //get hash value of timers and the print communicator
         if(getPrintCommunicator(printIndex,timersHash,printComm,comm)) {
            //generate file name
            stringstream fname;
            fname << fileNamePrefix << "_" << printIndex << ".txt";
            collectTimerStats(stats,_cumulativeTimers,printComm);
            collectGroupStats(groupStats,_cumulativeTimers,printComm);
            printTree(stats,groupStats,_cumulativeTimers,minFraction,fname.str(),printComm);
         }
         
         MPI_Comm_free(&printComm);
         MPI_Barrier(comm);
         
         double endPrintTime=getTime();
         removePrintTime(endPrintTime,_cumulativeTimers);
         removePrintTime(endPrintTime,_logTimers);
      }
      return true;
   }
   
//print in a log format   (will append)
   bool printLogProfile(MPI_Comm comm,double simulationTime,string fileNamePrefix,string separator,int maxLevel){
#pragma omp master
      {
         int timersHash,printIndex;
         int rank,nProcesses;
         MPI_Comm printComm;
         TimerStatistics stats;
         GroupStatistics groupStats;
         
         //_printStartTime defined in namespace, used to correct timings for open timers
         _printStartTime=getTime();
         MPI_Barrier(comm);
         
         MPI_Comm_rank(comm,&rank);
         MPI_Comm_size(comm,&nProcesses);
         
         //get hash value of timers and the print communicator
         if(getPrintCommunicator(printIndex,timersHash,printComm,comm)) {
         
            //generate file name, here we use timers hash to avoid overwriting old data!
            stringstream fname;
            fname << fileNamePrefix << "_" << timersHash << ".txt";
            
            //collect statistics
            collectTimerStats(stats,_logTimers,printComm);
            collectGroupStats(groupStats,_logTimers,printComm);
            //print log
            printLog(stats,groupStats,_logTimers,fname.str(),separator,simulationTime,maxLevel,printComm);
         }
         MPI_Comm_free(&printComm);
         MPI_Barrier(comm);
         
         double endPrintTime=getTime();
         removePrintTime(endPrintTime,_cumulativeTimers);
         resetTime(endPrintTime,_logTimers);
      }
      return true;
   }
      
}


   
   
