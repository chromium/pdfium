// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_utils.h"
#include "xfa/src/fxfa/src/common/xfa_object.h"
#include "xfa/src/fxfa/src/common/xfa_document.h"
#include "xfa/src/fxfa/src/common/xfa_parser.h"
#include "xfa/src/fxfa/src/common/xfa_script.h"
#include "xfa/src/fxfa/src/common/xfa_docdata.h"
#include "xfa/src/fxfa/src/common/xfa_doclayout.h"
#include "xfa/src/fxfa/src/common/xfa_localemgr.h"
#include "xfa/src/fxfa/src/common/xfa_fm2jsapi.h"
#include "xfa_basic_imp.h"
#include "xfa_document_layout_imp.h"
#include "xfa_script_datawindow.h"
#include "xfa_script_eventpseudomodel.h"
#include "xfa_script_hostpseudomodel.h"
#include "xfa_script_logpseudomodel.h"
#include "xfa_script_layoutpseudomodel.h"
#include "xfa_script_signaturepseudomodel.h"
CXFA_Document::CXFA_Document(IXFA_DocParser* pParser)
    : m_pParser(pParser),
      m_pScriptContext(nullptr),
      m_pLayoutProcessor(nullptr),
      m_pRootNode(nullptr),
      m_pLocalMgr(nullptr),
      m_pScriptDataWindow(nullptr),
      m_pScriptEvent(nullptr),
      m_pScriptHost(nullptr),
      m_pScriptLog(nullptr),
      m_pScriptLayout(nullptr),
      m_pScriptSignature(nullptr),
      m_eCurVersionMode(XFA_VERSION_DEFAULT),
      m_dwDocFlags(0) {
  ASSERT(m_pParser);
}
CXFA_Document::~CXFA_Document() {
  delete m_pRootNode;
  PurgeNodes();
}
void CXFA_Document::ClearLayoutData() {
  if (m_pLayoutProcessor) {
    delete m_pLayoutProcessor;
    m_pLayoutProcessor = NULL;
  }
  if (m_pScriptContext) {
    m_pScriptContext->Release();
    m_pScriptContext = NULL;
  }
  if (m_pLocalMgr) {
    delete m_pLocalMgr;
    m_pLocalMgr = NULL;
  }
  if (m_pScriptDataWindow) {
    delete m_pScriptDataWindow;
    m_pScriptDataWindow = NULL;
  }
  if (m_pScriptEvent) {
    delete m_pScriptEvent;
    m_pScriptEvent = NULL;
  }
  if (m_pScriptHost) {
    delete m_pScriptHost;
    m_pScriptHost = NULL;
  }
  if (m_pScriptLog) {
    delete m_pScriptLog;
    m_pScriptLog = NULL;
  }
  if (m_pScriptLayout) {
    delete m_pScriptLayout;
    m_pScriptLayout = NULL;
  }
  if (m_pScriptSignature) {
    delete m_pScriptSignature;
    m_pScriptSignature = NULL;
  }
}
void CXFA_Document::SetRoot(CXFA_Node* pNewRoot) {
  if (m_pRootNode) {
    AddPurgeNode(m_pRootNode);
  }
  m_pRootNode = pNewRoot;
  RemovePurgeNode(pNewRoot);
}
IXFA_Notify* CXFA_Document::GetNotify() const {
  return m_pParser->GetNotify();
}
CXFA_Object* CXFA_Document::GetXFANode(const CFX_WideStringC& wsNodeName) {
  return GetXFANode(
      FX_HashCode_String_GetW(wsNodeName.GetPtr(), wsNodeName.GetLength()));
}
CXFA_Object* CXFA_Document::GetXFANode(FX_DWORD dwNodeNameHash) {
  switch (dwNodeNameHash) {
    case XFA_HASHCODE_Data: {
      CXFA_Node* pDatasetsNode = (CXFA_Node*)GetXFANode(XFA_HASHCODE_Datasets);
      if (!pDatasetsNode) {
        return NULL;
      }
      for (CXFA_Node* pDatasetsChild =
               pDatasetsNode->GetFirstChildByClass(XFA_ELEMENT_DataGroup);
           pDatasetsChild;
           pDatasetsChild =
               pDatasetsChild->GetNextSameClassSibling(XFA_ELEMENT_DataGroup)) {
        if (pDatasetsChild->GetNameHash() != XFA_HASHCODE_Data) {
          continue;
        }
        CFX_WideString wsNamespaceURI;
        if (!pDatasetsChild->TryNamespace(wsNamespaceURI)) {
          continue;
        }
        CFX_WideString wsDatasetsURI;
        if (!pDatasetsNode->TryNamespace(wsDatasetsURI)) {
          continue;
        }
        if (wsNamespaceURI == wsDatasetsURI) {
          return pDatasetsChild;
        }
      }
    }
      return NULL;
    case XFA_HASHCODE_Record: {
      CXFA_Node* pData = (CXFA_Node*)GetXFANode(XFA_HASHCODE_Data);
      return pData ? pData->GetFirstChildByClass(XFA_ELEMENT_DataGroup) : NULL;
    }
    case XFA_HASHCODE_DataWindow: {
      if (m_pScriptDataWindow == NULL) {
        m_pScriptDataWindow = new CScript_DataWindow(this);
      }
      return m_pScriptDataWindow;
    }
    case XFA_HASHCODE_Event: {
      if (m_pScriptEvent == NULL) {
        m_pScriptEvent = new CScript_EventPseudoModel(this);
      }
      return m_pScriptEvent;
    }
    case XFA_HASHCODE_Host: {
      if (m_pScriptHost == NULL) {
        m_pScriptHost = new CScript_HostPseudoModel(this);
      }
      return m_pScriptHost;
    }
    case XFA_HASHCODE_Log: {
      if (m_pScriptLog == NULL) {
        m_pScriptLog = new CScript_LogPseudoModel(this);
      }
      return m_pScriptLog;
    }
    case XFA_HASHCODE_Signature: {
      if (m_pScriptSignature == NULL) {
        m_pScriptSignature = new CScript_SignaturePseudoModel(this);
      }
      return m_pScriptSignature;
    }
    case XFA_HASHCODE_Layout: {
      if (m_pScriptLayout == NULL) {
        m_pScriptLayout = new CScript_LayoutPseudoModel(this);
      }
      return m_pScriptLayout;
    }
    default:
      return m_pRootNode->GetFirstChildByName(dwNodeNameHash);
  }
}
CXFA_Node* CXFA_Document::CreateNode(FX_DWORD dwPacket, XFA_ELEMENT eElement) {
  XFA_LPCPACKETINFO pPacket = XFA_GetPacketByID(dwPacket);
  return CreateNode(pPacket, eElement);
}
CXFA_Node* CXFA_Document::CreateNode(XFA_LPCPACKETINFO pPacket,
                                     XFA_ELEMENT eElement) {
  if (pPacket == NULL) {
    return NULL;
  }
  XFA_LPCELEMENTINFO pElement = XFA_GetElementByID(eElement);
  if (pElement && (pElement->dwPackets & pPacket->eName)) {
    CXFA_Node* pNode = new CXFA_Node(this, pPacket->eName, pElement->eName);
    if (pNode) {
      AddPurgeNode(pNode);
    }
    return pNode;
  }
  return NULL;
}
void CXFA_Document::AddPurgeNode(CXFA_Node* pNode) {
  m_rgPurgeNodes.Add(pNode);
}
FX_BOOL CXFA_Document::RemovePurgeNode(CXFA_Node* pNode) {
  return m_rgPurgeNodes.RemoveKey(pNode);
}
void CXFA_Document::PurgeNodes() {
  FX_POSITION psNode = m_rgPurgeNodes.GetStartPosition();
  while (psNode) {
    CXFA_Node* pNode;
    m_rgPurgeNodes.GetNextAssoc(psNode, pNode);
    delete pNode;
  }
  m_rgPurgeNodes.RemoveAll();
}
void CXFA_Document::SetFlag(FX_DWORD dwFlag, FX_BOOL bOn) {
  if (bOn) {
    m_dwDocFlags |= dwFlag;
  } else {
    m_dwDocFlags &= ~dwFlag;
  }
}
FX_BOOL CXFA_Document::IsInteractive() {
  if (m_dwDocFlags & XFA_DOCFLAG_HasInteractive) {
    return m_dwDocFlags & XFA_DOCFLAG_Interactive;
  }
  CXFA_Node* pConfig = (CXFA_Node*)this->GetXFANode(XFA_HASHCODE_Config);
  if (!pConfig) {
    return FALSE;
  }
  CFX_WideString wsInteractive;
  CXFA_Node* pPresent = pConfig->GetFirstChildByClass(XFA_ELEMENT_Present);
  if (!pPresent) {
    return FALSE;
  }
  CXFA_Node* pPDF = pPresent->GetFirstChildByClass(XFA_ELEMENT_Pdf);
  if (!pPDF) {
    return FALSE;
  }
  CXFA_Node* pInteractive = pPDF->GetChild(0, XFA_ELEMENT_Interactive);
  if (pInteractive) {
    m_dwDocFlags |= XFA_DOCFLAG_HasInteractive;
    if (pInteractive->TryContent(wsInteractive) &&
        wsInteractive == FX_WSTRC(L"1")) {
      m_dwDocFlags |= XFA_DOCFLAG_Interactive;
      return TRUE;
    }
  }
  return FALSE;
}
CXFA_LocaleMgr* CXFA_Document::GetLocalMgr() {
  if (!m_pLocalMgr) {
    CFX_WideString wsLanguage;
    this->GetParser()->GetNotify()->GetAppProvider()->GetLanguage(wsLanguage);
    m_pLocalMgr = new CXFA_LocaleMgr(
        (CXFA_Node*)this->GetXFANode(XFA_HASHCODE_LocaleSet), wsLanguage);
  }
  return m_pLocalMgr;
}
IXFA_ScriptContext* CXFA_Document::InitScriptContext(FXJSE_HRUNTIME hRuntime) {
  if (!m_pScriptContext) {
    m_pScriptContext = XFA_ScriptContext_Create(this);
  }
  m_pScriptContext->Initialize(hRuntime);
  return m_pScriptContext;
}
IXFA_ScriptContext* CXFA_Document::GetScriptContext() {
  if (!m_pScriptContext) {
    m_pScriptContext = XFA_ScriptContext_Create(this);
  }
  return m_pScriptContext;
}
XFA_VERSION CXFA_Document::RecognizeXFAVersionNumber(
    CFX_WideString& wsTemplateNS) {
  CFX_WideStringC wsTemplateURIPrefix =
      XFA_GetPacketByIndex(XFA_PACKET_Template)->pURI;
  FX_STRSIZE nPrefixLength = wsTemplateURIPrefix.GetLength();
  if (CFX_WideStringC(wsTemplateNS, wsTemplateNS.GetLength()) !=
      wsTemplateURIPrefix) {
    return XFA_VERSION_UNKNOWN;
  }
  FX_STRSIZE nDotPos = wsTemplateNS.Find('.', nPrefixLength);
  if (nDotPos == (FX_STRSIZE)-1) {
    return XFA_VERSION_UNKNOWN;
  }
  int8_t iMajor =
      FXSYS_wtoi(wsTemplateNS.Mid(nPrefixLength, nDotPos - nPrefixLength));
  int8_t iMinor = FXSYS_wtoi(
      wsTemplateNS.Mid(nDotPos + 1, wsTemplateNS.GetLength() - nDotPos - 2));
  XFA_VERSION eVersion = (XFA_VERSION)((int32_t)iMajor * 100 + iMinor);
  if (eVersion < XFA_VERSION_MIN || eVersion > XFA_VERSION_MAX) {
    return XFA_VERSION_UNKNOWN;
  }
  m_eCurVersionMode = eVersion;
  return eVersion;
}
CXFA_Node* CXFA_Document::GetNodeByID(CXFA_Node* pRoot,
                                      const CFX_WideStringC& wsID) {
  if (!pRoot || wsID.IsEmpty()) {
    return NULL;
  }
  CXFA_NodeIterator sIterator(pRoot);
  for (CXFA_Node* pNode = sIterator.GetCurrent(); pNode;
       pNode = sIterator.MoveToNext()) {
    CFX_WideStringC wsIDVal;
    if (pNode->TryCData(XFA_ATTRIBUTE_Id, wsIDVal) && !wsIDVal.IsEmpty()) {
      if (wsIDVal == wsID) {
        return pNode;
      }
    }
  }
  return NULL;
}
static void XFA_ProtoMerge_MergeNodeRecurse(CXFA_Document* pDocument,
                                            CXFA_Node* pDestNodeParent,
                                            CXFA_Node* pProtoNode) {
  CXFA_Node* pExistingNode = NULL;
  for (CXFA_Node* pFormChild =
           pDestNodeParent->GetNodeItem(XFA_NODEITEM_FirstChild);
       pFormChild;
       pFormChild = pFormChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pFormChild->GetClassID() == pProtoNode->GetClassID() &&
        pFormChild->GetNameHash() == pProtoNode->GetNameHash() &&
        pFormChild->HasFlag(XFA_NODEFLAG_UnusedNode)) {
      pFormChild->SetFlag(XFA_NODEFLAG_UnusedNode, FALSE);
      pExistingNode = pFormChild;
      break;
    }
  }
  if (pExistingNode) {
    pExistingNode->SetTemplateNode(pProtoNode);
    for (CXFA_Node* pTemplateChild =
             pProtoNode->GetNodeItem(XFA_NODEITEM_FirstChild);
         pTemplateChild; pTemplateChild = pTemplateChild->GetNodeItem(
                             XFA_NODEITEM_NextSibling)) {
      XFA_ProtoMerge_MergeNodeRecurse(pDocument, pExistingNode, pTemplateChild);
    }
    return;
  }
  CXFA_Node* pNewNode = pProtoNode->Clone(TRUE);
  pNewNode->SetTemplateNode(pProtoNode);
  pDestNodeParent->InsertChild(pNewNode, NULL);
}
static void XFA_ProtoMerge_MergeNode(CXFA_Document* pDocument,
                                     CXFA_Node* pDestNode,
                                     CXFA_Node* pProtoNode) {
  {
    CXFA_NodeIterator sIterator(pDestNode);
    for (CXFA_Node* pNode = sIterator.GetCurrent(); pNode;
         pNode = sIterator.MoveToNext()) {
      pNode->SetFlag(XFA_NODEFLAG_UnusedNode);
    }
  }
  pDestNode->SetTemplateNode(pProtoNode);
  for (CXFA_Node* pTemplateChild =
           pProtoNode->GetNodeItem(XFA_NODEITEM_FirstChild);
       pTemplateChild;
       pTemplateChild = pTemplateChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    XFA_ProtoMerge_MergeNodeRecurse(pDocument, pDestNode, pTemplateChild);
  }
  {
    CXFA_NodeIterator sIterator(pDestNode);
    for (CXFA_Node* pNode = sIterator.GetCurrent(); pNode;
         pNode = sIterator.MoveToNext()) {
      pNode->SetFlag(XFA_NODEFLAG_UnusedNode, FALSE);
    }
  }
}
void CXFA_Document::DoProtoMerge() {
  CXFA_Node* pTemplateRoot = (CXFA_Node*)GetXFANode(XFA_HASHCODE_Template);
  if (!pTemplateRoot) {
    return;
  }
  CFX_MapPtrTemplate<FX_DWORD, CXFA_Node*> mIDMap;
  CXFA_NodeSet sUseNodes;
  CXFA_NodeIterator sIterator(pTemplateRoot);
  for (CXFA_Node* pNode = sIterator.GetCurrent(); pNode;
       pNode = sIterator.MoveToNext()) {
    CFX_WideStringC wsIDVal;
    if (pNode->TryCData(XFA_ATTRIBUTE_Id, wsIDVal) && !wsIDVal.IsEmpty()) {
      mIDMap[FX_HashCode_String_GetW(wsIDVal.GetPtr(), wsIDVal.GetLength())] =
          pNode;
    }
    CFX_WideStringC wsUseVal;
    if (pNode->TryCData(XFA_ATTRIBUTE_Use, wsUseVal) && !wsUseVal.IsEmpty()) {
      sUseNodes.Add(pNode);
    } else if (pNode->TryCData(XFA_ATTRIBUTE_Usehref, wsUseVal) &&
               !wsUseVal.IsEmpty()) {
      sUseNodes.Add(pNode);
    }
  }
  FX_POSITION pos = sUseNodes.GetStartPosition();
  while (pos) {
    CXFA_Node* pUseHrefNode = NULL;
    sUseNodes.GetNextAssoc(pos, pUseHrefNode);
    CFX_WideString wsUseVal;
    CFX_WideStringC wsURI, wsID, wsSOM;
    if (pUseHrefNode->TryCData(XFA_ATTRIBUTE_Usehref, wsUseVal) &&
        !wsUseVal.IsEmpty()) {
      FX_STRSIZE uSharpPos = wsUseVal.Find('#');
      if (uSharpPos < 0) {
        wsURI = wsUseVal;
      } else {
        wsURI = CFX_WideStringC((const FX_WCHAR*)wsUseVal, uSharpPos);
        FX_STRSIZE uLen = wsUseVal.GetLength();
        if (uLen >= uSharpPos + 5 &&
            CFX_WideStringC((const FX_WCHAR*)wsUseVal + uSharpPos, 5) ==
                FX_WSTRC(L"#som(") &&
            wsUseVal[uLen - 1] == ')') {
          wsSOM = CFX_WideStringC((const FX_WCHAR*)wsUseVal + uSharpPos + 5,
                                  uLen - 1 - uSharpPos - 5);
        } else {
          wsID = CFX_WideStringC((const FX_WCHAR*)wsUseVal + uSharpPos + 1,
                                 uLen - uSharpPos - 1);
        }
      }
    } else if (pUseHrefNode->TryCData(XFA_ATTRIBUTE_Use, wsUseVal) &&
               !wsUseVal.IsEmpty()) {
      if (wsUseVal[0] == '#') {
        wsID = CFX_WideStringC((const FX_WCHAR*)wsUseVal + 1,
                               wsUseVal.GetLength() - 1);
      } else {
        wsSOM =
            CFX_WideStringC((const FX_WCHAR*)wsUseVal, wsUseVal.GetLength());
      }
    }
    if (!wsURI.IsEmpty() && wsURI != FX_WSTRC(L".")) {
      continue;
    }
    CXFA_Node* pProtoNode = NULL;
    if (!wsSOM.IsEmpty()) {
      FX_DWORD dwFlag = XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Attributes |
                        XFA_RESOLVENODE_Properties | XFA_RESOLVENODE_Parent |
                        XFA_RESOLVENODE_Siblings;
      XFA_RESOLVENODE_RS resoveNodeRS;
      int32_t iRet = m_pScriptContext->ResolveObjects(pUseHrefNode, wsSOM,
                                                      resoveNodeRS, dwFlag);
      if (iRet > 0 && resoveNodeRS.nodes[0]->IsNode()) {
        pProtoNode = (CXFA_Node*)resoveNodeRS.nodes[0];
      }
    } else if (!wsID.IsEmpty()) {
      if (!mIDMap.Lookup(
              FX_HashCode_String_GetW(wsID.GetPtr(), wsID.GetLength()),
              pProtoNode)) {
        continue;
      }
    }
    if (!pProtoNode) {
      continue;
    }
    XFA_ProtoMerge_MergeNode(this, pUseHrefNode, pProtoNode);
  }
}
