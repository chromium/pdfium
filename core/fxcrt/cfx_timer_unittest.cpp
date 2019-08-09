// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/cfx_timer.h"

#include <memory>

#include "core/fxcrt/timerhandler_iface.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"

using testing::_;
using testing::DoAll;
using testing::Return;
using testing::SaveArg;

class MockTimerScheduler : public TimerHandlerIface {
 public:
  MOCK_METHOD2(SetTimer, int(int32_t uElapse, TimerCallback lpTimerFunc));
  MOCK_METHOD1(KillTimer, void(int32_t nID));
};

class MockTimerCallback : public CFX_Timer::CallbackIface {
 public:
  MOCK_METHOD0(OnTimerFired, void());
};

TEST(CFX_Timer, ValidTimers) {
  TimerHandlerIface::TimerCallback fn1 = nullptr;
  TimerHandlerIface::TimerCallback fn2 = nullptr;

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

  auto timer1 = pdfium::MakeUnique<CFX_Timer>(&scheduler, &cb1, 100);
  auto timer2 = pdfium::MakeUnique<CFX_Timer>(&scheduler, &cb2, 200);
  EXPECT_TRUE(timer1->HasValidID());
  EXPECT_TRUE(timer2->HasValidID());

  // Fire some timers.
  ASSERT_TRUE(fn1);
  ASSERT_TRUE(fn2);
  (*fn1)(1001);
  (*fn1)(1002);
  (*fn1)(1002);
}

TEST(CFX_Timer, MisbehavingEmbedder) {
  TimerHandlerIface::TimerCallback fn1 = nullptr;

  MockTimerScheduler scheduler;
  EXPECT_CALL(scheduler, SetTimer(100, _))
      .WillOnce(DoAll(SaveArg<1>(&fn1), Return(1001)));
  EXPECT_CALL(scheduler, KillTimer(1001));

  MockTimerCallback cb1;
  EXPECT_CALL(cb1, OnTimerFired()).Times(0);

  {
    auto timer1 = pdfium::MakeUnique<CFX_Timer>(&scheduler, &cb1, 100);
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
