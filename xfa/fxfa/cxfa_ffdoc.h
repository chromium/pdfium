// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFDOC_H_
#define XFA_FXFA_CXFA_FFDOC_H_

#include <map>
#include <memory>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/mask.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/dib/fx_dib.h"
#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/prefinalizer.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fxfa/fxfa.h"
#include "xfa/fxfa/parser/cxfa_document.h"

class CFGAS_GEFont;
class CFGAS_PDFFontMgr;
class CFX_DIBBase;
class CFX_DIBitmap;
class CFX_XMLDocument;
class CPDF_Document;
class CXFA_FFApp;
class CXFA_FFDoc;
class CXFA_FFDocView;
class CXFA_FFNotify;
class CXFA_FFPageView;
class CXFA_FFWidget;
class CXFA_LayoutProcessor;
class IJS_Runtime;

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
  enum class PageViewEvent {
    kPostAdded = 1,
    kPostRemoved = 3,
    kStopLayout = 4,
  };

  class CallbackIface {
   public:
    virtual ~CallbackIface() = default;

    virtual void SetChangeMark(CXFA_FFDoc* hDoc) = 0;
    virtual void InvalidateRect(CXFA_FFPageView* pPageView,
                                const CFX_RectF& rt) = 0;
    // Show or hide caret.
    virtual void DisplayCaret(CXFA_FFWidget* hWidget,
                              bool bVisible,
                              const CFX_RectF* pRtAnchor) = 0;

    virtual bool GetPopupPos(CXFA_FFWidget* hWidget,
                             float fMinPopup,
                             float fMaxPopup,
                             const CFX_RectF& rtAnchor,
                             CFX_RectF* pPopupRect) = 0;
    virtual bool PopupMenu(CXFA_FFWidget* hWidget,
                           const CFX_PointF& ptPopup) = 0;

    virtual void OnPageViewEvent(CXFA_FFPageView* pPageView,
                                 PageViewEvent eEvent) = 0;

    // Caller must not pass in nullptr.
    virtual void WidgetPostAdd(CXFA_FFWidget* hWidget) = 0;
    virtual void WidgetPreRemove(CXFA_FFWidget* hWidget) = 0;

    virtual int32_t CountPages(const CXFA_FFDoc* hDoc) const = 0;
    virtual int32_t GetCurrentPage(const CXFA_FFDoc* hDoc) const = 0;
    virtual void SetCurrentPage(CXFA_FFDoc* hDoc, int32_t iCurPage) = 0;
    virtual bool IsCalculationsEnabled(const CXFA_FFDoc* hDoc) const = 0;
    virtual void SetCalculationsEnabled(CXFA_FFDoc* hDoc, bool bEnabled) = 0;
    virtual WideString GetTitle(const CXFA_FFDoc* hDoc) const = 0;
    virtual void SetTitle(CXFA_FFDoc* hDoc, const WideString& wsTitle) = 0;
    virtual void ExportData(CXFA_FFDoc* hDoc,
                            const WideString& wsFilePath,
                            bool bXDP) = 0;
    virtual void GotoURL(CXFA_FFDoc* hDoc, const WideString& bsURL) = 0;
    virtual bool IsValidationsEnabled(const CXFA_FFDoc* hDoc) const = 0;
    virtual void SetValidationsEnabled(CXFA_FFDoc* hDoc, bool bEnabled) = 0;
    virtual void SetFocusWidget(CXFA_FFDoc* hDoc, CXFA_FFWidget* hWidget) = 0;
    virtual void Print(CXFA_FFDoc* hDoc,
                       int32_t nStartPage,
                       int32_t nEndPage,
                       Mask<XFA_PrintOpt> dwOptions) = 0;
    virtual FX_ARGB GetHighlightColor(const CXFA_FFDoc* hDoc) const = 0;
    virtual IJS_Runtime* GetIJSRuntime(const CXFA_FFDoc* hDoc) const = 0;
    virtual CFX_XMLDocument* GetXMLDoc() const = 0;
    virtual RetainPtr<IFX_SeekableReadStream> OpenLinkedFile(
        CXFA_FFDoc* hDoc,
        const WideString& wsLink) = 0;

#ifdef PDF_XFA_ELEMENT_SUBMIT_ENABLED
    virtual bool Submit(CXFA_FFDoc* hDoc, CXFA_Submit* submit) = 0;
#endif  // PDF_XFA_ELEMENT_SUBMIT_ENABLED
  };

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
  void OnPageViewEvent(CXFA_FFPageView* pPageView, PageViewEvent eEvent);
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
  void Print(int32_t nStartPage,
             int32_t nEndPage,
             Mask<XFA_PrintOpt> dwOptions);
  FX_ARGB GetHighlightColor() const;
  IJS_Runtime* GetIJSRuntime() const;
  CFX_XMLDocument* GetXMLDocument() const;
  RetainPtr<IFX_SeekableReadStream> OpenLinkedFile(const WideString& wsLink);

  CXFA_FFDocView* CreateDocView();
  FormType GetFormType() const { return m_FormType; }
  cppgc::Heap* GetHeap() const { return m_pHeap; }
  CXFA_Document* GetXFADoc() const { return m_pDocument; }
  CXFA_FFApp* GetApp() const { return m_pApp; }
  CPDF_Document* GetPDFDoc() const { return m_pPDFDoc; }
  CXFA_FFDocView* GetDocView(CXFA_LayoutProcessor* pLayout);
  CXFA_FFDocView* GetDocView();
  RetainPtr<CFGAS_GEFont> GetPDFFont(const WideString& family,
                                     uint32_t styles,
                                     bool strict);
  RetainPtr<CFX_DIBitmap> GetPDFNamedImage(WideStringView wsName,
                                           int32_t& iImageXDpi,
                                           int32_t& iImageYDpi);

  bool SavePackage(CXFA_Node* pNode,
                   const RetainPtr<IFX_SeekableStream>& pFile);

 private:
  CXFA_FFDoc(CXFA_FFApp* pApp,
             CallbackIface* pDocEnvironment,
             CPDF_Document* pPDFDoc,
             cppgc::Heap* pHeap);
  bool BuildDoc(CFX_XMLDocument* pXML);

  UnownedPtr<CallbackIface> const m_pDocEnvironment;
  UnownedPtr<CPDF_Document> const m_pPDFDoc;
  UnownedPtr<cppgc::Heap> const m_pHeap;
  cppgc::Member<CXFA_FFApp> const m_pApp;
  cppgc::Member<CXFA_FFNotify> m_pNotify;
  cppgc::Member<CXFA_Document> m_pDocument;
  cppgc::Member<CXFA_FFDocView> m_DocView;
  std::unique_ptr<CFGAS_PDFFontMgr> m_pPDFFontMgr;
  std::map<uint32_t, FX_IMAGEDIB_AND_DPI> m_HashToDibDpiMap;
  FormType m_FormType = FormType::kXFAForeground;
};

#endif  // XFA_FXFA_CXFA_FFDOC_H_
