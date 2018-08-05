#include "Test.h"
#include "TestCase.h"
#include "TestCommon.h"
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
#include <atomic>
#include <thread>
using namespace std;

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
  }
 
  /* Child process handler. [[ TODO: This! ]] */
  [[ noreturn ]] void childProcessHandler(function<void ()> testCase, uint8_t xorKey, int pipeFD) {
    /* Evaluate the test case and see what we get back. */
    uint8_t result = static_cast<uint8_t>(evaluateTestCase(testCase)) ^ xorKey;
    
    /* Write this back through the pipe. */
    int status = write(pipeFD, &result, sizeof(result));
    if (status == -1) emergencyAbort("Failed to write data into pipe?");
    if (status ==  0) emergencyAbort("Tried to write a byte, but failed?");
    
    /* Otherwise, we're all set! */  
    exit(0);
  }
  
  /* Parent handler for test case. We will wait for a specified time period for the
   * child to succeed before killing it and considering things a failure.
   */
  const long kChildWaitTime = 5;
  Result parentProcessHandler(pid_t childPID, uint8_t xorKey, int pipeFD) {
    /* Use select() to wait until data arrives or some amount of time passes. */
    fd_set set;
    FD_ZERO(&set);
    FD_SET(pipeFD, &set);
    
    struct timeval timeout;
    timeout.tv_sec = kChildWaitTime;
    timeout.tv_usec = 0;
    
    /* We aren't expecting any signals, and so we won't run select() in a loop. Any signal
     * indicates that something weird has happened.
     */
    int selectStatus = select(pipeFD + 1, &set, nullptr, nullptr, &timeout);
    if (selectStatus == -1) emergencyAbort("select() failed.");
    
    /* If nothing is ready to read, we assume the process hasn't yet terminated and that
     * we need to shut it down.
     */
    Result result;
    if (selectStatus == 0) {
      kill(childPID, SIGKILL);
      result = Result::TIMEOUT;
    }
    /* Otherwise, the child has already terminated. See what we got back. */
    else {
      uint8_t data;
      int readStatus = read(pipeFD, &data, sizeof(data));
      if (readStatus == -1) {
        emergencyAbort("read() failed.");
      } 
      /* We may get back 0 bytes, meaning that no data was sent back. We interpret this to
       * be a crash, since normal program termination would have sent something.
       */
      else if (readStatus == 0) {
        result = Result::CRASH;
      }      
      /* Or, we did get a byte back. Convert it back to a result. */
      else {
        result = static_cast<Result>(data ^ xorKey);
        
        /* If there was an internal test case error, we need to panic. */
        if (result == Result::INTERNAL_ERROR) emergencyAbort("Internal error occurred in test.");
      }
    }
    
    /* Wait for the child to exit. */
    int childStatus;
    if (waitpid(childPID, &childStatus, 0) == -1) emergencyAbort("Failed to wait for child.");
     
    /* If we ended the test for an abnormal reason, report some diagnostic information. */
    if (result != Result::PASS && result != Result::FAIL && result != Result::EXCEPTION) {
      if (WIFEXITED(childStatus)) {
        cout << "  Child process exited abnormally with status code " << WEXITSTATUS(childStatus) << endl;
      } else if (WIFSIGNALED(childStatus)) {
        cout << "  Child process terminated by signal " << WTERMSIG(childStatus)
             << " (" << strsignal(WTERMSIG(childStatus)) << ")" << endl;
      } else {
        emergencyAbort("Child terminated for unknown reason.");
      }
    }
    
    /* Close our end of the pipe. */
    close(pipeFD);
    
    return result;
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
    
    /* Create a pipe. The child process will write the result back to the parent. */
    int pipes[2];
    if (pipe(pipes) == -1) emergencyAbort("Couldn't create child/parent pipe.");
    
    /* Spawn a subprocess to evaluate the function in isolation. This shields us in
     * case the test case leads to a crash.
     */
    auto pid = fork();
    if (pid == -1) emergencyAbort("fork() failed.");
    
    /* Child needs to do the actual work. */
    if (pid == 0) {
      close(pipes[0]);
      childProcessHandler(testCase, key, pipes[1]); // Never returns
    } else {
      close(pipes[1]);
      return parentProcessHandler(pid, key, pipes[0]);
    }
  }
}

shared_ptr<TestResult> TestCase::run() {
  /* Run the test and see how it went. */
  cout << "Running test: " << name() << endl;
  auto result = runTest(testCase);
  cout << "  Result: " << to_string(result) << endl;
  
  return make_shared<SingleTestResult>(result, pointsPossible(), name());
}

Points TestCase::pointsPossible() const {
  return numPoints;
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

shared_ptr<TestResult> TestGroup::run() {
  set<shared_ptr<TestResult>> children;
  Score score;
  
  /* Run each test, incorporating the information we find. */
  for (auto test: tests) {
    auto oneResult = test.second->run();
    
    children.insert(oneResult);
    
    /* Update the score. */
    score.earned   += oneResult->score().earned;
    score.possible += oneResult->score().possible;
  }
  
  /* If we have a hard point cap, scale the points to fit. */
  if (numPoints != kDetermineAutomatically) {
    if (score.possible != 0) {
      score.earned   = score.earned * numPoints / score.possible;
      score.possible = numPoints;
    } else {
      score.earned = score.possible = 0;
    }
  }
  
  if (isPublic()) {
    return make_shared<PublicTestGroupResult>(score, name(), children);
  } else {
    return make_shared<PrivateTestGroupResult>(score, name(), children);
  }
}

void TestGroup::setPublic(bool isPublic) {
  amIPublic = isPublic;
}

Points TestGroup::pointsPossible() const {
  /* If we have a fixed number of points, return that. */
  if (numPoints != kDetermineAutomatically) return numPoints;
  
  /* Otherwise, sum our children. */
  Points result = 0;
  for (const auto& test: tests) {
    result += test.second->pointsPossible();
  }
  return result;
}
