/*
This file is part of the phiprof library

Copyright 2010, 2011, 2012 Finnish Meteorological Institute

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

#ifndef PHIPROF_HPP
#define PHIPROF_HPP

#include "string"
#include "vector"
#include "mpi.h"

#ifndef NDEBUG
#define phiprof_assert2(a,b) phiprof::phiprofAssert( a, b, __FILE__, __LINE__ )
#define phiprof_assert1(a) phiprof::phiprofAssert( a, #a, __FILE__, __LINE__ )
#else
#define phiprof_assert2(a,b)
#define phiprof_assert1(a) 
#endif
#define GET_MACRO(_1,_2,NAME,...) NAME
#define phiprof_assert(...) GET_MACRO(__VA_ARGS__, phiprof_assert2, phiprof_assert1)(__VA_ARGS__)

   

/* This files contains the C++ interface */

namespace phiprof
{

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

   bool print(MPI_Comm comm,std::string fileNamePrefix="profile",double minFraction=0.0);
   
    /**
   * Print the current timer state in a easily parsable format
   *
   * This function will print the timer statistics in a text based
   * parsable format, each unique set of hierarchical profiles
   * (labels, hierarchy, workunits) will be written out to a separate
   * file. This function will print the times since the last call to
   * printLogProfile, or since initialization for the first call to
   * printLogProfile. It is meant to be called multiple times, to
   * track the develpoment of performance metrics. 
   *
   *
   * @param comm
   *   Communicator for processes that print their profile.
   * @param fileNamePrefix
   *   (optional) Default value is "profile_log"
   *   The first part of the filename where the profile is printed. Each
   *   unique set of timers (label, hierarchy, workunits) will be
   *   assigned a unique hash number and the profile will be written
   *   out into a file called fileprefix_hash.txt
   * @param separator
   *   (optional) Default value is " "
   *   The separator between fields in the file.
   * @param maxLevel
   *   (optional) Default value is to print all timers
   *   Maxlevel can be used to limit the number of timers that are
   *   printed out. Only timers with a level in the
   *   hierarchy<=maxLevel are printed.
   * @return
   *   Returns true if pofile printed successfully.
   */
  bool printLogProfile(MPI_Comm comm,double simulationTime,std::string fileNamePrefix="profile_log",std::string separator=" ",int maxLevel=0);

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
   int initializeTimer(const std::string &label,const std::vector<std::string> &groups);
  /**
   * \overload int phiprof::initializeTimer(const std::string &label,const std::vector<std::string> &groups);
   */
   int initializeTimer(const std::string &label);
  /**
   * \overload int phiprof::initializeTimer(const std::string &label,const std::vector<std::string> &groups);
   */
   int initializeTimer(const std::string &label,const std::string &group1);
  /**
   * \overload int phiprof::initializeTimer(const std::string &label,const std::vector<std::string> &groups);
   */
   int initializeTimer(const std::string &label,const std::string &group1,const std::string &group2);
  /**
   * \overload int phiprof::initializeTimer(const std::string &label,const std::vector<std::string> &groups);
   */
   int initializeTimer(const std::string &label,const std::string &group1,const std::string &group2,const std::string &group3);



  
   int getId(const std::string &label);
   
  /*
   * \overload bool phiprof::start(const std::string &label)
   */
   bool start(int id);

   /**
   * \overload  bool phiprof::stop(const std::string &label,double workUnits=-1.0,const std::string &workUnitLabel="")
   */
   bool stop (int id,
              double workUnits=-1.0,
              const std::string &workUnitLabel="");


   /**
   * Assert function
   *
   * It prints out the error message, the suplied line and file
   * information. It also prints out the current position in the timer
   * stack in phiprof. Finally it terminates the process, and thus the
   * whole parallel program. Can also be used through the
   * phiprof_assert macro
   *
   * @param condition
   *   If false, then the error is raised.
   * @param error_message
   *   The error message
   * @param file
   *   The source  file where this is called, typically supplied by  __FILE__
   * @param line     
   *   The line in the source  file where this is called, typically supplied by  __LINE__ 
   */
   void phiprofAssert(bool condition, const std::string error_message, const std::string  file, int line );
}


#endif
