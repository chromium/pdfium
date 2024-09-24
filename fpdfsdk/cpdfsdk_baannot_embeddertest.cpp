// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "fpdfsdk/cpdfsdk_annotiterator.h"
#include "fpdfsdk/cpdfsdk_baannot.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "testing/embedder_test.h"

class CPDFSDKBAAnnotTest : public EmbedderTest {
 public:
  void SetUp() override {
    EmbedderTest::SetUp();
    ASSERT_TRUE(OpenDocument("links_highlights_annots.pdf"));
  }

  ScopedEmbedderTestPage SetUpBAAnnot() {
    ScopedEmbedderTestPage page = LoadScopedPage(0);
    if (!page) {
      ADD_FAILURE();
      return ScopedEmbedderTestPage();
    }
    m_pFormFillEnv =
        CPDFSDKFormFillEnvironmentFromFPDFFormHandle(form_handle());
    if (!m_pFormFillEnv) {
      ADD_FAILURE();
      return ScopedEmbedderTestPage();
    }

    m_pPageView =
        m_pFormFillEnv->GetOrCreatePageView(IPDFPageFromFPDFPage(page.get()));

    if (!m_pPageView) {
      ADD_FAILURE();
      return ScopedEmbedderTestPage();
    }

    return page;
  }

  CPDFSDK_FormFillEnvironment* GetFormFillEnv() const { return m_pFormFillEnv; }
  CPDFSDK_PageView* GetPageView() const { return m_pPageView; }

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
  CPDFSDK_PageView* m_pPageView = nullptr;
  CPDFSDK_FormFillEnvironment* m_pFormFillEnv = nullptr;
};

TEST_F(CPDFSDKBAAnnotTest, TabToLinkOrHighlightAnnot) {
  ScopedEmbedderTestPage page = SetUpBAAnnot();
  ASSERT_TRUE(page);

  std::vector<CPDF_Annot::Subtype> focusable_annot_types = {
      CPDF_Annot::Subtype::WIDGET, CPDF_Annot::Subtype::LINK,
      CPDF_Annot::Subtype::HIGHLIGHT};

  GetFormFillEnv()->SetFocusableAnnotSubtypes(focusable_annot_types);

  // Get link annot.
  CPDFSDK_Annot* pAnnot = GetNthFocusableAnnot(2);
  ASSERT_TRUE(pAnnot);
  EXPECT_EQ(pAnnot->GetAnnotSubtype(), CPDF_Annot::Subtype::LINK);

  {
    ObservedPtr<CPDFSDK_Annot> observer(pAnnot);
    EXPECT_TRUE(CPDFSDK_Annot::OnSetFocus(observer, {}));
    EXPECT_TRUE(CPDFSDK_Annot::OnKillFocus(observer, {}));
  }

  // Get highlight annot.
  pAnnot = GetNthFocusableAnnot(4);
  ASSERT_TRUE(pAnnot);
  EXPECT_EQ(pAnnot->GetAnnotSubtype(), CPDF_Annot::Subtype::HIGHLIGHT);

  {
    ObservedPtr<CPDFSDK_Annot> observer(pAnnot);
    EXPECT_TRUE(CPDFSDK_Annot::OnSetFocus(observer, {}));
    EXPECT_TRUE(CPDFSDK_Annot::OnKillFocus(observer, {}));
  }
}
