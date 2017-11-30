// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/xfa_document_datamerger_imp.h"

#include <map>
#include <vector>

#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "fxjs/cfxjse_engine.h"
#include "third_party/base/logging.h"
#include "third_party/base/stl_util.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_layoutprocessor.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_nodeiteratortemplate.h"
#include "xfa/fxfa/parser/cxfa_occurdata.h"
#include "xfa/fxfa/parser/cxfa_traversestrategy_xfacontainernode.h"
#include "xfa/fxfa/parser/cxfa_traversestrategy_xfanode.h"
#include "xfa/fxfa/parser/xfa_resolvenode_rs.h"
#include "xfa/fxfa/parser/xfa_utils.h"

namespace {

class CXFA_TraverseStrategy_DDGroup {
 public:
  static CXFA_Node* GetFirstChild(CXFA_Node* pDDGroupNode) {
    return pDDGroupNode->GetFirstChildByName(XFA_HASHCODE_Group);
  }
  static CXFA_Node* GetNextSibling(CXFA_Node* pDDGroupNode) {
    return pDDGroupNode->GetNextSameNameSibling(XFA_HASHCODE_Group);
  }
  static CXFA_Node* GetParent(CXFA_Node* pDDGroupNode) {
    return pDDGroupNode->GetNodeItem(XFA_NODEITEM_Parent);
  }
};

struct RecurseRecord {
  CXFA_Node* pTemplateChild;
  CXFA_Node* pDataChild;
};

CXFA_Node* FormValueNode_CreateChild(CXFA_Node* pValueNode, XFA_Element iType) {
  CXFA_Node* pChildNode = pValueNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  if (!pChildNode) {
    if (iType == XFA_Element::Unknown)
      return nullptr;
    pChildNode = pValueNode->JSNode()->GetProperty(0, iType, true);
  }
  return pChildNode;
}

void FormValueNode_MatchNoneCreateChild(CXFA_Node* pFormNode) {
  CXFA_WidgetData* pWidgetData = pFormNode->GetWidgetData();
  ASSERT(pWidgetData);
  pWidgetData->GetUIType();
}

bool FormValueNode_SetChildContent(CXFA_Node* pValueNode,
                                   const WideString& wsContent,
                                   XFA_Element iType = XFA_Element::Unknown) {
  if (!pValueNode)
    return false;

  ASSERT(pValueNode->GetPacketType() == XFA_PacketType::Form);
  CXFA_Node* pChildNode = FormValueNode_CreateChild(pValueNode, iType);
  if (!pChildNode)
    return false;

  switch (pChildNode->GetObjectType()) {
    case XFA_ObjectType::ContentNode: {
      CXFA_Node* pContentRawDataNode =
          pChildNode->GetNodeItem(XFA_NODEITEM_FirstChild);
      if (!pContentRawDataNode) {
        XFA_Element element = XFA_Element::Sharptext;
        if (pChildNode->GetElementType() == XFA_Element::ExData) {
          pdfium::Optional<WideString> contentType =
              pChildNode->JSNode()->TryAttribute(XFA_Attribute::ContentType,
                                                 false);
          if (contentType) {
            if (*contentType == L"text/html")
              element = XFA_Element::SharpxHTML;
            else if (*contentType == L"text/xml")
              element = XFA_Element::Sharpxml;
          }
        }
        pContentRawDataNode = pChildNode->CreateSamePacketNode(element);
        pChildNode->InsertChild(pContentRawDataNode, nullptr);
      }
      pContentRawDataNode->JSNode()->SetCData(XFA_Attribute::Value, wsContent,
                                              false, false);
      break;
    }
    case XFA_ObjectType::NodeC:
    case XFA_ObjectType::TextNode:
    case XFA_ObjectType::NodeV: {
      pChildNode->JSNode()->SetCData(XFA_Attribute::Value, wsContent, false,
                                     false);
      break;
    }
    default:
      NOTREACHED();
      break;
  }
  return true;
}

void CreateDataBinding(CXFA_Node* pFormNode,
                       CXFA_Node* pDataNode,
                       bool bDataToForm) {
  pFormNode->JSNode()->SetBindingNode(pDataNode);
  pDataNode->AddBindItem(pFormNode);
  XFA_Element eType = pFormNode->GetElementType();
  if (eType != XFA_Element::Field && eType != XFA_Element::ExclGroup)
    return;

  CXFA_WidgetData* pWidgetData = pFormNode->GetWidgetData();
  ASSERT(pWidgetData);
  XFA_Element eUIType = pWidgetData->GetUIType();
  CXFA_ValueData defValueData(
      pFormNode->JSNode()->GetProperty(0, XFA_Element::Value, true));
  if (!bDataToForm) {
    WideString wsValue;
    switch (eUIType) {
      case XFA_Element::ImageEdit: {
        CXFA_ImageData imageData = defValueData.GetImageData();
        WideString wsContentType;
        WideString wsHref;
        if (imageData.HasValidNode()) {
          wsValue = imageData.GetContent();
          wsContentType = imageData.GetContentType();
          wsHref = imageData.GetHref();
        }
        CFX_XMLElement* pXMLDataElement =
            static_cast<CFX_XMLElement*>(pDataNode->GetXMLMappingNode());
        ASSERT(pXMLDataElement);

        pDataNode->JSNode()->SetAttributeValue(
            wsValue, pWidgetData->GetFormatDataValue(wsValue), false, false);
        pDataNode->JSNode()->SetCData(XFA_Attribute::ContentType, wsContentType,
                                      false, false);
        if (!wsHref.IsEmpty())
          pXMLDataElement->SetString(L"href", wsHref);

        break;
      }
      case XFA_Element::ChoiceList:
        wsValue = defValueData.GetChildValueContent();
        if (pWidgetData->IsChoiceListMultiSelect()) {
          std::vector<WideString> wsSelTextArray =
              pWidgetData->GetSelectedItemsValue();
          if (!wsSelTextArray.empty()) {
            for (const auto& text : wsSelTextArray) {
              CXFA_Node* pValue =
                  pDataNode->CreateSamePacketNode(XFA_Element::DataValue);
              pValue->JSNode()->SetCData(XFA_Attribute::Name, L"value", false,
                                         false);
              pValue->CreateXMLMappingNode();
              pDataNode->InsertChild(pValue, nullptr);
              pValue->JSNode()->SetCData(XFA_Attribute::Value, text, false,
                                         false);
            }
          } else {
            CFX_XMLNode* pXMLNode = pDataNode->GetXMLMappingNode();
            ASSERT(pXMLNode->GetType() == FX_XMLNODE_Element);
            static_cast<CFX_XMLElement*>(pXMLNode)->SetString(L"xfa:dataNode",
                                                              L"dataGroup");
          }
        } else if (!wsValue.IsEmpty()) {
          pDataNode->JSNode()->SetAttributeValue(
              wsValue, pWidgetData->GetFormatDataValue(wsValue), false, false);
        }
        break;
      case XFA_Element::CheckButton:
        wsValue = defValueData.GetChildValueContent();
        if (wsValue.IsEmpty())
          break;

        pDataNode->JSNode()->SetAttributeValue(
            wsValue, pWidgetData->GetFormatDataValue(wsValue), false, false);
        break;
      case XFA_Element::ExclGroup: {
        CXFA_Node* pChecked = nullptr;
        CXFA_Node* pChild = pFormNode->GetNodeItem(XFA_NODEITEM_FirstChild);
        for (; pChild; pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
          if (pChild->GetElementType() != XFA_Element::Field)
            continue;

          CXFA_Node* pValue = pChild->GetChild(0, XFA_Element::Value, false);
          if (!pValue)
            continue;

          wsValue = CXFA_ValueData(pValue).GetChildValueContent();
          if (wsValue.IsEmpty())
            continue;

          CXFA_Node* pItems = pChild->GetChild(0, XFA_Element::Items, false);
          if (!pItems)
            continue;

          CXFA_Node* pText = pItems->GetNodeItem(XFA_NODEITEM_FirstChild);
          if (!pText)
            continue;

          WideString wsContent = pText->JSNode()->GetContent(false);
          if (wsContent == wsValue) {
            pChecked = pChild;
            pDataNode->JSNode()->SetAttributeValue(wsValue, wsValue, false,
                                                   false);
            pFormNode->JSNode()->SetCData(XFA_Attribute::Value, wsContent,
                                          false, false);
            break;
          }
        }
        if (!pChecked)
          break;

        pChild = pFormNode->GetNodeItem(XFA_NODEITEM_FirstChild);
        for (; pChild; pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
          if (pChild == pChecked)
            continue;
          if (pChild->GetElementType() != XFA_Element::Field)
            continue;

          CXFA_Node* pValue =
              pChild->JSNode()->GetProperty(0, XFA_Element::Value, true);
          CXFA_Node* pItems = pChild->GetChild(0, XFA_Element::Items, false);
          CXFA_Node* pText =
              pItems ? pItems->GetNodeItem(XFA_NODEITEM_FirstChild) : nullptr;
          if (pText)
            pText = pText->GetNodeItem(XFA_NODEITEM_NextSibling);

          WideString wsContent;
          if (pText)
            wsContent = pText->JSNode()->GetContent(false);

          FormValueNode_SetChildContent(pValue, wsContent, XFA_Element::Text);
        }
        break;
      }
      case XFA_Element::NumericEdit: {
        wsValue = defValueData.GetChildValueContent();
        if (wsValue.IsEmpty())
          break;

        wsValue = pWidgetData->NormalizeNumStr(wsValue);
        pDataNode->JSNode()->SetAttributeValue(
            wsValue, pWidgetData->GetFormatDataValue(wsValue), false, false);
        CXFA_Node* pValue =
            pFormNode->JSNode()->GetProperty(0, XFA_Element::Value, true);
        FormValueNode_SetChildContent(pValue, wsValue, XFA_Element::Float);
        break;
      }
      default:
        wsValue = defValueData.GetChildValueContent();
        if (wsValue.IsEmpty())
          break;

        pDataNode->JSNode()->SetAttributeValue(
            wsValue, pWidgetData->GetFormatDataValue(wsValue), false, false);
        break;
    }
    return;
  }

  WideString wsXMLValue = pDataNode->JSNode()->GetContent(false);
  WideString wsNormalizeValue = pWidgetData->GetNormalizeDataValue(wsXMLValue);

  pDataNode->JSNode()->SetAttributeValue(wsNormalizeValue, wsXMLValue, false,
                                         false);
  switch (eUIType) {
    case XFA_Element::ImageEdit: {
      FormValueNode_SetChildContent(defValueData.GetNode(), wsNormalizeValue,
                                    XFA_Element::Image);
      CXFA_ImageData imageData = defValueData.GetImageData();
      if (imageData.HasValidNode()) {
        CFX_XMLElement* pXMLDataElement =
            static_cast<CFX_XMLElement*>(pDataNode->GetXMLMappingNode());
        ASSERT(pXMLDataElement);

        WideString wsContentType =
            pXMLDataElement->GetString(L"xfa:contentType");
        if (!wsContentType.IsEmpty()) {
          pDataNode->JSNode()->SetCData(XFA_Attribute::ContentType,
                                        wsContentType, false, false);
          imageData.SetContentType(wsContentType);
        }

        WideString wsHref = pXMLDataElement->GetString(L"href");
        if (!wsHref.IsEmpty())
          imageData.SetHref(wsHref);
      }
      break;
    }
    case XFA_Element::ChoiceList:
      if (pWidgetData->IsChoiceListMultiSelect()) {
        std::vector<CXFA_Node*> items = pDataNode->GetNodeList(
            XFA_NODEFILTER_Children | XFA_NODEFILTER_Properties,
            XFA_Element::Unknown);
        if (!items.empty()) {
          bool single = items.size() == 1;
          wsNormalizeValue.clear();

          for (CXFA_Node* pNode : items) {
            WideString wsItem = pNode->JSNode()->GetContent(false);
            if (single)
              wsItem += L"\n";

            wsNormalizeValue += wsItem;
          }
          CXFA_ExDataData exData = defValueData.GetExData();
          ASSERT(exData.HasValidNode());

          exData.SetContentType(single ? L"text/plain" : L"text/xml");
        }
        FormValueNode_SetChildContent(defValueData.GetNode(), wsNormalizeValue,
                                      XFA_Element::ExData);
      } else {
        FormValueNode_SetChildContent(defValueData.GetNode(), wsNormalizeValue,
                                      XFA_Element::Text);
      }
      break;
    case XFA_Element::CheckButton:
      FormValueNode_SetChildContent(defValueData.GetNode(), wsNormalizeValue,
                                    XFA_Element::Text);
      break;
    case XFA_Element::ExclGroup: {
      pWidgetData->SetSelectedMemberByValue(wsNormalizeValue.AsStringView(),
                                            false, false, false);
      break;
    }
    case XFA_Element::DateTimeEdit:
      FormValueNode_SetChildContent(defValueData.GetNode(), wsNormalizeValue,
                                    XFA_Element::DateTime);
      break;
    case XFA_Element::NumericEdit: {
      WideString wsPicture =
          pWidgetData->GetPictureContent(XFA_VALUEPICTURE_DataBind);
      if (wsPicture.IsEmpty())
        wsNormalizeValue = pWidgetData->NormalizeNumStr(wsNormalizeValue);

      FormValueNode_SetChildContent(defValueData.GetNode(), wsNormalizeValue,
                                    XFA_Element::Float);
      break;
    }
    case XFA_Element::Barcode:
    case XFA_Element::Button:
    case XFA_Element::PasswordEdit:
    case XFA_Element::Signature:
    case XFA_Element::TextEdit:
    default:
      FormValueNode_SetChildContent(defValueData.GetNode(), wsNormalizeValue,
                                    XFA_Element::Text);
      break;
  }
}

CXFA_Node* GetGlobalBinding(CXFA_Document* pDocument, uint32_t dwNameHash) {
  auto it = pDocument->m_rgGlobalBinding.find(dwNameHash);
  return it != pDocument->m_rgGlobalBinding.end() ? it->second : nullptr;
}

void RegisterGlobalBinding(CXFA_Document* pDocument,
                           uint32_t dwNameHash,
                           CXFA_Node* pDataNode) {
  pDocument->m_rgGlobalBinding[dwNameHash] = pDataNode;
}

CXFA_Node* ScopeMatchGlobalBinding(CXFA_Node* pDataScope,
                                   uint32_t dwNameHash,
                                   XFA_Element eMatchDataNodeType,
                                   bool bUpLevel) {
  for (CXFA_Node *pCurDataScope = pDataScope, *pLastDataScope = nullptr;
       pCurDataScope &&
       pCurDataScope->GetPacketType() == XFA_PacketType::Datasets;
       pLastDataScope = pCurDataScope,
                 pCurDataScope =
                     pCurDataScope->GetNodeItem(XFA_NODEITEM_Parent)) {
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

    for (CXFA_Node* pDataChild =
             pCurDataScope->GetFirstChildByClass(XFA_Element::DataGroup);
         pDataChild; pDataChild = pDataChild->GetNextSameClassSibling(
                         XFA_Element::DataGroup)) {
      CXFA_Node* pDataNode = ScopeMatchGlobalBinding(pDataChild, dwNameHash,
                                                     eMatchDataNodeType, false);
      if (pDataNode)
        return pDataNode;
    }
    if (!bUpLevel)
      break;
  }
  return nullptr;
}

CXFA_Node* FindGlobalDataNode(CXFA_Document* pDocument,
                              const WideString& wsName,
                              CXFA_Node* pDataScope,
                              XFA_Element eMatchNodeType) {
  if (wsName.IsEmpty())
    return nullptr;

  uint32_t dwNameHash = FX_HashCode_GetW(wsName.AsStringView(), false);
  CXFA_Node* pBounded = GetGlobalBinding(pDocument, dwNameHash);
  if (!pBounded) {
    pBounded =
        ScopeMatchGlobalBinding(pDataScope, dwNameHash, eMatchNodeType, true);
    if (pBounded)
      RegisterGlobalBinding(pDocument, dwNameHash, pBounded);
  }
  return pBounded;
}

CXFA_Node* FindOnceDataNode(CXFA_Document* pDocument,
                            const WideString& wsName,
                            CXFA_Node* pDataScope,
                            XFA_Element eMatchNodeType) {
  if (wsName.IsEmpty())
    return nullptr;

  uint32_t dwNameHash = FX_HashCode_GetW(wsName.AsStringView(), false);
  CXFA_Node* pLastDataScope = nullptr;
  for (CXFA_Node* pCurDataScope = pDataScope;
       pCurDataScope &&
       pCurDataScope->GetPacketType() == XFA_PacketType::Datasets;
       pCurDataScope = pCurDataScope->GetNodeItem(XFA_NODEITEM_Parent)) {
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

CXFA_Node* FindDataRefDataNode(CXFA_Document* pDocument,
                               const WideString& wsRef,
                               CXFA_Node* pDataScope,
                               XFA_Element eMatchNodeType,
                               CXFA_Node* pTemplateNode,
                               bool bForceBind,
                               bool bUpLevel) {
  uint32_t dFlags = XFA_RESOLVENODE_Children | XFA_RESOLVENODE_BindNew;
  if (bUpLevel || wsRef != L"name")
    dFlags |= (XFA_RESOLVENODE_Parent | XFA_RESOLVENODE_Siblings);

  XFA_RESOLVENODE_RS rs;
  pDocument->GetScriptContext()->ResolveObjects(
      pDataScope, wsRef.AsStringView(), rs, dFlags, pTemplateNode);
  if (rs.dwFlags == XFA_RESOLVENODE_RSTYPE_CreateNodeAll ||
      rs.dwFlags == XFA_RESOLVENODE_RSTYPE_CreateNodeMidAll ||
      rs.objects.size() > 1) {
    return pDocument->GetNotBindNode(rs.objects);
  }

  if (rs.dwFlags == XFA_RESOLVENODE_RSTYPE_CreateNodeOne) {
    CXFA_Object* pObject = !rs.objects.empty() ? rs.objects.front() : nullptr;
    CXFA_Node* pNode = ToNode(pObject);
    return (bForceBind || !pNode || !pNode->HasBindItem()) ? pNode : nullptr;
  }
  return nullptr;
}

bool NeedGenerateForm(CXFA_Node* pTemplateChild, bool bUseInstanceManager) {
  XFA_Element eType = pTemplateChild->GetElementType();
  if (eType == XFA_Element::Variables)
    return true;
  if (pTemplateChild->IsContainerNode())
    return false;
  if (eType == XFA_Element::Proto ||
      (bUseInstanceManager && eType == XFA_Element::Occur)) {
    return false;
  }
  return true;
}

CXFA_Node* CloneOrMergeInstanceManager(CXFA_Document* pDocument,
                                       CXFA_Node* pFormParent,
                                       CXFA_Node* pTemplateNode,
                                       std::vector<CXFA_Node*>* subforms) {
  WideString wsSubformName =
      pTemplateNode->JSNode()->GetCData(XFA_Attribute::Name);
  WideString wsInstMgrNodeName = L"_" + wsSubformName;
  uint32_t dwInstNameHash =
      FX_HashCode_GetW(wsInstMgrNodeName.AsStringView(), false);
  CXFA_Node* pExistingNode = XFA_DataMerge_FindFormDOMInstance(
      pDocument, XFA_Element::InstanceManager, dwInstNameHash, pFormParent);
  if (pExistingNode) {
    uint32_t dwNameHash = pTemplateNode->GetNameHash();
    for (CXFA_Node* pNode =
             pExistingNode->GetNodeItem(XFA_NODEITEM_NextSibling);
         pNode;) {
      XFA_Element eCurType = pNode->GetElementType();
      if (eCurType == XFA_Element::InstanceManager)
        break;

      if ((eCurType != XFA_Element::Subform) &&
          (eCurType != XFA_Element::SubformSet)) {
        pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling);
        continue;
      }
      if (dwNameHash != pNode->GetNameHash())
        break;

      CXFA_Node* pNextNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling);
      pFormParent->RemoveChild(pNode, true);
      subforms->push_back(pNode);
      pNode = pNextNode;
    }
    pFormParent->RemoveChild(pExistingNode, true);
    pFormParent->InsertChild(pExistingNode, nullptr);
    pExistingNode->ClearFlag(XFA_NodeFlag_UnusedNode);
    pExistingNode->SetTemplateNode(pTemplateNode);
    return pExistingNode;
  }

  CXFA_Node* pNewNode =
      pDocument->CreateNode(XFA_PacketType::Form, XFA_Element::InstanceManager);
  wsInstMgrNodeName =
      L"_" + pTemplateNode->JSNode()->GetCData(XFA_Attribute::Name);
  pNewNode->JSNode()->SetCData(XFA_Attribute::Name, wsInstMgrNodeName, false,
                               false);
  pFormParent->InsertChild(pNewNode, nullptr);
  pNewNode->SetTemplateNode(pTemplateNode);
  return pNewNode;
}

CXFA_Node* FindMatchingDataNode(
    CXFA_Document* pDocument,
    CXFA_Node* pTemplateNode,
    CXFA_Node* pDataScope,
    bool& bAccessedDataDOM,
    bool bForceBind,
    CXFA_NodeIteratorTemplate<CXFA_Node,
                              CXFA_TraverseStrategy_XFAContainerNode>*
        pIterator,
    bool& bSelfMatch,
    XFA_AttributeEnum& eBindMatch,
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

    CXFA_Node* pTemplateNodeOccur =
        pCurTemplateNode->GetFirstChildByClass(XFA_Element::Occur);
    if (pTemplateNodeOccur) {
      int32_t iMin;
      int32_t iMax;
      int32_t iInit;
      std::tie(iMin, iMax, iInit) =
          CXFA_OccurData(pTemplateNodeOccur).GetOccurInfo();
      if (iMax == 0) {
        pCurTemplateNode = pIterator->MoveToNext();
        continue;
      }
    }

    CXFA_Node* pTemplateNodeBind =
        pCurTemplateNode->GetFirstChildByClass(XFA_Element::Bind);
    XFA_AttributeEnum eMatch =
        pTemplateNodeBind
            ? pTemplateNodeBind->JSNode()->GetEnum(XFA_Attribute::Match)
            : XFA_AttributeEnum::Once;
    eBindMatch = eMatch;
    switch (eMatch) {
      case XFA_AttributeEnum::None:
        pCurTemplateNode = pIterator->MoveToNext();
        continue;
      case XFA_AttributeEnum::Global:
        bAccessedDataDOM = true;
        if (!bForceBind) {
          pCurTemplateNode = pIterator->MoveToNext();
          continue;
        }
        if (eMatchNodeType == XFA_Element::DataValue ||
            (eMatchNodeType == XFA_Element::DataGroup &&
             XFA_FieldIsMultiListBox(pTemplateNodeBind))) {
          CXFA_Node* pGlobalBindNode = FindGlobalDataNode(
              pDocument,
              pCurTemplateNode->JSNode()->GetCData(XFA_Attribute::Name),
              pDataScope, eMatchNodeType);
          if (!pGlobalBindNode) {
            pCurTemplateNode = pIterator->MoveToNext();
            continue;
          }
          pResult = pGlobalBindNode;
          break;
        }
      case XFA_AttributeEnum::Once: {
        bAccessedDataDOM = true;
        CXFA_Node* pOnceBindNode = FindOnceDataNode(
            pDocument,
            pCurTemplateNode->JSNode()->GetCData(XFA_Attribute::Name),
            pDataScope, eMatchNodeType);
        if (!pOnceBindNode) {
          pCurTemplateNode = pIterator->MoveToNext();
          continue;
        }
        pResult = pOnceBindNode;
        break;
      }
      case XFA_AttributeEnum::DataRef: {
        bAccessedDataDOM = true;
        CXFA_Node* pDataRefBindNode = FindDataRefDataNode(
            pDocument,
            pTemplateNodeBind->JSNode()->GetCData(XFA_Attribute::Ref),
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
    if (pCurTemplateNode == pTemplateNode && pResult)
      bSelfMatch = true;
    break;
  }
  return pResult;
}

void SortRecurseRecord(std::vector<RecurseRecord>* rgRecords,
                       CXFA_Node* pDataScope,
                       bool bChoiceMode) {
  std::vector<RecurseRecord> rgResultRecord;
  for (CXFA_Node* pNode = pDataScope->GetNodeItem(XFA_NODEITEM_FirstChild);
       pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    auto it = std::find_if(rgRecords->begin(), rgRecords->end(),
                           [pNode](const RecurseRecord& record) {
                             return pNode == record.pDataChild;
                           });
    if (it != rgRecords->end()) {
      rgResultRecord.push_back(*it);
      rgRecords->erase(it);
      if (bChoiceMode)
        break;
    }
  }
  if (rgResultRecord.empty())
    return;

  if (!bChoiceMode) {
    rgResultRecord.insert(rgResultRecord.end(), rgRecords->begin(),
                          rgRecords->end());
  }
  *rgRecords = rgResultRecord;
}

CXFA_Node* CopyContainer_SubformSet(CXFA_Document* pDocument,
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
                                             pDocument, pFormParentNode,
                                             pTemplateNode, &subformArray)
                                       : nullptr;
    if (CXFA_Node* pOccurTemplateNode =
            pTemplateNode->GetFirstChildByClass(XFA_Element::Occur)) {
      pOccurNode = pInstMgrNode ? XFA_NodeMerge_CloneOrMergeContainer(
                                      pDocument, pInstMgrNode,
                                      pOccurTemplateNode, false, nullptr)
                                : pOccurTemplateNode;
    } else if (pInstMgrNode) {
      pOccurNode = pInstMgrNode->GetFirstChildByClass(XFA_Element::Occur);
      if (pOccurNode)
        pOccurNode->ClearFlag(XFA_NodeFlag_UnusedNode);
    }
    if (pInstMgrNode) {
      pInstMgrNode->SetFlag(XFA_NodeFlag_Initialized, true);
      pSearchArray = &subformArray;
      if (pFormParentNode->GetElementType() == XFA_Element::PageArea) {
        bOneInstance = true;
        if (subformArray.empty())
          pSearchArray = nullptr;
      } else if (pTemplateNode->GetNameHash() == 0 && subformArray.empty()) {
        pSearchArray = nullptr;
      }
    }
  }

  int32_t iMax = 1;
  int32_t iInit = 1;
  int32_t iMin = 1;
  if (!bOneInstance && pOccurNode)
    std::tie(iMin, iMax, iInit) = CXFA_OccurData(pOccurNode).GetOccurInfo();

  XFA_AttributeEnum eRelation =
      eType == XFA_Element::SubformSet
          ? pTemplateNode->JSNode()->GetEnum(XFA_Attribute::Relation)
          : XFA_AttributeEnum::Ordered;
  int32_t iCurRepeatIndex = 0;
  XFA_AttributeEnum eParentBindMatch = XFA_AttributeEnum::None;
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
        XFA_AttributeEnum eBindMatch = XFA_AttributeEnum::None;
        CXFA_Node* pDataNode = FindMatchingDataNode(
            pDocument, pTemplateNode, pDataScope, bAccessedDataDOM, false,
            &sNodeIterator, bSelfMatch, eBindMatch, true);
        if (!pDataNode || sNodeIterator.GetCurrent() != pTemplateNode)
          break;

        eParentBindMatch = eBindMatch;
        CXFA_Node* pSubformNode = XFA_NodeMerge_CloneOrMergeContainer(
            pDocument, pFormParentNode, pTemplateNode, false, pSearchArray);
        if (!pFirstInstance)
          pFirstInstance = pSubformNode;

        CreateDataBinding(pSubformNode, pDataNode, true);
        ASSERT(pSubformNode);
        subformMapArray[pSubformNode] = pDataNode;
        nodeArray.push_back(pSubformNode);
      }

      for (CXFA_Node* pSubform : nodeArray) {
        CXFA_Node* pDataNode = nullptr;
        auto it = subformMapArray.find(pSubform);
        if (it != subformMapArray.end())
          pDataNode = it->second;
        for (CXFA_Node* pTemplateChild =
                 pTemplateNode->GetNodeItem(XFA_NODEITEM_FirstChild);
             pTemplateChild; pTemplateChild = pTemplateChild->GetNodeItem(
                                 XFA_NODEITEM_NextSibling)) {
          if (NeedGenerateForm(pTemplateChild, bUseInstanceManager)) {
            XFA_NodeMerge_CloneOrMergeContainer(pDocument, pSubform,
                                                pTemplateChild, true, nullptr);
          } else if (pTemplateChild->IsContainerNode()) {
            pDocument->DataMerge_CopyContainer(pTemplateChild, pSubform,
                                               pDataNode, false, true, false);
          }
        }
      }
      subformMapArray.clear();
    }

    for (; iMax < 0 || iCurRepeatIndex < iMax; iCurRepeatIndex++) {
      bool bSelfMatch = false;
      XFA_AttributeEnum eBindMatch = XFA_AttributeEnum::None;
      if (!FindMatchingDataNode(pDocument, pTemplateNode, pDataScope,
                                bAccessedDataDOM, false, &sNodeIterator,
                                bSelfMatch, eBindMatch, true)) {
        break;
      }
      if (eBindMatch == XFA_AttributeEnum::DataRef &&
          eParentBindMatch == XFA_AttributeEnum::DataRef) {
        break;
      }

      if (eRelation == XFA_AttributeEnum::Choice ||
          eRelation == XFA_AttributeEnum::Unordered) {
        CXFA_Node* pSubformSetNode = XFA_NodeMerge_CloneOrMergeContainer(
            pDocument, pFormParentNode, pTemplateNode, false, pSearchArray);
        ASSERT(pSubformSetNode);
        if (!pFirstInstance)
          pFirstInstance = pSubformSetNode;

        std::vector<RecurseRecord> rgItemMatchList;
        std::vector<CXFA_Node*> rgItemUnmatchList;
        for (CXFA_Node* pTemplateChild =
                 pTemplateNode->GetNodeItem(XFA_NODEITEM_FirstChild);
             pTemplateChild; pTemplateChild = pTemplateChild->GetNodeItem(
                                 XFA_NODEITEM_NextSibling)) {
          if (NeedGenerateForm(pTemplateChild, bUseInstanceManager)) {
            XFA_NodeMerge_CloneOrMergeContainer(pDocument, pSubformSetNode,
                                                pTemplateChild, true, nullptr);
          } else if (pTemplateChild->IsContainerNode()) {
            bSelfMatch = false;
            eBindMatch = XFA_AttributeEnum::None;
            if (eRelation != XFA_AttributeEnum::Ordered) {
              CXFA_NodeIteratorTemplate<CXFA_Node,
                                        CXFA_TraverseStrategy_XFAContainerNode>
                  sChildIter(pTemplateChild);
              CXFA_Node* pDataMatch = FindMatchingDataNode(
                  pDocument, pTemplateChild, pDataScope, bAccessedDataDOM,
                  false, &sChildIter, bSelfMatch, eBindMatch, true);
              if (pDataMatch) {
                RecurseRecord sNewRecord = {pTemplateChild, pDataMatch};
                if (bSelfMatch)
                  rgItemMatchList.insert(rgItemMatchList.begin(), sNewRecord);
                else
                  rgItemMatchList.push_back(sNewRecord);
              } else {
                rgItemUnmatchList.push_back(pTemplateChild);
              }
            } else {
              rgItemUnmatchList.push_back(pTemplateChild);
            }
          }
        }

        switch (eRelation) {
          case XFA_AttributeEnum::Choice: {
            ASSERT(!rgItemMatchList.empty());
            SortRecurseRecord(&rgItemMatchList, pDataScope, true);
            pDocument->DataMerge_CopyContainer(
                rgItemMatchList.front().pTemplateChild, pSubformSetNode,
                pDataScope, false, true, true);
            break;
          }
          case XFA_AttributeEnum::Unordered: {
            if (!rgItemMatchList.empty()) {
              SortRecurseRecord(&rgItemMatchList, pDataScope, false);
              for (const auto& matched : rgItemMatchList) {
                pDocument->DataMerge_CopyContainer(matched.pTemplateChild,
                                                   pSubformSetNode, pDataScope,
                                                   false, true, true);
              }
            }
            for (auto* unmatched : rgItemUnmatchList) {
              pDocument->DataMerge_CopyContainer(unmatched, pSubformSetNode,
                                                 pDataScope, false, true, true);
            }
            break;
          }
          default:
            break;
        }
      } else {
        CXFA_Node* pSubformSetNode = XFA_NodeMerge_CloneOrMergeContainer(
            pDocument, pFormParentNode, pTemplateNode, false, pSearchArray);
        ASSERT(pSubformSetNode);
        if (!pFirstInstance)
          pFirstInstance = pSubformSetNode;

        for (CXFA_Node* pTemplateChild =
                 pTemplateNode->GetNodeItem(XFA_NODEITEM_FirstChild);
             pTemplateChild; pTemplateChild = pTemplateChild->GetNodeItem(
                                 XFA_NODEITEM_NextSibling)) {
          if (NeedGenerateForm(pTemplateChild, bUseInstanceManager)) {
            XFA_NodeMerge_CloneOrMergeContainer(pDocument, pSubformSetNode,
                                                pTemplateChild, true, nullptr);
          } else if (pTemplateChild->IsContainerNode()) {
            pDocument->DataMerge_CopyContainer(pTemplateChild, pSubformSetNode,
                                               pDataScope, false, true, true);
          }
        }
      }
    }

    if (iCurRepeatIndex == 0 && bAccessedDataDOM == false) {
      int32_t iLimit = iMax;
      if (pInstMgrNode && pTemplateNode->GetNameHash() == 0) {
        iLimit = pdfium::CollectionSize<int32_t>(subformArray);
        if (iLimit < iMin)
          iLimit = iInit;
      }

      for (; (iLimit < 0 || iCurRepeatIndex < iLimit); iCurRepeatIndex++) {
        if (pInstMgrNode) {
          if (pSearchArray && pSearchArray->empty()) {
            if (pTemplateNode->GetNameHash() != 0)
              break;
            pSearchArray = nullptr;
          }
        } else if (!XFA_DataMerge_FindFormDOMInstance(
                       pDocument, pTemplateNode->GetElementType(),
                       pTemplateNode->GetNameHash(), pFormParentNode)) {
          break;
        }
        CXFA_Node* pSubformNode = XFA_NodeMerge_CloneOrMergeContainer(
            pDocument, pFormParentNode, pTemplateNode, false, pSearchArray);
        ASSERT(pSubformNode);
        if (!pFirstInstance)
          pFirstInstance = pSubformNode;

        for (CXFA_Node* pTemplateChild =
                 pTemplateNode->GetNodeItem(XFA_NODEITEM_FirstChild);
             pTemplateChild; pTemplateChild = pTemplateChild->GetNodeItem(
                                 XFA_NODEITEM_NextSibling)) {
          if (NeedGenerateForm(pTemplateChild, bUseInstanceManager)) {
            XFA_NodeMerge_CloneOrMergeContainer(pDocument, pSubformNode,
                                                pTemplateChild, true, nullptr);
          } else if (pTemplateChild->IsContainerNode()) {
            pDocument->DataMerge_CopyContainer(pTemplateChild, pSubformNode,
                                               pDataScope, false, true, true);
          }
        }
      }
    }
  }

  int32_t iMinimalLimit = iCurRepeatIndex == 0 ? iInit : iMin;
  for (; iCurRepeatIndex < iMinimalLimit; iCurRepeatIndex++) {
    CXFA_Node* pSubformSetNode = XFA_NodeMerge_CloneOrMergeContainer(
        pDocument, pFormParentNode, pTemplateNode, false, pSearchArray);
    ASSERT(pSubformSetNode);
    if (!pFirstInstance)
      pFirstInstance = pSubformSetNode;

    bool bFound = false;
    for (CXFA_Node* pTemplateChild =
             pTemplateNode->GetNodeItem(XFA_NODEITEM_FirstChild);
         pTemplateChild; pTemplateChild = pTemplateChild->GetNodeItem(
                             XFA_NODEITEM_NextSibling)) {
      if (NeedGenerateForm(pTemplateChild, bUseInstanceManager)) {
        XFA_NodeMerge_CloneOrMergeContainer(pDocument, pSubformSetNode,
                                            pTemplateChild, true, nullptr);
      } else if (pTemplateChild->IsContainerNode()) {
        if (bFound && eRelation == XFA_AttributeEnum::Choice)
          continue;

        pDocument->DataMerge_CopyContainer(pTemplateChild, pSubformSetNode,
                                           pDataScope, false, bDataMerge, true);
        bFound = true;
      }
    }
  }
  return pFirstInstance;
}

CXFA_Node* CopyContainer_Field(CXFA_Document* pDocument,
                               CXFA_Node* pTemplateNode,
                               CXFA_Node* pFormNode,
                               CXFA_Node* pDataScope,
                               bool bDataMerge,
                               bool bUpLevel) {
  CXFA_Node* pFieldNode = XFA_NodeMerge_CloneOrMergeContainer(
      pDocument, pFormNode, pTemplateNode, false, nullptr);
  ASSERT(pFieldNode);
  for (CXFA_Node* pTemplateChildNode =
           pTemplateNode->GetNodeItem(XFA_NODEITEM_FirstChild);
       pTemplateChildNode; pTemplateChildNode = pTemplateChildNode->GetNodeItem(
                               XFA_NODEITEM_NextSibling)) {
    if (NeedGenerateForm(pTemplateChildNode, true)) {
      XFA_NodeMerge_CloneOrMergeContainer(pDocument, pFieldNode,
                                          pTemplateChildNode, true, nullptr);
    } else if (pTemplateNode->GetElementType() == XFA_Element::ExclGroup &&
               pTemplateChildNode->IsContainerNode()) {
      if (pTemplateChildNode->GetElementType() == XFA_Element::Field) {
        CopyContainer_Field(pDocument, pTemplateChildNode, pFieldNode, nullptr,
                            false, true);
      }
    }
  }
  if (bDataMerge) {
    bool bAccessedDataDOM = false;
    bool bSelfMatch = false;
    XFA_AttributeEnum eBindMatch;
    CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_XFAContainerNode>
        sNodeIter(pTemplateNode);
    CXFA_Node* pDataNode = FindMatchingDataNode(
        pDocument, pTemplateNode, pDataScope, bAccessedDataDOM, true,
        &sNodeIter, bSelfMatch, eBindMatch, bUpLevel);
    if (pDataNode)
      CreateDataBinding(pFieldNode, pDataNode, true);
  } else {
    FormValueNode_MatchNoneCreateChild(pFieldNode);
  }
  return pFieldNode;
}

CXFA_Node* MaybeCreateDataNode(CXFA_Document* pDocument,
                               CXFA_Node* pDataParent,
                               XFA_Element eNodeType,
                               const WideString& wsName) {
  if (!pDataParent)
    return nullptr;

  CXFA_Node* pParentDDNode = pDataParent->GetDataDescriptionNode();
  if (!pParentDDNode) {
    CXFA_Node* pDataNode =
        pDocument->CreateNode(XFA_PacketType::Datasets, eNodeType);
    pDataNode->JSNode()->SetCData(XFA_Attribute::Name, wsName, false, false);
    pDataNode->CreateXMLMappingNode();
    pDataParent->InsertChild(pDataNode, nullptr);
    pDataNode->SetFlag(XFA_NodeFlag_Initialized, false);
    return pDataNode;
  }

  CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_DDGroup> sIterator(
      pParentDDNode);
  for (CXFA_Node* pDDGroupNode = sIterator.GetCurrent(); pDDGroupNode;
       pDDGroupNode = sIterator.MoveToNext()) {
    if (pDDGroupNode != pParentDDNode) {
      if (pDDGroupNode->GetElementType() != XFA_Element::DataGroup)
        continue;

      pdfium::Optional<WideString> ns = pDDGroupNode->JSNode()->TryNamespace();
      if (!ns || *ns != L"http://ns.adobe.com/data-description/")
        continue;
    }

    CXFA_Node* pDDNode =
        pDDGroupNode->GetFirstChildByName(wsName.AsStringView());
    if (!pDDNode)
      continue;
    if (pDDNode->GetElementType() != eNodeType)
      break;

    CXFA_Node* pDataNode =
        pDocument->CreateNode(XFA_PacketType::Datasets, eNodeType);
    pDataNode->JSNode()->SetCData(XFA_Attribute::Name, wsName, false, false);
    pDataNode->CreateXMLMappingNode();
    if (eNodeType == XFA_Element::DataValue &&
        pDDNode->JSNode()->GetEnum(XFA_Attribute::Contains) ==
            XFA_AttributeEnum::MetaData) {
      pDataNode->JSNode()->SetEnum(XFA_Attribute::Contains,
                                   XFA_AttributeEnum::MetaData, false);
    }
    pDataParent->InsertChild(pDataNode, nullptr);
    pDataNode->SetDataDescriptionNode(pDDNode);
    pDataNode->SetFlag(XFA_NodeFlag_Initialized, false);
    return pDataNode;
  }
  return nullptr;
}

void UpdateBindingRelations(CXFA_Document* pDocument,
                            CXFA_Node* pFormNode,
                            CXFA_Node* pDataScope,
                            bool bDataRef,
                            bool bParentDataRef) {
  bool bMatchRef = true;
  XFA_Element eType = pFormNode->GetElementType();
  CXFA_Node* pDataNode = pFormNode->GetBindData();
  if (eType == XFA_Element::Subform || eType == XFA_Element::ExclGroup ||
      eType == XFA_Element::Field) {
    CXFA_Node* pTemplateNode = pFormNode->GetTemplateNode();
    CXFA_Node* pTemplateNodeBind =
        pTemplateNode ? pTemplateNode->GetFirstChildByClass(XFA_Element::Bind)
                      : nullptr;
    XFA_AttributeEnum eMatch =
        pTemplateNodeBind
            ? pTemplateNodeBind->JSNode()->GetEnum(XFA_Attribute::Match)
            : XFA_AttributeEnum::Once;
    switch (eMatch) {
      case XFA_AttributeEnum::None:
        if (!bDataRef || bParentDataRef)
          FormValueNode_MatchNoneCreateChild(pFormNode);
        break;
      case XFA_AttributeEnum::Once:
        if (!bDataRef || bParentDataRef) {
          if (!pDataNode) {
            if (pFormNode->GetNameHash() != 0 &&
                pFormNode->JSNode()->GetEnum(XFA_Attribute::Scope) !=
                    XFA_AttributeEnum::None) {
              XFA_Element eDataNodeType = (eType == XFA_Element::Subform ||
                                           XFA_FieldIsMultiListBox(pFormNode))
                                              ? XFA_Element::DataGroup
                                              : XFA_Element::DataValue;
              pDataNode = MaybeCreateDataNode(
                  pDocument, pDataScope, eDataNodeType,
                  WideString(
                      pFormNode->JSNode()->GetCData(XFA_Attribute::Name)));
              if (pDataNode)
                CreateDataBinding(pFormNode, pDataNode, false);
            }
            if (!pDataNode)
              FormValueNode_MatchNoneCreateChild(pFormNode);

          } else {
            CXFA_Node* pDataParent =
                pDataNode->GetNodeItem(XFA_NODEITEM_Parent);
            if (pDataParent != pDataScope) {
              ASSERT(pDataParent);
              pDataParent->RemoveChild(pDataNode, true);
              pDataScope->InsertChild(pDataNode, nullptr);
            }
          }
        }
        break;
      case XFA_AttributeEnum::Global:
        if (!bDataRef || bParentDataRef) {
          uint32_t dwNameHash = pFormNode->GetNameHash();
          if (dwNameHash != 0 && !pDataNode) {
            pDataNode = GetGlobalBinding(pDocument, dwNameHash);
            if (!pDataNode) {
              XFA_Element eDataNodeType = (eType == XFA_Element::Subform ||
                                           XFA_FieldIsMultiListBox(pFormNode))
                                              ? XFA_Element::DataGroup
                                              : XFA_Element::DataValue;
              CXFA_Node* pRecordNode =
                  ToNode(pDocument->GetXFAObject(XFA_HASHCODE_Record));
              pDataNode = MaybeCreateDataNode(
                  pDocument, pRecordNode, eDataNodeType,
                  WideString(
                      pFormNode->JSNode()->GetCData(XFA_Attribute::Name)));
              if (pDataNode) {
                CreateDataBinding(pFormNode, pDataNode, false);
                RegisterGlobalBinding(pDocument, pFormNode->GetNameHash(),
                                      pDataNode);
              }
            } else {
              CreateDataBinding(pFormNode, pDataNode, true);
            }
          }
          if (!pDataNode)
            FormValueNode_MatchNoneCreateChild(pFormNode);
        }
        break;
      case XFA_AttributeEnum::DataRef: {
        bMatchRef = bDataRef;
        bParentDataRef = true;
        if (!pDataNode && bDataRef) {
          WideString wsRef =
              pTemplateNodeBind->JSNode()->GetCData(XFA_Attribute::Ref);
          uint32_t dFlags =
              XFA_RESOLVENODE_Children | XFA_RESOLVENODE_CreateNode;
          XFA_RESOLVENODE_RS rs;
          pDocument->GetScriptContext()->ResolveObjects(
              pDataScope, wsRef.AsStringView(), rs, dFlags, pTemplateNode);
          CXFA_Object* pObject =
              !rs.objects.empty() ? rs.objects.front() : nullptr;
          pDataNode = ToNode(pObject);
          if (pDataNode) {
            CreateDataBinding(pFormNode, pDataNode,
                              rs.dwFlags == XFA_RESOLVENODE_RSTYPE_ExistNodes);
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
    for (CXFA_Node* pFormChild =
             pFormNode->GetNodeItem(XFA_NODEITEM_FirstChild);
         pFormChild;
         pFormChild = pFormChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
      if (!pFormChild->IsContainerNode())
        continue;
      if (pFormChild->IsUnusedNode())
        continue;

      UpdateBindingRelations(pDocument, pFormChild,
                             pDataNode ? pDataNode : pDataScope, bDataRef,
                             bParentDataRef);
    }
  }
}

void UpdateDataRelation(CXFA_Node* pDataNode, CXFA_Node* pDataDescriptionNode) {
  ASSERT(pDataDescriptionNode);
  for (CXFA_Node* pDataChild = pDataNode->GetNodeItem(XFA_NODEITEM_FirstChild);
       pDataChild;
       pDataChild = pDataChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    uint32_t dwNameHash = pDataChild->GetNameHash();
    if (!dwNameHash)
      continue;

    CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_DDGroup>
        sIterator(pDataDescriptionNode);
    for (CXFA_Node* pDDGroupNode = sIterator.GetCurrent(); pDDGroupNode;
         pDDGroupNode = sIterator.MoveToNext()) {
      if (pDDGroupNode != pDataDescriptionNode) {
        if (pDDGroupNode->GetElementType() != XFA_Element::DataGroup)
          continue;

        pdfium::Optional<WideString> ns =
            pDDGroupNode->JSNode()->TryNamespace();
        if (!ns || *ns != L"http://ns.adobe.com/data-description/")
          continue;
      }

      CXFA_Node* pDDNode = pDDGroupNode->GetFirstChildByName(dwNameHash);
      if (!pDDNode)
        continue;
      if (pDDNode->GetElementType() != pDataChild->GetElementType())
        break;

      pDataChild->SetDataDescriptionNode(pDDNode);
      UpdateDataRelation(pDataChild, pDDNode);
      break;
    }
  }
}

}  // namespace

CXFA_Node* XFA_DataMerge_FindFormDOMInstance(CXFA_Document* pDocument,
                                             XFA_Element eType,
                                             uint32_t dwNameHash,
                                             CXFA_Node* pFormParent) {
  CXFA_Node* pFormChild = pFormParent->GetNodeItem(XFA_NODEITEM_FirstChild);
  for (; pFormChild;
       pFormChild = pFormChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pFormChild->GetElementType() == eType &&
        pFormChild->GetNameHash() == dwNameHash && pFormChild->IsUnusedNode()) {
      return pFormChild;
    }
  }
  return nullptr;
}

CXFA_Node* XFA_NodeMerge_CloneOrMergeContainer(
    CXFA_Document* pDocument,
    CXFA_Node* pFormParent,
    CXFA_Node* pTemplateNode,
    bool bRecursive,
    std::vector<CXFA_Node*>* pSubformArray) {
  CXFA_Node* pExistingNode = nullptr;
  if (!pSubformArray) {
    pExistingNode = XFA_DataMerge_FindFormDOMInstance(
        pDocument, pTemplateNode->GetElementType(),
        pTemplateNode->GetNameHash(), pFormParent);
  } else if (!pSubformArray->empty()) {
    pExistingNode = pSubformArray->front();
    pSubformArray->erase(pSubformArray->begin());
  }
  if (pExistingNode) {
    if (pSubformArray) {
      pFormParent->InsertChild(pExistingNode, nullptr);
    } else if (pExistingNode->IsContainerNode()) {
      pFormParent->RemoveChild(pExistingNode, true);
      pFormParent->InsertChild(pExistingNode, nullptr);
    }
    pExistingNode->ClearFlag(XFA_NodeFlag_UnusedNode);
    pExistingNode->SetTemplateNode(pTemplateNode);
    if (bRecursive && pExistingNode->GetElementType() != XFA_Element::Items) {
      for (CXFA_Node* pTemplateChild =
               pTemplateNode->GetNodeItem(XFA_NODEITEM_FirstChild);
           pTemplateChild; pTemplateChild = pTemplateChild->GetNodeItem(
                               XFA_NODEITEM_NextSibling)) {
        if (NeedGenerateForm(pTemplateChild, true)) {
          XFA_NodeMerge_CloneOrMergeContainer(
              pDocument, pExistingNode, pTemplateChild, bRecursive, nullptr);
        }
      }
    }
    pExistingNode->SetFlag(XFA_NodeFlag_Initialized, true);
    return pExistingNode;
  }

  CXFA_Node* pNewNode = pTemplateNode->CloneTemplateToForm(false);
  pFormParent->InsertChild(pNewNode, nullptr);
  if (bRecursive) {
    for (CXFA_Node* pTemplateChild =
             pTemplateNode->GetNodeItem(XFA_NODEITEM_FirstChild);
         pTemplateChild; pTemplateChild = pTemplateChild->GetNodeItem(
                             XFA_NODEITEM_NextSibling)) {
      if (NeedGenerateForm(pTemplateChild, true)) {
        CXFA_Node* pNewChild = pTemplateChild->CloneTemplateToForm(true);
        pNewNode->InsertChild(pNewChild, nullptr);
      }
    }
  }
  return pNewNode;
}

CXFA_Node* XFA_DataMerge_FindDataScope(CXFA_Node* pParentFormNode) {
  for (CXFA_Node* pRootBoundNode = pParentFormNode;
       pRootBoundNode && pRootBoundNode->IsContainerNode();
       pRootBoundNode = pRootBoundNode->GetNodeItem(XFA_NODEITEM_Parent)) {
    CXFA_Node* pDataScope = pRootBoundNode->GetBindData();
    if (pDataScope)
      return pDataScope;
  }
  return ToNode(
      pParentFormNode->GetDocument()->GetXFAObject(XFA_HASHCODE_Data));
}

CXFA_Node* CXFA_Document::DataMerge_CopyContainer(CXFA_Node* pTemplateNode,
                                                  CXFA_Node* pFormNode,
                                                  CXFA_Node* pDataScope,
                                                  bool bOneInstance,
                                                  bool bDataMerge,
                                                  bool bUpLevel) {
  switch (pTemplateNode->GetElementType()) {
    case XFA_Element::SubformSet:
    case XFA_Element::Subform:
    case XFA_Element::Area:
    case XFA_Element::PageArea:
      return CopyContainer_SubformSet(this, pTemplateNode, pFormNode,
                                      pDataScope, bOneInstance, bDataMerge);
    case XFA_Element::ExclGroup:
    case XFA_Element::Field:
    case XFA_Element::Draw:
    case XFA_Element::ContentArea:
      return CopyContainer_Field(this, pTemplateNode, pFormNode, pDataScope,
                                 bDataMerge, bUpLevel);
    case XFA_Element::PageSet:
    case XFA_Element::Variables:
      break;
    default:
      NOTREACHED();
      break;
  }
  return nullptr;
}

void CXFA_Document::DataMerge_UpdateBindingRelations(
    CXFA_Node* pFormUpdateRoot) {
  CXFA_Node* pDataScope = XFA_DataMerge_FindDataScope(
      pFormUpdateRoot->GetNodeItem(XFA_NODEITEM_Parent));
  if (!pDataScope)
    return;

  UpdateBindingRelations(this, pFormUpdateRoot, pDataScope, false, false);
  UpdateBindingRelations(this, pFormUpdateRoot, pDataScope, true, false);
}

CXFA_Node* CXFA_Document::GetNotBindNode(
    const std::vector<CXFA_Object*>& arrayObjects) {
  for (CXFA_Object* pObject : arrayObjects) {
    CXFA_Node* pNode = pObject->AsNode();
    if (pNode && !pNode->HasBindItem())
      return pNode;
  }
  return nullptr;
}

void CXFA_Document::DoDataMerge() {
  CXFA_Node* pDatasetsRoot = ToNode(GetXFAObject(XFA_HASHCODE_Datasets));
  if (!pDatasetsRoot) {
    CFX_XMLElement* pDatasetsXMLNode = new CFX_XMLElement(L"xfa:datasets");
    pDatasetsXMLNode->SetString(L"xmlns:xfa",
                                L"http://www.xfa.org/schema/xfa-data/1.0/");
    pDatasetsRoot =
        CreateNode(XFA_PacketType::Datasets, XFA_Element::DataModel);
    pDatasetsRoot->JSNode()->SetCData(XFA_Attribute::Name, L"datasets", false,
                                      false);
    m_pRootNode->GetXMLMappingNode()->InsertChildNode(pDatasetsXMLNode);
    m_pRootNode->InsertChild(pDatasetsRoot, nullptr);
    pDatasetsRoot->SetXMLMappingNode(pDatasetsXMLNode);
  }
  CXFA_Node *pDataRoot = nullptr, *pDDRoot = nullptr;
  WideString wsDatasetsURI =
      pDatasetsRoot->JSNode()->TryNamespace().value_or(WideString());
  for (CXFA_Node* pChildNode =
           pDatasetsRoot->GetNodeItem(XFA_NODEITEM_FirstChild);
       pChildNode;
       pChildNode = pChildNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pChildNode->GetElementType() != XFA_Element::DataGroup)
      continue;

    if (!pDDRoot && pChildNode->GetNameHash() == XFA_HASHCODE_DataDescription) {
      pdfium::Optional<WideString> namespaceURI =
          pChildNode->JSNode()->TryNamespace();
      if (!namespaceURI)
        continue;
      if (*namespaceURI == L"http://ns.adobe.com/data-description/")
        pDDRoot = pChildNode;
    } else if (!pDataRoot && pChildNode->GetNameHash() == XFA_HASHCODE_Data) {
      pdfium::Optional<WideString> namespaceURI =
          pChildNode->JSNode()->TryNamespace();
      if (!namespaceURI)
        continue;
      if (*namespaceURI == wsDatasetsURI)
        pDataRoot = pChildNode;
    }
    if (pDataRoot && pDDRoot)
      break;
  }

  if (!pDataRoot) {
    CFX_XMLElement* pDataRootXMLNode = new CFX_XMLElement(L"xfa:data");
    pDataRoot = CreateNode(XFA_PacketType::Datasets, XFA_Element::DataGroup);
    pDataRoot->JSNode()->SetCData(XFA_Attribute::Name, L"data", false, false);
    pDataRoot->SetXMLMappingNode(pDataRootXMLNode);
    pDatasetsRoot->InsertChild(pDataRoot, nullptr);
  }

  CXFA_Node* pDataTopLevel =
      pDataRoot->GetFirstChildByClass(XFA_Element::DataGroup);
  uint32_t dwNameHash = pDataTopLevel ? pDataTopLevel->GetNameHash() : 0;
  CXFA_Node* pTemplateRoot =
      m_pRootNode->GetFirstChildByClass(XFA_Element::Template);
  if (!pTemplateRoot)
    return;

  CXFA_Node* pTemplateChosen =
      dwNameHash != 0 ? pTemplateRoot->GetFirstChildByName(dwNameHash)
                      : nullptr;
  if (!pTemplateChosen ||
      pTemplateChosen->GetElementType() != XFA_Element::Subform) {
    pTemplateChosen = pTemplateRoot->GetFirstChildByClass(XFA_Element::Subform);
  }
  if (!pTemplateChosen)
    return;

  CXFA_Node* pFormRoot = m_pRootNode->GetFirstChildByClass(XFA_Element::Form);
  bool bEmptyForm = false;
  if (!pFormRoot) {
    bEmptyForm = true;
    pFormRoot = CreateNode(XFA_PacketType::Form, XFA_Element::Form);
    ASSERT(pFormRoot);
    pFormRoot->JSNode()->SetCData(XFA_Attribute::Name, L"form", false, false);
    m_pRootNode->InsertChild(pFormRoot, nullptr);
  } else {
    CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_XFANode>
        sIterator(pFormRoot);
    for (CXFA_Node* pNode = sIterator.MoveToNext(); pNode;
         pNode = sIterator.MoveToNext()) {
      pNode->SetFlag(XFA_NodeFlag_UnusedNode, true);
    }
  }

  CXFA_Node* pSubformSetNode = XFA_NodeMerge_CloneOrMergeContainer(
      this, pFormRoot, pTemplateChosen, false, nullptr);
  ASSERT(pSubformSetNode);
  if (!pDataTopLevel) {
    WideString wsFormName =
        pSubformSetNode->JSNode()->GetCData(XFA_Attribute::Name);
    WideString wsDataTopLevelName(wsFormName.IsEmpty() ? L"form" : wsFormName);
    CFX_XMLElement* pDataTopLevelXMLNode =
        new CFX_XMLElement(wsDataTopLevelName);

    pDataTopLevel =
        CreateNode(XFA_PacketType::Datasets, XFA_Element::DataGroup);
    pDataTopLevel->JSNode()->SetCData(XFA_Attribute::Name, wsDataTopLevelName,
                                      false, false);
    pDataTopLevel->SetXMLMappingNode(pDataTopLevelXMLNode);
    CXFA_Node* pBeforeNode = pDataRoot->GetNodeItem(XFA_NODEITEM_FirstChild);
    pDataRoot->InsertChild(pDataTopLevel, pBeforeNode);
  }

  ASSERT(pDataTopLevel);
  CreateDataBinding(pSubformSetNode, pDataTopLevel, true);
  for (CXFA_Node* pTemplateChild =
           pTemplateChosen->GetNodeItem(XFA_NODEITEM_FirstChild);
       pTemplateChild;
       pTemplateChild = pTemplateChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (NeedGenerateForm(pTemplateChild, true)) {
      XFA_NodeMerge_CloneOrMergeContainer(this, pSubformSetNode, pTemplateChild,
                                          true, nullptr);
    } else if (pTemplateChild->IsContainerNode()) {
      DataMerge_CopyContainer(pTemplateChild, pSubformSetNode, pDataTopLevel,
                              false, true, true);
    }
  }
  if (pDDRoot)
    UpdateDataRelation(pDataRoot, pDDRoot);

  DataMerge_UpdateBindingRelations(pSubformSetNode);
  CXFA_Node* pPageSetNode =
      pSubformSetNode->GetFirstChildByClass(XFA_Element::PageSet);
  while (pPageSetNode) {
    m_pPendingPageSet.push_back(pPageSetNode);
    CXFA_Node* pNextPageSetNode =
        pPageSetNode->GetNextSameClassSibling(XFA_Element::PageSet);
    pSubformSetNode->RemoveChild(pPageSetNode, true);
    pPageSetNode = pNextPageSetNode;
  }

  if (bEmptyForm)
    return;

  CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_XFANode> sIterator(
      pFormRoot);
  CXFA_Node* pNode = sIterator.MoveToNext();
  while (pNode) {
    if (pNode->IsUnusedNode()) {
      if (pNode->IsContainerNode() ||
          pNode->GetElementType() == XFA_Element::InstanceManager) {
        CXFA_Node* pNext = sIterator.SkipChildrenAndMoveToNext();
        pNode->GetNodeItem(XFA_NODEITEM_Parent)->RemoveChild(pNode, true);
        pNode = pNext;
      } else {
        pNode->ClearFlag(XFA_NodeFlag_UnusedNode);
        pNode->SetFlag(XFA_NodeFlag_Initialized, true);
        pNode = sIterator.MoveToNext();
      }
    } else {
      pNode->SetFlag(XFA_NodeFlag_Initialized, true);
      pNode = sIterator.MoveToNext();
    }
  }
}

void CXFA_Document::DoDataRemerge(bool bDoDataMerge) {
  CXFA_Node* pFormRoot = ToNode(GetXFAObject(XFA_HASHCODE_Form));
  if (pFormRoot) {
    while (CXFA_Node* pNode = pFormRoot->GetNodeItem(XFA_NODEITEM_FirstChild))
      pFormRoot->RemoveChild(pNode, true);

    pFormRoot->JSNode()->SetBindingNode(nullptr);
  }
  m_rgGlobalBinding.clear();

  if (bDoDataMerge)
    DoDataMerge();

  CXFA_LayoutProcessor* pLayoutProcessor = GetLayoutProcessor();
  pLayoutProcessor->SetForceReLayout(true);
}
