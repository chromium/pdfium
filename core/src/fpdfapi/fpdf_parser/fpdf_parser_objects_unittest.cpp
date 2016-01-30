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
    m_ArrayObj = new CPDF_Array;
    m_ArrayObj->InsertAt(0, new CPDF_Number(8902));
    m_ArrayObj->InsertAt(1, new CPDF_Name("address"));
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
                           name_obj,          m_ArrayObj,       m_DictObj,
                           stream_obj,        null_obj};
    m_DirectObjTypes = {
        CPDF_Object::BOOLEAN, CPDF_Object::BOOLEAN, CPDF_Object::NUMBER,
        CPDF_Object::NUMBER,  CPDF_Object::STRING,  CPDF_Object::STRING,
        CPDF_Object::NAME,    CPDF_Object::ARRAY,   CPDF_Object::DICTIONARY,
        CPDF_Object::STREAM,  CPDF_Object::NULLOBJ};
    for (size_t i = 0; i < FX_ArraySize(objs); ++i)
      m_DirectObjs.emplace_back(objs[i]);

    // Indirect references to indirect objects.
    m_ObjHolder.reset(new CPDF_IndirectObjectHolder(nullptr));
    m_IndirectObjs = {boolean_true_obj, number_int_obj, str_spec_obj, name_obj,
                      m_ArrayObj,       m_DictObj,      stream_obj};
    for (size_t i = 0; i < m_IndirectObjs.size(); ++i) {
      m_ObjHolder->AddIndirectObject(m_IndirectObjs[i]);
      m_RefObjs.emplace_back(new CPDF_Reference(
          m_ObjHolder.get(), m_IndirectObjs[i]->GetObjNum()));
    }
  }

  bool Equal(CPDF_Object* obj1, CPDF_Object* obj2) {
    if (obj1 == obj2)
      return true;
    if (!obj1 || !obj2 || obj1->GetType() != obj2->GetType())
      return false;
    switch (obj1->GetType()) {
      case CPDF_Object::BOOLEAN:
        return obj1->GetInteger() == obj2->GetInteger();
      case CPDF_Object::NUMBER:
        return obj1->AsNumber()->IsInteger() == obj2->AsNumber()->IsInteger() &&
               obj1->GetInteger() == obj2->GetInteger();
      case CPDF_Object::STRING:
      case CPDF_Object::NAME:
        return obj1->GetString() == obj2->GetString();
      case CPDF_Object::ARRAY: {
        const CPDF_Array* array1 = obj1->AsArray();
        const CPDF_Array* array2 = obj2->AsArray();
        if (array1->GetCount() != array2->GetCount())
          return false;
        for (size_t i = 0; i < array1->GetCount(); ++i) {
          if (!Equal(array1->GetElement(i), array2->GetElement(i)))
            return false;
        }
        return true;
      }
      case CPDF_Object::DICTIONARY: {
        const CPDF_Dictionary* dict1 = obj1->AsDictionary();
        const CPDF_Dictionary* dict2 = obj2->AsDictionary();
        if (dict1->GetCount() != dict2->GetCount())
          return false;
        for (CPDF_Dictionary::const_iterator it = dict1->begin();
             it != dict1->end(); ++it) {
          if (!Equal(it->second, dict2->GetElement(it->first)))
            return false;
        }
        return true;
      }
      case CPDF_Object::NULLOBJ:
        return true;
      case CPDF_Object::STREAM: {
        const CPDF_Stream* stream1 = obj1->AsStream();
        const CPDF_Stream* stream2 = obj2->AsStream();
        if (!stream1->GetDict() && !stream2->GetDict())
          return true;
        // Compare dictionaries.
        if (!Equal(stream1->GetDict(), stream2->GetDict()))
          return false;
        // Compare sizes.
        if (stream1->GetRawSize() != stream2->GetRawSize())
          return false;
        // Compare contents.
        // Since this function is used for testing Clone(), only memory based
        // streams need to be handled.
        if (!stream1->IsMemoryBased() || !stream2->IsMemoryBased())
          return false;
        return FXSYS_memcmp(stream1->GetRawData(), stream2->GetRawData(),
                            stream1->GetRawSize()) == 0;
      }
      case CPDF_Object::REFERENCE:
        return obj1->AsReference()->GetRefObjNum() ==
               obj2->AsReference()->GetRefObjNum();
    }
    return false;
  }

 protected:
  using ScopedObj = std::unique_ptr<CPDF_Object, ReleaseDeleter<CPDF_Object>>;

  // m_ObjHolder needs to be declared first and destructed last since it also
  // refers to some objects in m_DirectObjs.
  std::unique_ptr<CPDF_IndirectObjectHolder> m_ObjHolder;
  std::vector<ScopedObj> m_DirectObjs;
  std::vector<int> m_DirectObjTypes;
  std::vector<ScopedObj> m_RefObjs;
  CPDF_Dictionary* m_DictObj;
  CPDF_Dictionary* m_StreamDictObj;
  CPDF_Array* m_ArrayObj;
  std::vector<CPDF_Object*> m_IndirectObjs;
};

TEST_F(PDFObjectsTest, GetString) {
  const char* direct_obj_results[] = {
      "false", "true", "1245", "9.00345", "A simple test", "\t\n", "space",
      "",      "",     "",     ""};
  // Check for direct objects.
  for (size_t i = 0; i < m_DirectObjs.size(); ++i)
    EXPECT_STREQ(m_DirectObjs[i]->GetString().c_str(), direct_obj_results[i]);

  // Check indirect references.
  const char* indirect_obj_results[] = {"true", "1245", "\t\n", "space",
                                        "",     "",     ""};
  for (size_t i = 0; i < m_RefObjs.size(); ++i) {
    EXPECT_STREQ(m_RefObjs[i]->GetString().c_str(), indirect_obj_results[i]);
  }
}

TEST_F(PDFObjectsTest, GetConstString) {
  const char* direct_obj_results[] = {
      nullptr, nullptr, nullptr, nullptr, "A simple test", "\t\n",
      "space", nullptr, nullptr, nullptr, nullptr};
  // Check for direct objects.
  for (size_t i = 0; i < m_DirectObjs.size(); ++i) {
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
  for (size_t i = 0; i < m_RefObjs.size(); ++i) {
    if (!indirect_obj_results[i]) {
      EXPECT_EQ(m_RefObjs[i]->GetConstString().GetCStr(), nullptr);
    } else {
      EXPECT_STREQ(m_RefObjs[i]->GetConstString().GetCStr(),
                   indirect_obj_results[i]);
    }
  }
}

TEST_F(PDFObjectsTest, GetUnicodeText) {
  const wchar_t* direct_obj_results[] = {
      L"",     L"",      L"", L"", L"A simple test",
      L"\t\n", L"space", L"", L"", L"abcdefghijklmnopqrstuvwxyz",
      L""};
  // Check for direct objects.
  for (size_t i = 0; i < m_DirectObjs.size(); ++i)
    EXPECT_STREQ(m_DirectObjs[i]->GetUnicodeText().c_str(),
                 direct_obj_results[i]);

  // Check indirect references.
  for (const auto& it : m_RefObjs)
    EXPECT_STREQ(it->GetUnicodeText().c_str(), L"");
}

TEST_F(PDFObjectsTest, GetNumber) {
  const FX_FLOAT direct_obj_results[] = {0, 0, 1245, 9.00345f, 0, 0,
                                         0, 0, 0,    0,        0};
  // Check for direct objects.
  for (size_t i = 0; i < m_DirectObjs.size(); ++i)
    EXPECT_EQ(m_DirectObjs[i]->GetNumber(), direct_obj_results[i]);

  // Check indirect references.
  const FX_FLOAT indirect_obj_results[] = {0, 1245, 0, 0, 0, 0, 0};
  for (size_t i = 0; i < m_RefObjs.size(); ++i)
    EXPECT_EQ(m_RefObjs[i]->GetNumber(), indirect_obj_results[i]);
}

TEST_F(PDFObjectsTest, GetInteger) {
  const int direct_obj_results[] = {0, 1, 1245, 9, 0, 0, 0, 0, 0, 0, 0};
  // Check for direct objects.
  for (size_t i = 0; i < m_DirectObjs.size(); ++i)
    EXPECT_EQ(m_DirectObjs[i]->GetInteger(), direct_obj_results[i]);

  // Check indirect references.
  const int indirect_obj_results[] = {1, 1245, 0, 0, 0, 0, 0};
  for (size_t i = 0; i < m_RefObjs.size(); ++i)
    EXPECT_EQ(m_RefObjs[i]->GetInteger(), indirect_obj_results[i]);
}

TEST_F(PDFObjectsTest, GetDict) {
  const CPDF_Dictionary* direct_obj_results[] = {
      nullptr, nullptr, nullptr,   nullptr,         nullptr, nullptr,
      nullptr, nullptr, m_DictObj, m_StreamDictObj, nullptr};
  // Check for direct objects.
  for (size_t i = 0; i < m_DirectObjs.size(); ++i)
    EXPECT_EQ(m_DirectObjs[i]->GetDict(), direct_obj_results[i]);

  // Check indirect references.
  const CPDF_Dictionary* indirect_obj_results[] = {
      nullptr, nullptr, nullptr, nullptr, nullptr, m_DictObj, m_StreamDictObj};
  for (size_t i = 0; i < m_RefObjs.size(); ++i)
    EXPECT_EQ(m_RefObjs[i]->GetDict(), indirect_obj_results[i]);
}

TEST_F(PDFObjectsTest, GetArray) {
  const CPDF_Array* direct_obj_results[] = {
      nullptr, nullptr,    nullptr, nullptr, nullptr, nullptr,
      nullptr, m_ArrayObj, nullptr, nullptr, nullptr};
  // Check for direct objects.
  for (size_t i = 0; i < m_DirectObjs.size(); ++i)
    EXPECT_EQ(m_DirectObjs[i]->GetArray(), direct_obj_results[i]);

  // Check indirect references.
  for (const auto& it : m_RefObjs)
    EXPECT_EQ(it->GetArray(), nullptr);
}

TEST_F(PDFObjectsTest, Clone) {
  // Check for direct objects.
  for (size_t i = 0; i < m_DirectObjs.size(); ++i) {
    ScopedObj obj(m_DirectObjs[i]->Clone());
    EXPECT_TRUE(Equal(m_DirectObjs[i].get(), obj.get()));
  }

  // Check indirect references.
  for (const auto& it : m_RefObjs) {
    ScopedObj obj(it->Clone());
    EXPECT_TRUE(Equal(it.get(), obj.get()));
  }
}

TEST_F(PDFObjectsTest, GetType) {
  // Check for direct objects.
  for (size_t i = 0; i < m_DirectObjs.size(); ++i)
    EXPECT_EQ(m_DirectObjs[i]->GetType(), m_DirectObjTypes[i]);

  // Check indirect references.
  for (const auto& it : m_RefObjs)
    EXPECT_EQ(it->GetType(), CPDF_Object::REFERENCE);
}

TEST_F(PDFObjectsTest, GetDirect) {
  // Check for direct objects.
  for (size_t i = 0; i < m_DirectObjs.size(); ++i)
    EXPECT_EQ(m_DirectObjs[i]->GetDirect(), m_DirectObjs[i].get());

  // Check indirect references.
  for (size_t i = 0; i < m_RefObjs.size(); ++i)
    EXPECT_EQ(m_RefObjs[i]->GetDirect(), m_IndirectObjs[i]);
}

TEST_F(PDFObjectsTest, SetString) {
  // Check for direct objects.
  const char* const set_values[] = {"true",    "fake", "3.125f", "097",
                                    "changed", "",     "NewName"};
  const char* expected[] = {"true",    "false", "3.125",  "97",
                            "changed", "",      "NewName"};
  for (size_t i = 0; i < FX_ArraySize(set_values); ++i) {
    m_DirectObjs[i]->SetString(set_values[i]);
    EXPECT_STREQ(m_DirectObjs[i]->GetString().c_str(), expected[i]);
  }
}

TEST_F(PDFObjectsTest, IsTypeAndAsType) {
  // Check for direct objects.
  for (size_t i = 0; i < m_DirectObjs.size(); ++i) {
    if (m_DirectObjTypes[i] == CPDF_Object::ARRAY) {
      EXPECT_TRUE(m_DirectObjs[i]->IsArray());
      EXPECT_EQ(m_DirectObjs[i]->AsArray(), m_DirectObjs[i].get());
    } else {
      EXPECT_FALSE(m_DirectObjs[i]->IsArray());
      EXPECT_EQ(m_DirectObjs[i]->AsArray(), nullptr);
    }

    if (m_DirectObjTypes[i] == CPDF_Object::BOOLEAN) {
      EXPECT_TRUE(m_DirectObjs[i]->IsBoolean());
      EXPECT_EQ(m_DirectObjs[i]->AsBoolean(), m_DirectObjs[i].get());
    } else {
      EXPECT_FALSE(m_DirectObjs[i]->IsBoolean());
      EXPECT_EQ(m_DirectObjs[i]->AsBoolean(), nullptr);
    }

    if (m_DirectObjTypes[i] == CPDF_Object::NAME) {
      EXPECT_TRUE(m_DirectObjs[i]->IsName());
      EXPECT_EQ(m_DirectObjs[i]->AsName(), m_DirectObjs[i].get());
    } else {
      EXPECT_FALSE(m_DirectObjs[i]->IsName());
      EXPECT_EQ(m_DirectObjs[i]->AsName(), nullptr);
    }

    if (m_DirectObjTypes[i] == CPDF_Object::NUMBER) {
      EXPECT_TRUE(m_DirectObjs[i]->IsNumber());
      EXPECT_EQ(m_DirectObjs[i]->AsNumber(), m_DirectObjs[i].get());
    } else {
      EXPECT_FALSE(m_DirectObjs[i]->IsNumber());
      EXPECT_EQ(m_DirectObjs[i]->AsNumber(), nullptr);
    }

    if (m_DirectObjTypes[i] == CPDF_Object::STRING) {
      EXPECT_TRUE(m_DirectObjs[i]->IsString());
      EXPECT_EQ(m_DirectObjs[i]->AsString(), m_DirectObjs[i].get());
    } else {
      EXPECT_FALSE(m_DirectObjs[i]->IsString());
      EXPECT_EQ(m_DirectObjs[i]->AsString(), nullptr);
    }

    if (m_DirectObjTypes[i] == CPDF_Object::DICTIONARY) {
      EXPECT_TRUE(m_DirectObjs[i]->IsDictionary());
      EXPECT_EQ(m_DirectObjs[i]->AsDictionary(), m_DirectObjs[i].get());
    } else {
      EXPECT_FALSE(m_DirectObjs[i]->IsDictionary());
      EXPECT_EQ(m_DirectObjs[i]->AsDictionary(), nullptr);
    }

    if (m_DirectObjTypes[i] == CPDF_Object::STREAM) {
      EXPECT_TRUE(m_DirectObjs[i]->IsStream());
      EXPECT_EQ(m_DirectObjs[i]->AsStream(), m_DirectObjs[i].get());
    } else {
      EXPECT_FALSE(m_DirectObjs[i]->IsStream());
      EXPECT_EQ(m_DirectObjs[i]->AsStream(), nullptr);
    }

    EXPECT_FALSE(m_DirectObjs[i]->IsReference());
    EXPECT_EQ(m_DirectObjs[i]->AsReference(), nullptr);
  }
  // Check indirect references.
  for (size_t i = 0; i < m_RefObjs.size(); ++i) {
    EXPECT_TRUE(m_RefObjs[i]->IsReference());
    EXPECT_EQ(m_RefObjs[i]->AsReference(), m_RefObjs[i].get());
  }
}
