CPP_FILES := $(wildcard *.cpp)
OBJ_FILES := $(CPP_FILES:.cpp=.o)

CC_FLAGS := -g -O0 --std=c++17

all: $(OBJ_FILES)

%.o: %.cpp
	g++ -c $(CC_FLAGS) -o $@ $<

.PHONY: clean

clean:
	rm -f *.o *~
