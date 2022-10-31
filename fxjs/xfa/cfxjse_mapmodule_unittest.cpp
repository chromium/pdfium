// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cfxjse_mapmodule.h"

#include "core/fxcrt/fx_string.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"

TEST(CFXJSEMapModule, EmptyModule) {
  CFXJSE_MapModule module;
  EXPECT_FALSE(module.HasKey(1));
  EXPECT_FALSE(module.HasKey(2));
  EXPECT_FALSE(module.HasKey(3));
  EXPECT_FALSE(module.GetValue(1).has_value());
  EXPECT_FALSE(module.GetString(2).has_value());
  EXPECT_FALSE(module.GetMeasurement(3).has_value());
}

TEST(CFXJSEMapModule, InsertDelete) {
  const int value = 101;
  WideString str(L"foo");
  CXFA_Measurement measure(L"1 pt");
  CFXJSE_MapModule module;

  module.SetValue(100, value);
  module.SetString(200, str);
  module.SetMeasurement(300, measure);
  EXPECT_TRUE(module.HasKey(100));
  EXPECT_TRUE(module.HasKey(200));
  EXPECT_TRUE(module.HasKey(300));

  EXPECT_EQ(module.GetValue(100).value(), value);
  EXPECT_FALSE(module.GetString(100).has_value());
  EXPECT_FALSE(module.GetMeasurement(100).has_value());

  EXPECT_FALSE(module.GetValue(200).has_value());
  EXPECT_EQ(module.GetString(200).value(), str);
  EXPECT_FALSE(module.GetMeasurement(200).has_value());

  EXPECT_FALSE(module.GetValue(300).has_value());
  EXPECT_FALSE(module.GetString(300).has_value());
  EXPECT_EQ(module.GetMeasurement(300).value().GetUnit(), measure.GetUnit());
  EXPECT_EQ(module.GetMeasurement(300).value().GetValue(), measure.GetValue());

  module.RemoveKey(100);
  module.RemoveKey(200);
  module.RemoveKey(300);
  EXPECT_FALSE(module.HasKey(100));
  EXPECT_FALSE(module.HasKey(200));
  EXPECT_FALSE(module.HasKey(300));
  EXPECT_FALSE(module.GetValue(100).has_value());
  EXPECT_FALSE(module.GetString(200).has_value());
  EXPECT_FALSE(module.GetMeasurement(200).has_value());
}

TEST(CFXJSEMapModule, KeyCollision) {
  const int value = 37;
  WideString str(L"foo");
  CXFA_Measurement measure(L"1 pt");
  CFXJSE_MapModule module;

  module.SetValue(100, value);
  EXPECT_TRUE(module.HasKey(100));
  EXPECT_EQ(module.GetValue(100).value(), value);
  EXPECT_FALSE(module.GetString(100).has_value());
  EXPECT_FALSE(module.GetMeasurement(100).has_value());

  module.SetString(100, str);
  EXPECT_TRUE(module.HasKey(100));
  EXPECT_FALSE(module.GetValue(100).has_value());
  EXPECT_EQ(module.GetString(100).value(), str);
  EXPECT_FALSE(module.GetMeasurement(100).has_value());

  module.SetMeasurement(100, measure);
  EXPECT_FALSE(module.GetValue(100).has_value());
  EXPECT_FALSE(module.GetString(100).has_value());
  EXPECT_EQ(module.GetMeasurement(100).value().GetUnit(), measure.GetUnit());

  module.SetValue(100, value);
  EXPECT_TRUE(module.HasKey(100));
  EXPECT_EQ(module.GetValue(100).value(), value);
  EXPECT_FALSE(module.GetString(100).has_value());
  EXPECT_FALSE(module.GetMeasurement(100).has_value());
}

TEST(CFXJSEMapModule, MergeData) {
  const int value1 = 42;
  const int value2 = -1999;
  WideString string1(L"foo");
  WideString string2(L"foo");
  CXFA_Measurement measure1(L"1 pt");
  CXFA_Measurement measure2(L"2 mm");
  CFXJSE_MapModule module1;
  CFXJSE_MapModule module2;

  module1.SetValue(100, value1);
  module1.SetValue(101, value1);
  module1.SetString(200, string1);
  module1.SetString(201, string1);
  module1.SetMeasurement(300, measure1);
  module1.SetMeasurement(301, measure1);

  module2.SetString(100, string2);
  module2.SetMeasurement(200, measure2);
  module2.SetValue(300, value2);

  module1.MergeDataFrom(&module2);
  EXPECT_EQ(module1.GetString(100).value(), string2);
  EXPECT_EQ(module1.GetValue(101).value(), value1);
  EXPECT_EQ(module1.GetMeasurement(200).value().GetUnit(), measure2.GetUnit());
  EXPECT_EQ(module1.GetString(201).value(), string1);
  EXPECT_EQ(module1.GetValue(300).value(), value2);
  EXPECT_EQ(module1.GetMeasurement(301).value().GetUnit(), measure1.GetUnit());

  // module2 is undisturbed.
  EXPECT_EQ(module2.GetString(100).value(), string2);
  EXPECT_EQ(module2.GetMeasurement(200).value().GetUnit(), measure2.GetUnit());
  EXPECT_EQ(module2.GetValue(300).value(), value2);
}
