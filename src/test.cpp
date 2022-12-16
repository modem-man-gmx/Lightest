#include "lightest.h"

MAIN
// LESS_MAIN

TEST(TestOutputMacros) {
    MSG("msg");
    WARN("warn");
    ERR("error");
    FAIL("fail");
    int a = 100;
    LOG(a);
}

TEST(TestTimerMacoros) {
    int i = 0;
    LOG(TIMER(i++));
    LOG(AVG_TIMER(i++, 100));
}

TEST(TestAssertionMacors) {
    int a = 0, b = 0, c = 1;
    REQUIRE(a == b); REQUIRE(a != c);
    REQ_OP(a, b, ==); REQ_OP(a, c, !=);
    int arr1[] = {1, 2, 3, 4}, arr2[] = {1, 2, 3, 4}, arr3[] = {1, 2, 3}, arr4[] = {1, 2, 3, 5};
    MUST(REQ_ARR(arr1, arr2, 4, 4, ==)); // PASS
    REQ_ARR(arr1, arr3, 4, 3, ==); REQ_ARR(arr1, arr4, 4, 4, ==); // FAIL
    //  MUST(REQUIRE(1 == 2)); // FAIL & Aborted
}
