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
#include "iostream"
#include "phiprof.hpp"

using namespace std;

int main(int argc,char **argv){

   int rank,nprocs;
   MPI_Init(&argc,&argv);
   MPI_Comm_rank(MPI_COMM_WORLD,&rank);
   MPI_Comm_size(MPI_COMM_WORLD,&nprocs);

   phiprof::start("Greeting");
   cout << "Hello world from rank " << rank << endl;
   phiprof::start("second");
   phiprof_assert(rank==0 && nprocs!=5,"5 tasks encountered using  phiprof_assert(a,b)");
   phiprof_assert(rank==0 && nprocs!=4);
   phiprof::phiprofAssert(rank==0 && nprocs!=3,"3 tasks for phiprof::phiprofAssert", __FILE__, __LINE__);

   phiprof::stop("second");
   phiprof::stop("Greeting", 1, "greetings");

   MPI_Barrier(MPI_COMM_WORLD);
   phiprof::print(MPI_COMM_WORLD);
   if (rank == 0) {
      cout << "Profiling results were printed into profile_*.txt" << endl;
   }

   MPI_Finalize();
   return 0;
}

