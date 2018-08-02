#include "TestResult.h"
#include "TestCommon.h"
#include <sstream>
using namespace std;

/* * * * * Result Implementation * * * * */
string to_string(Result r) {
  switch (r) {
  case Result::PASS:           return "test passed";
  case Result::FAIL:           return "test failed";
  case Result::EXCEPTION:      return "test triggered exception";
  case Result::CRASH:          return "test crashed";
  case Result::TIMEOUT:        return "test timed out";
  case Result::INTERNAL_ERROR: return "internal error (!!)";
  default: emergencyAbort("Unknown result type.");
  }
}

/* * * * * Base TestResult * * * * */
TestResult::TestResult(Score score, const string& name, size_t testsPassed, size_t numTests)
  : theScore(score), theName(name), theTestsPassed(testsPassed), theNumTests(numTests) {
  
}

Score TestResult::score() const {
  return theScore;
}

string TestResult::name() const {
  return theName;
}

/* Default string just reports how many tests were passed. */
string TestResult::displayText() const {
  ostringstream result;
  result << testsPassed() << " / " << numTests() << " Test"
         << (numTests() == 1? "" : "s")
         << " Passed.";
  return result.str();
}

size_t TestResult::testsPassed() const {
  return theTestsPassed;
}

size_t TestResult::numTests() const {
  return theNumTests;
}


/* * * * * SingleTestResult * * * * */

SingleTestResult::SingleTestResult(Result result, Points possible, const string& name)
  : TestResult({ (result == Result::PASS) * possible, possible }, name, result == Result::PASS, 1),
    result(result) {
}

/* We report failures by including ourself and our status if we didn't pass. */
set<string> SingleTestResult::reportFailedTests() const {
  if (result == Result::PASS) return { };
  return { name() + " (" + to_string(result) + ")" };
}

/* * * * * Public Test Results * * * * */

namespace {
  /* Utility function to add up the values of a specific set of member functions. */
  size_t countIn(const set<shared_ptr<TestResult>>& results,
                 size_t (TestResult::*function)() const) {
    size_t total = 0;
    for (auto result: results) {
      total += (result.get()->*function)();
    }
    return total;
  }
}

PublicTestGroupResult::PublicTestGroupResult(Score score, const std::string& name,
                                             const set<shared_ptr<TestResult>>& children)
  : TestResult(score, name, countIn(children, &TestResult::testsPassed),
                            countIn(children, &TestResult::numTests)), children(children) {
  
}

/* Display text reports all the failed tests we encountered. */
string PublicTestGroupResult::displayText() const {
  ostringstream result;
  result << TestResult::displayText() << "\\n";
  if (testsPassed() != numTests()) {
    result << "Tests that didn't pass:" << "\\n";
    
    for (auto message: reportFailedTests()) {
      result << "  " << message << "\\n";
    }
  }  
  return result.str();
}

/* We report failed tests by aggregating our children. */
set<string> PublicTestGroupResult::reportFailedTests() const {
  set<string> result;
  
  for (auto child: children) {
    for (const auto& entry: child->reportFailedTests()) {
      result.insert(entry);
    }
  }
  
  return result;
}


/* * * * * Private Test Results * * * * */
PrivateTestGroupResult::PrivateTestGroupResult(Score score, const std::string& name,
                                               const set<shared_ptr<TestResult>>& children)
  : TestResult(score, name, countIn(children, &TestResult::testsPassed),
                            countIn(children, &TestResult::numTests)), children(children) {
  
}

/* We report failed tests by seeing if any child failed and being very mysterious if so. */
set<string> PrivateTestGroupResult::reportFailedTests() const {
  if (testsPassed() == numTests()) return { };
  return { "(at least one private test case)" };
}