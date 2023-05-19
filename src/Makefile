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

# Set the default compiler type (pgi, nvcc, gcc, intel, clang). Can be overriden from command-line.
CC = gcc

# compiler calls
CCC = mpicxx
FTN = mpifort
#CCC = mpic++
#FTN = mpif90

#Set clock.
#CLOCK_MONOTONIC_COARSE is (much) faster, but has very poor
# resolution. Also only available on newer linux kernels see man
# clock_gettime
CLOCK_ID = CLOCK_MONOTONIC

# C++ compiler flags 
CCFLAGS = -fpic -O2 -std=c++17 -DCLOCK_ID=$(CLOCK_ID)
FFLAGS= -fpic -O2

LDFLAGS = -lstdc++ 

# Compiler-specific options
ifeq ($(CC),pgi)
   # Add openmp, NVTX profiling, warnings
   CCFLAGS += -mp -D_NVTX --remarks -Minform=inform # -a --pedantic give even more warnings
   LDFLAGS += -mp
else ifeq ($(CC),nvcc)
   # needs extra --compiler-options
   CCFLAGS = --compiler-options -fpic -O2 -std=c++17 -DCLOCK_ID=$(CLOCK_ID)
   CCFLAGS +=  -D_NVTX -Xcompiler -fopenmp
   LDFLAGS += -lgomp -lnvToolsExt
   CCC = OMPI_CXX='nvcc' OMPI_CXXFLAGS='' mpic++
else ifeq ($(CC),gcc)
   CCFLAGS += -fopenmp -W -Wall -Wextra -pedantic 
   LDFLAGS += -lgomp
else ifeq ($(CC),intel)
   CCFLAGS += -qopenmp -W -Wall -Wextra -pedantic 
   LDFLAGS += -qopenmp
else ifeq ($(CC),clang)
   CCFLAGS += -fopenmp -W -Wall -Wextra -pedantic -mp -D_NVTX --remarks -Minform=inform # -a --pedantic give even more warnings
   LDFLAGS += -lgomp -mp
endif

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
