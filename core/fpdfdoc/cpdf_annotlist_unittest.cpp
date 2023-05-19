// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfdoc/cpdf_annotlist.h"

#include <stdint.h>

#include <initializer_list>
#include <memory>

#include "constants/annotation_common.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/test_with_page_module.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/cpdf_test_document.h"
#include "core/fpdfdoc/cpdf_annot.h"
#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/widestring.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

class CPDFAnnotListTest : public TestWithPageModule {
 public:
  void SetUp() override {
    TestWithPageModule::SetUp();

    document_ = std::make_unique<CPDF_TestDocument>();
    document_->SetRoot(pdfium::MakeRetain<CPDF_Dictionary>());
    page_ = pdfium::MakeRetain<CPDF_Page>(
        document_.get(), pdfium::MakeRetain<CPDF_Dictionary>());
  }

  void TearDown() override {
    page_.Reset();
    document_.reset();

    TestWithPageModule::TearDown();
  }

 protected:
  void AddTextAnnotation(const ByteString& contents) {
    RetainPtr<CPDF_Dictionary> annotation =
        page_->GetOrCreateAnnotsArray()->AppendNew<CPDF_Dictionary>();
    annotation->SetNewFor<CPDF_Name>(pdfium::annotation::kSubtype, "Text");
    annotation->SetNewFor<CPDF_String>(pdfium::annotation::kContents, contents,
                                       /*bHex=*/false);
  }

  std::unique_ptr<CPDF_TestDocument> document_;
  RetainPtr<CPDF_Page> page_;
};

ByteString MakeByteString(std::initializer_list<uint8_t> bytes) {
  return ByteString(std::data(bytes), std::size(bytes));
}

ByteString GetRawContents(const CPDF_Annot* annotation) {
  return annotation->GetAnnotDict()->GetByteStringFor(
      pdfium::annotation::kContents);
}

WideString GetDecodedContents(const CPDF_Annot* annotation) {
  return annotation->GetAnnotDict()->GetUnicodeTextFor(
      pdfium::annotation::kContents);
}

}  // namespace

TEST_F(CPDFAnnotListTest, CreatePopupAnnotFromPdfEncoded) {
  const ByteString kContents = MakeByteString({'A', 'a', 0xE4, 0xA0});
  AddTextAnnotation(kContents);

  CPDF_AnnotList list(page_);

  ASSERT_EQ(2u, list.Count());
  EXPECT_EQ(kContents, GetRawContents(list.GetAt(1)));
  EXPECT_EQ(WideString::FromUTF8("Aaä€"), GetDecodedContents(list.GetAt(1)));
}

TEST_F(CPDFAnnotListTest, CreatePopupAnnotFromUnicode) {
  const ByteString kContents =
      MakeByteString({0xFE, 0xFF, 0x00, 'A', 0x00, 'a', 0x00, 0xE4, 0x20, 0xAC,
                      0xD8, 0x3C, 0xDF, 0xA8});
  AddTextAnnotation(kContents);

  CPDF_AnnotList list(page_);

  ASSERT_EQ(2u, list.Count());
  EXPECT_EQ(kContents, GetRawContents(list.GetAt(1)));

  EXPECT_EQ(WideString::FromUTF8("Aaä€🎨"), GetDecodedContents(list.GetAt(1)));
}

TEST_F(CPDFAnnotListTest, CreatePopupAnnotFromEmptyPdfEncoded) {
  AddTextAnnotation("");

  CPDF_AnnotList list(page_);

  EXPECT_EQ(1u, list.Count());
}

TEST_F(CPDFAnnotListTest, CreatePopupAnnotFromEmptyUnicode) {
  const ByteString kContents = MakeByteString({0xFE, 0xFF});
  AddTextAnnotation(kContents);

  CPDF_AnnotList list(page_);

  EXPECT_EQ(1u, list.Count());
}

TEST_F(CPDFAnnotListTest, CreatePopupAnnotFromEmptyUnicodedWithEscape) {
  const ByteString kContents =
      MakeByteString({0xFE, 0xFF, 0x00, 0x1B, 'j', 'a', 0x00, 0x1B});
  AddTextAnnotation(kContents);

  CPDF_AnnotList list(page_);

  EXPECT_EQ(1u, list.Count());
}
