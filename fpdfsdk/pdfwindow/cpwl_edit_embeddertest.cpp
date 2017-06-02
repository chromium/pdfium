// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fpdfsdk/cba_annotiterator.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/formfiller/cffl_formfiller.h"
#include "fpdfsdk/formfiller/cffl_interactiveformfiller.h"
#include "fpdfsdk/pdfwindow/cpwl_wnd.h"
#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class CPWLEditEmbeddertest : public EmbedderTest {};

TEST_F(CPWLEditEmbeddertest, TypeText) {
  EXPECT_TRUE(OpenDocument("text_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  CPDFSDK_FormFillEnvironment* pFormFillEnv =
      static_cast<CPDFSDK_FormFillEnvironment*>(form_handle());

  CPDFSDK_Annot* pAnnot;
  {
    CBA_AnnotIterator iter(pFormFillEnv->GetPageView(0),
                           CPDF_Annot::Subtype::WIDGET);
    pAnnot = iter.GetFirstAnnot();
    CPDFSDK_Annot* pLastAnnot = iter.GetLastAnnot();
    ASSERT_EQ(pAnnot, pLastAnnot);
    ASSERT_TRUE(pAnnot);
    ASSERT_EQ(CPDF_Annot::Subtype::WIDGET, pAnnot->GetAnnotSubtype());
  }

  CFFL_InteractiveFormFiller* pInteractiveFormFiller =
      pFormFillEnv->GetInteractiveFormFiller();
  {
    CPDFSDK_Annot::ObservedPtr pObserved(pAnnot);
    EXPECT_TRUE(pInteractiveFormFiller->OnSetFocus(&pObserved, 0));
  }

  CFFL_FormFiller* pFormFiller =
      pInteractiveFormFiller->GetFormFiller(pAnnot, false);
  ASSERT_TRUE(pFormFiller);

  CPWL_Wnd* pWindow =
      pFormFiller->GetPDFWindow(pFormFillEnv->GetPageView(0), false);
  ASSERT_TRUE(pWindow);
  ASSERT_EQ(PWL_CLASSNAME_EDIT, pWindow->GetClassName());

  CPWL_Edit* pEdit = static_cast<CPWL_Edit*>(pWindow);
  EXPECT_TRUE(pEdit->GetText().IsEmpty());

  EXPECT_TRUE(pFormFiller->OnChar(pAnnot, 'a', 0));
  EXPECT_TRUE(pFormFiller->OnChar(pAnnot, 'b', 0));
  EXPECT_TRUE(pFormFiller->OnChar(pAnnot, 'c', 0));

  EXPECT_STREQ(L"abc", pEdit->GetText().c_str());

  UnloadPage(page);
}
