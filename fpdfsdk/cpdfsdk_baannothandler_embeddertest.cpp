// Copyright 2019 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fpdfsdk/cpdfsdk_annotiterator.h"
#include "fpdfsdk/cpdfsdk_baannothandler.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "public/fpdf_annot.h"
#include "testing/embedder_test.h"

class CPDFSDK_BAAnnotHandlerTest : public EmbedderTest {
 public:
  void SetUp() override {
    // Test behaviour with currently supported annot i.e. Widget.
    // TODO(crbug.com/994500): Add an API that can set list of focusable
    // subtypes once other annots(links & highlights) are also supported.
    EmbedderTest::SetUp();
    SetUpBAAnnotHandler();
  }

  void TearDown() override {
    UnloadPage(m_page);
    EmbedderTest::TearDown();
  }

  void SetUpBAAnnotHandler() {
    EXPECT_TRUE(OpenDocument("links_highlights_annots.pdf"));
    m_page = LoadPage(0);
    ASSERT_TRUE(m_page);

    CPDFSDK_FormFillEnvironment* pFormFillEnv =
        CPDFSDKFormFillEnvironmentFromFPDFFormHandle(form_handle());
    ASSERT_TRUE(pFormFillEnv);
    m_pPageView = pFormFillEnv->GetPageView(IPDFPageFromFPDFPage(m_page), true);
    ASSERT_TRUE(m_pPageView);

    CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr =
        pFormFillEnv->GetAnnotHandlerMgr();
    ASSERT_TRUE(pAnnotHandlerMgr);
    m_pBAAnnotHandler = pAnnotHandlerMgr->m_pBAAnnotHandler.get();
    ASSERT_TRUE(m_pBAAnnotHandler);
  }

  CPDFSDK_PageView* GetPageView() const { return m_pPageView; }
  CPDFSDK_BAAnnotHandler* GetBAAnnotHandler() const {
    return m_pBAAnnotHandler;
  }

 private:
  FPDF_PAGE m_page = nullptr;
  CPDFSDK_PageView* m_pPageView = nullptr;
  CPDFSDK_BAAnnotHandler* m_pBAAnnotHandler = nullptr;
};

TEST_F(CPDFSDK_BAAnnotHandlerTest, TabToLinkOrHighlightAnnot) {
  // TODO(crbug.com/994500): Create annot iterator with list of supported
  // focusable subtypes as provided by host.
  CPDFSDK_AnnotIterator ai(GetPageView(), CPDF_Annot::Subtype::LINK);
  CPDFSDK_Annot* pAnnot = ai.GetFirstAnnot();
  ASSERT_TRUE(pAnnot);
  EXPECT_EQ(pAnnot->GetAnnotSubtype(), CPDF_Annot::Subtype::LINK);

  ObservedPtr<CPDFSDK_Annot> pNonWidgetAnnot(pAnnot);

  // TODO(crbug.com/994500): Change expected value as true once
  // links & highlights are supported.
  EXPECT_FALSE(GetBAAnnotHandler()->OnSetFocus(&pNonWidgetAnnot, 0));

  EXPECT_FALSE(GetBAAnnotHandler()->OnKillFocus(&pNonWidgetAnnot, 0));
}
