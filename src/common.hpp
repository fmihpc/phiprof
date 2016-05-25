/*
This file is part of the phiprof library

Copyright 2012, 2013, 2014, 2015 Finnish Meteorological Institute
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

#ifndef COMMON_H
#define COMMON_H
#include <time.h>

//djb2 hash function copied from
//http://www.cse.yorku.ca/~oz/hash.html
//TODO: replace by a better one
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
