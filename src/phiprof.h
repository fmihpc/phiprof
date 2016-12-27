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

#ifndef PHIPROF_H
#define PHIPROF_H

/* This files contains the C interface, see phiprof.hpp (C++
 *  interface) for documentation of the various functions.
*/


int phiprof_initialize();

int phiprof_initializeTimer(char *label);
int phiprof_initializeTimerWithGroups(char *label, int nGroups, char **groups);
int phiprof_initializeTimerWithGroups1(char *label, char *group1);
int phiprof_initializeTimerWithGroups2(char *label, char *group1, char *group2);
int phiprof_initializeTimerWithGroups3(char *label, char *group1, char *group2, char *group3);


int phiprof_getChildId(char *label);

int phiprof_start(char *label);
int phiprof_stop(char *label);
int phiprof_stopUnits(char *name,double units,char *unitName);

int phiprof_startId(int id);
int phiprof_stopId(int id);
int phiprof_stopIdUnits(int id,double units,char *unitName);

int phiprof_print(MPI_Comm comm, char *fileNamePrefix);


#endif
