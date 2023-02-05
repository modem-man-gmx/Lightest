# clang-format required
# in root directory of Lightest

INC=include/lightest/*.h
SRC=test/*.cpp

clang-format -i ${INC} ${SRC} && echo "OKAY" || echo "Failed"
