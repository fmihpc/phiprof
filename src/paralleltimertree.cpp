#include <iostream>    
#include <sstream>    
#include <iomanip>    
#include <vector>
#include <string>
#include <set>
#include <limits>
#include <algorithm>
#include <time.h>
#include "paralleltimertree.hpp"
#include "common.hpp"
#ifdef _OPENMP
#include "omp.h"
#endif
#include "mpi.h"
//defines print-area widths for print() output
const int _indentWidth=2; //how many spaces each level is indented
const int _floatWidth=11; //width of float fields;
const int _intWidth=6;   //width of int fields;
const int _unitWidth=4;  //width of workunit label
const int _levelWidth=5; //width of level label


   


//ParallelTimerTree::ParallelTimerTree(){
//}
   



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
   static std::vector<int> parentIndices;
   int currentIndex;
   doubleRankPair in;


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
   double currentTime=getTime(id);
         
   stats.id.push_back(id);
   stats.level.push_back((*this)[id].getLevel());
   
   time.push_back(currentTime);
   in.val=currentTime;
   in.rank=reportRank;
   timeRank.push_back(in);
   count.push_back((*this)[id].getAverageCount());
   threads.push_back((*this)[id].getThreads());
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
      workUnits.push_back(-1);
      parentIndices.push_back(currentIndex);
   }
         
   //End of function for id=0, we have now collected all timer data.
   //compute statistics now
   if(id==0){
      int nTimers=time.size();
      if(rankInPrint==0){
         stats.timeSum.resize(nTimers);
         stats.timeMax.resize(nTimers);
         stats.timeMin.resize(nTimers);
         stats.workUnitsSum.resize(nTimers);
         stats.hasWorkUnits.resize(nTimers);
         stats.countSum.resize(nTimers);
         stats.threadsSum.resize(nTimers);

         stats.timeTotalFraction.resize(nTimers);
         stats.timeParentFraction.resize(nTimers);
         std::vector<double> workUnitsMin;
         workUnitsMin.resize(nTimers);
               
         MPI_Reduce(&(time[0]),&(stats.timeSum[0]),nTimers,MPI_DOUBLE,MPI_SUM,0,printComm);
         MPI_Reduce(&(timeRank[0]),&(stats.timeMax[0]),nTimers,MPI_DOUBLE_INT,MPI_MAXLOC,0,printComm);
         MPI_Reduce(&(timeRank[0]),&(stats.timeMin[0]),nTimers,MPI_DOUBLE_INT,MPI_MINLOC,0,printComm);
               
         MPI_Reduce(&(workUnits[0]),&(stats.workUnitsSum[0]),nTimers,MPI_DOUBLE,MPI_SUM,0,printComm);
         MPI_Reduce(&(workUnits[0]),&(workUnitsMin[0]),nTimers,MPI_DOUBLE,MPI_MIN,0,printComm);
         MPI_Reduce(&(count[0]),&(stats.countSum[0]),nTimers,MPI_INT64_T,MPI_SUM,0,printComm);
         MPI_Reduce(&(threads[0]),&(stats.threadsSum[0]),nTimers,MPI_INT,MPI_SUM,0,printComm);
               
         for(int i=0;i<nTimers;i++){
            if(stats.workUnitsSum[i] <= 0)
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
         //not masterank, we do not resize and use stats std::vectors
         MPI_Reduce(&(time[0]),NULL,nTimers,MPI_DOUBLE,MPI_SUM,0,printComm);
         MPI_Reduce(&(timeRank[0]),NULL,nTimers,MPI_DOUBLE_INT,MPI_MAXLOC,0,printComm);
         MPI_Reduce(&(timeRank[0]),NULL,nTimers,MPI_DOUBLE_INT,MPI_MINLOC,0,printComm);
               
         MPI_Reduce(&(workUnits[0]),NULL,nTimers,MPI_DOUBLE,MPI_SUM,0,printComm);
         MPI_Reduce(&(workUnits[0]),NULL,nTimers,MPI_DOUBLE,MPI_MIN,0,printComm);
         MPI_Reduce(&(count[0]),NULL,nTimers,MPI_INT64_T,MPI_SUM,0,printComm);               
         MPI_Reduce(&(threads[0]),NULL,nTimers,MPI_INT,MPI_SUM,0,printComm);
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





////-------------------------------------------------------------------------
///  PRINT functions

////-------------------------------------------------------------------------      

// Creating std::map of group names to group one-letter ID
// We assume same timers exists in all timer std::vectors, so can use just one here
void ParallelTimerTree::getGroupIds(std::map<std::string, std::string> &groupIds,size_t &groupWidth){
   groupIds.clear();
   groupWidth=6;
   //add groups to std::map
   for(unsigned int id=0; id< size(); id++) {
      groupWidth = std::max((*this)[id].getGroups().size(), groupWidth);               
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


//print all timers in stats
bool ParallelTimerTree::printTreeTimerStatistics(double minFraction, size_t labelWidth, size_t groupWidth, int totalWidth,
                                                 const std::map<std::string,std::string> &groupIds,  std::fstream &output){
   
   for(int i = 0; i < totalWidth / 2 - 5; i++) output <<"-";
   output << " Profile ";
   for(int i = 0; i < totalWidth / 2 - 5; i++) output <<"-";
   output << std::endl;

   for(unsigned int i = 1; i < stats.id.size(); i++){
      int id = stats.id[i];
      if(stats.timeTotalFraction[i] >= minFraction){
         //print timer if enough time is spent in it
         bool hasNoGroups = true;
         int indent=(stats.level[i]-1)*_indentWidth;
         output << std::setw(_levelWidth+1) << stats.level[i];
               
         if(id!=-1){
            //other label has no groups
            for(auto &group : (*this)[id].getGroups()){
               std::string groupId=groupIds.count(group) ? groupIds.find(group)->second : std::string();
               output << std::setw(1) << groupId;
               hasNoGroups = false;
            }
         }
               
         if(hasNoGroups) output << std::setw(groupWidth+1) << "";
         else output << std::setw(groupWidth-(*this)[id].getGroups().size()+1) << "";
	       
         output << std::setw(indent) << "";
         if(id!=-1){
            output << std::setw(labelWidth+1-indent) << std::setiosflags(std::ios::left) << (*this)[id].getLabel();
         }
         else{
            output << std::setw(labelWidth+1-indent) << std::setiosflags(std::ios::left) << "Other";
         }

         output << std::setw(_floatWidth+1) << stats.threadsSum[i]/nProcesses;        	       
         output << std::setw(_floatWidth) << stats.timeSum[i]/nProcesses;
         output << std::setw(_floatWidth) << 100.0*stats.timeParentFraction[i];
         output << std::setw(_floatWidth) << stats.timeMax[i].val;
         output << std::setw(_intWidth)   << stats.timeMax[i].rank;
         output << std::setw(_floatWidth) << stats.timeMin[i].val;
         output << std::setw(_intWidth)   << stats.timeMin[i].rank;
         output << std::setw(_floatWidth) << stats.countSum[i]/nProcesses;

         if(stats.hasWorkUnits[i]){
            //print if units defined for all processes
            //note how the total rate is computed. This is to avoid one process with little data to     
            //skew results one way or the other                        
            if(stats.timeSum[i]>0){
               output << std::setw(_floatWidth) << nProcesses*(stats.workUnitsSum[i]/stats.timeSum[i]);
               output << std::setw(_floatWidth) << stats.workUnitsSum[i]/stats.timeSum[i];
            }
            else if (stats.workUnitsSum[i]>0){
               //time is zero
               output << std::setw(_floatWidth) << "inf";
               output << std::setw(_floatWidth) << "inf";
            }
            else {
               //zero time zero units
               output << std::setw(_floatWidth) << 0;
               output << std::setw(_floatWidth) << 0;
            }
            output << (*this)[id].getWorkUnitLabel()<<"/s";                     
         }
         output<<std::endl;
      }
            
   }
   return true;
}
      

bool ParallelTimerTree::printTreeFooter(int totalWidth,std::fstream &output){
   for(int i=0;i<totalWidth;i++) output <<"-";
   output<<std::endl;
   return true;
}
      
bool ParallelTimerTree::printTreeHeader(double minFraction,size_t labelWidth,size_t groupWidth,int totalWidth,int nProcs,std::fstream &output){
   for(int i=0;i<totalWidth;i++) output <<"-";
   output<<std::endl;
   output << "Phiprof results with time fraction of total time larger than " << minFraction;
   output<<std::endl;
   output << "Processes in set of timers " << nProcs;
#ifdef _OPENMP
   output << " with (up to) " << omp_get_max_threads() << " threads ";
#endif
   output<<std::endl;
   output << "Timer resolution is "<< wTick() << std::endl;
   for(int i=0;i<totalWidth;i++) output <<"-";
   output<<std::endl;
   output<<std::setw(_levelWidth+1+groupWidth+1+labelWidth+1)<< std::setiosflags(std::ios::left) << "";
   output<<std::setw(_floatWidth)<< "Threads";
   output<<std::setw(4*_floatWidth+2*_intWidth) <<"Time(s)";
   output<<std::setw(_floatWidth)<<"Calls";
   output<<std::setw(2*_floatWidth)<<"Workunit-rate";
   output<<std::endl;
   output<<std::setw(_levelWidth+1)<< "Level";	    
   output<<std::setw(groupWidth+1)<< "Groups";
//         output << std::setw(1) << "|";
   output<<std::setw(labelWidth+1)<< "Label";
//         output << std::setw(1) << "|";
   //  time
   output<<std::setw(_floatWidth) <<"Average";
   output<<std::setw(_floatWidth) <<"Average";
   output<<std::setw(_floatWidth) <<"parent %";
   output<<std::setw(_floatWidth) <<"Maximum";
   output<<std::setw(_intWidth) << "Rank";
   output<<std::setw(_floatWidth)<< "Minimum";
   output<<std::setw(_intWidth) << "Rank";
   //call count
   output<<std::setw(_floatWidth) << "Average";
   // workunit rate    
   output<<std::setw(_floatWidth) << "Average";
   output<<std::setw(_floatWidth) << "Per process";
   output<<std::endl;
      
   return true;
}

//print groups
bool ParallelTimerTree::printTreeGroupStatistics(double minFraction,
                                                 size_t labelWidth,
                                                 size_t groupWidth,
                                                 int totalWidth,
                                                 const std::map<std::string, std::string> &groupIds,
                                                 std::fstream &output){
   
   for(int i=0;i<totalWidth/2 -4;i++) output <<"-";
   output <<" Groups ";
   for(int i=0;i<totalWidth/2 -3;i++) output <<"-";
   output<<std::endl;

   for(unsigned int i=0;i<groupStats.name.size();i++){            
      if(minFraction<=groupStats.timeTotalFraction[i]){
         std::string groupId= groupIds.count(groupStats.name[i]) ? groupIds.find(groupStats.name[i])->second : std::string();
         output << std::setw(_levelWidth+1) << " ";
         output << std::setw(groupWidth+1) << groupId;
         output << std::setw(labelWidth+1) << groupStats.name[i];
         output << std::setw(_floatWidth) << " ";
         output << std::setw(_floatWidth) << groupStats.timeSum[i]/nProcesses;
         output << std::setw(_floatWidth) << 100.0*groupStats.timeTotalFraction[i];
         output << std::setw(_floatWidth) << groupStats.timeMax[i].val;
         output << std::setw(_intWidth)   << groupStats.timeMax[i].rank;
         output << std::setw(_floatWidth) << groupStats.timeMin[i].val;
         output << std::setw(_intWidth)   << groupStats.timeMin[i].rank;
         output << std::endl;
      }
   }

   return true;
}
         
      
      
//print out global timers
//If any labels differ, then this print will deadlock. Only call it with a communicator that is guaranteed to be consistent on all processes.
bool ParallelTimerTree::printTree(double minFraction, std::string fileName){
   int rank,nProcesses;
   std::map<std::string, std::string> groupIds;
   size_t labelWidth=0;    //width of column with timer labels
   size_t groupWidth=0;    //width of column with group letters
   int totalWidth=6;       //total width of the table
         
   //compute labelWidth
   if(rankInPrint==0){
      std::fstream output;
      output.open(fileName.c_str(), std::fstream::out);
      if (output.good() == false)
         return false;
            
      for (unsigned int i=0;i<size();i++){
         size_t width=(*this)[i].getLabel().length()+((*this)[i].getLevel()-1)*_indentWidth;
         labelWidth=std::max(labelWidth,width);
      }

      getGroupIds(groupIds,groupWidth);
	    
      //make sure we use default floats, and not fixed or other format
      output <<std::setiosflags( std::ios::floatfield );
      //set       float p  rec  ision
      output <<std::setprecision(_floatWidth-6); //6 is needed for ".", "e+xx" and a space
            
      unsigned int labelDelimeterWidth=2;
      totalWidth=_levelWidth+1+groupWidth+1+labelWidth+1+labelDelimeterWidth+_floatWidth*7+_intWidth*2+_unitWidth;
         
      //print header 
      printTreeHeader(minFraction,labelWidth,groupWidth,totalWidth,nProcesses,output);
      //print out all labels recursively
      printTreeTimerStatistics(minFraction,labelWidth,groupWidth,totalWidth,groupIds,output);
      //print groups
      printTreeGroupStatistics(minFraction,labelWidth,groupWidth,totalWidth,groupIds,output);
      //print footer  
      printTreeFooter(totalWidth,output);
      // start root timer again in case we continue and call print several times
      output.close();
            
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



bool ParallelTimerTree::print(MPI_Comm communicator, std::string fileNamePrefix,double minFraction){
   int timersHash,printIndex;
   
   //printStartTime defined in namespace, used to correct timings for open timers
   printStartTime = wTime();
   comm = communicator; //no dup, we will only use it without
   MPI_Comm_rank(comm, &rank);
   MPI_Comm_size(comm, &nProcesses);
   MPI_Barrier(comm);
   /*
   if(rank == 0) {
      std::cout << "rank 0, timerdata 1: label" << (*this)[1].getLabel() << std::endl;
      std::cout << "rank 0, timerdata 1: id" << (*this)[1].getId()  << std::endl;
      std::cout << "rank 0, timerdata 1: parentid" << (*this)[1].getParentId()  << std::endl     ;
      std::cout << "rank 0, timerdata 1: workunits" << (*this)[1].workUnitLabel  << std::endl     ;
   }
   */

   //get hash value of timers and the print communicator
   if(getPrintCommunicator(printIndex, timersHash)) {
      //generate file name
      std::stringstream fname;
      fname << fileNamePrefix << "_" << printIndex << ".txt";
      collectTimerStats(rank);
      collectGroupStats();
      printTree(minFraction,fname.str());
   } 
   MPI_Comm_free(&printComm);
   MPI_Barrier(comm);   
   double endPrintTime = wTime();
   shiftActiveStartTime(endPrintTime - printStartTime);
   
   
   return true;
}
   
