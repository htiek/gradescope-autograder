#include "TestCase.h"
#include "Simple.h"
#include <iostream>
using namespace std;

TEST_GROUP("Second batch of tests") {
  MAKE_TESTS_PUBLIC();
  ADD_TEST("Mirth") {
    cout << "Mirth!" << endl;
  }
  
  ADD_TEST("Sadness") {
    failTest(":-(");
  }
  
  
  TEST_GROUP("Nested private group.") {
    ADD_TEST("Whimsy!") {
      cout << "Whimsy!" << endl;
    }
    
    ADD_TEST("Sorrow") {
      failTest("D-:");
    }
    
    ADD_TEST("Sorrow 2") {
      failTest("D-:");
    }
  }
}

ADD_TEST("Solo Test") {
  expect(doSomething());
}
