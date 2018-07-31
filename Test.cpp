#include "Test.h"
#include "TestCase.h"
#include "Common.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <sstream>
#include <cstdint>
#include <random>
#include <future>
#include <chrono>
#include <cstring>
using namespace std;

/* * * * * Result Implementation * * * * */
string to_string(Result r) {
  switch (r) {
  case Result::PASS:           return "pass";
  case Result::FAIL:           return "fail";
  case Result::EXCEPTION:      return "exception";
  case Result::CRASH:          return "crash";
  case Result::TIMEOUT:        return "timeout";
  case Result::INTERNAL_ERROR: return "internal error (!!)";
  default: emergencyAbort("Unknown result type.");
  }
}

/* * * * * Score Implementation * * * * */
ostream& operator<< (ostream& out, const Score& score) {
  ostringstream result;
  result << score.earned << " / " << score.possible;
  return out << result.str();
}

/* * * * * Test Implementation * * * * */
Test::Test(const string& name) : theName(name) {

}

string Test::name() const {
  return theName;
}

/* * * * * TestCase Implementation * * * * */
TestCase::TestCase(const string& name,
                   function<void ()> theTest,
                   Points numPoints)
: Test(name), testCase(theTest), numPoints(numPoints) {
  if (numPoints == kDetermineAutomatically) {
    emergencyAbort("Cannot determine number of points in a test case automatically.");
  }
}

namespace {
  /* Helper function that, given a function, evaluates that function and returns a
   * status code based on how it went.
   */
  Result evaluateTestCase(function<void ()> testCase) {
    /* Set up a future that runs the test and tells us how it went. This future is
     * introduced so that timeouts don't cause us to hang indefinitely.
     *
     * One issue with std::future is that its destructor blocks until the calling thread
     * has finished. That's a problem for us, because if this function returns before
     * the test case has finished, we'll block until it's done, defeating the entire
     * purpose of using futures.
     *
     * To get around this, we'll just leak a bunch of memory and move the future to a
     * dynamically-allocated future object. We're going to terminate the program anyway
     * once we return from this function, so realistically that's not actually an issue.
     */
    const auto kMaxWaitTime = chrono::seconds(5);
    auto* result = new future<Result>(async(launch::async, [testCase] {
      try {
        testCase();
        return Result::PASS;
      } catch (const TestSucceededException &) {
        return Result::PASS;
      } catch (const TestFailedException& e) {
        cerr << "  Test failed: " << e.what() << endl;
        return Result::FAIL;
      } catch (const InternalErrorException& e) {
        cerr << "  INTERNAL TEST CASE FAILURE: " << e.what() << endl;
        return Result::INTERNAL_ERROR;
      } catch (const exception& e) {
        cerr << "  Exception: " << e.what() << endl;
        return Result::EXCEPTION;
      } catch (...) {
        cerr << "  Unknown exception generated." << endl;
        return Result::EXCEPTION;
      }
    }));
    
    auto status = result->wait_for(kMaxWaitTime);
    if (status == future_status::ready) {
      return result->get();
    } else if (status == future_status::timeout) {
      return Result::TIMEOUT;
    } else if (status == future_status::deferred) {
      cerr << "  INTERNAL TEST DRIVER FAILURE: Future was deferred?" << endl;
      return Result::INTERNAL_ERROR;
    } else {
      cerr << "  INTERNAL TEST DRIVER FAILURE: Unknown status code?" << endl;
      return Result::INTERNAL_ERROR;
    }
  }
  
  /* Returns a random byte. */
  uint8_t randomByte() {
    random_device rd;
    mt19937 generator(rd());
    return uniform_int_distribution<uint8_t>()(generator);
  }

  /* Helper function to run a test and report how it goes. */
  Result runTest(function<void ()> testCase) { 
    /* Just to guard against someone trying to guess what status code to return,
     * we'll introduce a random one-byte XOR mask.
     */
    uint8_t key = randomByte();
  
    /* Spawn a subprocess to evaluate the function in isolation. This shields us in
     * case the test case leads to a crash.
     */
    auto pid = fork();
    if (pid == -1) emergencyAbort("fork() failed.");
    
    /* Child needs to do the actual work. */
    if (pid == 0) {
      /* Do a quick exit - we don't care about reclaiming resources from other threads. */
      quick_exit(key ^ static_cast<int>(evaluateTestCase(testCase)));
    }
    
    /* Parent needs to wait for a result. */
    int status;
    if (waitpid(pid, &status, 0) == -1) emergencyAbort("Failed to wait for child.");
    
    if (WIFEXITED(status)) {
      auto result = static_cast<Result>(key ^ WEXITSTATUS(status));
      if (result == Result::INTERNAL_ERROR) emergencyAbort("Test failed due to internal error.");
      
      return result;
    } else if (WIFSIGNALED(status)) {
      auto signal = WTERMSIG(status);
      cout << "  Child process crashed with signal " << signal << " (" << strsignal(signal) << ")" << endl;
      return Result::CRASH;
    } else {
      cout << "  Child process crashed for an unknown reason?" << endl;
      return Result::CRASH;
    }
  }
}

TestResults TestCase::run() {
  /* Run the test and see how it went. */
  cout << "Running test: " << name() << endl;
  auto result = runTest(testCase);
  cout << "  Result: " << to_string(result) << endl;
  
  return {
    { { name(), result } },                               // We did however we did.
    { result == Result::PASS? numPoints : 0, numPoints }, // Only award points for a success.
    false                                                 // Individual tests are considered private
  };
}

/* * * * * TestGroup Implementation * * * * */

TestGroup::TestGroup(const string& name, Points numPoints)
  : Test(name), numPoints(numPoints) {
  
}

void TestGroup::addTest(shared_ptr<Test> test) {
  /* Add the test by name. */
  if (tests.count(test->name())) emergencyAbort("Duplicate test case: " + test->name());
  tests[test->name()] = test;
}

bool TestGroup::isPublic() const {
  return amIPublic;
}

shared_ptr<Test> TestGroup::testNamed(const string& name) const {
  return tests.at(name);
}

TestResults TestGroup::run() {
  TestResults results;
  
  /* Run each test, incorporating the information we find. */
  for (auto test: tests) {
    auto oneResult = test.second->run();
    
    /* Copy over all the individual test results. */
    for (auto oneTest: oneResult.individualResults) {
      results.individualResults[name() + "/" + oneTest.first] = oneTest.second;
    }
    
    /* Update the score. */
    results.score.earned   += oneResult.score.earned;
    results.score.possible += oneResult.score.possible;
  }
  
  /* Set whether this test is public or private based on whether we're public or private. */
  results.isPublic = isPublic();
  
  /* If we have a hard point cap, scale the points to fit. */
  if (numPoints != kDetermineAutomatically) {
    results.score.earned   = results.score.earned * numPoints / results.score.possible;
    results.score.possible = numPoints;
  }
  
  return results;
}

void TestGroup::setPublic(bool isPublic) {
  amIPublic = isPublic;
}
