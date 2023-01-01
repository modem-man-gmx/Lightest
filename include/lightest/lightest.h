/********************
| Lightest |
This is the core file of this library, which provides a lightest C++ unit test
framework. MIT licensed. Github Repo:
https://github.com/zhangzheheng12345/Lightest Author's Github:
https://github.com/zhangzheheng12345
********************/

#ifndef _LIGHTEST_H_
#define _LIGHTEST_H_

// Deal with different OS
#if defined(__unix__) || defined(__unix) || defined(__linux__)
#define _LINUX_
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#define _WIN_
#elif defined(__APPLE__) || defined(__MACH__)
#define _MAC_
#else
#warning Unknown platform to Lightest will cause no outputing color
#endif

#include <ctime>
#include <iostream>
#include <vector>

// For coloring on Windows
#ifdef _WIN_
#include <Windows.h>
#endif

namespace lightest {
using namespace std;

/* ========== Output Color ==========*/

enum class Color { Reset = 0, Black = 30, Red = 31, Green = 32, Yellow = 33 };
bool OutputColor = true;  // Use NO_COLOR() to set false

void SetColor(Color color) {
  if (OutputColor) {
#if defined(_LINUX_) || \
    defined(_MAC_)  // Use ASCII color code on Linux and MacOS
    cout << "\033[" << int(color) << "m";
#elif defined(_WIN_)  // Use Windows console API on Windows
    unsigned int winColor;
    switch (color) {
      case Color::Reset:
        winColor = 0x07;
        break;
      case Color::Black:
        winColor = 0x00;
      case Color::Red:
        winColor = 0x0c;
        break;
      case Color::Green:
        winColor = 0x0a;
        break;
      case Color::Yellow:
        winColor = 0x0e;
        break;
      default:
        winColor = 0x07;
        break;
    }
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), winColor);
#endif
  }
}

/* ========== Data ========== */

bool toOutput = true;  // Use NO_OUTPUT() to set to false

enum DataType { DATA_SET, DATA_REQ };

// Transfer clock_t to ms
// For on Linux clock_t's unit is us, while on Windows it's ms
inline double TimeToMs(clock_t time) {
  return double(time) / CLOCKS_PER_SEC * 1000;
}

// All test data classes should extend from Data
class Data {
 public:
  // For outputting
  virtual void Print() const = 0;
  // Offer type to enable transfer Data to exact class of test data
  virtual DataType Type() const = 0;
  virtual ~Data() {}
};

// Contain test actions on current level and sub tests' data
// Recursively call Print() to give out all the outpus
class DataSet : public Data {
 public:
  DataSet(const char* name) { this->name = name, duration = 0; }
  void Add(Data* son) { sons.push_back(son); }
  void End(bool failed, clock_t duration) {
    this->failed = failed, this->duration = duration;
  }
  void PrintSons() const {
    for (const Data* item : sons) {
      item->Print();
    }
  }
  void Print() const {
    cout << "[Begin ] " << name << endl;
    PrintSons();
    cout << "[End   ] " << name;
    if (failed) {
      SetColor(Color::Red);
      cout << " FAIL ";
    } else {
      SetColor(Color::Green);
      cout << " PASS ";
    }
    SetColor(Color::Reset);
    cout << TimeToMs(duration) << "ms" << endl;
  }
  DataType Type() const { return DATA_SET; }
  bool GetFailed() const { return failed; }
  clock_t GetDuration() const { return duration; }
  // Should offer a callback to iterate test actions and sub tests' data
  void IterSons(void (*func)(const Data*)) const {
    for (const Data* item : sons) {
      func(item);
    }
  }
  ~DataSet() {
    for(Data* item : sons) {
      delete item;
    }
  }
  const char* name;

 private:
  bool failed;
  clock_t duration;
  vector<Data*> sons;
};

// Data classes for test actions should to extend from DataUnit
// Because loggings should contain file & line information
class DataUnit {
 public:
  const char* file;
  unsigned int GetLine() const { return line; }

 protected:
  unsigned int line;
};

// Data class of REQ assertions
template <class T, class U>  // Different types for e.g. <int> == <double>
class DataReq : public Data, public DataUnit {
 public:
  DataReq(const char* file, unsigned int line, const T& actual_,
          const T& expected_, const char* operator_, const char* expr,
          bool failed_)
      : actual(actual_), expected(expected_), failed(failed_) {
    this->file = file, this->line = line, this->operator_ = operator_,
    this->expr = expr;
  }
  // Print data of REQ if assertion fails
  void Print() const {
    if (failed) {
      SetColor(Color::Red);
      cout << "    [Fail ] ";
      SetColor(Color::Reset);
      cout << file << ":" << line << ":"
           << " REQ [" << expr << "] failed" << endl
           << "        + ACTUAL: " << actual << endl
           << "        + EXPECTED: " << operator_ << " " << expected << endl;
    }
  }
  DataType Type() const { return DATA_REQ; }
  // Read only while processing data
  const T actual;
  const U expected;
  const char *operator_, *expr;
  const bool failed;
};

/* ========== Register ========== */

class Register {
 public:
  Register(const char* name) { testData = new DataSet(name); }
  Register() { testData = NULL; }
  typedef struct {
    DataSet* testData;
    int argn;
    char** argc;
  } Context;
  void Add(const char* name, void (*callerFunc)(Context&)) {
    registerList.push_back({name, callerFunc});
  }
  // Run tests registered
  void RunRegistered() {
    Context ctx = Context{testData, argn, argc};
    for (const signedFuncWrapper& item : registerList) {
      (*item.callerFunc)(ctx);
    }
  }
  // Restore argn & argc for CONFIG
  static void SetArg(int argn, char** argc) {
    Register::argn = argn, Register::argc = argc;
  }
  DataSet* testData;

 private:
  typedef struct {
    const char* name;
    void (*callerFunc)(Register::Context&);
  } signedFuncWrapper;
  vector<signedFuncWrapper> registerList;
  static int argn;
  static char** argc;
};
int Register::argn = 0;
char** Register::argc = nullptr;

Register globalRegisterConfig("");
Register globalRegisterTest("");
Register globalRegisterData("");

class Registering {
 public:
  Registering(Register& reg, const char* name,
              void (*callerFunc)(Register::Context&)) {
    reg.Add(name, callerFunc);
  }
};

/* ========== Testing ========== */

class Testing {
 public:
  Testing(const char* name) {
    reg = Register(name);
    failed = false;
    start = clock();
  }
  // Add a test data unit of a REQ assertion
  template <typename T, typename U>  // Differnt type for e.g. <int> == <double>
  void Req(const char* file, int line, const T& actual, const U& expected,
           const char* operator_, const char* expr, bool failed) {
    reg.testData->Add(new DataReq<T, U>(file, line, actual, expected, operator_,
                                        expr, failed));
    this->failed = failed;
  }
  DataSet* GetData() { return reg.testData; }
  ~Testing() {
    reg.RunRegistered();  // Run sub tests
    reg.testData->End(failed, clock() - start);
  }

 private:
  clock_t start;  // No need to report.
  bool failed;
  Register reg;
};

}  // namespace lightest

/* ========== Registering macros ========== */

#define CONFIG(name)                                                       \
  void name(int argn, char** argc);                                        \
  void call_##name(lightest::Register::Context& ctx) {                     \
    name(ctx.argn, ctx.argc);                                              \
  }                                                                        \
  lightest::Registering registering_##name(lightest::globalRegisterConfig, \
                                           #name, call_##name);            \
  void name(int argn, char** argc)

#define TEST(name)                                                       \
  void name(lightest::Testing& testing);                                 \
  void call_##name(lightest::Register::Context& ctx) {                   \
    lightest::Testing testing(#name);                                    \
    name(testing);                                                       \
    ctx.testData->Add(testing.GetData());                                \
  }                                                                      \
  lightest::Registering registering_##name(lightest::globalRegisterTest, \
                                           #name, call_##name);          \
  void name(lightest::Testing& testing)

#define DATA(name)                                                           \
  void name(const lightest::DataSet* data);                                  \
  void call_##name(lightest::Register::Context& ctx) { name(ctx.testData); } \
  lightest::Registering registering_##name(lightest::globalRegisterData,     \
                                           #name, call_##name);              \
  void name(const lightest::DataSet* data)

/* ========== Main ========== */

int main(int argn, char* argc[]) {
  // Offer arn & argc for CONFIG
  lightest::Register::SetArg(argn, argc);
  // 1. Run CONFIG
  // 2. Run tests (TEST)
  // 3. Pass test data to DATA registerer
  // 4. Run DATA
  lightest::globalRegisterConfig.RunRegistered();
  lightest::globalRegisterTest.RunRegistered();
  lightest::globalRegisterData.testData = lightest::globalRegisterTest.testData;
  lightest::globalRegisterData.RunRegistered();
  // Optionally print the default outputs
  if (lightest::toOutput) {
    lightest::globalRegisterData.testData->PrintSons();
  }
  std::cout << "Done. " << lightest::TimeToMs(clock()) << "ms used."
            << std::endl;
  return 0;
}

/* ========== Configuration macros ========== */

#define NO_COLOR() lightest::OutputColor = false;
#define NO_OUTPUT() lightest::toOutput = false;

/* ========= Timer Macros =========== */

// Unit: minisecond (ms)

// Run once and messure the time
#define TIMER(sentence)                         \
  ([&]() -> double {                            \
    clock_t start = clock();                    \
    (sentence);                                 \
    return lightest::TimeToMs(clock() - start); \
  }())

// Run several times and return the average time
#define AVG_TIMER(sentence, times)                          \
  ([&]() -> double {                                        \
    clock_t sum = 0, start;                                 \
    for (unsigned int index = 1; index <= times; index++) { \
      start = clock();                                      \
      (sentence);                                           \
      sum += clock() - start;                               \
    }                                                       \
    return lightest::TimeToMs(sum) / times;                 \
  }())

/* ========== Assertion Macros ========== */

// REQ assertion
// Additionally return a bool: true => pass, false => fail
#define REQ(actual, operator, expected)                          \
  ([&]() -> bool {                                               \
    bool res = (actual) operator(expected);                      \
    testing.Req(__FILE__, __LINE__, actual, expected, #operator, \
                #actual " " #operator" " #expected, !res);       \
    return res;                                                  \
  })()

// Condition must be true or stop currnet test
#define MUST(condition) \
  do {                  \
    if (!(condition)) { \
      return;           \
    }                   \
  } while (0)

#undef _LINUX_
#undef _WIN_
#undef _MAC_

#endif