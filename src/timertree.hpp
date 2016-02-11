
#ifndef TIMERTREE_H
#define TIMERTREE_H

#include <vector>
#include <set>
#include <map>
#include "timerdata.h"

class TimerTree {
 public:
  TimerTree(); //constructor
  int constructTimer(const string &label,int parentId,const vector<string> groups);  
  string getFullLabel(const vector<TimerData> &timers,int id,bool reverse=false);  
  double getTime(int id);
  double getGroupTime(int id, string group);

  int getHash(const vector<TimerData> &timers,int id=0);
 private:
  //current position in timer hierarchy 
  int _currentId=-1;
  unsigned long hash(const char *str);
  //vector with timers, cumulative and for log print
  vector<TimerData> _cumulativeTimers;
  double getTime();
  double getTick();
  
}



#endif
