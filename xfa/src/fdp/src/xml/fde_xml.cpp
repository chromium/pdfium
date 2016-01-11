// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <algorithm>

#include "xfa/src/foxitlib.h"
#include "fde_xml.h"
#ifdef __cplusplus
extern "C" {
#endif
#define FDE_XMLVALIDCHARRANGENUM 5
static FX_WCHAR g_XMLValidCharRange[FDE_XMLVALIDCHARRANGENUM][2] = {
    {0x09, 0x09},
    {0x0A, 0x0A},
    {0x0D, 0x0D},
    {0x20, 0xD7FF},
    {0xE000, 0xFFFD}};
FX_BOOL FDE_IsXMLValidChar(FX_WCHAR ch) {
  int32_t iStart = 0, iEnd = FDE_XMLVALIDCHARRANGENUM - 1, iMid;
  while (iStart <= iEnd) {
    iMid = (iStart + iEnd) / 2;
    if (ch < g_XMLValidCharRange[iMid][0]) {
      iEnd = iMid - 1;
    } else if (ch > g_XMLValidCharRange[iMid][1]) {
      iStart = iMid + 1;
    } else {
      return TRUE;
    }
  }
  return FALSE;
}
FX_BOOL FDE_IsXMLWhiteSpace(FX_WCHAR ch) {
  return ch == L' ' || ch == 0x0A || ch == 0x0D || ch == 0x09;
}
typedef struct _FDE_XMLNAMECHAR {
  FX_WCHAR wStart;
  FX_WCHAR wEnd;
  FX_BOOL bStartChar;
} FDE_XMLNAMECHAR;
#define FDE_XMLNAMECHARSNUM 20
static FDE_XMLNAMECHAR g_XMLNameChars[FDE_XMLNAMECHARSNUM] = {
    {L'-', L'.', FALSE},    {L'0', L'9', FALSE},     {L':', L':', FALSE},
    {L'A', L'Z', TRUE},     {L'_', L'_', TRUE},      {L'a', L'z', TRUE},
    {0xB7, 0xB7, FALSE},    {0xC0, 0xD6, TRUE},      {0xD8, 0xF6, TRUE},
    {0xF8, 0x02FF, TRUE},   {0x0300, 0x036F, FALSE}, {0x0370, 0x037D, TRUE},
    {0x037F, 0x1FFF, TRUE}, {0x200C, 0x200D, TRUE},  {0x203F, 0x2040, FALSE},
    {0x2070, 0x218F, TRUE}, {0x2C00, 0x2FEF, TRUE},  {0x3001, 0xD7FF, TRUE},
    {0xF900, 0xFDCF, TRUE}, {0xFDF0, 0xFFFD, TRUE},
};
FX_BOOL FDE_IsXMLNameChar(FX_WCHAR ch, FX_BOOL bFirstChar) {
  int32_t iStart = 0, iEnd = FDE_XMLNAMECHARSNUM - 1, iMid;
  while (iStart <= iEnd) {
    iMid = (iStart + iEnd) / 2;
    if (ch < g_XMLNameChars[iMid].wStart) {
      iEnd = iMid - 1;
    } else if (ch > g_XMLNameChars[iMid].wEnd) {
      iStart = iMid + 1;
    } else {
      if (bFirstChar) {
        return g_XMLNameChars[iMid].bStartChar;
      }
      return TRUE;
    }
  }
  return FALSE;
}
#ifdef __cplusplus
}
#endif
CFDE_XMLNode::CFDE_XMLNode()
    : m_pParent(NULL), m_pChild(NULL), m_pPrior(NULL), m_pNext(NULL) {}
CFDE_XMLNode::~CFDE_XMLNode() {
  DeleteChildren();
}
void CFDE_XMLNode::DeleteChildren() {
  CFDE_XMLNode *pChild = m_pChild, *pTemp;
  while (pChild != NULL) {
    pTemp = pChild->m_pNext;
    pChild->Release();
    pChild = pTemp;
  }
  m_pChild = NULL;
}
int32_t CFDE_XMLNode::CountChildNodes() const {
  int32_t iCount = 0;
  CFDE_XMLNode* pChild = m_pChild;
  while (pChild != NULL) {
    iCount++;
    pChild = pChild->m_pNext;
  }
  return iCount;
}
CFDE_XMLNode* CFDE_XMLNode::GetChildNode(int32_t index) const {
  CFDE_XMLNode* pChild = m_pChild;
  while (pChild != NULL) {
    if (index == 0) {
      return pChild;
    }
    index--;
    pChild = pChild->m_pNext;
  }
  return NULL;
}
int32_t CFDE_XMLNode::GetChildNodeIndex(CFDE_XMLNode* pNode) const {
  int32_t index = 0;
  CFDE_XMLNode* pChild = m_pChild;
  while (pChild != NULL) {
    if (pChild == pNode) {
      return index;
    }
    index++;
    pChild = pChild->m_pNext;
  }
  return -1;
}
CFDE_XMLNode* CFDE_XMLNode::GetPath(const FX_WCHAR* pPath,
                                    int32_t iLength,
                                    FX_BOOL bQualifiedName) const {
  FXSYS_assert(pPath != NULL);
  if (iLength < 0) {
    iLength = FXSYS_wcslen(pPath);
  }
  if (iLength == 0) {
    return NULL;
  }
  CFX_WideString csPath;
  const FX_WCHAR* pStart = pPath;
  const FX_WCHAR* pEnd = pPath + iLength;
  FX_WCHAR ch;
  while (pStart < pEnd) {
    ch = *pStart++;
    if (ch == L'/') {
      break;
    } else {
      csPath += ch;
    }
  }
  iLength -= pStart - pPath;
  CFDE_XMLNode* pFind = NULL;
  if (csPath.GetLength() < 1) {
    pFind = GetNodeItem(IFDE_XMLNode::Root);
  } else if (csPath.Compare(L"..") == 0) {
    pFind = m_pParent;
  } else if (csPath.Compare(L".") == 0) {
    pFind = (CFDE_XMLNode*)this;
  } else {
    CFX_WideString wsTag;
    CFDE_XMLNode* pNode = m_pChild;
    while (pNode != NULL) {
      if (pNode->GetType() == FDE_XMLNODE_Element) {
        if (bQualifiedName) {
          ((CFDE_XMLElement*)pNode)->GetTagName(wsTag);
        } else {
          ((CFDE_XMLElement*)pNode)->GetLocalTagName(wsTag);
        }
        if (wsTag.Compare(csPath) == 0) {
          if (iLength < 1) {
            pFind = pNode;
          } else {
            pFind = pNode->GetPath(pStart, iLength, bQualifiedName);
          }
          if (pFind != NULL) {
            return pFind;
          }
        }
      }
      pNode = pNode->m_pNext;
    }
  }
  if (pFind == NULL || iLength < 1) {
    return pFind;
  }
  return pFind->GetPath(pStart, iLength, bQualifiedName);
}
int32_t CFDE_XMLNode::InsertChildNode(CFDE_XMLNode* pNode, int32_t index) {
  FXSYS_assert(pNode != NULL);
  pNode->m_pParent = this;
  if (m_pChild == NULL) {
    m_pChild = pNode;
    pNode->m_pPrior = NULL;
    pNode->m_pNext = NULL;
    return 0;
  } else if (index == 0) {
    pNode->m_pNext = m_pChild;
    pNode->m_pPrior = NULL;
    m_pChild->m_pPrior = pNode;
    m_pChild = pNode;
    return 0;
  }
  int32_t iCount = 0;
  CFDE_XMLNode* pFind = m_pChild;
  while (++iCount != index && pFind->m_pNext != NULL) {
    pFind = pFind->m_pNext;
  }
  pNode->m_pPrior = pFind;
  pNode->m_pNext = pFind->m_pNext;
  if (pFind->m_pNext != NULL) {
    pFind->m_pNext->m_pPrior = pNode;
  }
  pFind->m_pNext = pNode;
  return iCount;
}
void CFDE_XMLNode::RemoveChildNode(CFDE_XMLNode* pNode) {
  FXSYS_assert(m_pChild != NULL && pNode != NULL);
  if (m_pChild == pNode) {
    m_pChild = pNode->m_pNext;
  } else {
    pNode->m_pPrior->m_pNext = pNode->m_pNext;
  }
  if (pNode->m_pNext != NULL) {
    pNode->m_pNext->m_pPrior = pNode->m_pPrior;
  }
  pNode->m_pParent = NULL;
  pNode->m_pNext = NULL;
  pNode->m_pPrior = NULL;
}
CFDE_XMLNode* CFDE_XMLNode::GetNodeItem(IFDE_XMLNode::NodeItem eItem) const {
  switch (eItem) {
    case IFDE_XMLNode::Root: {
      CFDE_XMLNode* pParent = (CFDE_XMLNode*)this;
      while (pParent->m_pParent != NULL) {
        pParent = pParent->m_pParent;
      }
      return pParent;
    }
    case IFDE_XMLNode::Parent:
      return m_pParent;
    case IFDE_XMLNode::FirstSibling: {
      CFDE_XMLNode* pItem = (CFDE_XMLNode*)this;
      while (pItem->m_pPrior != NULL) {
        pItem = pItem->m_pPrior;
      }
      return pItem == (CFDE_XMLNode*)this ? NULL : pItem;
    }
    case IFDE_XMLNode::PriorSibling:
      return m_pPrior;
    case IFDE_XMLNode::NextSibling:
      return m_pNext;
    case IFDE_XMLNode::LastSibling: {
      CFDE_XMLNode* pItem = (CFDE_XMLNode*)this;
      while (pItem->m_pNext != NULL) {
        pItem = pItem->m_pNext;
      }
      return pItem == (CFDE_XMLNode*)this ? NULL : pItem;
    }
    case IFDE_XMLNode::FirstNeighbor: {
      CFDE_XMLNode* pParent = (CFDE_XMLNode*)this;
      while (pParent->m_pParent != NULL) {
        pParent = pParent->m_pParent;
      }
      return pParent == (CFDE_XMLNode*)this ? NULL : pParent;
    }
    case IFDE_XMLNode::PriorNeighbor: {
      if (m_pPrior == NULL) {
        return m_pParent;
      }
      CFDE_XMLNode* pItem = m_pPrior;
      while (CFDE_XMLNode* pTemp = pItem->m_pChild) {
        pItem = pTemp;
        while ((pTemp = pItem->m_pNext) != NULL) {
          pItem = pTemp;
        }
      }
      return pItem;
    }
    case IFDE_XMLNode::NextNeighbor: {
      if (m_pChild != NULL) {
        return m_pChild;
      }
      if (m_pNext != NULL) {
        return m_pNext;
      }
      CFDE_XMLNode* pItem = m_pParent;
      while (pItem != NULL) {
        if (pItem->m_pNext != NULL) {
          return pItem->m_pNext;
        }
        pItem = pItem->m_pParent;
      }
      return NULL;
    }
    case IFDE_XMLNode::LastNeighbor: {
      CFDE_XMLNode* pItem = (CFDE_XMLNode*)this;
      while (pItem->m_pParent != NULL) {
        pItem = pItem->m_pParent;
      }
      while (TRUE) {
        while (pItem->m_pNext != NULL) {
          pItem = pItem->m_pNext;
        }
        if (pItem->m_pChild == NULL) {
          break;
        }
        pItem = pItem->m_pChild;
      }
      return pItem == (CFDE_XMLNode*)this ? NULL : pItem;
    }
    case IFDE_XMLNode::FirstChild:
      return m_pChild;
    case IFDE_XMLNode::LastChild: {
      if (m_pChild == NULL) {
        return NULL;
      }
      CFDE_XMLNode* pChild = m_pChild;
      while (pChild->m_pNext != NULL) {
        pChild = pChild->m_pNext;
      }
      return pChild;
    }
    default:
      break;
  }
  return NULL;
}
int32_t CFDE_XMLNode::GetNodeLevel() const {
  int32_t iLevel = 0;
  CFDE_XMLNode* pItem = (CFDE_XMLNode*)this;
  while ((pItem = pItem->m_pParent) != NULL) {
    iLevel++;
  }
  return iLevel;
}
FX_BOOL CFDE_XMLNode::InsertNodeItem(IFDE_XMLNode::NodeItem eItem,
                                     CFDE_XMLNode* pNode) {
  FXSYS_assert(pNode != NULL);
  switch (eItem) {
    case IFDE_XMLNode::NextSibling: {
      pNode->m_pParent = m_pParent;
      pNode->m_pNext = m_pNext;
      pNode->m_pPrior = this;
      if (m_pNext) {
        m_pNext->m_pPrior = pNode;
      }
      m_pNext = pNode;
      return TRUE;
    }
    case IFDE_XMLNode::PriorSibling: {
      pNode->m_pParent = m_pParent;
      pNode->m_pNext = this;
      pNode->m_pPrior = m_pPrior;
      if (m_pPrior) {
        m_pPrior->m_pNext = pNode;
      } else if (m_pParent) {
        m_pParent->m_pChild = pNode;
      }
      m_pPrior = pNode;
      return TRUE;
    }
    default:
      return FALSE;
  }
  return FALSE;
}
CFDE_XMLNode* CFDE_XMLNode::RemoveNodeItem(IFDE_XMLNode::NodeItem eItem) {
  CFDE_XMLNode* pNode = NULL;
  switch (eItem) {
    case IFDE_XMLNode::NextSibling:
      if (m_pNext) {
        pNode = m_pNext;
        m_pNext = pNode->m_pNext;
        if (m_pNext) {
          m_pNext->m_pPrior = this;
        }
        pNode->m_pParent = NULL;
        pNode->m_pNext = NULL;
        pNode->m_pPrior = NULL;
      }
      break;
    default:
      break;
  }
  return pNode;
}
CFDE_XMLNode* CFDE_XMLNode::Clone(FX_BOOL bRecursive) {
  return NULL;
}
void CFDE_XMLNode::SaveXMLNode(IFX_Stream* pXMLStream) {
  CFDE_XMLNode* pNode = (CFDE_XMLNode*)this;
  FXSYS_assert(pXMLStream != NULL && pNode != NULL);
  switch (pNode->GetType()) {
    case FDE_XMLNODE_Instruction: {
      CFX_WideString ws;
      CFDE_XMLInstruction* pInstruction = (CFDE_XMLInstruction*)pNode;
      if (pInstruction->m_wsTarget.CompareNoCase(L"xml") == 0) {
        ws = L"<?xml version=\"1.0\" encoding=\"";
        FX_WORD wCodePage = pXMLStream->GetCodePage();
        if (wCodePage == FX_CODEPAGE_UTF16LE) {
          ws += L"UTF-16";
        } else if (wCodePage == FX_CODEPAGE_UTF16BE) {
          ws += L"UTF-16be";
        } else {
          ws += L"UTF-8";
        }
        ws += L"\"?>";
        pXMLStream->WriteString(ws, ws.GetLength());
      } else {
        ws.Format(L"<?%s", (const FX_WCHAR*)pInstruction->m_wsTarget);
        pXMLStream->WriteString(ws, ws.GetLength());
        CFX_WideStringArray& attributes = pInstruction->m_Attributes;
        int32_t i, iCount = attributes.GetSize();
        CFX_WideString wsValue;
        for (i = 0; i < iCount; i += 2) {
          ws = L" ";
          ws += attributes[i];
          ws += L"=\"";
          wsValue = attributes[i + 1];
          wsValue.Replace(L"&", L"&amp;");
          wsValue.Replace(L"<", L"&lt;");
          wsValue.Replace(L">", L"&gt;");
          wsValue.Replace(L"\'", L"&apos;");
          wsValue.Replace(L"\"", L"&quot;");
          ws += wsValue;
          ws += L"\"";
          pXMLStream->WriteString(ws, ws.GetLength());
        }
        CFX_WideStringArray& targetdata = pInstruction->m_TargetData;
        iCount = targetdata.GetSize();
        for (i = 0; i < iCount; i++) {
          ws = L" \"";
          ws += targetdata[i];
          ws += L"\"";
          pXMLStream->WriteString(ws, ws.GetLength());
        }
        ws = L"?>";
        pXMLStream->WriteString(ws, ws.GetLength());
      }
    } break;
    case FDE_XMLNODE_Element: {
      CFX_WideString ws;
      ws = L"<";
      ws += ((CFDE_XMLElement*)pNode)->m_wsTag;
      pXMLStream->WriteString(ws, ws.GetLength());
      CFX_WideStringArray& attributes = ((CFDE_XMLElement*)pNode)->m_Attributes;
      int32_t iCount = attributes.GetSize();
      CFX_WideString wsValue;
      for (int32_t i = 0; i < iCount; i += 2) {
        ws = L" ";
        ws += attributes[i];
        ws += L"=\"";
        wsValue = attributes[i + 1];
        wsValue.Replace(L"&", L"&amp;");
        wsValue.Replace(L"<", L"&lt;");
        wsValue.Replace(L">", L"&gt;");
        wsValue.Replace(L"\'", L"&apos;");
        wsValue.Replace(L"\"", L"&quot;");
        ws += wsValue;
        ws += L"\"";
        pXMLStream->WriteString(ws, ws.GetLength());
      }
      if (pNode->m_pChild == NULL) {
        ws = L"\n/>";
        pXMLStream->WriteString(ws, ws.GetLength());
      } else {
        ws = L"\n>";
        pXMLStream->WriteString(ws, ws.GetLength());
        CFDE_XMLNode* pChild = pNode->m_pChild;
        while (pChild != NULL) {
          pChild->SaveXMLNode(pXMLStream);
          pChild = pChild->m_pNext;
        }
        ws = L"</";
        ws += ((CFDE_XMLElement*)pNode)->m_wsTag;
        ws += L"\n>";
        pXMLStream->WriteString(ws, ws.GetLength());
      }
    } break;
    case FDE_XMLNODE_Text: {
      CFX_WideString ws = ((CFDE_XMLText*)pNode)->m_wsText;
      ws.Replace(L"&", L"&amp;");
      ws.Replace(L"<", L"&lt;");
      ws.Replace(L">", L"&gt;");
      ws.Replace(L"\'", L"&apos;");
      ws.Replace(L"\"", L"&quot;");
      pXMLStream->WriteString(ws, ws.GetLength());
    } break;
    case FDE_XMLNODE_CharData: {
      CFX_WideString ws = L"<![CDATA[";
      ws += ((CFDE_XMLCharData*)pNode)->m_wsCharData;
      ws += L"]]>";
      pXMLStream->WriteString(ws, ws.GetLength());
    } break;
    case FDE_XMLNODE_Unknown:
      break;
    default:
      break;
  }
}
void CFDE_XMLNode::CloneChildren(CFDE_XMLNode* pClone) {
  if (!m_pChild) {
    return;
  }
  CFDE_XMLNode* pNext = m_pChild;
  CFDE_XMLNode* pCloneNext = pNext->Clone(TRUE);
  pClone->InsertChildNode(pCloneNext);
  pNext = pNext->m_pNext;
  while (pNext) {
    CFDE_XMLNode* pChild = pNext->Clone(TRUE);
    pCloneNext->InsertNodeItem(IFDE_XMLNode::NextSibling, pChild);
    pCloneNext = pChild;
    pNext = pNext->m_pNext;
  }
}
IFDE_XMLInstruction* IFDE_XMLInstruction::Create(
    const CFX_WideString& wsTarget) {
  return (IFDE_XMLInstruction*)new CFDE_XMLInstruction(wsTarget);
}
CFDE_XMLInstruction::CFDE_XMLInstruction(const CFX_WideString& wsTarget)
    : m_wsTarget(wsTarget) {
  FXSYS_assert(m_wsTarget.GetLength() > 0);
}
CFDE_XMLNode* CFDE_XMLInstruction::Clone(FX_BOOL bRecursive) {
  CFDE_XMLInstruction* pClone = new CFDE_XMLInstruction(m_wsTarget);
  if (!pClone) {
    return pClone;
  }
  pClone->m_Attributes.Copy(m_Attributes);
  pClone->m_TargetData.Copy(m_TargetData);
  if (bRecursive) {
    CloneChildren(pClone);
  }
  return pClone;
}
int32_t CFDE_XMLInstruction::CountAttributes() const {
  return m_Attributes.GetSize() / 2;
}
FX_BOOL CFDE_XMLInstruction::GetAttribute(int32_t index,
                                          CFX_WideString& wsAttriName,
                                          CFX_WideString& wsAttriValue) const {
  int32_t iCount = m_Attributes.GetSize();
  FXSYS_assert(index > -1 && index < iCount / 2);
  for (int32_t i = 0; i < iCount; i += 2) {
    if (index == 0) {
      wsAttriName = m_Attributes[i];
      wsAttriValue = m_Attributes[i + 1];
      return TRUE;
    }
    index--;
  }
  return FALSE;
}
FX_BOOL CFDE_XMLInstruction::HasAttribute(const FX_WCHAR* pwsAttriName) const {
  int32_t iCount = m_Attributes.GetSize();
  for (int32_t i = 0; i < iCount; i += 2) {
    if (m_Attributes[i].Compare(pwsAttriName) == 0) {
      return TRUE;
    }
  }
  return FALSE;
}
void CFDE_XMLInstruction::GetString(const FX_WCHAR* pwsAttriName,
                                    CFX_WideString& wsAttriValue,
                                    const FX_WCHAR* pwsDefValue) const {
  int32_t iCount = m_Attributes.GetSize();
  for (int32_t i = 0; i < iCount; i += 2) {
    if (m_Attributes[i].Compare(pwsAttriName) == 0) {
      wsAttriValue = m_Attributes[i + 1];
      return;
    }
  }
  wsAttriValue = pwsDefValue;
}
void CFDE_XMLInstruction::SetString(const CFX_WideString& wsAttriName,
                                    const CFX_WideString& wsAttriValue) {
  FXSYS_assert(wsAttriName.GetLength() > 0);
  int32_t iCount = m_Attributes.GetSize();
  for (int32_t i = 0; i < iCount; i += 2) {
    if (m_Attributes[i].Compare(wsAttriName) == 0) {
      m_Attributes[i] = wsAttriName;
      m_Attributes[i + 1] = wsAttriValue;
      return;
    }
  }
  m_Attributes.Add(wsAttriName);
  m_Attributes.Add(wsAttriValue);
}
int32_t CFDE_XMLInstruction::GetInteger(const FX_WCHAR* pwsAttriName,
                                        int32_t iDefValue) const {
  int32_t iCount = m_Attributes.GetSize();
  for (int32_t i = 0; i < iCount; i += 2) {
    if (m_Attributes[i].Compare(pwsAttriName) == 0) {
      return FXSYS_wtoi((const FX_WCHAR*)m_Attributes[i + 1]);
    }
  }
  return iDefValue;
}
void CFDE_XMLInstruction::SetInteger(const FX_WCHAR* pwsAttriName,
                                     int32_t iAttriValue) {
  CFX_WideString wsValue;
  wsValue.Format(L"%d", iAttriValue);
  SetString(pwsAttriName, wsValue);
}
FX_FLOAT CFDE_XMLInstruction::GetFloat(const FX_WCHAR* pwsAttriName,
                                       FX_FLOAT fDefValue) const {
  int32_t iCount = m_Attributes.GetSize();
  for (int32_t i = 0; i < iCount; i += 2) {
    if (m_Attributes[i].Compare(pwsAttriName) == 0) {
      return FX_wcstof((const FX_WCHAR*)m_Attributes[i + 1]);
    }
  }
  return fDefValue;
}
void CFDE_XMLInstruction::SetFloat(const FX_WCHAR* pwsAttriName,
                                   FX_FLOAT fAttriValue) {
  CFX_WideString wsValue;
  wsValue.Format(L"%f", fAttriValue);
  SetString(pwsAttriName, wsValue);
}
void CFDE_XMLInstruction::RemoveAttribute(const FX_WCHAR* pwsAttriName) {
  int32_t iCount = m_Attributes.GetSize();
  for (int32_t i = 0; i < iCount; i += 2) {
    if (m_Attributes[i].Compare(pwsAttriName) == 0) {
      m_Attributes.RemoveAt(i + 1);
      m_Attributes.RemoveAt(i);
      return;
    }
  }
}
int32_t CFDE_XMLInstruction::CountData() const {
  return m_TargetData.GetSize();
}
FX_BOOL CFDE_XMLInstruction::GetData(int32_t index,
                                     CFX_WideString& wsData) const {
  if (index < 0 || index >= m_TargetData.GetSize()) {
    return FALSE;
  }
  wsData = m_TargetData[index];
  return TRUE;
}
void CFDE_XMLInstruction::AppendData(const CFX_WideString& wsData) {
  m_TargetData.Add(wsData);
}
void CFDE_XMLInstruction::RemoveData(int32_t index) {
  m_TargetData.RemoveAt(index);
}
IFDE_XMLElement* IFDE_XMLElement::Create(const CFX_WideString& wsTag) {
  return (IFDE_XMLElement*)new CFDE_XMLElement(wsTag);
}
CFDE_XMLElement::CFDE_XMLElement(const CFX_WideString& wsTag)
    : CFDE_XMLNode(), m_wsTag(wsTag), m_Attributes() {
  FXSYS_assert(m_wsTag.GetLength() > 0);
}
CFDE_XMLElement::~CFDE_XMLElement() {
  m_Attributes.RemoveAll();
}
CFDE_XMLNode* CFDE_XMLElement::Clone(FX_BOOL bRecursive) {
  CFDE_XMLElement* pClone = new CFDE_XMLElement(m_wsTag);
  if (!pClone) {
    return NULL;
  }
  pClone->m_Attributes.Copy(m_Attributes);
  if (bRecursive) {
    CloneChildren(pClone);
  } else {
    CFX_WideString wsText;
    CFDE_XMLNode* pChild = m_pChild;
    while (pChild != NULL) {
      switch (pChild->GetType()) {
        case FDE_XMLNODE_Text:
          wsText += ((CFDE_XMLText*)pChild)->m_wsText;
          break;
        default:
          break;
      }
      pChild = pChild->m_pNext;
    }
    pClone->SetTextData(wsText);
  }
  return pClone;
}
void CFDE_XMLElement::GetTagName(CFX_WideString& wsTag) const {
  wsTag = m_wsTag;
}
void CFDE_XMLElement::GetLocalTagName(CFX_WideString& wsTag) const {
  FX_STRSIZE iFind = m_wsTag.Find(L':', 0);
  if (iFind < 0) {
    wsTag = m_wsTag;
  } else {
    wsTag = m_wsTag.Right(m_wsTag.GetLength() - iFind - 1);
  }
}
void CFDE_XMLElement::GetNamespacePrefix(CFX_WideString& wsPrefix) const {
  FX_STRSIZE iFind = m_wsTag.Find(L':', 0);
  if (iFind < 0) {
    wsPrefix.Empty();
  } else {
    wsPrefix = m_wsTag.Left(iFind);
  }
}
void CFDE_XMLElement::GetNamespaceURI(CFX_WideString& wsNamespace) const {
  CFX_WideString wsAttri(L"xmlns"), wsPrefix;
  GetNamespacePrefix(wsPrefix);
  if (wsPrefix.GetLength() > 0) {
    wsAttri += L":";
    wsAttri += wsPrefix;
  }
  wsNamespace.Empty();
  CFDE_XMLNode* pNode = (CFDE_XMLNode*)this;
  while (pNode != NULL) {
    if (pNode->GetType() != FDE_XMLNODE_Element) {
      break;
    }
    CFDE_XMLElement* pElement = (CFDE_XMLElement*)pNode;
    if (!pElement->HasAttribute(wsAttri)) {
      pNode = pNode->GetNodeItem(IFDE_XMLNode::Parent);
      continue;
    }
    pElement->GetString(wsAttri, wsNamespace);
    break;
  }
}
int32_t CFDE_XMLElement::CountAttributes() const {
  return m_Attributes.GetSize() / 2;
}
FX_BOOL CFDE_XMLElement::GetAttribute(int32_t index,
                                      CFX_WideString& wsAttriName,
                                      CFX_WideString& wsAttriValue) const {
  int32_t iCount = m_Attributes.GetSize();
  FXSYS_assert(index > -1 && index < iCount / 2);
  for (int32_t i = 0; i < iCount; i += 2) {
    if (index == 0) {
      wsAttriName = m_Attributes[i];
      wsAttriValue = m_Attributes[i + 1];
      return TRUE;
    }
    index--;
  }
  return FALSE;
}
FX_BOOL CFDE_XMLElement::HasAttribute(const FX_WCHAR* pwsAttriName) const {
  int32_t iCount = m_Attributes.GetSize();
  for (int32_t i = 0; i < iCount; i += 2) {
    if (m_Attributes[i].Compare(pwsAttriName) == 0) {
      return TRUE;
    }
  }
  return FALSE;
}
void CFDE_XMLElement::GetString(const FX_WCHAR* pwsAttriName,
                                CFX_WideString& wsAttriValue,
                                const FX_WCHAR* pwsDefValue) const {
  int32_t iCount = m_Attributes.GetSize();
  for (int32_t i = 0; i < iCount; i += 2) {
    if (m_Attributes[i].Compare(pwsAttriName) == 0) {
      wsAttriValue = m_Attributes[i + 1];
      return;
    }
  }
  wsAttriValue = pwsDefValue;
}
void CFDE_XMLElement::SetString(const CFX_WideString& wsAttriName,
                                const CFX_WideString& wsAttriValue) {
  FXSYS_assert(wsAttriName.GetLength() > 0);
  int32_t iCount = m_Attributes.GetSize();
  for (int32_t i = 0; i < iCount; i += 2) {
    if (m_Attributes[i].Compare(wsAttriName) == 0) {
      m_Attributes[i] = wsAttriName;
      m_Attributes[i + 1] = wsAttriValue;
      return;
    }
  }
  m_Attributes.Add(wsAttriName);
  m_Attributes.Add(wsAttriValue);
}
int32_t CFDE_XMLElement::GetInteger(const FX_WCHAR* pwsAttriName,
                                    int32_t iDefValue) const {
  int32_t iCount = m_Attributes.GetSize();
  for (int32_t i = 0; i < iCount; i += 2) {
    if (m_Attributes[i].Compare(pwsAttriName) == 0) {
      return FXSYS_wtoi((const FX_WCHAR*)m_Attributes[i + 1]);
    }
  }
  return iDefValue;
}
void CFDE_XMLElement::SetInteger(const FX_WCHAR* pwsAttriName,
                                 int32_t iAttriValue) {
  CFX_WideString wsValue;
  wsValue.Format(L"%d", iAttriValue);
  SetString(pwsAttriName, wsValue);
}
FX_FLOAT CFDE_XMLElement::GetFloat(const FX_WCHAR* pwsAttriName,
                                   FX_FLOAT fDefValue) const {
  int32_t iCount = m_Attributes.GetSize();
  for (int32_t i = 0; i < iCount; i += 2) {
    if (m_Attributes[i].Compare(pwsAttriName) == 0) {
      return FX_wcstof((const FX_WCHAR*)m_Attributes[i + 1]);
    }
  }
  return fDefValue;
}
void CFDE_XMLElement::SetFloat(const FX_WCHAR* pwsAttriName,
                               FX_FLOAT fAttriValue) {
  CFX_WideString wsValue;
  wsValue.Format(L"%f", fAttriValue);
  SetString(pwsAttriName, wsValue);
}
void CFDE_XMLElement::RemoveAttribute(const FX_WCHAR* pwsAttriName) {
  int32_t iCount = m_Attributes.GetSize();
  for (int32_t i = 0; i < iCount; i += 2) {
    if (m_Attributes[i].Compare(pwsAttriName) == 0) {
      m_Attributes.RemoveAt(i + 1);
      m_Attributes.RemoveAt(i);
      return;
    }
  }
}
void CFDE_XMLElement::GetTextData(CFX_WideString& wsText) const {
  CFX_WideTextBuf buffer;
  CFDE_XMLNode* pChild = m_pChild;
  while (pChild != NULL) {
    switch (pChild->GetType()) {
      case FDE_XMLNODE_Text:
        buffer << ((CFDE_XMLText*)pChild)->m_wsText;
        break;
      case FDE_XMLNODE_CharData:
        buffer << ((CFDE_XMLCharData*)pChild)->m_wsCharData;
        break;
      default:
        break;
    }
    pChild = pChild->m_pNext;
  }
  wsText = buffer.GetWideString();
}
void CFDE_XMLElement::SetTextData(const CFX_WideString& wsText) {
  if (wsText.GetLength() < 1) {
    return;
  }
  InsertChildNode(new CFDE_XMLText(wsText));
}
IFDE_XMLText* IFDE_XMLText::Create(const CFX_WideString& wsText) {
  return (IFDE_XMLText*)new CFDE_XMLText(wsText);
}
CFDE_XMLText::CFDE_XMLText(const CFX_WideString& wsText)
    : CFDE_XMLNode(), m_wsText(wsText) {}
CFDE_XMLNode* CFDE_XMLText::Clone(FX_BOOL bRecursive) {
  CFDE_XMLText* pClone = new CFDE_XMLText(m_wsText);
  return pClone;
}
IFDE_XMLCharData* IFDE_XMLCharData::Create(const CFX_WideString& wsCData) {
  return (IFDE_XMLCharData*)new CFDE_XMLCharData(wsCData);
}
CFDE_XMLCharData::CFDE_XMLCharData(const CFX_WideString& wsCData)
    : CFDE_XMLDeclaration(), m_wsCharData(wsCData) {}
CFDE_XMLNode* CFDE_XMLCharData::Clone(FX_BOOL bRecursive) {
  CFDE_XMLCharData* pClone = new CFDE_XMLCharData(m_wsCharData);
  return pClone;
}
IFDE_XMLDoc* IFDE_XMLDoc::Create() {
  return (IFDE_XMLDoc*)new CFDE_XMLDoc;
}
CFDE_XMLDoc::CFDE_XMLDoc()
    : m_pRoot(NULL), m_pSyntaxParser(NULL), m_pXMLParser(NULL) {
  Reset(TRUE);
  CFDE_XMLInstruction* pXML = new CFDE_XMLInstruction(L"xml");
  m_pRoot->InsertChildNode(pXML);
}
CFDE_XMLDoc::~CFDE_XMLDoc() {
  Reset(FALSE);
}
void CFDE_XMLDoc::Reset(FX_BOOL bInitRoot) {
  m_iStatus = 0;
  m_pStream = NULL;
  if (bInitRoot) {
    if (m_pRoot == NULL) {
      m_pRoot = new CFDE_XMLNode;
    } else {
      m_pRoot->DeleteChildren();
    }
  } else {
    if (m_pRoot != NULL) {
      m_pRoot->Release();
      m_pRoot = NULL;
    }
  }
  ReleaseParser();
}
void CFDE_XMLDoc::ReleaseParser() {
  if (m_pXMLParser != NULL) {
    m_pXMLParser->Release();
    m_pXMLParser = NULL;
  }
  if (m_pSyntaxParser != NULL) {
    m_pSyntaxParser->Release();
    m_pSyntaxParser = NULL;
  }
}
FX_BOOL CFDE_XMLDoc::LoadXML(IFX_Stream* pXMLStream,
                             int32_t iXMLPlaneSize,
                             int32_t iTextDataSize,
                             FDE_LPXMLREADERHANDLER pHandler) {
  if (pXMLStream == NULL) {
    return FALSE;
  }
  Reset(TRUE);
  iXMLPlaneSize = iXMLPlaneSize / 1024;
  if (iXMLPlaneSize < 1) {
    iXMLPlaneSize = 1;
  }
  iXMLPlaneSize *= 1024;
  if (iXMLPlaneSize < 4096) {
    iXMLPlaneSize = 4096;
  }
  iTextDataSize = iTextDataSize / 128;
  if (iTextDataSize < 1) {
    iTextDataSize = 1;
  }
  iTextDataSize *= 128;
  if (iTextDataSize < 128) {
    iTextDataSize = 128;
  }
  m_pStream = pXMLStream;
  FX_WORD wCodePage = m_pStream->GetCodePage();
  if (wCodePage != FX_CODEPAGE_UTF16LE && wCodePage != FX_CODEPAGE_UTF16BE &&
      wCodePage != FX_CODEPAGE_UTF8) {
    m_pStream->SetCodePage(FX_CODEPAGE_UTF8);
  }
  m_pSyntaxParser = IFDE_XMLSyntaxParser::Create();
  if (m_pSyntaxParser == NULL) {
    return FALSE;
  }
  m_pSyntaxParser->Init(m_pStream, iXMLPlaneSize, iTextDataSize);
  if (pHandler == NULL) {
    m_pXMLParser = new CFDE_XMLDOMParser(m_pRoot, m_pSyntaxParser);
  } else {
    m_pXMLParser = new CFDE_XMLSAXParser(pHandler, m_pSyntaxParser);
  }
  return TRUE;
}
FX_BOOL CFDE_XMLDoc::LoadXML(IFDE_XMLParser* pXMLParser) {
  if (pXMLParser == NULL) {
    return FALSE;
  }
  Reset(TRUE);
  m_pXMLParser = pXMLParser;
  return m_pXMLParser != NULL;
}
int32_t CFDE_XMLDoc::DoLoad(IFX_Pause* pPause) {
  if (m_iStatus >= 100) {
    return m_iStatus;
  }
  FXSYS_assert(m_pXMLParser != NULL);
  return m_iStatus = m_pXMLParser->DoParser(pPause);
}
void CFDE_XMLDoc::CloseXML() {
  ReleaseParser();
}
void CFDE_XMLDoc::SaveXMLNode(IFX_Stream* pXMLStream, IFDE_XMLNode* pINode) {
  CFDE_XMLNode* pNode = (CFDE_XMLNode*)pINode;
  FXSYS_assert(pXMLStream != NULL && pNode != NULL);
  switch (pNode->GetType()) {
    case FDE_XMLNODE_Instruction: {
      CFX_WideString ws;
      CFDE_XMLInstruction* pInstruction = (CFDE_XMLInstruction*)pNode;
      if (pInstruction->m_wsTarget.CompareNoCase(L"xml") == 0) {
        ws = L"<?xml version=\"1.0\" encoding=\"";
        FX_WORD wCodePage = pXMLStream->GetCodePage();
        if (wCodePage == FX_CODEPAGE_UTF16LE) {
          ws += L"UTF-16";
        } else if (wCodePage == FX_CODEPAGE_UTF16BE) {
          ws += L"UTF-16be";
        } else {
          ws += L"UTF-8";
        }
        ws += L"\"?>";
        pXMLStream->WriteString(ws, ws.GetLength());
      } else {
        ws.Format(L"<?%s", (const FX_WCHAR*)pInstruction->m_wsTarget);
        pXMLStream->WriteString(ws, ws.GetLength());
        CFX_WideStringArray& attributes = pInstruction->m_Attributes;
        int32_t i, iCount = attributes.GetSize();
        CFX_WideString wsValue;
        for (i = 0; i < iCount; i += 2) {
          ws = L" ";
          ws += attributes[i];
          ws += L"=\"";
          wsValue = attributes[i + 1];
          wsValue.Replace(L"&", L"&amp;");
          wsValue.Replace(L"<", L"&lt;");
          wsValue.Replace(L">", L"&gt;");
          wsValue.Replace(L"\'", L"&apos;");
          wsValue.Replace(L"\"", L"&quot;");
          ws += wsValue;
          ws += L"\"";
          pXMLStream->WriteString(ws, ws.GetLength());
        }
        CFX_WideStringArray& targetdata = pInstruction->m_TargetData;
        iCount = targetdata.GetSize();
        for (i = 0; i < iCount; i++) {
          ws = L" \"";
          ws += targetdata[i];
          ws += L"\"";
          pXMLStream->WriteString(ws, ws.GetLength());
        }
        ws = L"?>";
        pXMLStream->WriteString(ws, ws.GetLength());
      }
    } break;
    case FDE_XMLNODE_Element: {
      CFX_WideString ws;
      ws = L"<";
      ws += ((CFDE_XMLElement*)pNode)->m_wsTag;
      pXMLStream->WriteString(ws, ws.GetLength());
      CFX_WideStringArray& attributes = ((CFDE_XMLElement*)pNode)->m_Attributes;
      int32_t iCount = attributes.GetSize();
      CFX_WideString wsValue;
      for (int32_t i = 0; i < iCount; i += 2) {
        ws = L" ";
        ws += attributes[i];
        ws += L"=\"";
        wsValue = attributes[i + 1];
        wsValue.Replace(L"&", L"&amp;");
        wsValue.Replace(L"<", L"&lt;");
        wsValue.Replace(L">", L"&gt;");
        wsValue.Replace(L"\'", L"&apos;");
        wsValue.Replace(L"\"", L"&quot;");
        ws += wsValue;
        ws += L"\"";
        pXMLStream->WriteString(ws, ws.GetLength());
      }
      if (pNode->m_pChild == NULL) {
        ws = L"\n/>";
        pXMLStream->WriteString(ws, ws.GetLength());
      } else {
        ws = L"\n>";
        pXMLStream->WriteString(ws, ws.GetLength());
        CFDE_XMLNode* pChild = pNode->m_pChild;
        while (pChild != NULL) {
          SaveXMLNode(pXMLStream, (IFDE_XMLNode*)pChild);
          pChild = pChild->m_pNext;
        }
        ws = L"</";
        ws += ((CFDE_XMLElement*)pNode)->m_wsTag;
        ws += L"\n>";
        pXMLStream->WriteString(ws, ws.GetLength());
      }
    } break;
    case FDE_XMLNODE_Text: {
      CFX_WideString ws = ((CFDE_XMLText*)pNode)->m_wsText;
      ws.Replace(L"&", L"&amp;");
      ws.Replace(L"<", L"&lt;");
      ws.Replace(L">", L"&gt;");
      ws.Replace(L"\'", L"&apos;");
      ws.Replace(L"\"", L"&quot;");
      pXMLStream->WriteString(ws, ws.GetLength());
    } break;
    case FDE_XMLNODE_CharData: {
      CFX_WideString ws = L"<![CDATA[";
      ws += ((CFDE_XMLCharData*)pNode)->m_wsCharData;
      ws += L"]]>";
      pXMLStream->WriteString(ws, ws.GetLength());
    } break;
    case FDE_XMLNODE_Unknown:
      break;
    default:
      break;
  }
}
void CFDE_XMLDoc::SaveXML(IFX_Stream* pXMLStream, FX_BOOL bSaveBOM) {
  if (pXMLStream == NULL || pXMLStream == m_pStream) {
    m_pStream->Seek(FX_STREAMSEEK_Begin, 0);
    pXMLStream = m_pStream;
  }
  FXSYS_assert((pXMLStream->GetAccessModes() & FX_STREAMACCESS_Text) != 0);
  FXSYS_assert((pXMLStream->GetAccessModes() & FX_STREAMACCESS_Write) != 0);
  FX_WORD wCodePage = pXMLStream->GetCodePage();
  if (wCodePage != FX_CODEPAGE_UTF16LE && wCodePage != FX_CODEPAGE_UTF16BE &&
      wCodePage != FX_CODEPAGE_UTF8) {
    wCodePage = FX_CODEPAGE_UTF8;
    pXMLStream->SetCodePage(wCodePage);
  }
  if (bSaveBOM) {
    pXMLStream->WriteString(L"\xFEFF", 1);
  }
  CFDE_XMLNode* pNode = m_pRoot->m_pChild;
  while (pNode != NULL) {
    SaveXMLNode(pXMLStream, (IFDE_XMLNode*)pNode);
    pNode = pNode->m_pNext;
  }
  if (pXMLStream == m_pStream) {
    int32_t iPos = pXMLStream->GetPosition();
    pXMLStream->SetLength(iPos);
  }
}
CFDE_XMLDOMParser::CFDE_XMLDOMParser(CFDE_XMLNode* pRoot,
                                     IFDE_XMLSyntaxParser* pParser)
    : m_pParser(pParser),
      m_pParent(pRoot),
      m_pChild(NULL),
      m_NodeStack(16),
      m_ws1(),
      m_ws2() {
  m_NodeStack.Push(m_pParent);
}
CFDE_XMLDOMParser::~CFDE_XMLDOMParser() {
  m_NodeStack.RemoveAll();
  m_ws1.Empty();
  m_ws2.Empty();
}
int32_t CFDE_XMLDOMParser::DoParser(IFX_Pause* pPause) {
  FX_DWORD dwRet;
  int32_t iCount = 0;
  while (TRUE) {
    dwRet = m_pParser->DoSyntaxParse();
    switch (dwRet) {
      case FDE_XMLSYNTAXSTATUS_InstructionOpen:
        break;
      case FDE_XMLSYNTAXSTATUS_InstructionClose:
        if (m_pChild->GetType() != FDE_XMLNODE_Instruction) {
          dwRet = FDE_XMLSYNTAXSTATUS_Error;
          break;
        }
        m_pChild = m_pParent;
        break;
      case FDE_XMLSYNTAXSTATUS_ElementOpen:
      case FDE_XMLSYNTAXSTATUS_ElementBreak:
        break;
      case FDE_XMLSYNTAXSTATUS_ElementClose:
        if (m_pChild->GetType() != FDE_XMLNODE_Element) {
          dwRet = FDE_XMLSYNTAXSTATUS_Error;
          break;
        }
        m_pParser->GetTagName(m_ws1);
        ((CFDE_XMLElement*)m_pChild)->GetTagName(m_ws2);
        if (m_ws1.GetLength() > 0 && m_ws1.Compare(m_ws2) != 0) {
          dwRet = FDE_XMLSYNTAXSTATUS_Error;
          break;
        }
        m_NodeStack.Pop();
        if (m_NodeStack.GetSize() < 1) {
          dwRet = FDE_XMLSYNTAXSTATUS_Error;
          break;
        }
        m_pParent = (CFDE_XMLNode*)*m_NodeStack.GetTopElement();
        m_pChild = m_pParent;
        iCount++;
        break;
      case FDE_XMLSYNTAXSTATUS_TargetName:
        m_pParser->GetTargetName(m_ws1);
        m_pChild = new CFDE_XMLInstruction(m_ws1);
        m_pParent->InsertChildNode(m_pChild);
        m_ws1.Empty();
        break;
      case FDE_XMLSYNTAXSTATUS_TagName:
        m_pParser->GetTagName(m_ws1);
        m_pChild = new CFDE_XMLElement(m_ws1);
        m_pParent->InsertChildNode(m_pChild);
        m_NodeStack.Push(m_pChild);
        m_pParent = m_pChild;
        break;
      case FDE_XMLSYNTAXSTATUS_AttriName:
        m_pParser->GetAttributeName(m_ws1);
        break;
      case FDE_XMLSYNTAXSTATUS_AttriValue:
        if (m_pChild == NULL) {
          dwRet = FDE_XMLSYNTAXSTATUS_Error;
          break;
        }
        m_pParser->GetAttributeName(m_ws2);
        if (m_pChild->GetType() == FDE_XMLNODE_Element) {
          ((CFDE_XMLElement*)m_pChild)->SetString(m_ws1, m_ws2);
        } else if (m_pChild->GetType() == FDE_XMLNODE_Instruction) {
          ((CFDE_XMLInstruction*)m_pChild)->SetString(m_ws1, m_ws2);
        }
        m_ws1.Empty();
        break;
      case FDE_XMLSYNTAXSTATUS_Text:
        m_pParser->GetTextData(m_ws1);
        m_pChild = new CFDE_XMLText(m_ws1);
        m_pParent->InsertChildNode(m_pChild);
        m_pChild = m_pParent;
        break;
      case FDE_XMLSYNTAXSTATUS_CData:
        m_pParser->GetTextData(m_ws1);
        m_pChild = new CFDE_XMLCharData(m_ws1);
        m_pParent->InsertChildNode(m_pChild);
        m_pChild = m_pParent;
        break;
      case FDE_XMLSYNTAXSTATUS_TargetData:
        if (m_pChild == NULL ||
            m_pChild->GetType() != FDE_XMLNODE_Instruction) {
          dwRet = FDE_XMLSYNTAXSTATUS_Error;
          break;
        }
        if (!m_ws1.IsEmpty()) {
          ((CFDE_XMLInstruction*)m_pChild)->AppendData(m_ws1);
        }
        m_pParser->GetTargetData(m_ws1);
        ((CFDE_XMLInstruction*)m_pChild)->AppendData(m_ws1);
        m_ws1.Empty();
        break;
      default:
        break;
    }
    if (dwRet == FDE_XMLSYNTAXSTATUS_Error ||
        dwRet == FDE_XMLSYNTAXSTATUS_EOS) {
      break;
    }
    if (pPause != NULL && iCount > 500 && pPause->NeedToPauseNow()) {
      break;
    }
  }
  return m_pParser->GetStatus();
}
CFDE_XMLSAXParser::CFDE_XMLSAXParser(FDE_LPXMLREADERHANDLER pHandler,
                                     IFDE_XMLSyntaxParser* pParser)
    : m_pHandler(pHandler),
      m_pParser(pParser),
      m_TagStack(16),
      m_pTagTop(NULL),
      m_ws1(),
      m_ws2() {}
CFDE_XMLSAXParser::~CFDE_XMLSAXParser() {
  m_TagStack.RemoveAll();
  m_ws1.Empty();
  m_ws2.Empty();
}
int32_t CFDE_XMLSAXParser::DoParser(IFX_Pause* pPause) {
  FX_DWORD dwRet = 0;
  int32_t iCount = 0;
  while (TRUE) {
    dwRet = m_pParser->DoSyntaxParse();
    switch (dwRet) {
      case FDE_XMLSYNTAXSTATUS_ElementBreak:
        if (m_pTagTop == NULL) {
          dwRet = FDE_XMLSYNTAXSTATUS_Error;
          break;
        }
        if (m_pTagTop->eType == FDE_XMLNODE_Element) {
          m_pHandler->OnTagBreak(m_pHandler, m_pTagTop->wsTagName);
        }
        break;
      case FDE_XMLSYNTAXSTATUS_ElementClose:
        if (m_pTagTop == NULL || m_pTagTop->eType != FDE_XMLNODE_Element) {
          dwRet = FDE_XMLSYNTAXSTATUS_Error;
          break;
        }
        m_pParser->GetTagName(m_ws1);
        if (m_ws1.GetLength() > 0 && m_ws1.Compare(m_pTagTop->wsTagName) != 0) {
          dwRet = FDE_XMLSYNTAXSTATUS_Error;
          break;
        } else if (m_ws1.GetLength() == 0) {
          m_pHandler->OnTagBreak(m_pHandler, m_pTagTop->wsTagName);
        }
        m_pHandler->OnTagClose(m_pHandler, m_pTagTop->wsTagName);
        Pop();
        iCount++;
        break;
      case FDE_XMLSYNTAXSTATUS_TargetName: {
        m_pParser->GetTargetName(m_ws1);
        CFDE_XMLTAG xmlTag;
        xmlTag.wsTagName = m_ws1;
        xmlTag.eType = FDE_XMLNODE_Instruction;
        Push(xmlTag);
        m_pHandler->OnTagEnter(m_pHandler, FDE_XMLNODE_Instruction,
                               m_pTagTop->wsTagName);
        m_ws1.Empty();
      } break;
      case FDE_XMLSYNTAXSTATUS_TagName: {
        m_pParser->GetTargetName(m_ws1);
        CFDE_XMLTAG xmlTag;
        xmlTag.wsTagName = m_ws1;
        xmlTag.eType = FDE_XMLNODE_Element;
        Push(xmlTag);
        m_pHandler->OnTagEnter(m_pHandler, FDE_XMLNODE_Element,
                               m_pTagTop->wsTagName);
      } break;
      case FDE_XMLSYNTAXSTATUS_AttriName:
        m_pParser->GetTargetName(m_ws1);
        break;
      case FDE_XMLSYNTAXSTATUS_AttriValue:
        m_pParser->GetAttributeName(m_ws2);
        if (m_pTagTop == NULL) {
          dwRet = FDE_XMLSYNTAXSTATUS_Error;
          break;
        }
        if (m_pTagTop->eType == FDE_XMLNODE_Element) {
          m_pHandler->OnAttribute(m_pHandler, m_ws1, m_ws2);
        }
        m_ws1.Empty();
        break;
      case FDE_XMLSYNTAXSTATUS_CData:
        m_pParser->GetTextData(m_ws1);
        m_pHandler->OnData(m_pHandler, FDE_XMLNODE_CharData, m_ws1);
        break;
      case FDE_XMLSYNTAXSTATUS_Text:
        m_pParser->GetTextData(m_ws1);
        m_pHandler->OnData(m_pHandler, FDE_XMLNODE_Text, m_ws1);
        break;
      case FDE_XMLSYNTAXSTATUS_TargetData:
        m_pParser->GetTargetData(m_ws1);
        m_pHandler->OnData(m_pHandler, FDE_XMLNODE_Instruction, m_ws1);
        m_ws1.Empty();
        break;
      default:
        break;
    }
    if (dwRet == FDE_XMLSYNTAXSTATUS_Error ||
        dwRet == FDE_XMLSYNTAXSTATUS_EOS) {
      break;
    }
    if (pPause != NULL && iCount > 500 && pPause->NeedToPauseNow()) {
      break;
    }
  }
  return m_pParser->GetStatus();
}
inline void CFDE_XMLSAXParser::Push(const CFDE_XMLTAG& xmlTag) {
  m_TagStack.Push(xmlTag);
  m_pTagTop = m_TagStack.GetTopElement();
}
inline void CFDE_XMLSAXParser::Pop() {
  m_TagStack.Pop();
  m_pTagTop = m_TagStack.GetTopElement();
}
#ifdef _FDE_BLOCK_BUFFER
CFDE_BlockBuffer::CFDE_BlockBuffer(int32_t iAllocStep)
    : m_iDataLength(0),
      m_iBufferSize(0),
      m_iAllocStep(iAllocStep),
      m_iStartPosition(0) {
}
CFDE_BlockBuffer::~CFDE_BlockBuffer() {
  ClearBuffer();
}
FX_WCHAR* CFDE_BlockBuffer::GetAvailableBlock(int32_t& iIndexInBlock) {
  iIndexInBlock = 0;
  if (!m_BlockArray.GetSize()) {
    return nullptr;
  }
  int32_t iRealIndex = m_iStartPosition + m_iDataLength;
  if (iRealIndex == m_iBufferSize) {
    FX_WCHAR* pBlock = FX_Alloc(FX_WCHAR, m_iAllocStep);
    m_BlockArray.Add(pBlock);
    m_iBufferSize += m_iAllocStep;
    return pBlock;
  }
  iIndexInBlock = iRealIndex % m_iAllocStep;
  return (FX_WCHAR*)m_BlockArray[iRealIndex / m_iAllocStep];
}
FX_BOOL CFDE_BlockBuffer::InitBuffer(int32_t iBufferSize) {
  ClearBuffer();
  int32_t iNumOfBlock = (iBufferSize - 1) / m_iAllocStep + 1;
  for (int32_t i = 0; i < iNumOfBlock; i++) {
    m_BlockArray.Add(FX_Alloc(FX_WCHAR, m_iAllocStep));
  }
  m_iBufferSize = iNumOfBlock * m_iAllocStep;
  return TRUE;
}
void CFDE_BlockBuffer::SetTextChar(int32_t iIndex, FX_WCHAR ch) {
  if (iIndex < 0) {
    return;
  }
  int32_t iRealIndex = m_iStartPosition + iIndex;
  int32_t iBlockIndex = iRealIndex / m_iAllocStep;
  int32_t iInnerIndex = iRealIndex % m_iAllocStep;
  int32_t iBlockSize = m_BlockArray.GetSize();
  if (iBlockIndex >= iBlockSize) {
    int32_t iNewBlocks = iBlockIndex - iBlockSize + 1;
    do {
      FX_WCHAR* pBlock = FX_Alloc(FX_WCHAR, m_iAllocStep);
      m_BlockArray.Add(pBlock);
      m_iBufferSize += m_iAllocStep;
    } while (--iNewBlocks);
  }
  FX_WCHAR* pTextData = (FX_WCHAR*)m_BlockArray[iBlockIndex];
  *(pTextData + iInnerIndex) = ch;
  if (m_iDataLength <= iIndex) {
    m_iDataLength = iIndex + 1;
  }
}
int32_t CFDE_BlockBuffer::DeleteTextChars(int32_t iCount, FX_BOOL bDirection) {
  if (iCount <= 0) {
    return m_iDataLength;
  }
  if (iCount >= m_iDataLength) {
    Reset(FALSE);
    return 0;
  }
  if (bDirection) {
    m_iStartPosition += iCount;
    m_iDataLength -= iCount;
  } else {
    m_iDataLength -= iCount;
  }
  return m_iDataLength;
}
void CFDE_BlockBuffer::GetTextData(CFX_WideString& wsTextData,
                                   int32_t iStart,
                                   int32_t iLength) const {
  wsTextData.Empty();
  int32_t iMaybeDataLength = m_iBufferSize - 1 - m_iStartPosition;
  if (iStart < 0 || iStart > iMaybeDataLength) {
    return;
  }
  if (iLength == -1 || iLength > iMaybeDataLength) {
    iLength = iMaybeDataLength;
  }
  if (iLength <= 0) {
    return;
  }
  FX_WCHAR* pBuf = wsTextData.GetBuffer(iLength);
  if (!pBuf) {
    return;
  }
  int32_t iStartBlockIndex = 0;
  int32_t iStartInnerIndex = 0;
  TextDataIndex2BufIndex(iStart, iStartBlockIndex, iStartInnerIndex);
  int32_t iEndBlockIndex = 0;
  int32_t iEndInnerIndex = 0;
  TextDataIndex2BufIndex(iStart + iLength, iEndBlockIndex, iEndInnerIndex);
  int32_t iPointer = 0;
  for (int32_t i = iStartBlockIndex; i <= iEndBlockIndex; i++) {
    int32_t iBufferPointer = 0;
    int32_t iCopyLength = m_iAllocStep;
    if (i == iStartBlockIndex) {
      iCopyLength -= iStartInnerIndex;
      iBufferPointer = iStartInnerIndex;
    }
    if (i == iEndBlockIndex) {
      iCopyLength -= ((m_iAllocStep - 1) - iEndInnerIndex);
    }
    FX_WCHAR* pBlockBuf = (FX_WCHAR*)m_BlockArray[i];
    FXSYS_memcpy(pBuf + iPointer, pBlockBuf + iBufferPointer,
                 iCopyLength * sizeof(FX_WCHAR));
    iPointer += iCopyLength;
  }
  wsTextData.ReleaseBuffer(iLength);
}
void CFDE_BlockBuffer::TextDataIndex2BufIndex(const int32_t iIndex,
                                              int32_t& iBlockIndex,
                                              int32_t& iInnerIndex) const {
  FXSYS_assert(iIndex >= 0);
  int32_t iRealIndex = m_iStartPosition + iIndex;
  iBlockIndex = iRealIndex / m_iAllocStep;
  iInnerIndex = iRealIndex % m_iAllocStep;
}
void CFDE_BlockBuffer::ClearBuffer() {
  m_iBufferSize = 0;
  int32_t iSize = m_BlockArray.GetSize();
  for (int32_t i = 0; i < iSize; i++) {
    FX_Free(m_BlockArray[i]);
    m_BlockArray[i] = NULL;
  }
  m_BlockArray.RemoveAll();
}
#endif
IFDE_XMLSyntaxParser* IFDE_XMLSyntaxParser::Create() {
  return new CFDE_XMLSyntaxParser;
}
#ifdef _FDE_BLOCK_BUFFER
CFDE_XMLSyntaxParser::CFDE_XMLSyntaxParser()
    : m_pStream(nullptr),
      m_iXMLPlaneSize(-1),
      m_iCurrentPos(0),
      m_iCurrentNodeNum(-1),
      m_iLastNodeNum(-1),
      m_iParsedChars(0),
      m_iParsedBytes(0),
      m_pBuffer(nullptr),
      m_iBufferChars(0),
      m_bEOS(FALSE),
      m_pStart(nullptr),
      m_pEnd(nullptr),
      m_XMLNodeStack(16),
      m_iAllocStep(m_BlockBuffer.GetAllocStep()),
      m_iDataLength(m_BlockBuffer.GetDataLengthRef()),
      m_pCurrentBlock(nullptr),
      m_iIndexInBlock(0),
      m_iTextDataLength(0),
      m_dwStatus(FDE_XMLSYNTAXSTATUS_None),
      m_dwMode(FDE_XMLSYNTAXMODE_Text),
      m_wQuotationMark(0),
      m_iEntityStart(-1),
      m_SkipStack(16) {
  m_CurNode.iNodeNum = -1;
  m_CurNode.eNodeType = FDE_XMLNODE_Unknown;
}
void CFDE_XMLSyntaxParser::Init(IFX_Stream* pStream,
                                int32_t iXMLPlaneSize,
                                int32_t iTextDataSize) {
  FXSYS_assert(m_pStream == NULL && m_pBuffer == NULL);
  FXSYS_assert(pStream != NULL && iXMLPlaneSize > 0);
  int32_t iStreamLength = pStream->GetLength();
  FXSYS_assert(iStreamLength > 0);
  m_pStream = pStream;
  m_iXMLPlaneSize = std::min(iXMLPlaneSize, iStreamLength);
  uint8_t bom[4];
  m_iCurrentPos = m_pStream->GetBOM(bom);
  FXSYS_assert(m_pBuffer == NULL);
  m_pBuffer = FX_Alloc(FX_WCHAR, m_iXMLPlaneSize);
  m_pStart = m_pEnd = m_pBuffer;
  FXSYS_assert(!m_BlockBuffer.IsInitialized());
  m_BlockBuffer.InitBuffer();
  m_pCurrentBlock = m_BlockBuffer.GetAvailableBlock(m_iIndexInBlock);
  m_iParsedBytes = m_iParsedChars = 0;
  m_iBufferChars = 0;
}
FX_DWORD CFDE_XMLSyntaxParser::DoSyntaxParse() {
  if (m_dwStatus == FDE_XMLSYNTAXSTATUS_Error ||
      m_dwStatus == FDE_XMLSYNTAXSTATUS_EOS) {
    return m_dwStatus;
  }
  FXSYS_assert(m_pStream && m_pBuffer && m_BlockBuffer.IsInitialized());
  int32_t iStreamLength = m_pStream->GetLength();
  int32_t iPos;
  FX_WCHAR ch;
  FX_DWORD dwStatus = FDE_XMLSYNTAXSTATUS_None;
  while (TRUE) {
    if (m_pStart >= m_pEnd) {
      if (m_bEOS || m_iCurrentPos >= iStreamLength) {
        m_dwStatus = FDE_XMLSYNTAXSTATUS_EOS;
        return m_dwStatus;
      }
      m_iParsedChars += (m_pEnd - m_pBuffer);
      m_iParsedBytes = m_iCurrentPos;
      m_pStream->Lock();
      if (m_pStream->GetPosition() != m_iCurrentPos) {
        m_pStream->Seek(FX_STREAMSEEK_Begin, m_iCurrentPos);
      }
      m_iBufferChars =
          m_pStream->ReadString(m_pBuffer, m_iXMLPlaneSize, m_bEOS);
      iPos = m_pStream->GetPosition();
      m_pStream->Unlock();
      if (m_iBufferChars < 1) {
        m_iCurrentPos = iStreamLength;
        m_dwStatus = FDE_XMLSYNTAXSTATUS_EOS;
        return m_dwStatus;
      }
      m_iCurrentPos = iPos;
      m_pStart = m_pBuffer;
      m_pEnd = m_pBuffer + m_iBufferChars;
    }
    while (m_pStart < m_pEnd) {
      ch = *m_pStart;
      switch (m_dwMode) {
        case FDE_XMLSYNTAXMODE_Text:
          if (ch == L'<') {
            if (m_iDataLength > 0) {
              m_iTextDataLength = m_iDataLength;
              m_BlockBuffer.Reset();
              m_pCurrentBlock =
                  m_BlockBuffer.GetAvailableBlock(m_iIndexInBlock);
              m_iEntityStart = -1;
              dwStatus = FDE_XMLSYNTAXSTATUS_Text;
            } else {
              m_pStart++;
              m_dwMode = FDE_XMLSYNTAXMODE_Node;
            }
          } else {
            ParseTextChar(ch);
          }
          break;
        case FDE_XMLSYNTAXMODE_Node:
          if (ch == L'!') {
            m_pStart++;
            m_dwMode = FDE_XMLSYNTAXMODE_SkipCommentOrDecl;
          } else if (ch == L'/') {
            m_pStart++;
            m_dwMode = FDE_XMLSYNTAXMODE_CloseElement;
          } else if (ch == L'?') {
            m_iLastNodeNum++;
            m_iCurrentNodeNum = m_iLastNodeNum;
            m_CurNode.iNodeNum = m_iLastNodeNum;
            m_CurNode.eNodeType = FDE_XMLNODE_Instruction;
            m_XMLNodeStack.Push(m_CurNode);
            m_pStart++;
            m_dwMode = FDE_XMLSYNTAXMODE_Target;
            dwStatus = FDE_XMLSYNTAXSTATUS_InstructionOpen;
          } else {
            m_iLastNodeNum++;
            m_iCurrentNodeNum = m_iLastNodeNum;
            m_CurNode.iNodeNum = m_iLastNodeNum;
            m_CurNode.eNodeType = FDE_XMLNODE_Element;
            m_XMLNodeStack.Push(m_CurNode);
            m_dwMode = FDE_XMLSYNTAXMODE_Tag;
            dwStatus = FDE_XMLSYNTAXSTATUS_ElementOpen;
          }
          break;
        case FDE_XMLSYNTAXMODE_Target:
        case FDE_XMLSYNTAXMODE_Tag:
          if (!FDE_IsXMLNameChar(ch, m_iDataLength < 1)) {
            if (m_iDataLength < 1) {
              m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
              return m_dwStatus;
            } else {
              m_iTextDataLength = m_iDataLength;
              m_BlockBuffer.Reset();
              m_pCurrentBlock =
                  m_BlockBuffer.GetAvailableBlock(m_iIndexInBlock);
              if (m_dwMode != FDE_XMLSYNTAXMODE_Target) {
                dwStatus = FDE_XMLSYNTAXSTATUS_TagName;
              } else {
                dwStatus = FDE_XMLSYNTAXSTATUS_TargetName;
              }
              m_dwMode = FDE_XMLSYNTAXMODE_AttriName;
            }
          } else {
            if (m_iIndexInBlock == m_iAllocStep) {
              m_pCurrentBlock =
                  m_BlockBuffer.GetAvailableBlock(m_iIndexInBlock);
              if (!m_pCurrentBlock) {
                return FDE_XMLSYNTAXSTATUS_Error;
              }
            }
            m_pCurrentBlock[m_iIndexInBlock++] = ch;
            m_iDataLength++;
            m_pStart++;
          }
          break;
        case FDE_XMLSYNTAXMODE_AttriName:
          if (m_iDataLength < 1 && FDE_IsXMLWhiteSpace(ch)) {
            m_pStart++;
            break;
          }
          if (!FDE_IsXMLNameChar(ch, m_iDataLength < 1)) {
            if (m_iDataLength < 1) {
              if (m_CurNode.eNodeType == FDE_XMLNODE_Element) {
                if (ch == L'>' || ch == L'/') {
                  m_dwMode = FDE_XMLSYNTAXMODE_BreakElement;
                  break;
                }
              } else if (m_CurNode.eNodeType == FDE_XMLNODE_Instruction) {
                if (ch == L'?') {
                  m_dwMode = FDE_XMLSYNTAXMODE_CloseInstruction;
                  m_pStart++;
                } else {
                  m_dwMode = FDE_XMLSYNTAXMODE_TargetData;
                }
                break;
              }
              m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
              return m_dwStatus;
            } else {
              if (m_CurNode.eNodeType == FDE_XMLNODE_Instruction) {
                if (ch != '=' && !FDE_IsXMLWhiteSpace(ch)) {
                  m_dwMode = FDE_XMLSYNTAXMODE_TargetData;
                  break;
                }
              }
              m_iTextDataLength = m_iDataLength;
              m_BlockBuffer.Reset();
              m_pCurrentBlock =
                  m_BlockBuffer.GetAvailableBlock(m_iIndexInBlock);
              m_dwMode = FDE_XMLSYNTAXMODE_AttriEqualSign;
              dwStatus = FDE_XMLSYNTAXSTATUS_AttriName;
            }
          } else {
            if (m_iIndexInBlock == m_iAllocStep) {
              m_pCurrentBlock =
                  m_BlockBuffer.GetAvailableBlock(m_iIndexInBlock);
              if (!m_pCurrentBlock) {
                return FDE_XMLSYNTAXSTATUS_Error;
              }
            }
            m_pCurrentBlock[m_iIndexInBlock++] = ch;
            m_iDataLength++;
            m_pStart++;
          }
          break;
        case FDE_XMLSYNTAXMODE_AttriEqualSign:
          if (FDE_IsXMLWhiteSpace(ch)) {
            m_pStart++;
            break;
          }
          if (ch != L'=') {
            if (m_CurNode.eNodeType == FDE_XMLNODE_Instruction) {
              m_dwMode = FDE_XMLSYNTAXMODE_TargetData;
              break;
            }
            m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
            return m_dwStatus;
          } else {
            m_dwMode = FDE_XMLSYNTAXMODE_AttriQuotation;
            m_pStart++;
          }
          break;
        case FDE_XMLSYNTAXMODE_AttriQuotation:
          if (FDE_IsXMLWhiteSpace(ch)) {
            m_pStart++;
            break;
          }
          if (ch != L'\"' && ch != L'\'') {
            m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
            return m_dwStatus;
          } else {
            m_wQuotationMark = ch;
            m_dwMode = FDE_XMLSYNTAXMODE_AttriValue;
            m_pStart++;
          }
          break;
        case FDE_XMLSYNTAXMODE_AttriValue:
          if (ch == m_wQuotationMark) {
            if (m_iEntityStart > -1) {
              m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
              return m_dwStatus;
            }
            m_iTextDataLength = m_iDataLength;
            m_wQuotationMark = 0;
            m_BlockBuffer.Reset();
            m_pCurrentBlock = m_BlockBuffer.GetAvailableBlock(m_iIndexInBlock);
            m_pStart++;
            m_dwMode = FDE_XMLSYNTAXMODE_AttriName;
            dwStatus = FDE_XMLSYNTAXSTATUS_AttriValue;
          } else {
            ParseTextChar(ch);
          }
          break;
        case FDE_XMLSYNTAXMODE_CloseInstruction:
          if (ch != L'>') {
            if (m_iIndexInBlock == m_iAllocStep) {
              m_pCurrentBlock =
                  m_BlockBuffer.GetAvailableBlock(m_iIndexInBlock);
              if (!m_pCurrentBlock) {
                return FDE_XMLSYNTAXSTATUS_Error;
              }
            }
            m_pCurrentBlock[m_iIndexInBlock++] = ch;
            m_iDataLength++;
            m_dwMode = FDE_XMLSYNTAXMODE_TargetData;
          } else if (m_iDataLength > 0) {
            m_iTextDataLength = m_iDataLength;
            m_BlockBuffer.Reset();
            m_pCurrentBlock = m_BlockBuffer.GetAvailableBlock(m_iIndexInBlock);
            dwStatus = FDE_XMLSYNTAXSTATUS_TargetData;
          } else {
            m_pStart++;
            FDE_LPXMLNODE pXMLNode = m_XMLNodeStack.GetTopElement();
            if (pXMLNode == NULL) {
              m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
              return m_dwStatus;
            }
            m_XMLNodeStack.Pop();
            pXMLNode = m_XMLNodeStack.GetTopElement();
            if (pXMLNode == NULL) {
              m_CurNode.iNodeNum = -1;
              m_CurNode.eNodeType = FDE_XMLNODE_Unknown;
            } else {
              m_CurNode = *pXMLNode;
            }
            m_iCurrentNodeNum = m_CurNode.iNodeNum;
            m_BlockBuffer.Reset();
            m_pCurrentBlock = m_BlockBuffer.GetAvailableBlock(m_iIndexInBlock);
            m_dwMode = FDE_XMLSYNTAXMODE_Text;
            dwStatus = FDE_XMLSYNTAXSTATUS_InstructionClose;
          }
          break;
        case FDE_XMLSYNTAXMODE_BreakElement:
          if (ch == L'>') {
            m_dwMode = FDE_XMLSYNTAXMODE_Text;
            dwStatus = FDE_XMLSYNTAXSTATUS_ElementBreak;
          } else if (ch == L'/') {
            m_dwMode = FDE_XMLSYNTAXMODE_CloseElement;
          } else {
            m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
            return m_dwStatus;
          }
          m_pStart++;
          break;
        case FDE_XMLSYNTAXMODE_CloseElement:
          if (!FDE_IsXMLNameChar(ch, m_iDataLength < 1)) {
            if (ch == L'>') {
              FDE_LPXMLNODE pXMLNode = m_XMLNodeStack.GetTopElement();
              if (pXMLNode == NULL) {
                m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
                return m_dwStatus;
              }
              m_XMLNodeStack.Pop();
              pXMLNode = m_XMLNodeStack.GetTopElement();
              if (pXMLNode == NULL) {
                m_CurNode.iNodeNum = -1;
                m_CurNode.eNodeType = FDE_XMLNODE_Unknown;
              } else {
                m_CurNode = *pXMLNode;
              }
              m_iCurrentNodeNum = m_CurNode.iNodeNum;
              m_iTextDataLength = m_iDataLength;
              m_BlockBuffer.Reset();
              m_pCurrentBlock =
                  m_BlockBuffer.GetAvailableBlock(m_iIndexInBlock);
              m_dwMode = FDE_XMLSYNTAXMODE_Text;
              dwStatus = FDE_XMLSYNTAXSTATUS_ElementClose;
            } else if (!FDE_IsXMLWhiteSpace(ch)) {
              m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
              return m_dwStatus;
            }
          } else {
            if (m_iIndexInBlock == m_iAllocStep) {
              m_pCurrentBlock =
                  m_BlockBuffer.GetAvailableBlock(m_iIndexInBlock);
              if (!m_pCurrentBlock) {
                return FDE_XMLSYNTAXSTATUS_Error;
              }
            }
            m_pCurrentBlock[m_iIndexInBlock++] = ch;
            m_iDataLength++;
          }
          m_pStart++;
          break;
        case FDE_XMLSYNTAXMODE_SkipCommentOrDecl:
          if (ch == '-') {
            m_dwMode = FDE_XMLSYNTAXMODE_SkipComment;
          } else {
            m_dwMode = FDE_XMLSYNTAXMODE_SkipDeclNode;
            m_SkipChar = L'>';
            m_SkipStack.Push(L'>');
          }
          break;
        case FDE_XMLSYNTAXMODE_SkipDeclNode:
          if (m_SkipChar == L'\'' || m_SkipChar == L'\"') {
            m_pStart++;
            if (ch != m_SkipChar) {
              break;
            }
            m_SkipStack.Pop();
            FX_DWORD* pDWord = m_SkipStack.GetTopElement();
            if (pDWord == NULL) {
              m_dwMode = FDE_XMLSYNTAXMODE_Text;
            } else {
              m_SkipChar = (FX_WCHAR)*pDWord;
            }
          } else {
            switch (ch) {
              case L'<':
                m_SkipChar = L'>';
                m_SkipStack.Push(L'>');
                break;
              case L'[':
                m_SkipChar = L']';
                m_SkipStack.Push(L']');
                break;
              case L'(':
                m_SkipChar = L')';
                m_SkipStack.Push(L')');
                break;
              case L'\'':
                m_SkipChar = L'\'';
                m_SkipStack.Push(L'\'');
                break;
              case L'\"':
                m_SkipChar = L'\"';
                m_SkipStack.Push(L'\"');
                break;
              default:
                if (ch == m_SkipChar) {
                  m_SkipStack.Pop();
                  FX_DWORD* pDWord = m_SkipStack.GetTopElement();
                  if (pDWord == NULL) {
                    if (m_iDataLength >= 9) {
                      CFX_WideString wsHeader;
                      m_BlockBuffer.GetTextData(wsHeader, 0, 7);
                      if (wsHeader.Equal(FX_WSTRC(L"[CDATA["))) {
                        CFX_WideString wsTailer;
                        m_BlockBuffer.GetTextData(wsTailer, m_iDataLength - 2,
                                                  2);
                        if (wsTailer.Equal(FX_WSTRC(L"]]"))) {
                          m_BlockBuffer.DeleteTextChars(7, TRUE);
                          m_BlockBuffer.DeleteTextChars(2, FALSE);
                          dwStatus = FDE_XMLSYNTAXSTATUS_CData;
                        }
                      }
                    }
                    m_iTextDataLength = m_iDataLength;
                    m_BlockBuffer.Reset();
                    m_pCurrentBlock =
                        m_BlockBuffer.GetAvailableBlock(m_iIndexInBlock);
                    m_dwMode = FDE_XMLSYNTAXMODE_Text;
                  } else {
                    m_SkipChar = (FX_WCHAR)*pDWord;
                  }
                }
                break;
            }
            if (m_SkipStack.GetSize() > 0) {
              if (m_iIndexInBlock == m_iAllocStep) {
                m_pCurrentBlock =
                    m_BlockBuffer.GetAvailableBlock(m_iIndexInBlock);
                if (!m_pCurrentBlock) {
                  return FDE_XMLSYNTAXSTATUS_Error;
                }
              }
              m_pCurrentBlock[m_iIndexInBlock++] = ch;
              m_iDataLength++;
            }
            m_pStart++;
          }
          break;
        case FDE_XMLSYNTAXMODE_SkipComment:
          if (ch == L'-') {
            if (m_iIndexInBlock == m_iAllocStep) {
              m_pCurrentBlock =
                  m_BlockBuffer.GetAvailableBlock(m_iIndexInBlock);
              if (!m_pCurrentBlock) {
                return FDE_XMLSYNTAXSTATUS_Error;
              }
            }
            m_pCurrentBlock[m_iIndexInBlock++] = L'-';
            m_iDataLength++;
          } else if (ch == L'>') {
            if (m_iDataLength > 1) {
              m_BlockBuffer.Reset();
              m_pCurrentBlock =
                  m_BlockBuffer.GetAvailableBlock(m_iIndexInBlock);
              m_dwMode = FDE_XMLSYNTAXMODE_Text;
            }
          } else {
            m_BlockBuffer.Reset();
            m_pCurrentBlock = m_BlockBuffer.GetAvailableBlock(m_iIndexInBlock);
          }
          m_pStart++;
          break;
        case FDE_XMLSYNTAXMODE_TargetData:
          if (FDE_IsXMLWhiteSpace(ch)) {
            if (m_iDataLength < 1) {
              m_pStart++;
              break;
            } else if (m_wQuotationMark == 0) {
              m_iTextDataLength = m_iDataLength;
              m_wQuotationMark = 0;
              m_BlockBuffer.Reset();
              m_pCurrentBlock =
                  m_BlockBuffer.GetAvailableBlock(m_iIndexInBlock);
              m_pStart++;
              dwStatus = FDE_XMLSYNTAXSTATUS_TargetData;
              break;
            }
          }
          if (ch == '?') {
            m_dwMode = FDE_XMLSYNTAXMODE_CloseInstruction;
            m_pStart++;
          } else if (ch == '\"') {
            if (m_wQuotationMark == 0) {
              m_wQuotationMark = ch;
              m_pStart++;
            } else if (ch == m_wQuotationMark) {
              m_iTextDataLength = m_iDataLength;
              m_wQuotationMark = 0;
              m_BlockBuffer.Reset();
              m_pCurrentBlock =
                  m_BlockBuffer.GetAvailableBlock(m_iIndexInBlock);
              m_pStart++;
              dwStatus = FDE_XMLSYNTAXSTATUS_TargetData;
            } else {
              m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
              return m_dwStatus;
            }
          } else {
            if (m_iIndexInBlock == m_iAllocStep) {
              m_pCurrentBlock =
                  m_BlockBuffer.GetAvailableBlock(m_iIndexInBlock);
              if (!m_pCurrentBlock) {
                return FDE_XMLSYNTAXSTATUS_Error;
              }
            }
            m_pCurrentBlock[m_iIndexInBlock++] = ch;
            m_iDataLength++;
            m_pStart++;
          }
          break;
        default:
          break;
      }
      if (dwStatus != FDE_XMLSYNTAXSTATUS_None) {
        return dwStatus;
      }
    }
  }
  return 0;
}
#else
CFDE_XMLSyntaxParser::CFDE_XMLSyntaxParser()
    : m_pStream(NULL),
      m_iXMLPlaneSize(-1),
      m_iTextDataSize(256),
      m_iCurrentPos(0),
      m_iCurrentNodeNum(-1),
      m_iLastNodeNum(-1),
      m_iParsedChars(0),
      m_iParsedBytes(0),
      m_pBuffer(NULL),
      m_iBufferChars(0),
      m_bEOS(FALSE),
      m_pStart(NULL),
      m_pEnd(NULL),
      m_XMLNodeStack(16),
      m_pwsTextData(NULL),
      m_iDataPos(0),
      m_dwStatus(FDE_XMLSYNTAXSTATUS_None),
      m_dwMode(FDE_XMLSYNTAXMODE_Text),
      m_wQuotationMark(0),
      m_iTextDataLength(0),
      m_iEntityStart(-1),
      m_SkipStack(16) {
  m_CurNode.iNodeNum = -1;
  m_CurNode.eNodeType = FDE_XMLNODE_Unknown;
}
void CFDE_XMLSyntaxParser::Init(IFX_Stream* pStream,
                                int32_t iXMLPlaneSize,
                                int32_t iTextDataSize) {
  FXSYS_assert(m_pStream == NULL && m_pBuffer == NULL);
  FXSYS_assert(pStream != NULL && iXMLPlaneSize > 0 && iTextDataSize > 0);
  int32_t iStreamLength = pStream->GetLength();
  FXSYS_assert(iStreamLength > 0);
  m_pStream = pStream;
  m_iXMLPlaneSize = std::min(iXMLPlaneSize, iStreamLength);
  m_iTextDataSize = iTextDataSize;
  uint8_t bom[4];
  m_iCurrentPos = m_pStream->GetBOM(bom);
  FXSYS_assert(m_pBuffer == NULL);
  m_pBuffer = FX_Alloc(FX_WCHAR, m_iXMLPlaneSize);
  m_pStart = m_pEnd = m_pBuffer;
  FXSYS_assert(m_pwsTextData == NULL);
  m_pwsTextData = FX_Alloc(FX_WCHAR, m_iTextDataSize);
  m_iParsedBytes = 0;
  m_iParsedChars = 0;
  m_iBufferChars = 0;
}
FX_DWORD CFDE_XMLSyntaxParser::DoSyntaxParse() {
  if (m_dwStatus == FDE_XMLSYNTAXSTATUS_Error ||
      m_dwStatus == FDE_XMLSYNTAXSTATUS_EOS) {
    return m_dwStatus;
  }
  FXSYS_assert(m_pStream != NULL && m_pBuffer != NULL && m_pwsTextData != NULL);
  int32_t iStreamLength = m_pStream->GetLength();
  int32_t iPos;
  FX_WCHAR ch;
  FX_DWORD dwStatus = FDE_XMLSYNTAXSTATUS_None;
  while (TRUE) {
    if (m_pStart >= m_pEnd) {
      if (m_bEOS || m_iCurrentPos >= iStreamLength) {
        m_dwStatus = FDE_XMLSYNTAXSTATUS_EOS;
        return m_dwStatus;
      }
      m_iParsedChars += (m_pEnd - m_pBuffer);
      m_iParsedBytes = m_iCurrentPos;
      m_pStream->Lock();
      if (m_pStream->GetPosition() != m_iCurrentPos) {
        m_pStream->Seek(FX_STREAMSEEK_Begin, m_iCurrentPos);
      }
      m_iBufferChars =
          m_pStream->ReadString(m_pBuffer, m_iXMLPlaneSize, m_bEOS);
      iPos = m_pStream->GetPosition();
      m_pStream->Unlock();
      if (m_iBufferChars < 1) {
        m_iCurrentPos = iStreamLength;
        m_dwStatus = FDE_XMLSYNTAXSTATUS_EOS;
        return m_dwStatus;
      }
      m_iCurrentPos = iPos;
      m_pStart = m_pBuffer;
      m_pEnd = m_pBuffer + m_iBufferChars;
    }
    while (m_pStart < m_pEnd) {
      ch = *m_pStart;
      switch (m_dwMode) {
        case FDE_XMLSYNTAXMODE_Text:
          if (ch == L'<') {
            if (m_iDataPos > 0) {
              m_iTextDataLength = m_iDataPos;
              m_iDataPos = 0;
              m_iEntityStart = -1;
              dwStatus = FDE_XMLSYNTAXSTATUS_Text;
            } else {
              m_pStart++;
              m_dwMode = FDE_XMLSYNTAXMODE_Node;
            }
          } else {
            ParseTextChar(ch);
          }
          break;
        case FDE_XMLSYNTAXMODE_Node:
          if (ch == L'!') {
            m_pStart++;
            m_dwMode = FDE_XMLSYNTAXMODE_SkipCommentOrDecl;
          } else if (ch == L'/') {
            m_pStart++;
            m_dwMode = FDE_XMLSYNTAXMODE_CloseElement;
          } else if (ch == L'?') {
            m_iLastNodeNum++;
            m_iCurrentNodeNum = m_iLastNodeNum;
            m_CurNode.iNodeNum = m_iLastNodeNum;
            m_CurNode.eNodeType = FDE_XMLNODE_Instruction;
            m_XMLNodeStack.Push(m_CurNode);
            m_pStart++;
            m_dwMode = FDE_XMLSYNTAXMODE_Target;
            dwStatus = FDE_XMLSYNTAXSTATUS_InstructionOpen;
          } else {
            m_iLastNodeNum++;
            m_iCurrentNodeNum = m_iLastNodeNum;
            m_CurNode.iNodeNum = m_iLastNodeNum;
            m_CurNode.eNodeType = FDE_XMLNODE_Element;
            m_XMLNodeStack.Push(m_CurNode);
            m_dwMode = FDE_XMLSYNTAXMODE_Tag;
            dwStatus = FDE_XMLSYNTAXSTATUS_ElementOpen;
          }
          break;
        case FDE_XMLSYNTAXMODE_Target:
        case FDE_XMLSYNTAXMODE_Tag:
          if (!FDE_IsXMLNameChar(ch, m_iDataPos < 1)) {
            if (m_iDataPos < 1) {
              m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
              return m_dwStatus;
            } else {
              m_iTextDataLength = m_iDataPos;
              m_iDataPos = 0;
              if (m_dwMode != FDE_XMLSYNTAXMODE_Target) {
                dwStatus = FDE_XMLSYNTAXSTATUS_TagName;
              } else {
                dwStatus = FDE_XMLSYNTAXSTATUS_TargetName;
              }
              m_dwMode = FDE_XMLSYNTAXMODE_AttriName;
            }
          } else {
            if (m_iDataPos >= m_iTextDataSize) {
              ReallocTextDataBuffer();
            }
            m_pwsTextData[m_iDataPos++] = ch;
            m_pStart++;
          }
          break;
        case FDE_XMLSYNTAXMODE_AttriName:
          if (m_iDataPos < 1 && FDE_IsXMLWhiteSpace(ch)) {
            m_pStart++;
            break;
          }
          if (!FDE_IsXMLNameChar(ch, m_iDataPos < 1)) {
            if (m_iDataPos < 1) {
              if (m_CurNode.eNodeType == FDE_XMLNODE_Element) {
                if (ch == L'>' || ch == L'/') {
                  m_dwMode = FDE_XMLSYNTAXMODE_BreakElement;
                  break;
                }
              } else if (m_CurNode.eNodeType == FDE_XMLNODE_Instruction) {
                if (ch == L'?') {
                  m_dwMode = FDE_XMLSYNTAXMODE_CloseInstruction;
                  m_pStart++;
                } else {
                  m_dwMode = FDE_XMLSYNTAXMODE_TargetData;
                }
                break;
              }
              m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
              return m_dwStatus;
            } else {
              if (m_CurNode.eNodeType == FDE_XMLNODE_Instruction) {
                if (ch != '=' && !FDE_IsXMLWhiteSpace(ch)) {
                  m_dwMode = FDE_XMLSYNTAXMODE_TargetData;
                  break;
                }
              }
              m_iTextDataLength = m_iDataPos;
              m_iDataPos = 0;
              m_dwMode = FDE_XMLSYNTAXMODE_AttriEqualSign;
              dwStatus = FDE_XMLSYNTAXSTATUS_AttriName;
            }
          } else {
            if (m_iDataPos >= m_iTextDataSize) {
              ReallocTextDataBuffer();
            }
            m_pwsTextData[m_iDataPos++] = ch;
            m_pStart++;
          }
          break;
        case FDE_XMLSYNTAXMODE_AttriEqualSign:
          if (FDE_IsXMLWhiteSpace(ch)) {
            m_pStart++;
            break;
          }
          if (ch != L'=') {
            if (m_CurNode.eNodeType == FDE_XMLNODE_Instruction) {
              m_dwMode = FDE_XMLSYNTAXMODE_TargetData;
              break;
            }
            m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
            return m_dwStatus;
          } else {
            m_dwMode = FDE_XMLSYNTAXMODE_AttriQuotation;
            m_pStart++;
          }
          break;
        case FDE_XMLSYNTAXMODE_AttriQuotation:
          if (FDE_IsXMLWhiteSpace(ch)) {
            m_pStart++;
            break;
          }
          if (ch != L'\"' && ch != L'\'') {
            m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
            return m_dwStatus;
          } else {
            m_wQuotationMark = ch;
            m_dwMode = FDE_XMLSYNTAXMODE_AttriValue;
            m_pStart++;
          }
          break;
        case FDE_XMLSYNTAXMODE_AttriValue:
          if (ch == m_wQuotationMark) {
            if (m_iEntityStart > -1) {
              m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
              return m_dwStatus;
            }
            m_iTextDataLength = m_iDataPos;
            m_wQuotationMark = 0;
            m_iDataPos = 0;
            m_pStart++;
            m_dwMode = FDE_XMLSYNTAXMODE_AttriName;
            dwStatus = FDE_XMLSYNTAXSTATUS_AttriValue;
          } else {
            ParseTextChar(ch);
          }
          break;
        case FDE_XMLSYNTAXMODE_CloseInstruction:
          if (ch != L'>') {
            if (m_iDataPos >= m_iTextDataSize) {
              ReallocTextDataBuffer();
            }
            m_pwsTextData[m_iDataPos++] = ch;
            m_dwMode = FDE_XMLSYNTAXMODE_TargetData;
          } else if (m_iDataPos > 0) {
            m_iTextDataLength = m_iDataPos;
            m_iDataPos = 0;
            dwStatus = FDE_XMLSYNTAXSTATUS_TargetData;
          } else {
            m_pStart++;
            FDE_LPXMLNODE pXMLNode = m_XMLNodeStack.GetTopElement();
            if (pXMLNode == NULL) {
              m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
              return m_dwStatus;
            }
            m_XMLNodeStack.Pop();
            pXMLNode = m_XMLNodeStack.GetTopElement();
            if (pXMLNode == NULL) {
              m_CurNode.iNodeNum = -1;
              m_CurNode.eNodeType = FDE_XMLNODE_Unknown;
            } else {
              m_CurNode = *pXMLNode;
            }
            m_iCurrentNodeNum = m_CurNode.iNodeNum;
            m_iDataPos = 0;
            m_dwMode = FDE_XMLSYNTAXMODE_Text;
            dwStatus = FDE_XMLSYNTAXSTATUS_InstructionClose;
          }
          break;
        case FDE_XMLSYNTAXMODE_BreakElement:
          if (ch == L'>') {
            m_dwMode = FDE_XMLSYNTAXMODE_Text;
            dwStatus = FDE_XMLSYNTAXSTATUS_ElementBreak;
          } else if (ch == L'/') {
            m_dwMode = FDE_XMLSYNTAXMODE_CloseElement;
          } else {
            m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
            return m_dwStatus;
          }
          m_pStart++;
          break;
        case FDE_XMLSYNTAXMODE_CloseElement:
          if (!FDE_IsXMLNameChar(ch, m_iDataPos < 1)) {
            if (ch == L'>') {
              FDE_LPXMLNODE pXMLNode = m_XMLNodeStack.GetTopElement();
              if (pXMLNode == NULL) {
                m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
                return m_dwStatus;
              }
              m_XMLNodeStack.Pop();
              pXMLNode = m_XMLNodeStack.GetTopElement();
              if (pXMLNode == NULL) {
                m_CurNode.iNodeNum = -1;
                m_CurNode.eNodeType = FDE_XMLNODE_Unknown;
              } else {
                m_CurNode = *pXMLNode;
              }
              m_iCurrentNodeNum = m_CurNode.iNodeNum;
              m_iTextDataLength = m_iDataPos;
              m_iDataPos = 0;
              m_dwMode = FDE_XMLSYNTAXMODE_Text;
              dwStatus = FDE_XMLSYNTAXSTATUS_ElementClose;
            } else if (!FDE_IsXMLWhiteSpace(ch)) {
              m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
              return m_dwStatus;
            }
          } else {
            if (m_iDataPos >= m_iTextDataSize) {
              ReallocTextDataBuffer();
            }
            m_pwsTextData[m_iDataPos++] = ch;
          }
          m_pStart++;
          break;
        case FDE_XMLSYNTAXMODE_SkipCommentOrDecl:
          if (ch == '-') {
            m_dwMode = FDE_XMLSYNTAXMODE_SkipComment;
          } else {
            m_dwMode = FDE_XMLSYNTAXMODE_SkipDeclNode;
            m_SkipChar = L'>';
            m_SkipStack.Push(L'>');
          }
          break;
        case FDE_XMLSYNTAXMODE_SkipDeclNode:
          if (m_SkipChar == L'\'' || m_SkipChar == L'\"') {
            m_pStart++;
            if (ch != m_SkipChar) {
              break;
            }
            m_SkipStack.Pop();
            FX_DWORD* pDWord = m_SkipStack.GetTopElement();
            if (pDWord == NULL) {
              m_dwMode = FDE_XMLSYNTAXMODE_Text;
            } else {
              m_SkipChar = (FX_WCHAR)*pDWord;
            }
          } else {
            switch (ch) {
              case L'<':
                m_SkipChar = L'>';
                m_SkipStack.Push(L'>');
                break;
              case L'[':
                m_SkipChar = L']';
                m_SkipStack.Push(L']');
                break;
              case L'(':
                m_SkipChar = L')';
                m_SkipStack.Push(L')');
                break;
              case L'\'':
                m_SkipChar = L'\'';
                m_SkipStack.Push(L'\'');
                break;
              case L'\"':
                m_SkipChar = L'\"';
                m_SkipStack.Push(L'\"');
                break;
              default:
                if (ch == m_SkipChar) {
                  m_SkipStack.Pop();
                  FX_DWORD* pDWord = m_SkipStack.GetTopElement();
                  if (pDWord == NULL) {
                    m_iTextDataLength = m_iDataPos;
                    m_iDataPos = 0;
                    if (m_iTextDataLength >= 9 &&
                        FXSYS_memcmp(m_pwsTextData, L"[CDATA[",
                                     7 * sizeof(FX_WCHAR)) == 0 &&
                        FXSYS_memcmp(m_pwsTextData + m_iTextDataLength - 2,
                                     L"]]", 2 * sizeof(FX_WCHAR)) == 0) {
                      m_iTextDataLength -= 9;
                      FXSYS_memmove(m_pwsTextData, m_pwsTextData + 7,
                                    m_iTextDataLength * sizeof(FX_WCHAR));
                      dwStatus = FDE_XMLSYNTAXSTATUS_CData;
                    }
                    m_dwMode = FDE_XMLSYNTAXMODE_Text;
                  } else {
                    m_SkipChar = (FX_WCHAR)*pDWord;
                  }
                }
                break;
            }
            if (m_SkipStack.GetSize() > 0) {
              if (m_iDataPos >= m_iTextDataSize) {
                ReallocTextDataBuffer();
              }
              m_pwsTextData[m_iDataPos++] = ch;
            }
            m_pStart++;
          }
          break;
        case FDE_XMLSYNTAXMODE_SkipComment:
          if (ch == L'-') {
            m_iDataPos++;
          } else if (ch == L'>') {
            if (m_iDataPos > 1) {
              m_iDataPos = 0;
              m_dwMode = FDE_XMLSYNTAXMODE_Text;
            }
          } else {
            m_iDataPos = 0;
          }
          m_pStart++;
          break;
        case FDE_XMLSYNTAXMODE_TargetData:
          if (FDE_IsXMLWhiteSpace(ch)) {
            if (m_iDataPos < 1) {
              m_pStart++;
              break;
            } else if (m_wQuotationMark == 0) {
              m_iTextDataLength = m_iDataPos;
              m_wQuotationMark = 0;
              m_iDataPos = 0;
              m_pStart++;
              dwStatus = FDE_XMLSYNTAXSTATUS_TargetData;
              break;
            }
          }
          if (ch == '?') {
            m_dwMode = FDE_XMLSYNTAXMODE_CloseInstruction;
            m_pStart++;
          } else if (ch == '\"') {
            if (m_wQuotationMark == 0) {
              m_wQuotationMark = ch;
              m_pStart++;
            } else if (ch == m_wQuotationMark) {
              m_iTextDataLength = m_iDataPos;
              m_wQuotationMark = 0;
              m_iDataPos = 0;
              m_pStart++;
              dwStatus = FDE_XMLSYNTAXSTATUS_TargetData;
            } else {
              m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
              return m_dwStatus;
            }
          } else {
            if (m_iDataPos >= m_iTextDataSize) {
              ReallocTextDataBuffer();
            }
            m_pwsTextData[m_iDataPos++] = ch;
            m_pStart++;
          }
          break;
        default:
          break;
      }
      if (dwStatus != FDE_XMLSYNTAXSTATUS_None) {
        return dwStatus;
      }
    }
  }
  return 0;
}
#endif
CFDE_XMLSyntaxParser::~CFDE_XMLSyntaxParser() {
#ifdef _FDE_BLOCK_BUFFER
  if (m_pCurrentBlock) {
    m_pCurrentBlock = NULL;
  }
#else
  FX_Free(m_pwsTextData);
#endif
  FX_Free(m_pBuffer);
}
int32_t CFDE_XMLSyntaxParser::GetStatus() const {
  if (m_pStream == NULL) {
    return -1;
  }
  int32_t iStreamLength = m_pStream->GetLength();
  if (iStreamLength < 1) {
    return 100;
  }
  if (m_dwStatus == FDE_XMLSYNTAXSTATUS_Error) {
    return -1;
  }
  if (m_dwStatus == FDE_XMLSYNTAXSTATUS_EOS) {
    return 100;
  }
  return m_iParsedBytes * 100 / iStreamLength;
}
static int32_t FX_GetUTF8EncodeLength(const FX_WCHAR* pSrc, int32_t iSrcLen) {
  FX_DWORD unicode = 0;
  int32_t iDstNum = 0;
  while (iSrcLen-- > 0) {
    unicode = *pSrc++;
    int nbytes = 0;
    if ((FX_DWORD)unicode < 0x80) {
      nbytes = 1;
    } else if ((FX_DWORD)unicode < 0x800) {
      nbytes = 2;
    } else if ((FX_DWORD)unicode < 0x10000) {
      nbytes = 3;
    } else if ((FX_DWORD)unicode < 0x200000) {
      nbytes = 4;
    } else if ((FX_DWORD)unicode < 0x4000000) {
      nbytes = 5;
    } else {
      nbytes = 6;
    }
    iDstNum += nbytes;
  }
  return iDstNum;
}
FX_FILESIZE CFDE_XMLSyntaxParser::GetCurrentBinaryPos() const {
  if (m_pStream == NULL) {
    return 0;
  }
  int32_t nSrcLen = m_pStart - m_pBuffer;
  int32_t nDstLen = FX_GetUTF8EncodeLength(m_pBuffer, nSrcLen);
  return m_iParsedBytes + nDstLen;
}
#ifdef _FDE_BLOCK_BUFFER
void CFDE_XMLSyntaxParser::ParseTextChar(FX_WCHAR ch) {
  if (m_iIndexInBlock == m_iAllocStep) {
    m_pCurrentBlock = m_BlockBuffer.GetAvailableBlock(m_iIndexInBlock);
    if (!m_pCurrentBlock) {
      return;
    }
  }
  m_pCurrentBlock[m_iIndexInBlock++] = ch;
  m_iDataLength++;
  if (m_iEntityStart > -1 && ch == L';') {
    CFX_WideString csEntity;
    m_BlockBuffer.GetTextData(csEntity, m_iEntityStart + 1,
                              (m_iDataLength - 1) - m_iEntityStart - 1);
    int32_t iLen = csEntity.GetLength();
    if (iLen > 0) {
      if (csEntity[0] == L'#') {
        ch = 0;
        FX_WCHAR w;
        if (iLen > 1 && csEntity[1] == L'x') {
          for (int32_t i = 2; i < iLen; i++) {
            w = csEntity[i];
            if (w >= L'0' && w <= L'9') {
              ch = (ch << 4) + w - L'0';
            } else if (w >= L'A' && w <= L'F') {
              ch = (ch << 4) + w - 55;
            } else if (w >= L'a' && w <= L'f') {
              ch = (ch << 4) + w - 87;
            } else {
              break;
            }
          }
        } else {
          for (int32_t i = 1; i < iLen; i++) {
            w = csEntity[i];
            if (w < L'0' || w > L'9') {
              break;
            }
            ch = ch * 10 + w - L'0';
          }
        }
        if (ch != 0) {
          m_BlockBuffer.SetTextChar(m_iEntityStart, ch);
          m_iEntityStart++;
        }
      } else {
        if (csEntity.Compare(L"amp") == 0) {
          m_BlockBuffer.SetTextChar(m_iEntityStart, L'&');
          m_iEntityStart++;
        } else if (csEntity.Compare(L"lt") == 0) {
          m_BlockBuffer.SetTextChar(m_iEntityStart, L'<');
          m_iEntityStart++;
        } else if (csEntity.Compare(L"gt") == 0) {
          m_BlockBuffer.SetTextChar(m_iEntityStart, L'>');
          m_iEntityStart++;
        } else if (csEntity.Compare(L"apos") == 0) {
          m_BlockBuffer.SetTextChar(m_iEntityStart, L'\'');
          m_iEntityStart++;
        } else if (csEntity.Compare(L"quot") == 0) {
          m_BlockBuffer.SetTextChar(m_iEntityStart, L'\"');
          m_iEntityStart++;
        }
      }
    }
    m_BlockBuffer.DeleteTextChars(m_iDataLength - m_iEntityStart, FALSE);
    m_pCurrentBlock = m_BlockBuffer.GetAvailableBlock(m_iIndexInBlock);
    m_iEntityStart = -1;
  } else {
    if (m_iEntityStart < 0 && ch == L'&') {
      m_iEntityStart = m_iDataLength - 1;
    }
  }
  m_pStart++;
}
#else
void CFDE_XMLSyntaxParser::ParseTextChar(FX_WCHAR ch) {
  if (m_iDataPos >= m_iTextDataSize) {
    ReallocTextDataBuffer();
  }
  m_pwsTextData[m_iDataPos] = ch;
  if (m_iEntityStart > -1 && ch == L';') {
    CFX_WideString csEntity(m_pwsTextData + m_iEntityStart + 1,
                            m_iDataPos - m_iEntityStart - 1);
    int32_t iLen = csEntity.GetLength();
    if (iLen > 0) {
      if (csEntity[0] == L'#') {
        ch = 0;
        FX_WCHAR w;
        if (iLen > 1 && csEntity[1] == L'x') {
          for (int32_t i = 2; i < iLen; i++) {
            w = csEntity[i];
            if (w >= L'0' && w <= L'9') {
              ch = (ch << 4) + w - L'0';
            } else if (w >= L'A' && w <= L'F') {
              ch = (ch << 4) + w - 55;
            } else if (w >= L'a' && w <= L'f') {
              ch = (ch << 4) + w - 87;
            } else {
              break;
            }
          }
        } else {
          for (int32_t i = 1; i < iLen; i++) {
            w = csEntity[i];
            if (w < L'0' || w > L'9') {
              break;
            }
            ch = ch * 10 + w - L'0';
          }
        }
        if (ch != 0) {
          m_pwsTextData[m_iEntityStart++] = ch;
        }
      } else {
        if (csEntity.Compare(L"amp") == 0) {
          m_pwsTextData[m_iEntityStart++] = L'&';
        } else if (csEntity.Compare(L"lt") == 0) {
          m_pwsTextData[m_iEntityStart++] = L'<';
        } else if (csEntity.Compare(L"gt") == 0) {
          m_pwsTextData[m_iEntityStart++] = L'>';
        } else if (csEntity.Compare(L"apos") == 0) {
          m_pwsTextData[m_iEntityStart++] = L'\'';
        } else if (csEntity.Compare(L"quot") == 0) {
          m_pwsTextData[m_iEntityStart++] = L'\"';
        }
      }
    }
    m_iDataPos = m_iEntityStart;
    m_iEntityStart = -1;
  } else {
    if (m_iEntityStart < 0 && ch == L'&') {
      m_iEntityStart = m_iDataPos;
    }
    m_iDataPos++;
  }
  m_pStart++;
}
void CFDE_XMLSyntaxParser::ReallocTextDataBuffer() {
  FXSYS_assert(m_pwsTextData != NULL);
  if (m_iTextDataSize <= 1024 * 1024) {
    m_iTextDataSize *= 2;
  } else {
    m_iTextDataSize += 1024 * 1024;
  }
  m_pwsTextData = FX_Realloc(FX_WCHAR, m_pwsTextData, m_iTextDataSize);
}
void CFDE_XMLSyntaxParser::GetData(CFX_WideString& wsData) const {
  FX_WCHAR* pBuf = wsData.GetBuffer(m_iTextDataLength);
  FXSYS_memcpy(pBuf, m_pwsTextData, m_iTextDataLength * sizeof(FX_WCHAR));
  wsData.ReleaseBuffer(m_iTextDataLength);
}
#endif
