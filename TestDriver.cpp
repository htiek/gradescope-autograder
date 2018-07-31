#include "TestGroup.h"
#include "Common.h"
#include <iostream>
#include <string>
using namespace std;

/* Pull from SampleTests */
extern int G0;

int main() try {
  for (auto test: allTests()) {
    auto results = test->run();
    
    cout << "Test: " << test->name() << " {" << endl;
    cout << "  Results: " << endl;
    for (auto entry: results.individualResults) {
      cout << "    " << entry.first << ": " << to_string(entry.second) << endl;
    }
    
    cout << "  Score:  " << results.score << endl;
    cout << "  Public? " << boolalpha << results.isPublic << endl;
    cout << "}" << endl;
  }
} catch (const exception& e) {
  emergencyAbort(string("Unhandled exception: ") + e.what());
} catch (...) {
  emergencyAbort("Unhandled, unknown exception.");
}
