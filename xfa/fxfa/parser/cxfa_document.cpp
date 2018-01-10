// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_document.h"

#include "core/fxcrt/fx_extension.h"
#include "fxjs/cfxjse_engine.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/parser/cscript_datawindow.h"
#include "xfa/fxfa/parser/cscript_eventpseudomodel.h"
#include "xfa/fxfa/parser/cscript_hostpseudomodel.h"
#include "xfa/fxfa/parser/cscript_layoutpseudomodel.h"
#include "xfa/fxfa/parser/cscript_logpseudomodel.h"
#include "xfa/fxfa/parser/cscript_signaturepseudomodel.h"
#include "xfa/fxfa/parser/cxfa_datagroup.h"
#include "xfa/fxfa/parser/cxfa_document_parser.h"
#include "xfa/fxfa/parser/cxfa_interactive.h"
#include "xfa/fxfa/parser/cxfa_layoutprocessor.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_pdf.h"
#include "xfa/fxfa/parser/cxfa_present.h"
#include "xfa/fxfa/parser/cxfa_traversestrategy_xfanode.h"
#include "xfa/fxfa/parser/xfa_resolvenode_rs.h"
#include "xfa/fxfa/parser/xfa_utils.h"

namespace {

constexpr const wchar_t kTemplateNS[] =
    L"http://www.xfa.org/schema/xfa-template/";

void MergeNodeRecurse(CXFA_Document* pDocument,
                      CXFA_Node* pDestNodeParent,
                      CXFA_Node* pProtoNode) {
  CXFA_Node* pExistingNode = nullptr;
  for (CXFA_Node* pFormChild = pDestNodeParent->GetFirstChild(); pFormChild;
       pFormChild = pFormChild->GetNextSibling()) {
    if (pFormChild->GetElementType() == pProtoNode->GetElementType() &&
        pFormChild->GetNameHash() == pProtoNode->GetNameHash() &&
        pFormChild->IsUnusedNode()) {
      pFormChild->ClearFlag(XFA_NodeFlag_UnusedNode);
      pExistingNode = pFormChild;
      break;
    }
  }

  if (pExistingNode) {
    pExistingNode->SetTemplateNode(pProtoNode);
    for (CXFA_Node* pTemplateChild = pProtoNode->GetFirstChild();
         pTemplateChild; pTemplateChild = pTemplateChild->GetNextSibling()) {
      MergeNodeRecurse(pDocument, pExistingNode, pTemplateChild);
    }
    return;
  }
  CXFA_Node* pNewNode = pProtoNode->Clone(true);
  pNewNode->SetTemplateNode(pProtoNode);
  pDestNodeParent->InsertChild(pNewNode, nullptr);
}

void MergeNode(CXFA_Document* pDocument,
               CXFA_Node* pDestNode,
               CXFA_Node* pProtoNode) {
  {
    CXFA_NodeIterator sIterator(pDestNode);
    for (CXFA_Node* pNode = sIterator.GetCurrent(); pNode;
         pNode = sIterator.MoveToNext()) {
      pNode->SetFlag(XFA_NodeFlag_UnusedNode, true);
    }
  }
  pDestNode->SetTemplateNode(pProtoNode);
  for (CXFA_Node* pTemplateChild = pProtoNode->GetFirstChild(); pTemplateChild;
       pTemplateChild = pTemplateChild->GetNextSibling()) {
    MergeNodeRecurse(pDocument, pDestNode, pTemplateChild);
  }
  {
    CXFA_NodeIterator sIterator(pDestNode);
    for (CXFA_Node* pNode = sIterator.GetCurrent(); pNode;
         pNode = sIterator.MoveToNext()) {
      pNode->ClearFlag(XFA_NodeFlag_UnusedNode);
    }
  }
}

}  // namespace

CXFA_Document::CXFA_Document(CXFA_DocumentParser* pParser)
    : m_pParser(pParser),
      m_pRootNode(nullptr),
      m_eCurVersionMode(XFA_VERSION_DEFAULT),
      m_dwDocFlags(0) {
  ASSERT(m_pParser);
}

CXFA_Document::~CXFA_Document() {
  // Remove all the bindings before freeing the node as the ownership is wonky.
  if (m_pRootNode)
    m_pRootNode->ReleaseBindingNodes();

  delete m_pRootNode;

  for (CXFA_Node* pNode : m_PurgeNodes)
    delete pNode;
  m_PurgeNodes.clear();
}

CXFA_LayoutProcessor* CXFA_Document::GetLayoutProcessor() {
  if (!m_pLayoutProcessor)
    m_pLayoutProcessor = pdfium::MakeUnique<CXFA_LayoutProcessor>(this);
  return m_pLayoutProcessor.get();
}

CXFA_LayoutProcessor* CXFA_Document::GetDocLayout() {
  return GetLayoutProcessor();
}

void CXFA_Document::ClearLayoutData() {
  m_pLayoutProcessor.reset();
  m_pScriptContext.reset();
  m_pLocalMgr.reset();
  m_pScriptDataWindow.reset();
  m_pScriptEvent.reset();
  m_pScriptHost.reset();
  m_pScriptLog.reset();
  m_pScriptLayout.reset();
  m_pScriptSignature.reset();
}

void CXFA_Document::SetRoot(CXFA_Node* pNewRoot) {
  if (m_pRootNode)
    AddPurgeNode(m_pRootNode);

  m_pRootNode = pNewRoot;
  RemovePurgeNode(pNewRoot);
}

CFX_XMLDoc* CXFA_Document::GetXMLDoc() const {
  return m_pParser->GetXMLDoc();
}

CXFA_FFNotify* CXFA_Document::GetNotify() const {
  return m_pParser->GetNotify();
}

CXFA_Object* CXFA_Document::GetXFAObject(XFA_HashCode dwNodeNameHash) {
  switch (dwNodeNameHash) {
    case XFA_HASHCODE_Data: {
      CXFA_Node* pDatasetsNode = ToNode(GetXFAObject(XFA_HASHCODE_Datasets));
      if (!pDatasetsNode)
        return nullptr;

      for (CXFA_DataGroup* pDatasetsChild =
               pDatasetsNode->GetFirstChildByClass<CXFA_DataGroup>(
                   XFA_Element::DataGroup);
           pDatasetsChild;
           pDatasetsChild =
               pDatasetsChild->GetNextSameClassSibling<CXFA_DataGroup>(
                   XFA_Element::DataGroup)) {
        if (pDatasetsChild->GetNameHash() != XFA_HASHCODE_Data)
          continue;

        Optional<WideString> namespaceURI =
            pDatasetsChild->JSObject()->TryNamespace();
        if (!namespaceURI)
          continue;

        Optional<WideString> datasetsURI =
            pDatasetsNode->JSObject()->TryNamespace();
        if (!datasetsURI)
          continue;
        if (*namespaceURI == *datasetsURI)
          return pDatasetsChild;
      }
      return nullptr;
    }
    case XFA_HASHCODE_Record: {
      CXFA_Node* pData = ToNode(GetXFAObject(XFA_HASHCODE_Data));
      return pData ? pData->GetFirstChildByClass<CXFA_DataGroup>(
                         XFA_Element::DataGroup)
                   : nullptr;
    }
    case XFA_HASHCODE_DataWindow: {
      if (!m_pScriptDataWindow)
        m_pScriptDataWindow = pdfium::MakeUnique<CScript_DataWindow>(this);
      return m_pScriptDataWindow.get();
    }
    case XFA_HASHCODE_Event: {
      if (!m_pScriptEvent)
        m_pScriptEvent = pdfium::MakeUnique<CScript_EventPseudoModel>(this);
      return m_pScriptEvent.get();
    }
    case XFA_HASHCODE_Host: {
      if (!m_pScriptHost)
        m_pScriptHost = pdfium::MakeUnique<CScript_HostPseudoModel>(this);
      return m_pScriptHost.get();
    }
    case XFA_HASHCODE_Log: {
      if (!m_pScriptLog)
        m_pScriptLog = pdfium::MakeUnique<CScript_LogPseudoModel>(this);
      return m_pScriptLog.get();
    }
    case XFA_HASHCODE_Signature: {
      if (!m_pScriptSignature)
        m_pScriptSignature =
            pdfium::MakeUnique<CScript_SignaturePseudoModel>(this);
      return m_pScriptSignature.get();
    }
    case XFA_HASHCODE_Layout: {
      if (!m_pScriptLayout)
        m_pScriptLayout = pdfium::MakeUnique<CScript_LayoutPseudoModel>(this);
      return m_pScriptLayout.get();
    }
    default:
      return m_pRootNode->GetFirstChildByName(dwNodeNameHash);
  }
}

CXFA_Node* CXFA_Document::CreateNode(XFA_PacketType packet,
                                     XFA_Element eElement) {
  if (eElement == XFA_Element::Unknown)
    return nullptr;

  std::unique_ptr<CXFA_Node> pNode = CXFA_Node::Create(this, eElement, packet);
  if (!pNode)
    return nullptr;

  // TODO(dsinclair): AddPrugeNode should take ownership of the pointer.
  AddPurgeNode(pNode.get());
  return pNode.release();
}

void CXFA_Document::AddPurgeNode(CXFA_Node* pNode) {
  m_PurgeNodes.insert(pNode);
}

bool CXFA_Document::RemovePurgeNode(CXFA_Node* pNode) {
  return !!m_PurgeNodes.erase(pNode);
}

void CXFA_Document::SetFlag(uint32_t dwFlag, bool bOn) {
  if (bOn)
    m_dwDocFlags |= dwFlag;
  else
    m_dwDocFlags &= ~dwFlag;
}

bool CXFA_Document::IsInteractive() {
  if (m_dwDocFlags & XFA_DOCFLAG_HasInteractive)
    return !!(m_dwDocFlags & XFA_DOCFLAG_Interactive);

  CXFA_Node* pConfig = ToNode(GetXFAObject(XFA_HASHCODE_Config));
  if (!pConfig)
    return false;

  CXFA_Present* pPresent =
      pConfig->GetFirstChildByClass<CXFA_Present>(XFA_Element::Present);
  if (!pPresent)
    return false;

  CXFA_Pdf* pPDF = pPresent->GetFirstChildByClass<CXFA_Pdf>(XFA_Element::Pdf);
  if (!pPDF)
    return false;

  CXFA_Interactive* pFormFiller =
      pPDF->GetChild<CXFA_Interactive>(0, XFA_Element::Interactive, false);
  if (pFormFiller) {
    m_dwDocFlags |= XFA_DOCFLAG_HasInteractive;

    WideString wsInteractive = pFormFiller->JSObject()->GetContent(false);
    if (wsInteractive == L"1") {
      m_dwDocFlags |= XFA_DOCFLAG_Interactive;
      return true;
    }
  }
  return false;
}

CXFA_LocaleMgr* CXFA_Document::GetLocalMgr() {
  if (!m_pLocalMgr) {
    m_pLocalMgr = pdfium::MakeUnique<CXFA_LocaleMgr>(
        ToNode(GetXFAObject(XFA_HASHCODE_LocaleSet)),
        GetNotify()->GetAppProvider()->GetLanguage());
  }
  return m_pLocalMgr.get();
}

CFXJSE_Engine* CXFA_Document::InitScriptContext(v8::Isolate* pIsolate) {
  ASSERT(!m_pScriptContext);
  m_pScriptContext = pdfium::MakeUnique<CFXJSE_Engine>(this, pIsolate);
  return m_pScriptContext.get();
}

// We have to call |InitScriptContext| before any calls to |GetScriptContext|
// or the context won't have an isolate set into it.
CFXJSE_Engine* CXFA_Document::GetScriptContext() {
  ASSERT(m_pScriptContext);
  return m_pScriptContext.get();
}

XFA_VERSION CXFA_Document::RecognizeXFAVersionNumber(
    const WideString& wsTemplateNS) {
  WideStringView wsTemplateURIPrefix(kTemplateNS);
  size_t nPrefixLength = wsTemplateURIPrefix.GetLength();
  if (WideStringView(wsTemplateNS.c_str(), wsTemplateNS.GetLength()) !=
      wsTemplateURIPrefix) {
    return XFA_VERSION_UNKNOWN;
  }
  auto nDotPos = wsTemplateNS.Find('.', nPrefixLength);
  if (!nDotPos.has_value())
    return XFA_VERSION_UNKNOWN;

  int8_t iMajor = FXSYS_wtoi(
      wsTemplateNS.Mid(nPrefixLength, nDotPos.value() - nPrefixLength).c_str());
  int8_t iMinor =
      FXSYS_wtoi(wsTemplateNS
                     .Mid(nDotPos.value() + 1,
                          wsTemplateNS.GetLength() - nDotPos.value() - 2)
                     .c_str());
  XFA_VERSION eVersion = (XFA_VERSION)((int32_t)iMajor * 100 + iMinor);
  if (eVersion < XFA_VERSION_MIN || eVersion > XFA_VERSION_MAX)
    return XFA_VERSION_UNKNOWN;

  m_eCurVersionMode = eVersion;
  return eVersion;
}

CXFA_Node* CXFA_Document::GetNodeByID(CXFA_Node* pRoot,
                                      const WideStringView& wsID) {
  if (!pRoot || wsID.IsEmpty())
    return nullptr;

  CXFA_NodeIterator sIterator(pRoot);
  for (CXFA_Node* pNode = sIterator.GetCurrent(); pNode;
       pNode = sIterator.MoveToNext()) {
    WideString wsIDVal = pNode->JSObject()->GetCData(XFA_Attribute::Id);
    if (!wsIDVal.IsEmpty() && wsIDVal == wsID)
      return pNode;
  }
  return nullptr;
}

void CXFA_Document::DoProtoMerge() {
  CXFA_Node* pTemplateRoot = ToNode(GetXFAObject(XFA_HASHCODE_Template));
  if (!pTemplateRoot)
    return;

  std::map<uint32_t, CXFA_Node*> mIDMap;
  std::set<CXFA_Node*> sUseNodes;
  CXFA_NodeIterator sIterator(pTemplateRoot);
  for (CXFA_Node* pNode = sIterator.GetCurrent(); pNode;
       pNode = sIterator.MoveToNext()) {
    WideString wsIDVal = pNode->JSObject()->GetCData(XFA_Attribute::Id);
    if (!wsIDVal.IsEmpty())
      mIDMap[FX_HashCode_GetW(wsIDVal.AsStringView(), false)] = pNode;

    WideString wsUseVal = pNode->JSObject()->GetCData(XFA_Attribute::Use);
    if (!wsUseVal.IsEmpty()) {
      sUseNodes.insert(pNode);
    } else {
      wsUseVal = pNode->JSObject()->GetCData(XFA_Attribute::Usehref);
      if (!wsUseVal.IsEmpty())
        sUseNodes.insert(pNode);
    }
  }

  for (CXFA_Node* pUseHrefNode : sUseNodes) {
    WideStringView wsURI;
    WideStringView wsID;
    WideStringView wsSOM;

    WideString wsUseVal =
        pUseHrefNode->JSObject()->GetCData(XFA_Attribute::Usehref);
    if (!wsUseVal.IsEmpty()) {
      auto uSharpPos = wsUseVal.Find('#');
      if (!uSharpPos.has_value()) {
        wsURI = wsUseVal.AsStringView();
      } else {
        wsURI = WideStringView(wsUseVal.c_str(), uSharpPos.value());
        size_t uLen = wsUseVal.GetLength();
        if (uLen >= uSharpPos.value() + 5 &&
            WideStringView(wsUseVal.c_str() + uSharpPos.value(), 5) ==
                L"#som(" &&
            wsUseVal[uLen - 1] == ')') {
          wsSOM = WideStringView(wsUseVal.c_str() + uSharpPos.value() + 5,
                                 uLen - 1 - uSharpPos.value() - 5);
        } else {
          wsID = WideStringView(wsUseVal.c_str() + uSharpPos.value() + 1,
                                uLen - uSharpPos.value() - 1);
        }
      }
    } else {
      wsUseVal = pUseHrefNode->JSObject()->GetCData(XFA_Attribute::Use);
      if (!wsUseVal.IsEmpty()) {
        if (wsUseVal[0] == '#')
          wsID = WideStringView(wsUseVal.c_str() + 1, wsUseVal.GetLength() - 1);
        else
          wsSOM = WideStringView(wsUseVal.c_str(), wsUseVal.GetLength());
      }
    }

    if (!wsURI.IsEmpty() && wsURI != L".")
      continue;

    CXFA_Node* pProtoNode = nullptr;
    if (!wsSOM.IsEmpty()) {
      uint32_t dwFlag = XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Attributes |
                        XFA_RESOLVENODE_Properties | XFA_RESOLVENODE_Parent |
                        XFA_RESOLVENODE_Siblings;
      XFA_RESOLVENODE_RS resolveNodeRS;
      int32_t iRet = m_pScriptContext->ResolveObjects(
          pUseHrefNode, wsSOM, &resolveNodeRS, dwFlag, nullptr);
      if (iRet > 0 && resolveNodeRS.objects.front()->IsNode())
        pProtoNode = resolveNodeRS.objects.front()->AsNode();
    } else if (!wsID.IsEmpty()) {
      auto it = mIDMap.find(FX_HashCode_GetW(wsID, false));
      if (it == mIDMap.end())
        continue;
      pProtoNode = it->second;
    }
    if (!pProtoNode)
      continue;

    MergeNode(this, pUseHrefNode, pProtoNode);
  }
}
