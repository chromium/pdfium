// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/observed_ptr.h"

#include <utility>
#include <vector>

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"

namespace fxcrt {
namespace {

class PseudoObservable final : public Observable {
 public:
  int SomeMethod() { return 42; }
  size_t ActiveObservedPtrs() const { return ActiveObserversForTesting(); }
};

class SelfObservable final : public Observable {
 public:
  ObservedPtr<SelfObservable> m_pOther;
};

}  // namespace

TEST(ObservePtr, Null) {
  ObservedPtr<PseudoObservable> ptr;
  EXPECT_EQ(nullptr, ptr.Get());
}

TEST(ObservePtr, LivesLonger) {
  ObservedPtr<PseudoObservable> ptr;
  {
    auto pObs = pdfium::MakeUnique<PseudoObservable>();
    ptr.Reset(pObs.get());
    EXPECT_NE(nullptr, ptr.Get());
    EXPECT_EQ(1u, pObs->ActiveObservedPtrs());
  }
  EXPECT_EQ(nullptr, ptr.Get());
}

TEST(ObservePtr, LivesShorter) {
  PseudoObservable obs;
  {
    ObservedPtr<PseudoObservable> ptr(&obs);
    EXPECT_NE(nullptr, ptr.Get());
    EXPECT_EQ(1u, obs.ActiveObservedPtrs());
  }
  EXPECT_EQ(0u, obs.ActiveObservedPtrs());
}

TEST(ObservePtr, CopyConstruct) {
  PseudoObservable obs;
  {
    ObservedPtr<PseudoObservable> ptr(&obs);
    EXPECT_NE(nullptr, ptr.Get());
    EXPECT_EQ(1u, obs.ActiveObservedPtrs());
    {
      ObservedPtr<PseudoObservable> ptr2(ptr);
      EXPECT_NE(nullptr, ptr2.Get());
      EXPECT_EQ(2u, obs.ActiveObservedPtrs());
    }
    EXPECT_EQ(1u, obs.ActiveObservedPtrs());
  }
  EXPECT_EQ(0u, obs.ActiveObservedPtrs());
}

TEST(ObservePtr, CopyAssign) {
  PseudoObservable obs;
  {
    ObservedPtr<PseudoObservable> ptr(&obs);
    EXPECT_NE(nullptr, ptr.Get());
    EXPECT_EQ(1u, obs.ActiveObservedPtrs());
    {
      ObservedPtr<PseudoObservable> ptr2;
      ptr2 = ptr;
      EXPECT_NE(nullptr, ptr2.Get());
      EXPECT_EQ(2u, obs.ActiveObservedPtrs());
    }
    EXPECT_EQ(1u, obs.ActiveObservedPtrs());
  }
  EXPECT_EQ(0u, obs.ActiveObservedPtrs());
}

TEST(ObservePtr, Vector) {
  PseudoObservable obs;
  {
    std::vector<ObservedPtr<PseudoObservable>> vec1;
    std::vector<ObservedPtr<PseudoObservable>> vec2;
    vec1.emplace_back(&obs);
    vec1.emplace_back(&obs);
    EXPECT_NE(nullptr, vec1[0].Get());
    EXPECT_NE(nullptr, vec1[1].Get());
    EXPECT_EQ(2u, obs.ActiveObservedPtrs());
    vec2 = vec1;
    EXPECT_NE(nullptr, vec2[0].Get());
    EXPECT_NE(nullptr, vec2[1].Get());
    EXPECT_EQ(4u, obs.ActiveObservedPtrs());
    vec1.clear();
    EXPECT_EQ(2u, obs.ActiveObservedPtrs());
    vec2.resize(10000);
    EXPECT_EQ(2u, obs.ActiveObservedPtrs());
    vec2.resize(0);
    EXPECT_EQ(0u, obs.ActiveObservedPtrs());
  }
  EXPECT_EQ(0u, obs.ActiveObservedPtrs());
}

TEST(ObservePtr, VectorAutoClear) {
  std::vector<ObservedPtr<PseudoObservable>> vec1;
  {
    PseudoObservable obs;
    vec1.emplace_back(&obs);
    vec1.emplace_back(&obs);
    EXPECT_NE(nullptr, vec1[0].Get());
    EXPECT_NE(nullptr, vec1[1].Get());
    EXPECT_EQ(2u, obs.ActiveObservedPtrs());
  }
  EXPECT_EQ(nullptr, vec1[0].Get());
  EXPECT_EQ(nullptr, vec1[1].Get());
}

TEST(ObservePtr, ResetNull) {
  PseudoObservable obs;
  ObservedPtr<PseudoObservable> ptr(&obs);
  EXPECT_EQ(1u, obs.ActiveObservedPtrs());
  ptr.Reset();
  EXPECT_EQ(0u, obs.ActiveObservedPtrs());
}

TEST(ObservePtr, Reset) {
  PseudoObservable obs1;
  PseudoObservable obs2;
  ObservedPtr<PseudoObservable> ptr(&obs1);
  EXPECT_EQ(1u, obs1.ActiveObservedPtrs());
  EXPECT_EQ(0u, obs2.ActiveObservedPtrs());
  ptr.Reset(&obs2);
  EXPECT_EQ(0u, obs1.ActiveObservedPtrs());
  EXPECT_EQ(1u, obs2.ActiveObservedPtrs());
}

TEST(ObservePtr, Equals) {
  PseudoObservable obj1;
  PseudoObservable obj2;
  ObservedPtr<PseudoObservable> null_ptr1;
  ObservedPtr<PseudoObservable> obj1_ptr1(&obj1);
  ObservedPtr<PseudoObservable> obj2_ptr1(&obj2);
  EXPECT_TRUE(&obj1 == obj1_ptr1);
  EXPECT_TRUE(obj1_ptr1 == &obj1);
  EXPECT_TRUE(&obj2 == obj2_ptr1);
  EXPECT_TRUE(obj2_ptr1 == &obj2);
  {
    ObservedPtr<PseudoObservable> null_ptr2;
    EXPECT_TRUE(null_ptr1 == null_ptr2);

    ObservedPtr<PseudoObservable> obj1_ptr2(&obj1);
    EXPECT_TRUE(obj1_ptr1 == obj1_ptr2);

    ObservedPtr<PseudoObservable> obj2_ptr2(&obj2);
    EXPECT_TRUE(obj2_ptr1 == obj2_ptr2);
  }
  EXPECT_FALSE(null_ptr1 == obj1_ptr1);
  EXPECT_FALSE(null_ptr1 == obj2_ptr1);
  EXPECT_FALSE(obj1_ptr1 == obj2_ptr1);
}

TEST(ObservePtr, NotEquals) {
  PseudoObservable obj1;
  PseudoObservable obj2;
  ObservedPtr<PseudoObservable> null_ptr1;
  ObservedPtr<PseudoObservable> obj1_ptr1(&obj1);
  ObservedPtr<PseudoObservable> obj2_ptr1(&obj2);
  EXPECT_FALSE(&obj1 != obj1_ptr1);
  EXPECT_FALSE(obj1_ptr1 != &obj1);
  EXPECT_FALSE(&obj2 != obj2_ptr1);
  EXPECT_FALSE(obj2_ptr1 != &obj2);
  {
    ObservedPtr<PseudoObservable> null_ptr2;
    ObservedPtr<PseudoObservable> obj1_ptr2(&obj1);
    ObservedPtr<PseudoObservable> obj2_ptr2(&obj2);
    EXPECT_FALSE(null_ptr1 != null_ptr2);
    EXPECT_FALSE(obj1_ptr1 != obj1_ptr2);
    EXPECT_FALSE(obj2_ptr1 != obj2_ptr2);
  }
  EXPECT_TRUE(null_ptr1 != obj1_ptr1);
  EXPECT_TRUE(null_ptr1 != obj2_ptr1);
  EXPECT_TRUE(obj1_ptr1 != obj2_ptr1);
}

TEST(ObservePtr, Bool) {
  PseudoObservable obj1;
  ObservedPtr<PseudoObservable> null_ptr;
  ObservedPtr<PseudoObservable> obj1_ptr(&obj1);
  bool null_bool = !!null_ptr;
  bool obj1_bool = !!obj1_ptr;
  EXPECT_FALSE(null_bool);
  EXPECT_TRUE(obj1_bool);
}

TEST(ObservePtr, SelfObservable) {
  SelfObservable thing;
  thing.m_pOther.Reset(&thing);
  EXPECT_EQ(&thing, thing.m_pOther.Get());
  // Must be no ASAN violations upon cleanup here.
}

TEST(ObservePtr, PairwiseObservable) {
  SelfObservable thing1;
  {
    SelfObservable thing2;
    thing1.m_pOther.Reset(&thing2);
    thing2.m_pOther.Reset(&thing1);
    EXPECT_EQ(&thing2, thing1.m_pOther.Get());
    EXPECT_EQ(&thing1, thing2.m_pOther.Get());
  }
  EXPECT_EQ(nullptr, thing1.m_pOther.Get());
  // Must be no ASAN violations upon cleanup here.
}

}  // namespace fxcrt
