# source files.
SRC = assert_test.cpp
OBJ = $(SRC:.cpp=.o)

# include directories
INCLUDES = 

# C++ compiler flags (-g -O2 -Wall)
CCFLAGS = -O3 -I../../include 
LDFLAGS = -L../../lib -lphiprof -lgomp
# compiler
CCC = mpic++


.SUFFIXES: .cpp

default: $(OBJ) 
	$(CCC) -o assert_test  $(OBJ) $(LDFLAGS) 

.cpp.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) assert_test

