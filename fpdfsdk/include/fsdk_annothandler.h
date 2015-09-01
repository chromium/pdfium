// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FSDK_ANNOTHANDLER_H_
#define FPDFSDK_INCLUDE_FSDK_ANNOTHANDLER_H_

#include <map>

#include "../../core/include/fxcrt/fx_basic.h"

class CFFL_IFormFiller;
class CFX_RenderDevice;
class CPDFDoc_Environment;
class CPDFSDK_Annot;
class CPDFSDK_PageView;
class CPDF_Annot;
class CPDF_Matrix;
class CPDF_Point;
class CPDF_Rect;

class CPDFSDK_AnnotHandler {
 public:
  explicit CPDFSDK_AnnotHandler(CPDFDoc_Environment* pApp)
      : m_pApp(pApp), m_pFormFiller(nullptr) {}

  ~CPDFSDK_AnnotHandler() {}

  CFX_ByteString GetType() { return CFX_ByteString("Widget"); }
  CFX_ByteString GetName() { return CFX_ByteString("WidgetHandler"); }
  FX_BOOL CanAnswer(CPDFSDK_Annot* pAnnot);
  CPDFSDK_Annot* NewAnnot(CPDF_Annot* pAnnot, CPDFSDK_PageView* pPage);
  void ReleaseAnnot(CPDFSDK_Annot* pAnnot);
  void DeleteAnnot(CPDFSDK_Annot* pAnnot) {}
  CPDF_Rect GetViewBBox(CPDFSDK_PageView* pPageView, CPDFSDK_Annot* pAnnot);
  FX_BOOL HitTest(CPDFSDK_PageView* pPageView,
                  CPDFSDK_Annot* pAnnot,
                  const CPDF_Point& point);
  void OnDraw(CPDFSDK_PageView* pPageView,
              CPDFSDK_Annot* pAnnot,
              CFX_RenderDevice* pDevice,
              CPDF_Matrix* pUser2Device,
              FX_DWORD dwFlags);
  void OnDrawSleep(CPDFSDK_PageView* pPageView,
                   CPDFSDK_Annot* pAnnot,
                   CFX_RenderDevice* pDevice,
                   CPDF_Matrix* pUser2Device,
                   const CPDF_Rect& rcWindow,
                   FX_DWORD dwFlags) {}
  void OnCreate(CPDFSDK_Annot* pAnnot);
  void OnLoad(CPDFSDK_Annot* pAnnot);
  void OnDelete(CPDFSDK_Annot* pAnnot) {}
  void OnRelease(CPDFSDK_Annot* pAnnot) {}
  void OnMouseEnter(CPDFSDK_PageView* pPageView,
                    CPDFSDK_Annot* pAnnot,
                    FX_DWORD nFlag);
  void OnMouseExit(CPDFSDK_PageView* pPageView,
                   CPDFSDK_Annot* pAnnot,
                   FX_DWORD nFlag);
  FX_BOOL OnLButtonDown(CPDFSDK_PageView* pPageView,
                        CPDFSDK_Annot* pAnnot,
                        FX_DWORD nFlags,
                        const CPDF_Point& point);
  FX_BOOL OnLButtonUp(CPDFSDK_PageView* pPageView,
                      CPDFSDK_Annot* pAnnot,
                      FX_DWORD nFlags,
                      const CPDF_Point& point);
  FX_BOOL OnLButtonDblClk(CPDFSDK_PageView* pPageView,
                          CPDFSDK_Annot* pAnnot,
                          FX_DWORD nFlags,
                          const CPDF_Point& point);
  FX_BOOL OnMouseMove(CPDFSDK_PageView* pPageView,
                      CPDFSDK_Annot* pAnnot,
                      FX_DWORD nFlags,
                      const CPDF_Point& point);
  FX_BOOL OnMouseWheel(CPDFSDK_PageView* pPageView,
                       CPDFSDK_Annot* pAnnot,
                       FX_DWORD nFlags,
                       short zDelta,
                       const CPDF_Point& point);
  FX_BOOL OnRButtonDown(CPDFSDK_PageView* pPageView,
                        CPDFSDK_Annot* pAnnot,
                        FX_DWORD nFlags,
                        const CPDF_Point& point);
  FX_BOOL OnRButtonUp(CPDFSDK_PageView* pPageView,
                      CPDFSDK_Annot* pAnnot,
                      FX_DWORD nFlags,
                      const CPDF_Point& point);
  FX_BOOL OnRButtonDblClk(CPDFSDK_PageView* pPageView,
                          CPDFSDK_Annot* pAnnot,
                          FX_DWORD nFlags,
                          const CPDF_Point& point) {
    return FALSE;
  }
  FX_BOOL OnChar(CPDFSDK_Annot* pAnnot, FX_DWORD nChar, FX_DWORD nFlags);
  FX_BOOL OnKeyDown(CPDFSDK_Annot* pAnnot, int nKeyCode, int nFlag);
  FX_BOOL OnKeyUp(CPDFSDK_Annot* pAnnot, int nKeyCode, int nFlag);
  void OnDeSelected(CPDFSDK_Annot* pAnnot) {}
  void OnSelected(CPDFSDK_Annot* pAnnot) {}
  FX_BOOL OnSetFocus(CPDFSDK_Annot* pAnnot, FX_DWORD nFlag);
  FX_BOOL OnKillFocus(CPDFSDK_Annot* pAnnot, FX_DWORD nFlag);

  void SetFormFiller(CFFL_IFormFiller* pFiller) { m_pFormFiller = pFiller; }
  CFFL_IFormFiller* GetFormFiller() { return m_pFormFiller; }

 private:
  CPDFDoc_Environment* m_pApp;
  CFFL_IFormFiller* m_pFormFiller;
};

using CBA_AnnotHandlerArray = CFX_ArrayTemplate<CPDFSDK_AnnotHandler*>;

class CPDFSDK_AnnotHandlerMgr {
 public:
  // Destroy the handler
  explicit CPDFSDK_AnnotHandlerMgr(CPDFDoc_Environment* pApp);
  virtual ~CPDFSDK_AnnotHandlerMgr();

  void RegisterAnnotHandler(CPDFSDK_AnnotHandler* pAnnotHandler);
  void UnRegisterAnnotHandler(CPDFSDK_AnnotHandler* pAnnotHandler);

  virtual CPDFSDK_Annot* NewAnnot(CPDF_Annot* pAnnot,
                                  CPDFSDK_PageView* pPageView);
  virtual void ReleaseAnnot(CPDFSDK_Annot* pAnnot);

  virtual void Annot_OnCreate(CPDFSDK_Annot* pAnnot);
  virtual void Annot_OnLoad(CPDFSDK_Annot* pAnnot);

  CPDFSDK_AnnotHandler* GetAnnotHandler(CPDFSDK_Annot* pAnnot) const;
  virtual void Annot_OnDraw(CPDFSDK_PageView* pPageView,
                            CPDFSDK_Annot* pAnnot,
                            CFX_RenderDevice* pDevice,
                            CPDF_Matrix* pUser2Device,
                            FX_DWORD dwFlags);

  virtual void Annot_OnMouseEnter(CPDFSDK_PageView* pPageView,
                                  CPDFSDK_Annot* pAnnot,
                                  FX_DWORD nFlags);
  virtual void Annot_OnMouseExit(CPDFSDK_PageView* pPageView,
                                 CPDFSDK_Annot* pAnnot,
                                 FX_DWORD nFlags);

  virtual FX_BOOL Annot_OnLButtonDown(CPDFSDK_PageView* pPageView,
                                      CPDFSDK_Annot* pAnnot,
                                      FX_DWORD nFlags,
                                      const CPDF_Point& point);
  virtual FX_BOOL Annot_OnLButtonUp(CPDFSDK_PageView* pPageView,
                                    CPDFSDK_Annot* pAnnot,
                                    FX_DWORD nFlags,
                                    const CPDF_Point& point);
  virtual FX_BOOL Annot_OnLButtonDblClk(CPDFSDK_PageView* pPageView,
                                        CPDFSDK_Annot* pAnnot,
                                        FX_DWORD nFlags,
                                        const CPDF_Point& point);

  virtual FX_BOOL Annot_OnMouseMove(CPDFSDK_PageView* pPageView,
                                    CPDFSDK_Annot* pAnnot,
                                    FX_DWORD nFlags,
                                    const CPDF_Point& point);
  virtual FX_BOOL Annot_OnMouseWheel(CPDFSDK_PageView* pPageView,
                                     CPDFSDK_Annot* pAnnot,
                                     FX_DWORD nFlags,
                                     short zDelta,
                                     const CPDF_Point& point);
  virtual FX_BOOL Annot_OnRButtonDown(CPDFSDK_PageView* pPageView,
                                      CPDFSDK_Annot* pAnnot,
                                      FX_DWORD nFlags,
                                      const CPDF_Point& point);
  virtual FX_BOOL Annot_OnRButtonUp(CPDFSDK_PageView* pPageView,
                                    CPDFSDK_Annot* pAnnot,
                                    FX_DWORD nFlags,
                                    const CPDF_Point& point);

  virtual FX_BOOL Annot_OnChar(CPDFSDK_Annot* pAnnot,
                               FX_DWORD nChar,
                               FX_DWORD nFlags);
  virtual FX_BOOL Annot_OnKeyDown(CPDFSDK_Annot* pAnnot,
                                  int nKeyCode,
                                  int nFlag);
  virtual FX_BOOL Annot_OnKeyUp(CPDFSDK_Annot* pAnnot, int nKeyCode, int nFlag);

  virtual FX_BOOL Annot_OnSetFocus(CPDFSDK_Annot* pAnnot, FX_DWORD nFlag);
  virtual FX_BOOL Annot_OnKillFocus(CPDFSDK_Annot* pAnnot, FX_DWORD nFlag);

  virtual CPDF_Rect Annot_OnGetViewBBox(CPDFSDK_PageView* pPageView,
                                        CPDFSDK_Annot* pAnnot);
  virtual FX_BOOL Annot_OnHitTest(CPDFSDK_PageView* pPageView,
                                  CPDFSDK_Annot* pAnnot,
                                  const CPDF_Point& point);

 private:
  CPDFSDK_AnnotHandler* GetAnnotHandler(const CFX_ByteString& sType) const;
  CPDFSDK_Annot* GetNextAnnot(CPDFSDK_Annot* pSDKAnnot, FX_BOOL bNext);

  CBA_AnnotHandlerArray m_Handlers;
  std::map<CFX_ByteString, CPDFSDK_AnnotHandler*> m_mapType2Handler;
  CPDFDoc_Environment* m_pApp;
};

typedef int (*AI_COMPARE)(CPDFSDK_Annot* p1, CPDFSDK_Annot* p2);

class CPDFSDK_AnnotIterator {
 public:
  CPDFSDK_AnnotIterator(CPDFSDK_PageView* pPageView,
                        FX_BOOL bReverse,
                        FX_BOOL bIgnoreTopmost = FALSE,
                        FX_BOOL bCircle = FALSE,
                        CFX_PtrArray* pList = NULL);
  virtual ~CPDFSDK_AnnotIterator() {}

  virtual CPDFSDK_Annot* Next(const CPDFSDK_Annot* pCurrent);
  virtual CPDFSDK_Annot* Prev(const CPDFSDK_Annot* pCurrent);
  virtual CPDFSDK_Annot* Next(int& index);
  virtual CPDFSDK_Annot* Prev(int& index);
  virtual int Count() { return m_pIteratorAnnotList.GetSize(); }

  virtual FX_BOOL InitIteratorAnnotList(CPDFSDK_PageView* pPageView,
                                        CFX_PtrArray* pList = NULL);

  void InsertSort(CFX_PtrArray& arrayList, AI_COMPARE pCompare);

 protected:
  CPDFSDK_Annot* NextAnnot(const CPDFSDK_Annot* pCurrent);
  CPDFSDK_Annot* PrevAnnot(const CPDFSDK_Annot* pCurrent);
  CPDFSDK_Annot* NextAnnot(int& index);
  CPDFSDK_Annot* PrevAnnot(int& index);

  CFX_PtrArray m_pIteratorAnnotList;
  FX_BOOL m_bReverse;
  FX_BOOL m_bIgnoreTopmost;
  FX_BOOL m_bCircle;
};

#endif  // FPDFSDK_INCLUDE_FSDK_ANNOTHANDLER_H_
