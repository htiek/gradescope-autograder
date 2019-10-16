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
  vector<shared_ptr<TestResult>> runAllTests(const set<string>& missingFiles) {
    vector<shared_ptr<TestResult>> results;
    for (auto test: allTests()) {
      results.push_back(test->run(missingFiles));
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
  
  /* List of all missing files. */
  set<string> missingFiles(const string& missingList) {
    ifstream input(missingList);
    
    /* Nothing's missing, I guess? */
    if (!input) return {};
    
    set<string> result;
    for (string line; getline(input, line); ) {
      result.insert(line);
    }
    return result;
  }
  
  /* Given a list of missing files, produces a nice, human-readable message containing
   * those files.
   */
  string missingTextFor(const set<string>& missing) {
    if (missing.size() == 0) throw runtime_error("Asked for missing text, but no files are missing?");
  
    /* Nice message for just one file. */
    if (missing.size() == 1) {
      return "Your submission didn't include the file " + *missing.begin() + ".";
    }
  
    ostringstream result;
    result << "Your submission didn't include these files:" << endl;
    
    for (const auto& file: missing) {
      result << "  " << file << endl;
    }
    
    return result.str();
  }
  
  JSON missingToJSON(const set<string>& missing) {
    return JSON::object({
      { "name", "Warning: Not all required files submitted." },
      { "output", missingTextFor(missing) }
    });
  }
  
  JSON resultToJSON(shared_ptr<TestResult> result) {
    return JSON::object({
      { "score",     result->score().earned   },
      { "max_score", result->score().possible },
      { "name",      result->name()           },
      { "output",    result->displayText()    }
    });
  }
  
  /* Outputs a JSON report containing test results. */
  void reportResults(const string& missingList, const vector<shared_ptr<TestResult>>& results, ostream& out) {
    Score totalEarned = scoreOf(results);
    
    vector<JSON> tests;
    set<string> missing = missingFiles(missingList);
    
    /* Seed things with the missing files, if any weren't submitted. */
    if (!missing.empty()) {
      tests.push_back(missingToJSON(missing));
    }
    
    for (size_t i = 0; i < results.size(); i++) {
      tests.push_back(resultToJSON(results[i]));
    }
    
    out << JSON::object({
      { "score", totalEarned.earned },
      { "tests", tests }
    });
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
  void runTests(const string& outfile, const string& missingList) {
    ofstream output(outfile);
    if (!output) emergencyAbort("Could not open file " + outfile + " for writing.");
    
    reportResults(missingList, runAllTests(missingFiles(missingList)), output);
    
    /* For debugging purposes, dump the generated JSON. */
    output.close();
    cout << "Generated JSON file " << outfile << " with these contents: " << endl;
    cout << contentsOf(outfile) << endl;
  }
}

int main(int argc, const char* argv[]) try {
  const char* outputFile  = nullptr;
  const char* missingList = nullptr;
  bool countPoints = false;
  
  for (int i = 1; i < argc; i++) {
    if (string(argv[i]) == "--count-points") {
      countPoints = true;
    } else if (string(argv[i]) == "-o") {
      if (outputFile != nullptr) throw invalid_argument("Multiple -o flags.");
      if (i + 1 == argc)         throw invalid_argument("-o flag with no argument.");
      i++;
      outputFile = argv[i];
    } else if (string(argv[i]) == "-m") {
      if (missingList != nullptr) throw invalid_argument("Multiple -m flags.");
      if (i + 1 == argc)          throw invalid_argument("-m flag with no argument.");
      i++;
      missingList = argv[i];
    } else {
      throw invalid_argument("Unknown command-line option: " + string(argv[i]));
    }
  }
  
  /* Now, see what to do. */
  if (countPoints) {
    if (outputFile || missingList) throw invalid_argument("--count-points cannot be used with other flags.");
    countPossiblePoints();
  } else {
    if (!outputFile)  throw invalid_argument("No output file specified.");
    if (!missingList) throw invalid_argument("No missing file list specified.");
    runTests(outputFile, missingList);
  }
} catch (const exception& e) {
  emergencyAbort(string("Unhandled exception: ") + e.what());
} catch (...) {
  emergencyAbort("Unhandled, unknown exception.");
}
