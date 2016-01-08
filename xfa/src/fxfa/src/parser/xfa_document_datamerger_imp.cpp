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
#include "xfa_document_datadescription_imp.h"
#include "xfa_document_datamerger_imp.h"
#include "xfa_document_layout_imp.h"
static FX_BOOL XFA_GetOccurInfo(CXFA_Node* pOccurNode,
                                int32_t& iMin,
                                int32_t& iMax,
                                int32_t& iInit) {
  if (!pOccurNode) {
    return FALSE;
  }
  CXFA_Occur occur(pOccurNode);
  return occur.GetOccurInfo(iMin, iMax, iInit);
}
struct XFA_DataMerge_RecurseRecord {
  CXFA_Node* pTemplateChild;
  CXFA_Node* pDataChild;
};
static CXFA_Node* XFA_DataMerge_FormValueNode_CreateChild(
    CXFA_Node* pValueNode,
    XFA_ELEMENT iType = XFA_ELEMENT_UNKNOWN) {
  CXFA_Node* pChildNode = pValueNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  if (!pChildNode) {
    if (iType == XFA_ELEMENT_UNKNOWN) {
      return FALSE;
    }
    pChildNode = pValueNode->GetProperty(0, iType);
  }
  return pChildNode;
}
static void XFA_DataMerge_FormValueNode_MatchNoneCreateChild(
    CXFA_Node* pFormNode) {
  CXFA_WidgetData* pWidgetData = pFormNode->GetWidgetData();
  FXSYS_assert(pWidgetData);
  pWidgetData->GetUIType();
}
static FX_BOOL XFA_DataMerge_FormValueNode_SetChildContent(
    CXFA_Node* pValueNode,
    const CFX_WideString& wsContent,
    XFA_ELEMENT iType = XFA_ELEMENT_UNKNOWN) {
  if (!pValueNode) {
    return FALSE;
  }
  FXSYS_assert(pValueNode->GetPacketID() == XFA_XDPPACKET_Form);
  CXFA_Node* pChildNode =
      XFA_DataMerge_FormValueNode_CreateChild(pValueNode, iType);
  if (!pChildNode) {
    return FALSE;
  }
  XFA_OBJECTTYPE objectType = pChildNode->GetObjectType();
  switch (objectType) {
    case XFA_OBJECTTYPE_ContentNode: {
      CXFA_Node* pContentRawDataNode =
          pChildNode->GetNodeItem(XFA_NODEITEM_FirstChild);
      if (!pContentRawDataNode) {
        XFA_ELEMENT element = XFA_ELEMENT_Sharptext;
        if (pChildNode->GetClassID() == XFA_ELEMENT_ExData) {
          CFX_WideString wsContentType;
          pChildNode->GetAttribute(XFA_ATTRIBUTE_ContentType, wsContentType,
                                   FALSE);
          if (wsContentType.Equal(FX_WSTRC(L"text/html"))) {
            element = XFA_ELEMENT_SharpxHTML;
          } else if (wsContentType.Equal(FX_WSTRC(L"text/xml"))) {
            element = XFA_ELEMENT_Sharpxml;
          }
        }
        pContentRawDataNode = pChildNode->CreateSamePacketNode(element);
        pChildNode->InsertChild(pContentRawDataNode);
      }
      pContentRawDataNode->SetCData(XFA_ATTRIBUTE_Value, wsContent);
    } break;
    case XFA_OBJECTTYPE_NodeC:
    case XFA_OBJECTTYPE_TextNode:
    case XFA_OBJECTTYPE_NodeV: {
      pChildNode->SetCData(XFA_ATTRIBUTE_Value, wsContent);
    } break;
    default:
      FXSYS_assert(FALSE);
      break;
  }
  return TRUE;
}
static void XFA_DataMerge_CreateDataBinding(CXFA_Node* pFormNode,
                                            CXFA_Node* pDataNode,
                                            FX_BOOL bDataToForm = TRUE) {
  pFormNode->SetObject(XFA_ATTRIBUTE_BindingNode, pDataNode);
  pDataNode->AddBindItem(pFormNode);
  XFA_ELEMENT eClass = pFormNode->GetClassID();
  if (eClass != XFA_ELEMENT_Field && eClass != XFA_ELEMENT_ExclGroup) {
    return;
  }
  CXFA_WidgetData* pWidgetData = pFormNode->GetWidgetData();
  FXSYS_assert(pWidgetData);
  FX_BOOL bNotify = FALSE;
  XFA_ELEMENT eUIType = pWidgetData->GetUIType();
  CXFA_Value defValue = pFormNode->GetProperty(0, XFA_ELEMENT_Value);
  if (!bDataToForm) {
    CFX_WideString wsValue;
    CFX_WideString wsFormatedValue;
    switch (eUIType) {
      case XFA_ELEMENT_ImageEdit: {
        CXFA_Image image = defValue.GetImage();
        CFX_WideString wsContentType;
        CFX_WideString wsHref;
        if (image) {
          image.GetContent(wsValue);
          image.GetContentType(wsContentType);
          image.GetHref(wsHref);
        }
        IFDE_XMLElement* pXMLDataElement =
            (IFDE_XMLElement*)(pDataNode->GetXMLMappingNode());
        FXSYS_assert(pXMLDataElement);
        pWidgetData->GetFormatDataValue(wsValue, wsFormatedValue);
        pDataNode->SetAttributeValue(wsValue, wsFormatedValue);
        pDataNode->SetCData(XFA_ATTRIBUTE_ContentType, wsContentType);
        if (!wsHref.IsEmpty()) {
          pXMLDataElement->SetString(FX_WSTRC(L"href"), wsHref);
        }
      } break;
      case XFA_ELEMENT_ChoiceList:
        defValue.GetChildValueContent(wsValue);
        if (pWidgetData->GetChoiceListOpen() == XFA_ATTRIBUTEENUM_MultiSelect) {
          CFX_WideStringArray wsSelTextArray;
          pWidgetData->GetSelectedItemsValue(wsSelTextArray);
          int32_t iSize = wsSelTextArray.GetSize();
          if (iSize >= 1) {
            CXFA_Node* pValue = NULL;
            IFDE_XMLNode* pValueXMLNode = NULL;
            for (int32_t i = 0; i < iSize; i++) {
              pValue = pDataNode->CreateSamePacketNode(XFA_ELEMENT_DataValue);
              pValue->SetCData(XFA_ATTRIBUTE_Name, FX_WSTRC(L"value"));
              pValueXMLNode = pValue->CreateXMLMappingNode();
              pDataNode->InsertChild(pValue);
              pValue->SetCData(XFA_ATTRIBUTE_Value, wsSelTextArray[i]);
            }
          } else {
            IFDE_XMLNode* pXMLNode = pDataNode->GetXMLMappingNode();
            FXSYS_assert(pXMLNode->GetType() == FDE_XMLNODE_Element);
            ((IFDE_XMLElement*)pXMLNode)
                ->SetString(FX_WSTRC(L"xfa:dataNode"), FX_WSTRC(L"dataGroup"));
          }
        } else if (!wsValue.IsEmpty()) {
          pWidgetData->GetFormatDataValue(wsValue, wsFormatedValue);
          pDataNode->SetAttributeValue(wsValue, wsFormatedValue);
        }
        break;
      case XFA_ELEMENT_CheckButton:
        defValue.GetChildValueContent(wsValue);
        if (wsValue.IsEmpty()) {
          break;
        }
        pWidgetData->GetFormatDataValue(wsValue, wsFormatedValue);
        pDataNode->SetAttributeValue(wsValue, wsFormatedValue);
        break;
      case XFA_ELEMENT_ExclGroup: {
        CXFA_Node* pChecked = NULL;
        XFA_ELEMENT eValueType = XFA_ELEMENT_UNKNOWN;
        CXFA_Node* pChild = pFormNode->GetNodeItem(XFA_NODEITEM_FirstChild);
        for (; pChild; pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
          if (pChild->GetClassID() != XFA_ELEMENT_Field) {
            continue;
          }
          CXFA_Node* pValue = pChild->GetChild(0, XFA_ELEMENT_Value);
          if (!pValue) {
            continue;
          }
          CXFA_Value valueChild(pValue);
          valueChild.GetChildValueContent(wsValue);
          if (wsValue.IsEmpty()) {
            continue;
          }
          CXFA_Node* pItems = pChild->GetChild(0, XFA_ELEMENT_Items);
          if (!pItems) {
            continue;
          }
          CXFA_Node* pText = pItems->GetNodeItem(XFA_NODEITEM_FirstChild);
          if (!pText) {
            continue;
          }
          CFX_WideString wsContent;
          if (pText->TryContent(wsContent) && (wsContent == wsValue)) {
            pChecked = pChild;
            eValueType = pText->GetClassID();
            wsFormatedValue = wsValue;
            pDataNode->SetAttributeValue(wsValue, wsFormatedValue);
            pFormNode->SetCData(XFA_ATTRIBUTE_Value, wsContent);
            break;
          }
        }
        if (!pChecked) {
          break;
        }
        pChild = pFormNode->GetNodeItem(XFA_NODEITEM_FirstChild);
        for (; pChild; pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
          if (pChild == pChecked) {
            continue;
          }
          if (pChild->GetClassID() != XFA_ELEMENT_Field) {
            continue;
          }
          CXFA_Node* pValue = pChild->GetProperty(0, XFA_ELEMENT_Value);
          CXFA_Node* pItems = pChild->GetChild(0, XFA_ELEMENT_Items);
          CXFA_Node* pText =
              pItems ? pItems->GetNodeItem(XFA_NODEITEM_FirstChild) : NULL;
          if (pText) {
            pText = pText->GetNodeItem(XFA_NODEITEM_NextSibling);
          }
          CFX_WideString wsContent;
          if (pText) {
            pText->TryContent(wsContent);
          }
          XFA_DataMerge_FormValueNode_SetChildContent(pValue, wsContent,
                                                      XFA_ELEMENT_Text);
        }
      } break;
      case XFA_ELEMENT_NumericEdit: {
        defValue.GetChildValueContent(wsValue);
        if (wsValue.IsEmpty()) {
          break;
        }
        CFX_WideString wsOutput;
        pWidgetData->NormalizeNumStr(wsValue, wsOutput);
        wsValue = wsOutput;
        pWidgetData->GetFormatDataValue(wsValue, wsFormatedValue);
        pDataNode->SetAttributeValue(wsValue, wsFormatedValue);
        CXFA_Node* pValue = pFormNode->GetProperty(0, XFA_ELEMENT_Value);
        XFA_DataMerge_FormValueNode_SetChildContent(pValue, wsValue,
                                                    XFA_ELEMENT_Float);
      } break;
      default:
        defValue.GetChildValueContent(wsValue);
        if (wsValue.IsEmpty()) {
          break;
        }
        pWidgetData->GetFormatDataValue(wsValue, wsFormatedValue);
        pDataNode->SetAttributeValue(wsValue, wsFormatedValue);
        break;
    }
  } else {
    CFX_WideString wsXMLValue;
    pDataNode->TryContent(wsXMLValue);
    CFX_WideString wsNormailizeValue;
    pWidgetData->GetNormalizeDataValue(wsXMLValue, wsNormailizeValue);
    pDataNode->SetAttributeValue(wsNormailizeValue, wsXMLValue);
    switch (eUIType) {
      case XFA_ELEMENT_ImageEdit: {
        XFA_DataMerge_FormValueNode_SetChildContent(
            defValue.GetNode(), wsNormailizeValue, XFA_ELEMENT_Image);
        CXFA_Image image = defValue.GetImage();
        if (image) {
          IFDE_XMLElement* pXMLDataElement =
              (IFDE_XMLElement*)(pDataNode->GetXMLMappingNode());
          FXSYS_assert(pXMLDataElement);
          CFX_WideString wsContentType;
          CFX_WideString wsHref;
          pXMLDataElement->GetString(L"xfa:contentType", wsContentType);
          if (!wsContentType.IsEmpty()) {
            pDataNode->SetCData(XFA_ATTRIBUTE_ContentType, wsContentType);
            image.SetContentType(wsContentType);
          }
          pXMLDataElement->GetString(L"href", wsHref);
          if (!wsHref.IsEmpty()) {
            image.SetHref(wsHref);
          }
        }
      } break;
      case XFA_ELEMENT_ChoiceList:
        if (pWidgetData->GetChoiceListOpen() == XFA_ATTRIBUTEENUM_MultiSelect) {
          CXFA_NodeArray items;
          pDataNode->GetNodeList(items);
          int32_t iCounts = items.GetSize();
          if (iCounts > 0) {
            wsNormailizeValue.Empty();
            CFX_WideString wsItem;
            for (int32_t i = 0; i < iCounts; i++) {
              items[i]->TryContent(wsItem);
              wsItem = (iCounts == 1) ? wsItem : wsItem + FX_WSTRC(L"\n");
              wsNormailizeValue += wsItem;
            }
            CXFA_ExData exData = defValue.GetExData();
            FXSYS_assert(exData != NULL);
            exData.SetContentType((iCounts == 1) ? FX_WSTRC(L"text/plain")
                                                 : FX_WSTRC(L"text/xml"));
          }
          XFA_DataMerge_FormValueNode_SetChildContent(
              defValue.GetNode(), wsNormailizeValue, XFA_ELEMENT_ExData);
        } else {
          XFA_DataMerge_FormValueNode_SetChildContent(
              defValue.GetNode(), wsNormailizeValue, XFA_ELEMENT_Text);
        }
        break;
      case XFA_ELEMENT_CheckButton:
        XFA_DataMerge_FormValueNode_SetChildContent(
            defValue.GetNode(), wsNormailizeValue, XFA_ELEMENT_Text);
        break;
      case XFA_ELEMENT_ExclGroup: {
        pWidgetData->SetSelectedMemberByValue(wsNormailizeValue, bNotify, FALSE,
                                              FALSE);
      } break;
      case XFA_ELEMENT_DateTimeEdit:
        XFA_DataMerge_FormValueNode_SetChildContent(
            defValue.GetNode(), wsNormailizeValue, XFA_ELEMENT_DateTime);
        break;
      case XFA_ELEMENT_NumericEdit: {
        CFX_WideString wsPicture;
        pWidgetData->GetPictureContent(wsPicture, XFA_VALUEPICTURE_DataBind);
        if (wsPicture.IsEmpty()) {
          CFX_WideString wsOutput;
          pWidgetData->NormalizeNumStr(wsNormailizeValue, wsOutput);
          wsNormailizeValue = wsOutput;
        }
        XFA_DataMerge_FormValueNode_SetChildContent(
            defValue.GetNode(), wsNormailizeValue, XFA_ELEMENT_Float);
      } break;
      case XFA_ELEMENT_Barcode:
      case XFA_ELEMENT_Button:
      case XFA_ELEMENT_PasswordEdit:
      case XFA_ELEMENT_Signature:
      case XFA_ELEMENT_TextEdit:
      default:
        XFA_DataMerge_FormValueNode_SetChildContent(
            defValue.GetNode(), wsNormailizeValue, XFA_ELEMENT_Text);
        break;
    }
  }
}
static CXFA_Node* XFA_DataMerge_GetGlobalBinding(CXFA_Document* pDocument,
                                                 FX_DWORD dwNameHash) {
  CXFA_Node* pNode = NULL;
  pDocument->m_rgGlobalBinding.Lookup(dwNameHash, pNode);
  return pNode;
}
static void XFA_DataMerge_RegisterGlobalBinding(CXFA_Document* pDocument,
                                                FX_DWORD dwNameHash,
                                                CXFA_Node* pDataNode) {
  pDocument->m_rgGlobalBinding.SetAt(dwNameHash, pDataNode);
}
static void XFA_DataMerge_ClearGlobalBinding(CXFA_Document* pDocument) {
  pDocument->m_rgGlobalBinding.RemoveAll();
}
static CXFA_Node* XFA_DataMerge_ScopeMatchGlobalBinding(
    CXFA_Node* pDataScope,
    FX_DWORD dwNameHash,
    XFA_ELEMENT eMatchDataNodeType,
    FX_BOOL bUpLevel = TRUE) {
  for (CXFA_Node *pCurDataScope = pDataScope, *pLastDataScope = NULL;
       pCurDataScope && pCurDataScope->GetPacketID() == XFA_XDPPACKET_Datasets;
       pLastDataScope = pCurDataScope,
                 pCurDataScope =
                     pCurDataScope->GetNodeItem(XFA_NODEITEM_Parent)) {
    for (CXFA_Node* pDataChild = pCurDataScope->GetFirstChildByName(dwNameHash);
         pDataChild;
         pDataChild = pDataChild->GetNextSameNameSibling(dwNameHash)) {
      if (pDataChild == pLastDataScope ||
          (eMatchDataNodeType != XFA_ELEMENT_DataModel &&
           pDataChild->GetClassID() != eMatchDataNodeType) ||
          pDataChild->HasBindItem()) {
        continue;
      }
      return pDataChild;
    }
    for (CXFA_Node* pDataChild =
             pCurDataScope->GetFirstChildByClass(XFA_ELEMENT_DataGroup);
         pDataChild; pDataChild = pDataChild->GetNextSameClassSibling(
                         XFA_ELEMENT_DataGroup)) {
      CXFA_Node* pDataNode = XFA_DataMerge_ScopeMatchGlobalBinding(
          pDataChild, dwNameHash, eMatchDataNodeType, FALSE);
      if (pDataNode) {
        return pDataNode;
      }
    }
    if (!bUpLevel) {
      break;
    }
  }
  return NULL;
}
static CXFA_Node* XFA_DataMerge_FindGlobalDataNode(CXFA_Document* pDocument,
                                                   CFX_WideStringC wsName,
                                                   CXFA_Node* pDataScope,
                                                   XFA_ELEMENT eMatchNodeType) {
  FX_DWORD dwNameHash =
      wsName.IsEmpty() ? 0 : FX_HashCode_String_GetW(wsName.GetPtr(),
                                                     wsName.GetLength());
  if (dwNameHash != 0) {
    CXFA_Node* pBounded = XFA_DataMerge_GetGlobalBinding(pDocument, dwNameHash);
    if (!pBounded) {
      pBounded = XFA_DataMerge_ScopeMatchGlobalBinding(pDataScope, dwNameHash,
                                                       eMatchNodeType);
      if (pBounded) {
        XFA_DataMerge_RegisterGlobalBinding(pDocument, dwNameHash, pBounded);
      }
    }
    return pBounded;
  }
  return NULL;
}
static CXFA_Node* XFA_DataMerge_FindOnceDataNode(CXFA_Document* pDocument,
                                                 CFX_WideStringC wsName,
                                                 CXFA_Node* pDataScope,
                                                 XFA_ELEMENT eMatchNodeType) {
  FX_DWORD dwNameHash =
      wsName.IsEmpty() ? 0 : FX_HashCode_String_GetW(wsName.GetPtr(),
                                                     wsName.GetLength());
  if (dwNameHash != 0) {
    for (CXFA_Node *pCurDataScope = pDataScope, *pLastDataScope = NULL;
         pCurDataScope &&
         pCurDataScope->GetPacketID() == XFA_XDPPACKET_Datasets;
         pLastDataScope = pCurDataScope,
                   pCurDataScope =
                       pCurDataScope->GetNodeItem(XFA_NODEITEM_Parent)) {
      for (CXFA_Node* pDataChild =
               pCurDataScope->GetFirstChildByName(dwNameHash);
           pDataChild;
           pDataChild = pDataChild->GetNextSameNameSibling(dwNameHash)) {
        if (pDataChild == pLastDataScope ||
            (eMatchNodeType != XFA_ELEMENT_DataModel &&
             pDataChild->GetClassID() != eMatchNodeType) ||
            pDataChild->HasBindItem()) {
          continue;
        }
        return pDataChild;
      }
    }
  }
  return NULL;
}
static CXFA_Node* XFA_DataMerge_FindDataRefDataNode(CXFA_Document* pDocument,
                                                    CFX_WideStringC wsRef,
                                                    CXFA_Node* pDataScope,
                                                    XFA_ELEMENT eMatchNodeType,
                                                    CXFA_Node* pTemplateNode,
                                                    FX_BOOL bForceBind,
                                                    FX_BOOL bUpLevel = TRUE) {
  FX_DWORD dFlags = XFA_RESOLVENODE_Children | XFA_RESOLVENODE_BindNew;
  if (bUpLevel || wsRef != FX_WSTRC(L"name")) {
    dFlags |= (XFA_RESOLVENODE_Parent | XFA_RESOLVENODE_Siblings);
  }
  XFA_RESOLVENODE_RS rs;
  pDocument->GetScriptContext()->ResolveObjects(pDataScope, wsRef, rs, dFlags,
                                                pTemplateNode);
  if (rs.dwFlags == XFA_RESOLVENODE_RSTYPE_CreateNodeAll ||
      rs.dwFlags == XFA_RESOLVENODE_RSTYPE_CreateNodeMidAll ||
      rs.nodes.GetSize() > 1) {
    return pDocument->GetNotBindNode(rs.nodes);
  } else if (rs.dwFlags == XFA_RESOLVENODE_RSTYPE_CreateNodeOne) {
    CXFA_Object* pObject = (rs.nodes.GetSize() > 0) ? rs.nodes[0] : NULL;
    CXFA_Node* pNode =
        (pObject && pObject->IsNode()) ? (CXFA_Node*)pObject : NULL;
    if (!bForceBind && (pNode != NULL) && pNode->HasBindItem()) {
      pNode = NULL;
    }
    return pNode;
  }
  return NULL;
}
CXFA_Node* XFA_DataMerge_FindFormDOMInstance(CXFA_Document* pDocument,
                                             XFA_ELEMENT eClassID,
                                             FX_DWORD dwNameHash,
                                             CXFA_Node* pFormParent) {
  CXFA_Node* pFormChild = pFormParent->GetNodeItem(XFA_NODEITEM_FirstChild);
  for (; pFormChild;
       pFormChild = pFormChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pFormChild->GetClassID() == eClassID &&
        pFormChild->GetNameHash() == dwNameHash &&
        pFormChild->HasFlag(XFA_NODEFLAG_UnusedNode)) {
      return pFormChild;
    }
  }
  return NULL;
}
static FX_BOOL XFA_NeedGenerateForm(CXFA_Node* pTemplateChild,
                                    FX_BOOL bUseInstanceManager = TRUE) {
  XFA_ELEMENT eType = pTemplateChild->GetClassID();
  if (eType == XFA_ELEMENT_Variables) {
    return TRUE;
  }
  if (pTemplateChild->GetObjectType() == XFA_OBJECTTYPE_ContainerNode) {
    return FALSE;
  }
  if (eType == XFA_ELEMENT_Proto ||
      (bUseInstanceManager && eType == XFA_ELEMENT_Occur)) {
    return FALSE;
  }
  return TRUE;
}
CXFA_Node* XFA_NodeMerge_CloneOrMergeContainer(CXFA_Document* pDocument,
                                               CXFA_Node* pFormParent,
                                               CXFA_Node* pTemplateNode,
                                               FX_BOOL bRecursive,
                                               CXFA_NodeArray* pSubformArray) {
  CXFA_Node* pExistingNode = NULL;
  if (pSubformArray == NULL) {
    pExistingNode = XFA_DataMerge_FindFormDOMInstance(
        pDocument, pTemplateNode->GetClassID(), pTemplateNode->GetNameHash(),
        pFormParent);
  } else if (pSubformArray->GetSize() > 0) {
    pExistingNode = pSubformArray->GetAt(0);
    pSubformArray->RemoveAt(0);
  }
  if (pExistingNode) {
    if (pSubformArray) {
      pFormParent->InsertChild(pExistingNode);
    } else if (pExistingNode->IsContainerNode()) {
      pFormParent->RemoveChild(pExistingNode);
      pFormParent->InsertChild(pExistingNode);
    }
    pExistingNode->SetFlag(XFA_NODEFLAG_UnusedNode, FALSE);
    pExistingNode->SetTemplateNode(pTemplateNode);
    if (bRecursive && pExistingNode->GetClassID() != XFA_ELEMENT_Items) {
      for (CXFA_Node* pTemplateChild =
               pTemplateNode->GetNodeItem(XFA_NODEITEM_FirstChild);
           pTemplateChild; pTemplateChild = pTemplateChild->GetNodeItem(
                               XFA_NODEITEM_NextSibling)) {
        if (XFA_NeedGenerateForm(pTemplateChild)) {
          XFA_NodeMerge_CloneOrMergeContainer(pDocument, pExistingNode,
                                              pTemplateChild, bRecursive);
        }
      }
    }
    pExistingNode->SetFlag(XFA_NODEFLAG_Initialized);
    return pExistingNode;
  }
  CXFA_Node* pNewNode = pTemplateNode->CloneTemplateToForm(FALSE);
  pFormParent->InsertChild(pNewNode, NULL);
  if (bRecursive) {
    for (CXFA_Node* pTemplateChild =
             pTemplateNode->GetNodeItem(XFA_NODEITEM_FirstChild);
         pTemplateChild; pTemplateChild = pTemplateChild->GetNodeItem(
                             XFA_NODEITEM_NextSibling)) {
      if (XFA_NeedGenerateForm(pTemplateChild)) {
        CXFA_Node* pNewChild = pTemplateChild->CloneTemplateToForm(TRUE);
        pNewNode->InsertChild(pNewChild, NULL);
      }
    }
  }
  return pNewNode;
}
static CXFA_Node* XFA_NodeMerge_CloneOrMergeInstanceManager(
    CXFA_Document* pDocument,
    CXFA_Node* pFormParent,
    CXFA_Node* pTemplateNode,
    CXFA_NodeArray& subforms) {
  CFX_WideStringC wsSubformName = pTemplateNode->GetCData(XFA_ATTRIBUTE_Name);
  CFX_WideString wsInstMgrNodeName = FX_WSTRC(L"_") + wsSubformName;
  FX_DWORD dwInstNameHash =
      FX_HashCode_String_GetW(wsInstMgrNodeName, wsInstMgrNodeName.GetLength());
  CXFA_Node* pExistingNode = XFA_DataMerge_FindFormDOMInstance(
      pDocument, XFA_ELEMENT_InstanceManager, dwInstNameHash, pFormParent);
  if (pExistingNode) {
    FX_DWORD dwNameHash = pTemplateNode->GetNameHash();
    for (CXFA_Node* pNode =
             pExistingNode->GetNodeItem(XFA_NODEITEM_NextSibling);
         pNode;) {
      XFA_ELEMENT eCurType = pNode->GetClassID();
      if (eCurType == XFA_ELEMENT_InstanceManager) {
        break;
      }
      if ((eCurType != XFA_ELEMENT_Subform) &&
          (eCurType != XFA_ELEMENT_SubformSet)) {
        pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling);
        continue;
      }
      if (dwNameHash != pNode->GetNameHash()) {
        break;
      }
      CXFA_Node* pNextNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling);
      pFormParent->RemoveChild(pNode);
      subforms.Add(pNode);
      pNode = pNextNode;
    }
    pFormParent->RemoveChild(pExistingNode);
    pFormParent->InsertChild(pExistingNode);
    pExistingNode->SetFlag(XFA_NODEFLAG_UnusedNode, FALSE);
    pExistingNode->SetTemplateNode(pTemplateNode);
    return pExistingNode;
  }
  CXFA_Node* pNewNode = pDocument->GetParser()->GetFactory()->CreateNode(
      XFA_XDPPACKET_Form, XFA_ELEMENT_InstanceManager);
  FXSYS_assert(pNewNode);
  wsInstMgrNodeName =
      FX_WSTRC(L"_") + pTemplateNode->GetCData(XFA_ATTRIBUTE_Name);
  pNewNode->SetCData(XFA_ATTRIBUTE_Name, wsInstMgrNodeName);
  pFormParent->InsertChild(pNewNode, NULL);
  pNewNode->SetTemplateNode(pTemplateNode);
  return pNewNode;
}
static CXFA_Node* XFA_DataMerge_FindMatchingDataNode(
    CXFA_Document* pDocument,
    CXFA_Node* pTemplateNode,
    CXFA_Node* pDataScope,
    FX_BOOL& bAccessedDataDOM,
    FX_BOOL bForceBind,
    CXFA_NodeIteratorTemplate<CXFA_Node,
                              CXFA_TraverseStrategy_XFAContainerNode>*
        pIterator,
    FX_BOOL& bSelfMatch,
    XFA_ATTRIBUTEENUM& eBindMatch,
    FX_BOOL bUpLevel = TRUE) {
  FX_BOOL bOwnIterator = FALSE;
  if (!pIterator) {
    bOwnIterator = TRUE;
    pIterator = new CXFA_NodeIteratorTemplate<
        CXFA_Node, CXFA_TraverseStrategy_XFAContainerNode>(pTemplateNode);
  }
  CXFA_Node* pResult = NULL;
  for (CXFA_Node* pCurTemplateNode = pIterator->GetCurrent();
       pCurTemplateNode;) {
    XFA_ELEMENT eMatchNodeType;
    switch (pCurTemplateNode->GetClassID()) {
      case XFA_ELEMENT_Subform:
        eMatchNodeType = XFA_ELEMENT_DataGroup;
        break;
      case XFA_ELEMENT_Field: {
        eMatchNodeType = XFA_FieldIsMultiListBox(pCurTemplateNode)
                             ? XFA_ELEMENT_DataGroup
                             : XFA_ELEMENT_DataValue;
      } break;
      case XFA_ELEMENT_ExclGroup:
        eMatchNodeType = XFA_ELEMENT_DataValue;
        break;
      default:
        pCurTemplateNode = pIterator->MoveToNext();
        continue;
    }
    CXFA_Node* pTemplateNodeOccur =
        pCurTemplateNode->GetFirstChildByClass(XFA_ELEMENT_Occur);
    int32_t iMin, iMax, iInit;
    if (pTemplateNodeOccur &&
        XFA_GetOccurInfo(pTemplateNodeOccur, iMin, iMax, iInit) && iMax == 0) {
      pCurTemplateNode = pIterator->MoveToNext();
      continue;
    }
    CXFA_Node* pTemplateNodeBind =
        pCurTemplateNode->GetFirstChildByClass(XFA_ELEMENT_Bind);
    XFA_ATTRIBUTEENUM eMatch =
        pTemplateNodeBind ? pTemplateNodeBind->GetEnum(XFA_ATTRIBUTE_Match)
                          : XFA_ATTRIBUTEENUM_Once;
    eBindMatch = eMatch;
    switch (eMatch) {
      case XFA_ATTRIBUTEENUM_None:
        pCurTemplateNode = pIterator->MoveToNext();
        continue;
      case XFA_ATTRIBUTEENUM_Global:
        bAccessedDataDOM = TRUE;
        if (!bForceBind) {
          pCurTemplateNode = pIterator->MoveToNext();
          continue;
        }
        if (eMatchNodeType == XFA_ELEMENT_DataValue ||
            (eMatchNodeType == XFA_ELEMENT_DataGroup &&
             XFA_FieldIsMultiListBox(pTemplateNodeBind))) {
          CXFA_Node* pGlobalBindNode = XFA_DataMerge_FindGlobalDataNode(
              pDocument, pCurTemplateNode->GetCData(XFA_ATTRIBUTE_Name),
              pDataScope, eMatchNodeType);
          if (!pGlobalBindNode) {
            pCurTemplateNode = pIterator->MoveToNext();
            continue;
          }
          pResult = pGlobalBindNode;
          break;
        }
      case XFA_ATTRIBUTEENUM_Once: {
        bAccessedDataDOM = TRUE;
        CXFA_Node* pOnceBindNode = XFA_DataMerge_FindOnceDataNode(
            pDocument, pCurTemplateNode->GetCData(XFA_ATTRIBUTE_Name),
            pDataScope, eMatchNodeType);
        if (!pOnceBindNode) {
          pCurTemplateNode = pIterator->MoveToNext();
          continue;
        }
        pResult = pOnceBindNode;
      } break;
      case XFA_ATTRIBUTEENUM_DataRef: {
        bAccessedDataDOM = TRUE;
        CXFA_Node* pDataRefBindNode = XFA_DataMerge_FindDataRefDataNode(
            pDocument, pTemplateNodeBind->GetCData(XFA_ATTRIBUTE_Ref),
            pDataScope, eMatchNodeType, pTemplateNode, bForceBind, bUpLevel);
        if (pDataRefBindNode &&
            pDataRefBindNode->GetClassID() == eMatchNodeType) {
          pResult = pDataRefBindNode;
        }
        if (!pResult) {
          pCurTemplateNode = pIterator->SkipChildrenAndMoveToNext();
          continue;
        }
      } break;
      default:
        break;
    }
    if (pCurTemplateNode == pTemplateNode && pResult != NULL) {
      bSelfMatch = TRUE;
    }
    break;
  }
  if (bOwnIterator) {
    delete pIterator;
  }
  return pResult;
}
static void XFA_DataMerge_SortRecurseRecord(
    CFX_ArrayTemplate<XFA_DataMerge_RecurseRecord>& rgRecords,
    CXFA_Node* pDataScope,
    FX_BOOL bChoiceMode = FALSE) {
  int32_t iCount = rgRecords.GetSize();
  CFX_ArrayTemplate<XFA_DataMerge_RecurseRecord> rgResultRecord;
  for (CXFA_Node* pChildNode = pDataScope->GetNodeItem(XFA_NODEITEM_FirstChild);
       pChildNode;
       pChildNode = pChildNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    for (int32_t i = 0; i < iCount; i++) {
      CXFA_Node* pNode = rgRecords[i].pDataChild;
      if (pChildNode == pNode) {
        XFA_DataMerge_RecurseRecord sNewRecord = {rgRecords[i].pTemplateChild,
                                                  pNode};
        rgResultRecord.Add(sNewRecord);
        rgRecords.RemoveAt(i);
        iCount--;
        break;
      }
    }
    if (bChoiceMode && rgResultRecord.GetSize() > 0) {
      break;
    }
  }
  if (rgResultRecord.GetSize() > 0) {
    if (!bChoiceMode) {
      for (int32_t i = 0; i < iCount; i++) {
        XFA_DataMerge_RecurseRecord sNewRecord = {rgRecords[i].pTemplateChild,
                                                  rgRecords[i].pDataChild};
        rgResultRecord.Add(sNewRecord);
      }
    }
    rgRecords.RemoveAll();
    rgRecords.Copy(rgResultRecord);
  }
}
static CXFA_Node* XFA_DataMerge_CopyContainer_SubformSet(
    CXFA_Document* pDocument,
    CXFA_Node* pTemplateNode,
    CXFA_Node* pFormParentNode,
    CXFA_Node* pDataScope,
    FX_BOOL bOneInstance,
    FX_BOOL bDataMerge) {
  XFA_ELEMENT eElement = pTemplateNode->GetClassID();
  CXFA_Node* pOccurNode = NULL;
  CXFA_Node* pFirstInstance = NULL;
  FX_BOOL bUseInstanceManager =
      pFormParentNode->GetClassID() != XFA_ELEMENT_Area;
  CXFA_Node* pInstMgrNode = NULL;
  CXFA_NodeArray subformArray;
  CXFA_NodeArray* pSearchArray = NULL;
  if (!bOneInstance &&
      (eElement == XFA_ELEMENT_SubformSet || eElement == XFA_ELEMENT_Subform)) {
    pInstMgrNode =
        bUseInstanceManager
            ? XFA_NodeMerge_CloneOrMergeInstanceManager(
                  pDocument, pFormParentNode, pTemplateNode, subformArray)
            : NULL;
    if (CXFA_Node* pOccurTemplateNode =
            pTemplateNode->GetFirstChildByClass(XFA_ELEMENT_Occur)) {
      pOccurNode = pInstMgrNode != NULL
                       ? XFA_NodeMerge_CloneOrMergeContainer(
                             pDocument, pInstMgrNode, pOccurTemplateNode, FALSE)
                       : pOccurTemplateNode;
    } else if (pInstMgrNode) {
      pOccurNode = pInstMgrNode->GetFirstChildByClass(XFA_ELEMENT_Occur);
      if (pOccurNode) {
        pOccurNode->SetFlag(XFA_NODEFLAG_UnusedNode, FALSE);
      }
    }
    if (pInstMgrNode) {
      pInstMgrNode->SetFlag(XFA_NODEFLAG_Initialized);
      pSearchArray = &subformArray;
      if (pFormParentNode->GetClassID() == XFA_ELEMENT_PageArea) {
        bOneInstance = TRUE;
        if (subformArray.GetSize() < 1) {
          pSearchArray = NULL;
        }
      } else if ((pTemplateNode->GetNameHash() == 0) &&
                 (subformArray.GetSize() < 1)) {
        pSearchArray = NULL;
      }
    }
  }
  int32_t iMax = 1, iInit = 1, iMin = 1;
  if (!bOneInstance) {
    XFA_GetOccurInfo(pOccurNode, iMin, iMax, iInit);
  }
  XFA_ATTRIBUTEENUM eRelation =
      eElement == XFA_ELEMENT_SubformSet
          ? pTemplateNode->GetEnum(XFA_ATTRIBUTE_Relation)
          : XFA_ATTRIBUTEENUM_Ordered;
  int32_t iCurRepeatIndex = 0;
  XFA_ATTRIBUTEENUM eParentBindMatch = XFA_ATTRIBUTEENUM_None;
  if (bDataMerge) {
    CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_XFAContainerNode>
        sNodeIterator(pTemplateNode);
    FX_BOOL bAccessedDataDOM = FALSE;
    if (eElement == XFA_ELEMENT_SubformSet || eElement == XFA_ELEMENT_Area) {
      sNodeIterator.MoveToNext();
    } else {
      CFX_MapPtrTemplate<CXFA_Node*, CXFA_Node*> subformMapArray;
      CXFA_NodeArray subformArray;
      for (; iMax < 0 || iCurRepeatIndex < iMax; iCurRepeatIndex++) {
        FX_BOOL bSelfMatch = FALSE;
        XFA_ATTRIBUTEENUM eBindMatch = XFA_ATTRIBUTEENUM_None;
        CXFA_Node* pDataNode = XFA_DataMerge_FindMatchingDataNode(
            pDocument, pTemplateNode, pDataScope, bAccessedDataDOM, FALSE,
            &sNodeIterator, bSelfMatch, eBindMatch);
        if (!pDataNode || sNodeIterator.GetCurrent() != pTemplateNode) {
          break;
        }
        eParentBindMatch = eBindMatch;
        CXFA_Node* pSubformNode = XFA_NodeMerge_CloneOrMergeContainer(
            pDocument, pFormParentNode, pTemplateNode, FALSE, pSearchArray);
        if (!pFirstInstance) {
          pFirstInstance = pSubformNode;
        }
        XFA_DataMerge_CreateDataBinding(pSubformNode, pDataNode);
        FXSYS_assert(pSubformNode);
        subformMapArray.SetAt(pSubformNode, pDataNode);
        subformArray.Add(pSubformNode);
      }
      subformMapArray.GetStartPosition();
      for (int32_t iIndex = 0; iIndex < subformArray.GetSize(); iIndex++) {
        CXFA_Node* pSubform = subformArray[iIndex];
        CXFA_Node* pDataNode = (CXFA_Node*)subformMapArray.GetValueAt(pSubform);
        for (CXFA_Node* pTemplateChild =
                 pTemplateNode->GetNodeItem(XFA_NODEITEM_FirstChild);
             pTemplateChild; pTemplateChild = pTemplateChild->GetNodeItem(
                                 XFA_NODEITEM_NextSibling)) {
          if (XFA_NeedGenerateForm(pTemplateChild, bUseInstanceManager)) {
            XFA_NodeMerge_CloneOrMergeContainer(pDocument, pSubform,
                                                pTemplateChild, TRUE);
          } else if (pTemplateChild->GetObjectType() ==
                     XFA_OBJECTTYPE_ContainerNode) {
            pDocument->DataMerge_CopyContainer(pTemplateChild, pSubform,
                                               pDataNode, FALSE, TRUE, FALSE);
          }
        }
      }
      subformMapArray.RemoveAll();
    }
    for (; iMax < 0 || iCurRepeatIndex < iMax; iCurRepeatIndex++) {
      FX_BOOL bSelfMatch = FALSE;
      XFA_ATTRIBUTEENUM eBindMatch = XFA_ATTRIBUTEENUM_None;
      if (!XFA_DataMerge_FindMatchingDataNode(
              pDocument, pTemplateNode, pDataScope, bAccessedDataDOM, FALSE,
              &sNodeIterator, bSelfMatch, eBindMatch)) {
        break;
      }
      if (eBindMatch == XFA_ATTRIBUTEENUM_DataRef &&
          eParentBindMatch == XFA_ATTRIBUTEENUM_DataRef) {
        break;
      }
      if (eRelation == XFA_ATTRIBUTEENUM_Choice ||
          eRelation == XFA_ATTRIBUTEENUM_Unordered) {
        CXFA_Node* pSubformSetNode = XFA_NodeMerge_CloneOrMergeContainer(
            pDocument, pFormParentNode, pTemplateNode, FALSE, pSearchArray);
        FXSYS_assert(pSubformSetNode);
        if (!pFirstInstance) {
          pFirstInstance = pSubformSetNode;
        }
        CFX_ArrayTemplate<XFA_DataMerge_RecurseRecord> rgItemMatchList;
        CFX_ArrayTemplate<CXFA_Node*> rgItemUnmatchList;
        for (CXFA_Node* pTemplateChild =
                 pTemplateNode->GetNodeItem(XFA_NODEITEM_FirstChild);
             pTemplateChild; pTemplateChild = pTemplateChild->GetNodeItem(
                                 XFA_NODEITEM_NextSibling)) {
          if (XFA_NeedGenerateForm(pTemplateChild, bUseInstanceManager)) {
            XFA_NodeMerge_CloneOrMergeContainer(pDocument, pSubformSetNode,
                                                pTemplateChild, TRUE);
          } else if (pTemplateChild->GetObjectType() ==
                     XFA_OBJECTTYPE_ContainerNode) {
            CXFA_Node* pDataMatch;
            bSelfMatch = FALSE;
            eBindMatch = XFA_ATTRIBUTEENUM_None;
            if (eRelation != XFA_ATTRIBUTEENUM_Ordered &&
                (pDataMatch = XFA_DataMerge_FindMatchingDataNode(
                     pDocument, pTemplateChild, pDataScope, bAccessedDataDOM,
                     FALSE, NULL, bSelfMatch, eBindMatch))) {
              XFA_DataMerge_RecurseRecord sNewRecord = {pTemplateChild,
                                                        pDataMatch};
              if (bSelfMatch) {
                rgItemMatchList.InsertAt(0, sNewRecord);
              } else {
                rgItemMatchList.Add(sNewRecord);
              }
            } else {
              rgItemUnmatchList.Add(pTemplateChild);
            }
          }
        }
        switch (eRelation) {
          case XFA_ATTRIBUTEENUM_Choice: {
            FXSYS_assert(rgItemMatchList.GetSize());
            XFA_DataMerge_SortRecurseRecord(rgItemMatchList, pDataScope, TRUE);
            pDocument->DataMerge_CopyContainer(
                rgItemMatchList[0].pTemplateChild, pSubformSetNode, pDataScope);
          } break;
          case XFA_ATTRIBUTEENUM_Unordered: {
            if (rgItemMatchList.GetSize()) {
              XFA_DataMerge_SortRecurseRecord(rgItemMatchList, pDataScope);
              for (int32_t i = 0, count = rgItemMatchList.GetSize(); i < count;
                   i++) {
                pDocument->DataMerge_CopyContainer(
                    rgItemMatchList[i].pTemplateChild, pSubformSetNode,
                    pDataScope);
              }
            }
            for (int32_t i = 0, count = rgItemUnmatchList.GetSize(); i < count;
                 i++) {
              pDocument->DataMerge_CopyContainer(rgItemUnmatchList[i],
                                                 pSubformSetNode, pDataScope);
            }
          } break;
          default:
            break;
        }
      } else {
        CXFA_Node* pSubformSetNode = XFA_NodeMerge_CloneOrMergeContainer(
            pDocument, pFormParentNode, pTemplateNode, FALSE, pSearchArray);
        FXSYS_assert(pSubformSetNode);
        if (!pFirstInstance) {
          pFirstInstance = pSubformSetNode;
        }
        for (CXFA_Node* pTemplateChild =
                 pTemplateNode->GetNodeItem(XFA_NODEITEM_FirstChild);
             pTemplateChild; pTemplateChild = pTemplateChild->GetNodeItem(
                                 XFA_NODEITEM_NextSibling)) {
          if (XFA_NeedGenerateForm(pTemplateChild, bUseInstanceManager)) {
            XFA_NodeMerge_CloneOrMergeContainer(pDocument, pSubformSetNode,
                                                pTemplateChild, TRUE);
          } else if (pTemplateChild->GetObjectType() ==
                     XFA_OBJECTTYPE_ContainerNode) {
            pDocument->DataMerge_CopyContainer(pTemplateChild, pSubformSetNode,
                                               pDataScope);
          }
        }
      }
    }
    if (iCurRepeatIndex == 0 && bAccessedDataDOM == FALSE) {
      int32_t iLimit = iMax;
      if (pInstMgrNode && pTemplateNode->GetNameHash() == 0) {
        iLimit = subformArray.GetSize();
        if (iLimit < iMin) {
          iLimit = iInit;
        }
      }
      for (; (iLimit < 0 || iCurRepeatIndex < iLimit); iCurRepeatIndex++) {
        if (pInstMgrNode) {
          if (pSearchArray && pSearchArray->GetSize() < 1) {
            if (pTemplateNode->GetNameHash() != 0) {
              break;
            }
            pSearchArray = NULL;
          }
        } else if (!XFA_DataMerge_FindFormDOMInstance(
                       pDocument, pTemplateNode->GetClassID(),
                       pTemplateNode->GetNameHash(), pFormParentNode)) {
          break;
        }
        CXFA_Node* pSubformNode = XFA_NodeMerge_CloneOrMergeContainer(
            pDocument, pFormParentNode, pTemplateNode, FALSE, pSearchArray);
        FXSYS_assert(pSubformNode);
        if (!pFirstInstance) {
          pFirstInstance = pSubformNode;
        }
        for (CXFA_Node* pTemplateChild =
                 pTemplateNode->GetNodeItem(XFA_NODEITEM_FirstChild);
             pTemplateChild; pTemplateChild = pTemplateChild->GetNodeItem(
                                 XFA_NODEITEM_NextSibling)) {
          if (XFA_NeedGenerateForm(pTemplateChild, bUseInstanceManager)) {
            XFA_NodeMerge_CloneOrMergeContainer(pDocument, pSubformNode,
                                                pTemplateChild, TRUE);
          } else if (pTemplateChild->GetObjectType() ==
                     XFA_OBJECTTYPE_ContainerNode) {
            pDocument->DataMerge_CopyContainer(pTemplateChild, pSubformNode,
                                               pDataScope);
          }
        }
      }
    }
  }
  int32_t iMinimalLimit = iCurRepeatIndex == 0 ? iInit : iMin;
  for (; iCurRepeatIndex < iMinimalLimit; iCurRepeatIndex++) {
    CXFA_Node* pSubformSetNode = XFA_NodeMerge_CloneOrMergeContainer(
        pDocument, pFormParentNode, pTemplateNode, FALSE, pSearchArray);
    FXSYS_assert(pSubformSetNode);
    if (!pFirstInstance) {
      pFirstInstance = pSubformSetNode;
    }
    FX_BOOL bFound = FALSE;
    for (CXFA_Node* pTemplateChild =
             pTemplateNode->GetNodeItem(XFA_NODEITEM_FirstChild);
         pTemplateChild; pTemplateChild = pTemplateChild->GetNodeItem(
                             XFA_NODEITEM_NextSibling)) {
      if (XFA_NeedGenerateForm(pTemplateChild, bUseInstanceManager)) {
        XFA_NodeMerge_CloneOrMergeContainer(pDocument, pSubformSetNode,
                                            pTemplateChild, TRUE);
      } else if (pTemplateChild->GetObjectType() ==
                 XFA_OBJECTTYPE_ContainerNode) {
        if (bFound && eRelation == XFA_ATTRIBUTEENUM_Choice) {
          continue;
        }
        pDocument->DataMerge_CopyContainer(pTemplateChild, pSubformSetNode,
                                           pDataScope, FALSE, bDataMerge);
        bFound = TRUE;
      }
    }
  }
  return pFirstInstance;
}
static CXFA_Node* XFA_DataMerge_CopyContainer_Field(CXFA_Document* pDocument,
                                                    CXFA_Node* pTemplateNode,
                                                    CXFA_Node* pFormNode,
                                                    CXFA_Node* pDataScope,
                                                    FX_BOOL bDataMerge,
                                                    FX_BOOL bUpLevel = TRUE) {
  CXFA_Node* pFieldNode = XFA_NodeMerge_CloneOrMergeContainer(
      pDocument, pFormNode, pTemplateNode, FALSE);
  FXSYS_assert(pFieldNode);
  for (CXFA_Node* pTemplateChildNode =
           pTemplateNode->GetNodeItem(XFA_NODEITEM_FirstChild);
       pTemplateChildNode; pTemplateChildNode = pTemplateChildNode->GetNodeItem(
                               XFA_NODEITEM_NextSibling)) {
    if (XFA_NeedGenerateForm(pTemplateChildNode)) {
      XFA_NodeMerge_CloneOrMergeContainer(pDocument, pFieldNode,
                                          pTemplateChildNode, TRUE);
    } else if (pTemplateNode->GetClassID() == XFA_ELEMENT_ExclGroup &&
               pTemplateChildNode->IsContainerNode()) {
      if (pTemplateChildNode->GetClassID() == XFA_ELEMENT_Field) {
        XFA_DataMerge_CopyContainer_Field(pDocument, pTemplateChildNode,
                                          pFieldNode, NULL, FALSE);
      }
    }
  }
  if (bDataMerge) {
    FX_BOOL bAccessedDataDOM = FALSE;
    FX_BOOL bSelfMatch = FALSE;
    XFA_ATTRIBUTEENUM eBindMatch;
    CXFA_Node* pDataNode = XFA_DataMerge_FindMatchingDataNode(
        pDocument, pTemplateNode, pDataScope, bAccessedDataDOM, TRUE, NULL,
        bSelfMatch, eBindMatch, bUpLevel);
    if (pDataNode) {
      XFA_DataMerge_CreateDataBinding(pFieldNode, pDataNode);
    }
  } else {
    XFA_DataMerge_FormValueNode_MatchNoneCreateChild(pFieldNode);
  }
  return pFieldNode;
}
CXFA_Node* CXFA_Document::DataMerge_CopyContainer(CXFA_Node* pTemplateNode,
                                                  CXFA_Node* pFormNode,
                                                  CXFA_Node* pDataScope,
                                                  FX_BOOL bOneInstance,
                                                  FX_BOOL bDataMerge,
                                                  FX_BOOL bUpLevel) {
  switch (pTemplateNode->GetClassID()) {
    case XFA_ELEMENT_SubformSet:
    case XFA_ELEMENT_Subform:
    case XFA_ELEMENT_Area:
    case XFA_ELEMENT_PageArea:
      return XFA_DataMerge_CopyContainer_SubformSet(
          this, pTemplateNode, pFormNode, pDataScope, bOneInstance, bDataMerge);
    case XFA_ELEMENT_ExclGroup:
    case XFA_ELEMENT_Field:
    case XFA_ELEMENT_Draw:
    case XFA_ELEMENT_ContentArea:
      return XFA_DataMerge_CopyContainer_Field(
          this, pTemplateNode, pFormNode, pDataScope, bDataMerge, bUpLevel);
    case XFA_ELEMENT_PageSet:
      break;
    case XFA_ELEMENT_Variables:
      break;
    default:
      FXSYS_assert(FALSE);
      break;
  }
  return NULL;
}
#define XFA_DATAMERGE_UPDATEBINDINGRELATIONS_DFS
#ifdef XFA_DATAMERGE_UPDATEBINDINGRELATIONS_DFS
static void XFA_DataMerge_UpdateBindingRelations(CXFA_Document* pDocument,
                                                 CXFA_Node* pFormNode,
                                                 CXFA_Node* pDataScope,
                                                 FX_BOOL bDataRef,
                                                 FX_BOOL bParentDataRef) {
  FX_BOOL bMatchRef = TRUE;
  XFA_ELEMENT eClassID = pFormNode->GetClassID();
  CXFA_Node* pDataNode = pFormNode->GetBindData();
  if (eClassID == XFA_ELEMENT_Subform || eClassID == XFA_ELEMENT_ExclGroup ||
      eClassID == XFA_ELEMENT_Field) {
    CXFA_Node* pTemplateNode = pFormNode->GetTemplateNode();
    CXFA_Node* pTemplateNodeBind =
        pTemplateNode ? pTemplateNode->GetFirstChildByClass(XFA_ELEMENT_Bind)
                      : NULL;
    XFA_ATTRIBUTEENUM eMatch =
        pTemplateNodeBind ? pTemplateNodeBind->GetEnum(XFA_ATTRIBUTE_Match)
                          : XFA_ATTRIBUTEENUM_Once;
    switch (eMatch) {
      case XFA_ATTRIBUTEENUM_None:
        if (!bDataRef || bParentDataRef) {
          XFA_DataMerge_FormValueNode_MatchNoneCreateChild(pFormNode);
        }
        break;
      case XFA_ATTRIBUTEENUM_Once:
        if (!bDataRef || bParentDataRef) {
          if (!pDataNode) {
            if (pFormNode->GetNameHash() != 0 &&
                pFormNode->GetEnum(XFA_ATTRIBUTE_Scope) !=
                    XFA_ATTRIBUTEENUM_None) {
              XFA_ELEMENT eDataNodeType = (eClassID == XFA_ELEMENT_Subform ||
                                           XFA_FieldIsMultiListBox(pFormNode))
                                              ? XFA_ELEMENT_DataGroup
                                              : XFA_ELEMENT_DataValue;
              pDataNode = XFA_DataDescription_MaybeCreateDataNode(
                  pDocument, pDataScope, eDataNodeType,
                  pFormNode->GetCData(XFA_ATTRIBUTE_Name));
              if (pDataNode) {
                XFA_DataMerge_CreateDataBinding(pFormNode, pDataNode, FALSE);
              }
            }
            if (!pDataNode) {
              XFA_DataMerge_FormValueNode_MatchNoneCreateChild(pFormNode);
            }
          } else {
            CXFA_Node* pDataParent =
                pDataNode->GetNodeItem(XFA_NODEITEM_Parent);
            if (pDataParent != pDataScope) {
              FXSYS_assert(pDataParent);
              pDataParent->RemoveChild(pDataNode);
              pDataScope->InsertChild(pDataNode);
            }
          }
        }
        break;
      case XFA_ATTRIBUTEENUM_Global:
        if (!bDataRef || bParentDataRef) {
          FX_DWORD dwNameHash = pFormNode->GetNameHash();
          if (dwNameHash != 0 && !pDataNode) {
            pDataNode = XFA_DataMerge_GetGlobalBinding(pDocument, dwNameHash);
            if (!pDataNode) {
              XFA_ELEMENT eDataNodeType = (eClassID == XFA_ELEMENT_Subform ||
                                           XFA_FieldIsMultiListBox(pFormNode))
                                              ? XFA_ELEMENT_DataGroup
                                              : XFA_ELEMENT_DataValue;
              CXFA_Node* pRecordNode =
                  (CXFA_Node*)pDocument->GetXFANode(XFA_HASHCODE_Record);
              pDataNode = XFA_DataDescription_MaybeCreateDataNode(
                  pDocument, pRecordNode, eDataNodeType,
                  pFormNode->GetCData(XFA_ATTRIBUTE_Name));
              if (pDataNode) {
                XFA_DataMerge_CreateDataBinding(pFormNode, pDataNode, FALSE);
                XFA_DataMerge_RegisterGlobalBinding(
                    pDocument, pFormNode->GetNameHash(), pDataNode);
              }
            } else {
              XFA_DataMerge_CreateDataBinding(pFormNode, pDataNode);
            }
          }
          if (!pDataNode) {
            XFA_DataMerge_FormValueNode_MatchNoneCreateChild(pFormNode);
          }
        }
        break;
      case XFA_ATTRIBUTEENUM_DataRef: {
        bMatchRef = bDataRef;
        bParentDataRef = TRUE;
        if (!pDataNode && bDataRef) {
          CFX_WideStringC wsRef =
              pTemplateNodeBind->GetCData(XFA_ATTRIBUTE_Ref);
          FX_DWORD dFlags =
              XFA_RESOLVENODE_Children | XFA_RESOLVENODE_CreateNode;
          XFA_RESOLVENODE_RS rs;
          pDocument->GetScriptContext()->ResolveObjects(pDataScope, wsRef, rs,
                                                        dFlags, pTemplateNode);
          CXFA_Object* pObject = (rs.nodes.GetSize() > 0) ? rs.nodes[0] : NULL;
          pDataNode =
              (pObject && pObject->IsNode()) ? (CXFA_Node*)pObject : NULL;
          if (pDataNode) {
            XFA_DataMerge_CreateDataBinding(
                pFormNode, pDataNode,
                rs.dwFlags == XFA_RESOVENODE_RSTYPE_ExistNodes);
          } else {
            XFA_DataMerge_FormValueNode_MatchNoneCreateChild(pFormNode);
          }
        }
      } break;
      default:
        break;
    }
  }
  if (bMatchRef &&
      (eClassID == XFA_ELEMENT_Subform || eClassID == XFA_ELEMENT_SubformSet ||
       eClassID == XFA_ELEMENT_Area || eClassID == XFA_ELEMENT_PageArea ||
       eClassID == XFA_ELEMENT_PageSet)) {
    for (CXFA_Node* pFormChild =
             pFormNode->GetNodeItem(XFA_NODEITEM_FirstChild);
         pFormChild;
         pFormChild = pFormChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
      if (pFormChild->GetObjectType() != XFA_OBJECTTYPE_ContainerNode) {
        continue;
      }
      if (pFormChild->HasFlag(XFA_NODEFLAG_UnusedNode)) {
        continue;
      }
      XFA_DataMerge_UpdateBindingRelations(pDocument, pFormChild,
                                           pDataNode ? pDataNode : pDataScope,
                                           bDataRef, bParentDataRef);
    }
  }
}
#else
static void XFA_DataMerge_UpdateBindingRelations(CXFA_Document* pDocument, CXFA_Node* pFormNode, CXFA_Node* pDataScope, CFX_PtrList& rgFormNodeList, CFX_PtrList& rgDataScopeList, FX_BOOL bD _DEBUG
#ifdef _DEBUG
        CFX_WideString wsFormSOM; CFX_WideString wsDataScopeSOM;
        pFormNode->GetSOMExpression(wsFormSOM); pDataScope->GetSOMExpression(wsDataScopeSOM);
#endif
        XFA_ELEMENT eClassID = pFormNode->GetClassID();
        CXFA_Node* pDataNode = pFormNode->GetBindData();
        if(eClassID == XFA_ELEMENT_Subform || eClassID == XFA_ELEMENT_ExclGroup || eClassID == XFA_ELEMENT_Field)
{
  CXFA_Node* pTemplateNode = pFormNode->GetTemplateNode();
  CXFA_Node* pTemplateNodeBind =
      pTemplateNode ? pTemplateNode->GetFirstChildByClass(XFA_ELEMENT_Bind)
                    : NULL;
  XFA_ATTRIBUTEENUM eMatch =
      pTemplateNodeBind ? pTemplateNodeBind->GetEnum(XFA_ATTRIBUTE_Match)
                        : XFA_ATTRIBUTEENUM_Once;
  switch (eMatch) {
    case XFA_ATTRIBUTEENUM_None:
      break;
    case XFA_ATTRIBUTEENUM_Once: {
      if (!pDataNode) {
        if (pFormNode->GetNameHash() != 0 &&
            pFormNode->GetEnum(XFA_ATTRIBUTE_Scope) != XFA_ATTRIBUTEENUM_None) {
          XFA_ELEMENT eDataNodeType = eClassID == XFA_ELEMENT_Subform
                                          ? XFA_ELEMENT_DataGroup
                                          : XFA_ELEMENT_DataValue;
          pDataNode = XFA_DataDescription_MaybeCreateDataNode(
              pDocument, pDataScope, eDataNodeType,
              pFormNode->GetCData(XFA_ATTRIBUTE_Name));
          if (pDataNode) {
            XFA_DataMerge_CreateDataBinding(pFormNode, pDataNode, FALSE);
          }
        }
      } else {
        CXFA_Node* pDataParent = pDataNode->GetNodeItem(XFA_NODEITEM_Parent);
        if (pDataParent != pDataScope) {
          FXSYS_assert(pDataParent);
          pDataParent->RemoveChild(pDataNode);
          pDataScope->InsertChild(pDataNode);
        }
      }
    } break;
    case XFA_ATTRIBUTEENUM_Global: {
      FX_DWORD dwNameHash = pFormNode->GetNameHash();
      if (dwNameHash != 0 && !pDataNode) {
        pDataNode = XFA_DataMerge_GetGlobalBinding(pDocument, dwNameHash);
        if (!pDataNode) {
          XFA_ELEMENT eDataNodeType = eClassID == XFA_ELEMENT_Subform
                                          ? XFA_ELEMENT_DataGroup
                                          : XFA_ELEMENT_DataValue;
          CXFA_Node* pRecordNode =
              (CXFA_Node*)pDocument->GetXFANode(XFA_HASHCODE_Record);
          pDataNode = XFA_DataDescription_MaybeCreateDataNode(
              pDocument, pRecordNode, eDataNodeType,
              pFormNode->GetCData(XFA_ATTRIBUTE_Name));
        }
        if (pDataNode) {
          XFA_DataMerge_CreateDataBinding(pFormNode, pDataNode, FALSE);
          XFA_DataMerge_RegisterGlobalBinding(
              pDocument, pFormNode->GetNameHash(), pDataNode);
        }
      }
    } break;
    case XFA_ATTRIBUTEENUM_DataRef: {
      if (!pDataNode) {
        CFX_WideStringC wsRef = pTemplateNodeBind->GetCData(XFA_ATTRIBUTE_Ref);
        FX_DWORD dFlags = XFA_RESOLVENODE_Children |
                          XFA_RESOLVENODE_Attributes |
                          XFA_RESOLVENODE_Siblings | XFA_RESOLVENODE_Parent |
                          XFA_RESOLVENODE_CreateNode;
        XFA_RESOLVENODE_RS rs;
        pDocument->GetScriptContext()->ResolveObjects(pDataScope, wsRef, rs,
                                                      dFlags, pTemplateNode);
        CXFA_Object* pObject = (rs.nodes.GetSize() > 0) ? rs.nodes[0] : NULL;
        pDataNode = (pObject && pObject->IsNode()) ? (CXFA_Node*)pObject : NULL;
        if (pDataNode) {
          XFA_DataMerge_CreateDataBinding(pFormNode, pDataNode, FALSE);
        }
      }
    } break;
  }
}
if(eClassID == XFA_ELEMENT_Subform || eClassID == XFA_ELEMENT_ExclGroup || eClassID == XFA_ELEMENT_SubformSet || eClassID == XFA_ELEMENT_Area || eClassID == XFA_ELEMENT_PageArea)
{
  for (CXFA_Node* pFormChild = pFormNode->GetNodeItem(XFA_NODEITEM_FirstChild);
       pFormChild;
       pFormChild = pFormChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pFormChild->GetObjectType() != XFA_OBJECTTYPE_ContainerNode) {
      continue;
    }
    if (pFormChild->HasFlag(XFA_NODEFLAG_UnusedNode)) {
      continue;
    }
    rgFormNodeList.AddTail(pFormChild);
    rgDataScopeList.AddTail(pDataNode ? pDataNode : pDataScope);
  }
}
}
#endif
CXFA_Node* XFA_DataMerge_FindDataScope(CXFA_Node* pParentFormNode) {
  for (CXFA_Node* pRootBoundNode = pParentFormNode;
       pRootBoundNode &&
       pRootBoundNode->GetObjectType() == XFA_OBJECTTYPE_ContainerNode;
       pRootBoundNode = pRootBoundNode->GetNodeItem(XFA_NODEITEM_Parent)) {
    CXFA_Node* pDataScope = pRootBoundNode->GetBindData();
    if (pDataScope) {
      return pDataScope;
    }
  }
  return (CXFA_Node*)pParentFormNode->GetDocument()->GetXFANode(
      XFA_HASHCODE_Data);
}
void CXFA_Document::DataMerge_UpdateBindingRelations(
    CXFA_Node* pFormUpdateRoot) {
  CXFA_Node* pDataScope = XFA_DataMerge_FindDataScope(
      pFormUpdateRoot->GetNodeItem(XFA_NODEITEM_Parent));
  if (!pDataScope) {
    return;
  }
#ifdef XFA_DATAMERGE_UPDATEBINDINGRELATIONS_DFS
  XFA_DataMerge_UpdateBindingRelations(this, pFormUpdateRoot, pDataScope, FALSE,
                                       FALSE);
  XFA_DataMerge_UpdateBindingRelations(this, pFormUpdateRoot, pDataScope, TRUE,
                                       FALSE);
#else
  CFX_PtrList rgFormNodeList, rgDataScopeList;
  rgFormNodeList.AddTail(pFormUpdateRoot);
  rgDataScopeList.AddTail(pDataScope);
  while (rgFormNodeList.GetCount()) {
    FX_POSITION pos;
    pos = rgFormNodeList.GetHeadPosition();
    CXFA_Node* pCurFormNode = (CXFA_Node*)rgFormNodeList.GetAt(pos);
    rgFormNodeList.RemoveAt(pos);
    pos = rgDataScopeList.GetHeadPosition();
    CXFA_Node* pCurDataScope = (CXFA_Node*)rgDataScopeList.GetAt(pos);
    rgDataScopeList.RemoveAt(pos);
    XFA_DataMerge_UpdateBindingRelations(this, pCurFormNode, pCurDataScope,
                                         rgFormNodeList, rgDataScopeList);
  }
#endif
}
CXFA_Node* CXFA_Document::GetNotBindNode(CXFA_ObjArray& arrayNodes) {
  for (int32_t i = 0; i < arrayNodes.GetSize(); i++) {
    CXFA_Object* pObject = arrayNodes[i];
    if (!pObject->IsNode()) {
      continue;
    }
    if (((CXFA_Node*)pObject)->HasBindItem()) {
      continue;
    }
    return ((CXFA_Node*)pObject);
  }
  return NULL;
}
void CXFA_Document::DoDataMerge() {
  CXFA_Node* pDatasetsRoot = (CXFA_Node*)GetXFANode(XFA_HASHCODE_Datasets);
  if (!pDatasetsRoot) {
    IFDE_XMLElement* pDatasetsXMLNode =
        IFDE_XMLElement::Create(FX_WSTRC(L"xfa:datasets"));
    FXSYS_assert(pDatasetsXMLNode);
    pDatasetsXMLNode->SetString(
        FX_WSTRC(L"xmlns:xfa"),
        FX_WSTRC(L"http://www.xfa.org/schema/xfa-data/1.0/"));
    pDatasetsRoot = CreateNode(XFA_XDPPACKET_Datasets, XFA_ELEMENT_DataModel);
    pDatasetsRoot->SetCData(XFA_ATTRIBUTE_Name, FX_WSTRC(L"datasets"));
    m_pRootNode->GetXMLMappingNode()->InsertChildNode(pDatasetsXMLNode);
    m_pRootNode->InsertChild(pDatasetsRoot);
    pDatasetsRoot->SetXMLMappingNode(pDatasetsXMLNode);
  }
  CXFA_Node *pDataRoot = NULL, *pDDRoot = NULL;
  CFX_WideString wsDatasetsURI;
  pDatasetsRoot->TryNamespace(wsDatasetsURI);
  for (CXFA_Node* pChildNode =
           pDatasetsRoot->GetNodeItem(XFA_NODEITEM_FirstChild);
       pChildNode;
       pChildNode = pChildNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pChildNode->GetClassID() != XFA_ELEMENT_DataGroup) {
      continue;
    }
    CFX_WideString wsNamespaceURI;
    if (!pDDRoot && pChildNode->GetNameHash() == XFA_HASHCODE_DataDescription) {
      if (!pChildNode->TryNamespace(wsNamespaceURI)) {
        continue;
      }
      if (wsNamespaceURI ==
          FX_WSTRC(L"http://ns.adobe.com/data-description/")) {
        pDDRoot = pChildNode;
      }
    } else if (!pDataRoot && pChildNode->GetNameHash() == XFA_HASHCODE_Data) {
      if (!pChildNode->TryNamespace(wsNamespaceURI)) {
        continue;
      }
      if (wsNamespaceURI == wsDatasetsURI) {
        pDataRoot = pChildNode;
      }
    }
    if (pDataRoot && pDDRoot) {
      break;
    }
  }
  if (!pDataRoot) {
    IFDE_XMLElement* pDataRootXMLNode =
        IFDE_XMLElement::Create(FX_WSTRC(L"xfa:data"));
    FXSYS_assert(pDataRootXMLNode);
    pDataRoot = CreateNode(XFA_XDPPACKET_Datasets, XFA_ELEMENT_DataGroup);
    pDataRoot->SetCData(XFA_ATTRIBUTE_Name, FX_WSTRC(L"data"));
    pDataRoot->SetXMLMappingNode(pDataRootXMLNode);
    pDatasetsRoot->InsertChild(pDataRoot);
  }
  CXFA_Node* pDataTopLevel =
      pDataRoot->GetFirstChildByClass(XFA_ELEMENT_DataGroup);
  FX_DWORD dwNameHash = pDataTopLevel ? pDataTopLevel->GetNameHash() : 0;
  CXFA_Node* pTemplateRoot =
      m_pRootNode->GetFirstChildByClass(XFA_ELEMENT_Template);
  if (!pTemplateRoot) {
    return;
  }
  CXFA_Node* pTemplateChosen =
      dwNameHash != 0 ? pTemplateRoot->GetFirstChildByName(dwNameHash) : NULL;
  if (!pTemplateChosen ||
      pTemplateChosen->GetClassID() != XFA_ELEMENT_Subform) {
    pTemplateChosen = pTemplateRoot->GetFirstChildByClass(XFA_ELEMENT_Subform);
  }
  if (!pTemplateChosen) {
    return;
  }
  CXFA_Node* pFormRoot = m_pRootNode->GetFirstChildByClass(XFA_ELEMENT_Form);
  FX_BOOL bEmptyForm = FALSE;
  if (!pFormRoot) {
    bEmptyForm = TRUE;
    pFormRoot = CreateNode(XFA_XDPPACKET_Form, XFA_ELEMENT_Form);
    FXSYS_assert(pFormRoot);
    pFormRoot->SetCData(XFA_ATTRIBUTE_Name, FX_WSTRC(L"form"));
    m_pRootNode->InsertChild(pFormRoot, NULL);
  } else {
    CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_XFANode>
        sIterator(pFormRoot);
    for (CXFA_Node* pNode = sIterator.MoveToNext(); pNode;
         pNode = sIterator.MoveToNext()) {
      pNode->SetFlag(XFA_NODEFLAG_UnusedNode);
    }
  }
  CXFA_Node* pSubformSetNode = XFA_NodeMerge_CloneOrMergeContainer(
      this, pFormRoot, pTemplateChosen, FALSE);
  FXSYS_assert(pSubformSetNode);
  if (!pDataTopLevel) {
    CFX_WideStringC wsFormName = pSubformSetNode->GetCData(XFA_ATTRIBUTE_Name);
    CFX_WideString wsDataTopLevelName =
        wsFormName.IsEmpty() ? FX_WSTRC(L"form") : wsFormName;
    IFDE_XMLElement* pDataTopLevelXMLNode =
        IFDE_XMLElement::Create(wsDataTopLevelName);
    FXSYS_assert(pDataTopLevelXMLNode);
    pDataTopLevel = CreateNode(XFA_XDPPACKET_Datasets, XFA_ELEMENT_DataGroup);
    pDataTopLevel->SetCData(XFA_ATTRIBUTE_Name, wsDataTopLevelName);
    pDataTopLevel->SetXMLMappingNode(pDataTopLevelXMLNode);
    CXFA_Node* pBeforeNode = pDataRoot->GetNodeItem(XFA_NODEITEM_FirstChild);
    pDataRoot->InsertChild(pDataTopLevel, pBeforeNode);
  }
  FXSYS_assert(pDataTopLevel);
  XFA_DataMerge_CreateDataBinding(pSubformSetNode, pDataTopLevel);
  for (CXFA_Node* pTemplateChild =
           pTemplateChosen->GetNodeItem(XFA_NODEITEM_FirstChild);
       pTemplateChild;
       pTemplateChild = pTemplateChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (XFA_NeedGenerateForm(pTemplateChild)) {
      XFA_NodeMerge_CloneOrMergeContainer(this, pSubformSetNode, pTemplateChild,
                                          TRUE);
    } else if (pTemplateChild->GetObjectType() ==
               XFA_OBJECTTYPE_ContainerNode) {
      DataMerge_CopyContainer(pTemplateChild, pSubformSetNode, pDataTopLevel);
    }
  }
  if (pDDRoot) {
    XFA_DataDescription_UpdateDataRelation(pDataRoot, pDDRoot);
  }
  DataMerge_UpdateBindingRelations(pSubformSetNode);
  CXFA_Node* pPageSetNode =
      pSubformSetNode->GetFirstChildByClass(XFA_ELEMENT_PageSet);
  while (pPageSetNode) {
    m_pPendingPageSet.Add(pPageSetNode);
    CXFA_Node* pNextPageSetNode =
        pPageSetNode->GetNextSameClassSibling(XFA_ELEMENT_PageSet);
    pSubformSetNode->RemoveChild(pPageSetNode);
    pPageSetNode = pNextPageSetNode;
  }
  if (!bEmptyForm) {
    CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_XFANode>
        sIterator(pFormRoot);
    CXFA_Node* pNode = sIterator.MoveToNext();
    while (pNode) {
      if (pNode->HasFlag(XFA_NODEFLAG_UnusedNode)) {
        if (pNode->GetObjectType() == XFA_OBJECTTYPE_ContainerNode ||
            pNode->GetClassID() == XFA_ELEMENT_InstanceManager) {
          CXFA_Node* pNext = sIterator.SkipChildrenAndMoveToNext();
          pNode->GetNodeItem(XFA_NODEITEM_Parent)->RemoveChild(pNode);
          pNode = pNext;
        } else {
          pNode->SetFlag(XFA_NODEFLAG_UnusedNode, FALSE);
          pNode->SetFlag(XFA_NODEFLAG_Initialized);
          pNode = sIterator.MoveToNext();
        }
      } else {
        pNode->SetFlag(XFA_NODEFLAG_Initialized);
        pNode = sIterator.MoveToNext();
      }
    }
  }
}
void CXFA_Document::DoDataRemerge(FX_BOOL bDoDataMerge) {
  CXFA_Node* pFormRoot = (CXFA_Node*)this->GetXFANode(XFA_HASHCODE_Form);
  if (pFormRoot) {
    while (CXFA_Node* pNode = pFormRoot->GetNodeItem(XFA_NODEITEM_FirstChild)) {
      pFormRoot->RemoveChild(pNode);
    }
    pFormRoot->SetObject(XFA_ATTRIBUTE_BindingNode, NULL);
  }
  XFA_DataMerge_ClearGlobalBinding(this);
  if (bDoDataMerge) {
    DoDataMerge();
  }
  CXFA_LayoutProcessor* pLayoutProcessor = GetLayoutProcessor();
  pLayoutProcessor->SetForceReLayout(TRUE);
}
