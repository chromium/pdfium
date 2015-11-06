// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FSDK_MGR_H_
#define FPDFSDK_INCLUDE_FSDK_MGR_H_

#include <map>

#include "core/include/fpdftext/fpdf_text.h"
#include "fsdk_actionhandler.h"
#include "fsdk_annothandler.h"
#include "fsdk_baseannot.h"
#include "fsdk_baseform.h"
#include "fsdk_common.h"
#include "fsdk_define.h"
#include "fx_systemhandler.h"
#include "javascript/IJavaScript.h"
#include "public/fpdf_formfill.h"
#include "public/fpdf_fwlevent.h"  // cross platform keycode and events define.
#include "third_party/base/nonstd_unique_ptr.h"

class CFFL_IFormFiller;
class CPDFSDK_ActionHandler;
class CPDFSDK_Annot;
class CPDFSDK_Document;
class CPDFSDK_InterForm;
class CPDFSDK_PageView;
class CPDFSDK_Widget;
class IFX_SystemHandler;

class CPDFDoc_Environment final {
 public:
  CPDFDoc_Environment(CPDF_Document* pDoc, FPDF_FORMFILLINFO* pFFinfo);
  ~CPDFDoc_Environment();

  void FFI_Invalidate(FPDF_PAGE page,
                      double left,
                      double top,
                      double right,
                      double bottom) {
    if (m_pInfo && m_pInfo->FFI_Invalidate)
      m_pInfo->FFI_Invalidate(m_pInfo, page, left, top, right, bottom);
  }

  void FFI_OutputSelectedRect(FPDF_PAGE page,
                              double left,
                              double top,
                              double right,
                              double bottom) {
    if (m_pInfo && m_pInfo->FFI_OutputSelectedRect)
      m_pInfo->FFI_OutputSelectedRect(m_pInfo, page, left, top, right, bottom);
  }

  void FFI_SetCursor(int nCursorType) {
    if (m_pInfo && m_pInfo->FFI_SetCursor)
      m_pInfo->FFI_SetCursor(m_pInfo, nCursorType);
  }

  int FFI_SetTimer(int uElapse, TimerCallback lpTimerFunc) {
    if (m_pInfo && m_pInfo->FFI_SetTimer)
      return m_pInfo->FFI_SetTimer(m_pInfo, uElapse, lpTimerFunc);
    return -1;
  }

  void FFI_KillTimer(int nTimerID) {
    if (m_pInfo && m_pInfo->FFI_KillTimer)
      m_pInfo->FFI_KillTimer(m_pInfo, nTimerID);
  }

  FX_SYSTEMTIME FFI_GetLocalTime() const {
    FX_SYSTEMTIME fxtime;
    if (m_pInfo && m_pInfo->FFI_GetLocalTime) {
      FPDF_SYSTEMTIME systime = m_pInfo->FFI_GetLocalTime(m_pInfo);
      fxtime.wDay = systime.wDay;
      fxtime.wDayOfWeek = systime.wDayOfWeek;
      fxtime.wHour = systime.wHour;
      fxtime.wMilliseconds = systime.wMilliseconds;
      fxtime.wMinute = systime.wMinute;
      fxtime.wMonth = systime.wMonth;
      fxtime.wSecond = systime.wSecond;
      fxtime.wYear = systime.wYear;
    }
    return fxtime;
  }

  void FFI_OnChange() {
    if (m_pInfo && m_pInfo->FFI_OnChange)
      m_pInfo->FFI_OnChange(m_pInfo);
  }

  FX_BOOL FFI_IsSHIFTKeyDown(FX_DWORD nFlag) const {
    return (nFlag & FWL_EVENTFLAG_ShiftKey) != 0;
  }

  FX_BOOL FFI_IsCTRLKeyDown(FX_DWORD nFlag) const {
    return (nFlag & FWL_EVENTFLAG_ControlKey) != 0;
  }

  FX_BOOL FFI_IsALTKeyDown(FX_DWORD nFlag) const {
    return (nFlag & FWL_EVENTFLAG_AltKey) != 0;
  }

  FX_BOOL FFI_IsINSERTKeyDown(FX_DWORD nFlag) const { return FALSE; }

  int JS_appAlert(const FX_WCHAR* Msg,
                  const FX_WCHAR* Title,
                  FX_UINT Type,
                  FX_UINT Icon);
  int JS_appResponse(const FX_WCHAR* Question,
                     const FX_WCHAR* Title,
                     const FX_WCHAR* Default,
                     const FX_WCHAR* cLabel,
                     FPDF_BOOL bPassword,
                     void* response,
                     int length);

  void JS_appBeep(int nType) {
    if (m_pInfo && m_pInfo->m_pJsPlatform && m_pInfo->m_pJsPlatform->app_beep)
      m_pInfo->m_pJsPlatform->app_beep(m_pInfo->m_pJsPlatform, nType);
  }

  CFX_WideString JS_fieldBrowse();
  CFX_WideString JS_docGetFilePath();

  void JS_docSubmitForm(void* formData, int length, const FX_WCHAR* URL);
  void JS_docmailForm(void* mailData,
                      int length,
                      FPDF_BOOL bUI,
                      const FX_WCHAR* To,
                      const FX_WCHAR* Subject,
                      const FX_WCHAR* CC,
                      const FX_WCHAR* BCC,
                      const FX_WCHAR* Msg);

  void JS_docprint(FPDF_BOOL bUI,
                   int nStart,
                   int nEnd,
                   FPDF_BOOL bSilent,
                   FPDF_BOOL bShrinkToFit,
                   FPDF_BOOL bPrintAsImage,
                   FPDF_BOOL bReverse,
                   FPDF_BOOL bAnnotations) {
    if (m_pInfo && m_pInfo->m_pJsPlatform && m_pInfo->m_pJsPlatform->Doc_print)
      m_pInfo->m_pJsPlatform->Doc_print(m_pInfo->m_pJsPlatform, bUI, nStart,
                                        nEnd, bSilent, bShrinkToFit,
                                        bPrintAsImage, bReverse, bAnnotations);
  }

  void JS_docgotoPage(int nPageNum) {
    if (m_pInfo && m_pInfo->m_pJsPlatform &&
        m_pInfo->m_pJsPlatform->Doc_gotoPage)
      m_pInfo->m_pJsPlatform->Doc_gotoPage(m_pInfo->m_pJsPlatform, nPageNum);
  }

  FPDF_PAGE FFI_GetPage(FPDF_DOCUMENT document, int nPageIndex) {
    if (m_pInfo && m_pInfo->FFI_GetPage)
      return m_pInfo->FFI_GetPage(m_pInfo, document, nPageIndex);
    return NULL;
  }

  FPDF_PAGE FFI_GetCurrentPage(FPDF_DOCUMENT document) {
    if (m_pInfo && m_pInfo->FFI_GetCurrentPage)
      return m_pInfo->FFI_GetCurrentPage(m_pInfo, document);
    return NULL;
  }

  int FFI_GetRotation(FPDF_PAGE page) {
    if (m_pInfo && m_pInfo->FFI_GetRotation)
      return m_pInfo->FFI_GetRotation(m_pInfo, page);
    return 0;
  }

  void FFI_ExecuteNamedAction(const FX_CHAR* namedAction) {
    if (m_pInfo && m_pInfo->FFI_ExecuteNamedAction)
      m_pInfo->FFI_ExecuteNamedAction(m_pInfo, namedAction);
  }

  void FFI_OnSetFieldInputFocus(void* field,
                                FPDF_WIDESTRING focusText,
                                FPDF_DWORD nTextLen,
                                FX_BOOL bFocus) {
    if (m_pInfo && m_pInfo->FFI_SetTextFieldFocus)
      m_pInfo->FFI_SetTextFieldFocus(m_pInfo, focusText, nTextLen, bFocus);
  }

  void FFI_DoURIAction(const FX_CHAR* bsURI) {
    if (m_pInfo && m_pInfo->FFI_DoURIAction)
      m_pInfo->FFI_DoURIAction(m_pInfo, bsURI);
  }

  void FFI_DoGoToAction(int nPageIndex,
                        int zoomMode,
                        float* fPosArray,
                        int sizeOfArray) {
    if (m_pInfo && m_pInfo->FFI_DoGoToAction)
      m_pInfo->FFI_DoGoToAction(m_pInfo, nPageIndex, zoomMode, fPosArray,
                                sizeOfArray);
  }

  FX_BOOL IsJSInitiated() const { return m_pInfo && m_pInfo->m_pJsPlatform; }
  void SetSDKDocument(CPDFSDK_Document* pFXDoc) { m_pSDKDoc = pFXDoc; }
  CPDFSDK_Document* GetSDKDocument() const { return m_pSDKDoc; }
  CPDF_Document* GetPDFDocument() const { return m_pPDFDoc; }
  CFX_ByteString GetAppName() const { return ""; }
  IFX_SystemHandler* GetSysHandler() const { return m_pSysHandler.get(); }
  FPDF_FORMFILLINFO* GetFormFillInfo() const { return m_pInfo; }

  CFFL_IFormFiller* GetIFormFiller();             // Creates if not present.
  CPDFSDK_AnnotHandlerMgr* GetAnnotHandlerMgr();  // Creates if not present.
  IJS_Runtime* GetJSRuntime();                    // Creates if not present.
  CPDFSDK_ActionHandler* GetActionHander();       // Creates if not present.

 private:
  nonstd::unique_ptr<CPDFSDK_AnnotHandlerMgr> m_pAnnotHandlerMgr;
  nonstd::unique_ptr<CPDFSDK_ActionHandler> m_pActionHandler;
  nonstd::unique_ptr<IJS_Runtime> m_pJSRuntime;
  FPDF_FORMFILLINFO* const m_pInfo;
  CPDFSDK_Document* m_pSDKDoc;
  CPDF_Document* const m_pPDFDoc;
  nonstd::unique_ptr<CFFL_IFormFiller> m_pIFormFiller;
  nonstd::unique_ptr<IFX_SystemHandler> m_pSysHandler;
};

class CPDFSDK_Document {
 public:
  CPDFSDK_Document(CPDF_Document* pDoc, CPDFDoc_Environment* pEnv);
  ~CPDFSDK_Document();

  CPDFSDK_InterForm* GetInterForm();

  // Gets the document object for the next layer down; for master this is
  // a CPDF_Document, but for XFA it is a CPDFXFA_Document.
  CPDF_Document* GetDocument() const { return m_pDoc; }

  // Gets the CPDF_Document, either directly in master, or from the
  // CPDFXFA_Document for XFA.
  CPDF_Document* GetPDFDocument() const { return m_pDoc; }

  CPDFSDK_PageView* GetPageView(CPDF_Page* pPDFPage, FX_BOOL ReNew = TRUE);
  CPDFSDK_PageView* GetPageView(int nIndex);
  CPDFSDK_PageView* GetCurrentView();
  void ReMovePageView(CPDF_Page* pPDFPage);
  void UpdateAllViews(CPDFSDK_PageView* pSender, CPDFSDK_Annot* pAnnot);

  CPDFSDK_Annot* GetFocusAnnot();

  IJS_Runtime* GetJsRuntime();

  FX_BOOL SetFocusAnnot(CPDFSDK_Annot* pAnnot, FX_UINT nFlag = 0);
  FX_BOOL KillFocusAnnot(FX_UINT nFlag = 0);

  FX_BOOL ExtractPages(const CFX_WordArray& arrExtraPages,
                       CPDF_Document* pDstDoc);
  FX_BOOL InsertPages(int nInsertAt,
                      const CPDF_Document* pSrcDoc,
                      const CFX_WordArray& arrSrcPages);
  FX_BOOL ReplacePages(int nPage,
                       const CPDF_Document* pSrcDoc,
                       const CFX_WordArray& arrSrcPages);

  void OnCloseDocument();

  int GetPageCount() { return m_pDoc->GetPageCount(); }
  FX_BOOL GetPermissions(int nFlag);
  FX_BOOL GetChangeMark() { return m_bChangeMask; }
  void SetChangeMark() { m_bChangeMask = TRUE; }
  void ClearChangeMark() { m_bChangeMask = FALSE; }
  CFX_WideString GetPath();
  CPDF_Page* GetPage(int nIndex);
  CPDFDoc_Environment* GetEnv() { return m_pEnv; }
  void ProcJavascriptFun();
  FX_BOOL ProcOpenAction();
  CPDF_OCContext* GetOCContext();

 private:
  std::map<CPDF_Page*, CPDFSDK_PageView*> m_pageMap;
  CPDF_Document* m_pDoc;
  nonstd::unique_ptr<CPDFSDK_InterForm> m_pInterForm;
  CPDFSDK_Annot* m_pFocusAnnot;
  CPDFDoc_Environment* m_pEnv;
  nonstd::unique_ptr<CPDF_OCContext> m_pOccontent;
  FX_BOOL m_bChangeMask;
  FX_BOOL m_bBeingDestroyed;
};
class CPDFSDK_PageView final {
 public:
  CPDFSDK_PageView(CPDFSDK_Document* pSDKDoc, CPDF_Page* page);
  ~CPDFSDK_PageView();
  void PageView_OnDraw(CFX_RenderDevice* pDevice,
                       CPDF_Matrix* pUser2Device,
                       CPDF_RenderOptions* pOptions);
  const CPDF_Annot* GetPDFAnnotAtPoint(FX_FLOAT pageX, FX_FLOAT pageY);
  CPDFSDK_Annot* GetFXAnnotAtPoint(FX_FLOAT pageX, FX_FLOAT pageY);
  const CPDF_Annot* GetPDFWidgetAtPoint(FX_FLOAT pageX, FX_FLOAT pageY);
  CPDFSDK_Annot* GetFXWidgetAtPoint(FX_FLOAT pageX, FX_FLOAT pageY);
  CPDFSDK_Annot* GetFocusAnnot();
  void SetFocusAnnot(CPDFSDK_Annot* pSDKAnnot, FX_UINT nFlag = 0) {
    m_pSDKDoc->SetFocusAnnot(pSDKAnnot, nFlag);
  }
  FX_BOOL KillFocusAnnot(FX_UINT nFlag = 0) {
    return m_pSDKDoc->KillFocusAnnot(nFlag);
  }
  void KillFocusAnnotIfNeeded();
  FX_BOOL Annot_HasAppearance(CPDF_Annot* pAnnot);

  CPDFSDK_Annot* AddAnnot(CPDF_Dictionary* pDict);
  CPDFSDK_Annot* AddAnnot(const FX_CHAR* lpSubType, CPDF_Dictionary* pDict);
  CPDFSDK_Annot* AddAnnot(CPDF_Annot* pPDFAnnot);
  FX_BOOL DeleteAnnot(CPDFSDK_Annot* pAnnot);
  size_t CountAnnots() const;
  CPDFSDK_Annot* GetAnnot(size_t nIndex);
  CPDFSDK_Annot* GetAnnotByDict(CPDF_Dictionary* pDict);
  CPDF_Page* GetPDFPage() { return m_page; }
  CPDF_Document* GetPDFDocument();
  CPDFSDK_Document* GetSDKDocument() { return m_pSDKDoc; }
  FX_BOOL OnLButtonDown(const CPDF_Point& point, FX_UINT nFlag);
  FX_BOOL OnLButtonUp(const CPDF_Point& point, FX_UINT nFlag);
  FX_BOOL OnChar(int nChar, FX_UINT nFlag);
  FX_BOOL OnKeyDown(int nKeyCode, int nFlag);
  FX_BOOL OnKeyUp(int nKeyCode, int nFlag);

  FX_BOOL OnMouseMove(const CPDF_Point& point, int nFlag);
  FX_BOOL OnMouseWheel(double deltaX,
                       double deltaY,
                       const CPDF_Point& point,
                       int nFlag);
  bool IsValidAnnot(const CPDF_Annot* p) const;
  void GetCurrentMatrix(CPDF_Matrix& matrix) { matrix = m_curMatrix; }
  void UpdateRects(CFX_RectArray& rects);
  void UpdateView(CPDFSDK_Annot* pAnnot);
  const std::vector<CPDFSDK_Annot*>& GetAnnotList() const {
    return m_fxAnnotArray;
  }

  int GetPageIndex();
  void LoadFXAnnots();
  void SetValid(FX_BOOL bValid) { m_bValid = bValid; }
  FX_BOOL IsValid() { return m_bValid; }
  void SetLock(FX_BOOL bLocked) { m_bLocked = bLocked; }
  FX_BOOL IsLocked() { return m_bLocked; }
  void TakeOverPage() { m_bTakeOverPage = TRUE; }

 private:
  void PageView_OnHighlightFormFields(CFX_RenderDevice* pDevice,
                                      CPDFSDK_Widget* pWidget);

  CPDF_Matrix m_curMatrix;
  CPDF_Page* m_page;
  nonstd::unique_ptr<CPDF_AnnotList> m_pAnnotList;
  std::vector<CPDFSDK_Annot*> m_fxAnnotArray;
  CPDFSDK_Document* m_pSDKDoc;
  CPDFSDK_Widget* m_CaptureWidget;
  FX_BOOL m_bEnterWidget;
  FX_BOOL m_bExitWidget;
  FX_BOOL m_bOnWidget;
  FX_BOOL m_bValid;
  FX_BOOL m_bLocked;
  FX_BOOL m_bTakeOverPage;
};

template <class TYPE>
class CGW_ArrayTemplate : public CFX_ArrayTemplate<TYPE> {
 public:
  CGW_ArrayTemplate() {}
  ~CGW_ArrayTemplate() {}

  typedef int (*LP_COMPARE)(TYPE p1, TYPE p2);

  void Sort(LP_COMPARE pCompare, FX_BOOL bAscent = TRUE) {
    int nSize = this->GetSize();
    QuickSort(0, nSize - 1, bAscent, pCompare);
  }

 private:
  void QuickSort(FX_UINT nStartPos,
                 FX_UINT nStopPos,
                 FX_BOOL bAscend,
                 LP_COMPARE pCompare) {
    if (nStartPos >= nStopPos)
      return;

    if ((nStopPos - nStartPos) == 1) {
      TYPE Value1 = this->GetAt(nStartPos);
      TYPE Value2 = this->GetAt(nStopPos);

      int iGreate = (*pCompare)(Value1, Value2);
      if ((bAscend && iGreate > 0) || (!bAscend && iGreate < 0)) {
        this->SetAt(nStartPos, Value2);
        this->SetAt(nStopPos, Value1);
      }
      return;
    }

    FX_UINT m = nStartPos + (nStopPos - nStartPos) / 2;
    FX_UINT i = nStartPos;

    TYPE Value = this->GetAt(m);

    while (i < m) {
      TYPE temp = this->GetAt(i);

      int iGreate = (*pCompare)(temp, Value);
      if ((bAscend && iGreate > 0) || (!bAscend && iGreate < 0)) {
        this->InsertAt(m + 1, temp);
        this->RemoveAt(i);
        m--;
      } else {
        i++;
      }
    }

    FX_UINT j = nStopPos;

    while (j > m) {
      TYPE temp = this->GetAt(j);

      int iGreate = (*pCompare)(temp, Value);
      if ((bAscend && iGreate < 0) || (!bAscend && iGreate > 0)) {
        this->RemoveAt(j);
        this->InsertAt(m, temp);
        m++;
      } else {
        j--;
      }
    }

    if (nStartPos < m)
      QuickSort(nStartPos, m, bAscend, pCompare);
    if (nStopPos > m)
      QuickSort(m, nStopPos, bAscend, pCompare);
  }
};

#endif  // FPDFSDK_INCLUDE_FSDK_MGR_H_
