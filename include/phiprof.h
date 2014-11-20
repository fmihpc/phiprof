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

#ifndef PHIPROF_H
#define PHIPROF_H

/* This files contains the C interface, see phiprof.hpp (C++
 *  interface) for documentation of the various functions.
*/

#ifndef NDEBUG
#define phiprof_assert2(a,b) phiprof_phiprofAssert( a, b, __FILE__, __LINE__ )
#define phiprof_assert1(a) phiprof_phiprofAssert( a, #a, __FILE__, __LINE__ )
#else
#define phiprof_assert2(a,b)
#define phiprof_assert1(a) 
#endif
#define GET_MACRO(_1,_2,NAME,...) NAME
#define phiprof_assert(...) GET_MACRO(__VA_ARGS__, phiprof_assert2, phiprof_assert1)(__VA_ARGS__)


int phiprof_initializeTimer(char *label,int nGroups, ... );
int phiprof_getId(char *label);

int phiprof_start(char *label);
int phiprof_stop(char *label);
int phiprof_stopUnits(char *name,double units,char *unitName);
int phiprof_startId(int id);
int phiprof_stopId(int id);
int phiprof_stopIdUnits(int id,double units,char *unitName);

int phiprof_print(MPI_Comm comm, char *fileNamePrefix, double minFraction);
int phiprof_printLogProfile(MPI_Comm comm, double simulationTime, char *fileNamePrefix, char *separator, int maxLevel);

void phiprof_phiprofAssert(int condition, char* errorMessage, char* fileName, int line);

#endif
