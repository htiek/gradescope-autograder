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
  tuple<Result, string> evaluateTestCase(function<void ()> testCase) {
    try {
      testCase();
      return make_tuple(Result::PASS, "");
    } catch (const TestSucceededException &) {
      return make_tuple(Result::PASS, "");
    } catch (const TestFailedException& e) {
      cerr << "  Test failed: " << e.what() << endl;
      return make_tuple(Result::FAIL, "");
    } catch (const TestFailedVisiblyException& e) {
      cerr << "  Test failed visibly: " << e.what() << endl;
      return make_tuple(Result::VISIBLE_FAIL, e.what());
    } catch (const InternalErrorException& e) {
      cerr << "  INTERNAL TEST CASE FAILURE: " << e.what() << endl;
      return make_tuple(Result::INTERNAL_ERROR, "");
    } catch (const exception& e) {
      cerr << "  Exception: " << e.what() << endl;
      return make_tuple(Result::EXCEPTION, "");
    } catch (...) {
      cerr << "  Unknown exception generated." << endl;
      return make_tuple(Result::EXCEPTION, "");
    }
  }
 
  /* Child process handler. */
  [[ noreturn ]] void childProcessHandler(function<void ()> testCase, uint8_t xorKey, int pipeFD) {
    Result result;
    string message;
    
    /* Evaluate the test case and see what we get back. */
    tie(result, message) = evaluateTestCase(testCase);
  
    /* Encode the result with our XOR key. */
    char codedResult = static_cast<uint8_t>(result) ^ xorKey;
    
    /* Build the message to write back across the pipe. This is the status code
     * followed by a string message.
     */
    string pipeMessage = codedResult + message;
    
    /* Write this back across the pipe. */
    size_t index = 0;
    while (index != pipeMessage.size()) {
      auto written = write(pipeFD, pipeMessage.c_str(), pipeMessage.size() - index);
      if (written == -1) emergencyAbort("Couldn't write data across pipe.");
        
      index += written;
    }
    
    /* Terminate normally. We're done. */
    exit(0);
  }
  
  /* Given a file descriptor, reads the data from the child process from that descriptor,
   * returning what was read back.
   */
  const size_t kBufferSize = 1; // TODO: Make this bigger. This is just for testing.
  tuple<Result, string> readResult(int fd, uint8_t xorKey) {
    /* Build a string consisting of all the bytes we read back. */
    string data;
    
    while (true) {
      char buffer[kBufferSize];
      auto bytes = read(fd, buffer, kBufferSize);
      
      /* Handle errors and EOF. */
      if (bytes == -1) emergencyAbort("Error reading from child process.");
      if (bytes ==  0) break;
      
      data.append(buffer, buffer + bytes);
    }
    
    /* Decompose the data into the response code (byte 0) and everything else. The data
     * transmitted must be at least one byte for the response code. If we don't get that
     * back, we'll assume that the child crashed.
     */
    if (data.size() < 1) return make_tuple(Result::CRASH, "");
    
    Result result  = static_cast<Result>(static_cast<uint8_t>(data[0]) ^ xorKey);
    string message = data.substr(1);
    return make_tuple(result, message);
  }
  
  /* Parent handler for test case. We will wait for a specified time period for the
   * child to succeed before killing it and considering things a failure.
   */
  const long kChildWaitTime = 60; // One minute
  tuple<Result, string> parentProcessHandler(pid_t childPID, uint8_t xorKey, int pipeFD) {
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
    string message;
    if (selectStatus == 0) {
      kill(childPID, SIGKILL);
      result = Result::TIMEOUT;
    }
    /* Otherwise, the child has already terminated. See what we got back. */
    else {
      tie(result, message) = readResult(pipeFD, xorKey);

      /* If there was an internal test case error, we need to panic. */
      if (result == Result::INTERNAL_ERROR) emergencyAbort("Internal error occurred in test.");
    }
    
    /* Wait for the child to exit. */
    int childStatus;
    if (waitpid(childPID, &childStatus, 0) == -1) emergencyAbort("Failed to wait for child.");
     
    /* If we ended the test for an abnormal reason, report some diagnostic information. */
    if (result != Result::PASS &&
        result != Result::FAIL &&
        result != Result::VISIBLE_FAIL &&
        result != Result::EXCEPTION) {
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
    
    return make_tuple(result, message);
  }
  
  /* Returns a random byte. */
  uint8_t randomByte() {
    random_device rd;
    mt19937 generator(rd());
    return uniform_int_distribution<uint8_t>()(generator);
  }

  /* Helper function to run a test and report how it goes. */
  tuple<Result, string> runTest(function<void ()> testCase) { 
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

shared_ptr<TestResult> TestCase::run(const std::set<std::string> & /* unused */) {
  /* Run the test and see how it went. */
  cout << "Running test: " << name() << endl;
  
  Result result;
  string message;
  tie(result, message) = runTest(testCase);
  cout << "  Result: " << to_string(result) << endl;
  
  return make_shared<SingleTestResult>(result, message, pointsPossible(), name());
}

Points TestCase::pointsPossible() const {
  return numPoints;
}

size_t TestCase::numTests() const {
  return 1;
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

shared_ptr<TestResult> TestGroup::run(const std::set<std::string>& missingFiles) {
  /* Edge case: if not all needed files were submitted, report an error. */
  if (any_of(requirements.begin(), requirements.end(), [&](auto req) { return missingFiles.count(req); })) {
    return make_shared<MissingFileTestResult>(pointsPossible(), name());
  }

  /* Otherwise, all files are submitted. Run the tests. */
  set<shared_ptr<TestResult>> children;
  Score score;
  
  /* Run each test, incorporating the information we find. */
  for (auto test: tests) {
    auto oneResult = test.second->run(missingFiles);
    
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

set<string> TestGroup::requiredFiles() const {
  return requirements;
}

void TestGroup::addRequirement(const string& filename) {
  requirements.insert(filename);
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

size_t TestGroup::numTests() const {
  size_t result = 0;
  for (auto test: tests) {
    result += test.second->numTests(); 
  }
  return result;
}
