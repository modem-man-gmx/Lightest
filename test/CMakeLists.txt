cmake_minimum_required(VERSION 3.10)

project(LightestTest)

add_executable(LightestCoreTest core_test.cpp)
target_link_libraries(LightestCoreTest lightest::lightest)

add_executable(LightestDataAnalysisExtTest data_analysis_ext_test.cpp)
target_link_libraries(LightestDataAnalysisExtTest lightest::lightest)

add_executable(Mo_test_success test_success.cpp)
target_link_libraries(Mo_test_success lightest::lightest)

add_executable(Mo_test_failing_one test_failing_one.cpp)
target_link_libraries(Mo_test_failing_one lightest::lightest)

add_executable(Mo_test_failing_more test_failing_more.cpp)
target_link_libraries(Mo_test_failing_more lightest::lightest)
