#ifndef COMMON_H
#define COMMON_H
#include <time.h>

//djb2 hash function copied from
//http://www.cse.yorku.ca/~oz/hash.html
inline unsigned long hash(const char *str) {
   unsigned long hash = 5381;
   int c;
   while ( (c = *str++) )
      hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
   return hash;
}



//this function returns the time in seconds . 
inline double wTime(){
//time struct to get wall time
   struct timespec t;
   clock_gettime(CLOCK_ID,&t);
   return t.tv_sec + 1.0e-9 * t.tv_nsec;
}
//this function returns the accuracy of the timer     
inline double wTick(){
   struct timespec t;
   clock_getres(CLOCK_ID,&t);
   return t.tv_sec + 1.0e-9 * t.tv_nsec;
} 
  
#endif
