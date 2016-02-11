#include <vector>
#include <set>
#include <map>

#ifndef TIMERDATA_H
#define TIMERDATA_H


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
  int64_t count; //how many times have this been accumulated
  double time; // total time accumulated
  double startTime; //Starting time of previous start() call
  
  bool active;
};


#endif
