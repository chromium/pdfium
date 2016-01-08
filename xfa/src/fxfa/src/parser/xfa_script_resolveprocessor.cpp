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
#include "xfa_script_resolveprocessor.h"
#include "xfa_script_nodehelper.h"
#include "xfa_script_imp.h"
CXFA_ResolveProcessor::CXFA_ResolveProcessor(void)
    : m_pNodeHelper(NULL), m_iCurStart(0) {
  m_pNodeHelper = new CXFA_NodeHelper;
}
CXFA_ResolveProcessor::~CXFA_ResolveProcessor(void) {
  if (m_pNodeHelper) {
    delete m_pNodeHelper;
    m_pNodeHelper = NULL;
  }
}
int32_t CXFA_ResolveProcessor::XFA_ResolveNodes(CXFA_ResolveNodesData& rnd) {
  if (rnd.m_CurNode == NULL) {
    return -1;
  }
  if (!rnd.m_CurNode->IsNode()) {
    if (rnd.m_dwStyles & XFA_RESOLVENODE_Attributes) {
      return XFA_ResolveNodes_ForAttributeRs(rnd.m_CurNode, rnd, rnd.m_wsName);
    }
    return 0;
  }
  if (rnd.m_dwStyles & XFA_RESOLVENODE_AnyChild) {
    return XFA_ResolveNodes_AnyChild(rnd);
  }
  FX_WCHAR wch = rnd.m_wsName.GetAt(0);
  switch (wch) {
    case '$':
      return XFA_ResolveNodes_Dollar(rnd);
    case '!':
      return XFA_ResolveNodes_Excalmatory(rnd);
    case '#':
      return XFA_ResolveNodes_NumberSign(rnd);
    case '*':
      return XFA_ResolveNodes_Asterisk(rnd);
    case '.':
      return XFA_ResolveNodes_AnyChild(rnd);
    default:
      break;
  }
  if (rnd.m_uHashName == XFA_HASHCODE_This && rnd.m_nLevel == 0) {
    rnd.m_Nodes.Add(rnd.m_pSC->GetThisObject());
    return 1;
  } else if (rnd.m_CurNode->GetClassID() == XFA_ELEMENT_Xfa) {
    CXFA_Object* pObjNode =
        rnd.m_pSC->GetDocument()->GetXFANode(rnd.m_uHashName);
    if (pObjNode) {
      rnd.m_Nodes.Add(pObjNode);
    } else if (rnd.m_uHashName == XFA_HASHCODE_Xfa) {
      rnd.m_Nodes.Add(rnd.m_CurNode);
    } else if ((rnd.m_dwStyles & XFA_RESOLVENODE_Attributes) &&
               XFA_ResolveNodes_ForAttributeRs(rnd.m_CurNode, rnd,
                                               rnd.m_wsName)) {
      return 1;
    }
    if (rnd.m_Nodes.GetSize() > 0) {
      XFA_ResolveNode_FilterCondition(rnd, rnd.m_wsCondition);
    }
    return rnd.m_Nodes.GetSize();
  }
  int32_t nRet = XFA_ResolveNodes_Normal(rnd);
  if (nRet < 1 && rnd.m_uHashName == XFA_HASHCODE_Xfa) {
    rnd.m_Nodes.Add(rnd.m_pSC->GetDocument()->GetRoot());
  }
  return rnd.m_Nodes.GetSize();
}
int32_t CXFA_ResolveProcessor::XFA_ResolveNodes_AnyChild(
    CXFA_ResolveNodesData& rnd) {
  CFX_WideString wsName = rnd.m_wsName.Right(rnd.m_wsName.GetLength() - 1);
  CFX_WideString wsCondition = rnd.m_wsCondition;
  CXFA_Node* findNode = NULL;
  CXFA_NodeArray siblings;
  FX_BOOL bClassName = FALSE;
  if (wsName.GetAt(0) == '#') {
    bClassName = TRUE;
    wsName = wsName.Right(wsName.GetLength() - 1);
  }
  findNode = m_pNodeHelper->XFA_ResolveNodes_GetOneChild(
      (CXFA_Node*)rnd.m_CurNode, wsName, bClassName);
  if (findNode == NULL) {
    return 0;
  }
  if (wsCondition.IsEmpty()) {
    rnd.m_Nodes.Add(findNode);
    return rnd.m_Nodes.GetSize();
  }
  m_pNodeHelper->XFA_CountSiblings(findNode, XFA_LOGIC_Transparent,
                                   (CXFA_NodeArray*)&rnd.m_Nodes, bClassName);
  XFA_ResolveNode_FilterCondition(rnd, wsCondition);
  return rnd.m_Nodes.GetSize();
}
int32_t CXFA_ResolveProcessor::XFA_ResolveNodes_Dollar(
    CXFA_ResolveNodesData& rnd) {
  CXFA_ObjArray& nodes = rnd.m_Nodes;
  CFX_WideString wsName = rnd.m_wsName;
  CFX_WideString wsCondition = rnd.m_wsCondition;
  int32_t iNameLen = wsName.GetLength();
  if (iNameLen == 1) {
    nodes.Add(rnd.m_CurNode);
    return 1;
  }
  if (rnd.m_nLevel > 0) {
    return -1;
  }
  FX_DWORD dwNameHash =
      FX_HashCode_String_GetW((const FX_WCHAR*)wsName + 1, iNameLen - 1);
  if (dwNameHash == XFA_HASHCODE_Xfa) {
    nodes.Add(rnd.m_pSC->GetDocument()->GetRoot());
  } else {
    CXFA_Object* pObjNode = rnd.m_pSC->GetDocument()->GetXFANode(dwNameHash);
    if (pObjNode) {
      rnd.m_Nodes.Add(pObjNode);
    }
  }
  if (rnd.m_Nodes.GetSize() > 0) {
    XFA_ResolveNode_FilterCondition(rnd, wsCondition);
  }
  return rnd.m_Nodes.GetSize();
}
int32_t CXFA_ResolveProcessor::XFA_ResolveNodes_Excalmatory(
    CXFA_ResolveNodesData& rnd) {
  if (rnd.m_nLevel > 0) {
    return 0;
  }
  CXFA_Node* datasets =
      (CXFA_Node*)rnd.m_pSC->GetDocument()->GetXFANode(XFA_HASHCODE_Datasets);
  if (datasets == NULL) {
    return 0;
  }
  CXFA_ResolveNodesData rndFind;
  rndFind.m_pSC = rnd.m_pSC;
  rndFind.m_CurNode = datasets;
  rndFind.m_wsName = rnd.m_wsName.Right(rnd.m_wsName.GetLength() - 1);
  rndFind.m_uHashName =
      FX_HashCode_String_GetW(rndFind.m_wsName, rndFind.m_wsName.GetLength());
  rndFind.m_nLevel = rnd.m_nLevel + 1;
  rndFind.m_dwStyles = XFA_RESOLVENODE_Children;
  rndFind.m_wsCondition = rnd.m_wsCondition;
  XFA_ResolveNodes(rndFind);
  if (rndFind.m_Nodes.GetSize() > 0) {
    rnd.m_Nodes.Append(rndFind.m_Nodes);
    rndFind.m_Nodes.RemoveAll();
  }
  return rnd.m_Nodes.GetSize();
}
int32_t CXFA_ResolveProcessor::XFA_ResolveNodes_NumberSign(
    CXFA_ResolveNodesData& rnd) {
  CFX_WideString wsName = rnd.m_wsName.Right(rnd.m_wsName.GetLength() - 1);
  CFX_WideString wsCondition = rnd.m_wsCondition;
  CXFA_Node* curNode = (CXFA_Node*)rnd.m_CurNode;
  if (XFA_ResolveNodes_ForAttributeRs(curNode, rnd, wsName)) {
    return 1;
  }
  CXFA_ResolveNodesData rndFind;
  rndFind.m_pSC = rnd.m_pSC;
  rndFind.m_nLevel = rnd.m_nLevel + 1;
  rndFind.m_dwStyles = rnd.m_dwStyles;
  rndFind.m_dwStyles |= XFA_RESOLVENODE_TagName;
  rndFind.m_dwStyles &= ~XFA_RESOLVENODE_Attributes;
  rndFind.m_wsName = wsName;
  rndFind.m_uHashName =
      FX_HashCode_String_GetW(rndFind.m_wsName, rndFind.m_wsName.GetLength());
  rndFind.m_wsCondition = wsCondition;
  rndFind.m_CurNode = curNode;
  XFA_ResolveNodes_Normal(rndFind);
  if (rndFind.m_Nodes.GetSize() > 0) {
    if (wsCondition.GetLength() == 0 && rndFind.m_Nodes.Find(curNode) >= 0) {
      rnd.m_Nodes.Add(curNode);
    } else {
      rnd.m_Nodes.Append(rndFind.m_Nodes);
      rndFind.m_Nodes.RemoveAll();
    }
  }
  return rnd.m_Nodes.GetSize();
}
int32_t CXFA_ResolveProcessor::XFA_ResolveNodes_ForAttributeRs(
    CXFA_Object* curNode,
    CXFA_ResolveNodesData& rnd,
    const CFX_WideStringC& strAttr) {
  XFA_LPCSCRIPTATTRIBUTEINFO lpScriptAttribute =
      XFA_GetScriptAttributeByName(curNode->GetClassID(), strAttr);
  if (lpScriptAttribute) {
    rnd.m_pScriptAttribute = lpScriptAttribute;
    rnd.m_Nodes.Add(curNode);
    rnd.m_dwFlag = XFA_RESOVENODE_RSTYPE_Attribute;
    return 1;
  }
  return 0;
}
int32_t CXFA_ResolveProcessor::XFA_ResolveNodes_Normal(
    CXFA_ResolveNodesData& rnd) {
  if (rnd.m_nLevel > 32) {
    return 0;
  }
  if (!rnd.m_CurNode->IsNode()) {
    return 0;
  }
  CXFA_Node* curNode = (CXFA_Node*)rnd.m_CurNode;
  CXFA_ObjArray& nodes = rnd.m_Nodes;
  int32_t nNum = nodes.GetSize();
  FX_DWORD dwStyles = rnd.m_dwStyles;
  CFX_WideString& wsName = rnd.m_wsName;
  uint32_t uNameHash = rnd.m_uHashName;
  CFX_WideString& wsCondition = rnd.m_wsCondition;
  CXFA_ResolveNodesData rndFind;
  rndFind.m_wsName = rnd.m_wsName;
  rndFind.m_wsCondition = rnd.m_wsCondition;
  rndFind.m_pSC = rnd.m_pSC;
  rndFind.m_nLevel = rnd.m_nLevel + 1;
  rndFind.m_uHashName = uNameHash;
  CXFA_NodeArray children;
  CXFA_NodeArray properties;
  CXFA_Node* pVariablesNode = NULL;
  CXFA_Node* pPageSetNode = NULL;
  CXFA_Node* pChild = curNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  while (pChild) {
    if (pChild->GetClassID() == XFA_ELEMENT_Variables) {
      pVariablesNode = pChild;
      pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling);
      continue;
    } else if (pChild->GetClassID() == XFA_ELEMENT_PageSet) {
      pPageSetNode = pChild;
      pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling);
      continue;
    } else {
      XFA_LPCPROPERTY pPropert = XFA_GetPropertyOfElement(
          curNode->GetClassID(), pChild->GetClassID(), XFA_XDPPACKET_UNKNOWN);
      if (pPropert) {
        properties.Add(pChild);
      } else {
        children.Add(pChild);
      }
    }
    pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling);
  }
  if ((dwStyles & XFA_RESOLVENODE_Properties) && pVariablesNode) {
    uint32_t uPropHash = pVariablesNode->GetClassHashCode();
    if (uPropHash == uNameHash) {
      nodes.Add(pVariablesNode);
    } else {
      rndFind.m_CurNode = pVariablesNode;
      XFA_ResolveNodes_SetStylesForChild(dwStyles, rndFind);
      CFX_WideString wsSaveCondition = rndFind.m_wsCondition;
      rndFind.m_wsCondition.Empty();
      XFA_ResolveNodes_Normal(rndFind);
      rndFind.m_wsCondition = wsSaveCondition;
      if (rndFind.m_Nodes.GetSize() > 0) {
        nodes.Append(rndFind.m_Nodes);
        rndFind.m_Nodes.RemoveAll();
      }
    }
    if (nodes.GetSize() > nNum) {
      XFA_ResolveNode_FilterCondition(rnd, wsCondition);
      if (nodes.GetSize() > 0) {
        return 1;
      }
      return 0;
    }
  }
  if (dwStyles & XFA_RESOLVENODE_Children) {
    FX_BOOL bSetFlag = FALSE;
    if (pPageSetNode && (dwStyles & XFA_RESOLVENODE_Properties)) {
      children.Add(pPageSetNode);
    }
    for (int32_t i = 0; i < children.GetSize(); i++) {
      CXFA_Node* child = children[i];
      if (dwStyles & XFA_RESOLVENODE_TagName) {
        if (child->GetClassHashCode() == uNameHash) {
          nodes.Add(child);
        }
      } else if (child->GetNameHash() == uNameHash) {
        nodes.Add(child);
      }
      if (m_pNodeHelper->XFA_NodeIsTransparent(child) &&
          child->GetClassID() != XFA_ELEMENT_PageSet) {
        if (!bSetFlag) {
          XFA_ResolveNodes_SetStylesForChild(dwStyles, rndFind);
          bSetFlag = TRUE;
        }
        rndFind.m_CurNode = child;
        CFX_WideString wsSaveCondition = rndFind.m_wsCondition;
        rndFind.m_wsCondition.Empty();
        XFA_ResolveNodes_Normal(rndFind);
        rndFind.m_wsCondition = wsSaveCondition;
        if (rndFind.m_Nodes.GetSize() > 0) {
          nodes.Append(rndFind.m_Nodes);
          rndFind.m_Nodes.RemoveAll();
        }
      }
    }
    if (nodes.GetSize() > nNum) {
      if (!(dwStyles & XFA_RESOLVENODE_ALL)) {
        CXFA_NodeArray upArrayNodes;
        if (m_pNodeHelper->XFA_NodeIsTransparent((CXFA_Node*)curNode)) {
          m_pNodeHelper->XFA_CountSiblings(
              (CXFA_Node*)nodes[0], XFA_LOGIC_Transparent, &upArrayNodes,
              !!(dwStyles & XFA_RESOLVENODE_TagName));
        }
        if (upArrayNodes.GetSize() > nodes.GetSize()) {
          upArrayNodes[0] = (CXFA_Node*)nodes[0];
          nodes.RemoveAll();
          nodes.Append((CXFA_ObjArray&)upArrayNodes);
          upArrayNodes.RemoveAll();
        }
      }
      XFA_ResolveNode_FilterCondition(rnd, wsCondition);
      if (nodes.GetSize() > 0) {
        return 1;
      }
      return 0;
    }
  }
  if (dwStyles & XFA_RESOLVENODE_Attributes) {
    if (XFA_ResolveNodes_ForAttributeRs(curNode, rnd, wsName)) {
      return 1;
    }
  }
  if (dwStyles & XFA_RESOLVENODE_Properties) {
    for (int32_t i = 0; i < properties.GetSize(); i++) {
      CXFA_Node* childProperty = properties[i];
      if (childProperty->IsUnnamed()) {
        uint32_t uPropHash = childProperty->GetClassHashCode();
        if (uPropHash == uNameHash) {
          nodes.Add(childProperty);
        }
      } else if (childProperty->GetNameHash() == uNameHash &&
                 childProperty->GetClassID() != XFA_ELEMENT_Extras &&
                 childProperty->GetClassID() != XFA_ELEMENT_Items) {
        nodes.Add(childProperty);
      }
    }
    if (nodes.GetSize() > nNum) {
      XFA_ResolveNode_FilterCondition(rnd, wsCondition);
      if (nodes.GetSize() > 0) {
        return 1;
      }
      return 0;
    }
    CXFA_Node* pProp = NULL;
    if (XFA_ELEMENT_Subform == curNode->GetClassID() &&
        XFA_HASHCODE_Occur == uNameHash) {
      CXFA_Node* pInstanceManager =
          ((CXFA_Node*)curNode)->GetInstanceMgrOfSubform();
      if (pInstanceManager) {
        pProp = pInstanceManager->GetProperty(0, XFA_ELEMENT_Occur, TRUE);
      }
    } else {
      XFA_LPCELEMENTINFO pElement = XFA_GetElementByName(wsName);
      if (pElement) {
        pProp = ((CXFA_Node*)curNode)
                    ->GetProperty(0, pElement->eName,
                                  pElement->eName != XFA_ELEMENT_PageSet);
      }
    }
    if (pProp) {
      nodes.Add(pProp);
      return nodes.GetSize();
    }
  }
  CXFA_Node* parentNode = m_pNodeHelper->XFA_ResolveNodes_GetParent(
      (CXFA_Node*)curNode, XFA_LOGIC_NoTransparent);
  uint32_t uCurClassHash = curNode->GetClassHashCode();
  if (parentNode == NULL) {
    if (uCurClassHash == uNameHash) {
      nodes.Add((CXFA_Node*)curNode);
      XFA_ResolveNode_FilterCondition(rnd, wsCondition);
      if (nodes.GetSize() > 0) {
        return 1;
      }
    }
    return 0;
  }
  if (dwStyles & XFA_RESOLVENODE_Siblings) {
    CXFA_Node* child = parentNode->GetNodeItem(XFA_NODEITEM_FirstChild);
    FX_DWORD dwSubStyles =
        XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Properties;
    if (dwStyles & XFA_RESOLVENODE_TagName) {
      dwSubStyles |= XFA_RESOLVENODE_TagName;
    }
    if (dwStyles & XFA_RESOLVENODE_ALL) {
      dwSubStyles |= XFA_RESOLVENODE_ALL;
    }
    rndFind.m_dwStyles = dwSubStyles;
    while (child) {
      if (child == curNode) {
        if (dwStyles & XFA_RESOLVENODE_TagName) {
          if (uCurClassHash == uNameHash) {
            nodes.Add(curNode);
          }
        } else {
          if (child->GetNameHash() == uNameHash) {
            nodes.Add(curNode);
            if (rnd.m_nLevel == 0 && wsCondition.GetLength() == 0) {
              nodes.RemoveAll();
              nodes.Add(curNode);
              return 1;
            }
          }
        }
        child = child->GetNodeItem(XFA_NODEITEM_NextSibling);
        continue;
      }
      if (dwStyles & XFA_RESOLVENODE_TagName) {
        if (child->GetClassHashCode() == uNameHash) {
          nodes.Add(child);
        }
      } else if (child->GetNameHash() == uNameHash) {
        nodes.Add(child);
      }
      XFA_LPCPROPERTY pPropert = XFA_GetPropertyOfElement(
          parentNode->GetClassID(), child->GetClassID(), XFA_XDPPACKET_UNKNOWN);
      FX_BOOL bInnerSearch = FALSE;
      if (pPropert) {
        if ((child->GetClassID() == XFA_ELEMENT_Variables ||
             child->GetClassID() == XFA_ELEMENT_PageSet)) {
          bInnerSearch = TRUE;
        }
      } else {
        if (m_pNodeHelper->XFA_NodeIsTransparent(child)) {
          bInnerSearch = TRUE;
        }
      }
      if (bInnerSearch) {
        rndFind.m_CurNode = child;
        CFX_WideString wsOriginCondition = rndFind.m_wsCondition;
        rndFind.m_wsCondition.Empty();
        FX_DWORD dwOriginStyle = rndFind.m_dwStyles;
        rndFind.m_dwStyles = dwOriginStyle | XFA_RESOLVENODE_ALL;
        XFA_ResolveNodes_Normal(rndFind);
        rndFind.m_dwStyles = dwOriginStyle;
        rndFind.m_wsCondition = wsOriginCondition;
        if (rndFind.m_Nodes.GetSize() > 0) {
          nodes.Append(rndFind.m_Nodes);
          rndFind.m_Nodes.RemoveAll();
        }
      }
      child = child->GetNodeItem(XFA_NODEITEM_NextSibling);
    }
    if (nodes.GetSize() > nNum) {
      if (m_pNodeHelper->XFA_NodeIsTransparent(parentNode)) {
        CXFA_NodeArray upArrayNodes;
        m_pNodeHelper->XFA_CountSiblings(
            (CXFA_Node*)nodes[0], XFA_LOGIC_Transparent, &upArrayNodes,
            !!(dwStyles & XFA_RESOLVENODE_TagName));
        if (upArrayNodes.GetSize() > nodes.GetSize()) {
          upArrayNodes[0] = (CXFA_Node*)nodes[0];
          nodes.RemoveAll();
          nodes.Append((CXFA_ObjArray&)upArrayNodes);
          upArrayNodes.RemoveAll();
        }
      }
      XFA_ResolveNode_FilterCondition(rnd, wsCondition);
      if (nodes.GetSize() > 0) {
        return 1;
      }
      return 0;
    }
  }
  if (dwStyles & XFA_RESOLVENODE_Parent) {
    FX_DWORD dwSubStyles = XFA_RESOLVENODE_Siblings | XFA_RESOLVENODE_Parent |
                           XFA_RESOLVENODE_Properties;
    if (dwStyles & XFA_RESOLVENODE_TagName) {
      dwSubStyles |= XFA_RESOLVENODE_TagName;
    }
    if (dwStyles & XFA_RESOLVENODE_ALL) {
      dwSubStyles |= XFA_RESOLVENODE_ALL;
    }
    rndFind.m_dwStyles = dwSubStyles;
    rndFind.m_CurNode = parentNode;
    CXFA_NodeArray& array = rnd.m_pSC->GetUpObjectArray();
    array.Add(parentNode);
    XFA_ResolveNodes_Normal(rndFind);
    if (rndFind.m_Nodes.GetSize() > 0) {
      nodes.Append(rndFind.m_Nodes);
      rndFind.m_Nodes.RemoveAll();
    }
    if (nodes.GetSize() > nNum) {
      return 1;
    }
  }
  return 0;
}
int32_t CXFA_ResolveProcessor::XFA_ResolveNodes_Asterisk(
    CXFA_ResolveNodesData& rnd) {
  CXFA_Node* curNode = (CXFA_Node*)rnd.m_CurNode;
  CXFA_ObjArray& nodes = rnd.m_Nodes;
  CXFA_NodeArray array;
  curNode->GetNodeList(array,
                       XFA_NODEFILTER_Children | XFA_NODEFILTER_Properties);
  nodes.Append((CXFA_ObjArray&)array);
  return nodes.GetSize();
}
int32_t CXFA_ResolveProcessor::XFA_ResolveNodes_PopStack(
    CFX_Int32Array& stack) {
  int32_t nType = -1;
  int32_t iSize = stack.GetSize() - 1;
  if (iSize > -1) {
    nType = stack[iSize];
    stack.RemoveAt(iSize, 1);
  }
  return nType;
}
int32_t CXFA_ResolveProcessor::XFA_ResolveNodes_GetFilter(
    const CFX_WideStringC& wsExpression,
    int32_t nStart,
    CXFA_ResolveNodesData& rnd) {
  FXSYS_assert(nStart > -1);
  int32_t iLength = wsExpression.GetLength();
  if (nStart >= iLength) {
    return 0;
  }
  CFX_WideString& wsName = rnd.m_wsName;
  CFX_WideString& wsCondition = rnd.m_wsCondition;
  FX_WCHAR* pNameBuf = wsName.GetBuffer(iLength - nStart);
  FX_WCHAR* pConditionBuf = wsCondition.GetBuffer(iLength - nStart);
  int32_t nNameCount = 0;
  int32_t nConditionCount = 0;
  CFX_Int32Array stack;
  int32_t nType = -1;
  const FX_WCHAR* pSrc = wsExpression.GetPtr();
  FX_WCHAR wPrev = 0, wCur;
  FX_BOOL bIsCondition = FALSE;
  while (nStart < iLength) {
    wCur = pSrc[nStart++];
    if (wCur == '.') {
      if (wPrev == '\\') {
        pNameBuf[nNameCount - 1] = wPrev = '.';
        continue;
      }
      if (nNameCount == 0) {
        pNameBuf[nNameCount++] = wCur;
        continue;
      }
      FX_WCHAR wLookahead = nStart < iLength ? pSrc[nStart] : 0;
      if (wLookahead != '[' && wLookahead != '(') {
        if (nType < 0) {
          break;
        }
      }
    }
    if (wCur == '[' || wCur == '(') {
      bIsCondition = TRUE;
    } else if (wCur == '.' && nStart < iLength &&
               (pSrc[nStart] == '[' || pSrc[nStart] == '(')) {
      bIsCondition = TRUE;
    }
    if (bIsCondition) {
      pConditionBuf[nConditionCount++] = wCur;
    } else {
      pNameBuf[nNameCount++] = wCur;
    }
    FX_BOOL bRecursive = TRUE;
    switch (nType) {
      case 0:
        if (wCur == ']') {
          nType = XFA_ResolveNodes_PopStack(stack);
          bRecursive = FALSE;
        }
        break;
      case 1:
        if (wCur == ')') {
          nType = XFA_ResolveNodes_PopStack(stack);
          bRecursive = FALSE;
        }
        break;
      case 2:
        if (wCur == '"') {
          nType = XFA_ResolveNodes_PopStack(stack);
          bRecursive = FALSE;
        }
        break;
    }
    if (bRecursive) {
      switch (wCur) {
        case '[':
          stack.Add(nType);
          nType = 0;
          break;
        case '(':
          stack.Add(nType);
          nType = 1;
          break;
        case '"':
          stack.Add(nType);
          nType = 2;
          break;
      }
    }
    wPrev = wCur;
  }
  if (stack.GetSize() > 0) {
    return -1;
  }
  wsName.ReleaseBuffer(nNameCount);
  wsName.TrimLeft();
  wsName.TrimRight();
  wsCondition.ReleaseBuffer(nConditionCount);
  wsCondition.TrimLeft();
  wsCondition.TrimRight();
  rnd.m_uHashName = FX_HashCode_String_GetW(wsName, wsName.GetLength());
  return nStart;
}
void CXFA_ResolveProcessor::XFA_ResolveNode_ConditionArray(
    int32_t iCurIndex,
    CFX_WideString wsCondition,
    int32_t iFoundCount,
    CXFA_ResolveNodesData& rnd) {
  CXFA_NodeArray& findNodes = (CXFA_NodeArray&)rnd.m_Nodes;
  int32_t iLen = wsCondition.GetLength();
  FX_BOOL bRelative = FALSE;
  FX_BOOL bAll = FALSE;
  int32_t i = 1;
  for (; i < iLen; ++i) {
    FX_WCHAR ch = wsCondition[i];
    if (ch == ' ') {
      continue;
    }
    if (ch == '+' || ch == '-') {
      bRelative = TRUE;
      break;
    } else if (ch == '*') {
      bAll = TRUE;
      break;
    } else {
      break;
    }
  }
  if (bAll) {
    if (rnd.m_dwStyles & XFA_RESOLVENODE_CreateNode) {
      if (rnd.m_dwStyles & XFA_RESOLVENODE_Bind) {
        m_pNodeHelper->m_pCreateParent = (CXFA_Node*)rnd.m_CurNode;
        m_pNodeHelper->m_iCreateCount = 1;
        findNodes.RemoveAll();
        m_pNodeHelper->m_iCurAllStart = -1;
        m_pNodeHelper->m_pAllStartParent = NULL;
      } else {
        if (m_pNodeHelper->m_iCurAllStart == -1) {
          m_pNodeHelper->m_iCurAllStart = m_iCurStart;
          m_pNodeHelper->m_pAllStartParent = (CXFA_Node*)rnd.m_CurNode;
        }
      }
    } else if (rnd.m_dwStyles & XFA_RESOLVENODE_BindNew) {
      if (m_pNodeHelper->m_iCurAllStart == -1) {
        m_pNodeHelper->m_iCurAllStart = m_iCurStart;
      }
    }
    return;
  }
  if (iFoundCount == 1 && !iLen) {
    return;
  }
  CFX_WideString wsIndex;
  wsIndex = wsCondition.Mid(i, iLen - 1 - i);
  int32_t iIndex = wsIndex.GetInteger();
  if (bRelative) {
    iIndex += iCurIndex;
  }
  if (iFoundCount <= iIndex || iIndex < 0) {
    if (rnd.m_dwStyles & XFA_RESOLVENODE_CreateNode) {
      m_pNodeHelper->m_pCreateParent = (CXFA_Node*)rnd.m_CurNode;
      m_pNodeHelper->m_iCreateCount = iIndex - iFoundCount + 1;
    }
    findNodes.RemoveAll();
  } else {
    CXFA_Node* ret = findNodes[iIndex];
    findNodes.RemoveAll();
    findNodes.Add(ret);
  }
}
void CXFA_ResolveProcessor::XFA_ResolveNode_DoPredicateFilter(
    int32_t iCurIndex,
    CFX_WideString wsCondition,
    int32_t iFoundCount,
    CXFA_ResolveNodesData& rnd) {
  CXFA_NodeArray& findNodes = (CXFA_NodeArray&)rnd.m_Nodes;
  FXSYS_assert(iFoundCount == findNodes.GetSize());
  CFX_WideString wsExpression;
  IXFA_ScriptContext* pContext = NULL;
  XFA_SCRIPTLANGTYPE eLangType = XFA_SCRIPTLANGTYPE_Unkown;
  if (wsCondition.Left(2) == FX_WSTRC(L".[") &&
      wsCondition.Right(1) == FX_WSTRC(L"]")) {
    eLangType = XFA_SCRIPTLANGTYPE_Formcalc;
  } else if (wsCondition.Left(2) == FX_WSTRC(L".(") &&
             wsCondition.Right(1) == FX_WSTRC(L")")) {
    eLangType = XFA_SCRIPTLANGTYPE_Javascript;
  } else {
    return;
  }
  pContext = rnd.m_pSC;
  wsExpression = wsCondition.Mid(2, wsCondition.GetLength() - 3);
  for (int32_t i = iFoundCount - 1; i >= 0; i--) {
    CXFA_Object* node = findNodes[i];
    FX_BOOL bRet = FALSE;
    FXJSE_HVALUE pRetValue = FXJSE_Value_Create(rnd.m_pSC->GetRuntime());
    bRet = pContext->RunScript(eLangType, wsExpression, pRetValue, node);
    if (!bRet || !FXJSE_Value_ToBoolean(pRetValue)) {
      findNodes.RemoveAt(i);
    }
    FXJSE_Value_Release(pRetValue);
  }
  return;
}
void CXFA_ResolveProcessor::XFA_ResolveNode_FilterCondition(
    CXFA_ResolveNodesData& rnd,
    CFX_WideString wsCondition) {
  CXFA_NodeArray& findNodes = (CXFA_NodeArray&)rnd.m_Nodes;
  int32_t iCurrIndex = 0;
  const CXFA_NodeArray& array = rnd.m_pSC->GetUpObjectArray();
  int32_t iSize = array.GetSize();
  if (iSize) {
    CXFA_Node* curNode = array[iSize - 1];
    FX_BOOL bIsProperty = m_pNodeHelper->XFA_NodeIsProperty(curNode);
    if (curNode->IsUnnamed() ||
        (bIsProperty && curNode->GetClassID() != XFA_ELEMENT_PageSet)) {
      iCurrIndex = m_pNodeHelper->XFA_GetIndex(curNode, XFA_LOGIC_Transparent,
                                               bIsProperty, TRUE);
    } else {
      iCurrIndex = m_pNodeHelper->XFA_GetIndex(curNode, XFA_LOGIC_Transparent,
                                               bIsProperty, FALSE);
    }
  }
  int32_t iFoundCount = findNodes.GetSize();
  wsCondition.TrimLeft();
  wsCondition.TrimRight();
  int32_t iLen = wsCondition.GetLength();
  if (!iLen) {
    if (rnd.m_dwStyles & XFA_RESOLVENODE_ALL) {
      return;
    }
    if (iFoundCount == 1) {
      return;
    }
    if (iFoundCount <= iCurrIndex) {
      if (rnd.m_dwStyles & XFA_RESOLVENODE_CreateNode) {
        m_pNodeHelper->m_pCreateParent = (CXFA_Node*)rnd.m_CurNode;
        m_pNodeHelper->m_iCreateCount = iCurrIndex - iFoundCount + 1;
      }
      findNodes.RemoveAll();
      return;
    } else {
      CXFA_Node* ret = findNodes[iCurrIndex];
      findNodes.RemoveAll();
      findNodes.Add(ret);
      return;
    }
  }
  FX_WCHAR wTypeChar = wsCondition[0];
  switch (wTypeChar) {
    case '[':
      XFA_ResolveNode_ConditionArray(iCurrIndex, wsCondition, iFoundCount, rnd);
      return;
    case '(':
      return;
    case '"':
      return;
    case '.':
      if (iLen > 1 && (wsCondition[1] == '[' || wsCondition[1] == '(')) {
        XFA_ResolveNode_DoPredicateFilter(iCurrIndex, wsCondition, iFoundCount,
                                          rnd);
      }
    default:
      return;
  }
}
void CXFA_ResolveProcessor::XFA_ResolveNodes_SetStylesForChild(
    FX_DWORD dwParentStyles,
    CXFA_ResolveNodesData& rnd) {
  FX_DWORD dwSubStyles = XFA_RESOLVENODE_Children;
  if (dwParentStyles & XFA_RESOLVENODE_TagName) {
    dwSubStyles |= XFA_RESOLVENODE_TagName;
  }
  dwSubStyles &= ~XFA_RESOLVENODE_Parent;
  dwSubStyles &= ~XFA_RESOLVENODE_Siblings;
  dwSubStyles &= ~XFA_RESOLVENODE_Properties;
  dwSubStyles |= XFA_RESOLVENODE_ALL;
  rnd.m_dwStyles = dwSubStyles;
}
int32_t CXFA_ResolveProcessor::XFA_ResolveNode_SetResultCreateNode(
    XFA_RESOLVENODE_RS& resolveNodeRS,
    CFX_WideString& wsLastCondition) {
  if (m_pNodeHelper->m_pCreateParent) {
    resolveNodeRS.nodes.Add(m_pNodeHelper->m_pCreateParent);
  } else {
    m_pNodeHelper->XFA_CreateNode_ForCondition(wsLastCondition);
  }
  resolveNodeRS.dwFlags = m_pNodeHelper->m_iCreateFlag;
  if (resolveNodeRS.dwFlags == XFA_RESOLVENODE_RSTYPE_CreateNodeOne) {
    if (m_pNodeHelper->m_iCurAllStart != -1) {
      resolveNodeRS.dwFlags = XFA_RESOLVENODE_RSTYPE_CreateNodeMidAll;
    }
  }
  return resolveNodeRS.nodes.GetSize();
}
void CXFA_ResolveProcessor::XFA_ResolveNode_SetIndexDataBind(
    CFX_WideString& wsNextCondition,
    int32_t& iIndex,
    int32_t iCount) {
  if (m_pNodeHelper->XFA_CreateNode_ForCondition(wsNextCondition)) {
    if (m_pNodeHelper->m_eLastCreateType == XFA_ELEMENT_DataGroup) {
      iIndex = 0;
    } else {
      iIndex = iCount - 1;
    }
  } else {
    iIndex = iCount - 1;
  }
}
