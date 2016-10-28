Phiprof – Parallel Hierarchical Profiler
====

Copyright (c) 2011 - 2015 Finnish Meteorological Institute
Copyright (c) 2016   CSC - IT Center for Science

Licence: LGPL 3

Author: Sebastian von Alfthan (sebastian.von.alfthan at csc.fi)


Phiprof a simple library that can be used to profile parallel MPI
programs. It can be used to produce a hierarchical report of time
(average, max, min) spent in different timer regions. A log format can
also be written, where the performance as a function of time may be
reported.

Features
  * Supports C++ and C 
  * Low overhead (Less than 1 us)
  * Can print the total time as a human-readable hierarchical report
  * Automatically handles cases where groups of processes execute different codepaths

Not supported
  * No Fortran interface
  * Phiprof does not automatically add regions, these have to be added manually into the code.


## Installation

1) Enter src/ folder

2) Edit Makefile if needed to set the correct compilation options and
compiler names. Phiprof should be compiled with a MPI compiler and
openmp threading should be enabled

3) make 

4) The library files are after compilation in lib/ and headers in
include/ . These can manually be moved to sensible locations, or one
can add the correct -I and -L flags to the compiler commands. For
shared library you may also need to add the path to LD_LIBRARY_PATH



## Usage

### Adding timers 

The C++ interface is descibed in the header file
[phiprof.hpp](include/phiprof.hpp). To understand how to use it, it
can also be instructive to look at the [examples](example/), in
particular [hello world](example/hello_world/hello_world.cpp) which
has more extensive comments.

Note that when adding `phiprof::start`s to OpenMP threaded parts of
the code, one should only use the variant which uses an integer to
define the region, that has been obtained by a preceeding
`phiprof::initializeTimer(...)` call, since that is the only one that
does not include a critical region synchronization.


### Running code 

While running the code the phiprof reports are written out whenever
the `phiprof::print()` function is called. On file per unique set of
timers is printed, try to avoid unneccessary divergence to keep the
number limited. This function can be called at any time, multiple
times, and all timers do not need to be closed.


 What is printed out is steered with an environment variable
`PHIPROF_PRINTS`. It accepts a comma separated string with the
following options:

 * `groups` Prints out statistics for groups (see `phiprof::initializeTimer(...)` functions.
 * `compact` Prints out timer statistics for all timers where more that 1% of time was spent
 * `full`  Prints out all timers
 * `detailed` Prints out all timers in a alternative format with even more info on MPI (phiprof-1 style).

Default is `groups,compact`.

