// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_document.h"

#include <set>
#include <utility>

#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/notreached.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxcrt/xml/cfx_xmldocument.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "fxjs/gc/container_trace.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "fxjs/xfa/cfxjse_resolveprocessor.h"
#include "fxjs/xfa/cjx_object.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/parser/cscript_datawindow.h"
#include "xfa/fxfa/parser/cscript_eventpseudomodel.h"
#include "xfa/fxfa/parser/cscript_hostpseudomodel.h"
#include "xfa/fxfa/parser/cscript_layoutpseudomodel.h"
#include "xfa/fxfa/parser/cscript_logpseudomodel.h"
#include "xfa/fxfa/parser/cscript_signaturepseudomodel.h"
#include "xfa/fxfa/parser/cxfa_bind.h"
#include "xfa/fxfa/parser/cxfa_datagroup.h"
#include "xfa/fxfa/parser/cxfa_exdata.h"
#include "xfa/fxfa/parser/cxfa_form.h"
#include "xfa/fxfa/parser/cxfa_image.h"
#include "xfa/fxfa/parser/cxfa_interactive.h"
#include "xfa/fxfa/parser/cxfa_items.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_occur.h"
#include "xfa/fxfa/parser/cxfa_pageset.h"
#include "xfa/fxfa/parser/cxfa_pdf.h"
#include "xfa/fxfa/parser/cxfa_present.h"
#include "xfa/fxfa/parser/cxfa_subform.h"
#include "xfa/fxfa/parser/cxfa_template.h"
#include "xfa/fxfa/parser/cxfa_traversestrategy_xfacontainernode.h"
#include "xfa/fxfa/parser/cxfa_traversestrategy_xfanode.h"
#include "xfa/fxfa/parser/cxfa_value.h"
#include "xfa/fxfa/parser/xfa_document_datamerger_imp.h"
#include "xfa/fxfa/parser/xfa_utils.h"

namespace {

const wchar_t kTemplateNS[] = L"http://www.xfa.org/schema/xfa-template/";

struct RecurseRecord {
  cppgc::Persistent<CXFA_Node> pTemplateChild;
  cppgc::Persistent<CXFA_Node> pDataChild;
};

class CXFA_TraverseStrategy_DDGroup {
 public:
  static CXFA_Node* GetFirstChild(CXFA_Node* pDDGroupNode) {
    return pDDGroupNode->GetFirstChildByName(XFA_HASHCODE_Group);
  }
  static CXFA_Node* GetNextSibling(CXFA_Node* pDDGroupNode) {
    return pDDGroupNode->GetNextSameNameSibling(XFA_HASHCODE_Group);
  }
  static CXFA_Node* GetParent(CXFA_Node* pDDGroupNode) {
    return pDDGroupNode->GetParent();
  }
};

void FormValueNode_MatchNoneCreateChild(CXFA_Node* pFormNode) {
  DCHECK(pFormNode->IsWidgetReady());
  // GetUIChildNode has the side effect of creating the UI child.
  pFormNode->GetUIChildNode();
}

CXFA_Node* FormValueNode_CreateChild(CXFA_Node* pValueNode, XFA_Element iType) {
  CXFA_Node* pChildNode = pValueNode->GetFirstChild();
  if (!pChildNode) {
    if (iType == XFA_Element::Unknown) {
      return nullptr;
    }

    pChildNode =
        pValueNode->JSObject()->GetOrCreateProperty<CXFA_Node>(0, iType);
  }
  return pChildNode;
}

void FormValueNode_SetChildContent(CXFA_Node* pValueNode,
                                   const WideString& wsContent,
                                   XFA_Element iType) {
  if (!pValueNode) {
    return;
  }

  DCHECK_EQ(pValueNode->GetPacketType(), XFA_PacketType::Form);
  CXFA_Node* pChildNode = FormValueNode_CreateChild(pValueNode, iType);
  if (!pChildNode) {
    return;
  }

  switch (pChildNode->GetObjectType()) {
    case XFA_ObjectType::ContentNode: {
      CXFA_Node* pContentRawDataNode = pChildNode->GetFirstChild();
      if (!pContentRawDataNode) {
        XFA_Element element = XFA_Element::Sharptext;
        if (pChildNode->GetElementType() == XFA_Element::ExData) {
          std::optional<WideString> contentType =
              pChildNode->JSObject()->TryAttribute(XFA_Attribute::ContentType,
                                                   false);
          if (contentType.has_value()) {
            if (contentType.value().EqualsASCII("text/html")) {
              element = XFA_Element::SharpxHTML;
            } else if (contentType.value().EqualsASCII("text/xml")) {
              element = XFA_Element::Sharpxml;
            }
          }
        }
        pContentRawDataNode = pChildNode->CreateSamePacketNode(element);
        pChildNode->InsertChildAndNotify(pContentRawDataNode, nullptr);
      }
      pContentRawDataNode->JSObject()->SetCData(XFA_Attribute::Value,
                                                wsContent);
      break;
    }
    case XFA_ObjectType::NodeC:
    case XFA_ObjectType::TextNode:
    case XFA_ObjectType::NodeV: {
      pChildNode->JSObject()->SetCData(XFA_Attribute::Value, wsContent);
      break;
    }
    default:
      break;
  }
}

void MergeNodeRecurse(CXFA_Node* pDestNodeParent, CXFA_Node* pProtoNode) {
  CXFA_Node* pExistingNode = nullptr;
  for (CXFA_Node* pFormChild = pDestNodeParent->GetFirstChild(); pFormChild;
       pFormChild = pFormChild->GetNextSibling()) {
    if (pFormChild->GetElementType() == pProtoNode->GetElementType() &&
        pFormChild->GetNameHash() == pProtoNode->GetNameHash() &&
        pFormChild->IsUnusedNode()) {
      pFormChild->ClearFlag(XFA_NodeFlag::kUnusedNode);
      pExistingNode = pFormChild;
      break;
    }
  }

  if (pExistingNode) {
    pExistingNode->SetTemplateNode(pProtoNode);
    for (CXFA_Node* pTemplateChild = pProtoNode->GetFirstChild();
         pTemplateChild; pTemplateChild = pTemplateChild->GetNextSibling()) {
      MergeNodeRecurse(pExistingNode, pTemplateChild);
    }
    return;
  }
  CXFA_Node* pNewNode = pProtoNode->Clone(true);
  pNewNode->SetTemplateNode(pProtoNode);
  pDestNodeParent->InsertChildAndNotify(pNewNode, nullptr);
}

void MergeNode(CXFA_Node* pDestNode, CXFA_Node* pProtoNode) {
  {
    CXFA_NodeIterator sIterator(pDestNode);
    for (CXFA_Node* pNode = sIterator.GetCurrent(); pNode;
         pNode = sIterator.MoveToNext()) {
      pNode->SetFlag(XFA_NodeFlag::kUnusedNode);
    }
  }
  pDestNode->SetTemplateNode(pProtoNode);
  for (CXFA_Node* pTemplateChild = pProtoNode->GetFirstChild(); pTemplateChild;
       pTemplateChild = pTemplateChild->GetNextSibling()) {
    MergeNodeRecurse(pDestNode, pTemplateChild);
  }
  {
    CXFA_NodeIterator sIterator(pDestNode);
    for (CXFA_Node* pNode = sIterator.GetCurrent(); pNode;
         pNode = sIterator.MoveToNext()) {
      pNode->ClearFlag(XFA_NodeFlag::kUnusedNode);
    }
  }
}

CXFA_Node* CloneOrMergeInstanceManager(CXFA_Document* document,
                                       CXFA_Node* pFormParent,
                                       CXFA_Node* pTemplateNode,
                                       std::vector<CXFA_Node*>* subforms) {
  WideString wsSubformName =
      pTemplateNode->JSObject()->GetCData(XFA_Attribute::Name);
  WideString wsInstMgrNodeName = L"_" + wsSubformName;
  uint32_t dwInstNameHash = FX_HashCode_GetW(wsInstMgrNodeName.AsStringView());
  CXFA_Node* pExistingNode = XFA_DataMerge_FindFormDOMInstance(
      document, XFA_Element::InstanceManager, dwInstNameHash, pFormParent);
  if (pExistingNode) {
    uint32_t dwNameHash = pTemplateNode->GetNameHash();
    for (CXFA_Node* pNode = pExistingNode->GetNextSibling(); pNode;) {
      XFA_Element eCurType = pNode->GetElementType();
      if (eCurType == XFA_Element::InstanceManager) {
        break;
      }

      if ((eCurType != XFA_Element::Subform) &&
          (eCurType != XFA_Element::SubformSet)) {
        pNode = pNode->GetNextSibling();
        continue;
      }
      if (dwNameHash != pNode->GetNameHash()) {
        break;
      }

      CXFA_Node* pNextNode = pNode->GetNextSibling();
      pFormParent->RemoveChildAndNotify(pNode, true);
      subforms->push_back(pNode);
      pNode = pNextNode;
    }
    pFormParent->RemoveChildAndNotify(pExistingNode, true);
    pFormParent->InsertChildAndNotify(pExistingNode, nullptr);
    pExistingNode->ClearFlag(XFA_NodeFlag::kUnusedNode);
    pExistingNode->SetTemplateNode(pTemplateNode);
    return pExistingNode;
  }

  CXFA_Node* pNewNode =
      document->CreateNode(XFA_PacketType::Form, XFA_Element::InstanceManager);
  wsInstMgrNodeName =
      L"_" + pTemplateNode->JSObject()->GetCData(XFA_Attribute::Name);
  pNewNode->JSObject()->SetCData(XFA_Attribute::Name, wsInstMgrNodeName);
  pFormParent->InsertChildAndNotify(pNewNode, nullptr);
  pNewNode->SetTemplateNode(pTemplateNode);
  return pNewNode;
}

void SortRecurseRecord(std::vector<RecurseRecord>* rgRecords,
                       CXFA_Node* pDataScope,
                       bool bChoiceMode) {
  std::vector<RecurseRecord> rgResultRecord;
  for (CXFA_Node* pNode = pDataScope->GetFirstChild(); pNode;
       pNode = pNode->GetNextSibling()) {
    auto it =
        std::ranges::find_if(*rgRecords, [pNode](const RecurseRecord& record) {
          return pNode == record.pDataChild;
        });
    if (it != rgRecords->end()) {
      rgResultRecord.push_back(*it);
      rgRecords->erase(it);
      if (bChoiceMode) {
        break;
      }
    }
  }
  if (rgResultRecord.empty()) {
    return;
  }

  if (!bChoiceMode) {
    rgResultRecord.insert(rgResultRecord.end(), rgRecords->begin(),
                          rgRecords->end());
  }
  *rgRecords = rgResultRecord;
}

CXFA_Node* ScopeMatchGlobalBinding(CXFA_Node* pDataScope,
                                   uint32_t dwNameHash,
                                   XFA_Element eMatchDataNodeType,
                                   bool bUpLevel) {
  for (CXFA_Node *pCurDataScope = pDataScope, *pLastDataScope = nullptr;
       pCurDataScope &&
       pCurDataScope->GetPacketType() == XFA_PacketType::Datasets;
       pLastDataScope = pCurDataScope,
                 pCurDataScope = pCurDataScope->GetParent()) {
    for (CXFA_Node* pDataChild = pCurDataScope->GetFirstChildByName(dwNameHash);
         pDataChild;
         pDataChild = pDataChild->GetNextSameNameSibling(dwNameHash)) {
      if (pDataChild == pLastDataScope ||
          (eMatchDataNodeType != XFA_Element::DataModel &&
           pDataChild->GetElementType() != eMatchDataNodeType) ||
          pDataChild->HasBindItem()) {
        continue;
      }
      return pDataChild;
    }

    for (CXFA_DataGroup* pDataChild =
             pCurDataScope->GetFirstChildByClass<CXFA_DataGroup>(
                 XFA_Element::DataGroup);
         pDataChild;
         pDataChild = pDataChild->GetNextSameClassSibling<CXFA_DataGroup>(
             XFA_Element::DataGroup)) {
      CXFA_Node* pDataNode = ScopeMatchGlobalBinding(pDataChild, dwNameHash,
                                                     eMatchDataNodeType, false);
      if (pDataNode) {
        return pDataNode;
      }
    }
    if (!bUpLevel) {
      break;
    }
  }
  return nullptr;
}

CXFA_Node* FindGlobalDataNode(CXFA_Document* document,
                              const WideString& wsName,
                              CXFA_Node* pDataScope,
                              XFA_Element eMatchNodeType) {
  if (wsName.IsEmpty()) {
    return nullptr;
  }

  uint32_t dwNameHash = FX_HashCode_GetW(wsName.AsStringView());
  CXFA_Node* pBounded = document->GetGlobalBinding(dwNameHash);
  if (!pBounded) {
    pBounded =
        ScopeMatchGlobalBinding(pDataScope, dwNameHash, eMatchNodeType, true);
    if (pBounded) {
      document->RegisterGlobalBinding(dwNameHash, pBounded);
    }
  }
  return pBounded;
}

CXFA_Node* FindOnceDataNode(const WideString& wsName,
                            CXFA_Node* pDataScope,
                            XFA_Element eMatchNodeType) {
  if (wsName.IsEmpty()) {
    return nullptr;
  }

  uint32_t dwNameHash = FX_HashCode_GetW(wsName.AsStringView());
  CXFA_Node* pLastDataScope = nullptr;
  for (CXFA_Node* pCurDataScope = pDataScope;
       pCurDataScope &&
       pCurDataScope->GetPacketType() == XFA_PacketType::Datasets;
       pCurDataScope = pCurDataScope->GetParent()) {
    for (CXFA_Node* pDataChild = pCurDataScope->GetFirstChildByName(dwNameHash);
         pDataChild;
         pDataChild = pDataChild->GetNextSameNameSibling(dwNameHash)) {
      if (pDataChild == pLastDataScope || pDataChild->HasBindItem() ||
          (eMatchNodeType != XFA_Element::DataModel &&
           pDataChild->GetElementType() != eMatchNodeType)) {
        continue;
      }
      return pDataChild;
    }
    pLastDataScope = pCurDataScope;
  }
  return nullptr;
}

CXFA_Node* FindDataRefDataNode(CXFA_Document* document,
                               const WideString& wsRef,
                               CXFA_Node* pDataScope,
                               XFA_Element eMatchNodeType,
                               CXFA_Node* pTemplateNode,
                               bool bForceBind,
                               bool bUpLevel) {
  Mask<XFA_ResolveFlag> dwFlags = {XFA_ResolveFlag::kChildren,
                                   XFA_ResolveFlag::kBindNew};
  if (bUpLevel || !wsRef.EqualsASCII("name")) {
    dwFlags |= XFA_ResolveFlag::kParent;
    dwFlags |= XFA_ResolveFlag::kSiblings;
  }
  std::optional<CFXJSE_Engine::ResolveResult> maybeResult =
      document->GetScriptContext()->ResolveObjectsWithBindNode(
          pDataScope, wsRef.AsStringView(), dwFlags, pTemplateNode);
  if (!maybeResult.has_value()) {
    return nullptr;
  }

  if (maybeResult.value().type ==
          CFXJSE_Engine::ResolveResult::Type::kCreateNodeAll ||
      maybeResult.value().type ==
          CFXJSE_Engine::ResolveResult::Type::kCreateNodeMidAll ||
      maybeResult.value().objects.size() > 1) {
    return document->GetNotBindNode(maybeResult.value().objects);
  }

  if (maybeResult.value().type ==
      CFXJSE_Engine::ResolveResult::Type::kCreateNodeOne) {
    CXFA_Object* pObject = !maybeResult.value().objects.empty()
                               ? maybeResult.value().objects.front().Get()
                               : nullptr;
    CXFA_Node* pNode = ToNode(pObject);
    return (bForceBind || !pNode || !pNode->HasBindItem()) ? pNode : nullptr;
  }
  return nullptr;
}

CXFA_Node* FindMatchingDataNode(
    CXFA_Document* document,
    CXFA_Node* pTemplateNode,
    CXFA_Node* pDataScope,
    bool& bAccessedDataDOM,
    bool bForceBind,
    CXFA_NodeIteratorTemplate<CXFA_Node,
                              CXFA_TraverseStrategy_XFAContainerNode>*
        pIterator,
    bool& bSelfMatch,
    XFA_AttributeValue& eBindMatch,
    bool bUpLevel) {
  CXFA_Node* pResult = nullptr;
  CXFA_Node* pCurTemplateNode = pIterator->GetCurrent();
  while (pCurTemplateNode) {
    XFA_Element eMatchNodeType;
    switch (pCurTemplateNode->GetElementType()) {
      case XFA_Element::Subform:
        eMatchNodeType = XFA_Element::DataGroup;
        break;
      case XFA_Element::Field: {
        eMatchNodeType = XFA_FieldIsMultiListBox(pCurTemplateNode)
                             ? XFA_Element::DataGroup
                             : XFA_Element::DataValue;
      } break;
      case XFA_Element::ExclGroup:
        eMatchNodeType = XFA_Element::DataValue;
        break;
      default:
        pCurTemplateNode = pIterator->MoveToNext();
        continue;
    }

    CXFA_Occur* pTemplateNodeOccur =
        pCurTemplateNode->GetFirstChildByClass<CXFA_Occur>(XFA_Element::Occur);
    if (pTemplateNodeOccur) {
      auto [iMin, iMax, iInit] = pTemplateNodeOccur->GetOccurInfo();
      if (iMax == 0) {
        pCurTemplateNode = pIterator->MoveToNext();
        continue;
      }
    }

    CXFA_Bind* pTemplateNodeBind =
        pCurTemplateNode->GetFirstChildByClass<CXFA_Bind>(XFA_Element::Bind);
    XFA_AttributeValue eMatch =
        pTemplateNodeBind
            ? pTemplateNodeBind->JSObject()->GetEnum(XFA_Attribute::Match)
            : XFA_AttributeValue::Once;
    eBindMatch = eMatch;
    switch (eMatch) {
      case XFA_AttributeValue::None:
        pCurTemplateNode = pIterator->MoveToNext();
        continue;
      case XFA_AttributeValue::Global:
        bAccessedDataDOM = true;
        if (!bForceBind) {
          pCurTemplateNode = pIterator->MoveToNext();
          continue;
        }
        if (eMatchNodeType == XFA_Element::DataValue ||
            (eMatchNodeType == XFA_Element::DataGroup &&
             XFA_FieldIsMultiListBox(pTemplateNodeBind))) {
          CXFA_Node* pGlobalBindNode = FindGlobalDataNode(
              document,
              pCurTemplateNode->JSObject()->GetCData(XFA_Attribute::Name),
              pDataScope, eMatchNodeType);
          if (!pGlobalBindNode) {
            pCurTemplateNode = pIterator->MoveToNext();
            continue;
          }
          pResult = pGlobalBindNode;
          break;
        }
        [[fallthrough]];
      case XFA_AttributeValue::Once: {
        bAccessedDataDOM = true;
        CXFA_Node* pOnceBindNode = FindOnceDataNode(
            pCurTemplateNode->JSObject()->GetCData(XFA_Attribute::Name),
            pDataScope, eMatchNodeType);
        if (!pOnceBindNode) {
          pCurTemplateNode = pIterator->MoveToNext();
          continue;
        }
        pResult = pOnceBindNode;
        break;
      }
      case XFA_AttributeValue::DataRef: {
        bAccessedDataDOM = true;
        CXFA_Node* pDataRefBindNode = FindDataRefDataNode(
            document,
            pTemplateNodeBind->JSObject()->GetCData(XFA_Attribute::Ref),
            pDataScope, eMatchNodeType, pTemplateNode, bForceBind, bUpLevel);
        if (pDataRefBindNode &&
            pDataRefBindNode->GetElementType() == eMatchNodeType) {
          pResult = pDataRefBindNode;
        }
        if (!pResult) {
          pCurTemplateNode = pIterator->SkipChildrenAndMoveToNext();
          continue;
        }
        break;
      }
      default:
        break;
    }
    if (pCurTemplateNode == pTemplateNode && pResult) {
      bSelfMatch = true;
    }
    break;
  }
  return pResult;
}

void CreateDataBinding(CXFA_Node* pFormNode,
                       CXFA_Node* pDataNode,
                       bool bDataToForm) {
  pFormNode->SetBindingNode(pDataNode);
  pDataNode->AddBindItem(pFormNode);
  XFA_Element eType = pFormNode->GetElementType();
  if (eType != XFA_Element::Field && eType != XFA_Element::ExclGroup) {
    return;
  }

  DCHECK(pFormNode->IsWidgetReady());
  auto* defValue = pFormNode->JSObject()->GetOrCreateProperty<CXFA_Value>(
      0, XFA_Element::Value);
  if (!bDataToForm) {
    WideString wsValue;
    switch (pFormNode->GetFFWidgetType()) {
      case XFA_FFWidgetType::kImageEdit: {
        CXFA_Image* image = defValue ? defValue->GetImageIfExists() : nullptr;
        WideString wsContentType;
        WideString wsHref;
        if (image) {
          wsValue = image->GetContent();
          wsContentType = image->GetContentType();
          wsHref = image->GetHref();
        }
        CFX_XMLElement* pXMLDataElement =
            ToXMLElement(pDataNode->GetXMLMappingNode());
        DCHECK(pXMLDataElement);
        pDataNode->JSObject()->SetAttributeValue(
            wsValue, pFormNode->GetFormatDataValue(wsValue));
        pDataNode->JSObject()->SetCData(XFA_Attribute::ContentType,
                                        wsContentType);
        if (!wsHref.IsEmpty()) {
          pXMLDataElement->SetAttribute(L"href", wsHref);
        }

        break;
      }
      case XFA_FFWidgetType::kChoiceList:
        wsValue = defValue ? defValue->GetChildValueContent() : WideString();
        if (pFormNode->IsChoiceListMultiSelect()) {
          std::vector<WideString> wsSelTextArray =
              pFormNode->GetSelectedItemsValue();
          if (!wsSelTextArray.empty()) {
            for (const auto& text : wsSelTextArray) {
              CXFA_Node* pValue =
                  pDataNode->CreateSamePacketNode(XFA_Element::DataValue);
              pValue->JSObject()->SetCData(XFA_Attribute::Name, L"value");
              pValue->CreateXMLMappingNode();
              pDataNode->InsertChildAndNotify(pValue, nullptr);
              pValue->JSObject()->SetCData(XFA_Attribute::Value, text);
            }
          } else {
            CFX_XMLElement* pElement =
                ToXMLElement(pDataNode->GetXMLMappingNode());
            pElement->SetAttribute(L"xfa:dataNode", L"dataGroup");
          }
        } else if (!wsValue.IsEmpty()) {
          pDataNode->JSObject()->SetAttributeValue(
              wsValue, pFormNode->GetFormatDataValue(wsValue));
        }
        break;
      case XFA_FFWidgetType::kCheckButton:
        wsValue = defValue ? defValue->GetChildValueContent() : WideString();
        if (wsValue.IsEmpty()) {
          break;
        }

        pDataNode->JSObject()->SetAttributeValue(
            wsValue, pFormNode->GetFormatDataValue(wsValue));
        break;
      case XFA_FFWidgetType::kExclGroup: {
        CXFA_Node* pChecked = nullptr;
        CXFA_Node* pChild = pFormNode->GetFirstChild();
        for (; pChild; pChild = pChild->GetNextSibling()) {
          if (pChild->GetElementType() != XFA_Element::Field) {
            continue;
          }

          auto* pValue =
              pChild->GetChild<CXFA_Value>(0, XFA_Element::Value, false);
          if (!pValue) {
            continue;
          }

          wsValue = pValue->GetChildValueContent();
          if (wsValue.IsEmpty()) {
            continue;
          }

          CXFA_Items* pItems =
              pChild->GetChild<CXFA_Items>(0, XFA_Element::Items, false);
          if (!pItems) {
            continue;
          }

          CXFA_Node* pText = pItems->GetFirstChild();
          if (!pText) {
            continue;
          }

          WideString wsContent = pText->JSObject()->GetContent(false);
          if (wsContent == wsValue) {
            pChecked = pChild;
            pDataNode->JSObject()->SetAttributeValue(wsValue, wsValue);
            pFormNode->JSObject()->SetCData(XFA_Attribute::Value, wsContent);
            break;
          }
        }
        if (!pChecked) {
          break;
        }

        pChild = pFormNode->GetFirstChild();
        for (; pChild; pChild = pChild->GetNextSibling()) {
          if (pChild == pChecked) {
            continue;
          }
          if (pChild->GetElementType() != XFA_Element::Field) {
            continue;
          }

          CXFA_Value* pValue =
              pChild->JSObject()->GetOrCreateProperty<CXFA_Value>(
                  0, XFA_Element::Value);
          CXFA_Items* pItems =
              pChild->GetChild<CXFA_Items>(0, XFA_Element::Items, false);
          CXFA_Node* pText = pItems ? pItems->GetFirstChild() : nullptr;
          if (pText) {
            pText = pText->GetNextSibling();
          }

          WideString wsContent;
          if (pText) {
            wsContent = pText->JSObject()->GetContent(false);
          }

          FormValueNode_SetChildContent(pValue, wsContent, XFA_Element::Text);
        }
        break;
      }
      case XFA_FFWidgetType::kNumericEdit: {
        wsValue = defValue ? defValue->GetChildValueContent() : WideString();
        if (wsValue.IsEmpty()) {
          break;
        }

        wsValue = pFormNode->NormalizeNumStr(wsValue);
        pDataNode->JSObject()->SetAttributeValue(
            wsValue, pFormNode->GetFormatDataValue(wsValue));
        CXFA_Value* pValue =
            pFormNode->JSObject()->GetOrCreateProperty<CXFA_Value>(
                0, XFA_Element::Value);
        FormValueNode_SetChildContent(pValue, wsValue, XFA_Element::Float);
        break;
      }
      default:
        wsValue = defValue ? defValue->GetChildValueContent() : WideString();
        if (wsValue.IsEmpty()) {
          break;
        }

        pDataNode->JSObject()->SetAttributeValue(
            wsValue, pFormNode->GetFormatDataValue(wsValue));
        break;
    }
    return;
  }

  WideString wsXMLValue = pDataNode->JSObject()->GetContent(false);
  WideString wsNormalizeValue = pFormNode->GetNormalizeDataValue(wsXMLValue);

  pDataNode->JSObject()->SetAttributeValue(wsNormalizeValue, wsXMLValue);
  switch (pFormNode->GetFFWidgetType()) {
    case XFA_FFWidgetType::kImageEdit: {
      FormValueNode_SetChildContent(defValue, wsNormalizeValue,
                                    XFA_Element::Image);
      CXFA_Image* image = defValue ? defValue->GetImageIfExists() : nullptr;
      if (image) {
        CFX_XMLElement* pXMLDataElement =
            ToXMLElement(pDataNode->GetXMLMappingNode());
        WideString wsContentType =
            pXMLDataElement->GetAttribute(L"xfa:contentType");
        if (!wsContentType.IsEmpty()) {
          pDataNode->JSObject()->SetCData(XFA_Attribute::ContentType,
                                          wsContentType);
          image->SetContentType(wsContentType);
        }

        WideString wsHref = pXMLDataElement->GetAttribute(L"href");
        if (!wsHref.IsEmpty()) {
          image->SetHref(wsHref);
        }
      }
      break;
    }
    case XFA_FFWidgetType::kChoiceList:
      if (pFormNode->IsChoiceListMultiSelect()) {
        std::vector<CXFA_Node*> items = pDataNode->GetNodeListWithFilter(
            {XFA_NodeFilter::kChildren, XFA_NodeFilter::kProperties});
        if (!items.empty()) {
          bool single = items.size() == 1;
          wsNormalizeValue.clear();

          for (CXFA_Node* pNode : items) {
            WideString wsItem = pNode->JSObject()->GetContent(false);
            if (single) {
              wsItem += L"\n";
            }

            wsNormalizeValue += wsItem;
          }
          CXFA_ExData* exData =
              defValue ? defValue->GetExDataIfExists() : nullptr;
          if (exData) {
            exData->SetContentType(single ? L"text/plain" : L"text/xml");
          }
        }
        FormValueNode_SetChildContent(defValue, wsNormalizeValue,
                                      XFA_Element::ExData);
      } else {
        FormValueNode_SetChildContent(defValue, wsNormalizeValue,
                                      XFA_Element::Text);
      }
      break;
    case XFA_FFWidgetType::kExclGroup: {
      pFormNode->SetSelectedMemberByValue(wsNormalizeValue.AsStringView(),
                                          false, false, false);
      break;
    }
    case XFA_FFWidgetType::kDateTimeEdit:
      FormValueNode_SetChildContent(defValue, wsNormalizeValue,
                                    XFA_Element::DateTime);
      break;
    case XFA_FFWidgetType::kNumericEdit: {
      WideString wsPicture =
          pFormNode->GetPictureContent(XFA_ValuePicture::kDataBind);
      if (wsPicture.IsEmpty()) {
        wsNormalizeValue = pFormNode->NormalizeNumStr(wsNormalizeValue);
      }

      FormValueNode_SetChildContent(defValue, wsNormalizeValue,
                                    XFA_Element::Float);
      break;
    }
    default:
      FormValueNode_SetChildContent(defValue, wsNormalizeValue,
                                    XFA_Element::Text);
      break;
  }
}

CXFA_Node* MaybeCreateDataNode(CXFA_Document* document,
                               CXFA_Node* pDataParent,
                               XFA_Element eNodeType,
                               const WideString& wsName) {
  if (!pDataParent) {
    return nullptr;
  }

  CXFA_Node* pParentDDNode = pDataParent->GetDataDescriptionNode();
  if (!pParentDDNode) {
    CXFA_Node* pDataNode =
        document->CreateNode(XFA_PacketType::Datasets, eNodeType);
    pDataNode->JSObject()->SetCData(XFA_Attribute::Name, wsName);
    pDataNode->CreateXMLMappingNode();
    pDataParent->InsertChildAndNotify(pDataNode, nullptr);
    pDataNode->SetFlag(XFA_NodeFlag::kInitialized);
    return pDataNode;
  }

  CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_DDGroup> sIterator(
      pParentDDNode);
  for (CXFA_Node* pDDGroupNode = sIterator.GetCurrent(); pDDGroupNode;
       pDDGroupNode = sIterator.MoveToNext()) {
    if (pDDGroupNode != pParentDDNode) {
      if (pDDGroupNode->GetElementType() != XFA_Element::DataGroup) {
        continue;
      }

      std::optional<WideString> ns = pDDGroupNode->JSObject()->TryNamespace();
      if (!ns.has_value() ||
          !ns.value().EqualsASCII("http://ns.adobe.com/data-description/")) {
        continue;
      }
    }

    CXFA_Node* pDDNode =
        pDDGroupNode->GetFirstChildByName(wsName.AsStringView());
    if (!pDDNode) {
      continue;
    }
    if (pDDNode->GetElementType() != eNodeType) {
      break;
    }

    CXFA_Node* pDataNode =
        document->CreateNode(XFA_PacketType::Datasets, eNodeType);
    pDataNode->JSObject()->SetCData(XFA_Attribute::Name, wsName);
    pDataNode->CreateXMLMappingNode();
    if (eNodeType == XFA_Element::DataValue &&
        pDDNode->JSObject()->GetEnum(XFA_Attribute::Contains) ==
            XFA_AttributeValue::MetaData) {
      pDataNode->JSObject()->SetEnum(XFA_Attribute::Contains,
                                     XFA_AttributeValue::MetaData, false);
    }
    pDataParent->InsertChildAndNotify(pDataNode, nullptr);
    pDataNode->SetDataDescriptionNode(pDDNode);
    pDataNode->SetFlag(XFA_NodeFlag::kInitialized);
    return pDataNode;
  }
  return nullptr;
}

CXFA_Node* CopyContainer_Field(CXFA_Document* document,
                               CXFA_Node* pTemplateNode,
                               CXFA_Node* pFormNode,
                               CXFA_Node* pDataScope,
                               bool bDataMerge,
                               bool bUpLevel) {
  CXFA_Node* pFieldNode = XFA_NodeMerge_CloneOrMergeContainer(
      document, pFormNode, pTemplateNode, false, nullptr);
  DCHECK(pFieldNode);
  for (CXFA_Node* pTemplateChildNode = pTemplateNode->GetFirstChild();
       pTemplateChildNode;
       pTemplateChildNode = pTemplateChildNode->GetNextSibling()) {
    if (XFA_DataMerge_NeedGenerateForm(pTemplateChildNode, true)) {
      XFA_NodeMerge_CloneOrMergeContainer(document, pFieldNode,
                                          pTemplateChildNode, true, nullptr);
    } else if (pTemplateNode->GetElementType() == XFA_Element::ExclGroup &&
               pTemplateChildNode->IsContainerNode()) {
      if (pTemplateChildNode->GetElementType() == XFA_Element::Field) {
        CopyContainer_Field(document, pTemplateChildNode, pFieldNode, nullptr,
                            false, true);
      }
    }
  }
  if (bDataMerge) {
    bool bAccessedDataDOM = false;
    bool bSelfMatch = false;
    XFA_AttributeValue eBindMatch;
    CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_XFAContainerNode>
        sNodeIter(pTemplateNode);
    CXFA_Node* pDataNode = FindMatchingDataNode(
        document, pTemplateNode, pDataScope, bAccessedDataDOM, true, &sNodeIter,
        bSelfMatch, eBindMatch, bUpLevel);
    if (pDataNode) {
      CreateDataBinding(pFieldNode, pDataNode, true);
    }
  } else {
    FormValueNode_MatchNoneCreateChild(pFieldNode);
  }
  return pFieldNode;
}

CXFA_Node* CopyContainer_SubformSet(CXFA_Document* document,
                                    CXFA_Node* pTemplateNode,
                                    CXFA_Node* pFormParentNode,
                                    CXFA_Node* pDataScope,
                                    bool bOneInstance,
                                    bool bDataMerge) {
  XFA_Element eType = pTemplateNode->GetElementType();
  CXFA_Node* pOccurNode = nullptr;
  CXFA_Node* pFirstInstance = nullptr;
  bool bUseInstanceManager =
      pFormParentNode->GetElementType() != XFA_Element::Area;
  CXFA_Node* pInstMgrNode = nullptr;
  std::vector<CXFA_Node*> subformArray;
  std::vector<CXFA_Node*>* pSearchArray = nullptr;
  if (!bOneInstance &&
      (eType == XFA_Element::SubformSet || eType == XFA_Element::Subform)) {
    pInstMgrNode = bUseInstanceManager ? CloneOrMergeInstanceManager(
                                             document, pFormParentNode,
                                             pTemplateNode, &subformArray)
                                       : nullptr;
    if (CXFA_Occur* pOccurTemplateNode =
            pTemplateNode->GetFirstChildByClass<CXFA_Occur>(
                XFA_Element::Occur)) {
      pOccurNode = pInstMgrNode ? XFA_NodeMerge_CloneOrMergeContainer(
                                      document, pInstMgrNode,
                                      pOccurTemplateNode, false, nullptr)
                                : pOccurTemplateNode;
    } else if (pInstMgrNode) {
      pOccurNode =
          pInstMgrNode->GetFirstChildByClass<CXFA_Occur>(XFA_Element::Occur);
      if (pOccurNode) {
        pOccurNode->ClearFlag(XFA_NodeFlag::kUnusedNode);
      }
    }
    if (pInstMgrNode) {
      pInstMgrNode->SetInitializedFlagAndNotify();
      pSearchArray = &subformArray;
      if (pFormParentNode->GetElementType() == XFA_Element::PageArea) {
        bOneInstance = true;
        if (subformArray.empty()) {
          pSearchArray = nullptr;
        }
      } else if (pTemplateNode->GetNameHash() == 0 && subformArray.empty()) {
        pSearchArray = nullptr;
      }
    }
  }

  int32_t iMax = 1;
  int32_t iInit = 1;
  int32_t iMin = 1;
  if (!bOneInstance && pOccurNode) {
    std::tie(iMin, iMax, iInit) =
        static_cast<CXFA_Occur*>(pOccurNode)->GetOccurInfo();
  }

  XFA_AttributeValue eRelation =
      eType == XFA_Element::SubformSet
          ? pTemplateNode->JSObject()->GetEnum(XFA_Attribute::Relation)
          : XFA_AttributeValue::Ordered;
  int32_t iCurRepeatIndex = 0;
  XFA_AttributeValue eParentBindMatch = XFA_AttributeValue::None;
  if (bDataMerge) {
    CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_XFAContainerNode>
        sNodeIterator(pTemplateNode);
    bool bAccessedDataDOM = false;
    if (eType == XFA_Element::SubformSet || eType == XFA_Element::Area) {
      sNodeIterator.MoveToNext();
    } else {
      std::map<CXFA_Node*, CXFA_Node*> subformMapArray;
      std::vector<CXFA_Node*> nodeArray;
      for (; iMax < 0 || iCurRepeatIndex < iMax; iCurRepeatIndex++) {
        bool bSelfMatch = false;
        XFA_AttributeValue eBindMatch = XFA_AttributeValue::None;
        CXFA_Node* pDataNode = FindMatchingDataNode(
            document, pTemplateNode, pDataScope, bAccessedDataDOM, false,
            &sNodeIterator, bSelfMatch, eBindMatch, true);
        if (!pDataNode || sNodeIterator.GetCurrent() != pTemplateNode) {
          break;
        }

        eParentBindMatch = eBindMatch;
        CXFA_Node* pSubformNode = XFA_NodeMerge_CloneOrMergeContainer(
            document, pFormParentNode, pTemplateNode, false, pSearchArray);
        if (!pFirstInstance) {
          pFirstInstance = pSubformNode;
        }

        CreateDataBinding(pSubformNode, pDataNode, true);
        DCHECK(pSubformNode);
        subformMapArray[pSubformNode] = pDataNode;
        nodeArray.push_back(pSubformNode);
      }

      for (CXFA_Node* pSubform : nodeArray) {
        CXFA_Node* pDataNode = nullptr;
        auto it = subformMapArray.find(pSubform);
        if (it != subformMapArray.end()) {
          pDataNode = it->second;
        }
        for (CXFA_Node* pTemplateChild = pTemplateNode->GetFirstChild();
             pTemplateChild;
             pTemplateChild = pTemplateChild->GetNextSibling()) {
          if (XFA_DataMerge_NeedGenerateForm(pTemplateChild,
                                             bUseInstanceManager)) {
            XFA_NodeMerge_CloneOrMergeContainer(document, pSubform,
                                                pTemplateChild, true, nullptr);
          } else if (pTemplateChild->IsContainerNode()) {
            document->DataMerge_CopyContainer(pTemplateChild, pSubform,
                                              pDataNode, false, true, false);
          }
        }
      }
      subformMapArray.clear();
    }

    for (; iMax < 0 || iCurRepeatIndex < iMax; iCurRepeatIndex++) {
      bool bSelfMatch = false;
      XFA_AttributeValue eBindMatch = XFA_AttributeValue::None;
      if (!FindMatchingDataNode(document, pTemplateNode, pDataScope,
                                bAccessedDataDOM, false, &sNodeIterator,
                                bSelfMatch, eBindMatch, true)) {
        break;
      }
      if (eBindMatch == XFA_AttributeValue::DataRef &&
          eParentBindMatch == XFA_AttributeValue::DataRef) {
        break;
      }

      if (eRelation == XFA_AttributeValue::Choice ||
          eRelation == XFA_AttributeValue::Unordered) {
        CXFA_Node* pSubformSetNode = XFA_NodeMerge_CloneOrMergeContainer(
            document, pFormParentNode, pTemplateNode, false, pSearchArray);
        DCHECK(pSubformSetNode);
        if (!pFirstInstance) {
          pFirstInstance = pSubformSetNode;
        }

        std::vector<RecurseRecord> rgItemMatchList;
        std::vector<CXFA_Node*> rgItemUnmatchList;
        for (CXFA_Node* pTemplateChild = pTemplateNode->GetFirstChild();
             pTemplateChild;
             pTemplateChild = pTemplateChild->GetNextSibling()) {
          if (XFA_DataMerge_NeedGenerateForm(pTemplateChild,
                                             bUseInstanceManager)) {
            XFA_NodeMerge_CloneOrMergeContainer(document, pSubformSetNode,
                                                pTemplateChild, true, nullptr);
          } else if (pTemplateChild->IsContainerNode()) {
            bSelfMatch = false;
            eBindMatch = XFA_AttributeValue::None;
            if (eRelation != XFA_AttributeValue::Ordered) {
              CXFA_NodeIteratorTemplate<CXFA_Node,
                                        CXFA_TraverseStrategy_XFAContainerNode>
                  sChildIter(pTemplateChild);
              CXFA_Node* pDataMatch = FindMatchingDataNode(
                  document, pTemplateChild, pDataScope, bAccessedDataDOM, false,
                  &sChildIter, bSelfMatch, eBindMatch, true);
              if (pDataMatch) {
                RecurseRecord sNewRecord = {pTemplateChild, pDataMatch};
                if (bSelfMatch) {
                  rgItemMatchList.insert(rgItemMatchList.begin(), sNewRecord);
                } else {
                  rgItemMatchList.push_back(sNewRecord);
                }
              } else {
                rgItemUnmatchList.push_back(pTemplateChild);
              }
            } else {
              rgItemUnmatchList.push_back(pTemplateChild);
            }
          }
        }

        switch (eRelation) {
          case XFA_AttributeValue::Choice: {
            DCHECK(!rgItemMatchList.empty());
            SortRecurseRecord(&rgItemMatchList, pDataScope, true);
            document->DataMerge_CopyContainer(
                rgItemMatchList.front().pTemplateChild, pSubformSetNode,
                pDataScope, false, true, true);
            break;
          }
          case XFA_AttributeValue::Unordered: {
            if (!rgItemMatchList.empty()) {
              SortRecurseRecord(&rgItemMatchList, pDataScope, false);
              for (const auto& matched : rgItemMatchList) {
                document->DataMerge_CopyContainer(matched.pTemplateChild,
                                                  pSubformSetNode, pDataScope,
                                                  false, true, true);
              }
            }
            for (auto* unmatched : rgItemUnmatchList) {
              document->DataMerge_CopyContainer(unmatched, pSubformSetNode,
                                                pDataScope, false, true, true);
            }
            break;
          }
          default:
            break;
        }
      } else {
        CXFA_Node* pSubformSetNode = XFA_NodeMerge_CloneOrMergeContainer(
            document, pFormParentNode, pTemplateNode, false, pSearchArray);
        DCHECK(pSubformSetNode);
        if (!pFirstInstance) {
          pFirstInstance = pSubformSetNode;
        }

        for (CXFA_Node* pTemplateChild = pTemplateNode->GetFirstChild();
             pTemplateChild;
             pTemplateChild = pTemplateChild->GetNextSibling()) {
          if (XFA_DataMerge_NeedGenerateForm(pTemplateChild,
                                             bUseInstanceManager)) {
            XFA_NodeMerge_CloneOrMergeContainer(document, pSubformSetNode,
                                                pTemplateChild, true, nullptr);
          } else if (pTemplateChild->IsContainerNode()) {
            document->DataMerge_CopyContainer(pTemplateChild, pSubformSetNode,
                                              pDataScope, false, true, true);
          }
        }
      }
    }

    if (iCurRepeatIndex == 0 && !bAccessedDataDOM) {
      int32_t iLimit = iMax;
      if (pInstMgrNode && pTemplateNode->GetNameHash() == 0) {
        iLimit = fxcrt::CollectionSize<int32_t>(subformArray);
        if (iLimit < iMin) {
          iLimit = iInit;
        }
      }

      for (; (iLimit < 0 || iCurRepeatIndex < iLimit); iCurRepeatIndex++) {
        if (pInstMgrNode) {
          if (pSearchArray && pSearchArray->empty()) {
            if (pTemplateNode->GetNameHash() != 0) {
              break;
            }
            pSearchArray = nullptr;
          }
        } else if (!XFA_DataMerge_FindFormDOMInstance(
                       document, pTemplateNode->GetElementType(),
                       pTemplateNode->GetNameHash(), pFormParentNode)) {
          break;
        }
        CXFA_Node* pSubformNode = XFA_NodeMerge_CloneOrMergeContainer(
            document, pFormParentNode, pTemplateNode, false, pSearchArray);
        DCHECK(pSubformNode);
        if (!pFirstInstance) {
          pFirstInstance = pSubformNode;
        }

        for (CXFA_Node* pTemplateChild = pTemplateNode->GetFirstChild();
             pTemplateChild;
             pTemplateChild = pTemplateChild->GetNextSibling()) {
          if (XFA_DataMerge_NeedGenerateForm(pTemplateChild,
                                             bUseInstanceManager)) {
            XFA_NodeMerge_CloneOrMergeContainer(document, pSubformNode,
                                                pTemplateChild, true, nullptr);
          } else if (pTemplateChild->IsContainerNode()) {
            document->DataMerge_CopyContainer(pTemplateChild, pSubformNode,
                                              pDataScope, false, true, true);
          }
        }
      }
    }
  }

  int32_t iMinimalLimit = iCurRepeatIndex == 0 ? iInit : iMin;
  for (; iCurRepeatIndex < iMinimalLimit; iCurRepeatIndex++) {
    CXFA_Node* pSubformSetNode = XFA_NodeMerge_CloneOrMergeContainer(
        document, pFormParentNode, pTemplateNode, false, pSearchArray);
    DCHECK(pSubformSetNode);
    if (!pFirstInstance) {
      pFirstInstance = pSubformSetNode;
    }

    bool bFound = false;
    for (CXFA_Node* pTemplateChild = pTemplateNode->GetFirstChild();
         pTemplateChild; pTemplateChild = pTemplateChild->GetNextSibling()) {
      if (pTemplateChild->GetPacketType() != XFA_PacketType::Template) {
        continue;
      }
      if (XFA_DataMerge_NeedGenerateForm(pTemplateChild, bUseInstanceManager)) {
        XFA_NodeMerge_CloneOrMergeContainer(document, pSubformSetNode,
                                            pTemplateChild, true, nullptr);
      } else if (pTemplateChild->IsContainerNode()) {
        if (bFound && eRelation == XFA_AttributeValue::Choice) {
          continue;
        }

        document->DataMerge_CopyContainer(pTemplateChild, pSubformSetNode,
                                          pDataScope, false, bDataMerge, true);
        bFound = true;
      }
    }
  }
  return pFirstInstance;
}

void UpdateBindingRelations(CXFA_Document* document,
                            CXFA_Node* pFormNode,
                            CXFA_Node* pDataScope,
                            bool bDataRef,
                            bool bParentDataRef) {
  bool bMatchRef = true;
  XFA_Element eType = pFormNode->GetElementType();
  CXFA_Node* pDataNode = pFormNode->GetBindData();
  if (eType == XFA_Element::Subform || eType == XFA_Element::ExclGroup ||
      eType == XFA_Element::Field) {
    CXFA_Node* pTemplateNode = pFormNode->GetTemplateNodeIfExists();
    CXFA_Bind* pTemplateNodeBind =
        pTemplateNode
            ? pTemplateNode->GetFirstChildByClass<CXFA_Bind>(XFA_Element::Bind)
            : nullptr;
    XFA_AttributeValue eMatch =
        pTemplateNodeBind
            ? pTemplateNodeBind->JSObject()->GetEnum(XFA_Attribute::Match)
            : XFA_AttributeValue::Once;
    switch (eMatch) {
      case XFA_AttributeValue::None:
        if (!bDataRef || bParentDataRef) {
          FormValueNode_MatchNoneCreateChild(pFormNode);
        }
        break;
      case XFA_AttributeValue::Once:
        if (!bDataRef || bParentDataRef) {
          if (!pDataNode) {
            if (pFormNode->GetNameHash() != 0 &&
                pFormNode->JSObject()->GetEnum(XFA_Attribute::Scope) !=
                    XFA_AttributeValue::None) {
              XFA_Element eDataNodeType = (eType == XFA_Element::Subform ||
                                           XFA_FieldIsMultiListBox(pFormNode))
                                              ? XFA_Element::DataGroup
                                              : XFA_Element::DataValue;
              pDataNode = MaybeCreateDataNode(
                  document, pDataScope, eDataNodeType,
                  WideString(
                      pFormNode->JSObject()->GetCData(XFA_Attribute::Name)));
              if (pDataNode) {
                CreateDataBinding(pFormNode, pDataNode, false);
              }
            }
            if (!pDataNode) {
              FormValueNode_MatchNoneCreateChild(pFormNode);
            }

          } else {
            CXFA_Node* pDataParent = pDataNode->GetParent();
            if (pDataParent != pDataScope) {
              DCHECK(pDataParent);
              pDataParent->RemoveChildAndNotify(pDataNode, true);
              pDataScope->InsertChildAndNotify(pDataNode, nullptr);
            }
          }
        }
        break;
      case XFA_AttributeValue::Global:
        if (!bDataRef || bParentDataRef) {
          uint32_t dwNameHash = pFormNode->GetNameHash();
          if (dwNameHash != 0 && !pDataNode) {
            pDataNode = document->GetGlobalBinding(dwNameHash);
            if (!pDataNode) {
              XFA_Element eDataNodeType = (eType == XFA_Element::Subform ||
                                           XFA_FieldIsMultiListBox(pFormNode))
                                              ? XFA_Element::DataGroup
                                              : XFA_Element::DataValue;
              CXFA_Node* pRecordNode =
                  ToNode(document->GetXFAObject(XFA_HASHCODE_Record));
              pDataNode = MaybeCreateDataNode(
                  document, pRecordNode, eDataNodeType,
                  WideString(
                      pFormNode->JSObject()->GetCData(XFA_Attribute::Name)));
              if (pDataNode) {
                CreateDataBinding(pFormNode, pDataNode, false);
                document->RegisterGlobalBinding(pFormNode->GetNameHash(),
                                                pDataNode);
              }
            } else {
              CreateDataBinding(pFormNode, pDataNode, true);
            }
          }
          if (!pDataNode) {
            FormValueNode_MatchNoneCreateChild(pFormNode);
          }
        }
        break;
      case XFA_AttributeValue::DataRef: {
        bMatchRef = bDataRef;
        bParentDataRef = true;
        if (!pDataNode && bDataRef) {
          WideString wsRef =
              pTemplateNodeBind
                  ? pTemplateNodeBind->JSObject()->GetCData(XFA_Attribute::Ref)
                  : WideString();
          const Mask<XFA_ResolveFlag> kFlags = {XFA_ResolveFlag::kChildren,
                                                XFA_ResolveFlag::kCreateNode};
          std::optional<CFXJSE_Engine::ResolveResult> maybeResult =
              document->GetScriptContext()->ResolveObjectsWithBindNode(
                  pDataScope, wsRef.AsStringView(), kFlags, pTemplateNode);
          CXFA_Object* pObject =
              maybeResult.has_value() && !maybeResult.value().objects.empty()
                  ? maybeResult.value().objects.front().Get()
                  : nullptr;
          pDataNode = ToNode(pObject);
          if (pDataNode) {
            CreateDataBinding(
                pFormNode, pDataNode,
                maybeResult.value().type ==
                    CFXJSE_Engine::ResolveResult::Type::kExistNodes);
          } else {
            FormValueNode_MatchNoneCreateChild(pFormNode);
          }
        }
        break;
      }
      default:
        break;
    }
  }

  if (bMatchRef &&
      (eType == XFA_Element::Subform || eType == XFA_Element::SubformSet ||
       eType == XFA_Element::Area || eType == XFA_Element::PageArea ||
       eType == XFA_Element::PageSet)) {
    for (CXFA_Node* pFormChild = pFormNode->GetFirstChild(); pFormChild;
         pFormChild = pFormChild->GetNextSibling()) {
      if (!pFormChild->IsContainerNode()) {
        continue;
      }
      if (pFormChild->IsUnusedNode()) {
        continue;
      }

      UpdateBindingRelations(document, pFormChild,
                             pDataNode ? pDataNode : pDataScope, bDataRef,
                             bParentDataRef);
    }
  }
}

void UpdateDataRelation(CXFA_Node* pDataNode, CXFA_Node* pDataDescriptionNode) {
  DCHECK(pDataDescriptionNode);
  for (CXFA_Node* pDataChild = pDataNode->GetFirstChild(); pDataChild;
       pDataChild = pDataChild->GetNextSibling()) {
    uint32_t dwNameHash = pDataChild->GetNameHash();
    if (!dwNameHash) {
      continue;
    }

    CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_DDGroup>
        sIterator(pDataDescriptionNode);
    for (CXFA_Node* pDDGroupNode = sIterator.GetCurrent(); pDDGroupNode;
         pDDGroupNode = sIterator.MoveToNext()) {
      if (pDDGroupNode != pDataDescriptionNode) {
        if (pDDGroupNode->GetElementType() != XFA_Element::DataGroup) {
          continue;
        }

        std::optional<WideString> ns = pDDGroupNode->JSObject()->TryNamespace();
        if (!ns.has_value() ||
            !ns.value().EqualsASCII("http://ns.adobe.com/data-description/")) {
          continue;
        }
      }

      CXFA_Node* pDDNode = pDDGroupNode->GetFirstChildByName(dwNameHash);
      if (!pDDNode) {
        continue;
      }
      if (pDDNode->GetElementType() != pDataChild->GetElementType()) {
        break;
      }

      pDataChild->SetDataDescriptionNode(pDDNode);
      UpdateDataRelation(pDataChild, pDDNode);
      break;
    }
  }
}

}  // namespace

CXFA_Document::CXFA_Document(CXFA_FFNotify* notify,
                             cppgc::Heap* heap,
                             LayoutProcessorIface* pLayout)
    : heap_(heap),
      notify_(notify),
      node_owner_(cppgc::MakeGarbageCollected<CXFA_NodeOwner>(
          heap->GetAllocationHandle())),
      layout_processor_(pLayout) {
  if (layout_processor_) {
    layout_processor_->SetDocument(this);
  }
}

CXFA_Document::~CXFA_Document() = default;

void CXFA_Document::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(notify_);
  visitor->Trace(node_owner_);
  visitor->Trace(root_node_);
  visitor->Trace(locale_mgr_);
  visitor->Trace(layout_processor_);
  visitor->Trace(script_data_window_);
  visitor->Trace(script_event_);
  visitor->Trace(script_host_);
  visitor->Trace(script_log_);
  visitor->Trace(script_layout_);
  visitor->Trace(script_signature_);
  ContainerTrace(visitor, rg_global_binding_);
  ContainerTrace(visitor, pending_page_set_);
}

void CXFA_Document::ClearLayoutData() {
  layout_processor_ = nullptr;
  script_context_.reset();
  locale_mgr_.Clear();
  script_data_window_ = nullptr;
  script_event_ = nullptr;
  script_host_ = nullptr;
  script_log_ = nullptr;
  script_layout_ = nullptr;
  script_signature_ = nullptr;
}

CXFA_Object* CXFA_Document::GetXFAObject(XFA_HashCode dwNodeNameHash) {
  switch (dwNodeNameHash) {
    case XFA_HASHCODE_Data: {
      CXFA_Node* pDatasetsNode = ToNode(GetXFAObject(XFA_HASHCODE_Datasets));
      if (!pDatasetsNode) {
        return nullptr;
      }

      for (CXFA_DataGroup* pDatasetsChild =
               pDatasetsNode->GetFirstChildByClass<CXFA_DataGroup>(
                   XFA_Element::DataGroup);
           pDatasetsChild;
           pDatasetsChild =
               pDatasetsChild->GetNextSameClassSibling<CXFA_DataGroup>(
                   XFA_Element::DataGroup)) {
        if (pDatasetsChild->GetNameHash() != XFA_HASHCODE_Data) {
          continue;
        }

        std::optional<WideString> namespaceURI =
            pDatasetsChild->JSObject()->TryNamespace();
        if (!namespaceURI.has_value()) {
          continue;
        }

        std::optional<WideString> datasetsURI =
            pDatasetsNode->JSObject()->TryNamespace();
        if (!datasetsURI.has_value()) {
          continue;
        }
        if (namespaceURI.value() == datasetsURI.value()) {
          return pDatasetsChild;
        }
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
      if (!script_data_window_) {
        script_data_window_ = cppgc::MakeGarbageCollected<CScript_DataWindow>(
            GetHeap()->GetAllocationHandle(), this);
      }
      return script_data_window_;
    }
    case XFA_HASHCODE_Event: {
      if (!script_event_) {
        script_event_ = cppgc::MakeGarbageCollected<CScript_EventPseudoModel>(
            GetHeap()->GetAllocationHandle(), this);
      }
      return script_event_;
    }
    case XFA_HASHCODE_Host: {
      if (!script_host_) {
        script_host_ = cppgc::MakeGarbageCollected<CScript_HostPseudoModel>(
            GetHeap()->GetAllocationHandle(), this);
      }
      return script_host_;
    }
    case XFA_HASHCODE_Log: {
      if (!script_log_) {
        script_log_ = cppgc::MakeGarbageCollected<CScript_LogPseudoModel>(
            GetHeap()->GetAllocationHandle(), this);
      }
      return script_log_;
    }
    case XFA_HASHCODE_Signature: {
      if (!script_signature_) {
        script_signature_ =
            cppgc::MakeGarbageCollected<CScript_SignaturePseudoModel>(
                GetHeap()->GetAllocationHandle(), this);
      }
      return script_signature_;
    }
    case XFA_HASHCODE_Layout: {
      if (!script_layout_) {
        script_layout_ = cppgc::MakeGarbageCollected<CScript_LayoutPseudoModel>(
            GetHeap()->GetAllocationHandle(), this);
      }
      return script_layout_;
    }
    default:
      return root_node_->GetFirstChildByName(dwNodeNameHash);
  }
}

CXFA_Node* CXFA_Document::CreateNode(XFA_PacketType packet,
                                     XFA_Element eElement) {
  if (eElement == XFA_Element::Unknown) {
    return nullptr;
  }

  return CXFA_Node::Create(this, eElement, packet);
}

bool CXFA_Document::IsInteractive() {
  if (interactive_.has_value()) {
    return interactive_.value();
  }

  CXFA_Node* pConfig = ToNode(GetXFAObject(XFA_HASHCODE_Config));
  if (!pConfig) {
    return false;
  }

  CXFA_Present* pPresent =
      pConfig->GetFirstChildByClass<CXFA_Present>(XFA_Element::Present);
  if (!pPresent) {
    return false;
  }

  CXFA_Pdf* pPDF = pPresent->GetFirstChildByClass<CXFA_Pdf>(XFA_Element::Pdf);
  if (!pPDF) {
    return false;
  }

  CXFA_Interactive* pFormFiller =
      pPDF->GetChild<CXFA_Interactive>(0, XFA_Element::Interactive, false);
  if (!pFormFiller) {
    return false;
  }

  WideString wsInteractive = pFormFiller->JSObject()->GetContent(false);
  bool bInteractive = wsInteractive.EqualsASCII("1");
  interactive_ = bInteractive;
  return bInteractive;
}

CXFA_LocaleMgr* CXFA_Document::GetLocaleMgr() {
  if (!locale_mgr_) {
    locale_mgr_ = cppgc::MakeGarbageCollected<CXFA_LocaleMgr>(
        heap_->GetAllocationHandle(), heap_,
        ToNode(GetXFAObject(XFA_HASHCODE_LocaleSet)),
        GetNotify()->GetAppProvider()->GetLanguage());
  }
  return locale_mgr_;
}

cppgc::Heap* CXFA_Document::GetHeap() const {
  return heap_;
}

CFXJSE_Engine* CXFA_Document::InitScriptContext(CJS_Runtime* fxjs_runtime) {
  DCHECK(!script_context_);
  script_context_ = std::make_unique<CFXJSE_Engine>(this, fxjs_runtime);
  return script_context_.get();
}

CFXJSE_Engine* CXFA_Document::GetScriptContext() const {
  DCHECK(script_context_);
  return script_context_.get();
}

XFA_VERSION CXFA_Document::RecognizeXFAVersionNumber(
    const WideString& wsTemplateNS) {
  XFA_VERSION eVersion = ParseXFAVersion(wsTemplateNS);
  if (eVersion != XFA_VERSION_UNKNOWN) {
    cur_version_mode_ = eVersion;
  }

  return eVersion;
}

// static
XFA_VERSION CXFA_Document::ParseXFAVersion(const WideString& wsTemplateNS) {
  WideStringView wsTemplateURIPrefix(kTemplateNS);
  if (wsTemplateNS.GetLength() <= wsTemplateURIPrefix.GetLength()) {
    return XFA_VERSION_UNKNOWN;
  }

  size_t prefixLength = wsTemplateURIPrefix.GetLength();
  if (wsTemplateNS.AsStringView().First(prefixLength) != wsTemplateURIPrefix) {
    return XFA_VERSION_UNKNOWN;
  }

  auto nDotPos = wsTemplateNS.Find('.', prefixLength);
  if (!nDotPos.has_value()) {
    return XFA_VERSION_UNKNOWN;
  }

  int8_t iMajor = FXSYS_wtoi(
      wsTemplateNS.Substr(prefixLength, nDotPos.value() - prefixLength)
          .c_str());
  int8_t iMinor = FXSYS_wtoi(wsTemplateNS.Substr(nDotPos.value() + 1).c_str());
  XFA_VERSION eVersion =
      static_cast<XFA_VERSION>(static_cast<int32_t>(iMajor) * 100 + iMinor);
  if (eVersion < XFA_VERSION_MIN || eVersion > XFA_VERSION_MAX) {
    return XFA_VERSION_UNKNOWN;
  }

  return eVersion;
}

FormType CXFA_Document::GetFormType() const {
  return GetNotify()->GetFFDoc()->GetFormType();
}

CXFA_Node* CXFA_Document::GetNodeByID(CXFA_Node* pRoot,
                                      WideStringView wsID) const {
  if (!pRoot || wsID.IsEmpty()) {
    return nullptr;
  }

  CXFA_NodeIterator sIterator(pRoot);
  for (CXFA_Node* pNode = sIterator.GetCurrent(); pNode;
       pNode = sIterator.MoveToNext()) {
    WideString wsIDVal = pNode->JSObject()->GetCData(XFA_Attribute::Id);
    if (!wsIDVal.IsEmpty() && wsIDVal == wsID) {
      return pNode;
    }
  }
  return nullptr;
}

void CXFA_Document::DoProtoMerge() {
  CXFA_Node* pTemplateRoot = ToNode(GetXFAObject(XFA_HASHCODE_Template));
  if (!pTemplateRoot) {
    return;
  }

  std::map<uint32_t, CXFA_Node*> mIDMap;
  std::set<CXFA_Node*> sUseNodes;
  CXFA_NodeIterator sIterator(pTemplateRoot);
  for (CXFA_Node* pNode = sIterator.GetCurrent(); pNode;
       pNode = sIterator.MoveToNext()) {
    WideString wsIDVal = pNode->JSObject()->GetCData(XFA_Attribute::Id);
    if (!wsIDVal.IsEmpty()) {
      mIDMap[FX_HashCode_GetW(wsIDVal.AsStringView())] = pNode;
    }

    WideString wsUseVal = pNode->JSObject()->GetCData(XFA_Attribute::Use);
    if (!wsUseVal.IsEmpty()) {
      sUseNodes.insert(pNode);
    } else {
      wsUseVal = pNode->JSObject()->GetCData(XFA_Attribute::Usehref);
      if (!wsUseVal.IsEmpty()) {
        sUseNodes.insert(pNode);
      }
    }
  }

  for (CXFA_Node* pUseHrefNode : sUseNodes) {
    // Must outlive the WideStringViews below.
    WideString wsUseVal =
        pUseHrefNode->JSObject()->GetCData(XFA_Attribute::Usehref);
    WideStringView wsURI;
    WideStringView wsID;
    WideStringView wsSOM;
    if (!wsUseVal.IsEmpty()) {
      ParseUseHref(wsUseVal, wsURI, wsID, wsSOM);
      if (!wsURI.IsEmpty() && !wsURI.EqualsASCII(".")) {
        continue;
      }
    } else {
      wsUseVal = pUseHrefNode->JSObject()->GetCData(XFA_Attribute::Use);
      ParseUse(wsUseVal, wsID, wsSOM);
    }

    CXFA_Node* pProtoNode = nullptr;
    if (!wsSOM.IsEmpty()) {
      std::optional<CFXJSE_Engine::ResolveResult> maybeResult =
          script_context_->ResolveObjects(
              pUseHrefNode, wsSOM,
              Mask<XFA_ResolveFlag>{
                  XFA_ResolveFlag::kChildren, XFA_ResolveFlag::kAttributes,
                  XFA_ResolveFlag::kProperties, XFA_ResolveFlag::kParent,
                  XFA_ResolveFlag::kSiblings});
      if (maybeResult.has_value()) {
        auto* pFirstObject = maybeResult.value().objects.front().Get();
        if (pFirstObject && pFirstObject->IsNode()) {
          pProtoNode = pFirstObject->AsNode();
        }
      }
    } else if (!wsID.IsEmpty()) {
      auto it = mIDMap.find(FX_HashCode_GetW(wsID));
      if (it == mIDMap.end()) {
        continue;
      }
      pProtoNode = it->second;
    }
    if (!pProtoNode) {
      continue;
    }

    MergeNode(pUseHrefNode, pProtoNode);
  }
}

// static
void CXFA_Document::ParseUseHref(const WideString& wsUseVal,
                                 WideStringView& wsURI,
                                 WideStringView& wsID,
                                 WideStringView& wsSOM) {
  if (wsUseVal.IsEmpty()) {
    return;
  }

  auto uSharpPos = wsUseVal.Find('#');
  if (!uSharpPos.has_value()) {
    wsURI = wsUseVal.AsStringView();
    return;
  }
  wsURI = wsUseVal.AsStringView().First(uSharpPos.value());
  if (wsUseVal.AsStringView().Substr(uSharpPos.value(), 5) == L"#som(" &&
      wsUseVal.Back() == ')') {
    wsSOM = wsUseVal.AsStringView().Substr(
        uSharpPos.value() + 5,
        wsUseVal.GetLength() - 1 - uSharpPos.value() - 5);
    return;
  }
  wsID = wsUseVal.AsStringView().Substr(uSharpPos.value() + 1);
}

// static
void CXFA_Document::ParseUse(const WideString& wsUseVal,
                             WideStringView& wsID,
                             WideStringView& wsSOM) {
  if (wsUseVal.IsEmpty()) {
    return;
  }

  if (wsUseVal[0] == '#') {
    wsID = wsUseVal.AsStringView().Substr(1);
    return;
  }
  wsSOM = wsUseVal.AsStringView();
}

CXFA_Node* CXFA_Document::DataMerge_CopyContainer(CXFA_Node* pTemplateNode,
                                                  CXFA_Node* pFormNode,
                                                  CXFA_Node* pDataScope,
                                                  bool bOneInstance,
                                                  bool bDataMerge,
                                                  bool bUpLevel) {
  CHECK(pTemplateNode->IsContainerNode());
  switch (pTemplateNode->GetElementType()) {
    case XFA_Element::Area:
    case XFA_Element::PageArea:
    case XFA_Element::Subform:
    case XFA_Element::SubformSet:
      return CopyContainer_SubformSet(this, pTemplateNode, pFormNode,
                                      pDataScope, bOneInstance, bDataMerge);
    case XFA_Element::ContentArea:
    case XFA_Element::Draw:
    case XFA_Element::ExclGroup:
    case XFA_Element::Field:
      return CopyContainer_Field(this, pTemplateNode, pFormNode, pDataScope,
                                 bDataMerge, bUpLevel);
    case XFA_Element::PageSet:
    case XFA_Element::Variables:
      return nullptr;
    default:
      NOTREACHED();
  }
}

void CXFA_Document::DataMerge_UpdateBindingRelations(
    CXFA_Node* pFormUpdateRoot) {
  CXFA_Node* pDataScope =
      XFA_DataMerge_FindDataScope(pFormUpdateRoot->GetParent());
  if (!pDataScope) {
    return;
  }

  UpdateBindingRelations(this, pFormUpdateRoot, pDataScope, false, false);
  UpdateBindingRelations(this, pFormUpdateRoot, pDataScope, true, false);
}

CXFA_Node* CXFA_Document::GetNotBindNode(
    pdfium::span<cppgc::Member<CXFA_Object>> arrayObjects) const {
  for (auto& pObject : arrayObjects) {
    CXFA_Node* pNode = pObject->AsNode();
    if (pNode && !pNode->HasBindItem()) {
      return pNode;
    }
  }
  return nullptr;
}

void CXFA_Document::DoDataMerge() {
  CXFA_Node* pDatasetsRoot = ToNode(GetXFAObject(XFA_HASHCODE_Datasets));
  if (!pDatasetsRoot) {
    // Ownership will be passed in the AppendChild below to the XML tree.
    auto* pDatasetsXMLNode =
        notify_->GetFFDoc()->GetXMLDocument()->CreateNode<CFX_XMLElement>(
            L"xfa:datasets");
    pDatasetsXMLNode->SetAttribute(L"xmlns:xfa",
                                   L"http://www.xfa.org/schema/xfa-data/1.0/");
    pDatasetsRoot =
        CreateNode(XFA_PacketType::Datasets, XFA_Element::DataModel);
    pDatasetsRoot->JSObject()->SetCData(XFA_Attribute::Name, L"datasets");

    root_node_->GetXMLMappingNode()->AppendLastChild(pDatasetsXMLNode);
    root_node_->InsertChildAndNotify(pDatasetsRoot, nullptr);
    pDatasetsRoot->SetXMLMappingNode(pDatasetsXMLNode);
  }

  CXFA_Node* pDataRoot = nullptr;
  CXFA_Node* pDDRoot = nullptr;
  WideString wsDatasetsURI =
      pDatasetsRoot->JSObject()->TryNamespace().value_or(WideString());
  for (CXFA_Node* pChildNode = pDatasetsRoot->GetFirstChild(); pChildNode;
       pChildNode = pChildNode->GetNextSibling()) {
    if (pChildNode->GetElementType() != XFA_Element::DataGroup) {
      continue;
    }

    if (!pDDRoot && pChildNode->GetNameHash() == XFA_HASHCODE_DataDescription) {
      std::optional<WideString> namespaceURI =
          pChildNode->JSObject()->TryNamespace();
      if (!namespaceURI.has_value()) {
        continue;
      }
      if (namespaceURI.value().EqualsASCII(
              "http://ns.adobe.com/data-description/")) {
        pDDRoot = pChildNode;
      }
    } else if (!pDataRoot && pChildNode->GetNameHash() == XFA_HASHCODE_Data) {
      std::optional<WideString> namespaceURI =
          pChildNode->JSObject()->TryNamespace();
      if (!namespaceURI.has_value()) {
        continue;
      }
      if (namespaceURI == wsDatasetsURI) {
        pDataRoot = pChildNode;
      }
    }
    if (pDataRoot && pDDRoot) {
      break;
    }
  }

  if (!pDataRoot) {
    pDataRoot = CreateNode(XFA_PacketType::Datasets, XFA_Element::DataGroup);
    pDataRoot->JSObject()->SetCData(XFA_Attribute::Name, L"data");

    auto* elem =
        notify_->GetFFDoc()->GetXMLDocument()->CreateNode<CFX_XMLElement>(
            L"xfa:data");
    pDataRoot->SetXMLMappingNode(elem);
    pDatasetsRoot->InsertChildAndNotify(pDataRoot, nullptr);
  }

  CXFA_DataGroup* pDataTopLevel =
      pDataRoot->GetFirstChildByClass<CXFA_DataGroup>(XFA_Element::DataGroup);
  uint32_t dwNameHash = pDataTopLevel ? pDataTopLevel->GetNameHash() : 0;
  CXFA_Template* pTemplateRoot =
      root_node_->GetFirstChildByClass<CXFA_Template>(XFA_Element::Template);
  if (!pTemplateRoot) {
    return;
  }

  CXFA_Node* pTemplateChosen =
      dwNameHash != 0 ? pTemplateRoot->GetFirstChildByName(dwNameHash)
                      : nullptr;
  if (!pTemplateChosen ||
      pTemplateChosen->GetElementType() != XFA_Element::Subform) {
    pTemplateChosen =
        pTemplateRoot->GetFirstChildByClass<CXFA_Subform>(XFA_Element::Subform);
  }
  if (!pTemplateChosen) {
    return;
  }

  CXFA_Form* pFormRoot =
      root_node_->GetFirstChildByClass<CXFA_Form>(XFA_Element::Form);
  bool bEmptyForm = false;
  if (!pFormRoot) {
    bEmptyForm = true;
    pFormRoot = static_cast<CXFA_Form*>(
        CreateNode(XFA_PacketType::Form, XFA_Element::Form));
    DCHECK(pFormRoot);
    pFormRoot->JSObject()->SetCData(XFA_Attribute::Name, L"form");
    root_node_->InsertChildAndNotify(pFormRoot, nullptr);
  } else {
    CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_XFANode>
        sIterator(pFormRoot);
    for (CXFA_Node* pNode = sIterator.MoveToNext(); pNode;
         pNode = sIterator.MoveToNext()) {
      pNode->SetFlag(XFA_NodeFlag::kUnusedNode);
    }
  }

  CXFA_Node* pSubformSetNode = XFA_NodeMerge_CloneOrMergeContainer(
      this, pFormRoot, pTemplateChosen, false, nullptr);
  DCHECK(pSubformSetNode);
  if (!pDataTopLevel) {
    WideString wsFormName =
        pSubformSetNode->JSObject()->GetCData(XFA_Attribute::Name);
    WideString wsDataTopLevelName(wsFormName.IsEmpty() ? L"form" : wsFormName);

    pDataTopLevel = static_cast<CXFA_DataGroup*>(
        CreateNode(XFA_PacketType::Datasets, XFA_Element::DataGroup));
    pDataTopLevel->JSObject()->SetCData(XFA_Attribute::Name,
                                        wsDataTopLevelName);

    auto* elem =
        notify_->GetFFDoc()->GetXMLDocument()->CreateNode<CFX_XMLElement>(
            wsDataTopLevelName);
    pDataTopLevel->SetXMLMappingNode(elem);

    CXFA_Node* pBeforeNode = pDataRoot->GetFirstChild();
    pDataRoot->InsertChildAndNotify(pDataTopLevel, pBeforeNode);
  }

  DCHECK(pDataTopLevel);
  CreateDataBinding(pSubformSetNode, pDataTopLevel, true);
  for (CXFA_Node* pTemplateChild = pTemplateChosen->GetFirstChild();
       pTemplateChild; pTemplateChild = pTemplateChild->GetNextSibling()) {
    if (XFA_DataMerge_NeedGenerateForm(pTemplateChild, true)) {
      XFA_NodeMerge_CloneOrMergeContainer(this, pSubformSetNode, pTemplateChild,
                                          true, nullptr);
    } else if (pTemplateChild->IsContainerNode()) {
      DataMerge_CopyContainer(pTemplateChild, pSubformSetNode, pDataTopLevel,
                              false, true, true);
    }
  }
  if (pDDRoot) {
    UpdateDataRelation(pDataRoot, pDDRoot);
  }

  DataMerge_UpdateBindingRelations(pSubformSetNode);
  CXFA_PageSet* pPageSetNode =
      pSubformSetNode->GetFirstChildByClass<CXFA_PageSet>(XFA_Element::PageSet);
  while (pPageSetNode) {
    pending_page_set_.push_back(pPageSetNode);
    CXFA_PageSet* pNextPageSetNode =
        pPageSetNode->GetNextSameClassSibling<CXFA_PageSet>(
            XFA_Element::PageSet);
    pSubformSetNode->RemoveChildAndNotify(pPageSetNode, true);
    pPageSetNode = pNextPageSetNode;
  }

  if (bEmptyForm) {
    return;
  }

  CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_XFANode> sIterator(
      pFormRoot);
  CXFA_Node* pNode = sIterator.MoveToNext();
  while (pNode) {
    if (pNode->IsUnusedNode()) {
      if (pNode->IsContainerNode() ||
          pNode->GetElementType() == XFA_Element::InstanceManager) {
        CXFA_Node* pNext = sIterator.SkipChildrenAndMoveToNext();
        pNode->GetParent()->RemoveChildAndNotify(pNode, true);
        pNode = pNext;
      } else {
        pNode->ClearFlag(XFA_NodeFlag::kUnusedNode);
        pNode->SetInitializedFlagAndNotify();
        pNode = sIterator.MoveToNext();
      }
    } else {
      pNode->SetInitializedFlagAndNotify();
      pNode = sIterator.MoveToNext();
    }
  }
}

void CXFA_Document::DoDataRemerge() {
  CXFA_Node* pFormRoot = ToNode(GetXFAObject(XFA_HASHCODE_Form));
  if (pFormRoot) {
    while (CXFA_Node* pNode = pFormRoot->GetFirstChild()) {
      pFormRoot->RemoveChildAndNotify(pNode, true);
    }

    pFormRoot->SetBindingNode(nullptr);
  }
  rg_global_binding_.clear();
  DoDataMerge();
  GetLayoutProcessor()->SetForceRelayout();
}

CXFA_Node* CXFA_Document::GetGlobalBinding(uint32_t dwNameHash) {
  auto it = rg_global_binding_.find(dwNameHash);
  return it != rg_global_binding_.end() ? it->second : nullptr;
}

void CXFA_Document::RegisterGlobalBinding(uint32_t dwNameHash,
                                          CXFA_Node* pDataNode) {
  rg_global_binding_[dwNameHash] = pDataNode;
}

size_t CXFA_Document::GetPendingNodesCount() const {
  return pending_page_set_.size();
}

CXFA_Node* CXFA_Document::GetPendingNodeAtIndex(size_t index) const {
  return pending_page_set_[index];
}

void CXFA_Document::AppendPendingNode(CXFA_Node* node) {
  pending_page_set_.push_back(node);
}

void CXFA_Document::ClearPendingNodes() {
  pending_page_set_.clear();
}

void CXFA_Document::SetPendingNodesUnusedAndUnbound() {
  for (CXFA_Node* pPageNode : pending_page_set_) {
    CXFA_NodeIterator sIterator(pPageNode);
    for (CXFA_Node* pNode = sIterator.GetCurrent(); pNode;
         pNode = sIterator.MoveToNext()) {
      if (pNode->IsContainerNode()) {
        CXFA_Node* pBindNode = pNode->GetBindData();
        if (pBindNode) {
          pBindNode->RemoveBindItem(pNode);
          pNode->SetBindingNode(nullptr);
        }
      }
      pNode->SetFlag(XFA_NodeFlag::kUnusedNode);
    }
  }
}

CXFA_Document::LayoutProcessorIface::LayoutProcessorIface() = default;

CXFA_Document::LayoutProcessorIface::~LayoutProcessorIface() = default;

void CXFA_Document::LayoutProcessorIface::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(document_);
}
