#include "TestResult.h"
#include "TestCommon.h"
#include <sstream>
using namespace std;

/* * * * * Result Implementation * * * * */
string to_string(Result r) {
  switch (r) {
  case Result::PASS:           return "test passed";
  case Result::FAIL:           return "test failed";
  case Result::VISIBLE_FAIL:   return "test failed with message";
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

SingleTestResult::SingleTestResult(Result result, const std::string& message,
                                   Points possible, const string& name)
  : TestResult({ (result == Result::PASS) * possible, possible }, name, result == Result::PASS, 1),
    result(result), message(message) {
}

/* Our display text is the default, plus a status message. */
string SingleTestResult::displayText() const {
  ostringstream builder;
  builder << TestResult::displayText();
  
  /* If we didn't pass the test, explain why. */
  if (result != Result::PASS) {
    builder << "\n  (" << humanReadableMessage() << ")";
  }
  
  return builder.str();
}

/* We report failures by including ourself and our status if we didn't pass. */
set<string> SingleTestResult::reportFailedTests() const {
  if (result == Result::PASS) {
    return { };
  }
  return { name() + " (" + humanReadableMessage() + ")" };
}

/* Human-readable version of our status. */
string SingleTestResult::humanReadableMessage() const {
  if (result == Result::VISIBLE_FAIL) return message;
  else return to_string(result);
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
  result << TestResult::displayText() << endl;
  if (testsPassed() != numTests()) {
    result << "Tests that didn't pass:" << endl;
    
    for (auto message: reportFailedTests()) {
      result << "  " << message << endl;
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



/* * * * * Missing File Test Results * * * * */
MissingFileTestResult::MissingFileTestResult(Points pointsPossible, const std::string& name) 
  : TestResult({ 0, pointsPossible }, name, 0, 0) {

}

set<string> MissingFileTestResult::reportFailedTests() const {
  return { "(tests not run; not all needed files submitted)" };
}

/* Display text reports all the failed tests we encountered. */
string MissingFileTestResult::displayText() const {
  return "Tests not run; not all necessary files were submitted.";
}
