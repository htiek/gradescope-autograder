#include "Common.h"
#include <cstdlib>
#include <iostream>
using namespace std;

void emergencyAbort(const string& error) {
  cerr << "===========================" << endl;
  cerr << "= Internal Error Occurred =" << endl;
  cerr << "===========================" << endl;
  cerr << endl;
  cerr << "Error: " << error << endl;
  cerr << "Aborting. " << endl;
  abort();
}
