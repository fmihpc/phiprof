/*
This file is part of the phiprof library

Copyright 2010, 2011, 2012 Finnish Meteorological Institute

Vlasiator is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3
as published by the Free Software Foundation.

Vlasiator is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PHIPROF_H
#define PHIPROF_H

/* This files contains the C interface, see phiprof.hpp (C++
 *  interface) for documentation of the various functions.
*/

#ifndef NDEBUG
#define pp_assert(a,b) phiprof_assert( a, b, __FILE__, __LINE__ )
#else
#define pp_assert(a,b) 
#endif

int phiprof_initializeTimer(char *label,int nGroups, ... );
int phiprof_getId(char *label);

int phiprof_start(char *label);
int phiprof_stop(char *label);
int phiprof_stopUnits(char *name,double units,char *unitName);
int phiprof_startId(int id);
int phiprof_stopId(int id);
int phiprof_stopIdUnits(int id,double units,char *unitName);

int phiprof_print(MPI_Comm comm,char *fileNamePrefix,double minFraction);
int phiprof_printLogProfile(MPI_Comm comm,double simulationTime,char *fileNamePrefix,char *separator,int maxLevel);

int phiprof_assert(bool condition, char * error_message, char * file, int line );

#endif

