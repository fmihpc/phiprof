# source files.
SRC = hello.F90
OBJ = $(SRC:.F90=.o)

# include directories
INCLUDES = 

# C compiler flags (-g -O2 -Wall)
FFLAGS = -O3 -I../../include 
LDFLAGS = -L../../lib -dynamic -lphiprof  -lgomp -ldl
# compiler
FC = ftn
# linker 
LNK = ftn

.SUFFIXES: .F90

default: $(OBJ) 
	$(LNK) -o hello   $(OBJ)  $(LDFLAGS) 

.F90.o:
	$(FC) $(INCLUDES) $(FFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) test 


