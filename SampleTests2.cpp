#include "TestCase.h"
#include <iostream>
#include <csignal>
#include <unistd.h>
using namespace std;

TEST_GROUP("Second batch of tests") {
  ADD_TEST("Mirth") {
    cout << "Mirth!" << endl;
  }
  
  ADD_TEST("Whimsy!") {
    cout << "Whimsy!" << endl;
  }
}
