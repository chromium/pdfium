// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/cfx_timer.h"

#include <memory>

#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::_;
using testing::DoAll;
using testing::Return;
using testing::SaveArg;

namespace {

class MockTimerScheduler : public CFX_Timer::HandlerIface {
 public:
  MOCK_METHOD(int, SetTimer, (int32_t uElapse, TimerCallback lpTimerFunc));
  MOCK_METHOD(void, KillTimer, (int32_t nID));
};

class MockTimerCallback : public CFX_Timer::CallbackIface {
 public:
  MOCK_METHOD(void, OnTimerFired, ());
};

}  // namespace

class CFXTimer : public testing::Test {
  void SetUp() override { CFX_Timer::InitializeGlobals(); }
  void TearDown() override { CFX_Timer::DestroyGlobals(); }
};

TEST_F(CFXTimer, ValidTimers) {
  CFX_Timer::HandlerIface::TimerCallback fn1 = nullptr;
  CFX_Timer::HandlerIface::TimerCallback fn2 = nullptr;

  MockTimerScheduler scheduler;
  EXPECT_CALL(scheduler, SetTimer(100, _))
      .WillOnce(DoAll(SaveArg<1>(&fn1), Return(1001)));
  EXPECT_CALL(scheduler, SetTimer(200, _))
      .WillOnce(DoAll(SaveArg<1>(&fn2), Return(1002)));
  EXPECT_CALL(scheduler, KillTimer(1001));
  EXPECT_CALL(scheduler, KillTimer(1002));

  MockTimerCallback cb1;
  EXPECT_CALL(cb1, OnTimerFired()).Times(1);

  MockTimerCallback cb2;
  EXPECT_CALL(cb2, OnTimerFired()).Times(2);

  auto timer1 = std::make_unique<CFX_Timer>(&scheduler, &cb1, 100);
  auto timer2 = std::make_unique<CFX_Timer>(&scheduler, &cb2, 200);
  EXPECT_TRUE(timer1->HasValidID());
  EXPECT_TRUE(timer2->HasValidID());

  // Fire some timers.
  ASSERT_TRUE(fn1);
  ASSERT_TRUE(fn2);
  (*fn1)(1001);
  (*fn1)(1002);
  (*fn1)(1002);
}

TEST_F(CFXTimer, MisbehavingEmbedder) {
  CFX_Timer::HandlerIface::TimerCallback fn1 = nullptr;

  MockTimerScheduler scheduler;
  EXPECT_CALL(scheduler, SetTimer(100, _))
      .WillOnce(DoAll(SaveArg<1>(&fn1), Return(1001)));
  EXPECT_CALL(scheduler, KillTimer(1001));

  MockTimerCallback cb1;
  EXPECT_CALL(cb1, OnTimerFired()).Times(0);

  {
    auto timer1 = std::make_unique<CFX_Timer>(&scheduler, &cb1, 100);
    EXPECT_TRUE(timer1->HasValidID());

    // Fire callback with bad arguments.
    ASSERT_TRUE(fn1);
    (*fn1)(-1);
    (*fn1)(0);
    (*fn1)(1002);
  }

  // Fire callback against stale timer.
  (*fn1)(1001);
}
