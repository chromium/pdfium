// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cfxjse_resolveprocessor.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "core/fxcrt/fx_extension.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "fxjs/xfa/cfxjse_value.h"
#include "fxjs/xfa/cjx_object.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_nodehelper.h"
#include "xfa/fxfa/parser/cxfa_object.h"
#include "xfa/fxfa/parser/cxfa_occur.h"
#include "xfa/fxfa/parser/xfa_resolvenode_rs.h"
#include "xfa/fxfa/parser/xfa_utils.h"

namespace {

void DoPredicateFilter(WideString wsCondition,
                       size_t iFoundCount,
                       CFXJSE_ResolveNodeData* pRnd) {
  ASSERT(iFoundCount == pRnd->m_Objects.size());
  WideString wsExpression;
  CXFA_Script::Type eLangType = CXFA_Script::Type::Unknown;
  if (wsCondition.Left(2).EqualsASCII(".[") && wsCondition.Last() == L']')
    eLangType = CXFA_Script::Type::Formcalc;
  else if (wsCondition.Left(2).EqualsASCII(".(") && wsCondition.Last() == L')')
    eLangType = CXFA_Script::Type::Javascript;
  else
    return;

  wsExpression = wsCondition.Mid(2, wsCondition.GetLength() - 3);
  for (size_t i = iFoundCount; i > 0; --i) {
    auto pRetValue =
        pdfium::MakeUnique<CFXJSE_Value>(pRnd->m_pSC->GetIsolate());
    bool bRet =
        pRnd->m_pSC->RunScript(eLangType, wsExpression.AsStringView(),
                               pRetValue.get(), pRnd->m_Objects[i - 1].Get());
    if (!bRet || !pRetValue->ToBoolean())
      pRnd->m_Objects.erase(pRnd->m_Objects.begin() + i - 1);
  }
}

}  // namespace

CFXJSE_ResolveProcessor::CFXJSE_ResolveProcessor()
    : m_pNodeHelper(pdfium::MakeUnique<CXFA_NodeHelper>()) {}

CFXJSE_ResolveProcessor::~CFXJSE_ResolveProcessor() = default;

bool CFXJSE_ResolveProcessor::Resolve(CFXJSE_ResolveNodeData& rnd) {
  if (!rnd.m_CurObject)
    return false;

  if (!rnd.m_CurObject->IsNode()) {
    if (rnd.m_dwStyles & XFA_RESOLVENODE_Attributes) {
      return ResolveForAttributeRs(rnd.m_CurObject.Get(), rnd,
                                   rnd.m_wsName.AsStringView());
    }
    return false;
  }
  if (rnd.m_dwStyles & XFA_RESOLVENODE_AnyChild)
    return ResolveAnyChild(rnd);

  if (rnd.m_wsName.GetLength()) {
    wchar_t wch = rnd.m_wsName[0];
    switch (wch) {
      case '$':
        return ResolveDollar(rnd);
      case '!':
        return ResolveExcalmatory(rnd);
      case '#':
        return ResolveNumberSign(rnd);
      case '*':
        return ResolveAsterisk(rnd);
      // TODO(dsinclair): We could probably remove this.
      case '.':
        return ResolveAnyChild(rnd);
      default:
        break;
    }
  }
  if (rnd.m_uHashName == XFA_HASHCODE_This && rnd.m_nLevel == 0) {
    rnd.m_Objects.emplace_back(rnd.m_pSC->GetThisObject());
    return true;
  }
  if (rnd.m_CurObject->GetElementType() == XFA_Element::Xfa) {
    CXFA_Object* pObjNode =
        rnd.m_pSC->GetDocument()->GetXFAObject(rnd.m_uHashName);
    if (pObjNode) {
      rnd.m_Objects.emplace_back(pObjNode);
    } else if (rnd.m_uHashName == XFA_HASHCODE_Xfa) {
      rnd.m_Objects.push_back(rnd.m_CurObject);
    } else if ((rnd.m_dwStyles & XFA_RESOLVENODE_Attributes) &&
               ResolveForAttributeRs(rnd.m_CurObject.Get(), rnd,
                                     rnd.m_wsName.AsStringView())) {
      return true;
    }
    if (!rnd.m_Objects.empty())
      FilterCondition(rnd.m_wsCondition, &rnd);

    return !rnd.m_Objects.empty();
  }
  if (!ResolveNormal(rnd) && rnd.m_uHashName == XFA_HASHCODE_Xfa)
    rnd.m_Objects.emplace_back(rnd.m_pSC->GetDocument()->GetRoot());

  return !rnd.m_Objects.empty();
}

bool CFXJSE_ResolveProcessor::ResolveAnyChild(CFXJSE_ResolveNodeData& rnd) {
  CXFA_Node* pParent = ToNode(rnd.m_CurObject.Get());
  if (!pParent)
    return false;

  WideStringView wsName = rnd.m_wsName.AsStringView();
  WideString wsCondition = rnd.m_wsCondition;
  const bool bClassName = !wsName.IsEmpty() && wsName[0] == '#';
  CXFA_Node* const pChild =
      bClassName
          ? pParent->GetOneChildOfClass(wsName.Right(wsName.GetLength() - 1))
          : pParent->GetOneChildNamed(wsName);
  if (!pChild)
    return false;

  if (wsCondition.IsEmpty()) {
    rnd.m_Objects.emplace_back(pChild);
    return true;
  }

  std::vector<CXFA_Node*> nodes;
  for (const auto& pObject : rnd.m_Objects)
    nodes.push_back(pObject->AsNode());

  std::vector<CXFA_Node*> siblings = pChild->GetSiblings(bClassName);
  nodes.insert(nodes.end(), siblings.begin(), siblings.end());
  rnd.m_Objects =
      std::vector<UnownedPtr<CXFA_Object>>(nodes.begin(), nodes.end());
  FilterCondition(wsCondition, &rnd);
  return !rnd.m_Objects.empty();
}

bool CFXJSE_ResolveProcessor::ResolveDollar(CFXJSE_ResolveNodeData& rnd) {
  WideString wsName = rnd.m_wsName;
  WideString wsCondition = rnd.m_wsCondition;
  int32_t iNameLen = wsName.GetLength();
  if (iNameLen == 1) {
    rnd.m_Objects.push_back(rnd.m_CurObject);
    return true;
  }
  if (rnd.m_nLevel > 0)
    return false;

  XFA_HashCode dwNameHash = static_cast<XFA_HashCode>(
      FX_HashCode_GetW(wsName.AsStringView().Right(iNameLen - 1), false));
  if (dwNameHash == XFA_HASHCODE_Xfa) {
    rnd.m_Objects.emplace_back(rnd.m_pSC->GetDocument()->GetRoot());
  } else {
    CXFA_Object* pObjNode = rnd.m_pSC->GetDocument()->GetXFAObject(dwNameHash);
    if (pObjNode)
      rnd.m_Objects.emplace_back(pObjNode);
  }
  if (!rnd.m_Objects.empty())
    FilterCondition(wsCondition, &rnd);
  return !rnd.m_Objects.empty();
}

bool CFXJSE_ResolveProcessor::ResolveExcalmatory(CFXJSE_ResolveNodeData& rnd) {
  if (rnd.m_nLevel > 0)
    return false;

  CXFA_Node* datasets =
      ToNode(rnd.m_pSC->GetDocument()->GetXFAObject(XFA_HASHCODE_Datasets));
  if (!datasets)
    return false;

  CFXJSE_ResolveNodeData rndFind(rnd.m_pSC.Get());
  rndFind.m_CurObject = datasets;
  rndFind.m_wsName = rnd.m_wsName.Right(rnd.m_wsName.GetLength() - 1);
  rndFind.m_uHashName = static_cast<XFA_HashCode>(
      FX_HashCode_GetW(rndFind.m_wsName.AsStringView(), false));
  rndFind.m_nLevel = rnd.m_nLevel + 1;
  rndFind.m_dwStyles = XFA_RESOLVENODE_Children;
  rndFind.m_wsCondition = rnd.m_wsCondition;
  Resolve(rndFind);

  rnd.m_Objects.insert(rnd.m_Objects.end(), rndFind.m_Objects.begin(),
                       rndFind.m_Objects.end());
  return !rnd.m_Objects.empty();
}

bool CFXJSE_ResolveProcessor::ResolveNumberSign(CFXJSE_ResolveNodeData& rnd) {
  WideString wsName = rnd.m_wsName.Right(rnd.m_wsName.GetLength() - 1);
  WideString wsCondition = rnd.m_wsCondition;
  CXFA_Node* curNode = ToNode(rnd.m_CurObject.Get());
  if (ResolveForAttributeRs(curNode, rnd, wsName.AsStringView()))
    return true;

  CFXJSE_ResolveNodeData rndFind(rnd.m_pSC.Get());
  rndFind.m_nLevel = rnd.m_nLevel + 1;
  rndFind.m_dwStyles = rnd.m_dwStyles;
  rndFind.m_dwStyles |= XFA_RESOLVENODE_TagName;
  rndFind.m_dwStyles &= ~XFA_RESOLVENODE_Attributes;
  rndFind.m_wsName = std::move(wsName);
  rndFind.m_uHashName = static_cast<XFA_HashCode>(
      FX_HashCode_GetW(rndFind.m_wsName.AsStringView(), false));
  rndFind.m_wsCondition = wsCondition;
  rndFind.m_CurObject = curNode;
  ResolveNormal(rndFind);
  if (rndFind.m_Objects.empty())
    return false;

  if (wsCondition.GetLength() == 0 &&
      pdfium::ContainsValue(rndFind.m_Objects, curNode)) {
    rnd.m_Objects.emplace_back(curNode);
  } else {
    rnd.m_Objects.insert(rnd.m_Objects.end(), rndFind.m_Objects.begin(),
                         rndFind.m_Objects.end());
  }
  return !rnd.m_Objects.empty();
}

bool CFXJSE_ResolveProcessor::ResolveForAttributeRs(CXFA_Object* curNode,
                                                    CFXJSE_ResolveNodeData& rnd,
                                                    WideStringView strAttr) {
  Optional<XFA_SCRIPTATTRIBUTEINFO> info =
      XFA_GetScriptAttributeByName(curNode->GetElementType(), strAttr);
  if (!info.has_value())
    return false;

  rnd.m_ScriptAttribute = info.value();
  rnd.m_Objects.emplace_back(curNode);
  rnd.m_dwFlag = XFA_ResolveNode_RSType_Attribute;
  return true;
}

bool CFXJSE_ResolveProcessor::ResolveNormal(CFXJSE_ResolveNodeData& rnd) {
  if (rnd.m_nLevel > 32 || !rnd.m_CurObject->IsNode())
    return false;

  CXFA_Node* curNode = rnd.m_CurObject->AsNode();
  size_t nNum = rnd.m_Objects.size();
  uint32_t dwStyles = rnd.m_dwStyles;
  WideString& wsName = rnd.m_wsName;
  XFA_HashCode uNameHash = rnd.m_uHashName;
  WideString& wsCondition = rnd.m_wsCondition;

  CFXJSE_ResolveNodeData rndFind(rnd.m_pSC.Get());
  rndFind.m_wsName = rnd.m_wsName;
  rndFind.m_wsCondition = rnd.m_wsCondition;
  rndFind.m_nLevel = rnd.m_nLevel + 1;
  rndFind.m_uHashName = uNameHash;

  std::vector<CXFA_Node*> children;
  std::vector<CXFA_Node*> properties;
  CXFA_Node* pVariablesNode = nullptr;
  CXFA_Node* pPageSetNode = nullptr;
  for (CXFA_Node* pChild = curNode->GetFirstChild(); pChild;
       pChild = pChild->GetNextSibling()) {
    if (pChild->GetElementType() == XFA_Element::Variables) {
      pVariablesNode = pChild;
      continue;
    }
    if (pChild->GetElementType() == XFA_Element::PageSet) {
      pPageSetNode = pChild;
      continue;
    }
    if (curNode->HasProperty(pChild->GetElementType()))
      properties.push_back(pChild);
    else
      children.push_back(pChild);
  }
  if ((dwStyles & XFA_RESOLVENODE_Properties) && pVariablesNode) {
    if (pVariablesNode->GetClassHashCode() == uNameHash) {
      rnd.m_Objects.emplace_back(pVariablesNode);
    } else {
      rndFind.m_CurObject = pVariablesNode;
      SetStylesForChild(dwStyles, rndFind);
      WideString wsSaveCondition = std::move(rndFind.m_wsCondition);
      ResolveNormal(rndFind);
      rndFind.m_wsCondition = std::move(wsSaveCondition);
      rnd.m_Objects.insert(rnd.m_Objects.end(), rndFind.m_Objects.begin(),
                           rndFind.m_Objects.end());
      rndFind.m_Objects.clear();
    }
    if (rnd.m_Objects.size() > nNum) {
      FilterCondition(wsCondition, &rnd);
      return !rnd.m_Objects.empty();
    }
  }

  if (dwStyles & XFA_RESOLVENODE_Children) {
    bool bSetFlag = false;
    if (pPageSetNode && (dwStyles & XFA_RESOLVENODE_Properties))
      children.push_back(pPageSetNode);

    for (CXFA_Node* child : children) {
      if (dwStyles & XFA_RESOLVENODE_TagName) {
        if (child->GetClassHashCode() == uNameHash)
          rnd.m_Objects.emplace_back(child);
      } else if (child->GetNameHash() == uNameHash) {
        rnd.m_Objects.emplace_back(child);
      }

      if (child->GetElementType() != XFA_Element::PageSet &&
          child->IsTransparent()) {
        if (!bSetFlag) {
          SetStylesForChild(dwStyles, rndFind);
          bSetFlag = true;
        }
        rndFind.m_CurObject = child;

        WideString wsSaveCondition = std::move(rndFind.m_wsCondition);
        ResolveNormal(rndFind);
        rndFind.m_wsCondition = std::move(wsSaveCondition);
        rnd.m_Objects.insert(rnd.m_Objects.end(), rndFind.m_Objects.begin(),
                             rndFind.m_Objects.end());
        rndFind.m_Objects.clear();
      }
    }
    if (rnd.m_Objects.size() > nNum) {
      if (!(dwStyles & XFA_RESOLVENODE_ALL)) {
        std::vector<CXFA_Node*> upArrayNodes;
        if (curNode->IsTransparent()) {
          CXFA_Node* pCurrent = ToNode(rnd.m_Objects.front().Get());
          if (pCurrent) {
            upArrayNodes =
                pCurrent->GetSiblings(!!(dwStyles & XFA_RESOLVENODE_TagName));
          }
        }
        if (upArrayNodes.size() > rnd.m_Objects.size()) {
          CXFA_Object* pSaveObject = rnd.m_Objects.front().Get();
          rnd.m_Objects = std::vector<UnownedPtr<CXFA_Object>>(
              upArrayNodes.begin(), upArrayNodes.end());
          rnd.m_Objects.front() = pSaveObject;
        }
      }
      FilterCondition(wsCondition, &rnd);
      return !rnd.m_Objects.empty();
    }
  }
  if (dwStyles & XFA_RESOLVENODE_Attributes) {
    if (ResolveForAttributeRs(curNode, rnd, wsName.AsStringView()))
      return 1;
  }
  if (dwStyles & XFA_RESOLVENODE_Properties) {
    for (CXFA_Node* pChildProperty : properties) {
      if (pChildProperty->IsUnnamed()) {
        if (pChildProperty->GetClassHashCode() == uNameHash)
          rnd.m_Objects.emplace_back(pChildProperty);
        continue;
      }
      if (pChildProperty->GetNameHash() == uNameHash &&
          pChildProperty->GetElementType() != XFA_Element::Extras &&
          pChildProperty->GetElementType() != XFA_Element::Items) {
        rnd.m_Objects.emplace_back(pChildProperty);
      }
    }
    if (rnd.m_Objects.size() > nNum) {
      FilterCondition(wsCondition, &rnd);
      return !rnd.m_Objects.empty();
    }

    CXFA_Node* pProp = nullptr;
    if (XFA_Element::Subform == curNode->GetElementType() &&
        XFA_HASHCODE_Occur == uNameHash) {
      CXFA_Node* pInstanceManager = curNode->GetInstanceMgrOfSubform();
      if (pInstanceManager) {
        pProp = pInstanceManager->JSObject()->GetOrCreateProperty<CXFA_Occur>(
            0, XFA_Element::Occur);
      }
    } else {
      XFA_Element eType = XFA_GetElementByName(wsName.AsStringView());
      if (eType == XFA_Element::PageSet) {
        pProp = curNode->JSObject()->GetProperty<CXFA_Node>(0, eType);
      } else if (eType != XFA_Element::Unknown) {
        pProp = curNode->JSObject()->GetOrCreateProperty<CXFA_Node>(0, eType);
      }
    }
    if (pProp) {
      rnd.m_Objects.emplace_back(pProp);
      return !rnd.m_Objects.empty();
    }
  }

  CXFA_Node* const parentNode = curNode->GetParent();
  uint32_t uCurClassHash = curNode->GetClassHashCode();
  if (!parentNode) {
    if (uCurClassHash == uNameHash) {
      rnd.m_Objects.emplace_back(curNode);
      FilterCondition(wsCondition, &rnd);
      if (!rnd.m_Objects.empty())
        return true;
    }
    return false;
  }

  if (dwStyles & XFA_RESOLVENODE_Siblings) {
    CXFA_Node* child = parentNode->GetFirstChild();
    uint32_t dwSubStyles =
        XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Properties;
    if (dwStyles & XFA_RESOLVENODE_TagName)
      dwSubStyles |= XFA_RESOLVENODE_TagName;
    if (dwStyles & XFA_RESOLVENODE_ALL)
      dwSubStyles |= XFA_RESOLVENODE_ALL;

    rndFind.m_dwStyles = dwSubStyles;
    while (child) {
      if (child == curNode) {
        if (dwStyles & XFA_RESOLVENODE_TagName) {
          if (uCurClassHash == uNameHash)
            rnd.m_Objects.emplace_back(curNode);
        } else {
          if (child->GetNameHash() == uNameHash) {
            rnd.m_Objects.emplace_back(curNode);
            if (rnd.m_nLevel == 0 && wsCondition.GetLength() == 0) {
              rnd.m_Objects.clear();
              rnd.m_Objects.emplace_back(curNode);
              return true;
            }
          }
        }
        child = child->GetNextSibling();
        continue;
      }

      if (dwStyles & XFA_RESOLVENODE_TagName) {
        if (child->GetClassHashCode() == uNameHash)
          rnd.m_Objects.emplace_back(child);
      } else if (child->GetNameHash() == uNameHash) {
        rnd.m_Objects.emplace_back(child);
      }

      bool bInnerSearch = false;
      if (parentNode->HasProperty(child->GetElementType())) {
        if ((child->GetElementType() == XFA_Element::Variables ||
             child->GetElementType() == XFA_Element::PageSet)) {
          bInnerSearch = true;
        }
      } else if (child->IsTransparent()) {
        bInnerSearch = true;
      }
      if (bInnerSearch) {
        rndFind.m_CurObject = child;
        WideString wsOriginCondition = std::move(rndFind.m_wsCondition);
        uint32_t dwOriginStyle = rndFind.m_dwStyles;
        rndFind.m_dwStyles = dwOriginStyle | XFA_RESOLVENODE_ALL;
        ResolveNormal(rndFind);
        rndFind.m_dwStyles = dwOriginStyle;
        rndFind.m_wsCondition = std::move(wsOriginCondition);
        rnd.m_Objects.insert(rnd.m_Objects.end(), rndFind.m_Objects.begin(),
                             rndFind.m_Objects.end());
        rndFind.m_Objects.clear();
      }
      child = child->GetNextSibling();
    }
    if (rnd.m_Objects.size() > nNum) {
      if (parentNode->IsTransparent()) {
        std::vector<CXFA_Node*> upArrayNodes;
        CXFA_Node* pCurrent = ToNode(rnd.m_Objects.front().Get());
        if (pCurrent) {
          upArrayNodes =
              pCurrent->GetSiblings(!!(dwStyles & XFA_RESOLVENODE_TagName));
        }
        if (upArrayNodes.size() > rnd.m_Objects.size()) {
          CXFA_Object* pSaveObject = rnd.m_Objects.front().Get();
          rnd.m_Objects = std::vector<UnownedPtr<CXFA_Object>>(
              upArrayNodes.begin(), upArrayNodes.end());
          rnd.m_Objects.front() = pSaveObject;
        }
      }
      FilterCondition(wsCondition, &rnd);
      return !rnd.m_Objects.empty();
    }
  }

  if (dwStyles & XFA_RESOLVENODE_Parent) {
    uint32_t dwSubStyles = XFA_RESOLVENODE_Siblings | XFA_RESOLVENODE_Parent |
                           XFA_RESOLVENODE_Properties;
    if (dwStyles & XFA_RESOLVENODE_TagName)
      dwSubStyles |= XFA_RESOLVENODE_TagName;
    if (dwStyles & XFA_RESOLVENODE_ALL)
      dwSubStyles |= XFA_RESOLVENODE_ALL;

    rndFind.m_dwStyles = dwSubStyles;
    rndFind.m_CurObject = parentNode;
    rnd.m_pSC->GetUpObjectArray()->push_back(parentNode);
    ResolveNormal(rndFind);
    rnd.m_Objects.insert(rnd.m_Objects.end(), rndFind.m_Objects.begin(),
                         rndFind.m_Objects.end());
    rndFind.m_Objects.clear();
    if (rnd.m_Objects.size() > nNum)
      return true;
  }
  return false;
}

bool CFXJSE_ResolveProcessor::ResolveAsterisk(CFXJSE_ResolveNodeData& rnd) {
  CXFA_Node* curNode = ToNode(rnd.m_CurObject.Get());
  std::vector<CXFA_Node*> array = curNode->GetNodeListWithFilter(
      XFA_NODEFILTER_Children | XFA_NODEFILTER_Properties);
  rnd.m_Objects.insert(rnd.m_Objects.end(), array.begin(), array.end());
  return !rnd.m_Objects.empty();
}

int32_t CFXJSE_ResolveProcessor::GetFilter(WideStringView wsExpression,
                                           int32_t nStart,
                                           CFXJSE_ResolveNodeData& rnd) {
  ASSERT(nStart > -1);

  int32_t iLength = wsExpression.GetLength();
  if (nStart >= iLength)
    return 0;

  WideString& wsName = rnd.m_wsName;
  WideString& wsCondition = rnd.m_wsCondition;
  int32_t nNameCount = 0;
  int32_t nConditionCount = 0;
  {
    // Span's lifetime must end before ReleaseBuffer() below.
    pdfium::span<wchar_t> pNameBuf = wsName.GetBuffer(iLength - nStart);
    pdfium::span<wchar_t> pConditionBuf =
        wsCondition.GetBuffer(iLength - nStart);
    pdfium::span<const wchar_t> pSrc = wsExpression.span();
    std::vector<int32_t> stack;
    int32_t nType = -1;
    wchar_t wPrev = 0;
    wchar_t wCur;
    bool bIsCondition = false;
    while (nStart < iLength) {
      wCur = pSrc[nStart++];
      if (wCur == '.') {
        if (nNameCount == 0) {
          rnd.m_dwStyles |= XFA_RESOLVENODE_AnyChild;
          continue;
        }
        if (wPrev == '\\') {
          pNameBuf[nNameCount - 1] = wPrev = '.';
          continue;
        }

        wchar_t wLookahead = nStart < iLength ? pSrc[nStart] : 0;
        if (wLookahead != '[' && wLookahead != '(' && nType < 0)
          break;
      }
      if (wCur == '[' || wCur == '(') {
        bIsCondition = true;
      } else if (wCur == '.' && nStart < iLength &&
                 (pSrc[nStart] == '[' || pSrc[nStart] == '(')) {
        bIsCondition = true;
      }
      if (bIsCondition)
        pConditionBuf[nConditionCount++] = wCur;
      else
        pNameBuf[nNameCount++] = wCur;

      if ((nType == 0 && wCur == ']') || (nType == 1 && wCur == ')') ||
          (nType == 2 && wCur == '"')) {
        nType = stack.empty() ? -1 : stack.back();
        if (!stack.empty())
          stack.pop_back();
      } else if (wCur == '[') {
        stack.push_back(nType);
        nType = 0;
      } else if (wCur == '(') {
        stack.push_back(nType);
        nType = 1;
      } else if (wCur == '"') {
        stack.push_back(nType);
        nType = 2;
      }
      wPrev = wCur;
    }
    if (!stack.empty())
      return -1;
  }
  wsName.ReleaseBuffer(nNameCount);
  wsCondition.ReleaseBuffer(nConditionCount);
  wsName.Trim();
  wsCondition.Trim();
  rnd.m_uHashName =
      static_cast<XFA_HashCode>(FX_HashCode_GetW(wsName.AsStringView(), false));
  return nStart;
}

void CFXJSE_ResolveProcessor::ConditionArray(size_t iCurIndex,
                                             WideString wsCondition,
                                             size_t iFoundCount,
                                             CFXJSE_ResolveNodeData* pRnd) {
  size_t iLen = wsCondition.GetLength();
  bool bRelative = false;
  bool bAll = false;
  size_t i = 1;
  for (; i < iLen; ++i) {
    wchar_t ch = wsCondition[i];
    if (ch == ' ')
      continue;
    if (ch == '+' || ch == '-')
      bRelative = true;
    else if (ch == '*')
      bAll = true;

    break;
  }
  if (bAll) {
    if (pRnd->m_dwStyles & XFA_RESOLVENODE_CreateNode) {
      if (pRnd->m_dwStyles & XFA_RESOLVENODE_Bind) {
        m_pNodeHelper->m_pCreateParent = ToNode(pRnd->m_CurObject.Get());
        m_pNodeHelper->m_iCreateCount = 1;
        pRnd->m_Objects.clear();
        m_pNodeHelper->m_iCurAllStart = -1;
        m_pNodeHelper->m_pAllStartParent = nullptr;
      } else if (m_pNodeHelper->m_iCurAllStart == -1) {
        m_pNodeHelper->m_iCurAllStart = m_iCurStart;
        m_pNodeHelper->m_pAllStartParent = ToNode(pRnd->m_CurObject.Get());
      }
    } else if (pRnd->m_dwStyles & XFA_RESOLVENODE_BindNew) {
      if (m_pNodeHelper->m_iCurAllStart == -1)
        m_pNodeHelper->m_iCurAllStart = m_iCurStart;
    }
    return;
  }
  if (iFoundCount == 1 && !iLen)
    return;

  int32_t iIndex = wsCondition.Mid(i, iLen - 1 - i).GetInteger();
  if (bRelative)
    iIndex += iCurIndex;

  if (iIndex < 0 || static_cast<size_t>(iIndex) >= iFoundCount) {
    if (pRnd->m_dwStyles & XFA_RESOLVENODE_CreateNode) {
      m_pNodeHelper->m_pCreateParent = ToNode(pRnd->m_CurObject.Get());
      m_pNodeHelper->m_iCreateCount = iIndex - iFoundCount + 1;
    }
    pRnd->m_Objects.clear();
  } else {
    pRnd->m_Objects =
        std::vector<UnownedPtr<CXFA_Object>>(1, pRnd->m_Objects[iIndex]);
  }
}

void CFXJSE_ResolveProcessor::FilterCondition(WideString wsCondition,
                                              CFXJSE_ResolveNodeData* pRnd) {
  size_t iCurIndex = 0;
  const std::vector<CXFA_Node*>* pArray = pRnd->m_pSC->GetUpObjectArray();
  if (!pArray->empty()) {
    CXFA_Node* pNode = pArray->back();
    bool bIsProperty = pNode->IsProperty();
    bool bIsClassIndex =
        pNode->IsUnnamed() ||
        (bIsProperty && pNode->GetElementType() != XFA_Element::PageSet);
    iCurIndex = pNode->GetIndex(bIsProperty, bIsClassIndex);
  }

  size_t iFoundCount = pRnd->m_Objects.size();
  wsCondition.Trim();

  int32_t iLen = wsCondition.GetLength();
  if (!iLen) {
    if (pRnd->m_dwStyles & XFA_RESOLVENODE_ALL)
      return;
    if (iFoundCount == 1)
      return;

    if (iFoundCount <= iCurIndex) {
      if (pRnd->m_dwStyles & XFA_RESOLVENODE_CreateNode) {
        m_pNodeHelper->m_pCreateParent = ToNode(pRnd->m_CurObject.Get());
        m_pNodeHelper->m_iCreateCount = iCurIndex - iFoundCount + 1;
      }
      pRnd->m_Objects.clear();
      return;
    }

    pRnd->m_Objects =
        std::vector<UnownedPtr<CXFA_Object>>(1, pRnd->m_Objects[iCurIndex]);
    return;
  }

  wchar_t wTypeChar = wsCondition[0];
  switch (wTypeChar) {
    case '[':
      ConditionArray(iCurIndex, wsCondition, iFoundCount, pRnd);
      return;
    case '.':
      if (iLen > 1 && (wsCondition[1] == '[' || wsCondition[1] == '('))
        DoPredicateFilter(wsCondition, iFoundCount, pRnd);
      return;
    case '(':
    case '"':
    default:
      return;
  }
}

void CFXJSE_ResolveProcessor::SetStylesForChild(uint32_t dwParentStyles,
                                                CFXJSE_ResolveNodeData& rnd) {
  uint32_t dwSubStyles = XFA_RESOLVENODE_Children;
  if (dwParentStyles & XFA_RESOLVENODE_TagName)
    dwSubStyles |= XFA_RESOLVENODE_TagName;

  dwSubStyles &= ~XFA_RESOLVENODE_Parent;
  dwSubStyles &= ~XFA_RESOLVENODE_Siblings;
  dwSubStyles &= ~XFA_RESOLVENODE_Properties;
  dwSubStyles |= XFA_RESOLVENODE_ALL;
  rnd.m_dwStyles = dwSubStyles;
}

void CFXJSE_ResolveProcessor::SetIndexDataBind(WideString& wsNextCondition,
                                               int32_t& iIndex,
                                               int32_t iCount) {
  if (m_pNodeHelper->CreateNodeForCondition(wsNextCondition)) {
    if (m_pNodeHelper->m_eLastCreateType == XFA_Element::DataGroup) {
      iIndex = 0;
    } else {
      iIndex = iCount - 1;
    }
  } else {
    iIndex = iCount - 1;
  }
}

CFXJSE_ResolveNodeData::CFXJSE_ResolveNodeData(CFXJSE_Engine* pSC)
    : m_pSC(pSC) {}

CFXJSE_ResolveNodeData::~CFXJSE_ResolveNodeData() = default;
