# source files.
SRC = test.c
OBJ = $(SRC:.c=.o)

# include directories
INCLUDES = 

# C compiler flags (-g -O2 -Wall)
CFLAGS = -O3 -I../../include -fopenmp
LDFLAGS = -L../../lib -lphiprof  -lgomp -ldl
# compiler
CCC = cc
# linker (c++ compiler)
LNK = CC

.SUFFIXES: .c

default: $(OBJ) 
	$(LNK) -o test   $(OBJ)  $(LDFLAGS) 

.c.o:
	$(CCC) $(INCLUDES) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) test 


