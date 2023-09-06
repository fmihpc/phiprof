# source files.
SRC = test.cpp
OBJ = $(SRC:.cpp=.o)

# include directories
INCLUDES = 

# add compiler flag -D_NVTX to include NVTX range analysis in phiprof
# test includes small openacc region so compiler is given -acc and -Minfo=accel

# Run test with e.g.
# srun -n1 -p gputest --gres=gpu:v100:1 --account=project_2000203 --time=00:02:30 nvprof -f -o out.nvvp ./test

# C++ compiler flags (-g -O2 -Wall)
CCFLAGS = -O3 -I../../include -acc -Minfo=accel -D_NVTX
LDFLAGS = -L../../lib -lphiprof -acc -ldl -lrt -lnvToolsExt -L/appl/spack/install-tree/gcc-4.8.5/cuda-10.1.168-v5izax/lib64
# compiler
CCC = mpic++

#PGI openmp support
CCFLAGS += -mp
LDFLAGS += -mp

.SUFFIXES: .cpp

default: $(OBJ) 
	$(CCC) -o test  $(OBJ) $(LDFLAGS)   

.cpp.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) test


