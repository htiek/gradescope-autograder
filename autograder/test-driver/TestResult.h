/* Functions and types for working with test results. Running a set of tests produces a
 * result tree, a tree structure influenced by (but not isomorphic to) the test tree.
 */

#ifndef TestResult_Included
#define TestResult_Included

#include <string>
#include <limits>
#include <cstddef>
#include <set>
#include <memory>

/* Type representing a test outcome. */
enum class Result {
  PASS,           // Test passed.
  FAIL,           // Test failed due to an internal primitive being invoked.
  EXCEPTION,      // Test exited due to an exception we didn't trigger.
  CRASH,          // Test actually crashed!
  TIMEOUT,        // Test failed to complete in time.
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

/* Base type in the results hierarchy. */
class TestResult {
public:
  virtual ~TestResult() = default;
  
  /* Score for this test. */
  Score score() const;
  
  /* Name of this test. */
  std::string name() const;
  
  /* Returns the string to display for this test in the JSON result. */
  virtual std::string displayText() const;
  
  /* Returns how many tests within this result passed and how many were possible. */
  std::size_t testsPassed() const;
  std::size_t numTests() const;
  
  /* Produces a set with the names of failed tests within this result tree.
   * This very well might not actually report anything - for example, if this
   * is a private test.
   *
   * This is not meant to be broadly used and is a building block in some
   * implementations of displayText().
   */
  virtual std::set<std::string> reportFailedTests() const = 0;
    
protected:
  TestResult(Score score, const std::string& name, std::size_t testsPassed, std::size_t numTests);
  
private:
  Score       theScore;
  std::string theName;
  std::size_t theTestsPassed;
  std::size_t theNumTests;
};

/* Test result representing a single test case. */
class SingleTestResult: public TestResult {
public:
  SingleTestResult(Result result, Points possible, const std::string& name);
  std::set<std::string> reportFailedTests() const override;
  
private:
  Result result;
};

/* Test result representing a public test group. */
class PublicTestGroupResult: public TestResult {
public:
  PublicTestGroupResult(Score score, const std::string& name,
                        const std::set<std::shared_ptr<TestResult>>& children);

  std::string displayText() const override;
  std::set<std::string> reportFailedTests() const override;
  
private:
  std::set<std::shared_ptr<TestResult>> children;           
};

/* Test result representing a private test group. */
class PrivateTestGroupResult: public TestResult {
public:
  PrivateTestGroupResult(Score score, const std::string& name,
                         const std::set<std::shared_ptr<TestResult>>& children);
  std::set<std::string> reportFailedTests() const override;
  
private:
  std::set<std::shared_ptr<TestResult>> children;           
};

#endif
