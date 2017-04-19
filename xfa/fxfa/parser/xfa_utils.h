// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_XFA_UTILS_H_
#define XFA_FXFA_PARSER_XFA_UTILS_H_

#include "core/fxcrt/cfx_seekablestreamproxy.h"
#include "xfa/fxfa/fxfa_basic.h"

class CFX_XMLElement;
class CFX_XMLNode;
class CXFA_LocaleValue;
class CXFA_Node;
class CXFA_WidgetData;

double XFA_GetFractionalScale(uint32_t idx);
int XFA_GetMaxFractionalScale();

bool XFA_FDEExtension_ResolveNamespaceQualifier(
    CFX_XMLElement* pNode,
    const CFX_WideStringC& wsQualifier,
    CFX_WideString* wsNamespaceURI);

template <class NodeType, class TraverseStrategy>
class CXFA_NodeIteratorTemplate {
 public:
  explicit CXFA_NodeIteratorTemplate(NodeType* pRoot)
      : m_pRoot(pRoot), m_pCurrent(pRoot) {}

  NodeType* GetRoot() const { return m_pRoot; }
  NodeType* GetCurrent() const { return m_pCurrent; }

  void Reset() { m_pCurrent = m_pRoot; }
  bool SetCurrent(NodeType* pNode) {
    if (!RootReachableFromNode(pNode)) {
      m_pCurrent = nullptr;
      return false;
    }
    m_pCurrent = pNode;
    return true;
  }

  NodeType* MoveToPrev() {
    if (!m_pRoot)
      return nullptr;
    if (!m_pCurrent) {
      m_pCurrent = LastDescendant(m_pRoot);
      return m_pCurrent;
    }
    NodeType* pSibling = PreviousSiblingWithinSubtree(m_pCurrent);
    if (pSibling) {
      m_pCurrent = LastDescendant(pSibling);
      return m_pCurrent;
    }
    NodeType* pParent = ParentWithinSubtree(m_pCurrent);
    if (pParent) {
      m_pCurrent = pParent;
      return m_pCurrent;
    }
    return nullptr;
  }

  NodeType* MoveToNext() {
    if (!m_pRoot || !m_pCurrent)
      return nullptr;
    NodeType* pChild = TraverseStrategy::GetFirstChild(m_pCurrent);
    if (pChild) {
      m_pCurrent = pChild;
      return m_pCurrent;
    }
    return SkipChildrenAndMoveToNext();
  }

  NodeType* SkipChildrenAndMoveToNext() {
    if (!m_pRoot)
      return nullptr;
    NodeType* pNode = m_pCurrent;
    while (pNode) {
      NodeType* pSibling = NextSiblingWithinSubtree(pNode);
      if (pSibling) {
        m_pCurrent = pSibling;
        return m_pCurrent;
      }
      pNode = ParentWithinSubtree(pNode);
    }
    m_pCurrent = nullptr;
    return m_pCurrent;
  }

 private:
  bool RootReachableFromNode(NodeType* pNode) {
    if (!pNode)
      return false;
    if (pNode == m_pRoot)
      return true;
    return RootReachableFromNode(TraverseStrategy::GetParent(pNode));
  }

  NodeType* ParentWithinSubtree(NodeType* pNode) {
    if (!pNode || pNode == m_pRoot)
      return nullptr;
    return TraverseStrategy::GetParent(pNode);
  }

  NodeType* NextSiblingWithinSubtree(NodeType* pNode) {
    if (pNode == m_pRoot)
      return nullptr;
    return TraverseStrategy::GetNextSibling(pNode);
  }

  NodeType* PreviousSiblingWithinSubtree(NodeType* pNode) {
    NodeType* pParent = ParentWithinSubtree(pNode);
    if (!pParent)
      return nullptr;
    NodeType* pCurrent = TraverseStrategy::GetFirstChild(pParent);
    NodeType* pPrevious = nullptr;
    while (pCurrent != pNode) {
      pPrevious = pCurrent;
      pCurrent = TraverseStrategy::GetNextSibling(pCurrent);
    }
    return pPrevious;
  }

  NodeType* LastChild(NodeType* pNode) {
    NodeType* pPrevious = nullptr;
    NodeType* pChild = TraverseStrategy::GetFirstChild(pNode);
    while (pChild) {
      pPrevious = pChild;
      pChild = NextSiblingWithinSubtree(pChild);
    }
    return pPrevious;
  }

  NodeType* LastDescendant(NodeType* pNode) {
    NodeType* pChild = LastChild(pNode);
    return pChild ? LastDescendant(pChild) : pNode;
  }

  NodeType* m_pRoot;
  NodeType* m_pCurrent;
};

CXFA_LocaleValue XFA_GetLocaleValue(CXFA_WidgetData* pWidgetData);
double XFA_ByteStringToDouble(const CFX_ByteStringC& szStringVal);
int32_t XFA_MapRotation(int32_t nRotation);

bool XFA_RecognizeRichText(CFX_XMLElement* pRichTextXMLNode);
void XFA_GetPlainTextFromRichText(CFX_XMLNode* pXMLNode,
                                  CFX_WideString& wsPlainText);
bool XFA_FieldIsMultiListBox(CXFA_Node* pFieldNode);

void XFA_DataExporter_DealWithDataGroupNode(CXFA_Node* pDataNode);
void XFA_DataExporter_RegenerateFormFile(
    CXFA_Node* pNode,
    const CFX_RetainPtr<CFX_SeekableStreamProxy>& pStream,
    const char* pChecksum = nullptr,
    bool bSaveXML = false);

const XFA_NOTSUREATTRIBUTE* XFA_GetNotsureAttribute(
    XFA_Element eElement,
    XFA_ATTRIBUTE eAttribute,
    XFA_ATTRIBUTETYPE eType = XFA_ATTRIBUTETYPE_NOTSURE);

const XFA_SCRIPTATTRIBUTEINFO* XFA_GetScriptAttributeByName(
    XFA_Element eElement,
    const CFX_WideStringC& wsAttributeName);

const XFA_PROPERTY* XFA_GetPropertyOfElement(XFA_Element eElement,
                                             XFA_Element eProperty,
                                             uint32_t dwPacket);
const XFA_PROPERTY* XFA_GetElementProperties(XFA_Element eElement,
                                             int32_t& iCount);
const uint8_t* XFA_GetElementAttributes(XFA_Element eElement, int32_t& iCount);
const XFA_ELEMENTINFO* XFA_GetElementByID(XFA_Element eName);
XFA_Element XFA_GetElementTypeForName(const CFX_WideStringC& wsName);
CXFA_Measurement XFA_GetAttributeDefaultValue_Measure(XFA_Element eElement,
                                                      XFA_ATTRIBUTE eAttribute,
                                                      uint32_t dwPacket);
bool XFA_GetAttributeDefaultValue(void*& pValue,
                                  XFA_Element eElement,
                                  XFA_ATTRIBUTE eAttribute,
                                  XFA_ATTRIBUTETYPE eType,
                                  uint32_t dwPacket);
const XFA_ATTRIBUTEINFO* XFA_GetAttributeByName(const CFX_WideStringC& wsName);
const XFA_ATTRIBUTEINFO* XFA_GetAttributeByID(XFA_ATTRIBUTE eName);
const XFA_ATTRIBUTEENUMINFO* XFA_GetAttributeEnumByName(
    const CFX_WideStringC& wsName);
const XFA_PACKETINFO* XFA_GetPacketByIndex(XFA_PACKET ePacket);
const XFA_PACKETINFO* XFA_GetPacketByID(uint32_t dwPacket);

#endif  // XFA_FXFA_PARSER_XFA_UTILS_H_
