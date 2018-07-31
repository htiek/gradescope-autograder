CPP_FILES := $(wildcard *.cpp)
OBJ_FILES := $(CPP_FILES:.cpp=.o)

CC_FLAGS := -g -O0 -Wall -Werror -Wpedantic --std=c++17
LD_FLAGS := -pthread

# Group ID is used to assign each set of test cases globally unique names for their namespaces,
# ensuring that we don't get collisions.
GROUP_ID = 0

all: run-tests

run-tests: $(OBJ_FILES)
	g++-6 $(LD_FLAGS) -o $@ $^

%.o: %.cpp
# Build with GROUP subbed out for the current group ID.
	g++-6 -c $(CC_FLAGS) -DGROUP=GroupID_$(GROUP_ID) -o $@ $<
	
# Increment the group ID so that each set of tests gets its own group ID.
# Thanks to https://stackoverflow.com/questions/34142638/ for this big of sorcery.
	$(eval GROUP_ID=$(shell echo $$(($(GROUP_ID)+1))))

.PHONY: clean

clean:
	rm -f *.o run-tests *~
