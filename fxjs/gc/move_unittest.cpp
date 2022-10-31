// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>

#include "fxjs/gc/heap.h"
#include "testing/fxgc_unittest.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/persistent.h"

namespace {

class HeapObject : public cppgc::GarbageCollected<HeapObject> {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;

  void Trace(cppgc::Visitor* visitor) const {
    visitor->Trace(frick_);
    visitor->Trace(frack_);
  }

  cppgc::Member<HeapObject> frick_;
  cppgc::Member<HeapObject> frack_;

 private:
  HeapObject() = default;
};

class CppObject {
 public:
  CppObject() = default;

  cppgc::Persistent<HeapObject> click_;
  cppgc::Persistent<HeapObject> clack_;
};

}  // namespace

class MoveUnitTest : public FXGCUnitTest {};

TEST_F(MoveUnitTest, Member) {
  // Moving a Member<> leaves the moved-from object as null.
  auto* obj =
      cppgc::MakeGarbageCollected<HeapObject>(heap()->GetAllocationHandle());
  obj->frick_ = obj;
  obj->frack_ = std::move(obj->frick_);
  EXPECT_FALSE(obj->frick_);
  EXPECT_EQ(obj, obj->frack_);
}

TEST_F(MoveUnitTest, Persistent) {
  // Moving a Persistent<> leaves the moved-from object as null.
  auto* obj =
      cppgc::MakeGarbageCollected<HeapObject>(heap()->GetAllocationHandle());
  CppObject outsider;
  outsider.click_ = obj;
  outsider.clack_ = std::move(outsider.click_);
  EXPECT_FALSE(outsider.click_);
  EXPECT_EQ(obj, outsider.clack_);
}
