/*
  This file is part of the phiprof library

  Copyright 2011, 2012 Finnish Meteorological Institute

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


#include "mpi.h"
#include "omp.h"
#include "iostream"
#include "phiprof.hpp"

using namespace std;

int main(int argc,char **argv){

   int rank;
   MPI_Init(&argc,&argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   
   /*First initialize phiprof before calling other phiprof functions*/
   phiprof::initialize();
   
   /*One can start regions by giving them a name. All rehions need to
     be closed wiht a stop. Initializes the timer if it does not yet
     exist*/
   phiprof::start("Greetings");
   /*Two timers are created withing the Greetings timer. Separately
   initializing them enables one to use the id for more efficient
   starts & stops. Especially important for OpenMP since when a timer
   is started with a string label there will be a crtical region
   */
   int timer1 = phiprof::initializeTimer("getThread"); 
   int timer2 = phiprof::initializeTimer("cout"); 
#pragma omp parallel 
   {
      /*Measure region timer1 within a threaded region*/
      phiprof::start(timer1); 
      int thread = omp_get_thread_num();
      phiprof::stop(timer1);
      
      /*Measure region timer2 within a threaded region. In stop we also
        measure a performance metric, how many couts were done*/   
      phiprof::start(timer2);
#pragma omp critical(print) 
      cout << "Hello world from rank " << rank << " thread " << thread << endl;
      phiprof::stop(timer2, 1, "greetings");
   }
   
   /* End also outer region*/ 
   phiprof::stop("Greetings");
   
  /*Initialize timer separately. This time the motivation is to define
    a group it belongs to.*/
   phiprof::initializeTimer("LB","MPI");
   phiprof::start("LB");
   MPI_Barrier(MPI_COMM_WORLD);
   phiprof::stop("LB");
   
   /*print out final results into profile_0.txt*/
   phiprof::print(MPI_COMM_WORLD);
   
   MPI_Finalize();
   return 0;
}

