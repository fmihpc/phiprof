#ifndef TIMERTREE_H
#define TIMERTREE_H
#include <vector>
#include "timerdata.hpp"


class TimerTree {
public:
   TimerTree(); //constructor   

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
   bool start(int id);

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

   /**
    * 
   */
   bool stop (int id,
              double workUnits);
   

   /**
   * Fastest stop routine for cases when no workunits are defined.
   */
   bool stop (int id);


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
   int getHash(int id=0) const;
   std::string getFullLabel(int id,bool reverse=false) const;  

private:
   int currentId;
   std::vector<TimerData> timers;


   
};




#endif
