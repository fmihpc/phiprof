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
#include <iostream>
#include <iomanip> 
#include <string>    
#include <vector>     
#include "mpi.h"
#include "omp.h"
#include "phiprof.hpp"

using namespace std;

double compute(double seconds){
   const double overhead=0.000002; //estimated
   double a,b,c,d;
   int  i,iterations=1000;
   double t1,t2;

   a=1.00004;
   b=1e-10;
   c=1.00011;
   d=0;

   t2=t1=MPI_Wtime();
   while(t2-t1<seconds-overhead){
      for(i=0;i<iterations;i++){
         d+=a*b+c;
      }
      t2=MPI_Wtime();
   }
   return d;
}


int main(int argc,char **argv){

   int rank;
   MPI_Init(&argc,&argv);
   MPI_Comm_rank(MPI_COMM_WORLD,&rank);
   const int nIterations=1000000;
   if(rank==0)
      cout << "Measuring performance of start-stop calls" <<endl;
   
   phiprof::initialize();
   
   phiprof::Timer benchmark {"Benchmarking phiprof"}; 

   phiprof::Timer init {"Initalized timers using ID"};

   if(rank==0)
      cout << "  1/3" <<endl;
   int id_a = phiprof::initializeTimer("a","A with ID");
   for(int i=0;i<nIterations;i++){
      phiprof::Timer a {id_a};
   }
   init.stop(nIterations, "start-stop");

   if(rank==0)
      cout << "  2/3" <<endl;

   phiprof::Timer reinit {"Re-initialized timers using ID"};
   for(int i=0;i<nIterations;i++){
      id_a=phiprof::initializeTimer("a","A with ID");
      phiprof::Timer a {id_a};
   }
   reinit.stop(nIterations, "start-stop");

   if(rank==0)
      cout << "  3/3" <<endl;
   phiprof::Timer labels {"Timers using labels"};
   for(int i=0;i<nIterations;i++){
      phiprof::Timer a {"a"};
   }
   labels.stop(nIterations * 2, "start-stop"); // Why is it times two here?
   benchmark.stop();

   MPI_Barrier(MPI_COMM_WORLD);

   if(rank==0)
      cout << "Measuring accuracy" <<endl;

   phiprof::Timer accuracy {"Test accuracy"};

   if(rank==0)
      cout << "  1/3" <<endl;
   phiprof::Timer computations {"100x0.01s computations"}; 
   for(int i=0;i<100;i++){
      phiprof::Timer t {"compute"};
      compute(0.01);
   }
   computations.stop();

   if(rank==0)
      cout << "  2/3" <<endl;
   MPI_Barrier(MPI_COMM_WORLD);

   phiprof::Timer paraid {"100 x 0.01 (threadId + 1)s, ID"};
   int id = phiprof::initializeTimer("compute");
#pragma omp parallel
   for(int i=0;i<100;i++){
      phiprof::Timer t {id};
      compute(0.01 * (omp_get_thread_num() + 1));
   }
   paraid.stop();

   if(rank==0)
      cout << "  3/3" <<endl;
   MPI_Barrier(MPI_COMM_WORLD);

   phiprof::Timer parastring {"100 x 0.01 (threadId + 1)s, String"};
#pragma omp parallel
   for(int i=0;i<100;i++){
      phiprof::Timer t {"compute"};
      compute(0.01 * (omp_get_thread_num() + 1));
   }
   parastring.stop();



   accuracy.stop();

   if(rank%2 == 1) {
      phiprof::Timer groups {"Test-profile-groups"};
   }
   

   MPI_Barrier(MPI_COMM_WORLD);
   double t1=MPI_Wtime();
   phiprof::print(MPI_COMM_WORLD);
   if(rank==0)   
      cout<< "Print time is "<<MPI_Wtime()-t1<<endl;
//   phiprof::print(MPI_COMM_WORLD,0.1);
   MPI_Finalize();
}
