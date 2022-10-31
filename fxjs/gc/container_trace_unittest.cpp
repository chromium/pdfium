// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxjs/gc/container_trace.h"

#include <stdint.h>

#include <list>
#include <map>
#include <set>
#include <vector>

#include "testing/gtest/include/gtest/gtest.h"
#include "v8/include/cppgc/member.h"

namespace {

class Thing : public cppgc::GarbageCollected<Thing> {
 public:
  void Trace(cppgc::Visitor* visitor) const {}
};

class CountingVisitor {
 public:
  CountingVisitor() = default;

  void Trace(const void* that) { ++call_count_; }
  int call_count() const { return call_count_; }

 private:
  int call_count_ = 0;
};

}  // namespace

TEST(ContainerTrace, ActualListTrace) {
  std::list<cppgc::Member<Thing>> thing;
  thing.emplace_back(nullptr);

  CountingVisitor cv;
  ContainerTrace(&cv, thing);
  EXPECT_EQ(1, cv.call_count());
}

TEST(ContainerTrace, ActualMapTrace) {
  std::map<int, cppgc::Member<Thing>> thing;
  thing[42] = nullptr;

  CountingVisitor cv;
  ContainerTrace(&cv, thing);
  EXPECT_EQ(1, cv.call_count());
}

TEST(ContainerTrace, ActualSetTrace) {
  std::set<cppgc::Member<Thing>> thing;
  thing.insert(nullptr);

  CountingVisitor cv;
  ContainerTrace(&cv, thing);
  EXPECT_EQ(1, cv.call_count());
}

TEST(ContainerTrace, ActualVectorTrace) {
  std::vector<cppgc::Member<Thing>> thing;
  thing.emplace_back(nullptr);

  CountingVisitor cv;
  ContainerTrace(&cv, thing);
  EXPECT_EQ(1, cv.call_count());
}
