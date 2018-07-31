#ifndef Common_Included
#define Common_Included

#include <string>

/* Reports an error and shuts down the testing framework. */
[[ noreturn ]] void emergencyAbort(const std::string& error);

#endif
