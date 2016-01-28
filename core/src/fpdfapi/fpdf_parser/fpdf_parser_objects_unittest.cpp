// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/include/fpdfapi/fpdf_objects.h"

#include <memory>
#include <vector>

#include "core/include/fxcrt/fx_basic.h"
#include "testing/gtest/include/gtest/gtest.h"

class PDFObjectsTest : public testing::Test {
 public:
  void SetUp() override {
    // Initialize different kinds of objects.
    // Boolean objects.
    CPDF_Boolean* boolean_false_obj = new CPDF_Boolean(false);
    CPDF_Boolean* boolean_true_obj = new CPDF_Boolean(true);
    // Number objects.
    CPDF_Number* number_int_obj = new CPDF_Number(1245);
    CPDF_Number* number_float_obj = new CPDF_Number(9.00345f);
    // String objects.
    CPDF_String* str_reg_obj = new CPDF_String(L"A simple test");
    CPDF_String* str_spec_obj = new CPDF_String(L"\t\n");
    // Name object.
    CPDF_Name* name_obj = new CPDF_Name("space");
    // Array object.
    CPDF_Array* array_obj = new CPDF_Array;
    array_obj->InsertAt(0, new CPDF_Number(8902));
    array_obj->InsertAt(1, new CPDF_Name("address"));
    // Dictionary object.
    m_DictObj = new CPDF_Dictionary;
    m_DictObj->SetAt("bool", new CPDF_Boolean(false));
    m_DictObj->SetAt("num", new CPDF_Number(0.23f));
    // Stream object.
    const char content[] = "abcdefghijklmnopqrstuvwxyz";
    size_t buf_len = FX_ArraySize(content);
    uint8_t* buf = reinterpret_cast<uint8_t*>(malloc(buf_len));
    memcpy(buf, content, buf_len);
    m_StreamDictObj = new CPDF_Dictionary;
    m_StreamDictObj->SetAt("key1", new CPDF_String(L" test dict"));
    m_StreamDictObj->SetAt("key2", new CPDF_Number(-1));
    CPDF_Stream* stream_obj = new CPDF_Stream(buf, buf_len, m_StreamDictObj);
    // Null Object.
    CPDF_Null* null_obj = new CPDF_Null;
    // All direct objects.
    CPDF_Object* objs[] = {boolean_false_obj, boolean_true_obj, number_int_obj,
                           number_float_obj,  str_reg_obj,      str_spec_obj,
                           name_obj,          array_obj,        m_DictObj,
                           stream_obj,        null_obj};
    for (int i = 0; i < FX_ArraySize(objs); ++i)
      m_DirectObjs.emplace_back(objs[i]);

    // Indirect references to indirect objects.
    m_ObjHolder.reset(new CPDF_IndirectObjectHolder(nullptr));
    CPDF_Object* referred_objs[] = {
        boolean_true_obj, number_int_obj, str_spec_obj, name_obj,
        array_obj,        m_DictObj,      stream_obj};
    for (int i = 0; i < FX_ArraySize(referred_objs); ++i) {
      m_ObjHolder->AddIndirectObject(referred_objs[i]);
      m_RefObjs.emplace_back(
          new CPDF_Reference(m_ObjHolder.get(), referred_objs[i]->GetObjNum()));
    }
  }

 protected:
  using ScopedObj = std::unique_ptr<CPDF_Object, ReleaseDeleter<CPDF_Object>>;

  // m_ObjHolder needs to be declared first and destructed last since it also
  // refers to some objects in m_DirectObjs.
  std::unique_ptr<CPDF_IndirectObjectHolder> m_ObjHolder;
  std::vector<ScopedObj> m_DirectObjs;
  std::vector<ScopedObj> m_RefObjs;
  CPDF_Dictionary* m_DictObj;
  CPDF_Dictionary* m_StreamDictObj;
};

TEST_F(PDFObjectsTest, GetString) {
  const char* direct_obj_results[] = {
      "false", "true", "1245", "9.00345", "A simple test", "\t\n", "space",
      "",      "",     "",     ""};
  // Check for direct objects.
  for (int i = 0; i < m_DirectObjs.size(); ++i)
    EXPECT_STREQ(m_DirectObjs[i]->GetString().c_str(), direct_obj_results[i]);

  // Check indirect references.
  const char* indirect_obj_results[] = {"true", "1245", "\t\n", "space",
                                        "",     "",     ""};
  for (int i = 0; i < m_RefObjs.size(); ++i) {
    EXPECT_STREQ(m_RefObjs[i]->GetString().c_str(), indirect_obj_results[i]);
  }
}

TEST_F(PDFObjectsTest, GetConstString) {
  const char* direct_obj_results[] = {
      nullptr, nullptr, nullptr, nullptr, "A simple test", "\t\n",
      "space", nullptr, nullptr, nullptr, nullptr};
  // Check for direct objects.
  for (int i = 0; i < m_DirectObjs.size(); ++i) {
    if (!direct_obj_results[i]) {
      EXPECT_EQ(m_DirectObjs[i]->GetConstString().GetCStr(),
                direct_obj_results[i]);
    } else {
      EXPECT_STREQ(m_DirectObjs[i]->GetConstString().GetCStr(),
                   direct_obj_results[i]);
    }
  }
  // Check indirect references.
  const char* indirect_obj_results[] = {nullptr, nullptr, "\t\n", "space",
                                        nullptr, nullptr, nullptr};
  for (int i = 0; i < m_RefObjs.size(); ++i) {
    if (!indirect_obj_results[i])
      EXPECT_EQ(m_RefObjs[i]->GetConstString().GetCStr(), nullptr);
    else {
      EXPECT_STREQ(m_RefObjs[i]->GetConstString().GetCStr(),
                   indirect_obj_results[i]);
    }
  }
}

TEST_F(PDFObjectsTest, GetNumber) {
  const FX_FLOAT direct_obj_results[] = {0, 0, 1245, 9.00345f, 0, 0,
                                         0, 0, 0,    0,        0};
  // Check for direct objects.
  for (int i = 0; i < m_DirectObjs.size(); ++i)
    EXPECT_EQ(m_DirectObjs[i]->GetNumber(), direct_obj_results[i]);

  // Check indirect references.
  const FX_FLOAT indirect_obj_results[] = {0, 1245, 0, 0, 0, 0, 0};
  for (int i = 0; i < m_RefObjs.size(); ++i)
    EXPECT_EQ(m_RefObjs[i]->GetNumber(), indirect_obj_results[i]);
}

TEST_F(PDFObjectsTest, GetInteger) {
  const int direct_obj_results[] = {0, 1, 1245, 9, 0, 0, 0, 0, 0, 0, 0};
  // Check for direct objects.
  for (int i = 0; i < m_DirectObjs.size(); ++i)
    EXPECT_EQ(m_DirectObjs[i]->GetInteger(), direct_obj_results[i]);

  // Check indirect references.
  const int indirect_obj_results[] = {1, 1245, 0, 0, 0, 0, 0};
  for (int i = 0; i < m_RefObjs.size(); ++i)
    EXPECT_EQ(m_RefObjs[i]->GetInteger(), indirect_obj_results[i]);
}

TEST_F(PDFObjectsTest, GetDict) {
  const CPDF_Dictionary* direct_obj_results[] = {
      nullptr, nullptr, nullptr,   nullptr,         nullptr, nullptr,
      nullptr, nullptr, m_DictObj, m_StreamDictObj, nullptr};
  // Check for direct objects.
  for (int i = 0; i < m_DirectObjs.size(); ++i)
    EXPECT_EQ(m_DirectObjs[i]->GetDict(), direct_obj_results[i]);

  // Check indirect references.
  const CPDF_Dictionary* indirect_obj_results[] = {
      nullptr, nullptr, nullptr, nullptr, nullptr, m_DictObj, m_StreamDictObj};
  for (int i = 0; i < m_RefObjs.size(); ++i)
    EXPECT_EQ(m_RefObjs[i]->GetDict(), indirect_obj_results[i]);
}
