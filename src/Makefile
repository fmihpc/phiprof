# source files.
SRC = phiprof.cpp phiprof_c.cpp
SRC_NO = nophiprof.cpp phiprof_c.cpp
OBJ = $(SRC:.cpp=.o)
OBJ_NO = $(SRC_NO:.cpp=.o)
OUT = ../lib/libphiprof.a
OUT_NO = ../lib/libnophiprof.a

# include directories
INCLUDES = -I../include

#Set clock.
#CLOCK_MONOTONIC_COARSE is (much) faster, but has very poor
# resolution. Also only available on newer linux kernels see man
# clock_gettime
CLOCK_ID = CLOCK_MONOTONIC

# C++ compiler flags 
CCFLAGS = -O3 -fopenmp -DCLOCK_ID=$(CLOCK_ID) # -W -Wall -Wextra -pedantic 

# compiler
CCC = mpiCC

# library paths
LIBS = 

# compile flags
LDFLAGS = -O3 -lgomp

.SUFFIXES: .cpp

default: $(OUT) $(OUT_NO)

.cpp.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

$(OUT): $(OBJ) libdir
	ar rcs $(OUT) $(OBJ)

$(OUT_NO): $(OBJ_NO) libdir
	ar rcs $(OUT_NO) $(OBJ_NO)

libdir:
	mkdir -p ../lib

clean:
	rm -f $(OBJ) $(OUT) $(OBJ_NO) $(OUT_NO)

phiprof.o: ../include/phiprof.hpp

nophiprof.o: ../include/phiprof.hpp
