// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_CPDFSDK_ANNOT_H_
#define FPDFSDK_INCLUDE_CPDFSDK_ANNOT_H_

#include "core/fpdfdoc/include/cpdf_aaction.h"
#include "core/fpdfdoc/include/cpdf_annot.h"
#include "core/fpdfdoc/include/cpdf_defaultappearance.h"
#include "core/fxcrt/include/cfx_observable.h"
#include "core/fxcrt/include/fx_basic.h"
#include "fpdfsdk/cfx_systemhandler.h"
#include "fpdfsdk/include/fsdk_common.h"
#include "fpdfsdk/include/fsdk_define.h"

class CFX_Matrix;
class CFX_RenderDevice;
class CPDF_Page;
class CPDF_RenderOptions;
class CPDFSDK_PageView;

class CPDFSDK_Annot : public CFX_Observable<CPDFSDK_Annot> {
 public:
  explicit CPDFSDK_Annot(CPDFSDK_PageView* pPageView);
  virtual ~CPDFSDK_Annot();

#ifdef PDF_ENABLE_XFA
  virtual FX_BOOL IsXFAField();
  virtual CXFA_FFWidget* GetXFAWidget() const;
#endif  // PDF_ENABLE_XFA

  virtual FX_FLOAT GetMinWidth() const;
  virtual FX_FLOAT GetMinHeight() const;
  virtual int GetLayoutOrder() const;
  virtual CPDF_Annot* GetPDFAnnot() const;
  virtual CPDF_Annot::Subtype GetAnnotSubtype() const;
  virtual bool IsSignatureWidget() const;
  virtual CFX_FloatRect GetRect() const;

  virtual void SetRect(const CFX_FloatRect& rect);
  virtual void Annot_OnDraw(CFX_RenderDevice* pDevice,
                            CFX_Matrix* pUser2Device,
                            CPDF_RenderOptions* pOptions);

  UnderlyingPageType* GetUnderlyingPage();
  CPDF_Page* GetPDFPage();
#ifdef PDF_ENABLE_XFA
  CPDFXFA_Page* GetPDFXFAPage();
#endif  // PDF_ENABLE_XFA

  void SetPage(CPDFSDK_PageView* pPageView);
  CPDFSDK_PageView* GetPageView() const { return m_pPageView; }

  FX_BOOL IsSelected();
  void SetSelected(FX_BOOL bSelected);

 protected:
  CPDFSDK_PageView* m_pPageView;
  FX_BOOL m_bSelected;
};

#endif  // FPDFSDK_INCLUDE_CPDFSDK_ANNOT_H_
