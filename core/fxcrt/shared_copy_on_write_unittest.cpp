// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/shared_copy_on_write.h"

#include <map>
#include <string>

#include "core/fxcrt/retain_ptr.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace fxcrt {
namespace {

class Observer {
 public:
  void OnConstruct(const std::string& name) { construction_counts_[name]++; }
  void OnDestruct(const std::string& name) { destruction_counts_[name]++; }
  int GetConstructionCount(const std::string& name) {
    return construction_counts_[name];
  }
  int GetDestructionCount(const std::string& name) {
    return destruction_counts_[name];
  }

 private:
  std::map<std::string, int> construction_counts_;
  std::map<std::string, int> destruction_counts_;
};

class Object final : public Retainable {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;

  RetainPtr<Object> Clone() const { return pdfium::MakeRetain<Object>(*this); }

 private:
  Object(Observer* observer, const std::string& name)
      : name_(name), observer_(observer) {
    observer->OnConstruct(name_);
  }
  Object(const Object& that) : name_(that.name_), observer_(that.observer_) {
    observer_->OnConstruct(name_);
  }
  ~Object() override { observer_->OnDestruct(name_); }

  std::string name_;
  Observer* observer_;
};

}  // namespace

TEST(SharedCopyOnWrite, Null) {
  Observer observer;
  {
    SharedCopyOnWrite<Object> ptr;
    EXPECT_FALSE(ptr.GetObject());
  }
}

TEST(SharedCopyOnWrite, Copy) {
  Observer observer;
  {
    SharedCopyOnWrite<Object> ptr1;
    ptr1.Emplace(&observer, std::string("one"));
    {
      SharedCopyOnWrite<Object> ptr2 = ptr1;
      EXPECT_EQ(1, observer.GetConstructionCount("one"));
      EXPECT_EQ(0, observer.GetDestructionCount("one"));
    }
    {
      SharedCopyOnWrite<Object> ptr3(ptr1);
      EXPECT_EQ(1, observer.GetConstructionCount("one"));
      EXPECT_EQ(0, observer.GetDestructionCount("one"));
    }
    EXPECT_EQ(1, observer.GetConstructionCount("one"));
    EXPECT_EQ(0, observer.GetDestructionCount("one"));
  }
  EXPECT_EQ(1, observer.GetDestructionCount("one"));
}

TEST(SharedCopyOnWrite, AssignOverOld) {
  Observer observer;
  {
    SharedCopyOnWrite<Object> ptr1;
    ptr1.Emplace(&observer, std::string("one"));
    ptr1.Emplace(&observer, std::string("two"));
    EXPECT_EQ(1, observer.GetConstructionCount("one"));
    EXPECT_EQ(1, observer.GetConstructionCount("two"));
    EXPECT_EQ(1, observer.GetDestructionCount("one"));
    EXPECT_EQ(0, observer.GetDestructionCount("two"));
  }
  EXPECT_EQ(1, observer.GetDestructionCount("two"));
}

TEST(SharedCopyOnWrite, AssignOverRetained) {
  Observer observer;
  {
    SharedCopyOnWrite<Object> ptr1;
    ptr1.Emplace(&observer, std::string("one"));
    SharedCopyOnWrite<Object> ptr2(ptr1);
    ptr1.Emplace(&observer, std::string("two"));
    EXPECT_EQ(1, observer.GetConstructionCount("one"));
    EXPECT_EQ(1, observer.GetConstructionCount("two"));
    EXPECT_EQ(0, observer.GetDestructionCount("one"));
    EXPECT_EQ(0, observer.GetDestructionCount("two"));
  }
  EXPECT_EQ(1, observer.GetDestructionCount("one"));
  EXPECT_EQ(1, observer.GetDestructionCount("two"));
}

TEST(SharedCopyOnWrite, GetModify) {
  Observer observer;
  {
    SharedCopyOnWrite<Object> ptr;
    EXPECT_TRUE(ptr.GetPrivateCopy(&observer, std::string("one")));
    EXPECT_EQ(1, observer.GetConstructionCount("one"));
    EXPECT_EQ(0, observer.GetDestructionCount("one"));

    EXPECT_TRUE(ptr.GetPrivateCopy(&observer, std::string("one")));
    EXPECT_EQ(1, observer.GetConstructionCount("one"));
    EXPECT_EQ(0, observer.GetDestructionCount("one"));
    {
      SharedCopyOnWrite<Object> other(ptr);
      EXPECT_TRUE(ptr.GetPrivateCopy(&observer, std::string("one")));
      EXPECT_EQ(2, observer.GetConstructionCount("one"));
      EXPECT_EQ(0, observer.GetDestructionCount("one"));
    }
    EXPECT_EQ(2, observer.GetConstructionCount("one"));
    EXPECT_EQ(1, observer.GetDestructionCount("one"));
  }
  EXPECT_EQ(2, observer.GetDestructionCount("one"));
}

}  // namespace fxcrt
