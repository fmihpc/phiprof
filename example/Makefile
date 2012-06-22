# source files.
SRC = hello_world.cpp
OBJ = $(SRC:.cpp=.o)

# include directories
INCLUDES = 

# C++ compiler flags (-g -O2 -Wall)
CCFLAGS = -O3 -I../include 
LDFLAGS = -L../lib -lphiprof
# compiler
CCC = mpiCC


.SUFFIXES: .cpp

default: $(OBJ) 
	$(CCC) -o hello_world  $(OBJ) $(LDFLAGS) 

.cpp.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) hello_world profile_*txt

