// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffdoc.h"

#include <algorithm>
#include <memory>
#include <vector>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fpdfdoc/cpdf_nametree.h"
#include "core/fxcrt/cfx_checksumcontext.h"
#include "core/fxcrt/cfx_memorystream.h"
#include "core/fxcrt/cfx_seekablemultistream.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "fxjs/xfa/cjx_object.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/cxfa_fontmgr.h"
#include "xfa/fxfa/parser/cxfa_acrobat.h"
#include "xfa/fxfa/parser/cxfa_acrobat7.h"
#include "xfa/fxfa/parser/cxfa_dataexporter.h"
#include "xfa/fxfa/parser/cxfa_dataimporter.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_dynamicrender.h"
#include "xfa/fxfa/parser/cxfa_node.h"

namespace {

struct FX_BASE64DATA {
  uint32_t data1 : 2;
  uint32_t data2 : 6;
  uint32_t data3 : 4;
  uint32_t data4 : 4;
  uint32_t data5 : 6;
  uint32_t data6 : 2;
  uint32_t data7 : 8;
};

const uint8_t kStartValuesRemoved = 43;
const uint8_t kDecoderMapSize = 80;
const uint8_t g_FXBase64DecoderMap[kDecoderMapSize] = {
    0x3E, 0xFF, 0xFF, 0xFF, 0x3F, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A,
    0x3B, 0x3C, 0x3D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01,
    0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
    0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B,
    0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33,
};

uint8_t base64DecoderValue(uint8_t val) {
  if (val < kStartValuesRemoved || val >= kStartValuesRemoved + kDecoderMapSize)
    return 0xFF;
  return g_FXBase64DecoderMap[val - kStartValuesRemoved];
}

void Base64DecodePiece(const char src[4],
                       int32_t iChars,
                       FX_BASE64DATA& dst,
                       int32_t& iBytes) {
  ASSERT(iChars > 0 && iChars < 5);
  iBytes = 1;
  dst.data2 = base64DecoderValue(static_cast<uint8_t>(src[0]));
  if (iChars > 1) {
    uint8_t b = base64DecoderValue(static_cast<uint8_t>(src[1]));
    dst.data1 = b >> 4;
    dst.data4 = b;
    if (iChars > 2) {
      iBytes = 2;
      b = base64DecoderValue(static_cast<uint8_t>(src[2]));
      dst.data3 = b >> 2;
      dst.data6 = b;
      if (iChars > 3) {
        iBytes = 3;
        dst.data5 = base64DecoderValue(static_cast<uint8_t>(src[3]));
      } else {
        dst.data5 = 0;
      }
    } else {
      dst.data3 = 0;
    }
  } else {
    dst.data1 = 0;
  }
}

int32_t Base64DecodeW(const wchar_t* pSrc, int32_t iSrcLen, uint8_t* pDst) {
  ASSERT(pSrc);
  if (iSrcLen < 1) {
    return 0;
  }
  while (iSrcLen > 0 && pSrc[iSrcLen - 1] == '=') {
    iSrcLen--;
  }
  if (iSrcLen < 1) {
    return 0;
  }
  if (!pDst) {
    int32_t iDstLen = iSrcLen / 4 * 3;
    iSrcLen %= 4;
    if (iSrcLen == 1) {
      iDstLen += 1;
    } else if (iSrcLen == 2) {
      iDstLen += 1;
    } else if (iSrcLen == 3) {
      iDstLen += 2;
    }
    return iDstLen;
  }
  char srcData[4];
  FX_BASE64DATA dstData;
  int32_t iChars = 4, iBytes;
  uint8_t* pDstEnd = pDst;
  while (iSrcLen > 0) {
    if (iSrcLen > 3) {
      srcData[0] = (char)*pSrc++;
      srcData[1] = (char)*pSrc++;
      srcData[2] = (char)*pSrc++;
      srcData[3] = (char)*pSrc++;
      iSrcLen -= 4;
    } else {
      *((uint32_t*)&dstData) = 0;
      *((uint32_t*)srcData) = 0;
      srcData[0] = (char)*pSrc++;
      if (iSrcLen > 1) {
        srcData[1] = (char)*pSrc++;
      }
      if (iSrcLen > 2) {
        srcData[2] = (char)*pSrc++;
      }
      iChars = iSrcLen;
      iSrcLen = 0;
    }
    Base64DecodePiece(srcData, iChars, dstData, iBytes);
    *pDstEnd++ = ((uint8_t*)&dstData)[0];
    if (iBytes > 1) {
      *pDstEnd++ = ((uint8_t*)&dstData)[1];
    }
    if (iBytes > 2) {
      *pDstEnd++ = ((uint8_t*)&dstData)[2];
    }
  }
  return pDstEnd - pDst;
}

}  // namespace

CXFA_FFDoc::CXFA_FFDoc(CXFA_FFApp* pApp, IXFA_DocEnvironment* pDocEnvironment)
    : m_pDocEnvironment(pDocEnvironment), m_pApp(pApp) {}

CXFA_FFDoc::~CXFA_FFDoc() {
  CloseDoc();
}

int32_t CXFA_FFDoc::StartLoad() {
  m_pNotify = pdfium::MakeUnique<CXFA_FFNotify>(this);
  m_pDocumentParser = pdfium::MakeUnique<CXFA_DocumentParser>(m_pNotify.get());
  return m_pDocumentParser->StartParse(m_pStream, XFA_PacketType::Xdp);
}

bool XFA_GetPDFContentsFromPDFXML(CFX_XMLNode* pPDFElement,
                                  uint8_t*& pByteBuffer,
                                  int32_t& iBufferSize) {
  CFX_XMLElement* pDocumentElement = nullptr;
  for (CFX_XMLNode* pXMLNode =
           pPDFElement->GetNodeItem(CFX_XMLNode::FirstChild);
       pXMLNode; pXMLNode = pXMLNode->GetNodeItem(CFX_XMLNode::NextSibling)) {
    if (pXMLNode->GetType() == FX_XMLNODE_Element) {
      CFX_XMLElement* pXMLElement = static_cast<CFX_XMLElement*>(pXMLNode);
      WideString wsTagName = pXMLElement->GetName();
      if (wsTagName == L"document") {
        pDocumentElement = pXMLElement;
        break;
      }
    }
  }
  if (!pDocumentElement) {
    return false;
  }
  CFX_XMLElement* pChunkElement = nullptr;
  for (CFX_XMLNode* pXMLNode =
           pDocumentElement->GetNodeItem(CFX_XMLNode::FirstChild);
       pXMLNode; pXMLNode = pXMLNode->GetNodeItem(CFX_XMLNode::NextSibling)) {
    if (pXMLNode->GetType() == FX_XMLNODE_Element) {
      CFX_XMLElement* pXMLElement = static_cast<CFX_XMLElement*>(pXMLNode);
      WideString wsTagName = pXMLElement->GetName();
      if (wsTagName == L"chunk") {
        pChunkElement = pXMLElement;
        break;
      }
    }
  }
  if (!pChunkElement) {
    return false;
  }
  WideString wsPDFContent = pChunkElement->GetTextData();
  iBufferSize =
      Base64DecodeW(wsPDFContent.c_str(), wsPDFContent.GetLength(), nullptr);
  pByteBuffer = FX_Alloc(uint8_t, iBufferSize + 1);
  pByteBuffer[iBufferSize] = '0';  // FIXME: I bet this is wrong.
  Base64DecodeW(wsPDFContent.c_str(), wsPDFContent.GetLength(), pByteBuffer);
  return true;
}
void XFA_XPDPacket_MergeRootNode(CXFA_Node* pOriginRoot, CXFA_Node* pNewRoot) {
  CXFA_Node* pChildNode = pNewRoot->GetFirstChild();
  while (pChildNode) {
    CXFA_Node* pOriginChild =
        pOriginRoot->GetFirstChildByName(pChildNode->GetNameHash());
    if (pOriginChild) {
      pChildNode = pChildNode->GetNextSibling();
    } else {
      CXFA_Node* pNextSibling = pChildNode->GetNextSibling();
      pNewRoot->RemoveChild(pChildNode, true);
      pOriginRoot->InsertChild(pChildNode, nullptr);
      pChildNode = pNextSibling;
      pNextSibling = nullptr;
    }
  }
}

int32_t CXFA_FFDoc::DoLoad() {
  int32_t iStatus = m_pDocumentParser->DoParse();
  if (iStatus == XFA_PARSESTATUS_Done && !m_pPDFDoc)
    return XFA_PARSESTATUS_SyntaxErr;
  return iStatus;
}

void CXFA_FFDoc::StopLoad() {
  m_pPDFFontMgr = pdfium::MakeUnique<CFGAS_PDFFontMgr>(
      GetPDFDoc(), GetApp()->GetFDEFontMgr());

  m_FormType = FormType::kXFAForeground;
  CXFA_Node* pConfig = ToNode(
      m_pDocumentParser->GetDocument()->GetXFAObject(XFA_HASHCODE_Config));
  if (!pConfig)
    return;

  CXFA_Acrobat* pAcrobat =
      pConfig->GetFirstChildByClass<CXFA_Acrobat>(XFA_Element::Acrobat);
  if (!pAcrobat)
    return;

  CXFA_Acrobat7* pAcrobat7 =
      pAcrobat->GetFirstChildByClass<CXFA_Acrobat7>(XFA_Element::Acrobat7);
  if (!pAcrobat7)
    return;

  CXFA_DynamicRender* pDynamicRender =
      pAcrobat7->GetFirstChildByClass<CXFA_DynamicRender>(
          XFA_Element::DynamicRender);
  if (!pDynamicRender)
    return;

  WideString wsType = pDynamicRender->JSObject()->GetContent(false);
  if (wsType == L"required")
    m_FormType = FormType::kXFAFull;
}

CXFA_FFDocView* CXFA_FFDoc::CreateDocView() {
  if (!m_DocView)
    m_DocView = pdfium::MakeUnique<CXFA_FFDocView>(this);

  return m_DocView.get();
}

CXFA_FFDocView* CXFA_FFDoc::GetDocView(CXFA_LayoutProcessor* pLayout) {
  return m_DocView && m_DocView->GetXFALayout() == pLayout ? m_DocView.get()
                                                           : nullptr;
}

CXFA_FFDocView* CXFA_FFDoc::GetDocView() {
  return m_DocView.get();
}

bool CXFA_FFDoc::OpenDoc(CPDF_Document* pPDFDoc) {
  if (!pPDFDoc)
    return false;

  const CPDF_Dictionary* pRoot = pPDFDoc->GetRoot();
  if (!pRoot)
    return false;

  CPDF_Dictionary* pAcroForm = pRoot->GetDictFor("AcroForm");
  if (!pAcroForm)
    return false;

  CPDF_Object* pElementXFA = pAcroForm->GetDirectObjectFor("XFA");
  if (!pElementXFA)
    return false;

  std::vector<CPDF_Stream*> xfaStreams;
  if (pElementXFA->IsArray()) {
    CPDF_Array* pXFAArray = (CPDF_Array*)pElementXFA;
    for (size_t i = 0; i < pXFAArray->GetCount() / 2; i++) {
      if (CPDF_Stream* pStream = pXFAArray->GetStreamAt(i * 2 + 1))
        xfaStreams.push_back(pStream);
    }
  } else if (pElementXFA->IsStream()) {
    xfaStreams.push_back((CPDF_Stream*)pElementXFA);
  }
  if (xfaStreams.empty())
    return false;

  m_pPDFDoc = pPDFDoc;
  m_pStream = pdfium::MakeRetain<CFX_SeekableMultiStream>(xfaStreams);
  return true;
}

void CXFA_FFDoc::CloseDoc() {
  if (m_DocView) {
    m_DocView->RunDocClose();
    m_DocView.reset();
  }
  CXFA_Document* doc =
      m_pDocumentParser ? m_pDocumentParser->GetDocument() : nullptr;
  if (doc)
    doc->ClearLayoutData();

  m_pDocumentParser.reset();
  m_pNotify.reset();
  m_pPDFFontMgr.reset();
  m_HashToDibDpiMap.clear();
  m_pApp->ClearEventTargets();
}

RetainPtr<CFX_DIBitmap> CXFA_FFDoc::GetPDFNamedImage(
    const WideStringView& wsName,
    int32_t& iImageXDpi,
    int32_t& iImageYDpi) {
  if (!m_pPDFDoc)
    return nullptr;

  uint32_t dwHash = FX_HashCode_GetW(wsName, false);
  auto it = m_HashToDibDpiMap.find(dwHash);
  if (it != m_HashToDibDpiMap.end()) {
    iImageXDpi = it->second.iImageXDpi;
    iImageYDpi = it->second.iImageYDpi;
    return it->second.pDibSource.As<CFX_DIBitmap>();
  }

  const CPDF_Dictionary* pRoot = m_pPDFDoc->GetRoot();
  if (!pRoot)
    return nullptr;

  CPDF_Dictionary* pNames = pRoot->GetDictFor("Names");
  if (!pNames)
    return nullptr;

  CPDF_Dictionary* pXFAImages = pNames->GetDictFor("XFAImages");
  if (!pXFAImages)
    return nullptr;

  CPDF_NameTree nametree(pXFAImages);
  CPDF_Object* pObject = nametree.LookupValue(WideString(wsName));
  if (!pObject) {
    for (size_t i = 0; i < nametree.GetCount(); i++) {
      WideString wsTemp;
      CPDF_Object* pTempObject = nametree.LookupValueAndName(i, &wsTemp);
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

  RetainPtr<IFX_SeekableStream> pImageFileRead =
      pdfium::MakeRetain<CFX_MemoryStream>(
          const_cast<uint8_t*>(pAcc->GetData()), pAcc->GetSize(), false);

  RetainPtr<CFX_DIBitmap> pDibSource = XFA_LoadImageFromBuffer(
      pImageFileRead, FXCODEC_IMAGE_UNKNOWN, iImageXDpi, iImageYDpi);
  m_HashToDibDpiMap[dwHash] = {pDibSource, iImageXDpi, iImageYDpi};
  return pDibSource;
}

bool CXFA_FFDoc::SavePackage(CXFA_Node* pNode,
                             const RetainPtr<IFX_SeekableStream>& pFile,
                             CFX_ChecksumContext* pCSContext) {
  auto pExport = pdfium::MakeUnique<CXFA_DataExporter>(GetXFADoc());
  if (!pNode)
    return !!pExport->Export(pFile);

  ByteString bsChecksum;
  if (pCSContext)
    bsChecksum = pCSContext->GetChecksum();

  return !!pExport->Export(
      pFile, pNode, 0, bsChecksum.GetLength() ? bsChecksum.c_str() : nullptr);
}

bool CXFA_FFDoc::ImportData(const RetainPtr<IFX_SeekableStream>& pStream,
                            bool bXDP) {
  auto importer =
      pdfium::MakeUnique<CXFA_DataImporter>(m_pDocumentParser->GetDocument());
  return importer->ImportData(pStream);
}
