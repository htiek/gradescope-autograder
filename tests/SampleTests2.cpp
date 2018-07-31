#include "TestCase.h"
#include "Simple.h"
#include <iostream>
using namespace std;

TEST_GROUP("Second batch of tests") {
  ADD_TEST("Mirth") {
    cout << "Mirth!" << endl;
  }
  
  ADD_TEST("Whimsy!") {
    cout << "Whimsy!" << endl;
  }
}

ADD_TEST("Solo Test") {
  expect(doSomething());
}
