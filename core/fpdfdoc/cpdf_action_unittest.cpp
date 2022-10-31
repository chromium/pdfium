// Copyright 2021 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfdoc/cpdf_action.h"

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fxcrt/retain_ptr.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

RetainPtr<CPDF_Dictionary> CreateActionDictWithType(
    const ByteString& action_type) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "Action");
  dict->SetNewFor<CPDF_Name>("S", action_type);
  return dict;
}

RetainPtr<CPDF_Dictionary> CreateActionDictWithoutType(
    const ByteString& action_type) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("S", action_type);
  return dict;
}

RetainPtr<CPDF_Dictionary> CreateActionDictWithInvalidType(
    const ByteString& action_type) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "Lights");
  dict->SetNewFor<CPDF_Name>("S", action_type);
  return dict;
}

RetainPtr<CPDF_Dictionary> CreateInvalidActionDictWithType(
    const ByteString& action_type) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "Action");
  dict->SetNewFor<CPDF_String>("S", action_type, /*is_hex=*/false);
  return dict;
}

RetainPtr<CPDF_Dictionary> CreateInvalidActionDictWithoutType(
    const ByteString& action_type) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_String>("S", action_type, /*is_hex=*/false);
  return dict;
}

}  // namespace

TEST(CPDFActionTest, GetType) {
  static constexpr struct {
    const char* action_type;
    CPDF_Action::Type expected_type;
  } kValidTestCases[] = {
      {"GoTo", CPDF_Action::Type::kGoTo},
      {"GoToR", CPDF_Action::Type::kGoToR},
      {"GoToE", CPDF_Action::Type::kGoToE},
      {"Launch", CPDF_Action::Type::kLaunch},
      {"Thread", CPDF_Action::Type::kThread},
      {"URI", CPDF_Action::Type::kURI},
      {"Sound", CPDF_Action::Type::kSound},
      {"Movie", CPDF_Action::Type::kMovie},
      {"Hide", CPDF_Action::Type::kHide},
      {"Named", CPDF_Action::Type::kNamed},
      {"SubmitForm", CPDF_Action::Type::kSubmitForm},
      {"ResetForm", CPDF_Action::Type::kResetForm},
      {"ImportData", CPDF_Action::Type::kImportData},
      {"JavaScript", CPDF_Action::Type::kJavaScript},
      {"SetOCGState", CPDF_Action::Type::kSetOCGState},
      {"Rendition", CPDF_Action::Type::kRendition},
      {"Trans", CPDF_Action::Type::kTrans},
      {"GoTo3DView", CPDF_Action::Type::kGoTo3DView},
  };

  // Test correctly constructed actions.
  for (const auto& test_case : kValidTestCases) {
    {
      // Type is present.
      CPDF_Action action(CreateActionDictWithType(test_case.action_type));
      EXPECT_EQ(test_case.expected_type, action.GetType());
    }
    {
      // Type is optional, so omitting it is ok.
      CPDF_Action action(CreateActionDictWithoutType(test_case.action_type));
      EXPECT_EQ(test_case.expected_type, action.GetType());
    }
  }

  // Test incorrectly constructed actions.
  for (const auto& test_case : kValidTestCases) {
    {
      // Type is optional, but must be valid if present.
      CPDF_Action action(
          CreateActionDictWithInvalidType(test_case.action_type));
      EXPECT_EQ(CPDF_Action::Type::kUnknown, action.GetType());
    }
    {
      // The action type (/S) must be a name.
      CPDF_Action action(
          CreateInvalidActionDictWithType(test_case.action_type));
      EXPECT_EQ(CPDF_Action::Type::kUnknown, action.GetType());
    }
    {
      // The action type (/S) must be a name.
      CPDF_Action action(
          CreateInvalidActionDictWithoutType(test_case.action_type));
      EXPECT_EQ(CPDF_Action::Type::kUnknown, action.GetType());
    }
  }

  static constexpr const char* kInvalidTestCases[] = {
      "Camera",
      "Javascript",
      "Unknown",
  };

  // Test invalid actions.
  for (const char* test_case : kInvalidTestCases) {
    CPDF_Action action(CreateActionDictWithType(test_case));
    EXPECT_EQ(CPDF_Action::Type::kUnknown, action.GetType());
  }
}
