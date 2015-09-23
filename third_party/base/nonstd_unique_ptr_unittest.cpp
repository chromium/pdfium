// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>

#include "testing/gtest/include/gtest/gtest.h"
#include "macros.h"
#include "nonstd_unique_ptr.h"

using nonstd::unique_ptr;

namespace {

// Used to test depth subtyping.
class CtorDtorLoggerParent {
 public:
  virtual ~CtorDtorLoggerParent() {}

  virtual void SetPtr(int* ptr) = 0;

  virtual int SomeMeth(int x) const = 0;
};

class CtorDtorLogger : public CtorDtorLoggerParent {
 public:
  CtorDtorLogger() : ptr_(nullptr) {}
  explicit CtorDtorLogger(int* ptr) { SetPtr(ptr); }
  ~CtorDtorLogger() override { --*ptr_; }

  void SetPtr(int* ptr) override {
    ptr_ = ptr;
    ++*ptr_;
  }

  int SomeMeth(int x) const override { return x; }

 private:
  int* ptr_;

  // Disallow evil constructors.
  CtorDtorLogger(const CtorDtorLogger&) = delete;
  void operator=(const CtorDtorLogger&) = delete;
};

struct CountingDeleter {
  explicit CountingDeleter(int* count) : count_(count) {}
  inline void operator()(double* ptr) const { (*count_)++; }
  int* count_;
};

// Do not delete this function!  It's existence is to test that you can
// return a temporarily constructed version of the scoper.
unique_ptr<CtorDtorLogger> TestReturnOfType(int* constructed) {
  return unique_ptr<CtorDtorLogger>(new CtorDtorLogger(constructed));
}

}  // namespace

TEST(UniquePtrTest, MoveTest) {
  int constructed = 0;
  int constructed4 = 0;
  {
    unique_ptr<CtorDtorLogger> ptr1(new CtorDtorLogger(&constructed));
    EXPECT_EQ(1, constructed);
    EXPECT_TRUE(ptr1);

    unique_ptr<CtorDtorLogger> ptr2(nonstd::move(ptr1));
    EXPECT_EQ(1, constructed);
    EXPECT_FALSE(ptr1);
    EXPECT_TRUE(ptr2);

    unique_ptr<CtorDtorLogger> ptr3;
    ptr3 = nonstd::move(ptr2);
    EXPECT_EQ(1, constructed);
    EXPECT_FALSE(ptr2);
    EXPECT_TRUE(ptr3);

    unique_ptr<CtorDtorLogger> ptr4(new CtorDtorLogger(&constructed4));
    EXPECT_EQ(1, constructed4);
    ptr4 = nonstd::move(ptr3);
    EXPECT_EQ(0, constructed4);
    EXPECT_FALSE(ptr3);
    EXPECT_TRUE(ptr4);
  }
  EXPECT_EQ(0, constructed);
}

TEST(UniquePtrTest, UniquePtr) {
  int constructed = 0;

  // Ensure size of unique_ptr<> doesn't increase unexpectedly.
  static_assert(sizeof(int*) >= sizeof(unique_ptr<int>),
                "unique_ptr_larger_than_raw_ptr");

  {
    unique_ptr<CtorDtorLogger> scoper(new CtorDtorLogger(&constructed));
    EXPECT_EQ(1, constructed);
    EXPECT_TRUE(scoper.get());

    EXPECT_EQ(10, scoper->SomeMeth(10));
    EXPECT_EQ(10, scoper.get()->SomeMeth(10));
    EXPECT_EQ(10, (*scoper).SomeMeth(10));
  }
  EXPECT_EQ(0, constructed);

  // Test reset() and release()
  {
    unique_ptr<CtorDtorLogger> scoper(new CtorDtorLogger(&constructed));
    EXPECT_EQ(1, constructed);
    EXPECT_TRUE(scoper.get());

    scoper.reset(new CtorDtorLogger(&constructed));
    EXPECT_EQ(1, constructed);
    EXPECT_TRUE(scoper.get());

    scoper.reset();
    EXPECT_EQ(0, constructed);
    EXPECT_FALSE(scoper.get());

    scoper.reset(new CtorDtorLogger(&constructed));
    EXPECT_EQ(1, constructed);
    EXPECT_TRUE(scoper.get());

    CtorDtorLogger* take = scoper.release();
    EXPECT_EQ(1, constructed);
    EXPECT_FALSE(scoper.get());
    delete take;
    EXPECT_EQ(0, constructed);

    scoper.reset(new CtorDtorLogger(&constructed));
    EXPECT_EQ(1, constructed);
    EXPECT_TRUE(scoper.get());
  }
  EXPECT_EQ(0, constructed);

  // Test swap(), == and !=
  {
    unique_ptr<CtorDtorLogger> scoper1;
    unique_ptr<CtorDtorLogger> scoper2;
    EXPECT_TRUE(scoper1 == scoper2.get());
    EXPECT_FALSE(scoper1 != scoper2.get());

    CtorDtorLogger* logger = new CtorDtorLogger(&constructed);
    scoper1.reset(logger);
    EXPECT_EQ(logger, scoper1.get());
    EXPECT_FALSE(scoper2.get());
    EXPECT_FALSE(scoper1 == scoper2.get());
    EXPECT_TRUE(scoper1 != scoper2.get());

    scoper2.swap(scoper1);
    EXPECT_EQ(logger, scoper2.get());
    EXPECT_FALSE(scoper1.get());
    EXPECT_FALSE(scoper1 == scoper2.get());
    EXPECT_TRUE(scoper1 != scoper2.get());
  }
  EXPECT_EQ(0, constructed);
}

TEST(UniquePtrTest, UniquePtrWithArray) {
  static const int kNumLoggers = 12;

  int constructed = 0;

  {
    unique_ptr<CtorDtorLogger[]> scoper(new CtorDtorLogger[kNumLoggers]);
    EXPECT_TRUE(scoper);
    EXPECT_EQ(&scoper[0], scoper.get());
    for (int i = 0; i < kNumLoggers; ++i) {
      scoper[i].SetPtr(&constructed);
    }
    EXPECT_EQ(12, constructed);

    EXPECT_EQ(10, scoper.get()->SomeMeth(10));
    EXPECT_EQ(10, scoper[2].SomeMeth(10));
  }
  EXPECT_EQ(0, constructed);

  // Test reset() and release()
  {
    unique_ptr<CtorDtorLogger[]> scoper;
    EXPECT_FALSE(scoper.get());
    EXPECT_FALSE(scoper.release());
    EXPECT_FALSE(scoper.get());
    scoper.reset();
    EXPECT_FALSE(scoper.get());

    scoper.reset(new CtorDtorLogger[kNumLoggers]);
    for (int i = 0; i < kNumLoggers; ++i) {
      scoper[i].SetPtr(&constructed);
    }
    EXPECT_EQ(12, constructed);
    scoper.reset();
    EXPECT_EQ(0, constructed);

    scoper.reset(new CtorDtorLogger[kNumLoggers]);
    for (int i = 0; i < kNumLoggers; ++i) {
      scoper[i].SetPtr(&constructed);
    }
    EXPECT_EQ(12, constructed);
    CtorDtorLogger* ptr = scoper.release();
    EXPECT_EQ(12, constructed);
    delete[] ptr;
    EXPECT_EQ(0, constructed);
  }
  EXPECT_EQ(0, constructed);

  // Test swap(), ==, !=, and type-safe Boolean.
  {
    unique_ptr<CtorDtorLogger[]> scoper1;
    unique_ptr<CtorDtorLogger[]> scoper2;
    EXPECT_TRUE(scoper1 == scoper2.get());
    EXPECT_FALSE(scoper1 != scoper2.get());

    CtorDtorLogger* loggers = new CtorDtorLogger[kNumLoggers];
    for (int i = 0; i < kNumLoggers; ++i) {
      loggers[i].SetPtr(&constructed);
    }
    scoper1.reset(loggers);
    EXPECT_TRUE(scoper1);
    EXPECT_EQ(loggers, scoper1.get());
    EXPECT_FALSE(scoper2);
    EXPECT_FALSE(scoper2.get());
    EXPECT_FALSE(scoper1 == scoper2.get());
    EXPECT_TRUE(scoper1 != scoper2.get());

    scoper2.swap(scoper1);
    EXPECT_EQ(loggers, scoper2.get());
    EXPECT_FALSE(scoper1.get());
    EXPECT_FALSE(scoper1 == scoper2.get());
    EXPECT_TRUE(scoper1 != scoper2.get());
  }
  EXPECT_EQ(0, constructed);
}

TEST(UniquePtrTest, ReturnTypeBehavior) {
  int constructed = 0;

  // Test that we can return a unique_ptr.
  {
    CtorDtorLogger* logger = new CtorDtorLogger(&constructed);
    unique_ptr<CtorDtorLogger> scoper(logger);
    EXPECT_EQ(1, constructed);
  }
  EXPECT_EQ(0, constructed);

  // Test uncaught return type not leak.
  {
    CtorDtorLogger* logger = new CtorDtorLogger(&constructed);
    unique_ptr<CtorDtorLogger> scoper(logger);
    EXPECT_EQ(1, constructed);
  }
  EXPECT_EQ(0, constructed);

  // Call TestReturnOfType() so the compiler doesn't warn for an unused
  // function.
  { TestReturnOfType(&constructed); }
  EXPECT_EQ(0, constructed);
}

TEST(UniquePtrTest, CustomDeleter) {
  double dummy_value;  // Custom deleter never touches this value.
  int deletes = 0;
  int alternate_deletes = 0;

  // Normal delete support.
  {
    deletes = 0;
    unique_ptr<double, CountingDeleter> scoper(&dummy_value,
                                               CountingDeleter(&deletes));
    EXPECT_EQ(0, deletes);
    EXPECT_TRUE(scoper.get());
  }
  EXPECT_EQ(1, deletes);

  // Test reset() and release().
  deletes = 0;
  {
    unique_ptr<double, CountingDeleter> scoper(nullptr,
                                               CountingDeleter(&deletes));
    EXPECT_FALSE(scoper.get());
    EXPECT_FALSE(scoper.release());
    EXPECT_FALSE(scoper.get());
    scoper.reset();
    EXPECT_FALSE(scoper.get());
    EXPECT_EQ(0, deletes);

    scoper.reset(&dummy_value);
    scoper.reset();
    EXPECT_EQ(1, deletes);

    scoper.reset(&dummy_value);
    EXPECT_EQ(&dummy_value, scoper.release());
  }
  EXPECT_EQ(1, deletes);

  // Test get_deleter().
  deletes = 0;
  alternate_deletes = 0;
  {
    unique_ptr<double, CountingDeleter> scoper(&dummy_value,
                                               CountingDeleter(&deletes));
    // Call deleter manually.
    EXPECT_EQ(0, deletes);
    scoper.get_deleter()(&dummy_value);
    EXPECT_EQ(1, deletes);

    // Deleter is still there after reset.
    scoper.reset();
    EXPECT_EQ(2, deletes);
    scoper.get_deleter()(&dummy_value);
    EXPECT_EQ(3, deletes);

    // Deleter can be assigned into (matches C++11 unique_ptr<> spec).
    scoper.get_deleter() = CountingDeleter(&alternate_deletes);
    scoper.reset(&dummy_value);
    EXPECT_EQ(0, alternate_deletes);
  }
  EXPECT_EQ(3, deletes);
  EXPECT_EQ(1, alternate_deletes);

  // Test swap(), ==, !=, and type-safe Boolean.
  {
    unique_ptr<double, CountingDeleter> scoper1(nullptr,
                                                CountingDeleter(&deletes));
    unique_ptr<double, CountingDeleter> scoper2(nullptr,
                                                CountingDeleter(&deletes));
    EXPECT_TRUE(scoper1 == scoper2.get());
    EXPECT_FALSE(scoper1 != scoper2.get());

    scoper1.reset(&dummy_value);
    EXPECT_TRUE(scoper1);
    EXPECT_EQ(&dummy_value, scoper1.get());
    EXPECT_FALSE(scoper2);
    EXPECT_FALSE(scoper2.get());
    EXPECT_FALSE(scoper1 == scoper2.get());
    EXPECT_TRUE(scoper1 != scoper2.get());

    scoper2.swap(scoper1);
    EXPECT_EQ(&dummy_value, scoper2.get());
    EXPECT_FALSE(scoper1.get());
    EXPECT_FALSE(scoper1 == scoper2.get());
    EXPECT_TRUE(scoper1 != scoper2.get());
  }
}

unique_ptr<int> NullIntReturn() {
  return nullptr;
}

TEST(UniquePtrTest, Nullptr) {
  unique_ptr<int> scoper1(nullptr);
  unique_ptr<int> scoper2(new int);
  scoper2 = nullptr;
  unique_ptr<int> scoper3(NullIntReturn());
  unique_ptr<int> scoper4 = NullIntReturn();
  EXPECT_EQ(nullptr, scoper1.get());
  EXPECT_EQ(nullptr, scoper2.get());
  EXPECT_EQ(nullptr, scoper3.get());
  EXPECT_EQ(nullptr, scoper4.get());
}

unique_ptr<int[]> NullIntArrayReturn() {
  return nullptr;
}

TEST(UniquePtrTest, NullptrArray) {
  unique_ptr<int[]> scoper1(nullptr);
  unique_ptr<int[]> scoper2(new int[3]);
  scoper2 = nullptr;
  unique_ptr<int[]> scoper3(NullIntArrayReturn());
  unique_ptr<int[]> scoper4 = NullIntArrayReturn();
  EXPECT_EQ(nullptr, scoper1.get());
  EXPECT_EQ(nullptr, scoper2.get());
  EXPECT_EQ(nullptr, scoper3.get());
  EXPECT_EQ(nullptr, scoper4.get());
}

// Logging a unique_ptr<T> to an ostream shouldn't convert it to a boolean
// value first.
TEST(ScopedPtrTest, LoggingDoesntConvertToBoolean) {
  unique_ptr<int> x(new int);
  std::stringstream s1;
  s1 << x;

  std::stringstream s2;
  s2 << x.get();

  EXPECT_EQ(s2.str(), s1.str());
}
