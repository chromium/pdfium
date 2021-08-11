// Copyright 2019 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "fpdfsdk/cpdfsdk_annotiterator.h"
#include "fpdfsdk/cpdfsdk_baannothandler.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "testing/embedder_test.h"
#include "third_party/base/check.h"
#include "third_party/base/check_op.h"

class CPDFSDK_BAAnnotHandlerTest : public EmbedderTest {
 public:
  void SetUp() override {
    EmbedderTest::SetUp();
    SetUpBAAnnotHandler();
  }

  void TearDown() override {
    UnloadPage(m_page);
    EmbedderTest::TearDown();
  }

  void SetUpBAAnnotHandler() {
    ASSERT_TRUE(OpenDocument("links_highlights_annots.pdf"));
    m_page = LoadPage(0);
    ASSERT_TRUE(m_page);

    m_pFormFillEnv =
        CPDFSDKFormFillEnvironmentFromFPDFFormHandle(form_handle());
    ASSERT_TRUE(m_pFormFillEnv);
    m_pPageView =
        m_pFormFillEnv->GetOrCreatePageView(IPDFPageFromFPDFPage(m_page));
    ASSERT_TRUE(m_pPageView);

    CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr =
        m_pFormFillEnv->GetAnnotHandlerMgr();
    ASSERT_TRUE(pAnnotHandlerMgr);
    m_pBAAnnotHandler = pAnnotHandlerMgr->m_pBAAnnotHandler.get();
    ASSERT_TRUE(m_pBAAnnotHandler);
  }

  CPDFSDK_FormFillEnvironment* GetFormFillEnv() const { return m_pFormFillEnv; }
  CPDFSDK_PageView* GetPageView() const { return m_pPageView; }
  CPDFSDK_BAAnnotHandler* GetBAAnnotHandler() const {
    return m_pBAAnnotHandler;
  }

  CPDFSDK_Annot* GetNthFocusableAnnot(size_t n) {
    DCHECK_NE(n, 0);
    CPDFSDK_AnnotIterator ai(GetPageView(),
                             m_pFormFillEnv->GetFocusableAnnotSubtypes());
    CPDFSDK_Annot* pAnnot = ai.GetFirstAnnot();
    DCHECK(pAnnot);

    for (size_t i = 1; i < n; i++) {
      pAnnot = ai.GetNextAnnot(pAnnot);
      DCHECK(pAnnot);
    }

    return pAnnot;
  }

 private:
  FPDF_PAGE m_page = nullptr;
  CPDFSDK_PageView* m_pPageView = nullptr;
  CPDFSDK_FormFillEnvironment* m_pFormFillEnv = nullptr;
  CPDFSDK_BAAnnotHandler* m_pBAAnnotHandler = nullptr;
};

TEST_F(CPDFSDK_BAAnnotHandlerTest, TabToLinkOrHighlightAnnot) {
  std::vector<CPDF_Annot::Subtype> focusable_annot_types = {
      CPDF_Annot::Subtype::WIDGET, CPDF_Annot::Subtype::LINK,
      CPDF_Annot::Subtype::HIGHLIGHT};

  GetFormFillEnv()->SetFocusableAnnotSubtypes(focusable_annot_types);

  // Get link annot.
  CPDFSDK_Annot* pAnnot = GetNthFocusableAnnot(2);
  ASSERT_TRUE(pAnnot);
  EXPECT_EQ(pAnnot->GetAnnotSubtype(), CPDF_Annot::Subtype::LINK);

  ObservedPtr<CPDFSDK_Annot> pLinkAnnot(pAnnot);
  EXPECT_TRUE(GetBAAnnotHandler()->OnSetFocus(&pLinkAnnot, {}));
  EXPECT_TRUE(GetBAAnnotHandler()->OnKillFocus(&pLinkAnnot, {}));

  // Get highlight annot.
  pAnnot = GetNthFocusableAnnot(4);
  ASSERT_TRUE(pAnnot);
  EXPECT_EQ(pAnnot->GetAnnotSubtype(), CPDF_Annot::Subtype::HIGHLIGHT);

  ObservedPtr<CPDFSDK_Annot> pHighlightAnnot(pAnnot);
  EXPECT_TRUE(GetBAAnnotHandler()->OnSetFocus(&pHighlightAnnot, {}));
  EXPECT_TRUE(GetBAAnnotHandler()->OnKillFocus(&pHighlightAnnot, {}));
}
