# source files.
SRC = prettyprinttable.cpp timerdata.cpp timertree.cpp paralleltimertree.cpp  phiprof.cpp phiprof_c.cpp 
SRC_NO = nophiprof.cpp phiprof_c.cpp
OBJ = $(SRC:.cpp=.o) 
FOBJ = phiprof_fortran.o
OBJ_NO = $(SRC_NO:.cpp=.o) 
FOBJ_NO = phiprof_fortran.o
OUT_STATIC = ../lib/libphiprof.a
OUT_STATIC_NO = ../lib/libnophiprof.a
OUT_SHARED = ../lib/libphiprof.so
OUT_SHARED_NO = ../lib/libnophiprof.so



#Set clock.
#CLOCK_MONOTONIC_COARSE is (much) faster, but has very poor
# resolution. Also only available on newer linux kernels see man
# clock_gettime
CLOCK_ID = CLOCK_MONOTONIC

# C++ compiler flags 
CCFLAGS = -fpic -O2 -fopenmp -std=c++11 -DCLOCK_ID=$(CLOCK_ID) #-DDEBUG_PHIPROF_TIMERS # -W -Wall -Wextra -pedantic 
FFLAGS= -fpic -O2 
# compiler
CCC = mpicxx
FTN = mpifort

# compile flags
LDFLAGS =  -lgomp -lstdc++ 

.SUFFIXES: .cpp


default: all

all: $(OUT_STATIC) $(OUT_STATIC_NO) $(OUT_SHARED)  $(OUT_SHARED_NO) includedir

all-w-fortran:  $(OUT_STATIC) $(OUT_STATIC_NO) $(OUT_SHARED)  $(OUT_SHARED_NO)  includedir-w-fortran fortran


static:  $(OUT_STATIC) $(OUT_STATIC_NO) includedir

shared: $(OUT_SHARED)  $(OUT_SHARED_NO) includedir

static-w-fortran:  $(OUT_STATIC) $(OUT_STATIC_NO)  includedir-w-fortran fortran

shared-w-fortran: $(OUT_SHARED)  $(OUT_SHARED_NO)  includedir-w-fortra fortran


.cpp.o:
	$(CCC) $(CCFLAGS) -c $< -o $@

phiprof.mod: phiprof.f90 phiprof_fortran.o 

phiprof_fortran.o: phiprof.f90 
	$(FTN) $(FFLAGS) -c phiprof.f90 -o phiprof_fortran.o

$(OUT_STATIC): $(OBJ) libdir
	ar rcs $(OUT_STATIC) $(OBJ)

$(OUT_STATIC_NO): $(OBJ_NO) libdir 
	ar rcs $(OUT_STATIC_NO) $(OBJ_NO)


$(OUT_SHARED): $(OBJ) libdir
	$(CCC) -shared $(OBJ) -o $(OUT_SHARED) $(LDFLAGS)

$(OUT_SHARED_NO): $(OBJ_NO) libdir
	$(CCC) -shared $(OBJ_NO) -o $(OUT_SHARED_NO) $(LDFLAGS)

libdir:
	mkdir -p ../lib

fortran: includedir-w-fortran  phiprof.mod $(FOBJ) $(FOBJ_NO)

includedir-w-fortran: includedir
	cp  phiprof.mod ../include

includedir: 
	mkdir -p ../include
	cp phiprof.hpp phiprof.h  ../include



clean:
	rm -f $(OBJ) $(FOBJ) *.mod $(OUT_STATIC) $(OBJ_NO) $(FOBJ_NO) $(OUT_STATIC_NO) $(OUT_SHARED) $(OUT_SHARED_NO) ../include/* 

phiprof.o: phiprof.hpp


nophiprof.o: phiprof.hpp
