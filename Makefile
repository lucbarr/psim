MAIN = psim.out
CXX = g++
CXXFLAGS = -std=c++11
SRC := $(shell find . -name '*.cpp')

all: $(SRC)
				@echo Building...
				@$(CXX) $(CXXFLAGS) $(SRC) -o $(MAIN)
				@echo Done!!
clean:
				rm -f $(MAIN) *.o
