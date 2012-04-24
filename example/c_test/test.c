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
#include "phiprof.h"
#include <stdio.h>
#include <stdlib.h>

double compute(double seconds){
   double overhead=0.000002; //estimated
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

   int rank,i;
   MPI_Init(&argc,&argv);
   MPI_Comm_rank(MPI_COMM_WORLD,&rank);
   int nIterations=1000000;

   if(rank==0)
      printf( "Measuring performance of start-stop calls\n");

   
   phiprof_start("Benchmarking phiprof"); 
   phiprof_start("Initalized timers using ID");

   if(rank==0)
      printf( "  1/3\n" );

   int id_a=phiprof_initializeTimer("a",1,"A with ID");
   for(i=0;i<nIterations;i++){
      phiprof_startId(id_a);
      phiprof_stopId(id_a);
   }
   phiprof_stopUnits("Initalized timers using ID",nIterations,"start-stop");

   if(rank==0)
      printf( "  2/3\n" );
   phiprof_start("Re-initialized timers using ID");
   for(i=0;i<nIterations;i++){
     int id_a=phiprof_initializeTimer("a",1,"A with ID");
     phiprof_startId(id_a);
     phiprof_stopId(id_a);
   }
   phiprof_stopUnits("Re-initialized timers using ID",nIterations,"start-stop");

   if(rank==0)
      printf( "  3/3\n" );
   phiprof_start("Timers using labels");
   for(i=0;i<nIterations;i++){
      phiprof_start("a");
      phiprof_stop("a");
   }
   phiprof_stopUnits("Timers using labels",nIterations*2,"start-stop");
   phiprof_stop("Benchmarking phiprof"); 

   MPI_Barrier(MPI_COMM_WORLD);

   if(rank==0)
      printf( "Measuring accuracy\n" );
   phiprof_start("Test accuracy");
   if(rank==0)
      printf( "  1/2\n");
   phiprof_start("100 computations"); 
   for(i=0;i<100;i++){
      phiprof_start("compute");
      compute(0.1);
      phiprof_stop("compute");
   }
   phiprof_stop("100 computations");

   if(rank==0)
      printf( "  2/2\n" );
   MPI_Barrier(MPI_COMM_WORLD);
   phiprof_start("100 computations + logprofile"); 
   for(i=0;i<100;i++){
      phiprof_start("compute");
      compute(0.1);
      phiprof_printLogProfile(MPI_COMM_WORLD,i,"profile_log_alllev"," ",0);
      phiprof_printLogProfile(MPI_COMM_WORLD,i,"profile_log_maxlev1"," ",1);
      phiprof_stop("compute");
   }
   phiprof_stop("100 computations + logprofile"); 

   phiprof_stop("Test accuracy");
   
   MPI_Barrier(MPI_COMM_WORLD);
   double t1=MPI_Wtime();
   phiprof_print(MPI_COMM_WORLD,"profile_full",0.0);
   phiprof_print(MPI_COMM_WORLD,"profile_minfrac0.01",0.01);
   if(rank==0)   
      printf( "Print time is %g\n",MPI_Wtime()-t1);


   MPI_Finalize();
}
