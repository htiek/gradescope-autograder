/**
 * Header containing definitions used to define test cases.
 *
 * You should only include this header if you want to define test cases in a file.
 */
#ifndef TestCase_Included
#define TestCase_Included

/*** Utility functions to use in the context of your test cases. ***/

/* Checks whether the given condition is true. If so, nothing happens. If the condition
 * is false, then the test fails. For example:
 *
 *     EXPECT(vec.isEmpty());
 */
#define EXPECT(condition) /* Something internal you shouldn't worry about. */

/* Immediately signals that a test has ended with the stated result.
 *
 *    for (auto elem: list) {
 *       if (elem->hasProperty()) PASS_TEST();
 *    }
 *    FAIL_TEST("No element found.");
 *
 * The reason passed into FAIL_TEST will be logged in the autograder's stdout,
 * but will not be passed along to the student.
 *
 * If, on the other hand, you would like to fail the test, and communicate
 * the reason back to the student (assuming the test case is public), you
 * can use the FAIL_TEST_VISIBLY function, which fails the test and communicates
 * the reason back to the student. For example:
 *
 *    if (!pq.isEmpty()) FAIL_TEST_VISIBLY("Priority queue was supposed to be empty.");
 */
#define PASS_TEST()               /* Something internal you shouldn't worry about. */
#define FAIL_TEST(reason)         /* Something internal you shouldn't worry about. */
#define FAIL_TEST_VISIBLY(reason) /* Something internal you shouldn't worry about. */

/* Signals that a test has failed because of some sort of internal error with
 * the autograder. Use this only if you absolutely cannot run a particular test
 * with the likely culprit being a misconfiguration on our end. For example:
 *
 *    ifstream input("SourceFile.txt");
 *    if (!input) INTERNAL_ERROR("Cannot open SourceFile.txt.");
 */
#define INTERNAL_ERROR(reason) /* Something internal you shouldn't worry about. */

/* Defines a new test case. Each test case you define should be written as
 *
 *    ADD_TEST("Description of the test") {
 *       ... your testing code goes here ...
 *    }
 *
 * These tests will automatically be added into the main test driver.
 *
 * You can optionally specify the number of points to use for a test. If you don't say
 * anything, then the test defaults to being worth one point. You can alternatively
 * specify the number of points as a second argument:
 *
 *    ADD_TEST("Description of the test", 137) {
 *       ... your testing code goes here ...
 *    }
 */
#define ADD_TEST(description) /* Something internal you shouldn't worry about. */

/* Defines a new test group. Each test case you define should be written as
 *
 *    TEST_GROUP("Equivalence Relation Tests") {
 *       ... place functions, constants, etc. for the testing group here. ...
 *    }
 *
 * These tests will automatically be added into the main test driver.
 *
 * By default, your test group will automatically compute the number of points to award
 * based on the total point value of its tests. You can override this by specifying the
 * number of points as a second argument, as seen here:
 *
 *    TEST_GROUP("Equivalence Relation Tests", 137) {
 *       ... place functions, constants, etc. for the testing group here. ...
 *    }
 */
#define TEST_GROUP(description, optionalPoints) /* Something internal you shouldn't worry about. */

/* Sets the current group to be a collection of public tests. Omit this if you want the
 * test to be private, which is the default. For example:
 *
 *    TEST_GROUP("My Very Third Test Group") {
 *       MAKE_TESTS_PUBLIC();
 *       ...
 *    }
 */
#define MAKE_TESTS_PUBLIC() /* Something internal you shouldn't worry about. */

/* Requires that the named file be submitted in order for the given test group to run.
 * If that file isn't submitted, the tests in the section won't be run and the student
 * will see an error message indicating this.
 *
 * You can require multiple files to be submitted in order for a test group to run.
 */
#define REQUIRE_SUBMITTED_FILE(filename) /* Something internal you shouldn't worry about. */






/*********************************************************************************************/
/* * * * * * Everything below this point is hacky implementation. Here Be Dragons. * * * * * */
/*********************************************************************************************/

#include "Test.h"
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>

/* Exception types signifying that a test was aborted early (possibly successfully). */
class TestFailedException: public std::logic_error {
public:
  TestFailedException(const std::string& message, std::size_t line, const char* filename);
};

class TestFailedVisiblyException: public std::logic_error {
public:
  TestFailedVisiblyException(const std::string& message, std::size_t line, const char* filename);
};

class TestSucceededException {
  // Empty
};

class InternalErrorException {
public:
  InternalErrorException(const std::string& reason, std::size_t line, const char* filename);
  std::string what() const;
private:
  const std::string reason;
};

/* Implementation of testing primitive macros. */
#undef PASS_TEST
#define PASS_TEST() doPassTest()
[[ noreturn ]] void doPassTest();

#undef FAIL_TEST
#define FAIL_TEST(reason) doFailTest(reason, __LINE__, __FILE__)
[[ noreturn ]] void doFailTest(const std::string& reason, std::size_t line, const char* filename);

#undef FAIL_TEST_VISIBLY
#define FAIL_TEST_VISIBLY(reason) doFailTestVisibly(reason, __LINE__, __FILE__)
[[ noreturn ]] void doFailTestVisibly(const std::string& reason, std::size_t line, const char* filename);

#undef INTERNAL_ERROR
#define INTERNAL_ERROR(reason) doInternalError(reason, __LINE__, __FILE__);
[[ noreturn ]] void doInternalError(const std::string& reason, std::size_t line, const char* filename);

#undef EXPECT
#define EXPECT(condition) doExpect(condition, "expect(" #condition "): condition was false.", __LINE__, __FILE__)
void doExpect(bool condition, const char* expression, std::size_t line, const char* filename);

/* Bogus return type used for initialization of test cases. */

/* Root testing group. */
namespace Root {
  /* Adds new test cases or test groups. The argument represents a chain of all the test groups
   * encountered on the way back to the root, with more recent chains at the end (e.g. a stack)
   */
  std::shared_ptr<Test> installTest(std::vector<std::string> scopeStack, std::shared_ptr<Test> test);
}

/* Type that invokes a function when constructed. */
class Invoker {
public:
  Invoker(std::function<void ()>);
};

/* The default parent namespace is the root. */
namespace Parent = Root;

/* Macro: TEST_GROUP(name) {
 *    ...
 * }
 *
 * What it actually does: This introduces a new collection of nested namespaces that inject
 * hierarchies into the names of the tests that are introduced. The namespace contains a
 * variable that gets statically initalized by an installation function.
 */
#undef  TEST_GROUP

/* This clever trick for using variadic macros to select which macro overload to use comes from
 * https://stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments.
 */
#define TEST_GROUP_MACRO(_1, _2, NAME, ...) NAME
#define TEST_GROUP(...) TEST_GROUP_MACRO(__VA_ARGS__, MAKE_TEST, MAKE_TEST_DEFAULT, X)(__VA_ARGS__)

#define MAKE_TEST(groupName, numPoints)                                             \
    MAKE_TEST_GROUP(groupName, numPoints, GROUP, __LINE__)
    
#define MAKE_TEST_DEFAULT(groupName)                                                \
    MAKE_TEST_GROUP(groupName, kDetermineAutomatically, GROUP, __LINE__)

/* Expands out the definition of the test group. */
#define MAKE_TEST_GROUP(groupName, numPoints, group, line)                          \
    namespace JOIN3(group, _TestGroup_, line) {                                     \
      auto installTest(std::vector<std::string> path,                               \
                              std::shared_ptr<Test> test) {                         \
        path.push_back(groupName);                                                  \
        return Parent::installTest(path, test);                                     \
      }                                                                             \
                                                                                    \
      auto _thisGroup =                                                             \
        Parent::installTest({}, std::make_shared<TestGroup>(groupName, numPoints)); \
                                                                                    \
      namespace Contents {                                                          \
        namespace Parent = JOIN3(group, _TestGroup_, line);                         \
      }                                                                             \
    }                                                                               \
    namespace JOIN3(group, _TestGroup_, line)::Contents

/* Macro: ADD_TEST(name) {
 *    ...
 * }
 *
 * What it actually does: prototypes a function, installs it in the current chain, and then
 * defines that function.
 */
#undef  ADD_TEST

#define ADD_TEST_MACRO(_1, _2, NAME, ...) NAME
#define ADD_TEST(...) ADD_TEST_MACRO(__VA_ARGS__, ADD_NEW_TEST, ADD_NEW_TEST_DEFAULT, X)(__VA_ARGS__)

#define ADD_NEW_TEST(name, numPoints)                                         \
    DO_ADD_TEST(name, numPoints, GROUP, __LINE__)
    
#define ADD_NEW_TEST_DEFAULT(name)                                            \
    DO_ADD_TEST(name, 1, GROUP, __LINE__)

#define DO_ADD_TEST(name, points, group, line)                                \
    void JOIN3(group, _TestFunction_, line)();                                \
    auto JOIN3(_installer, _dummy_, line) =                                   \
      Parent::installTest({},                                                 \
                          std::make_shared<TestCase>(name,                    \
                          JOIN3(group, _TestFunction_, line), points));       \
    void JOIN3(group, _TestFunction_, line)()

#define JOIN2(first, second) first##second
#define JOIN3(first, second, third) first##second##third

/* Macro: MAKE_TESTS_PUBLIC
 *
 * What it actually does: Makes the current group public. We rely on scope resolution
 * to figure out which group _thisGroup refers to.
 */
#undef  MAKE_TESTS_PUBLIC
#define MAKE_TESTS_PUBLIC() DO_MAKE_TESTS_PUBLIC(__LINE__)

#define DO_MAKE_TESTS_PUBLIC(line)                                            \
    Invoker JOIN2(_temp_public_invoker_, line)([] {                           \
      std::static_pointer_cast<TestGroup>(_thisGroup)->setPublic();           \
    })
    
/* Macro: REQUIRE_SUBMITTED_FILE
 *
 * What it actually does: Uses scope resolution to select the right test group,
 * then add the given requirement.
 */
#undef  REQUIRE_SUBMITTED_FILE
#define REQUIRE_SUBMITTED_FILE(filename) DO_REQUIRE_SUBMITTED_FILE(filename, __LINE__)

#define DO_REQUIRE_SUBMITTED_FILE(filename, line)                                \
    Invoker JOIN2(_temp_requirement_invoker_, line)([] {                         \
      std::static_pointer_cast<TestGroup>(_thisGroup)->addRequirement(filename); \
    })


#endif
