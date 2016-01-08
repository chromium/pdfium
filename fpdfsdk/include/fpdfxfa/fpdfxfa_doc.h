// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFXFA_DOC_H_
#define FPDFXFA_DOC_H_

#include "public/fpdfview.h"
#include "xfa/include/fxfa/fxfa.h"

class CPDFXFA_App;
class CPDFXFA_Document;
class CPDFXFA_Page;
class CPDFSDK_Document;
class CPDFDoc_Environment;
class IJS_Runtime;
class IJS_Context;
class IXFA_DocHandler;

class CPDFXFA_Document : public IXFA_DocProvider {
 public:
  CPDFXFA_Document(CPDF_Document* pPDFDoc, CPDFXFA_App* pProvider);
  ~CPDFXFA_Document();

  FX_BOOL LoadXFADoc();
  CPDFXFA_App* GetApp() { return m_pApp; }
  CPDF_Document* GetPDFDoc() { return m_pPDFDoc; }
  IXFA_Doc* GetXFADoc() { return m_pXFADoc; }
  IXFA_DocView* GetXFADocView() { return m_pXFADocView; }

  int GetPageCount();
  CPDFXFA_Page* GetPage(int page_index);
  CPDFXFA_Page* GetPage(IXFA_PageView* pPage);
  void RemovePage(CPDFXFA_Page* page);
  int GetDocType() { return m_iDocType; }

  CPDFSDK_Document* GetSDKDocument(CPDFDoc_Environment* pFormFillEnv);

  void FXRect2PDFRect(const CFX_RectF& fxRectF, CPDF_Rect& pdfRect);

  virtual void SetChangeMark(IXFA_Doc* hDoc);
  virtual FX_BOOL GetChangeMark(IXFA_Doc* hDoc);
  // used in dynamic xfa, dwFlags refer to XFA_INVALIDATE_XXX macros.
  virtual void InvalidateRect(IXFA_PageView* pPageView,
                              const CFX_RectF& rt,
                              FX_DWORD dwFlags = 0);
  // used in static xfa, dwFlags refer to XFA_INVALIDATE_XXX macros.
  virtual void InvalidateRect(IXFA_Widget* hWidget, FX_DWORD dwFlags = 0);
  // show or hide caret
  virtual void DisplayCaret(IXFA_Widget* hWidget,
                            FX_BOOL bVisible,
                            const CFX_RectF* pRtAnchor);
  // dwPos: (0:bottom 1:top)
  virtual FX_BOOL GetPopupPos(IXFA_Widget* hWidget,
                              FX_FLOAT fMinPopup,
                              FX_FLOAT fMaxPopup,
                              const CFX_RectF& rtAnchor,
                              CFX_RectF& rtPopup);
  virtual FX_BOOL PopupMenu(IXFA_Widget* hWidget,
                            CFX_PointF ptPopup,
                            const CFX_RectF* pRectExclude = NULL);

  // dwFlags XFA_PAGEVIEWEVENT_Added, XFA_PAGEVIEWEVENT_Removing
  virtual void PageViewEvent(IXFA_PageView* pPageView, FX_DWORD dwFlags);
  // dwEvent refer to XFA_WIDGETEVENT_XXX
  virtual void WidgetEvent(IXFA_Widget* hWidget,
                           CXFA_WidgetAcc* pWidgetData,
                           FX_DWORD dwEvent,
                           void* pParam = NULL,
                           void* pAdditional = NULL);

  // return true if render it.
  virtual FX_BOOL RenderCustomWidget(IXFA_Widget* hWidget,
                                     CFX_Graphics* pGS,
                                     CFX_Matrix* pMatrix,
                                     const CFX_RectF& rtUI) {
    return FALSE;
  }

  // host method
  virtual int32_t CountPages(IXFA_Doc* hDoc);
  virtual int32_t GetCurrentPage(IXFA_Doc* hDoc);
  virtual void SetCurrentPage(IXFA_Doc* hDoc, int32_t iCurPage);
  virtual FX_BOOL IsCalculationsEnabled(IXFA_Doc* hDoc);
  virtual void SetCalculationsEnabled(IXFA_Doc* hDoc, FX_BOOL bEnabled);
  virtual void GetTitle(IXFA_Doc* hDoc, CFX_WideString& wsTitle);
  virtual void SetTitle(IXFA_Doc* hDoc, const CFX_WideStringC& wsTitle);
  virtual void ExportData(IXFA_Doc* hDoc,
                          const CFX_WideStringC& wsFilePath,
                          FX_BOOL bXDP = TRUE);
  virtual void ImportData(IXFA_Doc* hDoc, const CFX_WideStringC& wsFilePath);
  virtual void GotoURL(IXFA_Doc* hDoc,
                       const CFX_WideStringC& bsURL,
                       FX_BOOL bAppend = TRUE);
  virtual FX_BOOL IsValidationsEnabled(IXFA_Doc* hDoc);
  virtual void SetValidationsEnabled(IXFA_Doc* hDoc, FX_BOOL bEnabled);
  virtual void SetFocusWidget(IXFA_Doc* hDoc, IXFA_Widget* hWidget);
  virtual void Print(IXFA_Doc* hDoc,
                     int32_t nStartPage,
                     int32_t nEndPage,
                     FX_DWORD dwOptions);

  // LayoutPseudo method
  virtual int32_t AbsPageCountInBatch(IXFA_Doc* hDoc) { return 0; }
  virtual int32_t AbsPageInBatch(IXFA_Doc* hDoc, IXFA_Widget* hWidget) {
    return 0;
  }
  virtual int32_t SheetCountInBatch(IXFA_Doc* hDoc) { return 0; }
  virtual int32_t SheetInBatch(IXFA_Doc* hDoc, IXFA_Widget* hWidget) {
    return 0;
  }

  // SignaturePseudoModel method
  // TODO:
  virtual int32_t Verify(
      IXFA_Doc* hDoc,
      CXFA_Node* pSigNode,
      FX_BOOL
          bUsed = TRUE /*, SecurityHandler* pHandler, SignatureInfo &info*/) {
    return 0;
  }
  virtual FX_BOOL Sign(
      IXFA_Doc* hDoc,
      CXFA_NodeList* pNodeList,
      const CFX_WideStringC& wsExpression,
      const CFX_WideStringC& wsXMLIdent,
      const CFX_WideStringC& wsValue = FX_WSTRC(L"open"),
      FX_BOOL
          bUsed = TRUE /*, SecurityHandler* pHandler = NULL, SignatureInfo &info*/) {
    return 0;
  }
  virtual CXFA_NodeList* Enumerate(IXFA_Doc* hDoc) { return 0; }
  virtual FX_BOOL Clear(IXFA_Doc* hDoc,
                        CXFA_Node* pSigNode,
                        FX_BOOL bCleared = TRUE) {
    return 0;
  }

  // Get document path
  virtual void GetURL(IXFA_Doc* hDoc, CFX_WideString& wsDocURL);
  virtual FX_ARGB GetHighlightColor(IXFA_Doc* hDoc);
  virtual void AddDoRecord(IXFA_Widget* hWidget);
  /**
   *Submit data to email, http, ftp.
   * @param[in] hDoc The document handler.
   * @param[in] eFormat Determines the format in which the data will be
   *submitted. XFA_ATTRIBUTEENUM_Xdp, XFA_ATTRIBUTEENUM_Xml...
   * @param[in] wsTarget The URL to which the data will be submitted.
   * @param[in] eEncoding The encoding of text content.
   * @param[in] pXDPContent Controls what subset of the data is submitted, used
   *only when the format property is xdp.
   * @param[in] bEmbedPDF, specifies whether PDF is embedded in the submitted
   *content or not.
   */
  virtual FX_BOOL SubmitData(IXFA_Doc* hDoc, CXFA_Submit submit);

  virtual FX_BOOL CheckWord(IXFA_Doc* hDoc, const CFX_ByteStringC& sWord) {
    return FALSE;
  }
  virtual FX_BOOL GetSuggestWords(IXFA_Doc* hDoc,
                                  const CFX_ByteStringC& sWord,
                                  CFX_ByteStringArray& sSuggest) {
    return FALSE;
  }

  // Get PDF javascript object, set the object to hValue.
  virtual FX_BOOL GetPDFScriptObject(IXFA_Doc* hDoc,
                                     const CFX_ByteStringC& utf8Name,
                                     FXJSE_HVALUE hValue);

  virtual FX_BOOL GetGlobalProperty(IXFA_Doc* hDoc,
                                    const CFX_ByteStringC& szPropName,
                                    FXJSE_HVALUE hValue);
  virtual FX_BOOL SetGlobalProperty(IXFA_Doc* hDoc,
                                    const CFX_ByteStringC& szPropName,
                                    FXJSE_HVALUE hValue);
  virtual CPDF_Document* OpenPDF(IXFA_Doc* hDoc,
                                 IFX_FileRead* pFile,
                                 FX_BOOL bTakeOverFile) {
    return NULL;
  }

  virtual IFX_FileRead* OpenLinkedFile(IXFA_Doc* hDoc,
                                       const CFX_WideString& wsLink);

  FX_BOOL _GetHValueByName(const CFX_ByteStringC& utf8Name,
                           FXJSE_HVALUE hValue,
                           IJS_Runtime* runTime);
  FX_BOOL _OnBeforeNotifySumbit();
  void _OnAfterNotifySumbit();
  FX_BOOL _NotifySubmit(FX_BOOL bPrevOrPost);
  FX_BOOL _SubmitData(IXFA_Doc* hDoc, CXFA_Submit submit);
  FX_BOOL _MailToInfo(CFX_WideString& csURL,
                      CFX_WideString& csToAddress,
                      CFX_WideString& csCCAddress,
                      CFX_WideString& csBCCAddress,
                      CFX_WideString& csSubject,
                      CFX_WideString& csMsg);
  FX_BOOL _ExportSubmitFile(FPDF_FILEHANDLER* ppFileHandler,
                            int fileType,
                            FPDF_DWORD encodeType,
                            FPDF_DWORD flag = 0x01111111);
  void _ToXFAContentFlags(CFX_WideString csSrcContent, FPDF_DWORD& flag);
  void _ClearChangeMark();

 private:
  void CloseXFADoc(IXFA_DocHandler* pDoc) {
    if (pDoc) {
      pDoc->CloseDoc(m_pXFADoc);
      pDoc->ReleaseDoc(m_pXFADoc);
      m_pXFADoc = NULL;
      m_pXFADocView = NULL;
    }
  }

  int m_iDocType;
  CPDF_Document* m_pPDFDoc;
  CPDFSDK_Document* m_pSDKDoc;
  IXFA_Doc* m_pXFADoc;
  IXFA_DocView* m_pXFADocView;
  CPDFXFA_App* m_pApp;
  IJS_Context* m_pJSContext;
  CFX_ArrayTemplate<CPDFXFA_Page*> m_XFAPageList;
};

#endif  // FPDFXFA_DOC_H_
