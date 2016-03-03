#ifndef PARALLELTIMERTREE_H
#define PARALLELTIMERTREE_H
#include <vector>
#include <string>
#include <map>
#include <fstream>

#include "mpi.h"
#include "timertree.hpp"

class ParallelTimerTree: public TimerTree  {
public:

   /**
    * Print the  current timer state in a human readable file
    *
    * This function will print the timer statistics in a text based
    * hierarchical form into file(s), each unique set of hierarchical
    * profiles (labels, hierarchy, workunits) will be written out to a
    * separate file. This function will print the times since the
    * ininitalization of phiprof in the first start call. It can be
    * called multiple times, and will not close currently active
    * timers. The time spent in active timers uptill the print call is
    * taken into account, and the time spent in the print function will
    * be corrected for in them.
    *
    *
    * @param comm
    *   Communicator for processes that print their profile.
    * @param fileNamePrefix
    *   (optional) Default value is "profile"
    *   The first part of the filename where the profile is printed. Each
    *   unique set of timers (label, hierarchy, workunits) will be
    *   assigned a unique hash number and the profile will be written
    *   out into a file called fileprefix_hash.txt
    * @param minFraction
    *   (optional) Default value is to print all timers
    *   minFraction can be used to filter the timers being printed so
    *   that only the ones with a meaningfull amount of time are
    *   printed. Only timers with (timer time)/(total time)>=minFraction
    *   are printed. If minfraction is <=0.0 then all timers are printed.
    * @return
    *   Returns true if pofile printed successfully.
    */
   bool print(MPI_Comm comm, std::string fileNamePrefix="profile", double minFraction=0.0);
   
private:

   //used with MPI reduction operator
   struct doubleRankPair {
      double val;
      int rank;
   };

   struct TimerStatistics {
      std::vector<int> id; //id of the timer at this index in the statistics vectors
      std::vector<int> level;
      std::vector<double> timeSum;
      std::vector<doubleRankPair> timeMax;
      std::vector<doubleRankPair> timeMin;
      std::vector<double> timeTotalFraction;
      std::vector<double> timeParentFraction;
      std::vector<bool> hasWorkUnits;
      std::vector<double> workUnitsSum;
      std::vector<int64_t> countSum;
      
      std::vector<int> threadsSum;
      std::vector<double> threadImbalanceSum;
      std::vector<doubleRankPair> threadImbalanceMax;
      std::vector<doubleRankPair> threadImbalanceMin;
   };
   TimerStatistics stats;
   
      
   struct GroupStatistics {
      std::vector<std::string> name; 
      std::vector<double> timeSum;
      std::vector<doubleRankPair> timeMax;
      std::vector<doubleRankPair> timeMin;
      std::vector<double> timeTotalFraction;
   };
   GroupStatistics groupStats;

   void collectGroupStats();
   void getGroupIds(std::map<std::string, std::string>  &groupIds);
   void collectTimerStats(int reportRank,int id=0,int parentIndex=0);

   
   bool printTimers(double minFraction, 
                    const std::map<std::string, std::string> &groupIds,                  
                    std::ofstream &output);   
   bool printTimersDetailed(double minFraction, 
                    const std::map<std::string, std::string> &groupIds,                  
                    std::ofstream &output);   
   
   bool printGroupStatistics(double minFraction,
                             const std::map<std::string, std::string> &groupIds,
                             std::ofstream &output);

   bool getPrintCommunicator(int &printIndex, int &timersHash);

   MPI_Comm comm;
   MPI_Comm printComm;
   int rank;
   int nProcesses;
   int rankInPrint;
   int nProcessesInPrint;
   double printStartTime;
   
   // Updated in collectStats, only valid on root rank
   



};




#endif
