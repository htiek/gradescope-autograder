#include "TestCommon.h"
#include <cstdlib>
#include <iostream>
using namespace std;

void emergencyAbort(const string& error) {
  /* This may be run during static initialization. As a result, we need to forcibly ensure
   * that the basic streams are initialized.
   */
  ios_base::Init ensureStreamsActive;
  cerr << "===========================" << endl;
  cerr << "= Internal Error Occurred =" << endl;
  cerr << "===========================" << endl;
  cerr << endl;
  cerr << "Error: " << error << endl;
  cerr << "Aborting. " << endl;
  abort();
}
