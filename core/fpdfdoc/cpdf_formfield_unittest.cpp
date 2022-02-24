// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfdoc/cpdf_formfield.h"

#include <vector>

#include "constants/form_fields.h"
#include "constants/form_flags.h"
#include "core/fpdfapi/page/cpdf_pagemodule.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_indirect_object_holder.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/cpdf_test_document.h"
#include "core/fpdfdoc/cpdf_interactiveform.h"
#include "core/fxcrt/fx_memory.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/containers/contains.h"

namespace {

// Create and destroys the page module that is necessary when instantiating a
// CPDF_Document.
class ScopedCPDF_PageModule {
 public:
  FX_STACK_ALLOCATED();

  ScopedCPDF_PageModule() { CPDF_PageModule::Create(); }
  ~ScopedCPDF_PageModule() { CPDF_PageModule::Destroy(); }
};

void TestMultiselectFieldDict(RetainPtr<CPDF_Array> opt_array,
                              RetainPtr<CPDF_Object> values,
                              RetainPtr<CPDF_Object> selected_indices,
                              bool expected_use_indices,
                              const std::vector<int>& expected_indices,
                              const std::vector<int>& excluded_indices) {
  auto form_dict = pdfium::MakeRetain<CPDF_Dictionary>();
  form_dict->SetNewFor<CPDF_Name>("Type", "Annot");
  form_dict->SetNewFor<CPDF_Name>("Subtype", "Widget");
  form_dict->SetNewFor<CPDF_Name>(pdfium::form_fields::kFT,
                                  pdfium::form_fields::kCh);
  constexpr int kMuliSelectFlag = pdfium::form_flags::kChoiceMultiSelect;
  form_dict->SetNewFor<CPDF_Number>(pdfium::form_fields::kFf, kMuliSelectFlag);
  form_dict->SetFor("Opt", opt_array);
  form_dict->SetFor(pdfium::form_fields::kV, values);
  form_dict->SetFor("I", selected_indices);

  CPDF_TestDocument doc;
  CPDF_InteractiveForm form(&doc);
  CPDF_FormField form_field(&form, form_dict.Get());
  EXPECT_EQ(expected_use_indices, form_field.UseSelectedIndicesObject());
  for (int i = 0; i < form_field.CountOptions(); i++) {
    const bool expected_selected = pdfium::Contains(expected_indices, i);
    EXPECT_EQ(expected_selected, form_field.IsItemSelected(i));
  }
  for (int i : excluded_indices) {
    EXPECT_FALSE(form_field.IsItemSelected(i));
  }
}

}  // namespace

TEST(CPDF_FormFieldTest, GetFullNameForDict) {
  WideString name = CPDF_FormField::GetFullNameForDict(nullptr);
  EXPECT_TRUE(name.IsEmpty());

  CPDF_IndirectObjectHolder obj_holder;
  CPDF_Dictionary* root = obj_holder.NewIndirect<CPDF_Dictionary>();
  root->SetNewFor<CPDF_Name>("T", "foo");
  name = CPDF_FormField::GetFullNameForDict(root);
  EXPECT_STREQ("foo", name.ToUTF8().c_str());

  CPDF_Dictionary* dict1 = obj_holder.NewIndirect<CPDF_Dictionary>();
  root->SetNewFor<CPDF_Reference>("Parent", &obj_holder, dict1->GetObjNum());
  dict1->SetNewFor<CPDF_Name>("T", "bar");
  name = CPDF_FormField::GetFullNameForDict(root);
  EXPECT_STREQ("bar.foo", name.ToUTF8().c_str());

  CPDF_Dictionary* dict2 = dict1->SetNewFor<CPDF_Dictionary>("Parent");
  name = CPDF_FormField::GetFullNameForDict(root);
  EXPECT_STREQ("bar.foo", name.ToUTF8().c_str());

  CPDF_Dictionary* dict3 = obj_holder.NewIndirect<CPDF_Dictionary>();
  dict2->SetNewFor<CPDF_Reference>("Parent", &obj_holder, dict3->GetObjNum());

  dict3->SetNewFor<CPDF_Name>("T", "qux");
  name = CPDF_FormField::GetFullNameForDict(root);
  EXPECT_STREQ("qux.bar.foo", name.ToUTF8().c_str());

  dict3->SetNewFor<CPDF_Reference>("Parent", &obj_holder, root->GetObjNum());
  name = CPDF_FormField::GetFullNameForDict(root);
  EXPECT_STREQ("qux.bar.foo", name.ToUTF8().c_str());
  name = CPDF_FormField::GetFullNameForDict(dict1);
  EXPECT_STREQ("foo.qux.bar", name.ToUTF8().c_str());
  name = CPDF_FormField::GetFullNameForDict(dict2);
  EXPECT_STREQ("bar.foo.qux", name.ToUTF8().c_str());
  name = CPDF_FormField::GetFullNameForDict(dict3);
  EXPECT_STREQ("bar.foo.qux", name.ToUTF8().c_str());
}

TEST(CPDF_FormFieldTest, IsItemSelected) {
  ScopedCPDF_PageModule page_module;

  auto opt_array = pdfium::MakeRetain<CPDF_Array>();
  opt_array->AppendNew<CPDF_String>(L"Alpha");
  opt_array->AppendNew<CPDF_String>(L"Beta");
  opt_array->AppendNew<CPDF_String>(L"Gamma");
  opt_array->AppendNew<CPDF_String>(L"Delta");
  opt_array->AppendNew<CPDF_String>(L"Epsilon");

  {
    // No Values (/V) or Selected Indices (/I) objects.
    std::vector<int> expected_indices;
    std::vector<int> excluded_indices{-1, 5};
    TestMultiselectFieldDict(opt_array, /*values=*/nullptr,
                             /*selected_indices=*/nullptr,
                             /*expected_use_indices=*/false, expected_indices,
                             excluded_indices);
  }
  {
    // Values (/V) object is just a string.
    auto values = pdfium::MakeRetain<CPDF_String>(/*pPool=*/nullptr, L"Gamma");
    std::vector<int> expected_indices{2};
    std::vector<int> excluded_indices{-1, 5};
    TestMultiselectFieldDict(opt_array, values, /*selected_indices=*/nullptr,
                             /*expected_use_indices=*/false, expected_indices,
                             excluded_indices);
  }
  {
    // Values (/V) object is just an invalid string.
    auto values = pdfium::MakeRetain<CPDF_String>(/*pPool=*/nullptr, L"Omega");
    std::vector<int> expected_indices;
    std::vector<int> excluded_indices{-1, 5};
    TestMultiselectFieldDict(opt_array, values, /*selected_indices=*/nullptr,
                             /*expected_use_indices=*/false, expected_indices,
                             excluded_indices);
  }
  {
    // Values (/V) object is an array with one object.
    auto values = pdfium::MakeRetain<CPDF_Array>();
    values->AppendNew<CPDF_String>(L"Beta");
    std::vector<int> expected_indices{1};
    std::vector<int> excluded_indices{-1, 5};
    TestMultiselectFieldDict(opt_array, values, /*selected_indices=*/nullptr,
                             /*expected_use_indices=*/false, expected_indices,
                             excluded_indices);
  }
  {
    // Values (/V) object is an array with one invalid object.
    auto values = pdfium::MakeRetain<CPDF_Array>();
    values->AppendNew<CPDF_String>(L"Omega");
    std::vector<int> expected_indices;
    std::vector<int> excluded_indices{-1, 5};
    TestMultiselectFieldDict(opt_array, values, /*selected_indices=*/nullptr,
                             /*expected_use_indices=*/false, expected_indices,
                             excluded_indices);
  }
  {
    // Values (/V) object is an array with multiple objects.
    auto values = pdfium::MakeRetain<CPDF_Array>();
    values->AppendNew<CPDF_String>(L"Beta");
    values->AppendNew<CPDF_String>(L"Epsilon");
    std::vector<int> expected_indices{1, 4};
    std::vector<int> excluded_indices{-1, 5};
    TestMultiselectFieldDict(opt_array, values, /*selected_indices=*/nullptr,
                             /*expected_use_indices=*/false, expected_indices,
                             excluded_indices);
  }
  {
    // Values (/V) object is an array with multiple objects with one invalid.
    auto values = pdfium::MakeRetain<CPDF_Array>();
    values->AppendNew<CPDF_String>(L"Beta");
    values->AppendNew<CPDF_String>(L"Epsilon");
    values->AppendNew<CPDF_String>(L"Omega");
    std::vector<int> expected_indices{1, 4};
    std::vector<int> excluded_indices{-1, 5};
    TestMultiselectFieldDict(opt_array, values, /*selected_indices=*/nullptr,
                             /*expected_use_indices=*/false, expected_indices,
                             excluded_indices);
  }
  {
    // Selected indices (/I) object is just a number.
    auto selected_indices = pdfium::MakeRetain<CPDF_Number>(3);
    std::vector<int> expected_indices{3};
    std::vector<int> excluded_indices{-1, 5};
    TestMultiselectFieldDict(opt_array, /*values=*/nullptr, selected_indices,
                             /*expected_use_indices=*/true, expected_indices,
                             excluded_indices);
  }
  {
    // Selected indices (/I) object is just an invalid number.
    auto selected_indices = pdfium::MakeRetain<CPDF_Number>(26);
    std::vector<int> expected_indices;
    std::vector<int> excluded_indices{-1, 5, 26};
    TestMultiselectFieldDict(opt_array, /*values=*/nullptr, selected_indices,
                             /*expected_use_indices=*/true, expected_indices,
                             excluded_indices);
  }
  {
    // Selected indices (/I) object is an array with one object.
    auto selected_indices = pdfium::MakeRetain<CPDF_Array>();
    selected_indices->AppendNew<CPDF_Number>(0);
    std::vector<int> expected_indices{0};
    std::vector<int> excluded_indices{-1, 5};
    TestMultiselectFieldDict(opt_array, /*values=*/nullptr, selected_indices,
                             /*expected_use_indices=*/true, expected_indices,
                             excluded_indices);
  }
  {
    // Selected indices (/I) object is an array with multiple objects.
    auto selected_indices = pdfium::MakeRetain<CPDF_Array>();
    selected_indices->AppendNew<CPDF_Number>(0);
    selected_indices->AppendNew<CPDF_Number>(2);
    selected_indices->AppendNew<CPDF_Number>(3);
    std::vector<int> expected_indices{0, 2, 3};
    std::vector<int> excluded_indices{-1, 5};
    TestMultiselectFieldDict(opt_array, /*values=*/nullptr, selected_indices,
                             /*expected_use_indices=*/true, expected_indices,
                             excluded_indices);
  }
  {
    // Selected indices (/I) object is an array with multiple objects and some
    // are invalid.
    auto selected_indices = pdfium::MakeRetain<CPDF_Array>();
    selected_indices->AppendNew<CPDF_Number>(0);
    selected_indices->AppendNew<CPDF_Number>(2);
    selected_indices->AppendNew<CPDF_Number>(3);
    selected_indices->AppendNew<CPDF_Number>(-5);
    selected_indices->AppendNew<CPDF_Number>(12);
    selected_indices->AppendNew<CPDF_Number>(42);
    std::vector<int> expected_indices{0, 2, 3};
    std::vector<int> excluded_indices{-5, -1, 5, 12, 42};
    TestMultiselectFieldDict(opt_array, /*values=*/nullptr, selected_indices,
                             /*expected_use_indices=*/true, expected_indices,
                             excluded_indices);
  }
  {
    // Values (/V) or Selected Indices (/I) objects conflict with different
    // lengths.
    auto values = pdfium::MakeRetain<CPDF_Array>();
    values->AppendNew<CPDF_String>(L"Beta");
    values->AppendNew<CPDF_String>(L"Epsilon");
    auto selected_indices = pdfium::MakeRetain<CPDF_Array>();
    selected_indices->AppendNew<CPDF_Number>(0);
    selected_indices->AppendNew<CPDF_Number>(2);
    selected_indices->AppendNew<CPDF_Number>(3);
    std::vector<int> expected_indices{1, 4};
    std::vector<int> excluded_indices{-1, 5};
    TestMultiselectFieldDict(opt_array, values, selected_indices,
                             /*expected_use_indices=*/false, expected_indices,
                             excluded_indices);
  }
  {
    // Values (/V) or Selected Indices (/I) objects conflict with same lengths.
    auto values = pdfium::MakeRetain<CPDF_Array>();
    values->AppendNew<CPDF_String>(L"Alpha");
    values->AppendNew<CPDF_String>(L"Epsilon");
    auto selected_indices = pdfium::MakeRetain<CPDF_Array>();
    selected_indices->AppendNew<CPDF_Number>(2);
    selected_indices->AppendNew<CPDF_Number>(3);
    std::vector<int> expected_indices{0, 4};
    std::vector<int> excluded_indices{-1, 5};
    TestMultiselectFieldDict(opt_array, values, selected_indices,
                             /*expected_use_indices=*/false, expected_indices,
                             excluded_indices);
  }
  {
    // Values (/V) or Selected Indices (/I) objects conflict with values being
    // invalid.
    auto values = pdfium::MakeRetain<CPDF_Array>();
    values->AppendNew<CPDF_String>(L"Beta");
    values->AppendNew<CPDF_String>(L"Epsilon");
    values->AppendNew<CPDF_String>(L"Omega");
    auto selected_indices = pdfium::MakeRetain<CPDF_Array>();
    selected_indices->AppendNew<CPDF_Number>(1);
    selected_indices->AppendNew<CPDF_Number>(4);
    std::vector<int> expected_indices{1, 4};
    std::vector<int> excluded_indices{-1, 5};
    TestMultiselectFieldDict(opt_array, values, selected_indices,
                             /*expected_use_indices=*/false, expected_indices,
                             excluded_indices);
  }
  {
    // Values (/V) or Selected Indices (/I) objects conflict with selected
    // indices being invalid.
    auto values = pdfium::MakeRetain<CPDF_Array>();
    values->AppendNew<CPDF_String>(L"Beta");
    values->AppendNew<CPDF_String>(L"Epsilon");
    auto selected_indices = pdfium::MakeRetain<CPDF_Array>();
    selected_indices->AppendNew<CPDF_Number>(1);
    selected_indices->AppendNew<CPDF_Number>(4);
    selected_indices->AppendNew<CPDF_Number>(26);
    std::vector<int> expected_indices{1, 4};
    std::vector<int> excluded_indices{-1, 5, 26};
    TestMultiselectFieldDict(opt_array, values, selected_indices,
                             /*expected_use_indices=*/false, expected_indices,
                             excluded_indices);
  }
  {
    // Values (/V) or Selected Indices (/I) objects conflict with both being
    // invalid.
    auto values = pdfium::MakeRetain<CPDF_Array>();
    values->AppendNew<CPDF_String>(L"Beta");
    values->AppendNew<CPDF_String>(L"Epsilon");
    values->AppendNew<CPDF_String>(L"Omega");
    auto selected_indices = pdfium::MakeRetain<CPDF_Array>();
    selected_indices->AppendNew<CPDF_Number>(0);
    selected_indices->AppendNew<CPDF_Number>(2);
    selected_indices->AppendNew<CPDF_Number>(3);
    selected_indices->AppendNew<CPDF_Number>(26);
    std::vector<int> expected_indices{1, 4};
    std::vector<int> excluded_indices{-1, 5, 26};
    TestMultiselectFieldDict(opt_array, values, selected_indices,
                             /*expected_use_indices=*/false, expected_indices,
                             excluded_indices);
  }
  {
    // Values (/V) or Selected Indices (/I) objects conflict with each not being
    // an array.
    auto values = pdfium::MakeRetain<CPDF_String>(/*pPool=*/nullptr, L"Gamma");
    auto selected_indices = pdfium::MakeRetain<CPDF_Number>(4);
    std::vector<int> expected_indices{2};
    std::vector<int> excluded_indices{-1, 5};
    TestMultiselectFieldDict(opt_array, values, selected_indices,
                             /*expected_use_indices=*/false, expected_indices,
                             excluded_indices);
  }
}
