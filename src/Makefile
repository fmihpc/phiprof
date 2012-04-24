# source files.
SRC = phiprof.cpp phiprof_c.cpp
SRC_NO = nophiprof.cpp phiprof_c.cpp
OBJ = $(SRC:.cpp=.o)
OBJ_NO = $(SRC_NO:.cpp=.o)
OUT = ../lib/libphiprof.a
OUT_NO = ../lib/libnophiprof.a

# include directories
INCLUDES = -I../include

# C++ compiler flags 
CCFLAGS = -O3 # -W -Wall -Wextra -pedantic 

# compiler
CCC = mpiCC

# library paths
LIBS = 

# compile flags
LDFLAGS = -O3

.SUFFIXES: .cpp

default: $(OUT) $(OUT_NO)

.cpp.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)

$(OUT_NO): $(OBJ_NO)
	ar rcs $(OUT_NO) $(OBJ_NO)

clean:
	rm -f $(OBJ) $(OUT) $(OBJ_NO) $(OUT_NO)

phiprof.o: ../include/phiprof.hpp

nophiprof.o: ../include/phiprof.hpp
