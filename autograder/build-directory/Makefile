# Recursively build everything here
CPP_FILES := $(shell find . -name '*.cpp')
OBJ_FILES := $(CPP_FILES:.cpp=.o)

# Crank up the speed!
CC_FLAGS := -g -O3 --std=c++17

all: $(OBJ_FILES)

%.o: %.cpp
	g++ -c $(CC_FLAGS) -o $@ -IStanfordHeaders $<

.PHONY: clean

clean:
	find . -name '*.o' -delete
