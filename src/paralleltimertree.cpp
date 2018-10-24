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

#include <iostream>    
#include <sstream>    
#include <iomanip>    
#include <vector>
#include <string>
#include <cstring>
#include <set>
#include <limits>
#include <algorithm>
#include <time.h>
#include "paralleltimertree.hpp"
#include "prettyprinttable.hpp"
#include "common.hpp"

#ifdef _OPENMP
#include "omp.h"
#endif
#include "mpi.h"

   


////-------------------------------------------------------------------------
///  Collect statistics functions
////-------------------------------------------------------------------------            
     
      
      
void ParallelTimerTree::collectGroupStats(){
   int rank, nProcesses;
   //per process info. updated in collectStats
   std::vector<double> time;
   std::vector<doubleRankPair> timeRank;
   std::map<std::string,std::vector<int> > groups;
   doubleRankPair in;
   int totalIndex=0; //where we store the group for total time (called Total, in timer id=0)
   
   //construct std::map from groups to timers in group 
   for(unsigned int id=0;id < size();id++){
      for(auto &group: (*this)[id].getGroups() ){
         groups[group].push_back(id);
      }
   }

   int nGroups=groups.size();
   groupStats.name.clear(); // we will use push_back to add names to this std::vector
   
   //collect data for groups
   for(std::map<std::string, std::vector<int> >::iterator group=groups.begin();
       group!=groups.end();++group){
      double groupTime=0.0;
      if(group->first=="Total")
         totalIndex=groupStats.name.size();
      
      groupTime=getGroupTime(group->first, 0);
      groupStats.name.push_back(group->first);
      time.push_back(groupTime);
      in.val=groupTime;
      in.rank=rank;
      timeRank.push_back(in);
   }

   //Compute statistics using reduce operations
   if(rankInPrint==0){
      //reserve space for reduce operations output
      groupStats.timeSum.resize(nGroups);
      groupStats.timeMax.resize(nGroups);
      groupStats.timeMin.resize(nGroups);
      groupStats.timeTotalFraction.resize(nGroups);

      MPI_Reduce(&(time[0]),&(groupStats.timeSum[0]),nGroups,MPI_DOUBLE,MPI_SUM,0,printComm);
      MPI_Reduce(&(timeRank[0]),&(groupStats.timeMax[0]),nGroups,MPI_DOUBLE_INT,MPI_MAXLOC,0,printComm);
      MPI_Reduce(&(timeRank[0]),&(groupStats.timeMin[0]),nGroups,MPI_DOUBLE_INT,MPI_MINLOC,0,printComm);

      for(int i=0;i<nGroups;i++){
         if(groupStats.timeSum[totalIndex]>0)
            groupStats.timeTotalFraction[i]=groupStats.timeSum[i]/groupStats.timeSum[totalIndex];
         else
            groupStats.timeTotalFraction[i]=0.0;
      }
   }
   else{
      //not masterank, we do not resize and use groupStats std::vectors
      MPI_Reduce(&(time[0]),NULL,nGroups,MPI_DOUBLE,MPI_SUM,0,printComm);
      MPI_Reduce(&(timeRank[0]),NULL,nGroups,MPI_DOUBLE_INT,MPI_MAXLOC,0,printComm);
      MPI_Reduce(&(timeRank[0]),NULL,nGroups,MPI_DOUBLE_INT,MPI_MINLOC,0,printComm);            
   }

}

      
//collect timer stats, call children recursively. In original code this should be called for the first id=0
// reportRank is the rank to be used in the report, not the rank in the printComm communicator
void ParallelTimerTree::collectTimerStats(int reportRank, int id, int parentIndex){
   //per process info. updated in collectStats
   static std::vector<double> time;
   static std::vector<doubleRankPair> timeRank;
   static std::vector<double> workUnits;
   static std::vector<int64_t> count;
   static std::vector<int> threads;
   static std::vector<double> threadImbalance;
   static std::vector<doubleRankPair> threadImbalanceRank;
   static std::vector<int> parentIndices;
   int currentIndex;
   doubleRankPair in;


   //first time we call  this function
   if(id==0){
      time.clear();
      timeRank.clear();
      count.clear();
      threads.clear();
      threadImbalance.clear();
      threadImbalanceRank.clear();
      workUnits.clear();
      parentIndices.clear();
      stats.id.clear();
      stats.level.clear();
   }
         
   //collect statistics

   currentIndex=stats.id.size();         
   double currentTime=getTime(id);
   stats.id.push_back(id);
   stats.level.push_back((*this)[id].getLevel());   
   time.push_back(currentTime);
   in.val=currentTime;
   in.rank=reportRank;
   timeRank.push_back(in);
   count.push_back((*this)[id].getAverageCount());
   threads.push_back((*this)[id].getThreads());
   in.val = (*this)[id].getTimeImbalance();
   in.rank = reportRank;
   threadImbalance.push_back(in.val);
   threadImbalanceRank.push_back(in);
   workUnits.push_back((*this)[id].getAverageWorkUnits());
   parentIndices.push_back(parentIndex);
         
   double childTime=0;
   //collect data for children. Also compute total time spent in children
   for(auto &childId: (*this)[id].getChildIds()) {
      childTime+=(*this)[childId].getAverageTime();
      collectTimerStats(reportRank, childId, currentIndex);
   }
   
   if((*this)[id].getChildIds().size()>0){
      //Added timings for other time. These are assigned id=-1
      stats.id.push_back(-1);
      stats.level.push_back((*this)[(*this)[id].getChildIds()[0]].getLevel()); //same level as children
      time.push_back(currentTime-childTime);
      in.val=currentTime-childTime;
      in.rank=reportRank;
      timeRank.push_back(in);
      count.push_back((*this)[id].getAverageCount());
      threads.push_back((*this)[id].getThreads());
      //TODO - cannot yet compute thread imbalance for other timer 
      in.val = -1.0;
      in.rank = reportRank;
      threadImbalance.push_back(in.val);
      threadImbalanceRank.push_back(in);
      workUnits.push_back(-1.0);
      parentIndices.push_back(currentIndex);
   }
         
   //End of function for id=0, we have now collected all timer data.
   //compute statistics now
   if(id==0){
      int nTimers=time.size(); //note, this also includes the "other"
                               //timers
      if(rankInPrint == 0){
         std::vector<double> workUnitsMin;
         stats.timeSum.resize(nTimers);
         stats.timeMax.resize(nTimers);
         stats.timeMin.resize(nTimers);
         stats.workUnitsSum.resize(nTimers);
         stats.hasWorkUnits.resize(nTimers);
         workUnitsMin.resize(nTimers);
         stats.countSum.resize(nTimers);
         stats.threadsSum.resize(nTimers);
         stats.threadImbalanceSum.resize(nTimers);
         stats.threadImbalanceMax.resize(nTimers);
         stats.threadImbalanceMin.resize(nTimers);


         stats.timeTotalFraction.resize(nTimers);
         stats.timeParentFraction.resize(nTimers);



         MPI_Reduce(&(time[0]),&(stats.timeSum[0]),nTimers,MPI_DOUBLE,MPI_SUM,0,printComm);
         MPI_Reduce(&(timeRank[0]),&(stats.timeMax[0]),nTimers,MPI_DOUBLE_INT,MPI_MAXLOC,0,printComm);
         MPI_Reduce(&(timeRank[0]),&(stats.timeMin[0]),nTimers,MPI_DOUBLE_INT,MPI_MINLOC,0,printComm);
         
         MPI_Reduce(&(workUnits[0]),&(stats.workUnitsSum[0]),nTimers,MPI_DOUBLE,MPI_SUM,0,printComm);
         MPI_Reduce(&(workUnits[0]),&(workUnitsMin[0]),nTimers,MPI_DOUBLE,MPI_MIN,0,printComm);
         MPI_Reduce(&(count[0]),&(stats.countSum[0]),nTimers,MPI_INT64_T,MPI_SUM,0,printComm);
         MPI_Reduce(&(threads[0]),&(stats.threadsSum[0]),nTimers,MPI_INT,MPI_SUM,0,printComm);

         MPI_Reduce(&(threadImbalance[0]),&(stats.threadImbalanceSum[0]), nTimers, MPI_DOUBLE, MPI_SUM, 0, printComm);
         MPI_Reduce(&(threadImbalanceRank[0]),&(stats.threadImbalanceMax[0]), nTimers, MPI_DOUBLE_INT, MPI_MAXLOC, 0, printComm);
         MPI_Reduce(&(threadImbalanceRank[0]),&(stats.threadImbalanceMin[0]), nTimers, MPI_DOUBLE_INT, MPI_MINLOC, 0, printComm);
               
         for(int i=0;i<nTimers;i++){
            if(stats.workUnitsSum[i] <= 0)
               stats.hasWorkUnits[i] = false;
            else
               stats.hasWorkUnits[i] = true;
                  
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
         //not masterank, we do not resize and use stats std::vectors
         MPI_Reduce(&(time[0]),NULL,nTimers,MPI_DOUBLE,MPI_SUM,0,printComm);
         MPI_Reduce(&(timeRank[0]),NULL,nTimers,MPI_DOUBLE_INT,MPI_MAXLOC,0,printComm);
         MPI_Reduce(&(timeRank[0]),NULL,nTimers,MPI_DOUBLE_INT,MPI_MINLOC,0,printComm);
               
         MPI_Reduce(&(workUnits[0]),NULL,nTimers,MPI_DOUBLE,MPI_SUM,0,printComm);
         MPI_Reduce(&(workUnits[0]),NULL,nTimers,MPI_DOUBLE,MPI_MIN,0,printComm);
         MPI_Reduce(&(count[0]),NULL,nTimers,MPI_INT64_T,MPI_SUM,0,printComm);               
         MPI_Reduce(&(threads[0]),NULL,nTimers,MPI_INT,MPI_SUM,0,printComm);

         MPI_Reduce(&(threadImbalance[0]), NULL, nTimers, MPI_DOUBLE, MPI_SUM, 0, printComm);
         MPI_Reduce(&(threadImbalanceRank[0]), NULL, nTimers, MPI_DOUBLE_INT, MPI_MAXLOC, 0, printComm);
         MPI_Reduce(&(threadImbalanceRank[0]), NULL, nTimers, MPI_DOUBLE_INT, MPI_MINLOC, 0, printComm);
      }
      //clear temporary data structures
      time.clear();
      timeRank.clear();
      count.clear();
      threads.clear();
      threadImbalance.clear();
      threadImbalanceRank.clear();
      workUnits.clear();
      parentIndices.clear();
   }
}





////-------------------------------------------------------------------------
///  PRINT functions

////-------------------------------------------------------------------------      

// Creating std::map of group names to group one-letter ID
// We assume same timers exists in all timer std::vectors, so can use just one here
void ParallelTimerTree::getGroupIds(std::map<std::string, std::string> &groupIds){
   groupIds.clear();
   //add groups to std::map
   for(unsigned int id=0; id< size(); id++) {
      for(auto &group : (*this)[id].getGroups()){
         groupIds[group] = group;
      }
   }
   //assign letters
   int character = 65; // ASCII A, TODO skip after Z to a, see ascii table
   for(std::map<std::string, std::string>::const_iterator group = groupIds.begin();
       group != groupIds.end(); ++group) {
      groupIds[group->first] = character++;
   }
}




//print groups
bool ParallelTimerTree::printGroupStatistics(double minFraction,
                                             const std::map<std::string, std::string> &groupIds,
                                             std::ofstream &output){
   
   PrettyPrintTable table;
   table.addTitle("Groups");
   
   
   //print heders
   table.addHorizontalLine();      
   //row1
   table.addElement("",2);
   table.addElement("Time (s)",6);
   table.addHorizontalLine();
   //row2
   table.addElement("Group",1);
   table.addElement("Name",1);
   table.addElement("Avg",1);      
   table.addElement("% of total",1);      
   table.addElement("Max time,rank",2);      
   table.addElement("Min time,rank",2);      
   table.addHorizontalLine();

   for(unsigned int i=0;i<groupStats.name.size();i++){            
      if(minFraction<=groupStats.timeTotalFraction[i]){
         std::string groupId= groupIds.count(groupStats.name[i]) ? groupIds.find(groupStats.name[i])->second : std::string();
         table.addRow();
         table.addElement(groupId);
         table.addElement(groupStats.name[i]);
         if (nProcessesInPrint > 0)
            table.addElement(groupStats.timeSum[i]/nProcessesInPrint);
         else
            table.addElement(0.0);
         table.addElement(100.0*groupStats.timeTotalFraction[i]);
         table.addElement(groupStats.timeMax[i].val);
         table.addElement(groupStats.timeMax[i].rank);
         table.addElement(groupStats.timeMin[i].val);
         table.addElement(groupStats.timeMin[i].rank);
      }
   }
   table.addHorizontalLine();
   table.print(output);
   return true;
}
         
      
      
//print out global timers
//If any labels differ, then this print will deadlock. Only call it with a communicator that is guaranteed to be consistent on all processes.
bool ParallelTimerTree::printTimers(double minFraction, const std::map<std::string, std::string>& groupIds, std::ofstream &output){
   int rank,nProcesses;

   if(rankInPrint==0){
      PrettyPrintTable table;
      //print Title
      std::stringstream buffer;
      if(minFraction > 0.0 ) {
         
         buffer << "Timers with more than " << minFraction * 100 <<"% of total time. ";
         buffer <<  "Set of identical timers has "<< nProcessesInPrint << " processes";
         
#ifdef _OPENMP
         buffer << " with up to " << omp_get_max_threads() << " threads each";
#endif
         buffer << ".";
      }
      else{
         buffer << "All timers. Set of identical timers has "<< nProcessesInPrint <<" processes";
#ifdef _OPENMP
         buffer << " with up to " << omp_get_max_threads() << " threads each";
#endif
         buffer << ".";
      }
      table.addTitle(buffer.str());
      buffer.str("");
      
      //print heders
      table.addHorizontalLine();      
      //row1
      table.addElement("",4);
      table.addElement("Count",1);
      table.addElement("Process time",3);
      table.addElement("Thread imbalances",3);
      table.addElement("Workunits",1);      
      table.addHorizontalLine();
      //row2
      table.addElement("Id",1);
      table.addElement("Lvl",1);
      table.addElement("Grp",1);
      table.addElement("Name",1);
      table.addElement("Avg",1);
      table.addElement("Avg (s)",1);      
      table.addElement("Time %",1);      
      table.addElement("Imb %",1);
      table.addElement("No",1);            
      table.addElement("Avg %",1);
      table.addElement("Max %",1);

      table.addElement("Avg",1);       
      table.addHorizontalLine();

      //print out all labels recursively
      for(unsigned int i = 1; i < stats.id.size(); i++){
         int id = stats.id[i];
         if(stats.timeTotalFraction[i] >= minFraction){
            //print timer if enough time is spent in it
            if(id != -1) 
               table.addElement(id);
            else
               table.addElement("");
            
            table.addElement(stats.level[i]);
            if(id != -1) {
               //normal timer, not "other" timer
               //get and print group ids
               buffer.str("");
               for(auto &group : (*this)[id].getGroups()){
                  std::string groupId=groupIds.count(group) ? groupIds.find(group)->second : std::string();
                  buffer << groupId;
               }
               table.addElement(buffer.str());
               table.addElement((*this)[id].getLabel(), 1, stats.level[i]-1);
            }
            else{
               table.addElement(""); // no groups
               table.addElement("Other", 1, stats.level[i]-1);
            }

            if(nProcessesInPrint>1 && stats.countSum[i] != 0.0)
               table.addElement(stats.countSum[i]/nProcessesInPrint);
            else
               table.addElement(0.0);
            
            if(nProcessesInPrint>1 && stats.timeSum[i] != 0.0)
               table.addElement(stats.timeSum[i]/nProcessesInPrint);
            else
               table.addElement(0.0);
            
            table.addElement(100.0 * stats.timeParentFraction[i]);
            
            if(nProcessesInPrint>1 && stats.timeMax[i].val != 0.0) {
               double imbTime = stats.timeMax[i].val - stats.timeSum[i]/nProcessesInPrint;
               table.addElement(100 * imbTime / stats.timeMax[i].val  * ( nProcessesInPrint /(nProcessesInPrint - 1 )));
            }
            else{
               table.addElement(0.0);
            }

            if(nProcessesInPrint>1 && stats.threadsSum[i] != 0.0)
               table.addElement(stats.threadsSum[i]/nProcessesInPrint);
            else
               table.addElement(0.0);
            
            if(stats.threadsSum[i]/nProcessesInPrint > 1.0 && stats.threadImbalanceMax[i].val >= 0.0) {
               table.addElement(100.0 * stats.threadImbalanceSum[i]/nProcessesInPrint);
               table.addElement(100.0 * stats.threadImbalanceMax[i].val);
            }
            else {
               //not threaded or an "other" counter for which thread
               //imbalance is not computed
               table.addElement("");
               table.addElement("");
            }
            if(stats.hasWorkUnits[i] && id != -1){
               buffer.str("");
               
               //print if units defined for all processes
               //note how the total rate is computed. This is to avoid one process with little data to     
               //skew results one way or the other                        
               if(stats.timeSum[i]>0){
                  buffer << std::setprecision(4) << stats.workUnitsSum[i]/stats.timeSum[i];
               }
               else if (stats.workUnitsSum[i]>0){
                  //time is zero
                  buffer << "inf";
               }
               else {
                  //zero time zero units
                  buffer << 0;
               }
               buffer << " "<< (*this)[id].getWorkUnitLabel()<<"/s";                     
               table.addElement(buffer.str());

            }
            table.addRow();
         }
      }
      
      table.addHorizontalLine();
      table.print(output);
   }
   return true;
}


//print out global timers
//If any labels differ, then this print will deadlock. Only call it with a communicator that is guaranteed to be consistent on all processes.
bool ParallelTimerTree::printTimersDetailed(double minFraction, const std::map<std::string, std::string>& groupIds, std::ofstream &output){
   int rank,nProcesses;


   if(rankInPrint==0){
      PrettyPrintTable table;

      //print Title
      std::stringstream buffer;
      if(minFraction > 0.0 ) {
         
         buffer << "Timers with more than " << minFraction * 100 <<"% of total time. ";
         buffer <<  "Set of identical timers has "<< nProcessesInPrint << " processes";
         
#ifdef _OPENMP
         buffer << " with up to " << omp_get_max_threads() << " threads each";
#endif
         buffer << ".";
      }
      else{
         buffer << "All timers. Set of identical timers has "<< nProcessesInPrint <<" processes";
#ifdef _OPENMP
         buffer << " with up to " << omp_get_max_threads() << " threads each";
#endif
         buffer << ".";
      }
      table.addTitle(buffer.str());
      buffer.str("");
         

      
      //print heders
      table.addHorizontalLine();      
      //row1
      table.addElement("",4);
      table.addElement("Threads",1);
      table.addElement("Time (s)",6);
      table.addElement("Calls",1);
      table.addElement("Workunit-rate",3);      
      table.addHorizontalLine();
      //row2
      table.addElement("Id",1);
      table.addElement("Lvl",1);
      table.addElement("Grp",1);
      table.addElement("Label",1);
      table.addElement("Avg",1);
      table.addElement("Avg",1);      
      table.addElement("%",1);      
      table.addElement("Max time,rank",2);      
      table.addElement("Min time,rank",2);      
      table.addElement("Avg",1);      
      table.addElement("Total",1);     
      table.addElement("Per process",1);       
      table.addElement("Unit",1);       
      table.addHorizontalLine();

      //print out all labels recursively
      for(unsigned int i = 1; i < stats.id.size(); i++){
         int id = stats.id[i];
         if(stats.timeTotalFraction[i] >= minFraction){
            //print timer if enough time is spent in it
            if(id != -1) 
               table.addElement(id);
            else
               table.addElement("");

            table.addElement(stats.level[i]);
            if(id != -1) {
               //normal timer, not "other" timer
               //get and print group ids
               buffer.str("");
               for(auto &group : (*this)[id].getGroups()){
                  std::string groupId=groupIds.count(group) ? groupIds.find(group)->second : std::string();
                  buffer << groupId;
               }
               table.addElement(buffer.str());
               table.addElement((*this)[id].getLabel(), 1, stats.level[i]-1);
            }
            else{
               table.addElement(""); // no groups
               table.addElement("Other", 1, stats.level[i]-1);
            }
            if(nProcessesInPrint > 0 && stats.threadsSum[i] != 0.0)
               table.addElement(stats.threadsSum[i]/nProcessesInPrint);
            else
               table.addElement(0.0);
            if(nProcessesInPrint > 0 && stats.timeSum[i] != 0.0)
               table.addElement(stats.timeSum[i]/nProcessesInPrint);
            else
               table.addElement(0.0);
            table.addElement(100.0*stats.timeParentFraction[i]);
            table.addElement(stats.timeMax[i].val);            
            table.addElement(stats.timeMax[i].rank);
            table.addElement(stats.timeMin[i].val);
            table.addElement(stats.timeMin[i].rank);
            if(nProcessesInPrint > 0 && stats.countSum[i] != 0.0)
               table.addElement(stats.countSum[i]/nProcessesInPrint);
            else
               table.addElement(0.0);

            if(stats.hasWorkUnits[i]){
               //print if units defined for all processes
               //note how the total rate is computed. This is to avoid one process with little data to     
               //skew results one way or the other                        
               if(stats.timeSum[i]>0){
                  table.addElement(nProcessesInPrint*(stats.workUnitsSum[i]/stats.timeSum[i]));
                  table.addElement(stats.workUnitsSum[i]/stats.timeSum[i]);
               }
               else if (stats.workUnitsSum[i]>0){
                  //time is zero
                  table.addElement("inf");
                  table.addElement("inf");
               }
               else {
                  //zero time zero units
                  table.addElement(0);
                  table.addElement(0);
               }
               buffer.str("");
               buffer << (*this)[id].getWorkUnitLabel()<<"/s";                     
               table.addElement(buffer.str());
            }
            table.addRow();
         }
      }
      
      table.addHorizontalLine();
      table.print(output);
   }
   return true;
}


bool ParallelTimerTree::getPrintCommunicator(int &printIndex,int &timersHash){
   int mySuccess=1;
   int success;
   timersHash=getHash();
   int result = MPI_Comm_split(comm, timersHash, 0, &printComm);
         
   if (result != MPI_SUCCESS) {
      int error_string_len = MPI_MAX_ERROR_STRING;
      char error_string[MPI_MAX_ERROR_STRING + 1];
      MPI_Error_string(result, error_string, &error_string_len);
      error_string[MPI_MAX_ERROR_STRING] = 0;
      if(rank==0)
         std::cerr << "PHIPROF-ERROR: Error splitting communicator for printing: " << error_string << std::endl;
      mySuccess=0;
   }
         
   //Now compute the id for the print comms. First communicatr is given index 0, next 1 and so on.
   //get rank in printComm
   MPI_Comm_rank(printComm,&rankInPrint);
   MPI_Comm_size(printComm,&nProcessesInPrint);
   
   //communicator with printComm masters(rank=0), this will be used to number the printComm's
   MPI_Comm printCommMasters;
   MPI_Comm_split(comm,rankInPrint==0,-nProcessesInPrint,&printCommMasters);
   MPI_Comm_rank(printCommMasters,&printIndex);
   MPI_Comm_free(&printCommMasters);
   MPI_Bcast(&printIndex,1,MPI_INT,0,printComm);

   //check that the hashes at least have the same number of timers, just to be sure and to avoid crashes...

   int nTimers=size();
   int maxTimers;
   int minTimers;
         
   MPI_Reduce(&nTimers,&maxTimers,1,MPI_INT,MPI_MAX,0,printComm);
   MPI_Reduce(&nTimers,&minTimers,1,MPI_INT,MPI_MIN,0,printComm);

   if(rankInPrint==0) { 
      if(minTimers !=  maxTimers) {
         std::cerr << "PHIPROF-ERROR: Missmatch in number of timers, hash conflict?  maxTimers = " << maxTimers << " minTimers = " << minTimers << std::endl;
         mySuccess=0;
      }
   }
         
   //if any process failed, the whole routine failed
   MPI_Allreduce(&mySuccess,&success,1,MPI_INT,MPI_MIN,comm);
         
   return success;
}



bool ParallelTimerTree::print(MPI_Comm communicator, std::string fileNamePrefix){
   int timersHash,printIndex;
   
   //printStartTime defined in namespace, used to correct timings for open timers
   printStartTime = wTime();
   comm = communicator; //no dup, we will only use it without
   MPI_Comm_rank(comm, &rank);
   MPI_Comm_size(comm, &nProcesses);
   MPI_Barrier(comm);

   //get hash value of timers and the print communicator
   if(getPrintCommunicator(printIndex, timersHash)) {
      //generate file name
      std::stringstream fname;
      fname << fileNamePrefix << "_" << printIndex << ".txt";
      collectTimerStats(rank);
      collectGroupStats();
      
      if(rankInPrint == 0){
         char *envVariable;
         std::vector<std::string> prints;
         std::ofstream output;
         std::map<std::string, std::string> groupIds;

         /*read from environment variable what to print**/
         envVariable = getenv("PHIPROF_PRINTS");
         if(envVariable != NULL) {
            char *substring;
            substring = strtok(envVariable, ",");
            while(substring != NULL) {
               prints.push_back(std::string(substring));
               substring = strtok(NULL, ",");
            }
         }
         else {
            //set default print
            prints.push_back("groups");
            prints.push_back("compact");
         }
         
         getGroupIds(groupIds);
         output.open(fname.str(), std::fstream::out);
         if (output.good() == false)
            return false;
         
         for(const auto& p: prints) {
            if(p == "groups")
               printGroupStatistics(0.0, groupIds, output);
            else if(p=="compact")
               printTimers(0.01, groupIds, output);
            else if(p=="full")
               printTimers(0.0, groupIds, output);
            else if(p=="detailed")
               printTimersDetailed(0.0, groupIds, output);
            else
               if(rank == 0)
                  //Only really need the warning from one process
                  std::cerr <<"phiprof warning: nonexistent print style " << p << " in PHIPROF_PRINTS" << std::endl;
         }
         output.close();
      }
      MPI_Comm_free(&printComm);
   }

   MPI_Barrier(comm);   
   double endPrintTime = wTime();
   shiftActiveStartTime(endPrintTime - printStartTime);
   
   
   return true;
}
   
