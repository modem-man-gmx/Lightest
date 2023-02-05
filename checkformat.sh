# clang-format required
# in root directory of Lightest

INC=include/lightest/*.h
SRC=test/*.cpp

clang-format -n -Werror ${INC} ${SRC} && echo "Format OKAY" || echo "Coding style violation"
