// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffdoc.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fpdfdoc/cpdf_nametree.h"
#include "core/fxcrt/cfx_read_only_span_stream.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/xml/cfx_xmldocument.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "core/fxcrt/xml/cfx_xmlparser.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "fxjs/xfa/cjx_object.h"
#include "v8/include/cppgc/allocation.h"
#include "v8/include/cppgc/heap.h"
#include "xfa/fgas/font/cfgas_gefont.h"
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
    : doc_environment_(pDocEnvironment),
      pdfdoc_(pPDFDoc),
      heap_(pHeap),
      app_(pApp),
      notify_(cppgc::MakeGarbageCollected<CXFA_FFNotify>(
          pHeap->GetAllocationHandle(),
          this)),
      document_(cppgc::MakeGarbageCollected<CXFA_Document>(
          pHeap->GetAllocationHandle(),
          notify_,
          pHeap,
          cppgc::MakeGarbageCollected<CXFA_LayoutProcessor>(
              pHeap->GetAllocationHandle(),
              pHeap))) {}

CXFA_FFDoc::~CXFA_FFDoc() = default;

void CXFA_FFDoc::PreFinalize() {
  if (doc_view_) {
    doc_view_->RunDocClose();
  }

  if (document_) {
    document_->ClearLayoutData();
  }
}

void CXFA_FFDoc::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(app_);
  visitor->Trace(notify_);
  visitor->Trace(document_);
  visitor->Trace(doc_view_);
}

bool CXFA_FFDoc::BuildDoc(CFX_XMLDocument* pXML) {
  DCHECK(pXML);

  CXFA_DocumentBuilder builder(document_);
  if (!builder.BuildDocument(pXML, XFA_PacketType::Xdp))
    return false;

  document_->SetRoot(builder.GetRootNode());
  return true;
}

CXFA_FFDocView* CXFA_FFDoc::CreateDocView() {
  if (!doc_view_) {
    doc_view_ = cppgc::MakeGarbageCollected<CXFA_FFDocView>(
        heap_->GetAllocationHandle(), this);
  }
  return doc_view_;
}

void CXFA_FFDoc::SetChangeMark() {
  doc_environment_->SetChangeMark(this);
}

void CXFA_FFDoc::InvalidateRect(CXFA_FFPageView* pPageView,
                                const CFX_RectF& rt) {
  doc_environment_->InvalidateRect(pPageView, rt);
}

void CXFA_FFDoc::DisplayCaret(CXFA_FFWidget* hWidget,
                              bool bVisible,
                              const CFX_RectF* pRtAnchor) {
  return doc_environment_->DisplayCaret(hWidget, bVisible, pRtAnchor);
}

bool CXFA_FFDoc::GetPopupPos(CXFA_FFWidget* hWidget,
                             float fMinPopup,
                             float fMaxPopup,
                             const CFX_RectF& rtAnchor,
                             CFX_RectF* pPopupRect) const {
  return doc_environment_->GetPopupPos(hWidget, fMinPopup, fMaxPopup, rtAnchor,
                                       pPopupRect);
}

bool CXFA_FFDoc::PopupMenu(CXFA_FFWidget* hWidget, const CFX_PointF& ptPopup) {
  return doc_environment_->PopupMenu(hWidget, ptPopup);
}

void CXFA_FFDoc::OnPageViewEvent(CXFA_FFPageView* pPageView,
                                 PageViewEvent eEvent) {
  doc_environment_->OnPageViewEvent(pPageView, eEvent);
}

void CXFA_FFDoc::WidgetPostAdd(CXFA_FFWidget* hWidget) {
  doc_environment_->WidgetPostAdd(hWidget);
}

void CXFA_FFDoc::WidgetPreRemove(CXFA_FFWidget* hWidget) {
  doc_environment_->WidgetPreRemove(hWidget);
}

int32_t CXFA_FFDoc::CountPages() const {
  return doc_environment_->CountPages(this);
}

int32_t CXFA_FFDoc::GetCurrentPage() const {
  return doc_environment_->GetCurrentPage(this);
}

void CXFA_FFDoc::SetCurrentPage(int32_t iCurPage) {
  doc_environment_->SetCurrentPage(this, iCurPage);
}

bool CXFA_FFDoc::IsCalculationsEnabled() const {
  return doc_environment_->IsCalculationsEnabled(this);
}

void CXFA_FFDoc::SetCalculationsEnabled(bool bEnabled) {
  return doc_environment_->SetCalculationsEnabled(this, bEnabled);
}

WideString CXFA_FFDoc::GetTitle() const {
  return doc_environment_->GetTitle(this);
}

void CXFA_FFDoc::SetTitle(const WideString& wsTitle) {
  doc_environment_->SetTitle(this, wsTitle);
}

void CXFA_FFDoc::ExportData(const WideString& wsFilePath, bool bXDP) {
  doc_environment_->ExportData(this, wsFilePath, bXDP);
}

void CXFA_FFDoc::GotoURL(const WideString& bsURL) {
  doc_environment_->GotoURL(this, bsURL);
}

bool CXFA_FFDoc::IsValidationsEnabled() const {
  return doc_environment_->IsValidationsEnabled(this);
}

void CXFA_FFDoc::SetValidationsEnabled(bool bEnabled) {
  doc_environment_->SetValidationsEnabled(this, bEnabled);
}

void CXFA_FFDoc::SetFocusWidget(CXFA_FFWidget* hWidget) {
  doc_environment_->SetFocusWidget(this, hWidget);
}

void CXFA_FFDoc::Print(int32_t nStartPage,
                       int32_t nEndPage,
                       Mask<XFA_PrintOpt> dwOptions) {
  doc_environment_->Print(this, nStartPage, nEndPage, dwOptions);
}

FX_ARGB CXFA_FFDoc::GetHighlightColor() const {
  return doc_environment_->GetHighlightColor(this);
}

IJS_Runtime* CXFA_FFDoc::GetIJSRuntime() const {
  return doc_environment_->GetIJSRuntime(this);
}

CFX_XMLDocument* CXFA_FFDoc::GetXMLDocument() const {
  return doc_environment_->GetXMLDoc();
}

RetainPtr<IFX_SeekableReadStream> CXFA_FFDoc::OpenLinkedFile(
    const WideString& wsLink) {
  return doc_environment_->OpenLinkedFile(this, wsLink);
}

CXFA_FFDocView* CXFA_FFDoc::GetDocView(CXFA_LayoutProcessor* pLayout) {
  return doc_view_ && doc_view_->GetLayoutProcessor() == pLayout ? doc_view_
                                                                 : nullptr;
}

CXFA_FFDocView* CXFA_FFDoc::GetDocView() {
  return doc_view_;
}

bool CXFA_FFDoc::OpenDoc(CFX_XMLDocument* pXML) {
  if (!BuildDoc(pXML))
    return false;

  // At this point we've got an XFA document and we want to always return
  // true to signify the load succeeded.
  pdffont_mgr_ = std::make_unique<CFGAS_PDFFontMgr>(GetPDFDoc());
  form_type_ = FormType::kXFAForeground;
  CXFA_Node* pConfig = ToNode(document_->GetXFAObject(XFA_HASHCODE_Config));
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
    form_type_ = FormType::kXFAFull;

  return true;
}

RetainPtr<CFGAS_GEFont> CXFA_FFDoc::GetPDFFont(const WideString& family,
                                               uint32_t styles,
                                               bool strict) {
  if (!pdffont_mgr_) {
    return nullptr;
  }

  return pdffont_mgr_->GetFont(family, styles, strict);
}

RetainPtr<CFX_DIBitmap> CXFA_FFDoc::GetPDFNamedImage(WideStringView wsName,
                                                     int32_t& iImageXDpi,
                                                     int32_t& iImageYDpi) {
  uint32_t dwHash = FX_HashCode_GetW(wsName);
  auto it = hash_to_dib_dpi_map_.find(dwHash);
  if (it != hash_to_dib_dpi_map_.end()) {
    iImageXDpi = it->second.iImageXDpi;
    iImageYDpi = it->second.iImageYDpi;
    return it->second.pDibSource.As<CFX_DIBitmap>();
  }

  auto name_tree = CPDF_NameTree::Create(pdfdoc_, "XFAImages");
  size_t count = name_tree ? name_tree->GetCount() : 0;
  if (count == 0)
    return nullptr;

  RetainPtr<const CPDF_Object> pObject =
      name_tree->LookupValue(WideString(wsName));
  if (!pObject) {
    for (size_t i = 0; i < count; ++i) {
      WideString wsTemp;
      RetainPtr<CPDF_Object> pTempObject =
          name_tree->LookupValueAndName(i, &wsTemp);
      if (wsTemp == wsName) {
        pObject = std::move(pTempObject);
        break;
      }
    }
  }

  RetainPtr<const CPDF_Stream> pStream = ToStream(pObject);
  if (!pStream)
    return nullptr;

  auto pAcc = pdfium::MakeRetain<CPDF_StreamAcc>(std::move(pStream));
  pAcc->LoadAllDataFiltered();

  auto pImageFileRead =
      pdfium::MakeRetain<CFX_ReadOnlySpanStream>(pAcc->GetSpan());
  RetainPtr<CFX_DIBitmap> pDibSource = XFA_LoadImageFromBuffer(
      std::move(pImageFileRead), FXCODEC_IMAGE_UNKNOWN, iImageXDpi, iImageYDpi);
  hash_to_dib_dpi_map_[dwHash] = {pDibSource, iImageXDpi, iImageYDpi};
  return pDibSource;
}

bool CXFA_FFDoc::SavePackage(CXFA_Node* pNode,
                             const RetainPtr<IFX_SeekableStream>& pFile) {
  DCHECK(pNode || GetXFADoc()->GetRoot());

  CXFA_DataExporter exporter;
  return exporter.Export(pFile, pNode ? pNode : GetXFADoc()->GetRoot());
}
