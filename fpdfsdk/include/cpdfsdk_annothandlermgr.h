// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_CPDFSDK_ANNOTHANDLERMGR_H_
#define FPDFSDK_INCLUDE_CPDFSDK_ANNOTHANDLERMGR_H_

#include <map>
#include <memory>

#include "core/fpdfdoc/include/cpdf_annot.h"
#include "core/fxcrt/include/fx_basic.h"
#include "core/fxcrt/include/fx_coordinates.h"

class CFX_Matrix;
class CFX_RenderDevice;
class CPDFSDK_Environment;
class CPDFSDK_Annot;
class CPDFSDK_BAAnnotHandler;
class CPDFSDK_WidgetHandler;
class CPDFSDK_PageView;
class IPDFSDK_AnnotHandler;

#ifdef PDF_ENABLE_XFA
class CPDFSDK_XFAWidgetHandler;
class CXFA_FFWidget;
#endif  // PDF_ENABLE_XFA

class CPDFSDK_AnnotHandlerMgr {
 public:
  explicit CPDFSDK_AnnotHandlerMgr(CPDFSDK_Environment* pApp);
  virtual ~CPDFSDK_AnnotHandlerMgr();

  virtual CPDFSDK_Annot* NewAnnot(CPDF_Annot* pAnnot,
                                  CPDFSDK_PageView* pPageView);
#ifdef PDF_ENABLE_XFA
  virtual CPDFSDK_Annot* NewAnnot(CXFA_FFWidget* pAnnot,
                                  CPDFSDK_PageView* pPageView);
#endif  // PDF_ENABLE_XFA
  virtual void ReleaseAnnot(CPDFSDK_Annot* pAnnot);

  virtual void Annot_OnCreate(CPDFSDK_Annot* pAnnot);
  virtual void Annot_OnLoad(CPDFSDK_Annot* pAnnot);

  IPDFSDK_AnnotHandler* GetAnnotHandler(CPDFSDK_Annot* pAnnot) const;
  virtual void Annot_OnDraw(CPDFSDK_PageView* pPageView,
                            CPDFSDK_Annot* pAnnot,
                            CFX_RenderDevice* pDevice,
                            CFX_Matrix* pUser2Device,
                            bool bDrawAnnots);

  virtual void Annot_OnMouseEnter(CPDFSDK_PageView* pPageView,
                                  CPDFSDK_Annot* pAnnot,
                                  uint32_t nFlags);
  virtual void Annot_OnMouseExit(CPDFSDK_PageView* pPageView,
                                 CPDFSDK_Annot* pAnnot,
                                 uint32_t nFlags);
  virtual FX_BOOL Annot_OnLButtonDown(CPDFSDK_PageView* pPageView,
                                      CPDFSDK_Annot* pAnnot,
                                      uint32_t nFlags,
                                      const CFX_FloatPoint& point);
  virtual FX_BOOL Annot_OnLButtonUp(CPDFSDK_PageView* pPageView,
                                    CPDFSDK_Annot* pAnnot,
                                    uint32_t nFlags,
                                    const CFX_FloatPoint& point);
  virtual FX_BOOL Annot_OnLButtonDblClk(CPDFSDK_PageView* pPageView,
                                        CPDFSDK_Annot* pAnnot,
                                        uint32_t nFlags,
                                        const CFX_FloatPoint& point);
  virtual FX_BOOL Annot_OnMouseMove(CPDFSDK_PageView* pPageView,
                                    CPDFSDK_Annot* pAnnot,
                                    uint32_t nFlags,
                                    const CFX_FloatPoint& point);
  virtual FX_BOOL Annot_OnMouseWheel(CPDFSDK_PageView* pPageView,
                                     CPDFSDK_Annot* pAnnot,
                                     uint32_t nFlags,
                                     short zDelta,
                                     const CFX_FloatPoint& point);
  virtual FX_BOOL Annot_OnRButtonDown(CPDFSDK_PageView* pPageView,
                                      CPDFSDK_Annot* pAnnot,
                                      uint32_t nFlags,
                                      const CFX_FloatPoint& point);
  virtual FX_BOOL Annot_OnRButtonUp(CPDFSDK_PageView* pPageView,
                                    CPDFSDK_Annot* pAnnot,
                                    uint32_t nFlags,
                                    const CFX_FloatPoint& point);
  virtual FX_BOOL Annot_OnChar(CPDFSDK_Annot* pAnnot,
                               uint32_t nChar,
                               uint32_t nFlags);
  virtual FX_BOOL Annot_OnKeyDown(CPDFSDK_Annot* pAnnot,
                                  int nKeyCode,
                                  int nFlag);
  virtual FX_BOOL Annot_OnKeyUp(CPDFSDK_Annot* pAnnot, int nKeyCode, int nFlag);

  virtual FX_BOOL Annot_OnSetFocus(CPDFSDK_Annot* pAnnot, uint32_t nFlag);
  virtual FX_BOOL Annot_OnKillFocus(CPDFSDK_Annot* pAnnot, uint32_t nFlag);

#ifdef PDF_ENABLE_XFA
  virtual FX_BOOL Annot_OnChangeFocus(CPDFSDK_Annot* pSetAnnot,
                                      CPDFSDK_Annot* pKillAnnot);
#endif  // PDF_ENABLE_XFA

  virtual CFX_FloatRect Annot_OnGetViewBBox(CPDFSDK_PageView* pPageView,
                                            CPDFSDK_Annot* pAnnot);
  virtual FX_BOOL Annot_OnHitTest(CPDFSDK_PageView* pPageView,
                                  CPDFSDK_Annot* pAnnot,
                                  const CFX_FloatPoint& point);

 private:
  IPDFSDK_AnnotHandler* GetAnnotHandler(CPDF_Annot::Subtype nSubtype) const;
  CPDFSDK_Annot* GetNextAnnot(CPDFSDK_Annot* pSDKAnnot, FX_BOOL bNext);

  std::unique_ptr<CPDFSDK_BAAnnotHandler> m_pBAAnnotHandler;
  std::unique_ptr<CPDFSDK_WidgetHandler> m_pWidgetHandler;
#ifdef PDF_ENABLE_XFA
  std::unique_ptr<CPDFSDK_XFAWidgetHandler> m_pXFAWidgetHandler;
#endif  // PDF_ENABLE_XFA

  CPDFSDK_Environment* m_pApp;
};

#endif  // FPDFSDK_INCLUDE_CPDFSDK_ANNOTHANDLERMGR_H_
