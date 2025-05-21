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

  ScopedPage SetUpBAAnnot() {
    ScopedPage page = LoadScopedPage(0);
    if (!page) {
      ADD_FAILURE();
      return ScopedPage();
    }
    form_fill_env_ =
        CPDFSDKFormFillEnvironmentFromFPDFFormHandle(form_handle());
    if (!form_fill_env_) {
      ADD_FAILURE();
      return ScopedPage();
    }

    page_view_ =
        form_fill_env_->GetOrCreatePageView(IPDFPageFromFPDFPage(page.get()));

    if (!page_view_) {
      ADD_FAILURE();
      return ScopedPage();
    }

    return page;
  }

  CPDFSDK_FormFillEnvironment* GetFormFillEnv() const { return form_fill_env_; }
  CPDFSDK_PageView* GetPageView() const { return page_view_; }

  CPDFSDK_Annot* GetNthFocusableAnnot(size_t n) {
    DCHECK_NE(n, 0);
    CPDFSDK_AnnotIterator ai(GetPageView(),
                             form_fill_env_->GetFocusableAnnotSubtypes());
    CPDFSDK_Annot* pAnnot = ai.GetFirstAnnot();
    DCHECK(pAnnot);

    for (size_t i = 1; i < n; i++) {
      pAnnot = ai.GetNextAnnot(pAnnot);
      DCHECK(pAnnot);
    }

    return pAnnot;
  }

 private:
  CPDFSDK_PageView* page_view_ = nullptr;
  CPDFSDK_FormFillEnvironment* form_fill_env_ = nullptr;
};

TEST_F(CPDFSDKBAAnnotTest, TabToLinkOrHighlightAnnot) {
  ScopedPage page = SetUpBAAnnot();
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
