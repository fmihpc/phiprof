#ifndef PARALLELTIMERTREE_H
#define PARALLELTIMERTREE_H
#include <vector>
#include <string>
#include <map>
#include <fstream>

#include "mpi.h"
#include "timertree.hpp"

class ParallelTimerTree  {
public:
   //ParallelTimerTree();
   

 //constructor

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
   bool start(const std::string &label){
      return timerTree.start(label);
   }
   /*
    * \overload bool phiprof::start(const std::string &label)
   */
   bool start(int id){
      return timerTree.start(id);
   }
   
   int getId(const std::string &label) const{
      return timerTree.getId(label);
   };

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
             const std::string &workUnitLabel=""){
     return timerTree.stop(label, workUnits, workUnitLabel);
  }
   
   /**
    * \overload  bool phiprof::stop(const std::string &label,double workUnits=-1.0,const std::string &workUnitLabel="")
   */
   bool stop (int id,
              double workUnits,
              const std::string &workUnitLabel){
      return timerTree.stop(id, workUnits, workUnitLabel);
   }

   /**
   * Fastest stop routine for cases when no workunits are defined.
   */
   bool stop (int id){
      return timerTree.stop(id);
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
   int initializeTimer(const std::string &label,const std::vector<std::string> &groups){
      return timerTree.initializeTimer(label, groups); 
   }
   


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
   void getGroupIds(std::map<std::string, std::string>  &groupIds, size_t &groupWidth);
   void collectTimerStats(int reportRank,int id=0,int parentIndex=0);

   
   bool printTree(double minFraction,std::string fileName);
   bool printTreeHeader(double minFraction, size_t labelWidth, size_t groupWidth, 
                           int totalWidth, int nProcs, std::fstream &output);
   bool printTreeTimerStatistics(double minFraction, size_t labelWidth, 
                                 size_t groupWidth, int totalWidth,
                                 const std::map<std::string,std::string> &groupIds, 
                                 std::fstream &output);   
   bool printTreeGroupStatistics(double minFraction,
                                 size_t labelWidth,
                                 size_t groupWidth,
                                 int totalWidth,
                                 const std::map<std::string, std::string> &groupIds,
                                 std::fstream &output);
   bool printTreeFooter(int totalWidth, std::fstream &output);
   bool getPrintCommunicator(int &printIndex, int &timersHash);

   MPI_Comm comm;
   MPI_Comm printComm;
   int rank;
   int nProcesses;
   int rankInPrint;
   int nProcessesInPrint;
   double printStartTime;

   TimerTree timerTree;


// Updated in collectStats, only valid on root rank
   



};




#endif
