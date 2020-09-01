// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFDOC_H_
#define XFA_FXFA_CXFA_FFDOC_H_

#include <map>
#include <memory>

#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/unowned_ptr.h"
#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/prefinalizer.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fxfa/fxfa.h"
#include "xfa/fxfa/parser/cxfa_document.h"

class CFGAS_PDFFontMgr;
class CFX_ChecksumContext;
class CFX_DIBBase;
class CFX_DIBitmap;
class CFX_XMLDocument;
class CPDF_Document;
class CXFA_FFApp;
class CXFA_FFNotify;
class CXFA_FFDocView;
class CXFA_LayoutProcessor;

namespace cppgc {
class Heap;
}  // namespace cppgc

struct FX_IMAGEDIB_AND_DPI {
  FX_IMAGEDIB_AND_DPI();
  FX_IMAGEDIB_AND_DPI(const FX_IMAGEDIB_AND_DPI& that);
  FX_IMAGEDIB_AND_DPI(const RetainPtr<CFX_DIBBase>& pDib,
                      int32_t xDpi,
                      int32_t yDpi);
  ~FX_IMAGEDIB_AND_DPI();

  RetainPtr<CFX_DIBBase> pDibSource;
  int32_t iImageXDpi;
  int32_t iImageYDpi;
};

class CXFA_FFDoc : public cppgc::GarbageCollected<CXFA_FFDoc> {
  CPPGC_USING_PRE_FINALIZER(CXFA_FFDoc, PreFinalize);

 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FFDoc();

  void PreFinalize();
  void Trace(cppgc::Visitor* visitor) const;

  bool OpenDoc(CFX_XMLDocument* pXML);

  void SetChangeMark();
  void InvalidateRect(CXFA_FFPageView* pPageView, const CFX_RectF& rt);
  void DisplayCaret(CXFA_FFWidget* hWidget,
                    bool bVisible,
                    const CFX_RectF* pRtAnchor);
  bool GetPopupPos(CXFA_FFWidget* hWidget,
                   float fMinPopup,
                   float fMaxPopup,
                   const CFX_RectF& rtAnchor,
                   CFX_RectF* pPopupRect) const;
  bool PopupMenu(CXFA_FFWidget* hWidget, const CFX_PointF& ptPopup);
  void PageViewEvent(CXFA_FFPageView* pPageView, uint32_t dwFlags);
  void WidgetPostAdd(CXFA_FFWidget* hWidget);
  void WidgetPreRemove(CXFA_FFWidget* hWidget);
  int32_t CountPages() const;
  int32_t GetCurrentPage() const;
  void SetCurrentPage(int32_t iCurPage);
  bool IsCalculationsEnabled() const;
  void SetCalculationsEnabled(bool bEnabled);
  WideString GetTitle() const;
  void SetTitle(const WideString& wsTitle);
  void ExportData(const WideString& wsFilePath, bool bXDP);
  void GotoURL(const WideString& bsURL);
  bool IsValidationsEnabled() const;
  void SetValidationsEnabled(bool bEnabled);
  void SetFocusWidget(CXFA_FFWidget* hWidget);
  void Print(int32_t nStartPage, int32_t nEndPage, uint32_t dwOptions);
  FX_ARGB GetHighlightColor() const;
  IJS_Runtime* GetIJSRuntime() const;
  CFX_XMLDocument* GetXMLDocument() const;
  RetainPtr<IFX_SeekableReadStream> OpenLinkedFile(const WideString& wsLink);

  CXFA_FFDocView* CreateDocView();
  FormType GetFormType() const { return m_FormType; }
  cppgc::Heap* GetHeap() const { return m_pHeap.Get(); }
  CXFA_Document* GetXFADoc() const { return m_pDocument; }
  CXFA_FFApp* GetApp() const { return m_pApp.Get(); }
  CPDF_Document* GetPDFDoc() const { return m_pPDFDoc.Get(); }
  CFGAS_PDFFontMgr* GetPDFFontMgr() const { return m_pPDFFontMgr.get(); }
  CXFA_FFDocView* GetDocView(CXFA_LayoutProcessor* pLayout);
  CXFA_FFDocView* GetDocView();
  RetainPtr<CFX_DIBitmap> GetPDFNamedImage(WideStringView wsName,
                                           int32_t& iImageXDpi,
                                           int32_t& iImageYDpi);

  bool SavePackage(CXFA_Node* pNode,
                   const RetainPtr<IFX_SeekableStream>& pFile);

 private:
  CXFA_FFDoc(CXFA_FFApp* pApp,
             IXFA_DocEnvironment* pDocEnvironment,
             CPDF_Document* pPDFDoc,
             cppgc::Heap* pHeap);
  bool BuildDoc(CFX_XMLDocument* pXML);

  UnownedPtr<IXFA_DocEnvironment> const m_pDocEnvironment;
  UnownedPtr<CPDF_Document> const m_pPDFDoc;
  UnownedPtr<cppgc::Heap> const m_pHeap;
  UnownedPtr<CFX_XMLDocument> m_pXMLDoc;
  cppgc::Member<CXFA_FFApp> const m_pApp;
  cppgc::Member<CXFA_FFNotify> m_pNotify;
  cppgc::Member<CXFA_Document> m_pDocument;
  cppgc::Member<CXFA_FFDocView> m_DocView;
  std::unique_ptr<CFGAS_PDFFontMgr> m_pPDFFontMgr;
  std::map<uint32_t, FX_IMAGEDIB_AND_DPI> m_HashToDibDpiMap;
  FormType m_FormType = FormType::kXFAForeground;
};

#endif  // XFA_FXFA_CXFA_FFDOC_H_
