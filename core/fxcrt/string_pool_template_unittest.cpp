// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/string_pool_template.h"
#include "core/fxcrt/fx_string.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace fxcrt {

TEST(StringPool, ByteString) {
  ByteStringPool pool;

  ByteString null1;
  ByteString null2;
  ByteString goats1("goats");
  ByteString goats2("goats");

  // Underlying storage, if non-null, is not shared.
  EXPECT_FALSE(null1.data_.Get());
  EXPECT_FALSE(null2.data_.Get());
  EXPECT_NE(goats1.data_, goats2.data_);

  ByteString interned_null1 = pool.Intern(null1);
  ByteString interned_null2 = pool.Intern(null2);
  ByteString interned_goats1 = pool.Intern(goats1);
  ByteString interned_goats2 = pool.Intern(goats2);

  // Strings are logically equal after being interned.
  EXPECT_EQ(null1, interned_null1);
  EXPECT_EQ(null2, interned_null2);
  EXPECT_EQ(goats1, interned_goats1);
  EXPECT_EQ(goats2, interned_goats2);

  // Interned underlying storage, if non-null, belongs to first seen.
  EXPECT_FALSE(interned_null1.data_.Get());
  EXPECT_FALSE(interned_null2.data_.Get());
  EXPECT_EQ(goats1.data_, interned_goats1.data_);
  EXPECT_EQ(goats1.data_, interned_goats2.data_);

  pool.Clear();
  ByteString reinterned_null2 = pool.Intern(null2);
  ByteString reinterned_null1 = pool.Intern(null2);
  ByteString reinterned_goats2 = pool.Intern(goats2);
  ByteString reinterned_goats1 = pool.Intern(goats2);

  // After clearing pool, storage was re-interned using second strings.
  EXPECT_FALSE(interned_null1.data_.Get());
  EXPECT_FALSE(interned_null2.data_.Get());
  EXPECT_EQ(goats2.data_, reinterned_goats1.data_);
  EXPECT_EQ(goats2.data_, reinterned_goats2.data_);
}

TEST(StringPool, WideString) {
  WideStringPool pool;

  WideString null1;
  WideString null2;
  WideString goats1(L"goats");
  WideString goats2(L"goats");

  // Underlying storage, if non-null, is not shared.
  EXPECT_FALSE(null1.data_.Get());
  EXPECT_FALSE(null2.data_.Get());
  EXPECT_NE(goats1.data_, goats2.data_);

  WideString interned_null1 = pool.Intern(null1);
  WideString interned_null2 = pool.Intern(null2);
  WideString interned_goats1 = pool.Intern(goats1);
  WideString interned_goats2 = pool.Intern(goats2);

  // Strings are logically equal after being interned.
  EXPECT_EQ(null1, interned_null1);
  EXPECT_EQ(null2, interned_null2);
  EXPECT_EQ(goats1, interned_goats1);
  EXPECT_EQ(goats2, interned_goats2);

  // Interned underlying storage, if non-null, belongs to first seen.
  EXPECT_FALSE(interned_null1.data_.Get());
  EXPECT_FALSE(interned_null2.data_.Get());
  EXPECT_EQ(goats1.data_, interned_goats1.data_);
  EXPECT_EQ(goats1.data_, interned_goats2.data_);

  pool.Clear();
  WideString reinterned_null2 = pool.Intern(null2);
  WideString reinterned_null1 = pool.Intern(null2);
  WideString reinterned_goats2 = pool.Intern(goats2);
  WideString reinterned_goats1 = pool.Intern(goats2);

  // After clearing pool, storage was re-interned using second strings.
  EXPECT_FALSE(interned_null1.data_.Get());
  EXPECT_FALSE(interned_null2.data_.Get());
  EXPECT_EQ(goats2.data_, reinterned_goats1.data_);
  EXPECT_EQ(goats2.data_, reinterned_goats2.data_);
}

}  // namespace fxcrt
