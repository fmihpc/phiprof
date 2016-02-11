//initialize profiler, called by first start/initializeTimer. This adds the root timer
TimerTree(){
   vector<string> group;
   group.push_back("Total");
   //no timer yet        
   _currentId=-1;
   //mainId will be 0, parent is -1 (does not exist)
   int id=constructTimer("total",-1,group);
   //start root timer, is stopped in print.      
   start(id);
}


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


//this function returns the time in seconds . 
double getTime() {
  struct timespec t;
  clock_gettime(CLOCK_ID,&t);
  return t.tv_sec + 1.0e-9 * t.tv_nsec;
}
//this function returns the accuracy of the timer     
double getTick() {
  struct timespec t;
  clock_getres(CLOCK_ID,&t);
  return t.tv_sec + 1.0e-9 * t.tv_nsec;
}


double getTime(int id){
   double time;
   time=timers[id].time;
   //add time uptill print for active timers
   if(timers[id].active)
      time+=getTime()-timers[id].startTime;
   return time;
}


double getGroupTime(int id, string group){
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
int getHash(const vector<TimerData> &timers,int id=0){
   unsigned long hashValue;
   //add hash values from label, workunitlabel and groups. Everything has to match.
   hashValue=hash(timers[id].label.c_str());
   hashValue+=hash(timers[id].workUnitLabel.c_str());
   for (vector<string>::const_iterator g = timers[id].groups.begin();g != timers[id].groups.end(); ++g ) {
      hashValue+=hash((*g).c_str());
   }
         
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
         
