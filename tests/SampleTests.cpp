#include "TestCase.h"
#include <iostream>
#include <csignal>
#include <unistd.h>
using namespace std;

TEST_GROUP("All possible outcomes.") {
  MAKE_TESTS_PUBLIC();

  ADD_TEST("Abort") {
    abort();
  }

  ADD_TEST("Segfault") {
    raise(SIGSEGV);
  }

  ADD_TEST("Explicit success.") {
    passTest();
  }

  ADD_TEST("Explicit failure.") {
    failTest("Not your day, is it?");
  }

  ADD_TEST("Success by default.") {
    
  }

  ADD_TEST("Throw std::exception.") {
    throw runtime_error("Yeah, not feeling it.");
  }

  ADD_TEST("Throw something else.") {
    throw 137;
  }
  
  ADD_TEST("I'm a slowpoke!") {
    sleep(10);
  }
}
