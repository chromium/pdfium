// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffdoc.h"

#include <algorithm>
#include <memory>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fpdfdoc/cpdf_nametree.h"
#include "core/fxcrt/cfx_readonlymemorystream.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/xml/cfx_xmldocument.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "core/fxcrt/xml/cfx_xmlparser.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "fxjs/xfa/cjx_object.h"
#include "third_party/base/check.h"
#include "third_party/base/ptr_util.h"
#include "v8/include/cppgc/allocation.h"
#include "v8/include/cppgc/heap.h"
#include "xfa/fgas/font/cfgas_pdffontmgr.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/cxfa_fontmgr.h"
#include "xfa/fxfa/layout/cxfa_layoutprocessor.h"
#include "xfa/fxfa/parser/cxfa_acrobat.h"
#include "xfa/fxfa/parser/cxfa_acrobat7.h"
#include "xfa/fxfa/parser/cxfa_dataexporter.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_document_builder.h"
#include "xfa/fxfa/parser/cxfa_dynamicrender.h"
#include "xfa/fxfa/parser/cxfa_node.h"

FX_IMAGEDIB_AND_DPI::FX_IMAGEDIB_AND_DPI() = default;
FX_IMAGEDIB_AND_DPI::FX_IMAGEDIB_AND_DPI(const FX_IMAGEDIB_AND_DPI& that) =
    default;

FX_IMAGEDIB_AND_DPI::FX_IMAGEDIB_AND_DPI(const RetainPtr<CFX_DIBBase>& pDib,
                                         int32_t xDpi,
                                         int32_t yDpi)
    : pDibSource(pDib), iImageXDpi(xDpi), iImageYDpi(yDpi) {}

FX_IMAGEDIB_AND_DPI::~FX_IMAGEDIB_AND_DPI() = default;

CXFA_FFDoc::CXFA_FFDoc(CXFA_FFApp* pApp,
                       CallbackIface* pDocEnvironment,
                       CPDF_Document* pPDFDoc,
                       cppgc::Heap* pHeap)
    : m_pDocEnvironment(pDocEnvironment),
      m_pPDFDoc(pPDFDoc),
      m_pHeap(pHeap),
      m_pApp(pApp),
      m_pNotify(cppgc::MakeGarbageCollected<CXFA_FFNotify>(
          pHeap->GetAllocationHandle(),
          this)),
      m_pDocument(cppgc::MakeGarbageCollected<CXFA_Document>(
          pHeap->GetAllocationHandle(),
          m_pNotify,
          pHeap,
          cppgc::MakeGarbageCollected<CXFA_LayoutProcessor>(
              pHeap->GetAllocationHandle(),
              pHeap))) {}

CXFA_FFDoc::~CXFA_FFDoc() = default;

void CXFA_FFDoc::PreFinalize() {
  if (m_DocView)
    m_DocView->RunDocClose();

  if (m_pDocument)
    m_pDocument->ClearLayoutData();
}

void CXFA_FFDoc::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(m_pApp);
  visitor->Trace(m_pNotify);
  visitor->Trace(m_pDocument);
  visitor->Trace(m_DocView);
}

bool CXFA_FFDoc::BuildDoc(CFX_XMLDocument* pXML) {
  if (!pXML)
    return false;

  CXFA_DocumentBuilder builder(m_pDocument);
  if (!builder.BuildDocument(pXML, XFA_PacketType::Xdp))
    return false;

  m_pDocument->SetRoot(builder.GetRootNode());
  return true;
}

CXFA_FFDocView* CXFA_FFDoc::CreateDocView() {
  if (!m_DocView) {
    m_DocView = cppgc::MakeGarbageCollected<CXFA_FFDocView>(
        m_pHeap->GetAllocationHandle(), this);
  }
  return m_DocView;
}

void CXFA_FFDoc::SetChangeMark() {
  m_pDocEnvironment->SetChangeMark(this);
}

void CXFA_FFDoc::InvalidateRect(CXFA_FFPageView* pPageView,
                                const CFX_RectF& rt) {
  m_pDocEnvironment->InvalidateRect(pPageView, rt);
}

void CXFA_FFDoc::DisplayCaret(CXFA_FFWidget* hWidget,
                              bool bVisible,
                              const CFX_RectF* pRtAnchor) {
  return m_pDocEnvironment->DisplayCaret(hWidget, bVisible, pRtAnchor);
}

bool CXFA_FFDoc::GetPopupPos(CXFA_FFWidget* hWidget,
                             float fMinPopup,
                             float fMaxPopup,
                             const CFX_RectF& rtAnchor,
                             CFX_RectF* pPopupRect) const {
  return m_pDocEnvironment->GetPopupPos(hWidget, fMinPopup, fMaxPopup, rtAnchor,
                                        pPopupRect);
}

bool CXFA_FFDoc::PopupMenu(CXFA_FFWidget* hWidget, const CFX_PointF& ptPopup) {
  return m_pDocEnvironment->PopupMenu(hWidget, ptPopup);
}

void CXFA_FFDoc::OnPageViewEvent(CXFA_FFPageView* pPageView,
                                 PageViewEvent eEvent) {
  m_pDocEnvironment->OnPageViewEvent(pPageView, eEvent);
}

void CXFA_FFDoc::WidgetPostAdd(CXFA_FFWidget* hWidget) {
  m_pDocEnvironment->WidgetPostAdd(hWidget);
}

void CXFA_FFDoc::WidgetPreRemove(CXFA_FFWidget* hWidget) {
  m_pDocEnvironment->WidgetPreRemove(hWidget);
}

int32_t CXFA_FFDoc::CountPages() const {
  return m_pDocEnvironment->CountPages(this);
}

int32_t CXFA_FFDoc::GetCurrentPage() const {
  return m_pDocEnvironment->GetCurrentPage(this);
}

void CXFA_FFDoc::SetCurrentPage(int32_t iCurPage) {
  m_pDocEnvironment->SetCurrentPage(this, iCurPage);
}

bool CXFA_FFDoc::IsCalculationsEnabled() const {
  return m_pDocEnvironment->IsCalculationsEnabled(this);
}

void CXFA_FFDoc::SetCalculationsEnabled(bool bEnabled) {
  return m_pDocEnvironment->SetCalculationsEnabled(this, bEnabled);
}

WideString CXFA_FFDoc::GetTitle() const {
  return m_pDocEnvironment->GetTitle(this);
}

void CXFA_FFDoc::SetTitle(const WideString& wsTitle) {
  m_pDocEnvironment->SetTitle(this, wsTitle);
}

void CXFA_FFDoc::ExportData(const WideString& wsFilePath, bool bXDP) {
  m_pDocEnvironment->ExportData(this, wsFilePath, bXDP);
}

void CXFA_FFDoc::GotoURL(const WideString& bsURL) {
  m_pDocEnvironment->GotoURL(this, bsURL);
}

bool CXFA_FFDoc::IsValidationsEnabled() const {
  return m_pDocEnvironment->IsValidationsEnabled(this);
}

void CXFA_FFDoc::SetValidationsEnabled(bool bEnabled) {
  m_pDocEnvironment->SetValidationsEnabled(this, bEnabled);
}

void CXFA_FFDoc::SetFocusWidget(CXFA_FFWidget* hWidget) {
  m_pDocEnvironment->SetFocusWidget(this, hWidget);
}

void CXFA_FFDoc::Print(int32_t nStartPage,
                       int32_t nEndPage,
                       Mask<XFA_PrintOpt> dwOptions) {
  m_pDocEnvironment->Print(this, nStartPage, nEndPage, dwOptions);
}

FX_ARGB CXFA_FFDoc::GetHighlightColor() const {
  return m_pDocEnvironment->GetHighlightColor(this);
}

IJS_Runtime* CXFA_FFDoc::GetIJSRuntime() const {
  return m_pDocEnvironment->GetIJSRuntime(this);
}

CFX_XMLDocument* CXFA_FFDoc::GetXMLDocument() const {
  return m_pDocEnvironment->GetXMLDoc();
}

RetainPtr<IFX_SeekableReadStream> CXFA_FFDoc::OpenLinkedFile(
    const WideString& wsLink) {
  return m_pDocEnvironment->OpenLinkedFile(this, wsLink);
}

CXFA_FFDocView* CXFA_FFDoc::GetDocView(CXFA_LayoutProcessor* pLayout) {
  return m_DocView && m_DocView->GetLayoutProcessor() == pLayout ? m_DocView
                                                                 : nullptr;
}

CXFA_FFDocView* CXFA_FFDoc::GetDocView() {
  return m_DocView;
}

bool CXFA_FFDoc::OpenDoc(CFX_XMLDocument* pXML) {
  if (!BuildDoc(pXML))
    return false;

  // At this point we've got an XFA document and we want to always return
  // true to signify the load succeeded.
  m_pPDFFontMgr = std::make_unique<CFGAS_PDFFontMgr>(GetPDFDoc());
  m_FormType = FormType::kXFAForeground;
  CXFA_Node* pConfig = ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_Config));
  if (!pConfig)
    return true;

  CXFA_Acrobat* pAcrobat =
      pConfig->GetFirstChildByClass<CXFA_Acrobat>(XFA_Element::Acrobat);
  if (!pAcrobat)
    return true;

  CXFA_Acrobat7* pAcrobat7 =
      pAcrobat->GetFirstChildByClass<CXFA_Acrobat7>(XFA_Element::Acrobat7);
  if (!pAcrobat7)
    return true;

  CXFA_DynamicRender* pDynamicRender =
      pAcrobat7->GetFirstChildByClass<CXFA_DynamicRender>(
          XFA_Element::DynamicRender);
  if (!pDynamicRender)
    return true;

  WideString wsType = pDynamicRender->JSObject()->GetContent(false);
  if (wsType.EqualsASCII("required"))
    m_FormType = FormType::kXFAFull;

  return true;
}

RetainPtr<CFX_DIBitmap> CXFA_FFDoc::GetPDFNamedImage(WideStringView wsName,
                                                     int32_t& iImageXDpi,
                                                     int32_t& iImageYDpi) {
  uint32_t dwHash = FX_HashCode_GetW(wsName);
  auto it = m_HashToDibDpiMap.find(dwHash);
  if (it != m_HashToDibDpiMap.end()) {
    iImageXDpi = it->second.iImageXDpi;
    iImageYDpi = it->second.iImageYDpi;
    return it->second.pDibSource.As<CFX_DIBitmap>();
  }

  auto name_tree = CPDF_NameTree::Create(m_pPDFDoc.Get(), "XFAImages");
  size_t count = name_tree ? name_tree->GetCount() : 0;
  if (count == 0)
    return nullptr;

  CPDF_Object* pObject = name_tree->LookupValue(WideString(wsName));
  if (!pObject) {
    for (size_t i = 0; i < count; ++i) {
      WideString wsTemp;
      CPDF_Object* pTempObject = name_tree->LookupValueAndName(i, &wsTemp);
      if (wsTemp == wsName) {
        pObject = pTempObject;
        break;
      }
    }
  }

  CPDF_Stream* pStream = ToStream(pObject);
  if (!pStream)
    return nullptr;

  auto pAcc = pdfium::MakeRetain<CPDF_StreamAcc>(pStream);
  pAcc->LoadAllDataFiltered();

  auto pImageFileRead =
      pdfium::MakeRetain<CFX_ReadOnlyMemoryStream>(pAcc->GetSpan());

  RetainPtr<CFX_DIBitmap> pDibSource = XFA_LoadImageFromBuffer(
      pImageFileRead, FXCODEC_IMAGE_UNKNOWN, iImageXDpi, iImageYDpi);
  m_HashToDibDpiMap[dwHash] = {pDibSource, iImageXDpi, iImageYDpi};
  return pDibSource;
}

bool CXFA_FFDoc::SavePackage(CXFA_Node* pNode,
                             const RetainPtr<IFX_SeekableStream>& pFile) {
  DCHECK(pNode || GetXFADoc()->GetRoot());

  CXFA_DataExporter exporter;
  return exporter.Export(pFile, pNode ? pNode : GetXFADoc()->GetRoot());
}
