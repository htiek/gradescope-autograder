#ifndef TestGroup_Included
#define TestGroup_Included

#include <map>
#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <limits>
#include <ostream>

/* Type representing a test outcome. */
enum class Result {
  PASS,           // Test passed.
  FAIL,           // Test failed due to an internal primitive being invoked.
  EXCEPTION,      // Test exited due to an exception we didn't trigger.
  CRASH,          // Test actually crashed!
  INTERNAL_ERROR, // Oops... we blew it!
};

std::string to_string(Result r);

/* Type representing a number of points. */
using Points = std::size_t;

/* Constant representing "figure out how many points this is automatically. */
static constexpr Points kDetermineAutomatically = std::numeric_limits<Points>::max();

/* Type representing a score. */
struct Score {
  Points earned    = Points(0);
  Points possible  = Points(0);
};
std::ostream& operator<< (std::ostream& out, const Score& score);

/* Type representing the results of a test. */
struct TestResults {
  /* Map from individual test cases to their results. */
  std::map<std::string, Result> individualResults;
  Score score;
  bool isPublic;
};

/* Type representing some sort of test that can be run. */
class Test {
public:
  virtual ~Test() = default;
  
  /* Runs the tests, returning a collection of test results. */
  virtual TestResults run() = 0;
  
  /* Returns the name of this test. */
  std::string name() const;
  
protected:
  Test(const std::string& name);
  
private:
  const std::string theName;
};

/* Type representing an individual test case. */
class TestCase: public Test {
public:
  TestCase(const std::string& name,
           std::function<void ()> theTest,
           Points numPoints = 1);

  /* Runs the individual test, returning how it went. */
  TestResults run() override;
  
private:
  std::function<void ()> testCase;
  Points numPoints;
};

/* Type representing a group of test cases. */
class TestGroup: public Test {
public:
  TestGroup(const std::string& name, Points = kDetermineAutomatically);
  
  /* Adds a in new test to the group. */
  void addTest(std::shared_ptr<Test> test);
  
  /* Returns the test with the given name, returning an error if none exists. */
  std::shared_ptr<Test> testNamed(const std::string& name) const;
  
  /* Runs all the tests in the group. */
  TestResults run() override;
  
  /* Returns whether this group of tests is public. */
  bool isPublic() const;
  
  /* Changes the visibility of this test case. */
  void setPublic(bool isPublic = true);
  
private:
  std::map<std::string, std::shared_ptr<Test>> tests;
  Points numPoints;
  bool amIPublic = false;
  
  /* Needed for the test case definitions to be able to assemble tests. */
  friend class RootGroup;
};

/* Returns a list of all the tests in the root group. */
std::vector<std::shared_ptr<Test>> allTests();

#endif

