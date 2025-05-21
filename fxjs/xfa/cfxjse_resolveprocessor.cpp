// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cfxjse_resolveprocessor.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/containers/contains.h"
#include "core/fxcrt/fx_extension.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "fxjs/xfa/cfxjse_nodehelper.h"
#include "fxjs/xfa/cfxjse_value.h"
#include "fxjs/xfa/cjx_object.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_object.h"
#include "xfa/fxfa/parser/cxfa_occur.h"
#include "xfa/fxfa/parser/xfa_utils.h"

CFXJSE_ResolveProcessor::CFXJSE_ResolveProcessor(CFXJSE_Engine* pEngine,
                                                 CFXJSE_NodeHelper* pHelper)
    : engine_(pEngine), node_helper_(pHelper) {}

CFXJSE_ResolveProcessor::~CFXJSE_ResolveProcessor() = default;

bool CFXJSE_ResolveProcessor::Resolve(v8::Isolate* pIsolate, NodeData& rnd) {
  if (!rnd.cur_object_) {
    return false;
  }

  if (!rnd.cur_object_->IsNode()) {
    if (rnd.styles_ & XFA_ResolveFlag::kAttributes) {
      return ResolveForAttributeRs(rnd.cur_object_, &rnd.result_,
                                   rnd.name_.AsStringView());
    }
    return false;
  }
  if (rnd.styles_ & XFA_ResolveFlag::kAnyChild) {
    return ResolveAnyChild(pIsolate, rnd);
  }

  if (rnd.name_.GetLength()) {
    wchar_t wch = rnd.name_[0];
    switch (wch) {
      case '$':
        return ResolveDollar(pIsolate, rnd);
      case '!':
        return ResolveExcalmatory(pIsolate, rnd);
      case '#':
        return ResolveNumberSign(pIsolate, rnd);
      case '*':
        return ResolveAsterisk(rnd);
      // TODO(dsinclair): We could probably remove this.
      case '.':
        return ResolveAnyChild(pIsolate, rnd);
      default:
        break;
    }
  }
  if (rnd.hash_name_ == XFA_HASHCODE_This && rnd.level_ == 0) {
    rnd.result_.objects.emplace_back(engine_->GetThisObject());
    return true;
  }
  if (rnd.cur_object_->GetElementType() == XFA_Element::Xfa) {
    CXFA_Object* pObjNode =
        engine_->GetDocument()->GetXFAObject(rnd.hash_name_);
    if (pObjNode) {
      rnd.result_.objects.emplace_back(pObjNode);
    } else if (rnd.hash_name_ == XFA_HASHCODE_Xfa) {
      rnd.result_.objects.emplace_back(rnd.cur_object_);
    } else if ((rnd.styles_ & XFA_ResolveFlag::kAttributes) &&
               ResolveForAttributeRs(rnd.cur_object_, &rnd.result_,
                                     rnd.name_.AsStringView())) {
      return true;
    }
    if (!rnd.result_.objects.empty()) {
      FilterCondition(pIsolate, rnd.condition_, &rnd);
    }

    return !rnd.result_.objects.empty();
  }
  if (!ResolveNormal(pIsolate, rnd) && rnd.hash_name_ == XFA_HASHCODE_Xfa) {
    rnd.result_.objects.emplace_back(engine_->GetDocument()->GetRoot());
  }

  return !rnd.result_.objects.empty();
}

bool CFXJSE_ResolveProcessor::ResolveAnyChild(v8::Isolate* pIsolate,
                                              NodeData& rnd) {
  CXFA_Node* pParent = ToNode(rnd.cur_object_);
  if (!pParent) {
    return false;
  }

  WideStringView wsName = rnd.name_.AsStringView();
  WideString wsCondition = rnd.condition_;
  const bool bClassName = !wsName.IsEmpty() && wsName[0] == '#';
  CXFA_Node* const pChild =
      bClassName
          ? pParent->GetOneChildOfClass(wsName.Last(wsName.GetLength() - 1))
          : pParent->GetOneChildNamed(wsName);
  if (!pChild) {
    return false;
  }

  if (wsCondition.IsEmpty()) {
    rnd.result_.objects.emplace_back(pChild);
    return true;
  }

  std::vector<CXFA_Node*> nodes;
  for (const auto& pObject : rnd.result_.objects) {
    nodes.push_back(pObject->AsNode());
  }

  std::vector<CXFA_Node*> siblings = pChild->GetSiblings(bClassName);
  nodes.insert(nodes.end(), siblings.begin(), siblings.end());
  rnd.result_.objects =
      std::vector<cppgc::Member<CXFA_Object>>(nodes.begin(), nodes.end());
  FilterCondition(pIsolate, wsCondition, &rnd);
  return !rnd.result_.objects.empty();
}

bool CFXJSE_ResolveProcessor::ResolveDollar(v8::Isolate* pIsolate,
                                            NodeData& rnd) {
  WideString wsName = rnd.name_;
  WideString wsCondition = rnd.condition_;
  size_t nNameLen = wsName.GetLength();
  if (nNameLen == 1) {
    rnd.result_.objects.emplace_back(rnd.cur_object_);
    return true;
  }
  if (rnd.level_ > 0) {
    return false;
  }

  CXFA_Document* document = engine_->GetDocument();
  XFA_HashCode dwNameHash = static_cast<XFA_HashCode>(
      FX_HashCode_GetW(wsName.AsStringView().Last(nNameLen - 1)));
  if (dwNameHash == XFA_HASHCODE_Xfa) {
    rnd.result_.objects.emplace_back(document->GetRoot());
  } else {
    CXFA_Object* pObjNode = document->GetXFAObject(dwNameHash);
    if (pObjNode) {
      rnd.result_.objects.emplace_back(pObjNode);
    }
  }
  if (!rnd.result_.objects.empty()) {
    FilterCondition(pIsolate, wsCondition, &rnd);
  }
  return !rnd.result_.objects.empty();
}

bool CFXJSE_ResolveProcessor::ResolveExcalmatory(v8::Isolate* pIsolate,
                                                 NodeData& rnd) {
  if (rnd.level_ > 0) {
    return false;
  }

  CXFA_Node* datasets =
      ToNode(engine_->GetDocument()->GetXFAObject(XFA_HASHCODE_Datasets));
  if (!datasets) {
    return false;
  }

  NodeData rndFind;
  rndFind.cur_object_ = datasets;
  rndFind.name_ = rnd.name_.Last(rnd.name_.GetLength() - 1);
  rndFind.hash_name_ =
      static_cast<XFA_HashCode>(FX_HashCode_GetW(rndFind.name_.AsStringView()));
  rndFind.level_ = rnd.level_ + 1;
  rndFind.styles_ = XFA_ResolveFlag::kChildren;
  rndFind.condition_ = rnd.condition_;
  Resolve(pIsolate, rndFind);

  rnd.result_.objects.insert(rnd.result_.objects.end(),
                             rndFind.result_.objects.begin(),
                             rndFind.result_.objects.end());
  return !rnd.result_.objects.empty();
}

bool CFXJSE_ResolveProcessor::ResolveNumberSign(v8::Isolate* pIsolate,
                                                NodeData& rnd) {
  WideString wsName = rnd.name_.Last(rnd.name_.GetLength() - 1);
  WideString wsCondition = rnd.condition_;
  CXFA_Node* curNode = ToNode(rnd.cur_object_);
  if (ResolveForAttributeRs(curNode, &rnd.result_, wsName.AsStringView())) {
    return true;
  }

  NodeData rndFind;
  rndFind.level_ = rnd.level_ + 1;
  rndFind.styles_ = rnd.styles_;
  rndFind.styles_ |= XFA_ResolveFlag::kTagName;
  rndFind.styles_.Clear(XFA_ResolveFlag::kAttributes);
  rndFind.name_ = std::move(wsName);
  rndFind.hash_name_ =
      static_cast<XFA_HashCode>(FX_HashCode_GetW(rndFind.name_.AsStringView()));
  rndFind.condition_ = wsCondition;
  rndFind.cur_object_ = curNode;
  ResolveNormal(pIsolate, rndFind);
  if (rndFind.result_.objects.empty()) {
    return false;
  }

  if (wsCondition.IsEmpty() &&
      pdfium::Contains(rndFind.result_.objects, curNode)) {
    rnd.result_.objects.emplace_back(curNode);
  } else {
    rnd.result_.objects.insert(rnd.result_.objects.end(),
                               rndFind.result_.objects.begin(),
                               rndFind.result_.objects.end());
  }
  return !rnd.result_.objects.empty();
}

bool CFXJSE_ResolveProcessor::ResolveForAttributeRs(
    CXFA_Object* curNode,
    CFXJSE_Engine::ResolveResult* rnd,
    WideStringView strAttr) {
  std::optional<XFA_SCRIPTATTRIBUTEINFO> info =
      XFA_GetScriptAttributeByName(curNode->GetElementType(), strAttr);
  if (!info.has_value()) {
    return false;
  }

  rnd->type = CFXJSE_Engine::ResolveResult::Type::kAttribute;
  rnd->script_attribute = info.value();
  rnd->objects.emplace_back(curNode);
  return true;
}

bool CFXJSE_ResolveProcessor::ResolveNormal(v8::Isolate* pIsolate,
                                            NodeData& rnd) {
  if (rnd.level_ > 32 || !rnd.cur_object_->IsNode()) {
    return false;
  }

  CXFA_Node* curNode = rnd.cur_object_->AsNode();
  size_t nNum = rnd.result_.objects.size();
  Mask<XFA_ResolveFlag> dwStyles = rnd.styles_;
  WideString& wsName = rnd.name_;
  XFA_HashCode uNameHash = rnd.hash_name_;
  WideString& wsCondition = rnd.condition_;

  NodeData rndFind;
  rndFind.name_ = rnd.name_;
  rndFind.condition_ = rnd.condition_;
  rndFind.level_ = rnd.level_ + 1;
  rndFind.hash_name_ = uNameHash;

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
    if (curNode->HasProperty(pChild->GetElementType())) {
      properties.push_back(pChild);
    } else {
      children.push_back(pChild);
    }
  }
  if ((dwStyles & XFA_ResolveFlag::kProperties) && pVariablesNode) {
    if (pVariablesNode->GetClassHashCode() == uNameHash) {
      rnd.result_.objects.emplace_back(pVariablesNode);
    } else {
      rndFind.cur_object_ = pVariablesNode;
      SetStylesForChild(dwStyles, rndFind);
      WideString wsSaveCondition = std::move(rndFind.condition_);
      ResolveNormal(pIsolate, rndFind);
      rndFind.condition_ = std::move(wsSaveCondition);
      rnd.result_.objects.insert(rnd.result_.objects.end(),
                                 rndFind.result_.objects.begin(),
                                 rndFind.result_.objects.end());
      rndFind.result_.objects.clear();
    }
    if (rnd.result_.objects.size() > nNum) {
      FilterCondition(pIsolate, wsCondition, &rnd);
      return !rnd.result_.objects.empty();
    }
  }

  if (dwStyles & XFA_ResolveFlag::kChildren) {
    bool bSetFlag = false;
    if (pPageSetNode && (dwStyles & XFA_ResolveFlag::kProperties)) {
      children.push_back(pPageSetNode);
    }

    for (CXFA_Node* child : children) {
      if (dwStyles & XFA_ResolveFlag::kTagName) {
        if (child->GetClassHashCode() == uNameHash) {
          rnd.result_.objects.emplace_back(child);
        }
      } else if (child->GetNameHash() == uNameHash) {
        rnd.result_.objects.emplace_back(child);
      }

      if (child->GetElementType() != XFA_Element::PageSet &&
          child->IsTransparent()) {
        if (!bSetFlag) {
          SetStylesForChild(dwStyles, rndFind);
          bSetFlag = true;
        }
        rndFind.cur_object_ = child;

        WideString wsSaveCondition = std::move(rndFind.condition_);
        ResolveNormal(pIsolate, rndFind);
        rndFind.condition_ = std::move(wsSaveCondition);
        rnd.result_.objects.insert(rnd.result_.objects.end(),
                                   rndFind.result_.objects.begin(),
                                   rndFind.result_.objects.end());
        rndFind.result_.objects.clear();
      }
    }
    if (rnd.result_.objects.size() > nNum) {
      if (!(dwStyles & XFA_ResolveFlag::kALL)) {
        std::vector<CXFA_Node*> upArrayNodes;
        if (curNode->IsTransparent()) {
          CXFA_Node* pCurrent = ToNode(rnd.result_.objects.front().Get());
          if (pCurrent) {
            upArrayNodes =
                pCurrent->GetSiblings(!!(dwStyles & XFA_ResolveFlag::kTagName));
          }
        }
        if (upArrayNodes.size() > rnd.result_.objects.size()) {
          CXFA_Object* pSaveObject = rnd.result_.objects.front().Get();
          rnd.result_.objects = std::vector<cppgc::Member<CXFA_Object>>(
              upArrayNodes.begin(), upArrayNodes.end());
          rnd.result_.objects.front() = pSaveObject;
        }
      }
      FilterCondition(pIsolate, wsCondition, &rnd);
      return !rnd.result_.objects.empty();
    }
  }
  if (dwStyles & XFA_ResolveFlag::kAttributes) {
    if (ResolveForAttributeRs(curNode, &rnd.result_, wsName.AsStringView())) {
      return true;
    }
  }
  if (dwStyles & XFA_ResolveFlag::kProperties) {
    for (CXFA_Node* pChildProperty : properties) {
      if (pChildProperty->IsUnnamed()) {
        if (pChildProperty->GetClassHashCode() == uNameHash) {
          rnd.result_.objects.emplace_back(pChildProperty);
        }
        continue;
      }
      if (pChildProperty->GetNameHash() == uNameHash &&
          pChildProperty->GetElementType() != XFA_Element::Extras &&
          pChildProperty->GetElementType() != XFA_Element::Items) {
        rnd.result_.objects.emplace_back(pChildProperty);
      }
    }
    if (rnd.result_.objects.size() > nNum) {
      FilterCondition(pIsolate, wsCondition, &rnd);
      return !rnd.result_.objects.empty();
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
      rnd.result_.objects.emplace_back(pProp);
      return !rnd.result_.objects.empty();
    }
  }

  CXFA_Node* const parentNode = curNode->GetParent();
  uint32_t uCurClassHash = curNode->GetClassHashCode();
  if (!parentNode) {
    if (uCurClassHash == uNameHash) {
      rnd.result_.objects.emplace_back(curNode);
      FilterCondition(pIsolate, wsCondition, &rnd);
      if (!rnd.result_.objects.empty()) {
        return true;
      }
    }
    return false;
  }

  if (dwStyles & XFA_ResolveFlag::kSiblings) {
    CXFA_Node* child = parentNode->GetFirstChild();
    Mask<XFA_ResolveFlag> dwSubStyles = {XFA_ResolveFlag::kChildren,
                                         XFA_ResolveFlag::kProperties};
    if (dwStyles & XFA_ResolveFlag::kTagName) {
      dwSubStyles |= XFA_ResolveFlag::kTagName;
    }
    if (dwStyles & XFA_ResolveFlag::kALL) {
      dwSubStyles |= XFA_ResolveFlag::kALL;
    }

    rndFind.styles_ = dwSubStyles;
    while (child) {
      if (child == curNode) {
        if (dwStyles & XFA_ResolveFlag::kTagName) {
          if (uCurClassHash == uNameHash) {
            rnd.result_.objects.emplace_back(curNode);
          }
        } else {
          if (child->GetNameHash() == uNameHash) {
            rnd.result_.objects.emplace_back(curNode);
            if (rnd.level_ == 0 && wsCondition.IsEmpty()) {
              rnd.result_.objects.clear();
              rnd.result_.objects.emplace_back(curNode);
              return true;
            }
          }
        }
        child = child->GetNextSibling();
        continue;
      }

      if (dwStyles & XFA_ResolveFlag::kTagName) {
        if (child->GetClassHashCode() == uNameHash) {
          rnd.result_.objects.emplace_back(child);
        }
      } else if (child->GetNameHash() == uNameHash) {
        rnd.result_.objects.emplace_back(child);
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
        rndFind.cur_object_ = child;
        WideString wsOriginCondition = std::move(rndFind.condition_);
        Mask<XFA_ResolveFlag> dwOriginStyle = rndFind.styles_;
        rndFind.styles_ = dwOriginStyle | XFA_ResolveFlag::kALL;
        ResolveNormal(pIsolate, rndFind);
        rndFind.styles_ = dwOriginStyle;
        rndFind.condition_ = std::move(wsOriginCondition);
        rnd.result_.objects.insert(rnd.result_.objects.end(),
                                   rndFind.result_.objects.begin(),
                                   rndFind.result_.objects.end());
        rndFind.result_.objects.clear();
      }
      child = child->GetNextSibling();
    }
    if (rnd.result_.objects.size() > nNum) {
      if (parentNode->IsTransparent()) {
        std::vector<CXFA_Node*> upArrayNodes;
        CXFA_Node* pCurrent = ToNode(rnd.result_.objects.front().Get());
        if (pCurrent) {
          upArrayNodes =
              pCurrent->GetSiblings(!!(dwStyles & XFA_ResolveFlag::kTagName));
        }
        if (upArrayNodes.size() > rnd.result_.objects.size()) {
          CXFA_Object* pSaveObject = rnd.result_.objects.front().Get();
          rnd.result_.objects = std::vector<cppgc::Member<CXFA_Object>>(
              upArrayNodes.begin(), upArrayNodes.end());
          rnd.result_.objects.front() = pSaveObject;
        }
      }
      FilterCondition(pIsolate, wsCondition, &rnd);
      return !rnd.result_.objects.empty();
    }
  }

  if (dwStyles & XFA_ResolveFlag::kParent) {
    Mask<XFA_ResolveFlag> dwSubStyles = {XFA_ResolveFlag::kSiblings,
                                         XFA_ResolveFlag::kParent,
                                         XFA_ResolveFlag::kProperties};
    if (dwStyles & XFA_ResolveFlag::kTagName) {
      dwSubStyles |= XFA_ResolveFlag::kTagName;
    }
    if (dwStyles & XFA_ResolveFlag::kALL) {
      dwSubStyles |= XFA_ResolveFlag::kALL;
    }

    engine_->AddObjectToUpArray(parentNode);
    rndFind.styles_ = dwSubStyles;
    rndFind.cur_object_ = parentNode;
    ResolveNormal(pIsolate, rndFind);
    rnd.result_.objects.insert(rnd.result_.objects.end(),
                               rndFind.result_.objects.begin(),
                               rndFind.result_.objects.end());
    rndFind.result_.objects.clear();
    if (rnd.result_.objects.size() > nNum) {
      return true;
    }
  }
  return false;
}

bool CFXJSE_ResolveProcessor::ResolveAsterisk(NodeData& rnd) {
  CXFA_Node* curNode = ToNode(rnd.cur_object_);
  std::vector<CXFA_Node*> array = curNode->GetNodeListWithFilter(
      {XFA_NodeFilter::kChildren, XFA_NodeFilter::kProperties});
  rnd.result_.objects.insert(rnd.result_.objects.end(), array.begin(),
                             array.end());
  return !rnd.result_.objects.empty();
}

int32_t CFXJSE_ResolveProcessor::GetFilter(WideStringView wsExpression,
                                           int32_t nStart,
                                           NodeData& rnd) {
  DCHECK(nStart > -1);

  int32_t iLength = wsExpression.GetLength();
  if (nStart >= iLength) {
    return 0;
  }

  WideString& wsName = rnd.name_;
  WideString& wsCondition = rnd.condition_;
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
          rnd.styles_ |= XFA_ResolveFlag::kAnyChild;
          continue;
        }
        if (wPrev == '\\') {
          pNameBuf[nNameCount - 1] = wPrev = '.';
          continue;
        }

        wchar_t wLookahead = nStart < iLength ? pSrc[nStart] : 0;
        if (wLookahead != '[' && wLookahead != '(' && nType < 0) {
          break;
        }
      }
      if (wCur == '[' || wCur == '(') {
        bIsCondition = true;
      } else if (wCur == '.' && nStart < iLength &&
                 (pSrc[nStart] == '[' || pSrc[nStart] == '(')) {
        bIsCondition = true;
      }
      if (bIsCondition) {
        pConditionBuf[nConditionCount++] = wCur;
      } else {
        pNameBuf[nNameCount++] = wCur;
      }

      if ((nType == 0 && wCur == ']') || (nType == 1 && wCur == ')') ||
          (nType == 2 && wCur == '"')) {
        nType = stack.empty() ? -1 : stack.back();
        if (!stack.empty()) {
          stack.pop_back();
        }
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
    if (!stack.empty()) {
      return -1;
    }
  }
  wsName.ReleaseBuffer(nNameCount);
  wsCondition.ReleaseBuffer(nConditionCount);
  wsName.TrimWhitespace();
  wsCondition.TrimWhitespace();
  rnd.hash_name_ =
      static_cast<XFA_HashCode>(FX_HashCode_GetW(wsName.AsStringView()));
  return nStart;
}

void CFXJSE_ResolveProcessor::ConditionArray(size_t iCurIndex,
                                             WideString wsCondition,
                                             size_t iFoundCount,
                                             NodeData* pRnd) {
  size_t iLen = wsCondition.GetLength();
  bool bRelative = false;
  bool bAll = false;
  size_t i = 1;
  for (; i < iLen; ++i) {
    wchar_t ch = wsCondition[i];
    if (ch == ' ') {
      continue;
    }
    if (ch == '+' || ch == '-') {
      bRelative = true;
    } else if (ch == '*') {
      bAll = true;
    }

    break;
  }
  if (bAll) {
    if (pRnd->styles_ & XFA_ResolveFlag::kCreateNode) {
      if (pRnd->styles_ & XFA_ResolveFlag::kBind) {
        node_helper_->create_parent_ = ToNode(pRnd->cur_object_);
        node_helper_->create_count_ = 1;
        pRnd->result_.objects.clear();
        node_helper_->cur_all_start_ = -1;
        node_helper_->all_start_parent_ = nullptr;
      } else if (node_helper_->cur_all_start_ == -1) {
        node_helper_->cur_all_start_ = cur_start_;
        node_helper_->all_start_parent_ = ToNode(pRnd->cur_object_);
      }
    } else if (pRnd->styles_ & XFA_ResolveFlag::kBindNew) {
      if (node_helper_->cur_all_start_ == -1) {
        node_helper_->cur_all_start_ = cur_start_;
      }
    }
    return;
  }
  if (iFoundCount == 1 && !iLen) {
    return;
  }

  int32_t iIndex = wsCondition.Substr(i, iLen - 1 - i).GetInteger();
  if (bRelative) {
    iIndex += iCurIndex;
  }

  if (iIndex < 0 || static_cast<size_t>(iIndex) >= iFoundCount) {
    if (pRnd->styles_ & XFA_ResolveFlag::kCreateNode) {
      node_helper_->create_parent_ = ToNode(pRnd->cur_object_);
      node_helper_->create_count_ = iIndex - iFoundCount + 1;
    }
    pRnd->result_.objects.clear();
  } else {
    pRnd->result_.objects = std::vector<cppgc::Member<CXFA_Object>>(
        1, pRnd->result_.objects[iIndex]);
  }
}

void CFXJSE_ResolveProcessor::FilterCondition(v8::Isolate* pIsolate,
                                              WideString wsCondition,
                                              NodeData* pRnd) {
  size_t iCurIndex = 0;
  CXFA_Node* pNode = engine_->LastObjectFromUpArray();
  if (pNode) {
    const bool bIsProperty = pNode->IsProperty();
    const bool bIsClassIndex =
        pNode->IsUnnamed() ||
        (bIsProperty && pNode->GetElementType() != XFA_Element::PageSet);
    iCurIndex = pNode->GetIndex(bIsProperty, bIsClassIndex);
  }

  size_t iFoundCount = pRnd->result_.objects.size();
  wsCondition.TrimWhitespace();

  const size_t nLen = wsCondition.GetLength();
  if (nLen == 0) {
    if (pRnd->styles_ & XFA_ResolveFlag::kALL) {
      return;
    }
    if (iFoundCount == 1) {
      return;
    }

    if (iFoundCount <= iCurIndex) {
      if (pRnd->styles_ & XFA_ResolveFlag::kCreateNode) {
        node_helper_->create_parent_ = ToNode(pRnd->cur_object_);
        node_helper_->create_count_ = iCurIndex - iFoundCount + 1;
      }
      pRnd->result_.objects.clear();
      return;
    }

    pRnd->result_.objects = std::vector<cppgc::Member<CXFA_Object>>(
        1, pRnd->result_.objects[iCurIndex]);
    return;
  }

  wchar_t wTypeChar = wsCondition[0];
  switch (wTypeChar) {
    case '[':
      ConditionArray(iCurIndex, wsCondition, iFoundCount, pRnd);
      return;
    case '.':
      if (nLen > 1 && (wsCondition[1] == '[' || wsCondition[1] == '(')) {
        DoPredicateFilter(pIsolate, wsCondition, iFoundCount, pRnd);
      }
      return;
    case '(':
    case '"':
    default:
      return;
  }
}

void CFXJSE_ResolveProcessor::SetStylesForChild(
    Mask<XFA_ResolveFlag> dwParentStyles,
    NodeData& rnd) {
  Mask<XFA_ResolveFlag> dwSubStyles = {XFA_ResolveFlag::kChildren,
                                       XFA_ResolveFlag::kALL};
  if (dwParentStyles & XFA_ResolveFlag::kTagName) {
    dwSubStyles |= XFA_ResolveFlag::kTagName;
  }
  rnd.styles_ = dwSubStyles;
}

int32_t CFXJSE_ResolveProcessor::IndexForDataBind(
    const WideString& wsNextCondition,
    int32_t iCount) {
  if (node_helper_->CreateNodeForCondition(wsNextCondition) &&
      node_helper_->last_create_type_ == XFA_Element::DataGroup) {
    return 0;
  }
  return iCount - 1;
}

void CFXJSE_ResolveProcessor::DoPredicateFilter(v8::Isolate* pIsolate,
                                                WideString wsCondition,
                                                size_t iFoundCount,
                                                NodeData* pRnd) {
  DCHECK_EQ(iFoundCount, pRnd->result_.objects.size());
  CXFA_Script::Type eLangType = CXFA_Script::Type::Unknown;
  if (wsCondition.First(2).EqualsASCII(".[") && wsCondition.Back() == L']') {
    eLangType = CXFA_Script::Type::Formcalc;
  } else if (wsCondition.First(2).EqualsASCII(".(") &&
             wsCondition.Back() == L')') {
    eLangType = CXFA_Script::Type::Javascript;
  } else {
    return;
  }

  WideString wsExpression = wsCondition.Substr(2, wsCondition.GetLength() - 3);
  for (size_t i = iFoundCount; i > 0; --i) {
    CFXJSE_Context::ExecutionResult exec_result =
        engine_->RunScript(eLangType, wsExpression.AsStringView(),
                           pRnd->result_.objects[i - 1].Get());
    if (!exec_result.status || !exec_result.value->ToBoolean(pIsolate)) {
      pRnd->result_.objects.erase(pRnd->result_.objects.begin() + i - 1);
    }
  }
}

CFXJSE_ResolveProcessor::NodeData::NodeData() = default;

CFXJSE_ResolveProcessor::NodeData::~NodeData() = default;
