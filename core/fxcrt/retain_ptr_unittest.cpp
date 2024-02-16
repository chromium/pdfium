// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/retain_ptr.h"

#include <functional>
#include <set>
#include <utility>
#include <vector>

#include "core/fxcrt/containers/contains.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/pseudo_retainable.h"

namespace {

template <typename T, typename C = std::less<T>>
class NoLinearSearchSet : public std::set<T, C> {
 public:
  typename std::set<T, C>::iterator begin() noexcept = delete;
  typename std::set<T, C>::const_iterator cbegin() const noexcept = delete;
};

}  // namespace

namespace fxcrt {
namespace {

template <typename T, typename C = std::less<T>>
class NoLinearSearchSet : public std::set<T, C> {
 public:
  typename std::set<T, C>::iterator begin() noexcept = delete;
  typename std::set<T, C>::const_iterator cbegin() const noexcept = delete;
};

}  // namespace

TEST(RetainPtr, DefaultCtor) {
  RetainPtr<PseudoRetainable> ptr;
  EXPECT_FALSE(ptr.Get());
}

TEST(RetainPtr, NullptrCtor) {
  RetainPtr<PseudoRetainable> ptr(nullptr);
  EXPECT_FALSE(ptr.Get());
}

TEST(RetainPtr, RawCtor) {
  PseudoRetainable obj;
  {
    RetainPtr<PseudoRetainable> ptr(&obj);
    EXPECT_EQ(&obj, ptr.Get());
    EXPECT_EQ(1, obj.retain_count());
    EXPECT_EQ(0, obj.release_count());
  }
  EXPECT_EQ(1, obj.retain_count());
  EXPECT_EQ(1, obj.release_count());
}

TEST(RetainPtr, CopyCtor) {
  PseudoRetainable obj;
  {
    RetainPtr<PseudoRetainable> ptr1(&obj);
    {
      RetainPtr<PseudoRetainable> ptr2(ptr1);
      EXPECT_EQ(2, obj.retain_count());
      EXPECT_EQ(0, obj.release_count());
    }
    EXPECT_EQ(2, obj.retain_count());
    EXPECT_EQ(1, obj.release_count());
  }
  EXPECT_EQ(2, obj.retain_count());
  EXPECT_EQ(2, obj.release_count());
}

TEST(RetainPtr, MoveCtor) {
  PseudoRetainable obj;
  {
    RetainPtr<PseudoRetainable> ptr1(&obj);
    {
      RetainPtr<PseudoRetainable> ptr2(std::move(ptr1));
      EXPECT_FALSE(ptr1.Get());
      EXPECT_EQ(&obj, ptr2.Get());
      EXPECT_EQ(1, obj.retain_count());
      EXPECT_EQ(0, obj.release_count());
    }
    EXPECT_EQ(1, obj.retain_count());
    EXPECT_EQ(1, obj.release_count());
  }
  EXPECT_EQ(1, obj.retain_count());
  EXPECT_EQ(1, obj.release_count());
}

TEST(RetainPtr, CopyConversionCtor) {
  PseudoRetainable obj;
  {
    RetainPtr<PseudoRetainable> ptr1(&obj);
    {
      RetainPtr<const PseudoRetainable> ptr2(ptr1);
      EXPECT_EQ(2, obj.retain_count());
      EXPECT_EQ(0, obj.release_count());
    }
    EXPECT_EQ(2, obj.retain_count());
    EXPECT_EQ(1, obj.release_count());
  }
  EXPECT_EQ(2, obj.retain_count());
  EXPECT_EQ(2, obj.release_count());
}

TEST(RetainPtr, MoveConversionCtor) {
  PseudoRetainable obj;
  {
    RetainPtr<PseudoRetainable> ptr1(&obj);
    {
      RetainPtr<const PseudoRetainable> ptr2(std::move(ptr1));
      EXPECT_FALSE(ptr1.Get());
      EXPECT_EQ(&obj, ptr2.Get());
      EXPECT_EQ(1, obj.retain_count());
      EXPECT_EQ(0, obj.release_count());
    }
    EXPECT_EQ(1, obj.retain_count());
    EXPECT_EQ(1, obj.release_count());
  }
  EXPECT_EQ(1, obj.retain_count());
  EXPECT_EQ(1, obj.release_count());
}

TEST(RetainPtr, NullptrAssign) {
  PseudoRetainable obj;
  RetainPtr<PseudoRetainable> ptr(&obj);
  ptr = nullptr;
  EXPECT_FALSE(ptr);
}

TEST(RetainPtr, RawAssign) {
  PseudoRetainable obj;
  RetainPtr<PseudoRetainable> ptr;
  ptr = pdfium::WrapRetain(&obj);
  EXPECT_EQ(&obj, ptr);
}

TEST(RetainPtr, CopyAssign) {
  PseudoRetainable obj;
  {
    RetainPtr<PseudoRetainable> ptr(&obj);
    {
      RetainPtr<PseudoRetainable> ptr2;
      ptr2 = ptr;
      EXPECT_EQ(2, obj.retain_count());
      EXPECT_EQ(0, obj.release_count());
    }
    {
      // Test assignment from wrapped underlying type.
      RetainPtr<PseudoRetainable> ptr2;
      ptr2 = pdfium::WrapRetain(ptr.Get());
      EXPECT_EQ(3, obj.retain_count());
      EXPECT_EQ(1, obj.release_count());
    }
    EXPECT_EQ(3, obj.retain_count());
    EXPECT_EQ(2, obj.release_count());
  }
  EXPECT_EQ(3, obj.retain_count());
  EXPECT_EQ(3, obj.release_count());
}

TEST(RetainPtr, MoveAssign) {
  PseudoRetainable obj;
  {
    RetainPtr<PseudoRetainable> ptr1(&obj);
    {
      RetainPtr<PseudoRetainable> ptr2;
      EXPECT_EQ(&obj, ptr1.Get());
      EXPECT_FALSE(ptr2.Get());
      ptr2 = std::move(ptr1);
      EXPECT_FALSE(ptr1.Get());
      EXPECT_EQ(&obj, ptr2.Get());
      EXPECT_EQ(1, obj.retain_count());
      EXPECT_EQ(0, obj.release_count());
    }
    EXPECT_EQ(1, obj.retain_count());
    EXPECT_EQ(1, obj.release_count());
  }
  EXPECT_EQ(1, obj.retain_count());
  EXPECT_EQ(1, obj.release_count());
}

TEST(RetainPtr, CopyConvertAssign) {
  PseudoRetainable obj;
  {
    RetainPtr<PseudoRetainable> ptr(&obj);
    {
      RetainPtr<const PseudoRetainable> ptr2;
      ptr2 = ptr;
      EXPECT_EQ(2, obj.retain_count());
      EXPECT_EQ(0, obj.release_count());
    }
    {
      // Test assignment from wrapped underlying type.
      RetainPtr<const PseudoRetainable> ptr2;
      ptr2 = pdfium::WrapRetain(ptr.Get());
      EXPECT_EQ(3, obj.retain_count());
      EXPECT_EQ(1, obj.release_count());
    }
    EXPECT_EQ(3, obj.retain_count());
    EXPECT_EQ(2, obj.release_count());
  }
  EXPECT_EQ(3, obj.retain_count());
  EXPECT_EQ(3, obj.release_count());
}

TEST(RetainPtr, MoveConvertAssign) {
  PseudoRetainable obj;
  {
    RetainPtr<PseudoRetainable> ptr1(&obj);
    {
      RetainPtr<const PseudoRetainable> ptr2;
      ptr2 = std::move(ptr1);
      EXPECT_FALSE(ptr1);
      EXPECT_EQ(&obj, ptr2.Get());
      EXPECT_EQ(1, obj.retain_count());
      EXPECT_EQ(0, obj.release_count());
    }
    EXPECT_EQ(1, obj.retain_count());
    EXPECT_EQ(1, obj.release_count());
  }
  EXPECT_EQ(1, obj.retain_count());
  EXPECT_EQ(1, obj.release_count());
}

TEST(RetainPtr, AmbiguousExpression) {
  class A : public Retainable {};
  class B : public A {};

  // Test passes if it compiles without error.
  RetainPtr<A> var = (0) ? pdfium::MakeRetain<A>() : pdfium::MakeRetain<B>();
  EXPECT_TRUE(var);
}

TEST(RetainPtr, ResetNull) {
  PseudoRetainable obj;
  {
    RetainPtr<PseudoRetainable> ptr(&obj);
    ptr.Reset();
    EXPECT_EQ(1, obj.retain_count());
    EXPECT_EQ(1, obj.release_count());
  }
  EXPECT_EQ(1, obj.retain_count());
  EXPECT_EQ(1, obj.release_count());
}

TEST(RetainPtr, Reset) {
  PseudoRetainable obj1;
  PseudoRetainable obj2;
  {
    RetainPtr<PseudoRetainable> ptr(&obj1);
    ptr.Reset(&obj2);
    EXPECT_EQ(1, obj1.retain_count());
    EXPECT_EQ(1, obj1.release_count());
    EXPECT_EQ(1, obj2.retain_count());
    EXPECT_EQ(0, obj2.release_count());
  }
  EXPECT_EQ(1, obj1.retain_count());
  EXPECT_EQ(1, obj1.release_count());
  EXPECT_EQ(1, obj2.retain_count());
  EXPECT_EQ(1, obj2.release_count());
}

TEST(RetainPtr, Swap) {
  PseudoRetainable obj1;
  PseudoRetainable obj2;
  {
    RetainPtr<PseudoRetainable> ptr1(&obj1);
    {
      RetainPtr<PseudoRetainable> ptr2(&obj2);
      ptr1.Swap(ptr2);
      EXPECT_EQ(1, obj1.retain_count());
      EXPECT_EQ(0, obj1.release_count());
      EXPECT_EQ(1, obj2.retain_count());
      EXPECT_EQ(0, obj2.release_count());
    }
    EXPECT_EQ(1, obj1.retain_count());
    EXPECT_EQ(1, obj1.release_count());
    EXPECT_EQ(1, obj2.retain_count());
    EXPECT_EQ(0, obj2.release_count());
  }
  EXPECT_EQ(1, obj1.retain_count());
  EXPECT_EQ(1, obj1.release_count());
  EXPECT_EQ(1, obj2.retain_count());
  EXPECT_EQ(1, obj2.release_count());
}

TEST(RetainPtr, Leak) {
  PseudoRetainable obj;
  PseudoRetainable* leak;
  {
    RetainPtr<PseudoRetainable> ptr(&obj);
    leak = ptr.Leak();
    EXPECT_EQ(1, obj.retain_count());
    EXPECT_EQ(0, obj.release_count());
  }
  EXPECT_EQ(1, obj.retain_count());
  EXPECT_EQ(0, obj.release_count());
  {
    RetainPtr<PseudoRetainable> ptr;
    ptr.Unleak(leak);
    EXPECT_EQ(1, obj.retain_count());
    EXPECT_EQ(0, obj.release_count());
  }
  EXPECT_EQ(1, obj.retain_count());
  EXPECT_EQ(1, obj.release_count());
}

TEST(RetainPtr, SwapNull) {
  PseudoRetainable obj1;
  {
    RetainPtr<PseudoRetainable> ptr1(&obj1);
    {
      RetainPtr<PseudoRetainable> ptr2;
      ptr1.Swap(ptr2);
      EXPECT_FALSE(ptr1);
      EXPECT_TRUE(ptr2);
      EXPECT_EQ(1, obj1.retain_count());
      EXPECT_EQ(0, obj1.release_count());
    }
    EXPECT_EQ(1, obj1.retain_count());
    EXPECT_EQ(1, obj1.release_count());
  }
  EXPECT_EQ(1, obj1.retain_count());
  EXPECT_EQ(1, obj1.release_count());
}

TEST(RetainPtr, Equals) {
  PseudoRetainable obj1;
  PseudoRetainable obj2;
  RetainPtr<PseudoRetainable> null_ptr1;
  RetainPtr<PseudoRetainable> obj1_ptr1(&obj1);
  RetainPtr<PseudoRetainable> obj2_ptr1(&obj2);
  {
    RetainPtr<PseudoRetainable> null_ptr2;
    EXPECT_TRUE(null_ptr1 == null_ptr2);
    EXPECT_TRUE(null_ptr1 == nullptr);

    RetainPtr<PseudoRetainable> obj1_ptr2(&obj1);
    EXPECT_TRUE(obj1_ptr1 == obj1_ptr2);
    EXPECT_TRUE(obj1_ptr2 == &obj1);

    RetainPtr<PseudoRetainable> obj2_ptr2(&obj2);
    EXPECT_TRUE(obj2_ptr1 == obj2_ptr2);
  }
  EXPECT_FALSE(null_ptr1 == obj1_ptr1);
  EXPECT_FALSE(null_ptr1 == obj2_ptr1);
  EXPECT_FALSE(obj1_ptr1 == obj2_ptr1);
}

TEST(RetainPtr, EqualsReflexive) {
  PseudoRetainable obj1;
  PseudoRetainable obj2;
  RetainPtr<PseudoRetainable> obj1_ptr(&obj1);
  RetainPtr<PseudoRetainable> obj2_ptr(&obj2);
  EXPECT_TRUE(&obj1 == obj1_ptr);
  EXPECT_FALSE(&obj1 == obj2_ptr);
  EXPECT_FALSE(&obj2 == obj1_ptr);
  EXPECT_TRUE(&obj2 == obj2_ptr);
}

TEST(RetainPtr, NotEquals) {
  PseudoRetainable obj1;
  PseudoRetainable obj2;
  RetainPtr<PseudoRetainable> null_ptr1;
  RetainPtr<PseudoRetainable> obj1_ptr1(&obj1);
  RetainPtr<PseudoRetainable> obj2_ptr1(&obj2);
  {
    RetainPtr<PseudoRetainable> null_ptr2;
    RetainPtr<PseudoRetainable> obj1_ptr2(&obj1);
    RetainPtr<PseudoRetainable> obj2_ptr2(&obj2);
    EXPECT_FALSE(null_ptr1 != null_ptr2);
    EXPECT_FALSE(obj1_ptr1 != obj1_ptr2);
    EXPECT_FALSE(obj2_ptr1 != obj2_ptr2);
  }
  EXPECT_TRUE(null_ptr1 != obj1_ptr1);
  EXPECT_TRUE(null_ptr1 != obj2_ptr1);
  EXPECT_TRUE(obj1_ptr1 != obj2_ptr1);
}

TEST(RetainPtr, NotEqualsReflexive) {
  PseudoRetainable obj1;
  PseudoRetainable obj2;
  RetainPtr<PseudoRetainable> obj1_ptr(&obj1);
  RetainPtr<PseudoRetainable> obj2_ptr(&obj2);
  EXPECT_FALSE(&obj1 != obj1_ptr);
  EXPECT_TRUE(&obj1 != obj2_ptr);
  EXPECT_TRUE(&obj2 != obj1_ptr);
  EXPECT_FALSE(&obj2 != obj2_ptr);
}

TEST(RetainPtr, LessThan) {
  PseudoRetainable objs[2];
  RetainPtr<PseudoRetainable> obj1_ptr(&objs[0]);
  RetainPtr<PseudoRetainable> obj2_ptr(&objs[1]);
  EXPECT_TRUE(obj1_ptr < obj2_ptr);
  EXPECT_FALSE(obj2_ptr < obj1_ptr);
}

TEST(RetainPtr, Bool) {
  PseudoRetainable obj1;
  RetainPtr<PseudoRetainable> null_ptr;
  RetainPtr<PseudoRetainable> obj1_ptr(&obj1);
  bool null_bool = !!null_ptr;
  bool obj1_bool = !!obj1_ptr;
  EXPECT_FALSE(null_bool);
  EXPECT_TRUE(obj1_bool);
}

TEST(RetainPtr, MakeRetained) {
  auto ptr = pdfium::MakeRetain<Retainable>();
  EXPECT_TRUE(ptr->HasOneRef());
  {
    RetainPtr<Retainable> other = ptr;
    EXPECT_FALSE(ptr->HasOneRef());
  }
  EXPECT_TRUE(ptr->HasOneRef());
}

TEST(RetainPtr, VectorMove) {
  // Proves move ctor is selected by std::vector over copy/delete, this
  // may require the ctor to be marked "noexcept".
  PseudoRetainable obj;
  {
    RetainPtr<PseudoRetainable> ptr(&obj);
    std::vector<RetainPtr<PseudoRetainable>> vec1;
    vec1.push_back(std::move(ptr));
    EXPECT_EQ(1, obj.retain_count());
    EXPECT_EQ(0, obj.release_count());

    std::vector<RetainPtr<PseudoRetainable>> vec2 = std::move(vec1);
    EXPECT_EQ(1, obj.retain_count());
    EXPECT_EQ(0, obj.release_count());

    vec2.resize(4096);
    EXPECT_EQ(1, obj.retain_count());
    EXPECT_EQ(0, obj.release_count());
  }
  EXPECT_EQ(1, obj.retain_count());
  EXPECT_EQ(1, obj.release_count());
}

TEST(RetainPtr, SetContains) {
  // Makes sure pdfium::Contains() works the same way with raw pointers and
  // RetainPtrs for containers that use find().
  PseudoRetainable obj1;
  PseudoRetainable obj2;
  RetainPtr<PseudoRetainable> ptr1(&obj1);
  RetainPtr<PseudoRetainable> ptr2(&obj2);
  RetainPtr<const PseudoRetainable> const_ptr1(&obj1);
  RetainPtr<const PseudoRetainable> const_ptr2(&obj2);
  NoLinearSearchSet<RetainPtr<const PseudoRetainable>, std::less<>> the_set;

  // Intially, two smart pointers to each object.
  EXPECT_EQ(2, obj1.retain_count());
  EXPECT_EQ(0, obj1.release_count());
  EXPECT_EQ(2, obj2.retain_count());
  EXPECT_EQ(0, obj2.release_count());

  // Passed by const-ref, increment on copy into set's data structure.
  the_set.insert(const_ptr1);
  EXPECT_EQ(3, obj1.retain_count());
  EXPECT_EQ(0, obj1.release_count());
  EXPECT_EQ(2, obj2.retain_count());
  EXPECT_EQ(0, obj2.release_count());

  // None of the following should cause any churn.
  EXPECT_NE(the_set.end(), the_set.find(&obj1));
  EXPECT_EQ(the_set.end(), the_set.find(&obj2));
  EXPECT_TRUE(pdfium::Contains(the_set, &obj1));
  EXPECT_FALSE(pdfium::Contains(the_set, &obj2));
  EXPECT_EQ(3, obj1.retain_count());
  EXPECT_EQ(0, obj1.release_count());
  EXPECT_EQ(2, obj2.retain_count());
  EXPECT_EQ(0, obj2.release_count());

  EXPECT_NE(the_set.end(), the_set.find(const_ptr1));
  EXPECT_EQ(the_set.end(), the_set.find(const_ptr2));
  EXPECT_TRUE(pdfium::Contains(the_set, const_ptr1));
  EXPECT_FALSE(pdfium::Contains(the_set, const_ptr2));
  EXPECT_EQ(3, obj1.retain_count());
  EXPECT_EQ(0, obj1.release_count());
  EXPECT_EQ(2, obj2.retain_count());
  EXPECT_EQ(0, obj2.release_count());

  // These involve const-removing conversions which seem to churn.
  EXPECT_NE(the_set.end(), the_set.find(ptr1));
  EXPECT_EQ(the_set.end(), the_set.find(ptr2));
  EXPECT_TRUE(pdfium::Contains(the_set, ptr1));
  EXPECT_FALSE(pdfium::Contains(the_set, ptr2));
  EXPECT_EQ(5, obj1.retain_count());
  EXPECT_EQ(2, obj1.release_count());
  EXPECT_EQ(4, obj2.retain_count());
  EXPECT_EQ(2, obj2.release_count());
}

TEST(RetainPtr, VectorContains) {
  // Makes sure pdfium::Contains() works the same way with raw pointers and
  // RetainPtrs. for containers that use begin()/end().
  PseudoRetainable obj1;
  PseudoRetainable obj2;
  std::vector<const PseudoRetainable*> vec;
  vec.push_back(&obj1);
  EXPECT_TRUE(pdfium::Contains(vec, &obj1));
  EXPECT_FALSE(pdfium::Contains(vec, &obj2));

  RetainPtr<PseudoRetainable> ptr1(&obj1);
  RetainPtr<PseudoRetainable> ptr2(&obj2);
  EXPECT_TRUE(pdfium::Contains(vec, ptr1));
  EXPECT_FALSE(pdfium::Contains(vec, ptr2));

  RetainPtr<const PseudoRetainable> const_ptr1(&obj1);
  RetainPtr<const PseudoRetainable> const_ptr2(&obj2);
  EXPECT_TRUE(pdfium::Contains(vec, const_ptr1));
  EXPECT_FALSE(pdfium::Contains(vec, const_ptr2));
}

}  // namespace fxcrt
