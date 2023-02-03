# Generate benchmark source file for Lightest with 1000 tests
lightestTestFile = open("lightest_test.cpp", "w")

lightestTestFile.write("""
// Generated by generate_lightest_test.py
#include <lightest/lightest.h>
#include <lightest/data_analysis_ext.h>

// Provide a better file name info
#undef __FILE_NAME__
#define __FILE_NAME__ "lightest_test.cpp"

""")

# Generate 1000 tests
# e.g. TEST(Test1) { REQ(1, ==, 1); }
for i in range(1000):
  lightestTestFile.write("TEST(Test" + str(i + 1) + ") { REQ(" + str(i) + ", ==, " + str(i) + "); }\n")

lightestTestFile.write("""

REPORT() {
  REPORT_PASS_RATE();
}
""")

lightestTestFile.close()