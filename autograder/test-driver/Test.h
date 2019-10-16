#ifndef Test_Included
#define Test_Included

#include "TestResult.h"
#include <map>
#include <set>
#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <limits>
#include <ostream>

/* Type representing some sort of test that can be run. */
class Test: public std::enable_shared_from_this<Test> {
public:
  virtual ~Test() = default;
  
  /* Runs the tests, returning a collection of test results. */
  virtual std::shared_ptr<TestResult> run(const std::set<std::string>& missingFiles) = 0;
  
  /* Returns how many points this test is worth. */
  virtual Points pointsPossible() const = 0;
  
  /* How many tests are grouped here. */
  virtual std::size_t numTests() const = 0;
  
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
  std::shared_ptr<TestResult> run(const std::set<std::string> &) override;
  
  /* Returns the underlying number of points. */
  Points pointsPossible() const override;
  
  /* There's just one test. */
  std::size_t numTests() const override;
  
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
  std::shared_ptr<TestResult> run(const std::set<std::string>& missingFiles) override;
  
  /* Returns whether this group of tests is public. */
  bool isPublic() const;
  
  /* Changes the visibility of this test case. */
  void setPublic(bool isPublic = true);
  
  /* Returns all the files required to be submitted. */
  std::set<std::string> requiredFiles() const;
  
  /* Adds a new file to the list of requirements. */
  void addRequirement(const std::string& filename);
  
  /* Returns the underlying number of points, or calculates recursively as needed. */
  Points pointsPossible() const override;
  
  /* We can have lots of tests! */
  std::size_t numTests() const override;
  
private:
  std::map<std::string, std::shared_ptr<Test>> tests;
  std::set<std::string> requirements;
  Points numPoints;
  bool amIPublic = false;
  
  /* Needed for the test case definitions to be able to assemble tests. */
  friend class RootGroup;
};

/* Returns a list of all the tests in the root group. */
std::vector<std::shared_ptr<Test>> allTests();

#endif

