// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_common.h"
#include "xfa_ffapp.h"
#include "xfa_ffdoc.h"
#include "xfa_ffdocview.h"
#include "xfa_ffwidget.h"
#include "xfa_ffnotify.h"
#include "xfa_fontmgr.h"
CXFA_FFDoc::CXFA_FFDoc(CXFA_FFApp* pApp, IXFA_DocProvider* pDocProvider)
    : m_pDocProvider(pDocProvider),
      m_pDocument(nullptr),
      m_pStream(nullptr),
      m_pApp(pApp),
      m_pNotify(nullptr),
      m_pPDFDoc(nullptr),
      m_dwDocType(XFA_DOCTYPE_Static),
      m_bOwnStream(TRUE) {
}
CXFA_FFDoc::~CXFA_FFDoc() {
  CloseDoc();
}
FX_DWORD CXFA_FFDoc::GetDocType() {
  return m_dwDocType;
}
int32_t CXFA_FFDoc::StartLoad() {
  m_pNotify = new CXFA_FFNotify(this);
  IXFA_DocParser* pDocParser = IXFA_DocParser::Create(m_pNotify);
  int32_t iStatus = pDocParser->StartParse(m_pStream);
  m_pDocument = pDocParser->GetDocument();
  return iStatus;
}
FX_BOOL XFA_GetPDFContentsFromPDFXML(IFDE_XMLNode* pPDFElement,
                                     uint8_t*& pByteBuffer,
                                     int32_t& iBufferSize) {
  IFDE_XMLElement* pDocumentElement = NULL;
  for (IFDE_XMLNode* pXMLNode =
           pPDFElement->GetNodeItem(IFDE_XMLNode::FirstChild);
       pXMLNode; pXMLNode = pXMLNode->GetNodeItem(IFDE_XMLNode::NextSibling)) {
    if (pXMLNode->GetType() == FDE_XMLNODE_Element) {
      CFX_WideString wsTagName;
      IFDE_XMLElement* pXMLElement = (IFDE_XMLElement*)pXMLNode;
      pXMLElement->GetTagName(wsTagName);
      if (wsTagName.Equal(FX_WSTRC(L"document"))) {
        pDocumentElement = pXMLElement;
        break;
      }
    }
  }
  if (!pDocumentElement) {
    return FALSE;
  }
  IFDE_XMLElement* pChunkElement = NULL;
  for (IFDE_XMLNode* pXMLNode =
           pDocumentElement->GetNodeItem(IFDE_XMLNode::FirstChild);
       pXMLNode; pXMLNode = pXMLNode->GetNodeItem(IFDE_XMLNode::NextSibling)) {
    if (pXMLNode->GetType() == FDE_XMLNODE_Element) {
      CFX_WideString wsTagName;
      IFDE_XMLElement* pXMLElement = (IFDE_XMLElement*)pXMLNode;
      pXMLElement->GetTagName(wsTagName);
      if (wsTagName.Equal(FX_WSTRC(L"chunk"))) {
        pChunkElement = pXMLElement;
        break;
      }
    }
  }
  if (!pChunkElement) {
    return FALSE;
  }
  CFX_WideString wsPDFContent;
  pChunkElement->GetTextData(wsPDFContent);
  iBufferSize = FX_Base64DecodeW(wsPDFContent, wsPDFContent.GetLength(), NULL);
  pByteBuffer = FX_Alloc(uint8_t, iBufferSize + 1);
  pByteBuffer[iBufferSize] = '0';  // FIXME: I bet this is wrong.
  FX_Base64DecodeW(wsPDFContent, wsPDFContent.GetLength(), pByteBuffer);
  return TRUE;
}
void XFA_XPDPacket_MergeRootNode(CXFA_Node* pOriginRoot, CXFA_Node* pNewRoot) {
  CXFA_Node* pChildNode = pNewRoot->GetNodeItem(XFA_NODEITEM_FirstChild);
  while (pChildNode) {
    CXFA_Node* pOriginChild =
        pOriginRoot->GetFirstChildByName(pChildNode->GetNameHash());
    if (pOriginChild) {
      pChildNode = pChildNode->GetNodeItem(XFA_NODEITEM_NextSibling);
    } else {
      CXFA_Node* pNextSibling =
          pChildNode->GetNodeItem(XFA_NODEITEM_NextSibling);
      pNewRoot->RemoveChild(pChildNode);
      pOriginRoot->InsertChild(pChildNode);
      pChildNode = pNextSibling;
      pNextSibling = NULL;
    }
  }
}
int32_t CXFA_FFDoc::DoLoad(IFX_Pause* pPause) {
  int32_t iStatus = m_pDocument->GetParser()->DoParse(pPause);
  if (iStatus == XFA_PARSESTATUS_Done && !m_pPDFDoc) {
    CXFA_Node* pPDFNode = (CXFA_Node*)m_pDocument->GetXFANode(XFA_HASHCODE_Pdf);
    if (!pPDFNode) {
      return XFA_PARSESTATUS_SyntaxErr;
    }
    IFDE_XMLNode* pPDFXML = pPDFNode->GetXMLMappingNode();
    if (pPDFXML->GetType() != FDE_XMLNODE_Element) {
      return XFA_PARSESTATUS_SyntaxErr;
    }
    int32_t iBufferSize = 0;
    uint8_t* pByteBuffer = NULL;
    IFX_FileRead* pXFAReader = NULL;
    if (XFA_GetPDFContentsFromPDFXML(pPDFXML, pByteBuffer, iBufferSize)) {
      pXFAReader = FX_CreateMemoryStream(pByteBuffer, iBufferSize, TRUE);
      if (!pXFAReader) {
        if (pByteBuffer) {
          FX_Free(pByteBuffer);
          pByteBuffer = NULL;
        }
        return XFA_PARSESTATUS_SyntaxErr;
      }
    } else {
      CFX_WideString wsHref;
      ((IFDE_XMLElement*)pPDFXML)->GetString(L"href", wsHref);
      if (!wsHref.IsEmpty()) {
        pXFAReader = GetDocProvider()->OpenLinkedFile(this, wsHref);
      }
    }
    if (!pXFAReader) {
      return XFA_PARSESTATUS_SyntaxErr;
    }
    CPDF_Document* pPDFDocument =
        GetDocProvider()->OpenPDF(this, pXFAReader, TRUE);
    FXSYS_assert(!m_pPDFDoc);
    if (!OpenDoc(pPDFDocument)) {
      return XFA_PARSESTATUS_SyntaxErr;
    }
    IXFA_Parser* pParser = IXFA_Parser::Create(m_pDocument, TRUE);
    if (!pParser) {
      return XFA_PARSESTATUS_SyntaxErr;
    }
    CXFA_Node* pRootNode = NULL;
    if (pParser->StartParse(m_pStream) == XFA_PARSESTATUS_Ready &&
        pParser->DoParse(NULL) == XFA_PARSESTATUS_Done) {
      pRootNode = pParser->GetRootNode();
    }
    if (pRootNode && m_pDocument->GetRoot()) {
      XFA_XPDPacket_MergeRootNode(m_pDocument->GetRoot(), pRootNode);
      iStatus = XFA_PARSESTATUS_Done;
    } else {
      iStatus = XFA_PARSESTATUS_StatusErr;
    }
    pParser->Release();
    pParser = NULL;
  }
  return iStatus;
}
void CXFA_FFDoc::StopLoad() {
  m_pApp->GetXFAFontMgr()->LoadDocFonts(this);
  m_dwDocType = XFA_DOCTYPE_Static;
  CXFA_Node* pConfig = (CXFA_Node*)m_pDocument->GetXFANode(XFA_HASHCODE_Config);
  if (!pConfig) {
    return;
  }
  CXFA_Node* pAcrobat = pConfig->GetFirstChildByClass(XFA_ELEMENT_Acrobat);
  if (!pAcrobat) {
    return;
  }
  CXFA_Node* pAcrobat7 = pAcrobat->GetFirstChildByClass(XFA_ELEMENT_Acrobat7);
  if (!pAcrobat7) {
    return;
  }
  CXFA_Node* pDynamicRender =
      pAcrobat7->GetFirstChildByClass(XFA_ELEMENT_DynamicRender);
  if (!pDynamicRender) {
    return;
  }
  CFX_WideString wsType;
  if (pDynamicRender->TryContent(wsType) && wsType == FX_WSTRC(L"required")) {
    m_dwDocType = XFA_DOCTYPE_Dynamic;
  }
}
IXFA_DocView* CXFA_FFDoc::CreateDocView(FX_DWORD dwView) {
  CXFA_FFDocView* pDocView =
      (CXFA_FFDocView*)m_mapTypeToDocView.GetValueAt((void*)(uintptr_t)dwView);
  if (!pDocView) {
    pDocView = new CXFA_FFDocView(this);
    m_mapTypeToDocView.SetAt((void*)(uintptr_t)dwView, pDocView);
  }
  return pDocView;
}
CXFA_FFDocView* CXFA_FFDoc::GetDocView(IXFA_DocLayout* pLayout) {
  FX_POSITION ps = m_mapTypeToDocView.GetStartPosition();
  while (ps) {
    void* pType;
    CXFA_FFDocView* pDocView;
    m_mapTypeToDocView.GetNextAssoc(ps, pType, (void*&)pDocView);
    if (pDocView->GetXFALayout() == pLayout) {
      return pDocView;
    }
  }
  return NULL;
}
CXFA_FFDocView* CXFA_FFDoc::GetDocView() {
  FX_POSITION ps = m_mapTypeToDocView.GetStartPosition();
  if (ps) {
    void* pType;
    CXFA_FFDocView* pDocView;
    m_mapTypeToDocView.GetNextAssoc(ps, pType, (void*&)pDocView);
    return pDocView;
  }
  return NULL;
}
FX_BOOL CXFA_FFDoc::OpenDoc(IFX_FileRead* pStream, FX_BOOL bTakeOverFile) {
  m_bOwnStream = bTakeOverFile;
  m_pStream = pStream;
  return TRUE;
}
FX_BOOL CXFA_FFDoc::OpenDoc(CPDF_Document* pPDFDoc) {
  if (pPDFDoc == NULL) {
    return FALSE;
  }
  CPDF_Dictionary* pRoot = pPDFDoc->GetRoot();
  if (pRoot == NULL) {
    return FALSE;
  }
  CPDF_Dictionary* pAcroForm = pRoot->GetDict("AcroForm");
  if (pAcroForm == NULL) {
    return FALSE;
  }
  CPDF_Object* pElementXFA = pAcroForm->GetElementValue("XFA");
  if (pElementXFA == NULL) {
    return FALSE;
  }
  int32_t iObjType = pElementXFA->GetType();
  CFX_ArrayTemplate<CPDF_Stream*> xfaStreams;
  if (iObjType == PDFOBJ_ARRAY) {
    CPDF_Array* pXFAArray = (CPDF_Array*)pElementXFA;
    FX_DWORD count = pXFAArray->GetCount() / 2;
    for (FX_DWORD i = 0; i < count; i++) {
      CPDF_Stream* pStream = pXFAArray->GetStream(i * 2 + 1);
      if (pStream != NULL) {
        xfaStreams.Add(pStream);
      }
    }
  } else if (iObjType == PDFOBJ_STREAM) {
    xfaStreams.Add((CPDF_Stream*)pElementXFA);
  }
  if (xfaStreams.GetSize() < 1) {
    return FALSE;
  }
  IFX_FileRead* pFileRead = new CXFA_FileRead(xfaStreams);
  m_pPDFDoc = pPDFDoc;
  if (m_pStream) {
    m_pStream->Release();
    m_pStream = NULL;
  }
  m_pStream = pFileRead;
  m_bOwnStream = TRUE;
  return TRUE;
}
FX_BOOL CXFA_FFDoc::CloseDoc() {
  FX_POSITION psClose = m_mapTypeToDocView.GetStartPosition();
  while (psClose) {
    void* pType;
    CXFA_FFDocView* pDocView;
    m_mapTypeToDocView.GetNextAssoc(psClose, pType, (void*&)pDocView);
    pDocView->RunDocClose();
  }
  if (m_pDocument) {
    m_pDocument->ClearLayoutData();
  }
  FX_POSITION ps = m_mapTypeToDocView.GetStartPosition();
  while (ps) {
    void* pType;
    CXFA_FFDocView* pDocView;
    m_mapTypeToDocView.GetNextAssoc(ps, pType, (void*&)pDocView);
    delete pDocView;
  }
  m_mapTypeToDocView.RemoveAll();
  if (m_pDocument) {
    IXFA_Parser* pParser = m_pDocument->GetParser();
    pParser->Release();
    m_pDocument = NULL;
  }
  if (m_pNotify) {
    delete m_pNotify;
    m_pNotify = NULL;
  }
  m_pApp->GetXFAFontMgr()->ReleaseDocFonts(this);
  if (m_dwDocType != XFA_DOCTYPE_XDP && m_pStream && m_bOwnStream) {
    m_pStream->Release();
    m_pStream = NULL;
  }
  ps = m_mapNamedImages.GetStartPosition();
  while (ps) {
    void* pName;
    FX_IMAGEDIB_AND_DPI* pImage = NULL;
    m_mapNamedImages.GetNextAssoc(ps, pName, (void*&)pImage);
    if (pImage) {
      delete pImage->pDibSource;
      pImage->pDibSource = NULL;
      FX_Free(pImage);
      pImage = NULL;
    }
  }
  m_mapNamedImages.RemoveAll();
  IFWL_NoteDriver* pNoteDriver = FWL_GetApp()->GetNoteDriver();
  pNoteDriver->ClearEventTargets(FALSE);
  return TRUE;
}
void CXFA_FFDoc::SetDocType(FX_DWORD dwType) {
  m_dwDocType = dwType;
}
CPDF_Document* CXFA_FFDoc::GetPDFDoc() {
  return m_pPDFDoc;
}
#define _FXLIB_NEW_VERSION_
CFX_DIBitmap* CXFA_FFDoc::GetPDFNamedImage(const CFX_WideStringC& wsName,
                                           int32_t& iImageXDpi,
                                           int32_t& iImageYDpi) {
  if (!m_pPDFDoc) {
    return NULL;
  }
  FX_DWORD dwHash =
      FX_HashCode_String_GetW(wsName.GetPtr(), wsName.GetLength(), FALSE);
  FX_IMAGEDIB_AND_DPI* imageDIBDpi = NULL;
  if (m_mapNamedImages.Lookup((void*)(uintptr_t)dwHash, (void*&)imageDIBDpi)) {
    iImageXDpi = imageDIBDpi->iImageXDpi;
    iImageYDpi = imageDIBDpi->iImageYDpi;
    return (CFX_DIBitmap*)imageDIBDpi->pDibSource;
  }
  CPDF_Dictionary* pRoot = m_pPDFDoc->GetRoot();
  if (pRoot == NULL) {
    return NULL;
  }
  CPDF_Dictionary* pNames = pRoot->GetDict("Names");
  if (!pNames) {
    return NULL;
  }
  CPDF_Dictionary* pXFAImages = pNames->GetDict("XFAImages");
  if (!pXFAImages) {
    return NULL;
  }
  CPDF_NameTree nametree(pXFAImages);
#ifdef _FXLIB_NEW_VERSION_
  CFX_ByteString bsName = PDF_EncodeText(wsName.GetPtr(), wsName.GetLength());
  CPDF_Object* pObject = nametree.LookupValue(bsName);
  if (!pObject) {
    int32_t iCount = nametree.GetCount();
    for (int32_t i = 0; i < iCount; i++) {
      CFX_ByteString bsTemp;
      CPDF_Object* pTempObject = nametree.LookupValue(i, bsTemp);
      if (bsTemp == bsName) {
        pObject = pTempObject;
        break;
      }
    }
  }
#else
  CPDF_Object* pObject = nametree.LookupValue(wsName);
  if (!pObject) {
    int32_t iCount = nametree.GetCount();
    for (int32_t i = 0; i < iCount; i++) {
      CFX_WideString wsTemp;
      CPDF_Object* pTempObject = nametree.LookupValue(i, wsTemp);
      if (wsTemp == wsName) {
        pObject = pTempObject;
        break;
      }
    }
  }
#endif
  if (!pObject || pObject->GetType() != PDFOBJ_STREAM) {
    return NULL;
  }
  if (!imageDIBDpi) {
    imageDIBDpi = FX_Alloc(FX_IMAGEDIB_AND_DPI, 1);
    imageDIBDpi->pDibSource = NULL;
    imageDIBDpi->iImageXDpi = 0;
    imageDIBDpi->iImageYDpi = 0;
    CPDF_StreamAcc streamAcc;
    streamAcc.LoadAllData((CPDF_Stream*)pObject);
    IFX_FileRead* pImageFileRead = FX_CreateMemoryStream(
        (uint8_t*)streamAcc.GetData(), streamAcc.GetSize());
    imageDIBDpi->pDibSource = XFA_LoadImageFromBuffer(
        pImageFileRead, FXCODEC_IMAGE_UNKNOWN, iImageXDpi, iImageYDpi);
    imageDIBDpi->iImageXDpi = iImageXDpi;
    imageDIBDpi->iImageYDpi = iImageYDpi;
    pImageFileRead->Release();
  }
  m_mapNamedImages.SetAt((void*)(uintptr_t)dwHash, imageDIBDpi);
  return (CFX_DIBitmap*)imageDIBDpi->pDibSource;
}
IFDE_XMLElement* CXFA_FFDoc::GetPackageData(const CFX_WideStringC& wsPackage) {
  FX_DWORD packetHash =
      FX_HashCode_String_GetW(wsPackage.GetPtr(), wsPackage.GetLength());
  CXFA_Object* pObject = m_pDocument->GetXFANode(packetHash);
  CXFA_Node* pNode =
      (pObject && pObject->IsNode()) ? (CXFA_Node*)pObject : NULL;
  if (!pNode) {
    return NULL;
  }
  IFDE_XMLNode* pXMLNode = pNode->GetXMLMappingNode();
  return (pXMLNode && pXMLNode->GetType() == FDE_XMLNODE_Element)
             ? (IFDE_XMLElement*)pXMLNode
             : NULL;
}
FX_BOOL CXFA_FFDoc::SavePackage(const CFX_WideStringC& wsPackage,
                                IFX_FileWrite* pFile,
                                IXFA_ChecksumContext* pCSContext) {
  IXFA_PacketExport* pExport = IXFA_PacketExport::Create(m_pDocument);
  if (!pExport) {
    return FALSE;
  }
  FX_DWORD packetHash =
      FX_HashCode_String_GetW(wsPackage.GetPtr(), wsPackage.GetLength());
  CXFA_Node* pNode = NULL;
  if (packetHash == XFA_HASHCODE_Xfa) {
    pNode = m_pDocument->GetRoot();
  } else {
    CXFA_Object* pObject = m_pDocument->GetXFANode(packetHash);
    pNode = (pObject && pObject->IsNode()) ? (CXFA_Node*)pObject : NULL;
  }
  FX_BOOL bFlags = FALSE;
  if (pNode) {
    CFX_ByteString bsChecksum;
    if (pCSContext) {
      pCSContext->GetChecksum(bsChecksum);
    }
    bFlags = pExport->Export(pFile, pNode, 0, bsChecksum.GetLength()
                                                  ? (const FX_CHAR*)bsChecksum
                                                  : NULL);
  } else {
    bFlags = pExport->Export(pFile);
  }
  pExport->Release();
  return bFlags;
}
FX_BOOL CXFA_FFDoc::ImportData(IFX_FileRead* pStream, FX_BOOL bXDP) {
  FX_BOOL bRet = FALSE;
  IXFA_PacketImport* pImport = IXFA_PacketImport::Create(m_pDocument);
  if (pImport) {
    bRet = pImport->ImportData(pStream);
    pImport->Release();
  }
  return bRet;
}
