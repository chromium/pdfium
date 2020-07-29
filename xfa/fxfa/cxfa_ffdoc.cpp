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
#include "third_party/base/ptr_util.h"
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

// static
std::unique_ptr<CXFA_FFDoc> CXFA_FFDoc::CreateAndOpen(
    CXFA_FFApp* pApp,
    IXFA_DocEnvironment* pDocEnvironment,
    CPDF_Document* pPDFDoc,
    cppgc::Heap* pGCHeap,
    CFX_XMLDocument* pXML) {
  ASSERT(pApp);
  ASSERT(pDocEnvironment);
  ASSERT(pPDFDoc);

  // Use WrapUnique() to keep constructor private.
  auto result = pdfium::WrapUnique(
      new CXFA_FFDoc(pApp, pDocEnvironment, pPDFDoc, pGCHeap));
  if (!result->OpenDoc(pXML))
    return nullptr;

  return result;
}

CXFA_FFDoc::CXFA_FFDoc(CXFA_FFApp* pApp,
                       IXFA_DocEnvironment* pDocEnvironment,
                       CPDF_Document* pPDFDoc,
                       cppgc::Heap* pHeap)
    : m_pDocEnvironment(pDocEnvironment),
      m_pApp(pApp),
      m_pPDFDoc(pPDFDoc),
      m_pHeap(pHeap),
      m_pNotify(std::make_unique<CXFA_FFNotify>(this)),
      m_pDocument(std::make_unique<CXFA_Document>(
          m_pNotify.get(),
          std::make_unique<CXFA_LayoutProcessor>(pHeap))) {}

CXFA_FFDoc::~CXFA_FFDoc() {
  if (m_DocView) {
    m_DocView->RunDocClose();
    m_DocView.reset();
  }
  if (m_pDocument)
    m_pDocument->ClearLayoutData();

  m_pDocument.reset();
  m_pNotify.reset();
  m_pPDFFontMgr.reset();
  m_HashToDibDpiMap.clear();
}

bool CXFA_FFDoc::BuildDoc(CFX_XMLDocument* pXML) {
  if (!pXML)
    return false;

  CXFA_DocumentBuilder builder(m_pDocument.get());
  if (!builder.BuildDocument(pXML, XFA_PacketType::Xdp))
    return false;

  m_pDocument->SetRoot(builder.GetRootNode());
  return true;
}

CXFA_FFDocView* CXFA_FFDoc::CreateDocView() {
  if (!m_DocView)
    m_DocView = std::make_unique<CXFA_FFDocView>(this);

  return m_DocView.get();
}

CXFA_FFDocView* CXFA_FFDoc::GetDocView(CXFA_LayoutProcessor* pLayout) {
  return m_DocView && m_DocView->GetLayoutProcessor() == pLayout
             ? m_DocView.get()
             : nullptr;
}

CXFA_FFDocView* CXFA_FFDoc::GetDocView() {
  return m_DocView.get();
}

bool CXFA_FFDoc::OpenDoc(CFX_XMLDocument* pXML) {
  if (!BuildDoc(pXML))
    return false;

  CFGAS_FontMgr* mgr = GetApp()->GetFDEFontMgr();
  if (!mgr)
    return false;

  // At this point we've got an XFA document and we want to always return
  // true to signify the load succeeded.
  m_pPDFFontMgr = std::make_unique<CFGAS_PDFFontMgr>(GetPDFDoc(), mgr);

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
  uint32_t dwHash = FX_HashCode_GetW(wsName, false);
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
  ASSERT(pNode || GetXFADoc()->GetRoot());

  CXFA_DataExporter exporter;
  return exporter.Export(pFile, pNode ? pNode : GetXFADoc()->GetRoot());
}
