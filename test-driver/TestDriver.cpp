#include "Test.h"
#include "TestCommon.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
using namespace std;

namespace {
  /* Runs all the root tests, returning the results. */
  vector<TestResults> runAllTests() {
    vector<TestResults> results;
    for (auto test: allTests()) {
      results.push_back(test->run());
    }
    return results;
  }
  
  /* Totals the number of points earned through a set of test results. */
  Points totalPointsFrom(const vector<TestResults>& results) {
    Points earned = 0;
    
    for (const auto& result: results) {
      earned += result.score.earned;
    }
    
    return earned;
  }
  
  /* Returns a count of how many tests in the given test passed. */
  size_t numPassedTestsIn(const TestResults& results) {
    size_t result = 0;
    for (const auto& entry: results.individualResults) {
      if (entry.second == Result::PASS) result++;
    }
    return result;
  }
  
  /* Produces a human-readable description of the test case.
   *
   * If there's exactly one test case, we just show its status.
   *
   * If there are multiple test cases, we'll show how many of them passed, and if it's public,
   * we'll then report all tests that failed.
   */
  string summarizeTestCase(const TestResults& results) {
    if (results.individualResults.size() == 0) return "";
    if (results.individualResults.size() == 1) {
      auto status = results.individualResults.begin()->second;
      return status == Result::PASS? "" : to_string(status);
    }
    
    /* Otherwise, this is a group of tests. */
    ostringstream builder;
    builder << numPassedTestsIn(results) << " / "
            << results.individualResults.size()
            << " Test"
            << (results.individualResults.size() == 1? "" : "s")
            << " Passed.";
    
    /* If this test is public, report all individual tests. */
    if (results.isPublic) {
      builder << "\\nTests that didn't pass: \\n";
      vector<string> testFails;
      for (const auto& entry: results.individualResults) {
        if (entry.second != Result::PASS) {
          testFails.push_back(entry.first + " (" + to_string(entry.second) + ")");
        }
      }
      
      for (size_t i = 0; i < testFails.size(); i++) {
        builder << "  " << testFails[i] << "\00\n";
      }
    }
    
    return builder.str();
  } 
  
  /* Outputs a single test result to the JSON output. */
  void outputSingleResult(const TestResults& result, ostream& out) {
    /* Basic info; same for all tests. */
    out << "{" << endl;
    out << "  \"score\":      " << result.score.earned << "," << endl;
    out << "  \"max_score\":  " << result.score.possible << "," << endl;
    out << "  \"name\":       " << '"' << result.name << "\"," << endl;
    out << "  \"output\":     " << '"' << summarizeTestCase(result) << "\"," << endl;
    out << "  \"visibility\": " << "\"visible\"" << endl;
    out << "}" << endl;
  }
  
  /* Outputs a JSON report containing test results. */
  void reportResults(const vector<TestResults>& results, ostream& out) {
    Points totalEarned = totalPointsFrom(results);
    
    out << "{" << endl;
    out << "  \"score\": " << totalEarned << "," << endl;
    out << "  \"tests\": [" << endl;
    
    for (size_t i = 0; i < results.size(); i++) {
      outputSingleResult(results[i], out);
      if (i + 1 != results.size()) out << "," << endl;
    }
    
    out << "  ]" << endl;
    out << "}" << endl;
  }
}

int main(int argc, const char* argv[]) try {
  if (argc != 2) emergencyAbort("argc != 2; instead got " + to_string(argc));
  ofstream output(argv[1]);
  if (!output) emergencyAbort("Could not open file " + string(argv[1]) + " for writing.");
  
  reportResults(runAllTests(), output);
} catch (const exception& e) {
  emergencyAbort(string("Unhandled exception: ") + e.what());
} catch (...) {
  emergencyAbort("Unhandled, unknown exception.");
}
