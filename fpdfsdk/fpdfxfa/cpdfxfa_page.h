// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_FPDFXFA_CPDFXFA_PAGE_H_
#define FPDFSDK_FPDFXFA_CPDFXFA_PAGE_H_

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/ipdf_page.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "third_party/base/optional.h"

class CFX_RenderDevice;
class CPDF_Dictionary;
class CPDF_Document;
class CPDFSDK_Annot;
class CXFA_FFPageView;

class CPDFXFA_Page final : public IPDF_Page {
 public:
  template <typename T, typename... Args>
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  // IPDF_Page:
  CPDF_Page* AsPDFPage() override;
  CPDFXFA_Page* AsXFAPage() override;
  CPDF_Document* GetDocument() const override;
  float GetPageWidth() const override;
  float GetPageHeight() const override;
  CFX_Matrix GetDisplayMatrix(const FX_RECT& rect, int iRotate) const override;
  Optional<CFX_PointF> DeviceToPage(
      const FX_RECT& rect,
      int rotate,
      const CFX_PointF& device_point) const override;
  Optional<CFX_PointF> PageToDevice(
      const FX_RECT& rect,
      int rotate,
      const CFX_PointF& page_point) const override;

  bool LoadPage();
  void LoadPDFPageFromDict(CPDF_Dictionary* pPageDict);
  int GetPageIndex() const { return m_iPageIndex; }
  void SetXFAPageViewIndex(int index) { m_iPageIndex = index; }
  CXFA_FFPageView* GetXFAPageView() const;
  CPDFSDK_Annot* GetNextXFAAnnot(CPDFSDK_Annot* pSDKAnnot, bool bNext);
  int HasFormFieldAtPoint(const CFX_PointF& point) const;
  void DrawFocusAnnot(CFX_RenderDevice* pDevice,
                      CPDFSDK_Annot* pAnnot,
                      const CFX_Matrix& mtUser2Device,
                      const FX_RECT& rtClip);

 private:
  // Refcounted class.
  CPDFXFA_Page(CPDF_Document* pDocument, int page_index);
  ~CPDFXFA_Page() override;

  bool LoadPDFPage();

  RetainPtr<CPDF_Page> m_pPDFPage;  // Backing page, if any.
  UnownedPtr<CPDF_Document> const m_pDocument;
  int m_iPageIndex;
};

#endif  // FPDFSDK_FPDFXFA_CPDFXFA_PAGE_H_
