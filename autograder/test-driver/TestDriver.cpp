#include "Test.h"
#include "TestCommon.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
using namespace std;

namespace {
  /* Runs all the root tests, returning the results. */
  vector<shared_ptr<TestResult>> runAllTests() {
    vector<shared_ptr<TestResult>> results;
    for (auto test: allTests()) {
      results.push_back(test->run());
    }
    return results;
  }
  
  /* Totals the number of points earned through a set of test results. */
  Score scoreOf(const vector<shared_ptr<TestResult>>& results) {
    Score score;
    
    for (auto result: results) {
      score.earned   += result->score().earned;
      score.possible += result->score().possible;
    }
    
    return score;
  }
  
  /* Outputs a single test result to the JSON output. */
  void outputSingleResult(shared_ptr<TestResult> result, ostream& out) {
    /* Basic info; same for all tests. */
    out << "{" << endl;
    out << "  \"score\":      " << result->score().earned << "," << endl;
    out << "  \"max_score\":  " << result->score().possible << "," << endl;
    out << "  \"name\":       " << '"' << result->name() << "\"," << endl;
    out << "  \"output\":     " << '"' << result->displayText() << "\"," << endl;
    out << "  \"visibility\": " << "\"visible\"" << endl;
    out << "}" << endl;
  }
  
  /* Outputs a JSON report containing test results. */
  void reportResults(const vector<shared_ptr<TestResult>>& results, ostream& out) {
    Score totalEarned = scoreOf(results);
    
    out << "{" << endl;
    out << "  \"score\": " << totalEarned.earned << "," << endl;
    out << "  \"tests\": [" << endl;
    
    for (size_t i = 0; i < results.size(); i++) {
      outputSingleResult(results[i], out);
      if (i + 1 != results.size()) out << "," << endl;
    }
    
    out << "  ]" << endl;
    out << "}" << endl;
  }
  
  /* Returns the contents of the specified file as a string. */
  string contentsOf(const string& filename) {
    ifstream input(filename);
    if (!input) emergencyAbort("Cannot open file " + filename + " for reading.");
    
    ostringstream result;
    result << input.rdbuf();
    return result.str();
  }
  
  /* Program mode: Count points */
  void countPossiblePoints() {
    Points total = 0;
    for (auto test: allTests()) {
      total += test->pointsPossible();
    }
    cout << total;
  }
  
  /* Program mode: Run all tests! */
  void runTests(const string& outfile) {
    ofstream output(outfile);
    if (!output) emergencyAbort("Could not open file " + outfile + " for writing.");
    
    reportResults(runAllTests(), output);
    
    /* For debugging purposes, dump the generated JSON. */
    output.close();
    cout << "Generated JSON file " << outfile << " with these contents: " << endl;
    cout << contentsOf(outfile) << endl;
  }
}

int main(int argc, const char* argv[]) try {
  if (argc != 2) emergencyAbort("argc != 2; instead got " + to_string(argc));
  string argument = argv[1];
  
  /* See what to do based on what we were provided as our argument. */
  if (argument == "--count-points") {
    countPossiblePoints();
  } else if (!argument.empty() && argument[0] == '-') {
    emergencyAbort("Unknown command-line switch: " + argument);
  } else {
    runTests(argv[1]);
  }
} catch (const exception& e) {
  emergencyAbort(string("Unhandled exception: ") + e.what());
} catch (...) {
  emergencyAbort("Unhandled, unknown exception.");
}
