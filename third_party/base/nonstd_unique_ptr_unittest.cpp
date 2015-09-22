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
