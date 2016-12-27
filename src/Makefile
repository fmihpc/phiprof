# source files.
SRC = prettyprinttable.cpp timerdata.cpp timertree.cpp paralleltimertree.cpp  phiprof.cpp phiprof_c.cpp 
SRC_NO = nophiprof.cpp phiprof_c.cpp
OBJ = $(SRC:.cpp=.o) 
OBJ_NO = $(SRC_NO:.cpp=.o) 
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
CCFLAGS = -fpic -O3 -fopenmp -std=c++11 -DCLOCK_ID=$(CLOCK_ID) #-DDEBUG_PHIPROF_TIMERS # -W -Wall -Wextra -pedantic 

# compiler
CCC = CC
FTN = ftn

# compile flags
LDFLAGS =  -lgomp -lstdc++ 

.SUFFIXES: .cpp


default: $(OUT_STATIC) $(OUT_STATIC_NO) $(OUT_SHARED)  $(OUT_SHARED_NO) phiprof.mod

static:  $(OUT_STATIC) $(OUT_STATIC_NO) phiprof.mod

shared: $(OUT_SHARED)  $(OUT_SHARED_NO) phiprof.mod


.cpp.o:
	$(CCC) $(CCFLAGS) -c $< -o $@

phiprof.mod: phiprof.F08
	$(FTN)  -c phiprof.F08

$(OUT_STATIC): $(OBJ) libdir includedir
	ar rcs $(OUT_STATIC) $(OBJ)

$(OUT_STATIC_NO): $(OBJ_NO) libdir includedir
	ar rcs $(OUT_STATIC_NO) $(OBJ_NO)


$(OUT_SHARED): $(OBJ) libdir
	$(CCC) -shared $(OBJ) -o $(OUT_SHARED) $(LDFLAGS)

$(OUT_SHARED_NO): $(OBJ_NO) libdir
	$(CCC) -shared $(OBJ_NO) -o $(OUT_SHARED_NO) $(LDFLAGS)

libdir:
	mkdir -p ../lib
includedir:
	mkdir -p ../include
	cp phiprof.hpp phiprof.h ../include

clean:
	rm -f $(OBJ) *.mod $(OUT_STATIC) $(OBJ_NO) $(OUT_STATIC_NO) $(OUT_SHARED) $(OUT_SHARED_NO)

phiprof.o: phiprof.hpp


nophiprof.o: phiprof.hpp
