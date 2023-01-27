// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/unowned_ptr.h"

#include <functional>
#include <memory>
#include <set>
#include <utility>

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/containers/contains.h"

namespace fxcrt {
namespace {

template <typename T, typename C = std::less<T>>
class NoLinearSearchSet : public std::set<T, C> {
 public:
  typename std::set<T, C>::iterator begin() noexcept = delete;
  typename std::set<T, C>::const_iterator cbegin() const noexcept = delete;
};

class Clink {
 public:
  UnownedPtr<Clink> next_ = nullptr;
};

void DeleteDangling() {
  auto ptr2 = std::make_unique<Clink>();
  {
    auto ptr1 = std::make_unique<Clink>();
    ptr2->next_ = ptr1.get();
  }
}

void AssignDangling() {
  auto ptr2 = std::make_unique<Clink>();
  {
    auto ptr1 = std::make_unique<Clink>();
    ptr2->next_ = ptr1.get();
  }
  ptr2->next_ = nullptr;
}

void ReleaseDangling() {
  auto ptr2 = std::make_unique<Clink>();
  {
    auto ptr1 = std::make_unique<Clink>();
    ptr2->next_ = ptr1.get();
  }
  ptr2->next_.ExtractAsDangling();
}

}  // namespace

TEST(UnownedPtr, DefaultCtor) {
  UnownedPtr<Clink> ptr;
  EXPECT_FALSE(ptr);
}

TEST(UnownedPtr, NullptrCtor) {
  UnownedPtr<Clink> ptr(nullptr);
  EXPECT_FALSE(ptr);
}

TEST(UnownedPtr, RawCtor) {
  auto obj = std::make_unique<Clink>();
  UnownedPtr<Clink> ptr(obj.get());
  EXPECT_EQ(obj.get(), ptr);
}

TEST(UnownedPtr, CopyCtor) {
  std::unique_ptr<Clink> obj = std::make_unique<Clink>();
  UnownedPtr<Clink> ptr1(obj.get());
  UnownedPtr<Clink> ptr2(ptr1);
  EXPECT_EQ(obj.get(), ptr2);
  EXPECT_EQ(obj.get(), ptr1);
}

TEST(UnownedPtr, MoveCtor) {
  std::unique_ptr<Clink> obj = std::make_unique<Clink>();
  UnownedPtr<Clink> ptr1(obj.get());
  UnownedPtr<Clink> ptr2(std::move(ptr1));
  EXPECT_EQ(obj.get(), ptr2);
  EXPECT_FALSE(ptr1);
}

TEST(UnownedPtr, CopyConversionCtor) {
  std::unique_ptr<Clink> obj = std::make_unique<Clink>();
  UnownedPtr<Clink> ptr1(obj.get());
  UnownedPtr<const Clink> ptr2(ptr1);
  EXPECT_EQ(obj.get(), ptr2);
  EXPECT_EQ(obj.get(), ptr1);
}

TEST(UnownedPtr, MoveConversionCtor) {
  std::unique_ptr<Clink> obj = std::make_unique<Clink>();
  UnownedPtr<Clink> ptr1(obj.get());
  UnownedPtr<const Clink> ptr2(std::move(ptr1));
  EXPECT_EQ(obj.get(), ptr2);
  EXPECT_FALSE(ptr1);
}

TEST(UnownedPtr, NullptrAssign) {
  std::unique_ptr<Clink> obj = std::make_unique<Clink>();
  UnownedPtr<Clink> ptr(obj.get());
  ptr = nullptr;
  EXPECT_FALSE(ptr);
}

TEST(UnownedPtr, RawAssign) {
  std::unique_ptr<Clink> obj = std::make_unique<Clink>();
  UnownedPtr<Clink> ptr;
  ptr = obj.get();
  EXPECT_EQ(obj.get(), ptr);
}

TEST(UnownedPtr, CopyAssign) {
  std::unique_ptr<Clink> obj = std::make_unique<Clink>();
  UnownedPtr<Clink> ptr1(obj.get());
  UnownedPtr<Clink> ptr2;
  ptr2 = ptr1;
  EXPECT_EQ(obj.get(), ptr1);
  EXPECT_EQ(obj.get(), ptr2);
}

TEST(UnownedPtr, MoveAssign) {
  std::unique_ptr<Clink> obj = std::make_unique<Clink>();
  UnownedPtr<Clink> ptr1(obj.get());
  UnownedPtr<Clink> ptr2;
  ptr2 = std::move(ptr1);
  EXPECT_FALSE(ptr1);
  EXPECT_EQ(obj.get(), ptr2);
}

TEST(UnownedPtr, CopyConversionAssign) {
  std::unique_ptr<Clink> obj = std::make_unique<Clink>();
  UnownedPtr<Clink> ptr1(obj.get());
  UnownedPtr<const Clink> ptr2;
  ptr2 = ptr1;
  EXPECT_EQ(obj.get(), ptr1);
  EXPECT_EQ(obj.get(), ptr2);
}

TEST(UnownedPtr, MoveConversionAssign) {
  std::unique_ptr<Clink> obj = std::make_unique<Clink>();
  UnownedPtr<Clink> ptr1(obj.get());
  UnownedPtr<const Clink> ptr2;
  ptr2 = std::move(ptr1);
  EXPECT_FALSE(ptr1);
  EXPECT_EQ(obj.get(), ptr2);
}

TEST(UnownedPtr, PtrOk) {
  auto ptr1 = std::make_unique<Clink>();
  {
    auto ptr2 = std::make_unique<Clink>();
    ptr2->next_ = ptr1.get();
  }
}

TEST(UnownedPtr, PtrNotOk) {
#if defined(ADDRESS_SANITIZER)
  EXPECT_DEATH(DeleteDangling(), "");
#else
  DeleteDangling();
#endif
}

TEST(UnownedPtr, AssignOk) {
  auto ptr1 = std::make_unique<Clink>();
  {
    auto ptr2 = std::make_unique<Clink>();
    ptr2->next_ = ptr1.get();
    ptr2->next_ = nullptr;
  }
}

TEST(UnownedPtr, AssignNotOk) {
#if defined(ADDRESS_SANITIZER)
  EXPECT_DEATH(AssignDangling(), "");
#else
  AssignDangling();
#endif
}

TEST(UnownedPtr, ReleaseOk) {
  auto ptr2 = std::make_unique<Clink>();
  {
    auto ptr1 = std::make_unique<Clink>();
    ptr2->next_ = ptr1.get();
    ptr2->next_.ExtractAsDangling();
  }
}

TEST(UnownedPtr, ReleaseNotOk) {
#if defined(ADDRESS_SANITIZER)
  EXPECT_DEATH(ReleaseDangling(), "");
#else
  ReleaseDangling();
#endif
}

TEST(UnownedPtr, OperatorEQ) {
  int foo;
  UnownedPtr<int> ptr1;
  EXPECT_TRUE(ptr1 == ptr1);

  UnownedPtr<int> ptr2;
  EXPECT_TRUE(ptr1 == ptr2);

  UnownedPtr<int> ptr3(&foo);
  EXPECT_TRUE(&foo == ptr3);
  EXPECT_TRUE(ptr3 == &foo);
  EXPECT_FALSE(ptr1 == ptr3);

  ptr1 = &foo;
  EXPECT_TRUE(ptr1 == ptr3);
}

TEST(UnownedPtr, OperatorNE) {
  int foo;
  UnownedPtr<int> ptr1;
  EXPECT_FALSE(ptr1 != ptr1);

  UnownedPtr<int> ptr2;
  EXPECT_FALSE(ptr1 != ptr2);

  UnownedPtr<int> ptr3(&foo);
  EXPECT_FALSE(&foo != ptr3);
  EXPECT_FALSE(ptr3 != &foo);
  EXPECT_TRUE(ptr1 != ptr3);

  ptr1 = &foo;
  EXPECT_FALSE(ptr1 != ptr3);
}

TEST(UnownedPtr, OperatorLT) {
  int foos[2];
  UnownedPtr<int> ptr1(&foos[0]);
  UnownedPtr<int> ptr2(&foos[1]);

  EXPECT_FALSE(ptr1 < ptr1);
  EXPECT_TRUE(ptr1 < ptr2);
  EXPECT_FALSE(ptr2 < ptr1);
}

TEST(UnownedPtr, TransparentCompare) {
  int foos[2];
  UnownedPtr<int> ptr1(&foos[0]);
  UnownedPtr<int> ptr2(&foos[1]);
  NoLinearSearchSet<UnownedPtr<int>, std::less<>> holder;
  holder.insert(ptr1);
  EXPECT_NE(holder.end(), holder.find(&foos[0]));
  EXPECT_EQ(holder.end(), holder.find(&foos[1]));
  EXPECT_TRUE(pdfium::Contains(holder, &foos[0]));
  EXPECT_FALSE(pdfium::Contains(holder, &foos[1]));
}

}  // namespace fxcrt
