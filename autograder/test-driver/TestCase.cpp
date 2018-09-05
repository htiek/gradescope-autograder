#include "TestCase.h"
using namespace std;

/* * * * * Implementation of unit testing primitives. * * * * */
namespace {
  [[ noreturn ]] void hardFailTest(const string& message, size_t line, const char* filename) {
    throw TestFailedException(message, line, filename);
  }
}

/* Implementation of the individual testing macros. */
void doPassTest() {
  throw TestSucceededException();
}

void doInternalError(const string& reason, size_t line, const char* filename) {
  throw InternalErrorException(reason, line, filename);
}

void doFailTest(const string& reason, size_t line, const char* filename) {
  hardFailTest(reason, line, filename);
}

void doFailTestVisibly(const string& message, size_t line, const char* filename) {
  throw TestFailedVisiblyException(message, line, filename);
}

void doExpect(bool condition, const char* expression, size_t line, const char* filename) {
  if (!condition) {
    hardFailTest(expression, line, filename);
  }
}

/* * * * * Exception types. * * * * */
TestFailedException::TestFailedException(const string& message, std::size_t line, const char*)
  : logic_error("Line " + to_string(line) + ": " + message) {

}
TestFailedVisiblyException::TestFailedVisiblyException(const string& message, std::size_t, const char*)
  : logic_error(message) {

}
InternalErrorException::InternalErrorException(const string& message, std::size_t line, const char*)
  : reason("Line " + to_string(line) + ": " + message) {

}
string InternalErrorException::what() const {
  return reason;
}


/* * * * * Test case assembly implementation * * * * */

/* Root testing group. This is a singleton that wraps a testing group. */
class RootGroup {
public:
  shared_ptr<TestGroup>    testGroup();
  vector<shared_ptr<Test>> allTests();
   
  static RootGroup& instance();
private:
  /* Singleton. */
  RootGroup() = default;
  RootGroup(const RootGroup&) = delete;
  RootGroup(RootGroup&&) = delete;
  void operator= (RootGroup) = delete;
    
  shared_ptr<TestGroup> tests = make_shared<TestGroup>("root");
};

RootGroup& RootGroup::instance() {
  static RootGroup theRootGroup;
  return theRootGroup;
}

shared_ptr<TestGroup> RootGroup::testGroup() {
  return tests;
}

vector<shared_ptr<Test>> RootGroup::allTests() {
  vector<shared_ptr<Test>> result;
  for (auto test: tests->tests) {
    result.push_back(test.second);
  }
  return result;
}

vector<shared_ptr<Test>> allTests() {
  return RootGroup::instance().allTests();
}

namespace {
  /* Utility function that walks down the scope chain and returns the resulting test group. */
  shared_ptr<TestGroup> groupFor(const vector<string>& scopeStack) {
    auto curr = RootGroup::instance().testGroup();
    for (size_t i = scopeStack.size(); i > 0; i--) {
      curr = static_pointer_cast<TestGroup>(curr->testNamed(scopeStack[i - 1]));
    }
    return curr;
  }
}

/* Root installation function just forwards to the root group. */
shared_ptr<Test> Root::installTest(vector<string> scopeStack, shared_ptr<Test> test) {
  groupFor(scopeStack)->addTest(test);
  return test;
}

/* * * * * Invoker Implementation * * * * */
Invoker::Invoker(function<void ()> function) {
  function();
}
