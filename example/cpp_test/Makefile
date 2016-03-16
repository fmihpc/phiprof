# source files.
SRC = test.cpp
OBJ = $(SRC:.cpp=.o)

# include directories
INCLUDES = 

# C++ compiler flags (-g -O2 -Wall)
CCFLAGS = -O3 -I../../include  -fopenmp
LDFLAGS = -L../../lib -lphiprof  -lgomp -ldl
# compiler
CCC = CC


.SUFFIXES: .cpp

default: $(OBJ) 
	$(CCC) -o test  $(OBJ) $(LDFLAGS)   

.cpp.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) test


