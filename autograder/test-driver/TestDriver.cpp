#include "Test.h"
#include "TestCommon.h"
#include "JSON.h"
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
  
  JSON resultToJSON(shared_ptr<TestResult> result) {
    unordered_map<string, JSON> json;
    
    json.insert(make_pair("score",      JSON(int64_t(result->score().earned))));
    json.insert(make_pair("max_score",  JSON(int64_t(result->score().possible))));
    json.insert(make_pair("name",       JSON(result->name())));
    json.insert(make_pair("output",     JSON(result->displayText())));
    json.insert(make_pair("visibility", "visible"));
    
    return JSON(json);
  }
  
  /* Outputs a JSON report containing test results. */
  void reportResults(const vector<shared_ptr<TestResult>>& results, ostream& out) {
    Score totalEarned = scoreOf(results);
    
    vector<JSON> tests;
    for (size_t i = 0; i < results.size(); i++) {
      tests.push_back(resultToJSON(results[i]));
    }
    
    unordered_map<string, JSON> result;
    result.insert(make_pair("score", JSON(int64_t(totalEarned.earned))));
    result.insert(make_pair("tests", JSON(tests)));
    out << JSON(result);
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
