// Copyright 2024 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfdoc/cpdf_interactiveform.h"

#include <memory>

#include "core/fpdfapi/page/test_with_page_module.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/cpdf_test_document.h"
#include "core/fxcrt/string_view_template.h"
#include "testing/gtest/include/gtest/gtest.h"

using CPDFInteractiveFormTest = TestWithPageModule;

TEST_F(CPDFInteractiveFormTest, LoadFieldsWithReferencedNames) {
  auto doc = std::make_unique<CPDF_TestDocument>();
  doc->CreateNewDoc();
  RetainPtr<CPDF_Dictionary> root = doc->GetMutableRoot();
  ASSERT_TRUE(root);

  // For use in the field dictionaries below in the /T field. The field type
  // should be string.
  auto good_string = doc->NewIndirect<CPDF_String>("good_string");
  auto bad_name = doc->NewIndirect<CPDF_Name>("bad_name");

  auto bad_stream = doc->NewIndirect<CPDF_Stream>(doc->New<CPDF_Dictionary>());
  bad_stream->SetData(ByteStringView("bad_stream").unsigned_span());

  auto acroform_dict = root->SetNewFor<CPDF_Dictionary>("AcroForm");
  auto fields_array = acroform_dict->SetNewFor<CPDF_Array>("Fields");

  auto good_string_field_dict = fields_array->AppendNew<CPDF_Dictionary>();
  good_string_field_dict->SetNewFor<CPDF_Name>("Type", "Annot");
  good_string_field_dict->SetNewFor<CPDF_Name>("Subtype", "Widget");
  good_string_field_dict->SetNewFor<CPDF_Name>("FT", "Btn");
  good_string_field_dict->SetNewFor<CPDF_Reference>("T", doc.get(),
                                                    good_string->GetObjNum());

  auto bad_name_field_dict = fields_array->AppendNew<CPDF_Dictionary>();
  bad_name_field_dict->SetNewFor<CPDF_Name>("Type", "Annot");
  bad_name_field_dict->SetNewFor<CPDF_Name>("Subtype", "Widget");
  bad_name_field_dict->SetNewFor<CPDF_Name>("FT", "Btn");
  bad_name_field_dict->SetNewFor<CPDF_Reference>("T", doc.get(),
                                                 bad_name->GetObjNum());

  auto bad_stream_field_dict = fields_array->AppendNew<CPDF_Dictionary>();
  bad_stream_field_dict->SetNewFor<CPDF_Name>("Type", "Annot");
  bad_stream_field_dict->SetNewFor<CPDF_Name>("Subtype", "Widget");
  bad_stream_field_dict->SetNewFor<CPDF_Name>("FT", "Btn");
  bad_stream_field_dict->SetNewFor<CPDF_Reference>("T", doc.get(),
                                                   bad_stream->GetObjNum());

  // Let `interactive_form` parse the dictionaries above and fix them up.
  CPDF_InteractiveForm interactive_form(doc.get());

  auto good_string_field_t = good_string_field_dict->GetStringFor("T");
  ASSERT_TRUE(good_string_field_t);
  EXPECT_EQ("good_string", good_string_field_t->GetString());

  auto bad_name_field_t = bad_name_field_dict->GetStringFor("T");
  ASSERT_TRUE(bad_name_field_t);
  EXPECT_TRUE(bad_name_field_t->GetString().IsEmpty());

  auto bad_stream_field_t = bad_stream_field_dict->GetStringFor("T");
  ASSERT_TRUE(bad_stream_field_t);
  EXPECT_TRUE(bad_stream_field_t->GetString().IsEmpty());
}
