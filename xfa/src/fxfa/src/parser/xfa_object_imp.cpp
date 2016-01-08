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
CXFA_Object::CXFA_Object(CXFA_Document* pDocument, FX_DWORD uFlags)
    : m_pDocument(pDocument), m_uFlags(uFlags) {}
void CXFA_Object::GetClassName(CFX_WideStringC& wsName) const {
  XFA_LPCELEMENTINFO pElement = XFA_GetElementByID(GetClassID());
  ASSERT(pElement != NULL);
  wsName = pElement->pName;
}
uint32_t CXFA_Object::GetClassHashCode() const {
  XFA_LPCELEMENTINFO pElement = XFA_GetElementByID(GetClassID());
  ASSERT(pElement != NULL);
  return pElement->uHash;
}
XFA_ELEMENT CXFA_Object::GetClassID() const {
  if (IsNode()) {
    return ((const CXFA_Node*)this)->GetClassID();
  } else if (IsOrdinaryObject()) {
    return ((const CXFA_OrdinaryObject*)this)->GetClassID();
  } else if (IsNodeList()) {
    return ((const CXFA_NodeList*)this)->GetClassID();
  } else if (IsOrdinaryList()) {
    return XFA_ELEMENT_List;
  }
  ASSERT(FALSE);
  return (XFA_ELEMENT)0;
}
void CXFA_Object::Script_ObjectClass_ClassName(FXJSE_HVALUE hValue,
                                               FX_BOOL bSetting,
                                               XFA_ATTRIBUTE eAttribute) {
  if (!bSetting) {
    CFX_WideStringC className;
    GetClassName(className);
    FXJSE_Value_SetUTF8String(
        hValue, FX_UTF8Encode(className.GetPtr(), className.GetLength()));
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
  }
}
void CXFA_Object::ThrowScriptErrorMessage(int32_t iStringID, ...) {
  IXFA_AppProvider* pAppProvider = m_pDocument->GetNotify()->GetAppProvider();
  FXSYS_assert(pAppProvider);
  CFX_WideString wsFormat;
  pAppProvider->LoadString(iStringID, wsFormat);
  CFX_WideString wsMessage;
  va_list arg_ptr;
  va_start(arg_ptr, iStringID);
  wsMessage.FormatV((const FX_WCHAR*)wsFormat, arg_ptr);
  va_end(arg_ptr);
  FXJSE_ThrowMessage("", FX_UTF8Encode(wsMessage, wsMessage.GetLength()));
}
static void XFA_DeleteWideString(void* pData) {
  if (pData) {
    delete (CFX_WideString*)pData;
  }
}
static void XFA_CopyWideString(void*& pData) {
  if (pData) {
    CFX_WideString* pNewData = new CFX_WideString(*(CFX_WideString*)pData);
    pData = pNewData;
  }
}
static XFA_MAPDATABLOCKCALLBACKINFO deleteWideStringCallBack = {
    XFA_DeleteWideString, XFA_CopyWideString};
static XFA_OBJECTTYPE XFA_GetElementObjectType(XFA_ELEMENT eElement) {
  XFA_LPCELEMENTINFO pElement = XFA_GetElementByID(eElement);
  ASSERT(pElement != NULL);
  return (XFA_OBJECTTYPE)pElement->eObjectType;
}
CXFA_Node::CXFA_Node(CXFA_Document* pDoc, FX_WORD ePacket, XFA_ELEMENT eElement)
    : CXFA_Object(pDoc, XFA_GetElementObjectType(eElement)),
      m_pNext(nullptr),
      m_pChild(nullptr),
      m_pLastChild(nullptr),
      m_pParent(nullptr),
      m_pXMLNode(nullptr),
      m_eNodeClass(eElement),
      m_ePacket(ePacket),
      m_dwNameHash(0),
      m_pAuxNode(nullptr),
      m_pMapModuleData(nullptr) {
  ASSERT(m_pDocument);
}
CXFA_Node::~CXFA_Node() {
  FXSYS_assert(m_pParent == NULL);
  RemoveMapModuleKey();
  CXFA_Node *pNext, *pNode = m_pChild;
  while (pNode) {
    pNext = pNode->m_pNext;
    pNode->m_pParent = NULL;
    delete pNode;
    pNode = pNext;
  }
  if (m_pXMLNode && HasFlag(XFA_NODEFLAG_OwnXMLNode)) {
    m_pXMLNode->Release();
  }
}
CXFA_Node* CXFA_Node::Clone(FX_BOOL bRecursive) {
  IXFA_ObjFactory* pFactory = m_pDocument->GetParser()->GetFactory();
  CXFA_Node* pClone = pFactory->CreateNode(m_ePacket, m_eNodeClass);
  if (!pClone) {
    return NULL;
  }
  MergeAllData(pClone);
  pClone->UpdateNameHash();
  if (IsNeedSavingXMLNode()) {
    IFDE_XMLNode* pCloneXML = NULL;
    if (IsAttributeInXML()) {
      CFX_WideString wsName;
      this->GetAttribute(XFA_ATTRIBUTE_Name, wsName, FALSE);
      IFDE_XMLElement* pCloneXMLElement = IFDE_XMLElement::Create(wsName);
      CFX_WideStringC wsValue = this->GetCData(XFA_ATTRIBUTE_Value);
      if (!wsValue.IsEmpty()) {
        pCloneXMLElement->SetTextData(wsValue);
      }
      pCloneXML = pCloneXMLElement;
      pCloneXMLElement = NULL;
      pClone->SetEnum(XFA_ATTRIBUTE_Contains, XFA_ATTRIBUTEENUM_Unknown);
    } else {
      pCloneXML = m_pXMLNode->Clone(FALSE);
    }
    pClone->SetXMLMappingNode(pCloneXML);
    pClone->SetFlag(XFA_NODEFLAG_OwnXMLNode, TRUE, FALSE);
  }
  if (bRecursive) {
    for (CXFA_Node* pChild = GetNodeItem(XFA_NODEITEM_FirstChild); pChild;
         pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
      pClone->InsertChild(pChild->Clone(bRecursive));
    }
  }
  pClone->SetFlag(XFA_NODEFLAG_Initialized);
  pClone->SetObject(XFA_ATTRIBUTE_BindingNode, NULL);
  return pClone;
}
CXFA_Node* CXFA_Node::GetNodeItem(XFA_NODEITEM eItem) const {
  switch (eItem) {
    case XFA_NODEITEM_NextSibling:
      return m_pNext;
    case XFA_NODEITEM_FirstChild:
      return m_pChild;
    case XFA_NODEITEM_Parent:
      return m_pParent;
    case XFA_NODEITEM_PrevSibling:
      if (m_pParent) {
        CXFA_Node* pSibling = m_pParent->m_pChild;
        CXFA_Node* pPrev = NULL;
        while (pSibling && pSibling != this) {
          pPrev = pSibling;
          pSibling = pSibling->m_pNext;
        }
        return pPrev;
      }
      return NULL;
    default:
      break;
  }
  return NULL;
}
CXFA_Node* CXFA_Node::GetNodeItem(XFA_NODEITEM eItem,
                                  XFA_OBJECTTYPE eType) const {
  CXFA_Node* pNode = NULL;
  switch (eItem) {
    case XFA_NODEITEM_NextSibling:
      pNode = m_pNext;
      if (eType != XFA_OBJECTTYPEMASK)
        while (pNode && pNode->GetObjectType() != eType) {
          pNode = pNode->m_pNext;
        }
      break;
    case XFA_NODEITEM_FirstChild:
      pNode = m_pChild;
      if (eType != XFA_OBJECTTYPEMASK)
        while (pNode && pNode->GetObjectType() != eType) {
          pNode = pNode->m_pNext;
        }
      break;
    case XFA_NODEITEM_Parent:
      pNode = m_pParent;
      if (eType != XFA_OBJECTTYPEMASK)
        while (pNode && pNode->GetObjectType() != eType) {
          pNode = pNode->m_pParent;
        }
      break;
    case XFA_NODEITEM_PrevSibling:
      if (m_pParent) {
        CXFA_Node* pSibling = m_pParent->m_pChild;
        while (pSibling && pSibling != this) {
          if (eType == XFA_OBJECTTYPEMASK ||
              eType == pSibling->GetObjectType()) {
            pNode = pSibling;
          }
          pSibling = pSibling->m_pNext;
        }
      }
      break;
    default:
      break;
  }
  return pNode;
}
int32_t CXFA_Node::GetNodeList(CXFA_NodeArray& nodes,
                               FX_DWORD dwTypeFilter,
                               XFA_ELEMENT eElementFilter,
                               int32_t iLevel) {
  if (--iLevel < 0) {
    return nodes.GetSize();
  }
  if (eElementFilter != XFA_ELEMENT_UNKNOWN) {
    CXFA_Node* pChild = m_pChild;
    while (pChild) {
      if (pChild->GetClassID() == eElementFilter) {
        nodes.Add(pChild);
        if (iLevel > 0) {
          GetNodeList(nodes, dwTypeFilter, eElementFilter, iLevel);
        }
      }
      pChild = pChild->m_pNext;
    }
  } else if (dwTypeFilter ==
             (XFA_NODEFILTER_Children | XFA_NODEFILTER_Properties)) {
    CXFA_Node* pChild = m_pChild;
    while (pChild) {
      nodes.Add(pChild);
      if (iLevel > 0) {
        GetNodeList(nodes, dwTypeFilter, eElementFilter, iLevel);
      }
      pChild = pChild->m_pNext;
    }
  } else if (dwTypeFilter != 0) {
    FX_BOOL bFilterChildren = (dwTypeFilter & XFA_NODEFILTER_Children) != 0;
    FX_BOOL bFilterProperties = (dwTypeFilter & XFA_NODEFILTER_Properties) != 0;
    FX_BOOL bFilterOneOfProperties =
        (dwTypeFilter & XFA_NODEFILTER_OneOfProperty) != 0;
    CXFA_Node* pChild = m_pChild;
    while (pChild) {
      XFA_LPCPROPERTY pPropert = XFA_GetPropertyOfElement(
          GetClassID(), pChild->GetClassID(), XFA_XDPPACKET_UNKNOWN);
      if (pPropert) {
        if (bFilterProperties) {
          nodes.Add(pChild);
        } else if (bFilterOneOfProperties &&
                   (pPropert->uFlags & XFA_PROPERTYFLAG_OneOf)) {
          nodes.Add(pChild);
        } else if (bFilterChildren &&
                   (pChild->GetClassID() == XFA_ELEMENT_Variables ||
                    pChild->GetClassID() == XFA_ELEMENT_PageSet)) {
          nodes.Add(pChild);
        }
      } else {
        if (bFilterChildren) {
          nodes.Add(pChild);
        }
      }
      pChild = pChild->m_pNext;
    }
    if (bFilterOneOfProperties && nodes.GetSize() < 1) {
      int32_t iProperties = 0;
      XFA_LPCPROPERTY pProperty =
          XFA_GetElementProperties(GetClassID(), iProperties);
      if (pProperty == NULL || iProperties < 1) {
        return 0;
      }
      for (int32_t i = 0; i < iProperties; i++) {
        if (pProperty[i].uFlags & XFA_PROPERTYFLAG_DefaultOneOf) {
          IXFA_ObjFactory* pFactory = m_pDocument->GetParser()->GetFactory();
          XFA_LPCPACKETINFO pPacket = XFA_GetPacketByID(GetPacketID());
          CXFA_Node* pNewNode =
              pFactory->CreateNode(pPacket, (XFA_ELEMENT)pProperty[i].eName);
          if (!pNewNode) {
            break;
          }
          InsertChild(pNewNode, NULL);
          pNewNode->SetFlag(XFA_NODEFLAG_Initialized);
          nodes.Add(pNewNode);
          break;
        }
      }
    }
  }
  return nodes.GetSize();
}
CXFA_Node* CXFA_Node::CreateSamePacketNode(XFA_ELEMENT eElement,
                                           FX_DWORD dwFlags) {
  IXFA_ObjFactory* pFactory = m_pDocument->GetParser()->GetFactory();
  CXFA_Node* pNode = pFactory->CreateNode(m_ePacket, eElement);
  pNode->SetFlag(dwFlags);
  return pNode;
}
CXFA_Node* CXFA_Node::CloneTemplateToForm(FX_BOOL bRecursive) {
  FXSYS_assert(m_ePacket == XFA_XDPPACKET_Template);
  IXFA_ObjFactory* pFactory = m_pDocument->GetParser()->GetFactory();
  CXFA_Node* pClone = pFactory->CreateNode(XFA_XDPPACKET_Form, m_eNodeClass);
  if (!pClone) {
    return NULL;
  }
  pClone->SetTemplateNode(this);
  pClone->UpdateNameHash();
  pClone->SetXMLMappingNode(GetXMLMappingNode());
  if (bRecursive) {
    for (CXFA_Node* pChild = GetNodeItem(XFA_NODEITEM_FirstChild); pChild;
         pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
      pClone->InsertChild(pChild->CloneTemplateToForm(bRecursive));
    }
  }
  pClone->SetFlag(XFA_NODEFLAG_Initialized);
  return pClone;
}
CXFA_Node* CXFA_Node::GetTemplateNode() {
  return m_pAuxNode;
}
void CXFA_Node::SetTemplateNode(CXFA_Node* pTemplateNode) {
  m_pAuxNode = pTemplateNode;
}
CXFA_Node* CXFA_Node::GetBindData() {
  ASSERT(GetPacketID() == XFA_XDPPACKET_Form);
  return (CXFA_Node*)GetObject(XFA_ATTRIBUTE_BindingNode);
}
int32_t CXFA_Node::GetBindItems(CXFA_NodeArray& formItems) {
  if (m_uFlags & XFA_NODEFLAG_BindFormItems) {
    CXFA_NodeArray* pItems = NULL;
    TryObject(XFA_ATTRIBUTE_BindingNode, (void*&)pItems);
    formItems.Copy(*pItems);
    return formItems.GetSize();
  }
  CXFA_Node* pFormNode = (CXFA_Node*)GetObject(XFA_ATTRIBUTE_BindingNode);
  if (pFormNode) {
    formItems.Add(pFormNode);
  }
  return formItems.GetSize();
}
static void XFA_DataNodeDeleteBindItem(void* pData) {
  if (pData) {
    delete ((CXFA_NodeArray*)pData);
  }
}
static XFA_MAPDATABLOCKCALLBACKINFO deleteBindItemCallBack = {
    XFA_DataNodeDeleteBindItem, NULL};
int32_t CXFA_Node::AddBindItem(CXFA_Node* pFormNode) {
  ASSERT(pFormNode);
  if (m_uFlags & XFA_NODEFLAG_BindFormItems) {
    CXFA_NodeArray* pItems = NULL;
    TryObject(XFA_ATTRIBUTE_BindingNode, (void*&)pItems);
    ASSERT(pItems);
    if (pItems->Find(pFormNode) < 0) {
      pItems->Add(pFormNode);
    }
    return pItems->GetSize();
  }
  CXFA_Node* pOldFormItem = (CXFA_Node*)GetObject(XFA_ATTRIBUTE_BindingNode);
  if (!pOldFormItem) {
    SetObject(XFA_ATTRIBUTE_BindingNode, pFormNode);
    return 1;
  } else if (pOldFormItem == pFormNode) {
    return 1;
  }
  CXFA_NodeArray* pItems = new CXFA_NodeArray;
  SetObject(XFA_ATTRIBUTE_BindingNode, pItems, &deleteBindItemCallBack);
  pItems->Add(pOldFormItem);
  pItems->Add(pFormNode);
  m_uFlags |= XFA_NODEFLAG_BindFormItems;
  return 2;
}
int32_t CXFA_Node::RemoveBindItem(CXFA_Node* pFormNode) {
  if (m_uFlags & XFA_NODEFLAG_BindFormItems) {
    CXFA_NodeArray* pItems = NULL;
    TryObject(XFA_ATTRIBUTE_BindingNode, (void*&)pItems);
    ASSERT(pItems);
    int32_t iIndex = pItems->Find(pFormNode);
    int32_t iCount = pItems->GetSize();
    if (iIndex >= 0) {
      if (iIndex != iCount - 1) {
        pItems->SetAt(iIndex, pItems->GetAt(iCount - 1));
      }
      pItems->RemoveAt(iCount - 1);
      if (iCount == 2) {
        CXFA_Node* pLastFormNode = pItems->GetAt(0);
        SetObject(XFA_ATTRIBUTE_BindingNode, pLastFormNode);
        m_uFlags &= ~XFA_NODEFLAG_BindFormItems;
      }
      iCount--;
    }
    return iCount;
  }
  CXFA_Node* pOldFormItem = (CXFA_Node*)GetObject(XFA_ATTRIBUTE_BindingNode);
  if (pOldFormItem == pFormNode) {
    SetObject(XFA_ATTRIBUTE_BindingNode, NULL);
    pOldFormItem = NULL;
  }
  return pOldFormItem == NULL ? 0 : 1;
}
FX_BOOL CXFA_Node::HasBindItem() {
  return (GetPacketID() == XFA_XDPPACKET_Datasets) &&
         GetObject(XFA_ATTRIBUTE_BindingNode) != NULL;
}
CXFA_WidgetData* CXFA_Node::GetWidgetData() {
  return (CXFA_WidgetData*)GetObject(XFA_ATTRIBUTE_WidgetData);
}
CXFA_WidgetData* CXFA_Node::GetContainerWidgetData() {
  if (GetPacketID() != XFA_XDPPACKET_Form) {
    return NULL;
  }
  XFA_ELEMENT classID = GetClassID();
  if (classID == XFA_ELEMENT_ExclGroup) {
    return NULL;
  }
  CXFA_Node* pParentNode = GetNodeItem(XFA_NODEITEM_Parent);
  if (pParentNode && pParentNode->GetClassID() == XFA_ELEMENT_ExclGroup) {
    return NULL;
  }
  if (classID == XFA_ELEMENT_Field) {
    CXFA_WidgetData* pFieldWidgetData = this->GetWidgetData();
    if (pFieldWidgetData &&
        pFieldWidgetData->GetChoiceListOpen() ==
            XFA_ATTRIBUTEENUM_MultiSelect) {
      return NULL;
    } else {
      CFX_WideString wsPicture;
      if (pFieldWidgetData) {
        pFieldWidgetData->GetPictureContent(wsPicture,
                                            XFA_VALUEPICTURE_DataBind);
      }
      if (!wsPicture.IsEmpty()) {
        return pFieldWidgetData;
      }
      CXFA_Node* pDataNode = this->GetBindData();
      if (!pDataNode) {
        return NULL;
      }
      pFieldWidgetData = NULL;
      CXFA_NodeArray formNodes;
      pDataNode->GetBindItems(formNodes);
      for (int32_t i = 0; i < formNodes.GetSize(); i++) {
        CXFA_Node* pFormNode = formNodes.GetAt(i);
        if (!pFormNode || pFormNode->HasFlag(XFA_NODEFLAG_HasRemoved)) {
          continue;
        }
        pFieldWidgetData = pFormNode->GetWidgetData();
        if (pFieldWidgetData) {
          pFieldWidgetData->GetPictureContent(wsPicture,
                                              XFA_VALUEPICTURE_DataBind);
        }
        if (!wsPicture.IsEmpty()) {
          break;
        }
        pFieldWidgetData = NULL;
      }
      return pFieldWidgetData;
    }
  }
  CXFA_Node* pGrandNode =
      pParentNode ? pParentNode->GetNodeItem(XFA_NODEITEM_Parent) : NULL;
  CXFA_Node* pValueNode =
      (pParentNode && pParentNode->GetClassID() == XFA_ELEMENT_Value)
          ? pParentNode
          : NULL;
  if (!pValueNode) {
    pValueNode = (pGrandNode && pGrandNode->GetClassID() == XFA_ELEMENT_Value)
                     ? pGrandNode
                     : NULL;
  }
  CXFA_Node* pParentOfValueNode =
      pValueNode ? pValueNode->GetNodeItem(XFA_NODEITEM_Parent) : NULL;
  return pParentOfValueNode ? pParentOfValueNode->GetContainerWidgetData()
                            : NULL;
}
FX_BOOL CXFA_Node::GetLocaleName(CFX_WideString& wsLocaleName) {
  CXFA_Node* pForm = (CXFA_Node*)GetDocument()->GetXFANode(XFA_HASHCODE_Form);
  CXFA_Node* pTopSubform = pForm->GetFirstChildByClass(XFA_ELEMENT_Subform);
  FXSYS_assert(pTopSubform);
  CXFA_Node* pLocaleNode = this;
  FX_BOOL bLocale = FALSE;
  do {
    bLocale = pLocaleNode->TryCData(XFA_ATTRIBUTE_Locale, wsLocaleName, FALSE);
    if (!bLocale) {
      pLocaleNode = pLocaleNode->GetNodeItem(XFA_NODEITEM_Parent);
    }
  } while (pLocaleNode && pLocaleNode != pTopSubform && !bLocale);
  if (bLocale) {
    return bLocale;
  }
  CXFA_Node* pConfig =
      (CXFA_Node*)GetDocument()->GetXFANode(XFA_HASHCODE_Config);
  wsLocaleName = GetDocument()->GetLocalMgr()->GetConfigLocaleName(pConfig);
  if (!wsLocaleName.IsEmpty()) {
    bLocale = TRUE;
  }
  if (bLocale) {
    return bLocale;
  }
  if (pTopSubform) {
    bLocale = pTopSubform->TryCData(XFA_ATTRIBUTE_Locale, wsLocaleName, FALSE);
  }
  if (bLocale) {
    return bLocale;
  }
  IFX_Locale* pLocale = GetDocument()->GetLocalMgr()->GetDefLocale();
  if (pLocale) {
    wsLocaleName = pLocale->GetName();
    bLocale = TRUE;
  }
  return bLocale;
}
XFA_ATTRIBUTEENUM CXFA_Node::GetIntact() {
  XFA_ELEMENT eElement = GetClassID();
  CXFA_Node* pKeep = GetFirstChildByClass(XFA_ELEMENT_Keep);
  XFA_ATTRIBUTEENUM eLayoutType = GetEnum(XFA_ATTRIBUTE_Layout);
  if (pKeep) {
    XFA_ATTRIBUTEENUM eIntact;
    if (pKeep->TryEnum(XFA_ATTRIBUTE_Intact, eIntact, FALSE)) {
      if (eIntact == XFA_ATTRIBUTEENUM_None &&
          eLayoutType == XFA_ATTRIBUTEENUM_Row &&
          m_pDocument->GetCurVersionMode() < XFA_VERSION_208) {
        CXFA_Node* pPreviewRow =
            GetNodeItem(XFA_NODEITEM_PrevSibling, XFA_OBJECTTYPE_ContainerNode);
        if (pPreviewRow &&
            pPreviewRow->GetEnum(XFA_ATTRIBUTE_Layout) ==
                XFA_ATTRIBUTEENUM_Row) {
          XFA_ATTRIBUTEENUM eValue;
          if (pKeep->TryEnum(XFA_ATTRIBUTE_Previous, eValue, FALSE)) {
            if (eValue == XFA_ATTRIBUTEENUM_ContentArea ||
                eValue == XFA_ATTRIBUTEENUM_PageArea) {
              return XFA_ATTRIBUTEENUM_ContentArea;
            }
          }
          CXFA_Node* pKeep =
              pPreviewRow->GetFirstChildByClass(XFA_ELEMENT_Keep);
          if (pKeep) {
            if (pKeep->TryEnum(XFA_ATTRIBUTE_Next, eValue, FALSE)) {
              if (eValue == XFA_ATTRIBUTEENUM_ContentArea ||
                  eValue == XFA_ATTRIBUTEENUM_PageArea) {
                return XFA_ATTRIBUTEENUM_ContentArea;
              }
            }
          }
        }
      }
      return eIntact;
    }
  }
  switch (eElement) {
    case XFA_ELEMENT_Subform:
      switch (eLayoutType) {
        case XFA_ATTRIBUTEENUM_Position:
        case XFA_ATTRIBUTEENUM_Row:
          return XFA_ATTRIBUTEENUM_ContentArea;
        case XFA_ATTRIBUTEENUM_Tb:
        case XFA_ATTRIBUTEENUM_Table:
        case XFA_ATTRIBUTEENUM_Lr_tb:
        case XFA_ATTRIBUTEENUM_Rl_tb:
          return XFA_ATTRIBUTEENUM_None;
        default:
          break;
      }
      break;
    case XFA_ELEMENT_Field: {
      CXFA_Node* pParentNode = this->GetNodeItem(XFA_NODEITEM_Parent);
      if (!pParentNode || pParentNode->GetClassID() == XFA_ELEMENT_PageArea) {
        return XFA_ATTRIBUTEENUM_ContentArea;
      }
      if (pParentNode->GetIntact() == XFA_ATTRIBUTEENUM_None) {
        XFA_ATTRIBUTEENUM eParLayout =
            pParentNode->GetEnum(XFA_ATTRIBUTE_Layout);
        if (eParLayout == XFA_ATTRIBUTEENUM_Position ||
            eParLayout == XFA_ATTRIBUTEENUM_Row ||
            eParLayout == XFA_ATTRIBUTEENUM_Table) {
          return XFA_ATTRIBUTEENUM_None;
        }
        XFA_VERSION version = m_pDocument->GetCurVersionMode();
        if (eParLayout == XFA_ATTRIBUTEENUM_Tb && version < XFA_VERSION_208) {
          CXFA_Measurement measureH;
          if (this->TryMeasure(XFA_ATTRIBUTE_H, measureH, FALSE)) {
            return XFA_ATTRIBUTEENUM_ContentArea;
          }
        }
        return XFA_ATTRIBUTEENUM_None;
      }
      return XFA_ATTRIBUTEENUM_ContentArea;
    }
    case XFA_ELEMENT_Draw:
      return XFA_ATTRIBUTEENUM_ContentArea;
    default:
      break;
  }
  return XFA_ATTRIBUTEENUM_None;
}
CXFA_Node* CXFA_Node::GetDataDescriptionNode() {
  if (m_ePacket == XFA_XDPPACKET_Datasets) {
    return m_pAuxNode;
  }
  return NULL;
}
void CXFA_Node::SetDataDescriptionNode(CXFA_Node* pDataDescriptionNode) {
  FXSYS_assert(m_ePacket == XFA_XDPPACKET_Datasets);
  m_pAuxNode = pDataDescriptionNode;
}
void CXFA_Node::Script_TreeClass_ResolveNode(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"resolveNode");
    return;
  }
  CFX_WideString wsExpression;
  CFX_ByteString bsExpression = pArguments->GetUTF8String(0);
  wsExpression =
      CFX_WideString::FromUTF8(bsExpression, bsExpression.GetLength());
  IXFA_ScriptContext* pScriptContext = m_pDocument->GetScriptContext();
  if (!pScriptContext) {
    return;
  }
  CXFA_Node* refNode = this;
  if (refNode->GetClassID() == XFA_ELEMENT_Xfa) {
    refNode = (CXFA_Node*)pScriptContext->GetThisObject();
  }
  FX_DWORD dwFlag = XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Attributes |
                    XFA_RESOLVENODE_Properties | XFA_RESOLVENODE_Parent |
                    XFA_RESOLVENODE_Siblings;
  XFA_RESOLVENODE_RS resoveNodeRS;
  int32_t iRet = pScriptContext->ResolveObjects(refNode, wsExpression,
                                                resoveNodeRS, dwFlag);
  if (iRet < 1) {
    return FXJSE_Value_SetNull(pArguments->GetReturnValue());
  }
  FXJSE_HVALUE hValue = NULL;
  if (resoveNodeRS.dwFlags == XFA_RESOVENODE_RSTYPE_Nodes) {
    CXFA_Object* pNode = resoveNodeRS.nodes[0];
    hValue = pScriptContext->GetJSValueFromMap(pNode);
    FXJSE_Value_Set(pArguments->GetReturnValue(), hValue);
  } else {
    XFA_LPCSCRIPTATTRIBUTEINFO lpAttributeInfo = resoveNodeRS.pScriptAttribute;
    if (lpAttributeInfo && lpAttributeInfo->eValueType == XFA_SCRIPT_Object) {
      hValue = FXJSE_Value_Create(pScriptContext->GetRuntime());
      (resoveNodeRS.nodes[0]->*(lpAttributeInfo->lpfnCallback))(
          hValue, FALSE, (XFA_ATTRIBUTE)lpAttributeInfo->eAttribute);
      FXJSE_Value_Set(pArguments->GetReturnValue(), hValue);
      FXJSE_Value_Release(hValue);
    } else {
      FXJSE_Value_SetNull(pArguments->GetReturnValue());
    }
  }
}
void CXFA_Node::Script_TreeClass_ResolveNodes(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"resolveNodes");
    return;
  }
  CFX_WideString wsExpression;
  CFX_ByteString bsExpression = pArguments->GetUTF8String(0);
  wsExpression =
      CFX_WideString::FromUTF8(bsExpression, bsExpression.GetLength());
  FXJSE_HVALUE hValue = pArguments->GetReturnValue();
  if (!hValue) {
    return;
  }
  FX_DWORD dwFlag = XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Attributes |
                    XFA_RESOLVENODE_Properties | XFA_RESOLVENODE_Parent |
                    XFA_RESOLVENODE_Siblings;
  CXFA_Node* refNode = this;
  if (refNode->GetClassID() == XFA_ELEMENT_Xfa) {
    refNode = (CXFA_Node*)m_pDocument->GetScriptContext()->GetThisObject();
  }
  Script_Som_ResolveNodeList(hValue, wsExpression, dwFlag, refNode);
}
void CXFA_Node::Script_Som_ResolveNodeList(FXJSE_HVALUE hValue,
                                           CFX_WideString wsExpression,
                                           FX_DWORD dwFlag,
                                           CXFA_Node* refNode) {
  IXFA_ScriptContext* pScriptContext = m_pDocument->GetScriptContext();
  if (!pScriptContext) {
    return;
  }
  XFA_RESOLVENODE_RS resoveNodeRS;
  if (refNode == NULL) {
    refNode = this;
  }
  pScriptContext->ResolveObjects(refNode, wsExpression,
                                 resoveNodeRS, dwFlag);
  CXFA_ArrayNodeList* pNodeList = new CXFA_ArrayNodeList(m_pDocument);
  if (resoveNodeRS.dwFlags == XFA_RESOVENODE_RSTYPE_Nodes) {
    for (int32_t i = 0; i < resoveNodeRS.nodes.GetSize(); i++) {
      if (resoveNodeRS.nodes[i]->IsNode()) {
        pNodeList->Append((CXFA_Node*)resoveNodeRS.nodes[i]);
      }
    }
  } else {
    CXFA_HVALUEArray hValueArray(pScriptContext->GetRuntime());
    if (resoveNodeRS.GetAttributeResult(hValueArray) > 0) {
      CXFA_ObjArray objectArray;
      hValueArray.GetAttributeObject(objectArray);
      for (int32_t i = 0; i < objectArray.GetSize(); i++) {
        if (objectArray[i]->IsNode()) {
          pNodeList->Append((CXFA_Node*)objectArray[i]);
        }
      }
    }
  }
  FXJSE_Value_SetObject(hValue, (CXFA_Object*)pNodeList,
                        pScriptContext->GetJseNormalClass());
}
void CXFA_Node::Script_TreeClass_All(FXJSE_HVALUE hValue,
                                     FX_BOOL bSetting,
                                     XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
  } else {
    FX_DWORD dwFlag = XFA_RESOLVENODE_Siblings | XFA_RESOLVENODE_ALL;
    CFX_WideString wsName;
    GetAttribute(XFA_ATTRIBUTE_Name, wsName);
    CFX_WideString wsExpression = wsName + FX_WSTRC(L"[*]");
    Script_Som_ResolveNodeList(hValue, wsExpression, dwFlag);
  }
}
void CXFA_Node::Script_TreeClass_Nodes(FXJSE_HVALUE hValue,
                                       FX_BOOL bSetting,
                                       XFA_ATTRIBUTE eAttribute) {
  IXFA_ScriptContext* pScriptContext = m_pDocument->GetScriptContext();
  if (!pScriptContext) {
    return;
  }
  if (bSetting) {
    IXFA_AppProvider* pAppProvider = m_pDocument->GetNotify()->GetAppProvider();
    FXSYS_assert(pAppProvider);
    CFX_WideString wsMessage;
    pAppProvider->LoadString(XFA_IDS_Unable_TO_SET, wsMessage);
    FXJSE_ThrowMessage("", FX_UTF8Encode(wsMessage, wsMessage.GetLength()));
  } else {
    CXFA_AttachNodeList* pNodeList = new CXFA_AttachNodeList(m_pDocument, this);
    FXJSE_Value_SetObject(hValue, (CXFA_Object*)pNodeList,
                          pScriptContext->GetJseNormalClass());
  }
}
void CXFA_Node::Script_TreeClass_ClassAll(FXJSE_HVALUE hValue,
                                          FX_BOOL bSetting,
                                          XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
  } else {
    FX_DWORD dwFlag = XFA_RESOLVENODE_Siblings | XFA_RESOLVENODE_ALL;
    CFX_WideStringC wsName;
    this->GetClassName(wsName);
    CFX_WideString wsExpression = FX_WSTRC(L"#") + wsName + FX_WSTRC(L"[*]");
    Script_Som_ResolveNodeList(hValue, wsExpression, dwFlag);
  }
}
void CXFA_Node::Script_TreeClass_Parent(FXJSE_HVALUE hValue,
                                        FX_BOOL bSetting,
                                        XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
  } else {
    CXFA_Node* pParent = this->GetNodeItem(XFA_NODEITEM_Parent);
    if (pParent) {
      FXJSE_Value_Set(
          hValue, m_pDocument->GetScriptContext()->GetJSValueFromMap(pParent));
    } else {
      FXJSE_Value_SetNull(hValue);
    }
  }
}
void CXFA_Node::Script_TreeClass_Index(FXJSE_HVALUE hValue,
                                       FX_BOOL bSetting,
                                       XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
  } else {
    FXJSE_Value_SetInteger(hValue, GetNodeSameNameIndex());
  }
}
void CXFA_Node::Script_TreeClass_ClassIndex(FXJSE_HVALUE hValue,
                                            FX_BOOL bSetting,
                                            XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
  } else {
    FXJSE_Value_SetInteger(hValue, GetNodeSameClassIndex());
  }
}
void CXFA_Node::Script_TreeClass_SomExpression(FXJSE_HVALUE hValue,
                                               FX_BOOL bSetting,
                                               XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
  } else {
    CFX_WideString wsSOMExpression;
    GetSOMExpression(wsSOMExpression);
    FXJSE_Value_SetUTF8String(hValue, FX_UTF8Encode(wsSOMExpression));
  }
}
void CXFA_Node::Script_NodeClass_ApplyXSL(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"applyXSL");
    return;
  }
  CFX_WideString wsExpression;
  CFX_ByteString bsExpression = pArguments->GetUTF8String(0);
  wsExpression =
      CFX_WideString::FromUTF8(bsExpression, bsExpression.GetLength());
}
void CXFA_Node::Script_NodeClass_AssignNode(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 1 || iLength > 3) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"assignNode");
    return;
  }
  CFX_WideString wsExpression;
  CFX_WideString wsValue;
  int32_t iAction = 0;
  if (iLength >= 1) {
    CFX_ByteString bsExpression = pArguments->GetUTF8String(0);
    wsExpression =
        CFX_WideString::FromUTF8(bsExpression, bsExpression.GetLength());
  }
  if (iLength >= 2) {
    CFX_ByteString bsValue = pArguments->GetUTF8String(1);
    wsValue = CFX_WideString::FromUTF8(bsValue, bsValue.GetLength());
  }
  if (iLength >= 3) {
    iAction = pArguments->GetInt32(2);
  }
}
void CXFA_Node::Script_NodeClass_Clone(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"clone");
    return;
  }
  FX_BOOL bClone = TRUE;
  bClone = pArguments->GetInt32(0) == 0 ? FALSE : TRUE;
  CXFA_Node* pCloneNode = this->Clone(bClone);
  FXJSE_Value_Set(
      pArguments->GetReturnValue(),
      m_pDocument->GetScriptContext()->GetJSValueFromMap(pCloneNode));
}
void CXFA_Node::Script_NodeClass_GetAttribute(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"getAttribute");
    return;
  }
  CFX_WideString wsExpression;
  CFX_ByteString bsExpression = pArguments->GetUTF8String(0);
  wsExpression =
      CFX_WideString::FromUTF8(bsExpression, bsExpression.GetLength());
  CFX_WideString wsValue;
  this->GetAttribute(wsExpression, wsValue);
  FXJSE_HVALUE hValue = pArguments->GetReturnValue();
  if (hValue) {
    FXJSE_Value_SetUTF8String(hValue, FX_UTF8Encode(wsValue));
  }
}
void CXFA_Node::Script_NodeClass_GetElement(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 1 || iLength > 2) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"getElement");
    return;
  }
  CFX_WideString wsExpression;
  int32_t iValue = 0;
  if (iLength >= 1) {
    CFX_ByteString bsExpression = pArguments->GetUTF8String(0);
    wsExpression =
        CFX_WideString::FromUTF8(bsExpression, bsExpression.GetLength());
  }
  if (iLength >= 2) {
    iValue = pArguments->GetInt32(1);
  }
  XFA_LPCELEMENTINFO pElementInfo = XFA_GetElementByName(wsExpression);
  CXFA_Node* pNode = this->GetProperty(iValue, pElementInfo->eName);
  FXJSE_Value_Set(pArguments->GetReturnValue(),
                  m_pDocument->GetScriptContext()->GetJSValueFromMap(pNode));
}
void CXFA_Node::Script_NodeClass_IsPropertySpecified(
    CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 1 || iLength > 3) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"isPropertySpecified");
    return;
  }
  CFX_WideString wsExpression;
  FX_BOOL bParent = TRUE;
  int32_t iIndex = 0;
  if (iLength >= 1) {
    CFX_ByteString bsExpression = pArguments->GetUTF8String(0);
    wsExpression =
        CFX_WideString::FromUTF8(bsExpression, bsExpression.GetLength());
  }
  if (iLength >= 2) {
    bParent = pArguments->GetInt32(1) == 0 ? FALSE : TRUE;
  }
  if (iLength >= 3) {
    iIndex = pArguments->GetInt32(2);
  }
  FX_BOOL bHas = FALSE;
  XFA_LPCATTRIBUTEINFO pAttributeInfo = XFA_GetAttributeByName(wsExpression);
  CFX_WideString wsValue;
  if (pAttributeInfo) {
    bHas = this->HasAttribute(pAttributeInfo->eName);
  }
  if (!bHas) {
    XFA_LPCELEMENTINFO pElementInfo = XFA_GetElementByName(wsExpression);
    bHas = (this->GetProperty(iIndex, pElementInfo->eName) != NULL);
  }
  FXJSE_HVALUE hValue = pArguments->GetReturnValue();
  if (hValue) {
    FXJSE_Value_SetBoolean(hValue, bHas);
  }
}
void CXFA_Node::Script_NodeClass_LoadXML(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 1 || iLength > 3) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"loadXML");
    return;
  }
  CFX_WideString wsExpression;
  FX_BOOL bIgnoreRoot = TRUE;
  FX_BOOL bOverwrite = 0;
  if (iLength >= 1) {
    CFX_ByteString bsExpression = pArguments->GetUTF8String(0);
    wsExpression =
        CFX_WideString::FromUTF8(bsExpression, bsExpression.GetLength());
    if (wsExpression.IsEmpty()) {
      return;
    }
  }
  if (iLength >= 2) {
    bIgnoreRoot = pArguments->GetInt32(1) == 0 ? FALSE : TRUE;
  }
  if (iLength >= 3) {
    bOverwrite = pArguments->GetInt32(2) == 0 ? FALSE : TRUE;
  }
  IXFA_Parser* pParser = IXFA_Parser::Create(m_pDocument);
  if (!pParser) {
    return;
  }
  IFDE_XMLNode* pXMLNode = NULL;
  int32_t iParserStatus = pParser->ParseXMLData(wsExpression, pXMLNode, NULL);
  if (iParserStatus != XFA_PARSESTATUS_Done || !pXMLNode) {
    pParser->Release();
    pParser = NULL;
    return;
  }
  if (bIgnoreRoot && (pXMLNode->GetType() != FDE_XMLNODE_Element ||
                      XFA_RecognizeRichText((IFDE_XMLElement*)pXMLNode))) {
    bIgnoreRoot = FALSE;
  }
  CXFA_Node* pFakeRoot = this->Clone(FALSE);
  CFX_WideStringC wsContentType = this->GetCData(XFA_ATTRIBUTE_ContentType);
  if (!wsContentType.IsEmpty()) {
    pFakeRoot->SetCData(XFA_ATTRIBUTE_ContentType, wsContentType);
  }
  IFDE_XMLNode* pFakeXMLRoot = pFakeRoot->GetXMLMappingNode();
  if (!pFakeXMLRoot) {
    IFDE_XMLNode* pThisXMLRoot = this->GetXMLMappingNode();
    pFakeXMLRoot = pThisXMLRoot ? pThisXMLRoot->Clone(FALSE) : NULL;
  }
  if (!pFakeXMLRoot) {
    CFX_WideStringC wsClassName;
    this->GetClassName(wsClassName);
    pFakeXMLRoot = IFDE_XMLElement::Create(wsClassName);
  }
  if (bIgnoreRoot) {
    IFDE_XMLNode* pXMLChild = pXMLNode->GetNodeItem(IFDE_XMLNode::FirstChild);
    while (pXMLChild) {
      IFDE_XMLNode* pXMLSibling =
          pXMLChild->GetNodeItem(IFDE_XMLNode::NextSibling);
      pXMLNode->RemoveChildNode(pXMLChild);
      pFakeXMLRoot->InsertChildNode(pXMLChild);
      pXMLChild = pXMLSibling;
    }
  } else {
    IFDE_XMLNode* pXMLParent = pXMLNode->GetNodeItem(IFDE_XMLNode::Parent);
    if (pXMLParent) {
      pXMLParent->RemoveChildNode(pXMLNode);
    }
    pFakeXMLRoot->InsertChildNode(pXMLNode);
  }
  pParser->ConstructXFANode(pFakeRoot, pFakeXMLRoot);
  pFakeRoot = pParser->GetRootNode();
  if (pFakeRoot) {
    if (bOverwrite) {
      CXFA_Node* pChild = this->GetNodeItem(XFA_NODEITEM_FirstChild);
      CXFA_Node* pNewChild = pFakeRoot->GetNodeItem(XFA_NODEITEM_FirstChild);
      int32_t index = 0;
      while (pNewChild) {
        CXFA_Node* pItem = pNewChild->GetNodeItem(XFA_NODEITEM_NextSibling);
        pFakeRoot->RemoveChild(pNewChild);
        this->InsertChild(index++, pNewChild);
        pNewChild->SetFlag(XFA_NODEFLAG_Initialized);
        pNewChild = pItem;
      }
      while (pChild) {
        CXFA_Node* pItem = pChild->GetNodeItem(XFA_NODEITEM_NextSibling);
        this->RemoveChild(pChild);
        pFakeRoot->InsertChild(pChild);
        pChild = pItem;
      }
      if (GetPacketID() == XFA_XDPPACKET_Form &&
          GetClassID() == XFA_ELEMENT_ExData) {
        IFDE_XMLNode* pTempXMLNode = this->GetXMLMappingNode();
        this->SetXMLMappingNode(pFakeXMLRoot);
        this->SetFlag(XFA_NODEFLAG_OwnXMLNode, TRUE, FALSE);
        if (pTempXMLNode &&
            pTempXMLNode->GetNodeItem(IFDE_XMLNode::Parent) == NULL) {
          pFakeXMLRoot = pTempXMLNode;
        } else {
          pFakeXMLRoot = NULL;
        }
      }
      MoveBufferMapData(pFakeRoot, this, XFA_CalcData, TRUE);
    } else {
      CXFA_Node* pChild = pFakeRoot->GetNodeItem(XFA_NODEITEM_FirstChild);
      while (pChild) {
        CXFA_Node* pItem = pChild->GetNodeItem(XFA_NODEITEM_NextSibling);
        pFakeRoot->RemoveChild(pChild);
        this->InsertChild(pChild);
        pChild->SetFlag(XFA_NODEFLAG_Initialized);
        pChild = pItem;
      }
    }
    if (pFakeXMLRoot) {
      pFakeRoot->SetXMLMappingNode(pFakeXMLRoot);
      pFakeRoot->SetFlag(XFA_NODEFLAG_OwnXMLNode, TRUE, FALSE);
    }
    pFakeRoot->SetFlag(XFA_NODEFLAG_HasRemoved, TRUE, FALSE);
  } else {
    if (pFakeXMLRoot) {
      pFakeXMLRoot->Release();
      pFakeXMLRoot = NULL;
    }
  }
  pParser->Release();
  pParser = NULL;
}
void CXFA_Node::Script_NodeClass_SaveFilteredXML(CFXJSE_Arguments* pArguments) {
}
void CXFA_Node::Script_NodeClass_SaveXML(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 0 || iLength > 1) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"saveXML");
    return;
  }
  FX_BOOL bPrettyMode = FALSE;
  if (iLength == 1) {
    CFX_ByteString bsPretty = pArguments->GetUTF8String(0);
    if (bsPretty.Equal("pretty")) {
      bPrettyMode = TRUE;
    } else {
      ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
      return;
    }
  }
  CFX_ByteStringC bsXMLHeader = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  if (GetPacketID() == XFA_XDPPACKET_Form) {
    IFX_MemoryStream* pMemoryStream = FX_CreateMemoryStream(TRUE);
    if (!pMemoryStream) {
      FXJSE_Value_SetUTF8String(pArguments->GetReturnValue(), bsXMLHeader);
      return;
    }
    IFX_Stream* pStream = IFX_Stream::CreateStream(
        (IFX_FileWrite*)pMemoryStream,
        FX_STREAMACCESS_Text | FX_STREAMACCESS_Write | FX_STREAMACCESS_Append);
    if (!pStream) {
      FXJSE_Value_SetUTF8String(pArguments->GetReturnValue(), bsXMLHeader);
      pMemoryStream->Release();
      pMemoryStream = NULL;
      return;
    }
    pStream->SetCodePage(FX_CODEPAGE_UTF8);
    pStream->WriteData(bsXMLHeader.GetPtr(), bsXMLHeader.GetLength());
    XFA_DataExporter_RegenerateFormFile(this, pStream, NULL, TRUE);
    FXJSE_Value_SetUTF8String(
        pArguments->GetReturnValue(),
        CFX_ByteStringC(pMemoryStream->GetBuffer(), pMemoryStream->GetSize()));
    pStream->Release();
    pStream = NULL;
    if (pMemoryStream) {
      pMemoryStream->Release();
      pMemoryStream = NULL;
    }
    return;
  } else if (GetPacketID() == XFA_XDPPACKET_Datasets) {
    IFDE_XMLNode* pElement = this->GetXMLMappingNode();
    if (!pElement || pElement->GetType() != FDE_XMLNODE_Element) {
      FXJSE_Value_SetUTF8String(pArguments->GetReturnValue(), bsXMLHeader);
      return;
    }
    XFA_DataExporter_DealWithDataGroupNode(this);
    IFX_MemoryStream* pMemoryStream = FX_CreateMemoryStream(TRUE);
    if (!pMemoryStream) {
      FXJSE_Value_SetUTF8String(pArguments->GetReturnValue(), bsXMLHeader);
      return;
    }
    if (pMemoryStream) {
      IFX_Stream* pStream = IFX_Stream::CreateStream(
          (IFX_FileWrite*)pMemoryStream, FX_STREAMACCESS_Text |
                                             FX_STREAMACCESS_Write |
                                             FX_STREAMACCESS_Append);
      if (pStream) {
        pStream->SetCodePage(FX_CODEPAGE_UTF8);
        pStream->WriteData(bsXMLHeader.GetPtr(), bsXMLHeader.GetLength());
        pElement->SaveXMLNode(pStream);
        FXJSE_Value_SetUTF8String(pArguments->GetReturnValue(),
                                  CFX_ByteStringC(pMemoryStream->GetBuffer(),
                                                  pMemoryStream->GetSize()));
        pStream->Release();
        pStream = NULL;
      }
      if (pMemoryStream) {
        pMemoryStream->Release();
        pMemoryStream = NULL;
      }
      return;
    }
  } else {
    FXJSE_Value_SetUTF8String(pArguments->GetReturnValue(), "");
  }
}
void CXFA_Node::Script_NodeClass_SetAttribute(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 2) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"setAttribute");
    return;
  }
  CFX_WideString wsAttribute;
  CFX_WideString wsAttributeValue;
  CFX_ByteString bsAttributeValue = pArguments->GetUTF8String(0);
  CFX_ByteString bsAttribute = pArguments->GetUTF8String(1);
  wsAttributeValue =
      CFX_WideString::FromUTF8(bsAttributeValue, bsAttributeValue.GetLength());
  wsAttribute = CFX_WideString::FromUTF8(bsAttribute, bsAttribute.GetLength());
  this->SetAttribute(wsAttribute, wsAttributeValue, TRUE);
}
void CXFA_Node::Script_NodeClass_SetElement(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1 && iLength != 2) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"setElement");
    return;
  }
  CXFA_Node* pNode = NULL;
  CFX_WideString wsName;
  if (iLength >= 1) {
    pNode = (CXFA_Node*)pArguments->GetObject(0);
  }
  if (iLength >= 2) {
    CFX_ByteString bsName = pArguments->GetUTF8String(1);
    wsName = CFX_WideString::FromUTF8(bsName, bsName.GetLength());
  }
}
void CXFA_Node::Script_NodeClass_Ns(FXJSE_HVALUE hValue,
                                    FX_BOOL bSetting,
                                    XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
  } else {
    CFX_WideString wsNameSpace;
    this->TryNamespace(wsNameSpace);
    FXJSE_Value_SetUTF8String(hValue, FX_UTF8Encode(wsNameSpace));
  }
}
void CXFA_Node::Script_NodeClass_Model(FXJSE_HVALUE hValue,
                                       FX_BOOL bSetting,
                                       XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
  } else {
    FXJSE_Value_Set(hValue, m_pDocument->GetScriptContext()->GetJSValueFromMap(
                                this->GetModelNode()));
  }
}
void CXFA_Node::Script_NodeClass_IsContainer(FXJSE_HVALUE hValue,
                                             FX_BOOL bSetting,
                                             XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
  } else {
    FXJSE_Value_SetBoolean(hValue, this->IsContainerNode());
  }
}
void CXFA_Node::Script_NodeClass_IsNull(FXJSE_HVALUE hValue,
                                        FX_BOOL bSetting,
                                        XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
  } else {
    if (this->GetClassID() == XFA_ELEMENT_Subform) {
      FXJSE_Value_SetBoolean(hValue, FALSE);
      return;
    }
    CFX_WideString strValue;
    FXJSE_Value_SetBoolean(hValue, !TryContent(strValue) || strValue.IsEmpty());
  }
}
void CXFA_Node::Script_NodeClass_OneOfChild(FXJSE_HVALUE hValue,
                                            FX_BOOL bSetting,
                                            XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
  } else {
    CXFA_NodeArray properts;
    int32_t iSize = this->GetNodeList(properts, XFA_NODEFILTER_OneOfProperty);
    if (iSize > 0) {
      FXJSE_Value_Set(
          hValue,
          m_pDocument->GetScriptContext()->GetJSValueFromMap(properts[0]));
    }
  }
}
void CXFA_Node::Script_ContainerClass_GetDelta(CFXJSE_Arguments* pArguments) {}
void CXFA_Node::Script_ContainerClass_GetDeltas(CFXJSE_Arguments* pArguments) {
  CXFA_ArrayNodeList* pFormNodes = new CXFA_ArrayNodeList(m_pDocument);
  FXJSE_Value_SetObject(pArguments->GetReturnValue(), (CXFA_Object*)pFormNodes,
                        m_pDocument->GetScriptContext()->GetJseNormalClass());
}
void CXFA_Node::Script_ModelClass_ClearErrorList(CFXJSE_Arguments* pArguments) {
}
void CXFA_Node::Script_ModelClass_CreateNode(CFXJSE_Arguments* pArguments) {
  Script_Template_CreateNode(pArguments);
}
void CXFA_Node::Script_ModelClass_IsCompatibleNS(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 1) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"isCompatibleNS");
    return;
  }
  CFX_WideString wsNameSpace;
  if (iLength >= 1) {
    CFX_ByteString bsNameSpace = pArguments->GetUTF8String(0);
    wsNameSpace =
        CFX_WideString::FromUTF8(bsNameSpace, bsNameSpace.GetLength());
  }
  CFX_WideString wsNodeNameSpace;
  this->TryNamespace(wsNodeNameSpace);
  FXJSE_HVALUE hValue = pArguments->GetReturnValue();
  if (hValue) {
    FXJSE_Value_SetBoolean(hValue, wsNodeNameSpace.Equal(wsNameSpace));
  }
}
void CXFA_Node::Script_ModelClass_Context(FXJSE_HVALUE hValue,
                                          FX_BOOL bSetting,
                                          XFA_ATTRIBUTE eAttribute) {}
void CXFA_Node::Script_ModelClass_AliasNode(FXJSE_HVALUE hValue,
                                            FX_BOOL bSetting,
                                            XFA_ATTRIBUTE eAttribute) {}
void CXFA_Node::Script_Attribute_Integer(FXJSE_HVALUE hValue,
                                         FX_BOOL bSetting,
                                         XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    SetInteger(eAttribute, FXJSE_Value_ToInteger(hValue), TRUE);
  } else {
    FXJSE_Value_SetInteger(hValue, GetInteger(eAttribute));
  }
}
void CXFA_Node::Script_Attribute_IntegerRead(FXJSE_HVALUE hValue,
                                             FX_BOOL bSetting,
                                             XFA_ATTRIBUTE eAttribute) {
  if (!bSetting) {
    FXJSE_Value_SetInteger(hValue, GetInteger(eAttribute));
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
  }
}
void CXFA_Node::Script_Attribute_BOOL(FXJSE_HVALUE hValue,
                                      FX_BOOL bSetting,
                                      XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    SetBoolean(eAttribute, FXJSE_Value_ToBoolean(hValue), TRUE);
  } else {
    FXJSE_Value_SetUTF8String(hValue, GetBoolean(eAttribute) ? "1" : "0");
  }
}
void CXFA_Node::Script_Attribute_BOOLRead(FXJSE_HVALUE hValue,
                                          FX_BOOL bSetting,
                                          XFA_ATTRIBUTE eAttribute) {
  if (!bSetting) {
    FXJSE_Value_SetUTF8String(hValue, GetBoolean(eAttribute) ? "1" : "0");
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
  }
}
void CXFA_Node::Script_Attribute_SendAttributeChangeMessage(
    void* eAttribute,
    void* eValue,
    FX_BOOL bScriptModify) {
  CXFA_LayoutProcessor* pLayoutPro = m_pDocument->GetLayoutProcessor();
  if (!pLayoutPro) {
    return;
  }
  IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
  if (!pNotify) {
    return;
  }
  FX_DWORD dwPacket = this->GetPacketID();
  if (dwPacket & XFA_XDPPACKET_Form) {
    FX_BOOL bNeedFindContainer = FALSE;
    XFA_ELEMENT eType = this->GetClassID();
    switch (eType) {
      case XFA_ELEMENT_Caption:
        bNeedFindContainer = TRUE;
        pNotify->OnNodeEvent(this, XFA_NODEEVENT_ValueChanged, eAttribute,
                             eValue, this,
                             this->GetNodeItem(XFA_NODEITEM_Parent));
        break;
      case XFA_ELEMENT_Font:
      case XFA_ELEMENT_Para: {
        bNeedFindContainer = TRUE;
        CXFA_Node* pParentNode = this->GetNodeItem(XFA_NODEITEM_Parent);
        if (pParentNode->GetClassID() == XFA_ELEMENT_Caption) {
          pNotify->OnNodeEvent(this, XFA_NODEEVENT_ValueChanged, eAttribute,
                               eValue, pParentNode,
                               pParentNode->GetNodeItem(XFA_NODEITEM_Parent));
        } else {
          pNotify->OnNodeEvent(this, XFA_NODEEVENT_ValueChanged, eAttribute,
                               eValue, this, pParentNode);
        }
      } break;
      case XFA_ELEMENT_Margin: {
        bNeedFindContainer = TRUE;
        CXFA_Node* pParentNode = this->GetNodeItem(XFA_NODEITEM_Parent);
        XFA_ELEMENT eParentType = pParentNode->GetClassID();
        if (pParentNode->IsContainerNode()) {
          pNotify->OnNodeEvent(this, XFA_NODEEVENT_ValueChanged, eAttribute,
                               eValue, this, pParentNode);
        } else if (eParentType == XFA_ELEMENT_Caption) {
          pNotify->OnNodeEvent(this, XFA_NODEEVENT_ValueChanged, eAttribute,
                               eValue, pParentNode,
                               pParentNode->GetNodeItem(XFA_NODEITEM_Parent));
        } else {
          CXFA_Node* pNode = pParentNode->GetNodeItem(XFA_NODEITEM_Parent);
          if (pNode && pNode->GetClassID() == XFA_ELEMENT_Ui) {
            pNotify->OnNodeEvent(this, XFA_NODEEVENT_ValueChanged, eAttribute,
                                 eValue, pNode,
                                 pNode->GetNodeItem(XFA_NODEITEM_Parent));
          }
        }
      } break;
      case XFA_ELEMENT_Comb: {
        CXFA_Node* pEditNode = this->GetNodeItem(XFA_NODEITEM_Parent);
        XFA_ELEMENT eUIType = pEditNode->GetClassID();
        if (pEditNode && (eUIType == XFA_ELEMENT_DateTimeEdit ||
                          eUIType == XFA_ELEMENT_NumericEdit ||
                          eUIType == XFA_ELEMENT_TextEdit)) {
          CXFA_Node* pUINode = pEditNode->GetNodeItem(XFA_NODEITEM_Parent);
          if (pUINode) {
            pNotify->OnNodeEvent(this, XFA_NODEEVENT_ValueChanged, eAttribute,
                                 eValue, pUINode,
                                 pUINode->GetNodeItem(XFA_NODEITEM_Parent));
          }
        }
      } break;
      case XFA_ELEMENT_Button:
      case XFA_ELEMENT_Barcode:
      case XFA_ELEMENT_ChoiceList:
      case XFA_ELEMENT_DateTimeEdit:
      case XFA_ELEMENT_NumericEdit:
      case XFA_ELEMENT_PasswordEdit:
      case XFA_ELEMENT_TextEdit: {
        CXFA_Node* pUINode = this->GetNodeItem(XFA_NODEITEM_Parent);
        if (pUINode) {
          pNotify->OnNodeEvent(this, XFA_NODEEVENT_ValueChanged, eAttribute,
                               eValue, pUINode,
                               pUINode->GetNodeItem(XFA_NODEITEM_Parent));
        }
      } break;
      case XFA_ELEMENT_CheckButton: {
        bNeedFindContainer = TRUE;
        CXFA_Node* pUINode = this->GetNodeItem(XFA_NODEITEM_Parent);
        if (pUINode) {
          pNotify->OnNodeEvent(this, XFA_NODEEVENT_ValueChanged, eAttribute,
                               eValue, pUINode,
                               pUINode->GetNodeItem(XFA_NODEITEM_Parent));
        }
      } break;
      case XFA_ELEMENT_Keep:
      case XFA_ELEMENT_Bookend:
      case XFA_ELEMENT_Break:
      case XFA_ELEMENT_BreakAfter:
      case XFA_ELEMENT_BreakBefore:
      case XFA_ELEMENT_Overflow:
        bNeedFindContainer = TRUE;
        break;
      case XFA_ELEMENT_Area:
      case XFA_ELEMENT_Draw:
      case XFA_ELEMENT_ExclGroup:
      case XFA_ELEMENT_Field:
      case XFA_ELEMENT_Subform:
      case XFA_ELEMENT_SubformSet:
        pLayoutPro->AddChangedContainer(this);
        pNotify->OnNodeEvent(this, XFA_NODEEVENT_ValueChanged, eAttribute,
                             eValue, this, this);
        break;
      case XFA_ELEMENT_Sharptext:
      case XFA_ELEMENT_Sharpxml:
      case XFA_ELEMENT_SharpxHTML: {
        CXFA_Node* pTextNode = this->GetNodeItem(XFA_NODEITEM_Parent);
        if (!pTextNode) {
          return;
        }
        CXFA_Node* pValueNode = pTextNode->GetNodeItem(XFA_NODEITEM_Parent);
        if (!pValueNode) {
          return;
        }
        XFA_ELEMENT eType = pValueNode->GetClassID();
        if (eType == XFA_ELEMENT_Value) {
          bNeedFindContainer = TRUE;
          CXFA_Node* pNode = pValueNode->GetNodeItem(XFA_NODEITEM_Parent);
          if (pNode && pNode->IsContainerNode()) {
            if (bScriptModify) {
              pValueNode = pNode;
            }
            pNotify->OnNodeEvent(this, XFA_NODEEVENT_ValueChanged, eAttribute,
                                 eValue, pValueNode, pNode);
          } else {
            pNotify->OnNodeEvent(this, XFA_NODEEVENT_ValueChanged, eAttribute,
                                 eValue, pNode,
                                 pNode->GetNodeItem(XFA_NODEITEM_Parent));
          }
        } else {
          if (eType == XFA_ELEMENT_Items) {
            CXFA_Node* pNode = pValueNode->GetNodeItem(XFA_NODEITEM_Parent);
            if (pNode && pNode->IsContainerNode()) {
              pNotify->OnNodeEvent(this, XFA_NODEEVENT_ValueChanged, eAttribute,
                                   eValue, pValueNode, pNode);
            }
          }
        }
      } break;
      default:
        break;
    }
    if (bNeedFindContainer) {
      CXFA_Node* pParent = this;
      while (pParent) {
        if (pParent->IsContainerNode()) {
          break;
        }
        pParent = pParent->GetNodeItem(XFA_NODEITEM_Parent);
      }
      if (pParent) {
        pLayoutPro->AddChangedContainer(pParent);
      }
    }
  } else {
    pNotify->OnNodeEvent(this, XFA_NODEEVENT_ValueChanged, eAttribute, eValue,
                         this, this);
  }
}
void CXFA_Node::Script_Attribute_String(FXJSE_HVALUE hValue,
                                        FX_BOOL bSetting,
                                        XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    CFX_ByteString szValue;
    FXJSE_Value_ToUTF8String(hValue, szValue);
    CFX_WideString wsValue =
        CFX_WideString::FromUTF8(szValue, szValue.GetLength());
    SetAttribute(eAttribute, wsValue, TRUE);
    if (eAttribute == XFA_ATTRIBUTE_Use &&
        this->GetClassID() == XFA_ELEMENT_Desc) {
      CFX_WideString wsUseVal = wsValue, wsID, wsSOM;
      CXFA_Node* pTemplateNode =
          (CXFA_Node*)m_pDocument->GetXFANode(XFA_HASHCODE_Template);
      CXFA_Node* pProtoRoot =
          pTemplateNode->GetFirstChildByClass(XFA_ELEMENT_Subform)
              ->GetFirstChildByClass(XFA_ELEMENT_Proto);
      if (!wsUseVal.IsEmpty()) {
        if (wsUseVal[0] == '#') {
          wsID = CFX_WideString((const FX_WCHAR*)wsUseVal + 1,
                                wsUseVal.GetLength() - 1);
        } else {
          wsSOM =
              CFX_WideString((const FX_WCHAR*)wsUseVal, wsUseVal.GetLength());
        }
      }
      CXFA_Node* pProtoNode = NULL;
      if (!wsSOM.IsEmpty()) {
        FX_DWORD dwFlag = XFA_RESOLVENODE_Children |
                          XFA_RESOLVENODE_Attributes |
                          XFA_RESOLVENODE_Properties | XFA_RESOLVENODE_Parent |
                          XFA_RESOLVENODE_Siblings;
        XFA_RESOLVENODE_RS resoveNodeRS;
        int32_t iRet = m_pDocument->GetScriptContext()->ResolveObjects(
            pProtoRoot, wsSOM, resoveNodeRS, dwFlag);
        if (iRet > 0 && resoveNodeRS.nodes[0]->IsNode()) {
          pProtoNode = (CXFA_Node*)resoveNodeRS.nodes[0];
        }
      } else if (!wsID.IsEmpty()) {
        pProtoNode = m_pDocument->GetNodeByID(pProtoRoot, wsID);
      }
      if (pProtoNode) {
        CXFA_Node* pHeadChild = GetNodeItem(XFA_NODEITEM_FirstChild);
        while (pHeadChild) {
          CXFA_Node* pSibling =
              pHeadChild->GetNodeItem(XFA_NODEITEM_NextSibling);
          RemoveChild(pHeadChild);
          pHeadChild = pSibling;
        }
        CXFA_Node* pProtoForm = pProtoNode->CloneTemplateToForm(TRUE);
        pHeadChild = pProtoForm->GetNodeItem(XFA_NODEITEM_FirstChild);
        while (pHeadChild) {
          CXFA_Node* pSibling =
              pHeadChild->GetNodeItem(XFA_NODEITEM_NextSibling);
          pProtoForm->RemoveChild(pHeadChild);
          InsertChild(pHeadChild);
          pHeadChild = pSibling;
        }
        m_pDocument->RemovePurgeNode(pProtoForm);
        delete pProtoForm;
      }
    }
  } else {
    CFX_WideString wsValue;
    GetAttribute(eAttribute, wsValue);
    FXJSE_Value_SetUTF8String(hValue,
                              FX_UTF8Encode(wsValue, wsValue.GetLength()));
  }
}
void CXFA_Node::Script_Attribute_StringRead(FXJSE_HVALUE hValue,
                                            FX_BOOL bSetting,
                                            XFA_ATTRIBUTE eAttribute) {
  if (!bSetting) {
    CFX_WideString wsValue;
    GetAttribute(eAttribute, wsValue);
    FXJSE_Value_SetUTF8String(hValue,
                              FX_UTF8Encode(wsValue, wsValue.GetLength()));
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
  }
}
void CXFA_Node::Script_WsdlConnection_Execute(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if ((argc == 0) || (argc == 1)) {
    FXJSE_Value_SetBoolean(pArguments->GetReturnValue(), FALSE);
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"execute");
  }
}
void CXFA_Node::Script_Delta_Restore(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"restore");
  }
}
void CXFA_Node::Script_Delta_CurrentValue(FXJSE_HVALUE hValue,
                                          FX_BOOL bSetting,
                                          XFA_ATTRIBUTE eAttribute) {}
void CXFA_Node::Script_Delta_SavedValue(FXJSE_HVALUE hValue,
                                        FX_BOOL bSetting,
                                        XFA_ATTRIBUTE eAttribute) {}
void CXFA_Node::Script_Delta_Target(FXJSE_HVALUE hValue,
                                    FX_BOOL bSetting,
                                    XFA_ATTRIBUTE eAttribute) {}
void CXFA_Node::Script_Som_Message(FXJSE_HVALUE hValue,
                                   FX_BOOL bSetting,
                                   XFA_SOM_MESSAGETYPE iMessageType) {
  CXFA_WidgetData* pWidgetData = GetWidgetData();
  if (!pWidgetData) {
    return;
  }
  FX_BOOL bNew = FALSE;
  CXFA_Validate validate = pWidgetData->GetValidate();
  if (!validate) {
    validate = pWidgetData->GetValidate(TRUE);
    bNew = TRUE;
  }
  if (bSetting) {
    CFX_ByteString bsMessage;
    FXJSE_Value_ToUTF8String(hValue, bsMessage);
    switch (iMessageType) {
      case XFA_SOM_ValidationMessage:
        validate.SetScriptMessageText(
            CFX_WideString::FromUTF8(bsMessage, bsMessage.GetLength()));
        break;
      case XFA_SOM_FormatMessage:
        validate.SetFormatMessageText(
            CFX_WideString::FromUTF8(bsMessage, bsMessage.GetLength()));
        break;
      case XFA_SOM_MandatoryMessage:
        validate.SetNullMessageText(
            CFX_WideString::FromUTF8(bsMessage, bsMessage.GetLength()));
        break;
      default:
        break;
    }
    if (!bNew) {
      IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
      if (!pNotify) {
        return;
      }
      pNotify->AddCalcValidate(this);
    }
  } else {
    CFX_WideString wsMessage;
    switch (iMessageType) {
      case XFA_SOM_ValidationMessage:
        validate.GetScriptMessageText(wsMessage);
        break;
      case XFA_SOM_FormatMessage:
        validate.GetFormatMessageText(wsMessage);
        break;
      case XFA_SOM_MandatoryMessage:
        validate.GetNullMessageText(wsMessage);
        break;
      default:
        break;
    }
    FXJSE_Value_SetUTF8String(hValue, FX_UTF8Encode(wsMessage));
  }
}
void CXFA_Node::Script_Som_ValidationMessage(FXJSE_HVALUE hValue,
                                             FX_BOOL bSetting,
                                             XFA_ATTRIBUTE eAttribute) {
  Script_Som_Message(hValue, bSetting, XFA_SOM_ValidationMessage);
}
void CXFA_Node::Script_Field_Length(FXJSE_HVALUE hValue,
                                    FX_BOOL bSetting,
                                    XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
  } else {
    CXFA_WidgetData* pWidgetData = GetWidgetData();
    if (!pWidgetData) {
      FXJSE_Value_SetInteger(hValue, 0);
      return;
    }
    FXJSE_Value_SetInteger(hValue, pWidgetData->CountChoiceListItems(TRUE));
  }
}
void CXFA_Node::Script_Som_DefaultValue(FXJSE_HVALUE hValue,
                                        FX_BOOL bSetting,
                                        XFA_ATTRIBUTE eAttribute) {
  XFA_ELEMENT classID = GetClassID();
  if (classID == XFA_ELEMENT_Field) {
    Script_Field_DefaultValue(hValue, bSetting, eAttribute);
    return;
  } else if (classID == XFA_ELEMENT_Draw) {
    Script_Draw_DefaultValue(hValue, bSetting, eAttribute);
    return;
  } else if (classID == XFA_ELEMENT_Boolean) {
    Script_Boolean_Value(hValue, bSetting, eAttribute);
    return;
  }
  if (bSetting) {
    CFX_ByteString newValue;
    if (!(FXJSE_Value_IsNull(hValue) || FXJSE_Value_IsUndefined(hValue))) {
      FXJSE_Value_ToUTF8String(hValue, newValue);
    }
    CFX_WideString wsNewValue =
        CFX_WideString::FromUTF8(newValue, newValue.GetLength());
    CFX_WideString wsFormatValue(wsNewValue);
    CXFA_WidgetData* pContainerWidgetData = NULL;
    if (GetPacketID() == XFA_XDPPACKET_Datasets) {
      CXFA_NodeArray formNodes;
      this->GetBindItems(formNodes);
      CFX_WideString wsPicture;
      for (int32_t i = 0; i < formNodes.GetSize(); i++) {
        CXFA_Node* pFormNode = formNodes.GetAt(i);
        if (!pFormNode || pFormNode->HasFlag(XFA_NODEFLAG_HasRemoved)) {
          continue;
        }
        pContainerWidgetData = pFormNode->GetContainerWidgetData();
        if (pContainerWidgetData) {
          pContainerWidgetData->GetPictureContent(wsPicture,
                                                  XFA_VALUEPICTURE_DataBind);
        }
        if (!wsPicture.IsEmpty()) {
          break;
        }
        pContainerWidgetData = NULL;
      }
    } else if (GetPacketID() == XFA_XDPPACKET_Form) {
      pContainerWidgetData = GetContainerWidgetData();
    }
    if (pContainerWidgetData) {
      pContainerWidgetData->GetFormatDataValue(wsNewValue, wsFormatValue);
    }
    SetScriptContent(wsNewValue, wsFormatValue, TRUE, TRUE);
  } else {
    CFX_WideString content = GetScriptContent(TRUE);
    if (content.IsEmpty() && classID != XFA_ELEMENT_Text &&
        classID != XFA_ELEMENT_SubmitUrl) {
      FXJSE_Value_SetNull(hValue);
    } else if (classID == XFA_ELEMENT_Integer) {
      FXJSE_Value_SetInteger(hValue, FXSYS_wtoi(content));
    } else if (classID == XFA_ELEMENT_Float || classID == XFA_ELEMENT_Decimal) {
      CFX_Decimal decimal(content);
      FXJSE_Value_SetFloat(hValue, (FX_FLOAT)(double)decimal);
    } else {
      FXJSE_Value_SetUTF8String(hValue,
                                FX_UTF8Encode(content, content.GetLength()));
    }
  }
}
void CXFA_Node::Script_Som_DefaultValue_Read(FXJSE_HVALUE hValue,
                                             FX_BOOL bSetting,
                                             XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
    return;
  }
  CFX_WideString content = GetScriptContent(TRUE);
  if (content.IsEmpty()) {
    FXJSE_Value_SetNull(hValue);
  } else {
    FXJSE_Value_SetUTF8String(hValue,
                              FX_UTF8Encode(content, content.GetLength()));
  }
}
void CXFA_Node::Script_Boolean_Value(FXJSE_HVALUE hValue,
                                     FX_BOOL bSetting,
                                     XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    CFX_ByteString newValue;
    if (!(FXJSE_Value_IsNull(hValue) || FXJSE_Value_IsUndefined(hValue))) {
      FXJSE_Value_ToUTF8String(hValue, newValue);
    }
    int32_t iValue = FXSYS_atoi(newValue);
    CFX_WideString wsNewValue = (iValue == 0) ? FX_WSTRC(L"0") : FX_WSTRC(L"1");
    CFX_WideString wsFormatValue(wsNewValue);
    CXFA_WidgetData* pContainerWidgetData = GetContainerWidgetData();
    if (pContainerWidgetData) {
      pContainerWidgetData->GetFormatDataValue(wsNewValue, wsFormatValue);
    }
    SetScriptContent(wsNewValue, wsFormatValue, TRUE, TRUE);
  } else {
    CFX_WideString wsValue = GetScriptContent(TRUE);
    FXJSE_Value_SetBoolean(hValue, wsValue.Equal(FX_WSTRC(L"1")));
  }
}
struct XFA_ExecEventParaInfo {
 public:
  uint32_t m_uHash;
  const FX_WCHAR* m_lpcEventName;
  XFA_EVENTTYPE m_eventType;
  uint32_t m_validFlags;
};
static const XFA_ExecEventParaInfo gs_eventParaInfos[] = {
    {0x02a6c55a, L"postSubmit", XFA_EVENT_PostSubmit, 0},
    {0x0ab466bb, L"preSubmit", XFA_EVENT_PreSubmit, 0},
    {0x109d7ce7, L"mouseEnter", XFA_EVENT_MouseEnter, 5},
    {0x17fad373, L"postPrint", XFA_EVENT_PostPrint, 0},
    {0x1bfc72d9, L"preOpen", XFA_EVENT_PreOpen, 7},
    {0x2196a452, L"initialize", XFA_EVENT_Initialize, 1},
    {0x27410f03, L"mouseExit", XFA_EVENT_MouseExit, 5},
    {0x33c43dec, L"docClose", XFA_EVENT_DocClose, 0},
    {0x361fa1b6, L"preSave", XFA_EVENT_PreSave, 0},
    {0x36f1c6d8, L"preSign", XFA_EVENT_PreSign, 6},
    {0x4731d6ba, L"exit", XFA_EVENT_Exit, 2},
    {0x56bf456b, L"docReady", XFA_EVENT_DocReady, 0},
    {0x7233018a, L"validate", XFA_EVENT_Validate, 1},
    {0x8808385e, L"indexChange", XFA_EVENT_IndexChange, 3},
    {0x891f4606, L"change", XFA_EVENT_Change, 4},
    {0x9528a7b4, L"prePrint", XFA_EVENT_PrePrint, 0},
    {0x9f693b21, L"mouseDown", XFA_EVENT_MouseDown, 5},
    {0xcdce56b3, L"full", XFA_EVENT_Full, 4},
    {0xd576d08e, L"mouseUp", XFA_EVENT_MouseUp, 5},
    {0xd95657a6, L"click", XFA_EVENT_Click, 4},
    {0xdbfbe02e, L"calculate", XFA_EVENT_Calculate, 1},
    {0xe25fa7b8, L"postOpen", XFA_EVENT_PostOpen, 7},
    {0xe28dce7e, L"enter", XFA_EVENT_Enter, 2},
    {0xfc82d695, L"postSave", XFA_EVENT_PostSave, 0},
    {0xfd54fbb7, L"postSign", XFA_EVENT_PostSign, 6},
};
const XFA_ExecEventParaInfo* GetEventParaInfoByName(
    const CFX_WideStringC& wsEventName) {
  int32_t iLength = wsEventName.GetLength();
  uint32_t uHash = FX_HashCode_String_GetW(wsEventName.GetPtr(), iLength);
  const XFA_ExecEventParaInfo* eventParaInfo = NULL;
  int32_t iStart = 0,
          iEnd = (sizeof(gs_eventParaInfos) / sizeof(gs_eventParaInfos[0])) - 1;
  int32_t iMid = (iStart + iEnd) / 2;
  do {
    iMid = (iStart + iEnd) / 2;
    eventParaInfo = &gs_eventParaInfos[iMid];
    if (uHash == eventParaInfo->m_uHash) {
      return eventParaInfo;
    } else if (uHash < eventParaInfo->m_uHash) {
      iEnd = iMid - 1;
    } else {
      iStart = iMid + 1;
    }
  } while (iStart <= iEnd);
  return NULL;
}
void XFA_STRING_TO_RGB(CFX_WideString& strRGB,
                       int32_t& r,
                       int32_t& g,
                       int32_t& b) {
  r = 0;
  g = 0;
  b = 0;
  FX_WCHAR zero = '0';
  int32_t iIndex = 0;
  int32_t iLen = strRGB.GetLength();
  for (int32_t i = 0; i < iLen; ++i) {
    FX_WCHAR ch = strRGB.GetAt(i);
    if (ch == L',') {
      ++iIndex;
    }
    if (iIndex > 2) {
      break;
    }
    int32_t iValue = ch - zero;
    if (iValue >= 0 && iValue <= 9) {
      switch (iIndex) {
        case 0:
          r = r * 10 + iValue;
          break;
        case 1:
          g = g * 10 + iValue;
          break;
        default:
          b = b * 10 + iValue;
          break;
      }
    }
  }
}
void CXFA_Node::Script_Som_BorderColor(FXJSE_HVALUE hValue,
                                       FX_BOOL bSetting,
                                       XFA_ATTRIBUTE eAttribute) {
  CXFA_WidgetData* pWidgetData = GetWidgetData();
  if (!pWidgetData) {
    return;
  }
  CXFA_Border border = pWidgetData->GetBorder(TRUE);
  int32_t iSize = border.CountEdges();
  CFX_WideString strColor;
  if (bSetting) {
    CFX_ByteString bsValue;
    FXJSE_Value_ToUTF8String(hValue, bsValue);
    strColor = CFX_WideString::FromUTF8(bsValue, bsValue.GetLength());
    int32_t r = 0, g = 0, b = 0;
    XFA_STRING_TO_RGB(strColor, r, g, b);
    FX_ARGB rgb = ArgbEncode(100, r, g, b);
    for (int32_t i = 0; i < iSize; ++i) {
      CXFA_Edge edge = border.GetEdge(i);
      edge.SetColor(rgb);
    }
  } else {
    CXFA_Edge edge = border.GetEdge(0);
    FX_ARGB color = edge.GetColor();
    int32_t a, r, g, b;
    ArgbDecode(color, a, r, g, b);
    strColor.Format(L"%d,%d,%d", r, g, b);
    FXJSE_Value_SetUTF8String(hValue, FX_UTF8Encode(strColor));
  }
}
void CXFA_Node::Script_Som_BorderWidth(FXJSE_HVALUE hValue,
                                       FX_BOOL bSetting,
                                       XFA_ATTRIBUTE eAttribute) {
  CXFA_WidgetData* pWidgetData = GetWidgetData();
  if (!pWidgetData) {
    return;
  }
  CXFA_Border border = pWidgetData->GetBorder(TRUE);
  int32_t iSize = border.CountEdges();
  CFX_WideString wsThickness;
  if (bSetting) {
    CFX_ByteString bsValue;
    FXJSE_Value_ToUTF8String(hValue, bsValue);
    wsThickness = CFX_WideString::FromUTF8(bsValue, bsValue.GetLength());
    for (int32_t i = 0; i < iSize; ++i) {
      CXFA_Edge edge = border.GetEdge(i);
      CXFA_Measurement thickness(wsThickness);
      edge.SetMSThickness(thickness);
    }
  } else {
    CXFA_Edge edge = border.GetEdge(0);
    CXFA_Measurement thickness = edge.GetMSThickness();
    thickness.ToString(wsThickness);
    FXJSE_Value_SetUTF8String(hValue, FX_UTF8Encode(wsThickness));
  }
}
void CXFA_Node::Script_Som_FillColor(FXJSE_HVALUE hValue,
                                     FX_BOOL bSetting,
                                     XFA_ATTRIBUTE eAttribute) {
  CXFA_WidgetData* pWidgetData = GetWidgetData();
  if (!pWidgetData) {
    return;
  }
  CXFA_Border border = pWidgetData->GetBorder(TRUE);
  CXFA_Fill borderfill = border.GetFill(TRUE);
  CXFA_Node* pNode = (CXFA_Node*)borderfill;
  if (!pNode) {
    return;
  }
  CFX_WideString wsColor;
  if (bSetting) {
    CFX_ByteString bsValue;
    FXJSE_Value_ToUTF8String(hValue, bsValue);
    wsColor = CFX_WideString::FromUTF8(bsValue, bsValue.GetLength());
    int32_t r, g, b;
    XFA_STRING_TO_RGB(wsColor, r, g, b);
    FX_ARGB color = ArgbEncode(0xff, r, g, b);
    borderfill.SetColor(color);
  } else {
    FX_ARGB color = borderfill.GetColor();
    int32_t a, r, g, b;
    ArgbDecode(color, a, r, g, b);
    wsColor.Format(L"%d,%d,%d", r, g, b);
    FXJSE_Value_SetUTF8String(hValue, FX_UTF8Encode(wsColor));
  }
}
void CXFA_Node::Script_Som_DataNode(FXJSE_HVALUE hValue,
                                    FX_BOOL bSetting,
                                    XFA_ATTRIBUTE eAttribute) {
  if (!bSetting) {
    CXFA_Node* pDataNode = GetBindData();
    if (pDataNode) {
      FXJSE_Value_Set(
          hValue,
          m_pDocument->GetScriptContext()->GetJSValueFromMap(pDataNode));
    } else {
      FXJSE_Value_SetNull(hValue);
    }
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
  }
}
void CXFA_Node::Script_Draw_DefaultValue(FXJSE_HVALUE hValue,
                                         FX_BOOL bSetting,
                                         XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    if (FXJSE_Value_IsUTF8String(hValue)) {
      CXFA_WidgetData* pWidgetData = GetWidgetData();
      FXSYS_assert(pWidgetData);
      XFA_ELEMENT uiType = pWidgetData->GetUIType();
      if (uiType == XFA_ELEMENT_Text) {
        CFX_ByteString newValue;
        FXJSE_Value_ToUTF8String(hValue, newValue);
        CFX_WideString wsNewValue =
            CFX_WideString::FromUTF8(newValue, newValue.GetLength());
        CFX_WideString wsFormatValue(wsNewValue);
        SetScriptContent(wsNewValue, wsFormatValue, TRUE, TRUE);
      } else if (uiType != XFA_ELEMENT_Image) {
      }
    }
  } else {
    CFX_WideString content = GetScriptContent(TRUE);
    if (content.IsEmpty()) {
      FXJSE_Value_SetNull(hValue);
    } else {
      FXJSE_Value_SetUTF8String(hValue,
                                FX_UTF8Encode(content, content.GetLength()));
    }
  }
}
void CXFA_Node::Script_Field_DefaultValue(FXJSE_HVALUE hValue,
                                          FX_BOOL bSetting,
                                          XFA_ATTRIBUTE eAttribute) {
  CXFA_WidgetData* pWidgetData = GetWidgetData();
  if (!pWidgetData) {
    return;
  }
  if (bSetting) {
    if (FXJSE_Value_IsNull(hValue)) {
      pWidgetData->m_bPreNull = pWidgetData->m_bIsNull;
      pWidgetData->m_bIsNull = TRUE;
    } else {
      pWidgetData->m_bPreNull = pWidgetData->m_bIsNull;
      pWidgetData->m_bIsNull = FALSE;
    }
    CFX_ByteString newValue;
    if (!(FXJSE_Value_IsNull(hValue) || FXJSE_Value_IsUndefined(hValue))) {
      FXJSE_Value_ToUTF8String(hValue, newValue);
    }
    CFX_WideString wsNewText =
        CFX_WideString::FromUTF8(newValue, newValue.GetLength());
    CXFA_Node* pUIChild = pWidgetData->GetUIChild();
    if (pUIChild->GetClassID() == XFA_ELEMENT_NumericEdit) {
      int32_t iLeadDigits = 0;
      int32_t iFracDigits = 0;
      pWidgetData->GetLeadDigits(iLeadDigits);
      pWidgetData->GetFracDigits(iFracDigits);
      wsNewText = XFA_NumericLimit(wsNewText, iLeadDigits, iFracDigits);
    }
    CXFA_WidgetData* pContainerWidgetData = GetContainerWidgetData();
    CFX_WideString wsFormatText(wsNewText);
    if (pContainerWidgetData) {
      pContainerWidgetData->GetFormatDataValue(wsNewText, wsFormatText);
    }
    SetScriptContent(wsNewText, wsFormatText, TRUE, TRUE);
  } else {
    CFX_WideString content = GetScriptContent(TRUE);
    if (content.IsEmpty()) {
      FXJSE_Value_SetNull(hValue);
    } else {
      CXFA_Node* pUIChild = pWidgetData->GetUIChild();
      XFA_ELEMENT eUI = pUIChild->GetClassID();
      CXFA_Value defVal = pWidgetData->GetFormValue();
      CXFA_Node* pNode = defVal.GetNode()->GetNodeItem(XFA_NODEITEM_FirstChild);
      if (pNode && pNode->GetClassID() == XFA_ELEMENT_Decimal) {
        if (eUI == XFA_ELEMENT_NumericEdit &&
            (pNode->GetInteger(XFA_ATTRIBUTE_FracDigits) == -1)) {
          FXJSE_Value_SetUTF8String(
              hValue, FX_UTF8Encode(content, content.GetLength()));
        } else {
          CFX_Decimal decimal(content);
          FXJSE_Value_SetFloat(hValue, (FX_FLOAT)(double)decimal);
        }
      } else if (pNode && pNode->GetClassID() == XFA_ELEMENT_Integer) {
        FXJSE_Value_SetInteger(hValue, FXSYS_wtoi(content));
      } else if (pNode && pNode->GetClassID() == XFA_ELEMENT_Boolean) {
        FXJSE_Value_SetBoolean(hValue, FXSYS_wtoi(content) == 0 ? FALSE : TRUE);
      } else if (pNode && pNode->GetClassID() == XFA_ELEMENT_Float) {
        CFX_Decimal decimal(content);
        FXJSE_Value_SetFloat(hValue, (FX_FLOAT)(double)decimal);
      } else {
        FXJSE_Value_SetUTF8String(hValue,
                                  FX_UTF8Encode(content, content.GetLength()));
      }
    }
  }
}
void CXFA_Node::Script_Field_EditValue(FXJSE_HVALUE hValue,
                                       FX_BOOL bSetting,
                                       XFA_ATTRIBUTE eAttribute) {
  CXFA_WidgetData* pWidgetData = GetWidgetData();
  if (!pWidgetData) {
    return;
  }
  CFX_WideString wsValue;
  if (bSetting) {
    CFX_ByteString bsValue;
    FXJSE_Value_ToUTF8String(hValue, bsValue);
    wsValue = CFX_WideString::FromUTF8(bsValue, bsValue.GetLength());
    pWidgetData->SetValue(wsValue, XFA_VALUEPICTURE_Edit);
  } else {
    pWidgetData->GetValue(wsValue, XFA_VALUEPICTURE_Edit);
    FXJSE_Value_SetUTF8String(hValue, FX_UTF8Encode(wsValue));
  }
}
void CXFA_Node::Script_Som_FontColor(FXJSE_HVALUE hValue,
                                     FX_BOOL bSetting,
                                     XFA_ATTRIBUTE eAttribute) {
  CXFA_WidgetData* pWidgetData = GetWidgetData();
  if (!pWidgetData) {
    return;
  }
  CXFA_Font font = pWidgetData->GetFont(TRUE);
  CXFA_Node* pNode = (CXFA_Node*)font;
  if (!pNode) {
    return;
  }
  CFX_WideString wsColor;
  if (bSetting) {
    CFX_ByteString bsValue;
    FXJSE_Value_ToUTF8String(hValue, bsValue);
    wsColor = CFX_WideString::FromUTF8(bsValue, bsValue.GetLength());
    int32_t r, g, b;
    XFA_STRING_TO_RGB(wsColor, r, g, b);
    FX_ARGB color = ArgbEncode(0xff, r, g, b);
    font.SetColor(color);
  } else {
    FX_ARGB color = font.GetColor();
    int32_t a, r, g, b;
    ArgbDecode(color, a, r, g, b);
    wsColor.Format(L"%d,%d,%d", r, g, b);
    FXJSE_Value_SetUTF8String(hValue, FX_UTF8Encode(wsColor));
  }
}
void CXFA_Node::Script_Field_FormatMessage(FXJSE_HVALUE hValue,
                                           FX_BOOL bSetting,
                                           XFA_ATTRIBUTE eAttribute) {
  Script_Som_Message(hValue, bSetting, XFA_SOM_FormatMessage);
}
void CXFA_Node::Script_Field_FormattedValue(FXJSE_HVALUE hValue,
                                            FX_BOOL bSetting,
                                            XFA_ATTRIBUTE eAttribute) {
  CXFA_WidgetData* pWidgetData = GetWidgetData();
  if (!pWidgetData) {
    return;
  }
  CFX_WideString wsValue;
  if (bSetting) {
    CFX_ByteString bsValue;
    FXJSE_Value_ToUTF8String(hValue, bsValue);
    wsValue = CFX_WideString::FromUTF8(bsValue, bsValue.GetLength());
    pWidgetData->SetValue(wsValue, XFA_VALUEPICTURE_Display);
  } else {
    pWidgetData->GetValue(wsValue, XFA_VALUEPICTURE_Display);
    FXJSE_Value_SetUTF8String(hValue, FX_UTF8Encode(wsValue));
  }
}
void CXFA_Node::Script_Som_Mandatory(FXJSE_HVALUE hValue,
                                     FX_BOOL bSetting,
                                     XFA_ATTRIBUTE eAttribute) {
  CXFA_WidgetData* pWidgetData = GetWidgetData();
  if (!pWidgetData) {
    return;
  }
  CXFA_Validate validate = pWidgetData->GetValidate(TRUE);
  CFX_WideString wsValue;
  if (bSetting) {
    CFX_ByteString bsValue;
    FXJSE_Value_ToUTF8String(hValue, bsValue);
    wsValue = CFX_WideString::FromUTF8(bsValue, bsValue.GetLength());
    validate.SetNullTest(wsValue);
  } else {
    int32_t iValue = validate.GetNullTest();
    XFA_LPCATTRIBUTEENUMINFO pInfo =
        XFA_GetAttributeEnumByID((XFA_ATTRIBUTEENUM)iValue);
    if (pInfo) {
      wsValue = pInfo->pName;
    }
    FXJSE_Value_SetUTF8String(hValue, FX_UTF8Encode(wsValue));
  }
}
void CXFA_Node::Script_Som_MandatoryMessage(FXJSE_HVALUE hValue,
                                            FX_BOOL bSetting,
                                            XFA_ATTRIBUTE eAttribute) {
  Script_Som_Message(hValue, bSetting, XFA_SOM_MandatoryMessage);
}
void CXFA_Node::Script_Field_ParentSubform(FXJSE_HVALUE hValue,
                                           FX_BOOL bSetting,
                                           XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
  } else {
    FXJSE_Value_SetNull(hValue);
  }
}
void CXFA_Node::Script_Field_SelectedIndex(FXJSE_HVALUE hValue,
                                           FX_BOOL bSetting,
                                           XFA_ATTRIBUTE eAttribute) {
  CXFA_WidgetData* pWidgetData = GetWidgetData();
  if (!pWidgetData) {
    return;
  }
  if (bSetting) {
    int32_t iIndex = FXJSE_Value_ToInteger(hValue);
    if (iIndex == -1) {
      pWidgetData->ClearAllSelections();
      return;
    }
    pWidgetData->SetItemState(iIndex, TRUE, TRUE, TRUE);
  } else {
    FXJSE_Value_SetInteger(hValue, pWidgetData->GetSelectedItem());
  }
}
void CXFA_Node::Script_Field_ClearItems(CFXJSE_Arguments* pArguments) {
  CXFA_WidgetData* pWidgetData = GetWidgetData();
  if (!pWidgetData) {
    return;
  }
  pWidgetData->DeleteItem(-1, TRUE);
}
void CXFA_Node::Script_Field_ExecEvent(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 1) {
    CFX_ByteString eventString = pArguments->GetUTF8String(0);
    int32_t iRet = execSingleEventByName(
        CFX_WideString::FromUTF8(eventString, eventString.GetLength()),
        XFA_ELEMENT_Field);
    if (eventString == "validate") {
      FXJSE_Value_SetBoolean(pArguments->GetReturnValue(),
                             ((iRet == XFA_EVENTERROR_Error) ? FALSE : TRUE));
    }
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"execEvent");
  }
}
void CXFA_Node::Script_Field_ExecInitialize(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
    IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
    if (!pNotify) {
      return;
    }
    pNotify->ExecEventByDeepFirst(this, XFA_EVENT_Initialize, FALSE, FALSE);
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"execInitialize");
  }
}
void CXFA_Node::Script_Field_DeleteItem(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"deleteItem");
    return;
  }
  CXFA_WidgetData* pWidgetData = GetWidgetData();
  if (!pWidgetData) {
    return;
  }
  int32_t iIndex = pArguments->GetInt32(0);
  FX_BOOL bValue = pWidgetData->DeleteItem(iIndex, TRUE, TRUE);
  FXJSE_HVALUE hValue = pArguments->GetReturnValue();
  if (hValue) {
    FXJSE_Value_SetBoolean(hValue, bValue);
  }
}
void CXFA_Node::Script_Field_GetSaveItem(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"getSaveItem");
    return;
  }
  int32_t iIndex = pArguments->GetInt32(0);
  if (iIndex < 0) {
    FXJSE_Value_SetNull(pArguments->GetReturnValue());
    return;
  }
  CXFA_WidgetData* pWidgetData = GetWidgetData();
  if (!pWidgetData) {
    FXJSE_Value_SetNull(pArguments->GetReturnValue());
    return;
  }
  CFX_WideString wsValue;
  FX_BOOL bHasItem = pWidgetData->GetChoiceListItem(wsValue, iIndex, TRUE);
  if (bHasItem) {
    FXJSE_Value_SetUTF8String(pArguments->GetReturnValue(),
                              FX_UTF8Encode(wsValue, wsValue.GetLength()));
  } else {
    FXJSE_Value_SetNull(pArguments->GetReturnValue());
  }
}
void CXFA_Node::Script_Field_BoundItem(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"boundItem");
    return;
  }
  CXFA_WidgetData* pWidgetData = GetWidgetData();
  if (!pWidgetData) {
    return;
  }
  CFX_ByteString bsValue = pArguments->GetUTF8String(0);
  CFX_WideString wsValue =
      CFX_WideString::FromUTF8(bsValue, bsValue.GetLength());
  CFX_WideString wsBoundValue;
  pWidgetData->GetItemValue(wsValue, wsBoundValue);
  FXJSE_HVALUE hValue = pArguments->GetReturnValue();
  if (hValue) {
    FXJSE_Value_SetUTF8String(hValue, FX_UTF8Encode(wsBoundValue));
  }
}
void CXFA_Node::Script_Field_GetItemState(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"getItemState");
    return;
  }
  CXFA_WidgetData* pWidgetData = GetWidgetData();
  if (!pWidgetData) {
    return;
  }
  int32_t iIndex = pArguments->GetInt32(0);
  FX_BOOL bValue = pWidgetData->GetItemState(iIndex);
  FXJSE_HVALUE hValue = pArguments->GetReturnValue();
  if (hValue) {
    FXJSE_Value_SetBoolean(hValue, bValue);
  }
}
void CXFA_Node::Script_Field_ExecCalculate(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
    IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
    if (!pNotify) {
      return;
    }
    pNotify->ExecEventByDeepFirst(this, XFA_EVENT_Calculate, FALSE, FALSE);
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"execCalculate");
  }
}
void CXFA_Node::Script_Field_SetItems(CFXJSE_Arguments* pArguments) {}
void CXFA_Node::Script_Field_GetDisplayItem(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"getDisplayItem");
    return;
  }
  int32_t iIndex = pArguments->GetInt32(0);
  if (iIndex < 0) {
    FXJSE_Value_SetNull(pArguments->GetReturnValue());
    return;
  }
  CXFA_WidgetData* pWidgetData = GetWidgetData();
  if (!pWidgetData) {
    FXJSE_Value_SetNull(pArguments->GetReturnValue());
    return;
  }
  CFX_WideString wsValue;
  FX_BOOL bHasItem = pWidgetData->GetChoiceListItem(wsValue, iIndex, FALSE);
  if (bHasItem) {
    FXJSE_Value_SetUTF8String(pArguments->GetReturnValue(),
                              FX_UTF8Encode(wsValue, wsValue.GetLength()));
  } else {
    FXJSE_Value_SetNull(pArguments->GetReturnValue());
  }
}
void CXFA_Node::Script_Field_SetItemState(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 2) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"setItemState");
    return;
  }
  CXFA_WidgetData* pWidgetData = GetWidgetData();
  if (!pWidgetData) {
    return;
  }
  int32_t iIndex = pArguments->GetInt32(0);
  FX_BOOL bAdd = pArguments->GetInt32(1) == 0 ? FALSE : TRUE;
  if (bAdd) {
    pWidgetData->SetItemState(iIndex, TRUE, TRUE, TRUE);
  } else {
    if (pWidgetData->GetItemState(iIndex)) {
      pWidgetData->SetItemState(iIndex, FALSE, TRUE, TRUE);
    }
  }
}
void CXFA_Node::Script_Field_AddItem(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 1 || iLength > 2) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"addItem");
    return;
  }
  CXFA_WidgetData* pWidgetData = GetWidgetData();
  if (!pWidgetData) {
    return;
  }
  CFX_WideString wsLabel;
  CFX_WideString wsValue;
  if (iLength >= 1) {
    CFX_ByteString bsLable = pArguments->GetUTF8String(0);
    wsLabel = CFX_WideString::FromUTF8(bsLable, bsLable.GetLength());
  }
  if (iLength >= 2) {
    CFX_ByteString bsValue = pArguments->GetUTF8String(1);
    wsValue = CFX_WideString::FromUTF8(bsValue, bsValue.GetLength());
  }
  pWidgetData->InsertItem(wsLabel, wsValue, -1, TRUE);
}
void CXFA_Node::Script_Field_ExecValidate(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
    IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
    if (!pNotify) {
      FXJSE_Value_SetBoolean(pArguments->GetReturnValue(), FALSE);
    } else {
      int32_t iRet =
          pNotify->ExecEventByDeepFirst(this, XFA_EVENT_Validate, FALSE, FALSE);
      FXJSE_Value_SetBoolean(pArguments->GetReturnValue(),
                             ((iRet == XFA_EVENTERROR_Error) ? FALSE : TRUE));
    }
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"execValidate");
  }
}
void CXFA_Node::Script_ExclGroup_ErrorText(FXJSE_HVALUE hValue,
                                           FX_BOOL bSetting,
                                           XFA_ATTRIBUTE eAttribute) {
  if (!bSetting) {
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
  }
}
void CXFA_Node::Script_ExclGroup_DefaultAndRawValue(FXJSE_HVALUE hValue,
                                                    FX_BOOL bSetting,
                                                    XFA_ATTRIBUTE eAttribute) {
  CXFA_WidgetData* pWidgetData = GetWidgetData();
  if (!pWidgetData) {
    return;
  }
  if (bSetting) {
    CFX_ByteString bsValue;
    FXJSE_Value_ToUTF8String(hValue, bsValue);
    pWidgetData->SetSelectedMemberByValue(
        CFX_WideString::FromUTF8(bsValue, bsValue.GetLength()), TRUE, TRUE);
  } else {
    CFX_WideString wsValue = GetScriptContent(TRUE);
    XFA_VERSION curVersion = GetDocument()->GetCurVersionMode();
    if (wsValue.IsEmpty() && curVersion >= XFA_VERSION_300) {
      FXJSE_Value_SetNull(hValue);
    } else {
      FXJSE_Value_SetUTF8String(hValue, FX_UTF8Encode(wsValue));
    }
  }
}
void CXFA_Node::Script_ExclGroup_Transient(FXJSE_HVALUE hValue,
                                           FX_BOOL bSetting,
                                           XFA_ATTRIBUTE eAttribute) {}
void CXFA_Node::Script_ExclGroup_ExecEvent(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 1) {
    CFX_ByteString eventString = pArguments->GetUTF8String(0);
    execSingleEventByName(
        CFX_WideString::FromUTF8(eventString, eventString.GetLength()),
        XFA_ELEMENT_ExclGroup);
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"execEvent");
  }
}
void CXFA_Node::Script_ExclGroup_SelectedMember(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if ((argc == 0) || (argc == 1)) {
    CXFA_WidgetData* pWidgetData = GetWidgetData();
    if (!pWidgetData) {
      FXJSE_Value_SetNull(pArguments->GetReturnValue());
    } else {
      CXFA_Node* pReturnNode = NULL;
      if (argc == 0) {
        pReturnNode = pWidgetData->GetSelectedMember();
      } else {
        CFX_ByteString szName;
        szName = pArguments->GetUTF8String(0);
        pReturnNode = pWidgetData->SetSelectedMember(
            CFX_WideString::FromUTF8(szName, szName.GetLength()));
      }
      if (pReturnNode) {
        FXJSE_Value_Set(
            pArguments->GetReturnValue(),
            m_pDocument->GetScriptContext()->GetJSValueFromMap(pReturnNode));
      } else {
        FXJSE_Value_SetNull(pArguments->GetReturnValue());
      }
    }
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"selectedMember");
  }
}
void CXFA_Node::Script_ExclGroup_ExecInitialize(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
    IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
    if (!pNotify) {
      return;
    }
    pNotify->ExecEventByDeepFirst(this, XFA_EVENT_Initialize);
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"execInitialize");
  }
}
void CXFA_Node::Script_ExclGroup_ExecCalculate(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
    IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
    if (!pNotify) {
      return;
    }
    pNotify->ExecEventByDeepFirst(this, XFA_EVENT_Calculate);
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"execCalculate");
  }
}
void CXFA_Node::Script_ExclGroup_ExecValidate(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
    IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
    if (!pNotify) {
      FXJSE_Value_SetBoolean(pArguments->GetReturnValue(), FALSE);
    } else {
      int32_t iRet = pNotify->ExecEventByDeepFirst(this, XFA_EVENT_Validate);
      FXJSE_Value_SetBoolean(pArguments->GetReturnValue(),
                             ((iRet == XFA_EVENTERROR_Error) ? FALSE : TRUE));
    }
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"execValidate");
  }
}
static CXFA_Node* XFA_ScriptInstanceManager_GetItem(CXFA_Node* pInstMgrNode,
                                                    int32_t iIndex) {
  ASSERT(pInstMgrNode);
  int32_t iCount = 0;
  FX_DWORD dwNameHash = 0;
  for (CXFA_Node* pNode = pInstMgrNode->GetNodeItem(XFA_NODEITEM_NextSibling);
       pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    XFA_ELEMENT eCurType = pNode->GetClassID();
    if (eCurType == XFA_ELEMENT_InstanceManager) {
      break;
    }
    if ((eCurType != XFA_ELEMENT_Subform) &&
        (eCurType != XFA_ELEMENT_SubformSet)) {
      continue;
    }
    if (iCount == 0) {
      CFX_WideStringC wsName = pNode->GetCData(XFA_ATTRIBUTE_Name);
      CFX_WideStringC wsInstName = pInstMgrNode->GetCData(XFA_ATTRIBUTE_Name);
      if (wsInstName.GetLength() < 1 || wsInstName.GetAt(0) != '_' ||
          wsInstName.Mid(1) != wsName) {
        return NULL;
      }
      dwNameHash = pNode->GetNameHash();
    }
    if (dwNameHash != pNode->GetNameHash()) {
      break;
    }
    iCount++;
    if (iCount > iIndex) {
      return pNode;
    }
  }
  return NULL;
}
void CXFA_Node::Script_Som_InstanceIndex(FXJSE_HVALUE hValue,
                                         FX_BOOL bSetting,
                                         XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    int32_t iTo = FXJSE_Value_ToInteger(hValue);
    int32_t iFrom = Subform_and_SubformSet_InstanceIndex();
    CXFA_Node* pManagerNode = NULL;
    for (CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_PrevSibling);
         pNode != NULL; pNode = pNode->GetNodeItem(XFA_NODEITEM_PrevSibling)) {
      if (pNode->GetClassID() == XFA_ELEMENT_InstanceManager) {
        pManagerNode = pNode;
        break;
      }
    }
    if (pManagerNode) {
      pManagerNode->InstanceManager_MoveInstance(iTo, iFrom);
      IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
      if (!pNotify) {
        return;
      }
      CXFA_Node* pToInstance =
          XFA_ScriptInstanceManager_GetItem(pManagerNode, iTo);
      if (pToInstance && pToInstance->GetClassID() == XFA_ELEMENT_Subform) {
        pNotify->RunSubformIndexChange(pToInstance);
      }
      CXFA_Node* pFromInstance =
          XFA_ScriptInstanceManager_GetItem(pManagerNode, iFrom);
      if (pFromInstance && pFromInstance->GetClassID() == XFA_ELEMENT_Subform) {
        pNotify->RunSubformIndexChange(pFromInstance);
      }
    }
  } else {
    FXJSE_Value_SetInteger(hValue, Subform_and_SubformSet_InstanceIndex());
  }
}
void CXFA_Node::Script_Subform_InstanceManager(FXJSE_HVALUE hValue,
                                               FX_BOOL bSetting,
                                               XFA_ATTRIBUTE eAttribute) {
  if (!bSetting) {
    CFX_WideStringC wsName = this->GetCData(XFA_ATTRIBUTE_Name);
    CXFA_Node* pInstanceMgr = NULL;
    for (CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_PrevSibling);
         pNode != NULL; pNode = pNode->GetNodeItem(XFA_NODEITEM_PrevSibling)) {
      if (pNode->GetClassID() == XFA_ELEMENT_InstanceManager) {
        CFX_WideStringC wsInstMgrName = pNode->GetCData(XFA_ATTRIBUTE_Name);
        if (wsInstMgrName.GetLength() >= 1 && wsInstMgrName.GetAt(0) == '_' &&
            wsInstMgrName.Mid(1) == wsName) {
          pInstanceMgr = pNode;
        }
        break;
      }
    }
    if (pInstanceMgr) {
      FXJSE_Value_Set(
          hValue,
          m_pDocument->GetScriptContext()->GetJSValueFromMap(pInstanceMgr));
    } else {
      FXJSE_Value_SetNull(hValue);
    }
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
  }
}
void CXFA_Node::Script_Subform_Locale(FXJSE_HVALUE hValue,
                                      FX_BOOL bSetting,
                                      XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    CFX_ByteString bsLocaleName;
    FXJSE_Value_ToUTF8String(hValue, bsLocaleName);
    this->SetCData(
        XFA_ATTRIBUTE_Locale,
        CFX_WideString::FromUTF8(bsLocaleName, bsLocaleName.GetLength()), TRUE,
        TRUE);
  } else {
    CFX_WideString wsLocaleName;
    GetLocaleName(wsLocaleName);
    FXJSE_Value_SetUTF8String(
        hValue, FX_UTF8Encode(wsLocaleName, wsLocaleName.GetLength()));
  }
}
void CXFA_Node::Script_Subform_ExecEvent(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 1) {
    CFX_ByteString eventString = pArguments->GetUTF8String(0);
    execSingleEventByName(
        CFX_WideString::FromUTF8(eventString, eventString.GetLength()),
        XFA_ELEMENT_Subform);
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"execEvent");
  }
}
void CXFA_Node::Script_Subform_ExecInitialize(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
    IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
    if (!pNotify) {
      return;
    }
    pNotify->ExecEventByDeepFirst(this, XFA_EVENT_Initialize);
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"execInitialize");
  }
}
void CXFA_Node::Script_Subform_ExecCalculate(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
    IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
    if (!pNotify) {
      return;
    }
    pNotify->ExecEventByDeepFirst(this, XFA_EVENT_Calculate);
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"execCalculate");
  }
}
void CXFA_Node::Script_Subform_ExecValidate(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
    IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
    if (!pNotify) {
      FXJSE_Value_SetBoolean(pArguments->GetReturnValue(), FALSE);
    } else {
      int32_t iRet = pNotify->ExecEventByDeepFirst(this, XFA_EVENT_Validate);
      FXJSE_Value_SetBoolean(pArguments->GetReturnValue(),
                             ((iRet == XFA_EVENTERROR_Error) ? FALSE : TRUE));
    }
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"execValidate");
  }
}
void CXFA_Node::Script_Subform_GetInvalidObjects(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"getInvalidObjects");
  }
}
int32_t CXFA_Node::Subform_and_SubformSet_InstanceIndex() {
  int32_t index = 0;
  for (CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_PrevSibling); pNode != NULL;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_PrevSibling)) {
    if ((pNode->GetClassID() == XFA_ELEMENT_Subform) ||
        (pNode->GetClassID() == XFA_ELEMENT_SubformSet)) {
      index++;
    } else {
      break;
    }
  }
  return index;
}
void CXFA_Node::Script_Template_FormNodes(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 1) {
    FXJSE_Value_SetBoolean(pArguments->GetReturnValue(), TRUE);
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"formNodes");
  }
}
void CXFA_Node::Script_Template_Remerge(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
    m_pDocument->DoDataRemerge(TRUE);
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"remerge");
  }
}
void CXFA_Node::Script_Template_ExecInitialize(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
    CXFA_WidgetData* pWidgetData = GetWidgetData();
    if (!pWidgetData) {
      FXJSE_Value_SetBoolean(pArguments->GetReturnValue(), FALSE);
    } else {
      FXJSE_Value_SetBoolean(pArguments->GetReturnValue(), TRUE);
    }
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"execInitialize");
  }
}
void CXFA_Node::Script_Template_CreateNode(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if ((argc > 0) && (argc < 4)) {
    CFX_WideString strTagName;
    CFX_WideString strName;
    CFX_WideString strNameSpace;
    CFX_ByteString bsTagName = pArguments->GetUTF8String(0);
    strTagName = CFX_WideString::FromUTF8(bsTagName, bsTagName.GetLength());
    if (argc > 1) {
      CFX_ByteString bsName = pArguments->GetUTF8String(1);
      strName = CFX_WideString::FromUTF8(bsName, bsName.GetLength());
      if (argc == 3) {
        CFX_ByteString bsNameSpace = pArguments->GetUTF8String(2);
        strNameSpace =
            CFX_WideString::FromUTF8(bsNameSpace, bsNameSpace.GetLength());
      }
    }
    XFA_LPCELEMENTINFO pElement = XFA_GetElementByName(strTagName);
    CXFA_Node* pNewNode = CreateSamePacketNode(pElement->eName);
    if (!pNewNode) {
      FXJSE_Value_SetNull(pArguments->GetReturnValue());
    } else {
      if (!strName.IsEmpty()) {
        if (XFA_GetAttributeOfElement(pElement->eName, XFA_ATTRIBUTE_Name,
                                      XFA_XDPPACKET_UNKNOWN)) {
          pNewNode->SetAttribute(XFA_ATTRIBUTE_Name, strName, TRUE);
          if (pNewNode->GetPacketID() == XFA_XDPPACKET_Datasets) {
            pNewNode->CreateXMLMappingNode();
          }
          FXJSE_Value_Set(
              pArguments->GetReturnValue(),
              m_pDocument->GetScriptContext()->GetJSValueFromMap(pNewNode));
        } else {
          ThrowScriptErrorMessage(XFA_IDS_NOT_HAVE_PROPERTY,
                                  (const FX_WCHAR*)strTagName, L"name");
        }
      } else {
        FXJSE_Value_Set(
            pArguments->GetReturnValue(),
            m_pDocument->GetScriptContext()->GetJSValueFromMap(pNewNode));
      }
    }
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"createNode");
  }
}
void CXFA_Node::Script_Template_Recalculate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() == 1) {
    FXJSE_Value_SetBoolean(pArguments->GetReturnValue(), TRUE);
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"recalculate");
  }
}
void CXFA_Node::Script_Template_ExecCalculate(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
    CXFA_WidgetData* pWidgetData = GetWidgetData();
    if (!pWidgetData) {
      FXJSE_Value_SetBoolean(pArguments->GetReturnValue(), FALSE);
    } else {
      FXJSE_Value_SetBoolean(pArguments->GetReturnValue(), TRUE);
    }
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"execCalculate");
  }
}
void CXFA_Node::Script_Template_ExecValidate(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
    CXFA_WidgetData* pWidgetData = GetWidgetData();
    if (!pWidgetData) {
      FXJSE_Value_SetBoolean(pArguments->GetReturnValue(), FALSE);
    } else {
      FXJSE_Value_SetBoolean(pArguments->GetReturnValue(), TRUE);
    }
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"execValidate");
  }
}
void CXFA_Node::Script_Manifest_Evaluate(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
    CXFA_WidgetData* pWidgetData = GetWidgetData();
    if (!pWidgetData) {
      FXJSE_Value_SetBoolean(pArguments->GetReturnValue(), FALSE);
    } else {
      FXJSE_Value_SetBoolean(pArguments->GetReturnValue(), TRUE);
    }
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"evaluate");
  }
}
void CXFA_Node::Script_InstanceManager_Max(FXJSE_HVALUE hValue,
                                           FX_BOOL bSetting,
                                           XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
    return;
  } else {
    CXFA_Occur nodeOccur = GetOccurNode();
    FXJSE_Value_SetInteger(hValue, nodeOccur.GetMax());
  }
}
void CXFA_Node::Script_InstanceManager_Min(FXJSE_HVALUE hValue,
                                           FX_BOOL bSetting,
                                           XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
    return;
  } else {
    CXFA_Occur nodeOccur = GetOccurNode();
    FXJSE_Value_SetInteger(hValue, nodeOccur.GetMin());
  }
}
static int32_t XFA_ScriptInstanceManager_GetCount(CXFA_Node* pInstMgrNode) {
  ASSERT(pInstMgrNode);
  int32_t iCount = 0;
  FX_DWORD dwNameHash = 0;
  for (CXFA_Node* pNode = pInstMgrNode->GetNodeItem(XFA_NODEITEM_NextSibling);
       pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    XFA_ELEMENT eCurType = pNode->GetClassID();
    if (eCurType == XFA_ELEMENT_InstanceManager) {
      break;
    }
    if ((eCurType != XFA_ELEMENT_Subform) &&
        (eCurType != XFA_ELEMENT_SubformSet)) {
      continue;
    }
    if (iCount == 0) {
      CFX_WideStringC wsName = pNode->GetCData(XFA_ATTRIBUTE_Name);
      CFX_WideStringC wsInstName = pInstMgrNode->GetCData(XFA_ATTRIBUTE_Name);
      if (wsInstName.GetLength() < 1 || wsInstName.GetAt(0) != '_' ||
          wsInstName.Mid(1) != wsName) {
        return iCount;
      }
      dwNameHash = pNode->GetNameHash();
    }
    if (dwNameHash != pNode->GetNameHash()) {
      break;
    }
    iCount++;
  }
  return iCount;
}
static void
XFA_ScriptInstanceManager_ReorderDataNodes_SortNodeArrayByDocumentIdx(
    const CXFA_NodeSet& rgNodeSet,
    CXFA_NodeArray& rgNodeArray,
    CFX_ArrayTemplate<int32_t>& rgIdxArray) {
  int32_t iCount = rgNodeSet.GetCount();
  rgNodeArray.SetSize(iCount);
  rgIdxArray.SetSize(iCount);
  if (iCount == 0) {
    return;
  }
  int32_t iIndex = -1, iTotalIndex = -1;
  CXFA_Node* pNode = NULL;
  FX_POSITION pos = rgNodeSet.GetStartPosition();
  rgNodeSet.GetNextAssoc(pos, pNode);
  for (pNode = pNode->GetNodeItem(XFA_NODEITEM_Parent)
                   ->GetNodeItem(XFA_NODEITEM_FirstChild);
       pNode && iIndex < iCount;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    iTotalIndex++;
    if (rgNodeSet.Lookup(pNode)) {
      iIndex++;
      rgNodeArray[iIndex] = pNode;
      rgIdxArray[iIndex] = iTotalIndex;
    }
  }
}
struct CXFA_DualNodeArray {
  CXFA_NodeSet firstNodeList;
  CXFA_NodeSet secondNodeList;
};
static void XFA_ScriptInstanceManager_ReorderDataNodes(CXFA_NodeSet& sSet1,
                                                       CXFA_NodeSet& sSet2,
                                                       FX_BOOL bInsertBefore) {
  CFX_MapPtrTemplate<CXFA_Node*,
                     CFX_MapPtrTemplate<FX_DWORD, CXFA_DualNodeArray*>*>
      rgNodeListMap;
  FX_POSITION pos;
  pos = sSet1.GetStartPosition();
  while (pos) {
    CXFA_Node* pNode = NULL;
    sSet1.GetNextAssoc(pos, pNode);
    CXFA_Node* pParentNode = pNode->GetNodeItem(XFA_NODEITEM_Parent);
    FX_DWORD dwNameHash = pNode->GetNameHash();
    if (!pParentNode || !dwNameHash) {
      continue;
    }
    CFX_MapPtrTemplate<FX_DWORD, CXFA_DualNodeArray*>* pNodeListChildMap =
        rgNodeListMap[pParentNode];
    if (!pNodeListChildMap) {
      rgNodeListMap[pParentNode] = pNodeListChildMap =
          new CFX_MapPtrTemplate<FX_DWORD, CXFA_DualNodeArray*>;
    }
    CXFA_DualNodeArray* pDualNodeArray = (*pNodeListChildMap)[dwNameHash];
    if (!pDualNodeArray) {
      (*pNodeListChildMap)[dwNameHash] = pDualNodeArray =
          new CXFA_DualNodeArray;
    }
    pDualNodeArray->firstNodeList.Add(pNode);
  }
  pos = sSet2.GetStartPosition();
  while (pos) {
    CXFA_Node* pNode = NULL;
    sSet2.GetNextAssoc(pos, pNode);
    CXFA_Node* pParentNode = pNode->GetNodeItem(XFA_NODEITEM_Parent);
    FX_DWORD dwNameHash = pNode->GetNameHash();
    if (!pParentNode || !dwNameHash) {
      continue;
    }
    CFX_MapPtrTemplate<FX_DWORD, CXFA_DualNodeArray*>* pNodeListChildMap =
        rgNodeListMap[pParentNode];
    if (!pNodeListChildMap) {
      rgNodeListMap[pParentNode] = pNodeListChildMap =
          new CFX_MapPtrTemplate<FX_DWORD, CXFA_DualNodeArray*>;
    }
    CXFA_DualNodeArray* pDualNodeArray = (*pNodeListChildMap)[dwNameHash];
    if (!pDualNodeArray) {
      (*pNodeListChildMap)[dwNameHash] = pDualNodeArray =
          new CXFA_DualNodeArray;
    }
    if (pDualNodeArray->firstNodeList.Lookup(pNode)) {
      pDualNodeArray->firstNodeList.RemoveKey(pNode);
    } else {
      pDualNodeArray->secondNodeList.Add(pNode);
    }
  }
  pos = rgNodeListMap.GetStartPosition();
  while (pos) {
    CXFA_Node* pParentNode = NULL;
    CFX_MapPtrTemplate<FX_DWORD, CXFA_DualNodeArray*>* pNodeListChildMap = NULL;
    rgNodeListMap.GetNextAssoc(pos, pParentNode, pNodeListChildMap);
    if (!pNodeListChildMap) {
      continue;
    }
    FX_POSITION childpos = pNodeListChildMap->GetStartPosition();
    while (childpos) {
      FX_DWORD dwNameHash = 0;
      CXFA_DualNodeArray* pDualNodeArray = NULL;
      pNodeListChildMap->GetNextAssoc(childpos, dwNameHash, pDualNodeArray);
      if (!pDualNodeArray) {
        continue;
      }
      if (pDualNodeArray->firstNodeList.GetCount() != 0 &&
          pDualNodeArray->secondNodeList.GetCount() != 0) {
        CXFA_NodeArray rgNodeArray1, rgNodeArray2;
        CFX_ArrayTemplate<int32_t> rgIdxArray1, rgIdxArray2;
        XFA_ScriptInstanceManager_ReorderDataNodes_SortNodeArrayByDocumentIdx(
            pDualNodeArray->firstNodeList, rgNodeArray1, rgIdxArray1);
        XFA_ScriptInstanceManager_ReorderDataNodes_SortNodeArrayByDocumentIdx(
            pDualNodeArray->secondNodeList, rgNodeArray2, rgIdxArray2);
        int32_t iLimit;
        CXFA_Node *pParentNode = NULL, *pBeforeNode = NULL;
        if (bInsertBefore) {
          iLimit = rgIdxArray2[0];
          pBeforeNode = rgNodeArray2[0];
          pParentNode = pBeforeNode->GetNodeItem(XFA_NODEITEM_Parent);
        } else {
          iLimit = rgIdxArray2[rgIdxArray2.GetSize() - 1];
          CXFA_Node* pLastNode = rgNodeArray2[rgIdxArray2.GetSize() - 1];
          pParentNode = pLastNode->GetNodeItem(XFA_NODEITEM_Parent);
          pBeforeNode = pLastNode->GetNodeItem(XFA_NODEITEM_NextSibling);
        }
        for (int32_t iIdx = 0, iCount = rgIdxArray1.GetSize(); iIdx < iCount;
             iIdx++) {
          CXFA_Node* pCurNode = rgNodeArray1[iIdx];
          pParentNode->RemoveChild(pCurNode);
          pParentNode->InsertChild(pCurNode, pBeforeNode);
        }
      }
      delete pDualNodeArray;
    }
    pNodeListChildMap->RemoveAll();
  }
  rgNodeListMap.RemoveAll();
}
static void XFA_ScriptInstanceManager_InsertItem(
    CXFA_Node* pInstMgrNode,
    CXFA_Node* pNewInstance,
    int32_t iPos,
    int32_t iCount = -1,
    FX_BOOL bMoveDataBindingNodes = TRUE) {
  if (iCount < 0) {
    iCount = XFA_ScriptInstanceManager_GetCount(pInstMgrNode);
  }
  if (iPos < 0) {
    iPos = iCount;
  }
  if (iPos == iCount) {
    CXFA_Node* pNextSibling =
        iCount > 0
            ? XFA_ScriptInstanceManager_GetItem(pInstMgrNode, iCount - 1)
                  ->GetNodeItem(XFA_NODEITEM_NextSibling)
            : pInstMgrNode->GetNodeItem(XFA_NODEITEM_NextSibling);
    pInstMgrNode->GetNodeItem(XFA_NODEITEM_Parent)
        ->InsertChild(pNewInstance, pNextSibling);
    if (bMoveDataBindingNodes) {
      CXFA_NodeSet sNew, sAfter;
      CXFA_NodeIteratorTemplate<CXFA_Node,
                                CXFA_TraverseStrategy_XFAContainerNode>
          sIteratorNew(pNewInstance);
      for (CXFA_Node* pNode = sIteratorNew.GetCurrent(); pNode;
           pNode = sIteratorNew.MoveToNext()) {
        CXFA_Node* pDataNode = pNode->GetBindData();
        if (!pDataNode) {
          continue;
        }
        sNew.Add(pDataNode);
      }
      CXFA_NodeIteratorTemplate<CXFA_Node,
                                CXFA_TraverseStrategy_XFAContainerNode>
          sIteratorAfter(pNextSibling);
      for (CXFA_Node* pNode = sIteratorAfter.GetCurrent(); pNode;
           pNode = sIteratorAfter.MoveToNext()) {
        CXFA_Node* pDataNode = pNode->GetBindData();
        if (!pDataNode) {
          continue;
        }
        sAfter.Add(pDataNode);
      }
      XFA_ScriptInstanceManager_ReorderDataNodes(sNew, sAfter, FALSE);
    }
  } else {
    CXFA_Node* pBeforeInstance =
        XFA_ScriptInstanceManager_GetItem(pInstMgrNode, iPos);
    pInstMgrNode->GetNodeItem(XFA_NODEITEM_Parent)
        ->InsertChild(pNewInstance, pBeforeInstance);
    if (bMoveDataBindingNodes) {
      CXFA_NodeSet sNew, sBefore;
      CXFA_NodeIteratorTemplate<CXFA_Node,
                                CXFA_TraverseStrategy_XFAContainerNode>
          sIteratorNew(pNewInstance);
      for (CXFA_Node* pNode = sIteratorNew.GetCurrent(); pNode;
           pNode = sIteratorNew.MoveToNext()) {
        CXFA_Node* pDataNode = pNode->GetBindData();
        if (!pDataNode) {
          continue;
        }
        sNew.Add(pDataNode);
      }
      CXFA_NodeIteratorTemplate<CXFA_Node,
                                CXFA_TraverseStrategy_XFAContainerNode>
          sIteratorBefore(pBeforeInstance);
      for (CXFA_Node* pNode = sIteratorBefore.GetCurrent(); pNode;
           pNode = sIteratorBefore.MoveToNext()) {
        CXFA_Node* pDataNode = pNode->GetBindData();
        if (!pDataNode) {
          continue;
        }
        sBefore.Add(pDataNode);
      }
      XFA_ScriptInstanceManager_ReorderDataNodes(sNew, sBefore, TRUE);
    }
  }
}
static void XFA_ScriptInstanceManager_RemoveItem(
    CXFA_Node* pInstMgrNode,
    CXFA_Node* pRemoveInstance,
    FX_BOOL bRemoveDataBinding = TRUE) {
  pInstMgrNode->GetNodeItem(XFA_NODEITEM_Parent)->RemoveChild(pRemoveInstance);
  if (!bRemoveDataBinding) {
    return;
  }
  CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_XFAContainerNode>
      sIterator(pRemoveInstance);
  for (CXFA_Node* pFormNode = sIterator.GetCurrent(); pFormNode;
       pFormNode = sIterator.MoveToNext()) {
    CXFA_Node* pDataNode = pFormNode->GetBindData();
    if (!pDataNode) {
      continue;
    }
    if (pDataNode->RemoveBindItem(pFormNode) == 0) {
      if (CXFA_Node* pDataParent =
              pDataNode->GetNodeItem(XFA_NODEITEM_Parent)) {
        pDataParent->RemoveChild(pDataNode);
      }
    }
    pFormNode->SetObject(XFA_ATTRIBUTE_BindingNode, NULL);
  }
}
static CXFA_Node* XFA_ScriptInstanceManager_CreateInstance(
    CXFA_Node* pInstMgrNode,
    FX_BOOL bDataMerge) {
  CXFA_Document* pDocument = pInstMgrNode->GetDocument();
  CXFA_Node* pTemplateNode = pInstMgrNode->GetTemplateNode();
  CXFA_Node* pFormParent = pInstMgrNode->GetNodeItem(XFA_NODEITEM_Parent);
  CXFA_Node* pDataScope = NULL;
  for (CXFA_Node* pRootBoundNode = pFormParent;
       pRootBoundNode &&
       pRootBoundNode->GetObjectType() == XFA_OBJECTTYPE_ContainerNode;
       pRootBoundNode = pRootBoundNode->GetNodeItem(XFA_NODEITEM_Parent)) {
    pDataScope = pRootBoundNode->GetBindData();
    if (pDataScope) {
      break;
    }
  }
  if (!pDataScope) {
    pDataScope = (CXFA_Node*)pDocument->GetXFANode(XFA_HASHCODE_Record);
    ASSERT(pDataScope);
  }
  CXFA_Node* pInstance = pDocument->DataMerge_CopyContainer(
      pTemplateNode, pFormParent, pDataScope, TRUE, bDataMerge);
  if (pInstance) {
    pDocument->DataMerge_UpdateBindingRelations(pInstance);
    pFormParent->RemoveChild(pInstance);
  }
  return pInstance;
}
void CXFA_Node::Script_InstanceManager_Count(FXJSE_HVALUE hValue,
                                             FX_BOOL bSetting,
                                             XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    int32_t iDesired = FXJSE_Value_ToInteger(hValue);
    InstanceManager_SetInstances(iDesired);
  } else {
    FXJSE_Value_SetInteger(hValue, XFA_ScriptInstanceManager_GetCount(this));
  }
}
void CXFA_Node::Script_InstanceManager_MoveInstance(
    CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc != 2) {
    FXJSE_Value_SetUndefined(pArguments->GetReturnValue());
    return;
  }
  int32_t iFrom = pArguments->GetInt32(0);
  int32_t iTo = pArguments->GetInt32(1);
  InstanceManager_MoveInstance(iTo, iFrom);
  IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
  if (!pNotify) {
    return;
  }
  CXFA_Node* pToInstance = XFA_ScriptInstanceManager_GetItem(this, iTo);
  if (pToInstance && pToInstance->GetClassID() == XFA_ELEMENT_Subform) {
    pNotify->RunSubformIndexChange(pToInstance);
  }
  CXFA_Node* pFromInstance = XFA_ScriptInstanceManager_GetItem(this, iFrom);
  if (pFromInstance && pFromInstance->GetClassID() == XFA_ELEMENT_Subform) {
    pNotify->RunSubformIndexChange(pFromInstance);
  }
}
void CXFA_Node::Script_InstanceManager_RemoveInstance(
    CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc != 1) {
    FXJSE_Value_SetUndefined(pArguments->GetReturnValue());
    return;
  }
  int32_t iIndex = pArguments->GetInt32(0);
  int32_t iCount = XFA_ScriptInstanceManager_GetCount(this);
  if (iIndex < 0 || iIndex >= iCount) {
    ThrowScriptErrorMessage(XFA_IDS_INDEX_OUT_OF_BOUNDS);
    return;
  }
  CXFA_Occur nodeOccur = GetOccurNode();
  int32_t iMin = nodeOccur.GetMin();
  if (iCount - 1 < iMin) {
    ThrowScriptErrorMessage(XFA_IDS_VIOLATE_BOUNDARY, L"min");
    return;
  }
  CXFA_Node* pRemoveInstance = XFA_ScriptInstanceManager_GetItem(this, iIndex);
  XFA_ScriptInstanceManager_RemoveItem(this, pRemoveInstance);
  IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
  if (pNotify) {
    for (int32_t i = iIndex; i < iCount - 1; i++) {
      CXFA_Node* pSubformInstance = XFA_ScriptInstanceManager_GetItem(this, i);
      if (pSubformInstance &&
          pSubformInstance->GetClassID() == XFA_ELEMENT_Subform) {
        pNotify->RunSubformIndexChange(pSubformInstance);
      }
    }
  }
  CXFA_LayoutProcessor* pLayoutPro = m_pDocument->GetLayoutProcessor();
  if (!pLayoutPro) {
    return;
  }
  pLayoutPro->AddChangedContainer(
      (CXFA_Node*)m_pDocument->GetXFANode(XFA_HASHCODE_Form));
}
void CXFA_Node::Script_InstanceManager_SetInstances(
    CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc != 1) {
    FXJSE_Value_SetUndefined(pArguments->GetReturnValue());
    return;
  }
  int32_t iDesired = pArguments->GetInt32(0);
  InstanceManager_SetInstances(iDesired);
}
void CXFA_Node::Script_InstanceManager_AddInstance(
    CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if ((argc != 0) && (argc != 1)) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"addInstance");
    return;
  }
  FX_BOOL fFlags = TRUE;
  if (argc == 1) {
    fFlags = pArguments->GetInt32(0) == 0 ? FALSE : TRUE;
  }
  int32_t iCount = XFA_ScriptInstanceManager_GetCount(this);
  CXFA_Occur nodeOccur = GetOccurNode();
  int32_t iMax = nodeOccur.GetMax();
  if (iMax >= 0 && iCount >= iMax) {
    ThrowScriptErrorMessage(XFA_IDS_VIOLATE_BOUNDARY, L"max");
    return;
  }
  CXFA_Node* pNewInstance =
      XFA_ScriptInstanceManager_CreateInstance(this, fFlags);
  XFA_ScriptInstanceManager_InsertItem(this, pNewInstance, iCount, iCount,
                                       FALSE);
  FXJSE_Value_Set(
      pArguments->GetReturnValue(),
      m_pDocument->GetScriptContext()->GetJSValueFromMap(pNewInstance));
  IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
  if (!pNotify) {
    return;
  }
  pNotify->RunNodeInitialize(pNewInstance);
  CXFA_LayoutProcessor* pLayoutPro = m_pDocument->GetLayoutProcessor();
  if (!pLayoutPro) {
    return;
  }
  pLayoutPro->AddChangedContainer(
      (CXFA_Node*)m_pDocument->GetXFANode(XFA_HASHCODE_Form));
}
void CXFA_Node::Script_InstanceManager_InsertInstance(
    CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if ((argc != 1) && (argc != 2)) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"insertInstance");
    return;
  }
  int32_t iIndex = pArguments->GetInt32(0);
  FX_BOOL bBind = FALSE;
  if (argc == 2) {
    bBind = pArguments->GetInt32(1) == 0 ? FALSE : TRUE;
  }
  CXFA_Occur nodeOccur = GetOccurNode();
  int32_t iCount = XFA_ScriptInstanceManager_GetCount(this);
  if (iIndex < 0 || iIndex > iCount) {
    ThrowScriptErrorMessage(XFA_IDS_INDEX_OUT_OF_BOUNDS);
    return;
  }
  int32_t iMax = nodeOccur.GetMax();
  if (iMax >= 0 && iCount >= iMax) {
    ThrowScriptErrorMessage(XFA_IDS_VIOLATE_BOUNDARY, L"max");
    return;
  }
  CXFA_Node* pNewInstance =
      XFA_ScriptInstanceManager_CreateInstance(this, bBind);
  XFA_ScriptInstanceManager_InsertItem(this, pNewInstance, iIndex, iCount,
                                       TRUE);
  FXJSE_Value_Set(
      pArguments->GetReturnValue(),
      m_pDocument->GetScriptContext()->GetJSValueFromMap(pNewInstance));
  IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
  if (!pNotify) {
    return;
  }
  pNotify->RunNodeInitialize(pNewInstance);
  CXFA_LayoutProcessor* pLayoutPro = m_pDocument->GetLayoutProcessor();
  if (!pLayoutPro) {
    return;
  }
  pLayoutPro->AddChangedContainer(
      (CXFA_Node*)m_pDocument->GetXFANode(XFA_HASHCODE_Form));
}
int32_t CXFA_Node::InstanceManager_SetInstances(int32_t iDesired) {
  CXFA_Occur nodeOccur = GetOccurNode();
  int32_t iMax = nodeOccur.GetMax();
  int32_t iMin = nodeOccur.GetMin();
  if (iDesired < iMin) {
    ThrowScriptErrorMessage(XFA_IDS_VIOLATE_BOUNDARY, L"min");
    return 1;
  }
  if ((iMax >= 0) && (iDesired > iMax)) {
    ThrowScriptErrorMessage(XFA_IDS_VIOLATE_BOUNDARY, L"max");
    return 2;
  }
  int32_t iCount = XFA_ScriptInstanceManager_GetCount(this);
  if (iDesired == iCount) {
    return 0;
  }
  if (iDesired < iCount) {
    CFX_WideStringC wsInstManagerName = this->GetCData(XFA_ATTRIBUTE_Name);
    CFX_WideString wsInstanceName = wsInstManagerName.IsEmpty()
                                        ? wsInstManagerName
                                        : wsInstManagerName.Mid(1);
    FX_DWORD dInstanceNameHash =
        wsInstanceName.IsEmpty() ? 0 : FX_HashCode_String_GetW(
                                           wsInstanceName,
                                           wsInstanceName.GetLength());
    CXFA_Node* pPrevSibling =
        (iDesired == 0) ? this
                        : XFA_ScriptInstanceManager_GetItem(this, iDesired - 1);
    while (iCount > iDesired) {
      CXFA_Node* pRemoveInstance =
          pPrevSibling->GetNodeItem(XFA_NODEITEM_NextSibling);
      if (pRemoveInstance->GetClassID() != XFA_ELEMENT_Subform &&
          pRemoveInstance->GetClassID() != XFA_ELEMENT_SubformSet) {
        continue;
      }
      if (pRemoveInstance->GetClassID() == XFA_ELEMENT_InstanceManager) {
        FXSYS_assert(FALSE);
        break;
      }
      if (pRemoveInstance->GetNameHash() == dInstanceNameHash) {
        XFA_ScriptInstanceManager_RemoveItem(this, pRemoveInstance);
        iCount--;
      }
    }
  } else if (iDesired > iCount) {
    while (iCount < iDesired) {
      CXFA_Node* pNewInstance =
          XFA_ScriptInstanceManager_CreateInstance(this, TRUE);
      XFA_ScriptInstanceManager_InsertItem(this, pNewInstance, iCount, iCount,
                                           FALSE);
      iCount++;
      IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
      if (!pNotify) {
        return 0;
      }
      pNotify->RunNodeInitialize(pNewInstance);
    }
  }
  CXFA_LayoutProcessor* pLayoutPro = m_pDocument->GetLayoutProcessor();
  if (!pLayoutPro) {
    return 0;
  }
  pLayoutPro->AddChangedContainer(
      (CXFA_Node*)m_pDocument->GetXFANode(XFA_HASHCODE_Form));
  return 0;
}
int32_t CXFA_Node::InstanceManager_MoveInstance(int32_t iTo, int32_t iFrom) {
  int32_t iCount = XFA_ScriptInstanceManager_GetCount(this);
  if (iFrom > iCount || iTo > iCount - 1) {
    ThrowScriptErrorMessage(XFA_IDS_INDEX_OUT_OF_BOUNDS);
    return 1;
  }
  if (iFrom < 0 || iTo < 0 || iFrom == iTo) {
    return 0;
  }
  CXFA_Node* pMoveInstance = XFA_ScriptInstanceManager_GetItem(this, iFrom);
  XFA_ScriptInstanceManager_RemoveItem(this, pMoveInstance, FALSE);
  XFA_ScriptInstanceManager_InsertItem(this, pMoveInstance, iTo, iCount - 1,
                                       TRUE);
  CXFA_LayoutProcessor* pLayoutPro = m_pDocument->GetLayoutProcessor();
  if (!pLayoutPro) {
    return 0;
  }
  pLayoutPro->AddChangedContainer(
      (CXFA_Node*)m_pDocument->GetXFANode(XFA_HASHCODE_Form));
  return 0;
}
void CXFA_Node::Script_Occur_Max(FXJSE_HVALUE hValue,
                                 FX_BOOL bSetting,
                                 XFA_ATTRIBUTE eAttribute) {
  CXFA_Occur occur(this);
  if (bSetting) {
    int32_t iMax = FXJSE_Value_ToInteger(hValue);
    occur.SetMax(iMax);
  } else {
    FXJSE_Value_SetInteger(hValue, occur.GetMax());
  }
}
void CXFA_Node::Script_Occur_Min(FXJSE_HVALUE hValue,
                                 FX_BOOL bSetting,
                                 XFA_ATTRIBUTE eAttribute) {
  CXFA_Occur occur(this);
  if (bSetting) {
    int32_t iMin = FXJSE_Value_ToInteger(hValue);
    occur.SetMin(iMin);
  } else {
    FXJSE_Value_SetInteger(hValue, occur.GetMin());
  }
}
void CXFA_Node::Script_Desc_Metadata(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if ((argc == 0) || (argc == 1)) {
    FXJSE_Value_SetUTF8String(pArguments->GetReturnValue(), "");
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"metadata");
  }
}
void CXFA_Node::Script_Form_FormNodes(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 1) {
    CXFA_Node* pDataNode = (CXFA_Node*)pArguments->GetObject(0);
    if (pDataNode) {
      CXFA_NodeArray formItems;
      CXFA_ArrayNodeList* pFormNodes = new CXFA_ArrayNodeList(m_pDocument);
      pFormNodes->SetArrayNodeList(formItems);
      FXJSE_Value_SetObject(
          pArguments->GetReturnValue(), (CXFA_Object*)pFormNodes,
          m_pDocument->GetScriptContext()->GetJseNormalClass());
    } else {
      ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
    }
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"formNodes");
  }
}
void CXFA_Node::Script_Form_Remerge(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
    m_pDocument->DoDataRemerge(TRUE);
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"remerge");
  }
}
void CXFA_Node::Script_Form_ExecInitialize(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
    IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
    if (!pNotify) {
      return;
    }
    pNotify->ExecEventByDeepFirst(this, XFA_EVENT_Initialize);
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"execInitialize");
  }
}
void CXFA_Node::Script_Form_Recalculate(CFXJSE_Arguments* pArguments) {
  CXFA_EventParam* pEventParam =
      m_pDocument->GetScriptContext()->GetEventParam();
  if (pEventParam->m_eType == XFA_EVENT_Calculate ||
      pEventParam->m_eType == XFA_EVENT_InitCalculate) {
    return;
  }
  int32_t argc = pArguments->GetLength();
  if (argc == 1) {
    FX_BOOL bScriptFlags = pArguments->GetInt32(0) == 0 ? FALSE : TRUE;
    IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
    if (!pNotify) {
      return;
    }
    if (bScriptFlags) {
      pNotify->ExecEventByDeepFirst(this, XFA_EVENT_Calculate);
      pNotify->ExecEventByDeepFirst(this, XFA_EVENT_Validate);
      pNotify->ExecEventByDeepFirst(this, XFA_EVENT_Ready, TRUE);
    } else {
    }
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"recalculate");
  }
}
void CXFA_Node::Script_Form_ExecCalculate(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
    IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
    if (!pNotify) {
      return;
    }
    pNotify->ExecEventByDeepFirst(this, XFA_EVENT_Calculate);
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"execCalculate");
  }
}
void CXFA_Node::Script_Form_ExecValidate(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
    IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
    if (!pNotify) {
      FXJSE_Value_SetBoolean(pArguments->GetReturnValue(), FALSE);
    } else {
      int32_t iRet = pNotify->ExecEventByDeepFirst(this, XFA_EVENT_Validate);
      FXJSE_Value_SetBoolean(pArguments->GetReturnValue(),
                             ((iRet == XFA_EVENTERROR_Error) ? FALSE : TRUE));
    }
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"execValidate");
  }
}
void CXFA_Node::Script_Form_Checksum(FXJSE_HVALUE hValue,
                                     FX_BOOL bSetting,
                                     XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    CFX_ByteString bsChecksum;
    FXJSE_Value_ToUTF8String(hValue, bsChecksum);
    SetAttribute(XFA_ATTRIBUTE_Checksum,
                 CFX_WideString::FromUTF8(bsChecksum, bsChecksum.GetLength()));
  } else {
    CFX_WideString wsChecksum;
    GetAttribute(XFA_ATTRIBUTE_Checksum, wsChecksum, FALSE);
    FXJSE_Value_SetUTF8String(
        hValue, FX_UTF8Encode(wsChecksum, wsChecksum.GetLength()));
  }
}
void CXFA_Node::Script_Packet_GetAttribute(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 1) {
    CFX_ByteString bsAttributeName = pArguments->GetUTF8String(0);
    CFX_WideString wsAttributeValue;
    IFDE_XMLNode* pXMLNode = GetXMLMappingNode();
    if (pXMLNode && pXMLNode->GetType() == FDE_XMLNODE_Element) {
      ((IFDE_XMLElement*)pXMLNode)
          ->GetString(CFX_WideString::FromUTF8(bsAttributeName,
                                               bsAttributeName.GetLength()),
                      wsAttributeValue);
    }
    FXJSE_Value_SetUTF8String(
        pArguments->GetReturnValue(),
        FX_UTF8Encode(wsAttributeValue, wsAttributeValue.GetLength()));
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"getAttribute");
  }
}
void CXFA_Node::Script_Packet_SetAttribute(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 2) {
    CFX_ByteString bsValue = pArguments->GetUTF8String(0);
    CFX_ByteString bsName = pArguments->GetUTF8String(1);
    IFDE_XMLNode* pXMLNode = GetXMLMappingNode();
    if (pXMLNode && pXMLNode->GetType() == FDE_XMLNODE_Element) {
      ((IFDE_XMLElement*)pXMLNode)
          ->SetString(CFX_WideString::FromUTF8(bsName, bsName.GetLength()),
                      CFX_WideString::FromUTF8(bsValue, bsValue.GetLength()));
    }
    FXJSE_Value_SetNull(pArguments->GetReturnValue());
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"setAttribute");
  }
}
void CXFA_Node::Script_Packet_RemoveAttribute(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 1) {
    CFX_ByteString bsName = pArguments->GetUTF8String(0);
    CFX_WideString wsName =
        CFX_WideString::FromUTF8(bsName, bsName.GetLength());
    IFDE_XMLNode* pXMLNode = GetXMLMappingNode();
    if (pXMLNode && pXMLNode->GetType() == FDE_XMLNODE_Element) {
      IFDE_XMLElement* pXMLElement = (IFDE_XMLElement*)pXMLNode;
      if (pXMLElement->HasAttribute(wsName)) {
        pXMLElement->RemoveAttribute(wsName);
      }
    }
    FXJSE_Value_SetNull(pArguments->GetReturnValue());
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"removeAttribute");
  }
}
void CXFA_Node::Script_Packet_Content(FXJSE_HVALUE hValue,
                                      FX_BOOL bSetting,
                                      XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    CFX_ByteString bsNewContent;
    FXJSE_Value_ToUTF8String(hValue, bsNewContent);
    IFDE_XMLNode* pXMLNode = GetXMLMappingNode();
    if (pXMLNode && pXMLNode->GetType() == FDE_XMLNODE_Element) {
      IFDE_XMLElement* pXMLElement = (IFDE_XMLElement*)pXMLNode;
      pXMLElement->SetTextData(
          CFX_WideString::FromUTF8(bsNewContent, bsNewContent.GetLength()));
    }
  } else {
    CFX_WideString wsTextData;
    IFDE_XMLNode* pXMLNode = GetXMLMappingNode();
    if (pXMLNode && pXMLNode->GetType() == FDE_XMLNODE_Element) {
      IFDE_XMLElement* pXMLElement = (IFDE_XMLElement*)pXMLNode;
      pXMLElement->GetTextData(wsTextData);
    }
    FXJSE_Value_SetUTF8String(
        hValue, FX_UTF8Encode(wsTextData, wsTextData.GetLength()));
  }
}
void CXFA_Node::Script_Source_Next(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"next");
  }
}
void CXFA_Node::Script_Source_CancelBatch(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"cancelBatch");
  }
}
void CXFA_Node::Script_Source_First(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"first");
  }
}
void CXFA_Node::Script_Source_UpdateBatch(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"updateBatch");
  }
}
void CXFA_Node::Script_Source_Previous(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"previous");
  }
}
void CXFA_Node::Script_Source_IsBOF(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"isBOF");
  }
}
void CXFA_Node::Script_Source_IsEOF(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"isEOF");
  }
}
void CXFA_Node::Script_Source_Cancel(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"cancel");
  }
}
void CXFA_Node::Script_Source_Update(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"update");
  }
}
void CXFA_Node::Script_Source_Open(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"open");
  }
}
void CXFA_Node::Script_Source_Delete(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"delete");
  }
}
void CXFA_Node::Script_Source_AddNew(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"addNew");
  }
}
void CXFA_Node::Script_Source_Requery(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"requery");
  }
}
void CXFA_Node::Script_Source_Resync(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"resync");
  }
}
void CXFA_Node::Script_Source_Close(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"close");
  }
}
void CXFA_Node::Script_Source_Last(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"last");
  }
}
void CXFA_Node::Script_Source_HasDataChanged(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 0) {
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                            L"hasDataChanged");
  }
}
void CXFA_Node::Script_Source_Db(FXJSE_HVALUE hValue,
                                 FX_BOOL bSetting,
                                 XFA_ATTRIBUTE eAttribute) {}
void CXFA_Node::Script_Xfa_This(FXJSE_HVALUE hValue,
                                FX_BOOL bSetting,
                                XFA_ATTRIBUTE eAttribute) {
  if (!bSetting) {
    CXFA_Object* pThis = m_pDocument->GetScriptContext()->GetThisObject();
    FXSYS_assert(pThis);
    FXJSE_Value_Set(hValue,
                    m_pDocument->GetScriptContext()->GetJSValueFromMap(pThis));
  }
}
void CXFA_Node::Script_Handler_Version(FXJSE_HVALUE hValue,
                                       FX_BOOL bSetting,
                                       XFA_ATTRIBUTE eAttribute) {}
void CXFA_Node::Script_SubmitFormat_Mode(FXJSE_HVALUE hValue,
                                         FX_BOOL bSetting,
                                         XFA_ATTRIBUTE eAttribute) {}
void CXFA_Node::Script_Extras_Type(FXJSE_HVALUE hValue,
                                   FX_BOOL bSetting,
                                   XFA_ATTRIBUTE eAttribute) {}
void CXFA_Node::Script_Script_Stateless(FXJSE_HVALUE hValue,
                                        FX_BOOL bSetting,
                                        XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
    return;
  }
  FXJSE_Value_SetUTF8String(hValue, FX_UTF8Encode(FX_WSTRC(L"0")));
}
void CXFA_Node::Script_Encrypt_Format(FXJSE_HVALUE hValue,
                                      FX_BOOL bSetting,
                                      XFA_ATTRIBUTE eAttribute) {}
enum XFA_KEYTYPE {
  XFA_KEYTYPE_Custom,
  XFA_KEYTYPE_Element,
};
void* XFA_GetMapKey_Custom(const CFX_WideStringC& wsKey) {
  FX_DWORD dwKey = FX_HashCode_String_GetW(wsKey.GetPtr(), wsKey.GetLength());
  return (void*)(uintptr_t)((dwKey << 1) | XFA_KEYTYPE_Custom);
}
void* XFA_GetMapKey_Element(XFA_ELEMENT eElement, XFA_ATTRIBUTE eAttribute) {
  return (void*)(uintptr_t)((eElement << 16) | (eAttribute << 8) |
                            XFA_KEYTYPE_Element);
}
static inline FX_BOOL XFA_NodeData_PrepareKey(XFA_ELEMENT eElem,
                                              XFA_ATTRIBUTE eAttr,
                                              void*& pKey) {
  pKey = XFA_GetMapKey_Element(eElem, eAttr);
  return TRUE;
}
FX_BOOL CXFA_Node::HasAttribute(XFA_ATTRIBUTE eAttr, FX_BOOL bCanInherit) {
  void* pKey = NULL;
  if (!XFA_NodeData_PrepareKey(GetClassID(), eAttr, pKey)) {
    return FALSE;
  }
  return HasMapModuleKey(pKey, bCanInherit);
}
FX_BOOL CXFA_Node::SetAttribute(XFA_ATTRIBUTE eAttr,
                                const CFX_WideStringC& wsValue,
                                FX_BOOL bNotify) {
  XFA_LPCATTRIBUTEINFO pAttr = XFA_GetAttributeByID(eAttr);
  if (pAttr == NULL) {
    return FALSE;
  }
  XFA_ATTRIBUTETYPE eType = pAttr->eType;
  if (eType == XFA_ATTRIBUTETYPE_NOTSURE) {
    XFA_LPCNOTSUREATTRIBUTE pNotsure =
        XFA_GetNotsureAttribute(GetClassID(), pAttr->eName);
    eType = pNotsure ? pNotsure->eType : XFA_ATTRIBUTETYPE_Cdata;
  }
  switch (eType) {
    case XFA_ATTRIBUTETYPE_Enum: {
      XFA_LPCATTRIBUTEENUMINFO pEnum = XFA_GetAttributeEnumByName(wsValue);
      return SetEnum(pAttr->eName,
                     pEnum ? pEnum->eName
                           : (XFA_ATTRIBUTEENUM)(intptr_t)(pAttr->pDefValue),
                     bNotify);
    } break;
    case XFA_ATTRIBUTETYPE_Cdata:
      return SetCData(pAttr->eName, wsValue, bNotify);
    case XFA_ATTRIBUTETYPE_Boolean:
      return SetBoolean(pAttr->eName, wsValue != FX_WSTRC(L"0"), bNotify);
    case XFA_ATTRIBUTETYPE_Integer:
      return SetInteger(
          pAttr->eName,
          FXSYS_round(FX_wcstof(wsValue.GetPtr(), wsValue.GetLength())),
          bNotify);
    case XFA_ATTRIBUTETYPE_Measure:
      return SetMeasure(pAttr->eName, CXFA_Measurement(wsValue), bNotify);
    default:
      break;
  }
  return FALSE;
}
FX_BOOL CXFA_Node::GetAttribute(XFA_ATTRIBUTE eAttr,
                                CFX_WideString& wsValue,
                                FX_BOOL bUseDefault) {
  XFA_LPCATTRIBUTEINFO pAttr = XFA_GetAttributeByID(eAttr);
  if (pAttr == NULL) {
    return FALSE;
  }
  XFA_ATTRIBUTETYPE eType = pAttr->eType;
  if (eType == XFA_ATTRIBUTETYPE_NOTSURE) {
    XFA_LPCNOTSUREATTRIBUTE pNotsure =
        XFA_GetNotsureAttribute(GetClassID(), pAttr->eName);
    eType = pNotsure ? pNotsure->eType : XFA_ATTRIBUTETYPE_Cdata;
  }
  switch (eType) {
    case XFA_ATTRIBUTETYPE_Enum: {
      XFA_ATTRIBUTEENUM eValue;
      if (!TryEnum(pAttr->eName, eValue, bUseDefault)) {
        return FALSE;
      }
      wsValue = XFA_GetAttributeEnumByID(eValue)->pName;
      return TRUE;
    } break;
    case XFA_ATTRIBUTETYPE_Cdata: {
      CFX_WideStringC wsValueC;
      if (!TryCData(pAttr->eName, wsValueC, bUseDefault)) {
        return FALSE;
      }
      wsValue = wsValueC;
      return TRUE;
    } break;
    case XFA_ATTRIBUTETYPE_Boolean: {
      FX_BOOL bValue;
      if (!TryBoolean(pAttr->eName, bValue, bUseDefault)) {
        return FALSE;
      }
      wsValue = bValue ? FX_WSTRC(L"1") : FX_WSTRC(L"0");
      return TRUE;
    } break;
    case XFA_ATTRIBUTETYPE_Integer: {
      int32_t iValue;
      if (!TryInteger(pAttr->eName, iValue, bUseDefault)) {
        return FALSE;
      }
      wsValue.Format(L"%d", iValue);
      return TRUE;
    } break;
    case XFA_ATTRIBUTETYPE_Measure: {
      CXFA_Measurement mValue;
      if (!TryMeasure(pAttr->eName, mValue, bUseDefault)) {
        return FALSE;
      }
      mValue.ToString(wsValue);
      return TRUE;
    } break;
    default:
      break;
  }
  return FALSE;
}
FX_BOOL CXFA_Node::SetAttribute(const CFX_WideStringC& wsAttr,
                                const CFX_WideStringC& wsValue,
                                FX_BOOL bNotify) {
  XFA_LPCATTRIBUTEINFO pAttributeInfo = XFA_GetAttributeByName(wsValue);
  if (pAttributeInfo) {
    return SetAttribute(pAttributeInfo->eName, wsValue, bNotify);
  }
  void* pKey = XFA_GetMapKey_Custom(wsAttr);
  SetMapModuleString(pKey, wsValue);
  return TRUE;
}
FX_BOOL CXFA_Node::GetAttribute(const CFX_WideStringC& wsAttr,
                                CFX_WideString& wsValue,
                                FX_BOOL bUseDefault) {
  XFA_LPCATTRIBUTEINFO pAttributeInfo = XFA_GetAttributeByName(wsAttr);
  if (pAttributeInfo) {
    return GetAttribute(pAttributeInfo->eName, wsValue, bUseDefault);
  }
  void* pKey = XFA_GetMapKey_Custom(wsAttr);
  CFX_WideStringC wsValueC;
  if (GetMapModuleString(pKey, wsValueC)) {
    wsValue = wsValueC;
  }
  return TRUE;
}
FX_BOOL CXFA_Node::RemoveAttribute(const CFX_WideStringC& wsAttr) {
  void* pKey = XFA_GetMapKey_Custom(wsAttr);
  RemoveMapModuleKey(pKey);
  return TRUE;
}
FX_BOOL CXFA_Node::TryBoolean(XFA_ATTRIBUTE eAttr,
                              FX_BOOL& bValue,
                              FX_BOOL bUseDefault) {
  void* pValue = NULL;
  if (!GetValue(eAttr, XFA_ATTRIBUTETYPE_Boolean, bUseDefault, pValue)) {
    return FALSE;
  }
  bValue = (FX_BOOL)(uintptr_t)pValue;
  return TRUE;
}
FX_BOOL CXFA_Node::TryInteger(XFA_ATTRIBUTE eAttr,
                              int32_t& iValue,
                              FX_BOOL bUseDefault) {
  void* pValue = NULL;
  if (!GetValue(eAttr, XFA_ATTRIBUTETYPE_Integer, bUseDefault, pValue)) {
    return FALSE;
  }
  iValue = (int32_t)(uintptr_t)pValue;
  return TRUE;
}
FX_BOOL CXFA_Node::TryEnum(XFA_ATTRIBUTE eAttr,
                           XFA_ATTRIBUTEENUM& eValue,
                           FX_BOOL bUseDefault) {
  void* pValue = NULL;
  if (!GetValue(eAttr, XFA_ATTRIBUTETYPE_Enum, bUseDefault, pValue)) {
    return FALSE;
  }
  eValue = (XFA_ATTRIBUTEENUM)(uintptr_t)pValue;
  return TRUE;
}
FX_BOOL CXFA_Node::SetMeasure(XFA_ATTRIBUTE eAttr,
                              CXFA_Measurement mValue,
                              FX_BOOL bNotify) {
  void* pKey = NULL;
  if (!XFA_NodeData_PrepareKey(GetClassID(), eAttr, pKey)) {
    return FALSE;
  }
  OnChanging(eAttr, &mValue, bNotify);
  SetMapModuleBuffer(pKey, &mValue, sizeof(CXFA_Measurement));
  OnChanged(eAttr, &mValue, bNotify);
  return TRUE;
}
FX_BOOL CXFA_Node::TryMeasure(XFA_ATTRIBUTE eAttr,
                              CXFA_Measurement& mValue,
                              FX_BOOL bUseDefault) {
  void* pKey = NULL;
  if (!XFA_NodeData_PrepareKey(GetClassID(), eAttr, pKey)) {
    return FALSE;
  }
  void* pValue;
  int32_t iBytes;
  if (GetMapModuleBuffer(pKey, pValue, iBytes) && iBytes == sizeof(mValue)) {
    FX_memcpy(&mValue, pValue, sizeof(mValue));
    return TRUE;
  }
  if (bUseDefault &&
      XFA_GetAttributeDefaultValue(pValue, GetClassID(), eAttr,
                                   XFA_ATTRIBUTETYPE_Measure, m_ePacket)) {
    FX_memcpy(&mValue, pValue, sizeof(mValue));
    return TRUE;
  }
  return FALSE;
}
FX_BOOL CXFA_Node::SetCData(XFA_ATTRIBUTE eAttr,
                            const CFX_WideString& wsValue,
                            FX_BOOL bNotify,
                            FX_BOOL bScriptModify) {
  void* pKey = NULL;
  if (!XFA_NodeData_PrepareKey(GetClassID(), eAttr, pKey)) {
    return FALSE;
  }
  OnChanging(eAttr, (void*)(const FX_WCHAR*)wsValue, bNotify);
  if (eAttr == XFA_ATTRIBUTE_Value) {
    CFX_WideString* pClone = new CFX_WideString(wsValue);
    SetUserData(pKey, pClone, &deleteWideStringCallBack);
  } else {
    SetMapModuleString(pKey, wsValue);
    if (eAttr == XFA_ATTRIBUTE_Name) {
      UpdateNameHash();
      if (XFA_LPCJSBUILTININFO pBuiltin =
              XFA_GetJSBuiltinByHash(m_dwNameHash)) {
        m_pDocument->GetScriptContext()->AddJSBuiltinObject(pBuiltin);
      }
    }
  }
  OnChanged(eAttr, (void*)(const FX_WCHAR*)wsValue, bNotify, bScriptModify);
  if (IsNeedSavingXMLNode() && eAttr != XFA_ATTRIBUTE_QualifiedName &&
      eAttr != XFA_ATTRIBUTE_BindingNode) {
    if (eAttr == XFA_ATTRIBUTE_Name &&
        (m_eNodeClass == XFA_ELEMENT_DataValue ||
         m_eNodeClass == XFA_ELEMENT_DataGroup)) {
      return TRUE;
    }
    if (eAttr == XFA_ATTRIBUTE_Value) {
      FDE_XMLNODETYPE eXMLType = m_pXMLNode->GetType();
      switch (eXMLType) {
        case FDE_XMLNODE_Element:
          if (IsAttributeInXML()) {
            ((IFDE_XMLElement*)m_pXMLNode)
                ->SetString(GetCData(XFA_ATTRIBUTE_QualifiedName), wsValue);
          } else {
            FX_BOOL bDeleteChildren = TRUE;
            if (GetPacketID() == XFA_XDPPACKET_Datasets) {
              for (CXFA_Node* pChildDataNode =
                       this->GetNodeItem(XFA_NODEITEM_FirstChild);
                   pChildDataNode; pChildDataNode = pChildDataNode->GetNodeItem(
                                       XFA_NODEITEM_NextSibling)) {
                CXFA_NodeArray formNodes;
                if (pChildDataNode->GetBindItems(formNodes) > 0) {
                  bDeleteChildren = FALSE;
                  break;
                }
              }
            }
            if (bDeleteChildren) {
              ((IFDE_XMLElement*)m_pXMLNode)->DeleteChildren();
            }
            ((IFDE_XMLElement*)m_pXMLNode)->SetTextData(wsValue);
          }
          break;
        case FDE_XMLNODE_Text:
          ((IFDE_XMLText*)m_pXMLNode)->SetText(wsValue);
          break;
        default:
          FXSYS_assert(0);
      }
      return TRUE;
    }
    XFA_LPCATTRIBUTEINFO pInfo = XFA_GetAttributeByID(eAttr);
    if (pInfo) {
      FXSYS_assert(m_pXMLNode->GetType() == FDE_XMLNODE_Element);
      CFX_WideString wsAttrName = pInfo->pName;
      if (pInfo->eName == XFA_ATTRIBUTE_ContentType) {
        wsAttrName = FX_WSTRC(L"xfa:") + wsAttrName;
      }
      ((IFDE_XMLElement*)m_pXMLNode)->SetString(wsAttrName, wsValue);
    }
  }
  return TRUE;
}
FX_BOOL CXFA_Node::SetAttributeValue(const CFX_WideString& wsValue,
                                     const CFX_WideString& wsXMLValue,
                                     FX_BOOL bNotify,
                                     FX_BOOL bScriptModify) {
  void* pKey = NULL;
  if (!XFA_NodeData_PrepareKey(GetClassID(), XFA_ATTRIBUTE_Value, pKey)) {
    return FALSE;
  }
  OnChanging(XFA_ATTRIBUTE_Value, (void*)(const FX_WCHAR*)wsValue, bNotify);
  CFX_WideString* pClone = new CFX_WideString(wsValue);
  SetUserData(pKey, pClone, &deleteWideStringCallBack);
  OnChanged(XFA_ATTRIBUTE_Value, (void*)(const FX_WCHAR*)wsValue, bNotify,
            bScriptModify);
  if (IsNeedSavingXMLNode()) {
    FDE_XMLNODETYPE eXMLType = m_pXMLNode->GetType();
    switch (eXMLType) {
      case FDE_XMLNODE_Element:
        if (IsAttributeInXML()) {
          ((IFDE_XMLElement*)m_pXMLNode)
              ->SetString(GetCData(XFA_ATTRIBUTE_QualifiedName), wsXMLValue);
        } else {
          FX_BOOL bDeleteChildren = TRUE;
          if (GetPacketID() == XFA_XDPPACKET_Datasets) {
            for (CXFA_Node* pChildDataNode =
                     this->GetNodeItem(XFA_NODEITEM_FirstChild);
                 pChildDataNode; pChildDataNode = pChildDataNode->GetNodeItem(
                                     XFA_NODEITEM_NextSibling)) {
              CXFA_NodeArray formNodes;
              if (pChildDataNode->GetBindItems(formNodes) > 0) {
                bDeleteChildren = FALSE;
                break;
              }
            }
          }
          if (bDeleteChildren) {
            ((IFDE_XMLElement*)m_pXMLNode)->DeleteChildren();
          }
          ((IFDE_XMLElement*)m_pXMLNode)->SetTextData(wsXMLValue);
        }
        break;
      case FDE_XMLNODE_Text:
        ((IFDE_XMLText*)m_pXMLNode)->SetText(wsXMLValue);
        break;
      default:
        FXSYS_assert(0);
    }
  }
  return TRUE;
}
FX_BOOL CXFA_Node::TryCData(XFA_ATTRIBUTE eAttr,
                            CFX_WideString& wsValue,
                            FX_BOOL bUseDefault,
                            FX_BOOL bProto) {
  void* pKey = NULL;
  if (!XFA_NodeData_PrepareKey(GetClassID(), eAttr, pKey)) {
    return FALSE;
  }
  if (eAttr == XFA_ATTRIBUTE_Value) {
    CFX_WideString* pStr = (CFX_WideString*)GetUserData(pKey, bProto);
    if (pStr) {
      wsValue = *pStr;
      return TRUE;
    }
  } else {
    CFX_WideStringC wsValueC;
    if (GetMapModuleString(pKey, wsValueC)) {
      wsValue = wsValueC;
      return TRUE;
    }
  }
  if (!bUseDefault) {
    return FALSE;
  }
  void* pValue = NULL;
  if (XFA_GetAttributeDefaultValue(pValue, GetClassID(), eAttr,
                                   XFA_ATTRIBUTETYPE_Cdata, m_ePacket)) {
    wsValue = (const FX_WCHAR*)pValue;
    return TRUE;
  }
  return FALSE;
}
FX_BOOL CXFA_Node::TryCData(XFA_ATTRIBUTE eAttr,
                            CFX_WideStringC& wsValue,
                            FX_BOOL bUseDefault,
                            FX_BOOL bProto) {
  void* pKey = NULL;
  if (!XFA_NodeData_PrepareKey(GetClassID(), eAttr, pKey)) {
    return FALSE;
  }
  if (eAttr == XFA_ATTRIBUTE_Value) {
    CFX_WideString* pStr = (CFX_WideString*)GetUserData(pKey, bProto);
    if (pStr) {
      wsValue = *pStr;
      return TRUE;
    }
  } else {
    if (GetMapModuleString(pKey, wsValue)) {
      return TRUE;
    }
  }
  if (!bUseDefault) {
    return FALSE;
  }
  void* pValue = NULL;
  if (XFA_GetAttributeDefaultValue(pValue, GetClassID(), eAttr,
                                   XFA_ATTRIBUTETYPE_Cdata, m_ePacket)) {
    wsValue = (CFX_WideStringC)(const FX_WCHAR*)pValue;
    return TRUE;
  }
  return FALSE;
}
FX_BOOL CXFA_Node::SetObject(XFA_ATTRIBUTE eAttr,
                             void* pData,
                             XFA_MAPDATABLOCKCALLBACKINFO* pCallbackInfo) {
  void* pKey = NULL;
  if (!XFA_NodeData_PrepareKey(GetClassID(), eAttr, pKey)) {
    return FALSE;
  }
  return SetUserData(pKey, pData, pCallbackInfo);
}
FX_BOOL CXFA_Node::TryObject(XFA_ATTRIBUTE eAttr, void*& pData) {
  void* pKey = NULL;
  if (!XFA_NodeData_PrepareKey(GetClassID(), eAttr, pKey)) {
    return FALSE;
  }
  pData = GetUserData(pKey);
  return pData != NULL;
}
FX_BOOL CXFA_Node::SetValue(XFA_ATTRIBUTE eAttr,
                            XFA_ATTRIBUTETYPE eType,
                            void* pValue,
                            FX_BOOL bNotify) {
  void* pKey = NULL;
  if (!XFA_NodeData_PrepareKey(GetClassID(), eAttr, pKey)) {
    return FALSE;
  }
  OnChanging(eAttr, pValue, bNotify);
  SetMapModuleValue(pKey, pValue);
  OnChanged(eAttr, pValue, bNotify);
  if (IsNeedSavingXMLNode()) {
    FXSYS_assert(m_pXMLNode->GetType() == FDE_XMLNODE_Element);
    XFA_LPCATTRIBUTEINFO pInfo = XFA_GetAttributeByID(eAttr);
    if (pInfo) {
      switch (eType) {
        case XFA_ATTRIBUTETYPE_Enum:
          ((IFDE_XMLElement*)m_pXMLNode)
              ->SetString(
                  pInfo->pName,
                  XFA_GetAttributeEnumByID((XFA_ATTRIBUTEENUM)(uintptr_t)pValue)
                      ->pName);
          break;
        case XFA_ATTRIBUTETYPE_Boolean:
          ((IFDE_XMLElement*)m_pXMLNode)
              ->SetString(pInfo->pName,
                          pValue ? FX_WSTRC(L"1") : FX_WSTRC(L"0"));
          break;
        case XFA_ATTRIBUTETYPE_Integer:
          ((IFDE_XMLElement*)m_pXMLNode)
              ->SetInteger(pInfo->pName, (int32_t)(uintptr_t)pValue);
          break;
        default:
          FXSYS_assert(0);
      }
    }
  }
  return TRUE;
}
FX_BOOL CXFA_Node::GetValue(XFA_ATTRIBUTE eAttr,
                            XFA_ATTRIBUTETYPE eType,
                            FX_BOOL bUseDefault,
                            void*& pValue) {
  void* pKey = NULL;
  if (!XFA_NodeData_PrepareKey(GetClassID(), eAttr, pKey)) {
    return FALSE;
  }
  if (GetMapModuleValue(pKey, pValue)) {
    return TRUE;
  }
  if (!bUseDefault) {
    return FALSE;
  }
  return XFA_GetAttributeDefaultValue(pValue, GetClassID(), eAttr, eType,
                                      m_ePacket);
}
static void XFA_DefaultFreeData(void* pData) {}
static XFA_MAPDATABLOCKCALLBACKINFO gs_XFADefaultFreeData = {
    XFA_DefaultFreeData, NULL};
FX_BOOL CXFA_Node::SetUserData(void* pKey,
                               void* pData,
                               XFA_MAPDATABLOCKCALLBACKINFO* pCallbackInfo) {
  SetMapModuleBuffer(pKey, &pData, sizeof(void*),
                     pCallbackInfo ? pCallbackInfo : &gs_XFADefaultFreeData);
  return TRUE;
}
FX_BOOL CXFA_Node::TryUserData(void* pKey, void*& pData, FX_BOOL bProtoAlso) {
  int32_t iBytes = 0;
  if (!GetMapModuleBuffer(pKey, pData, iBytes, bProtoAlso)) {
    return FALSE;
  }
  return iBytes == sizeof(void*) && FXSYS_memcpy(&pData, pData, iBytes);
}
FX_BOOL CXFA_Node::SetScriptContent(const CFX_WideString& wsContent,
                                    const CFX_WideString& wsXMLValue,
                                    FX_BOOL bNotify,
                                    FX_BOOL bScriptModify,
                                    FX_BOOL bSyncData) {
  CXFA_Node* pNode = NULL;
  CXFA_Node* pBindNode = NULL;
  switch (GetObjectType()) {
    case XFA_OBJECTTYPE_ContainerNode: {
      if (XFA_FieldIsMultiListBox(this)) {
        CXFA_Node* pValue = GetProperty(0, XFA_ELEMENT_Value);
        CXFA_Node* pChildValue = pValue->GetNodeItem(XFA_NODEITEM_FirstChild);
        FXSYS_assert(pChildValue);
        pChildValue->SetCData(XFA_ATTRIBUTE_ContentType, FX_WSTRC(L"text/xml"));
        pChildValue->SetScriptContent(wsContent, wsContent, bNotify,
                                      bScriptModify, FALSE);
        CXFA_Node* pBind = GetBindData();
        if (bSyncData && pBind) {
          CFX_WideStringArray wsSaveTextArray;
          int32_t iSize = 0;
          if (!wsContent.IsEmpty()) {
            int32_t iStart = 0;
            int32_t iLength = wsContent.GetLength();
            int32_t iEnd = wsContent.Find(L'\n', iStart);
            iEnd = (iEnd == -1) ? iLength : iEnd;
            while (iEnd >= iStart) {
              wsSaveTextArray.Add(wsContent.Mid(iStart, iEnd - iStart));
              iStart = iEnd + 1;
              if (iStart >= iLength) {
                break;
              }
              iEnd = wsContent.Find(L'\n', iStart);
              if (iEnd < 0) {
                wsSaveTextArray.Add(wsContent.Mid(iStart, iLength - iStart));
              }
            }
            iSize = wsSaveTextArray.GetSize();
          }
          if (iSize == 0) {
            while (CXFA_Node* pChildNode =
                       pBind->GetNodeItem(XFA_NODEITEM_FirstChild)) {
              pBind->RemoveChild(pChildNode);
            }
          } else {
            CXFA_NodeArray valueNodes;
            int32_t iDatas = pBind->GetNodeList(
                valueNodes, XFA_NODEFILTER_Children, XFA_ELEMENT_DataValue);
            if (iDatas < iSize) {
              int32_t iAddNodes = iSize - iDatas;
              CXFA_Node* pValueNodes = NULL;
              while (iAddNodes-- > 0) {
                pValueNodes =
                    pBind->CreateSamePacketNode(XFA_ELEMENT_DataValue);
                pValueNodes->SetCData(XFA_ATTRIBUTE_Name, FX_WSTRC(L"value"));
                pValueNodes->CreateXMLMappingNode();
                pBind->InsertChild(pValueNodes);
              }
              pValueNodes = NULL;
            } else if (iDatas > iSize) {
              int32_t iDelNodes = iDatas - iSize;
              while (iDelNodes-- > 0) {
                pBind->RemoveChild(pBind->GetNodeItem(XFA_NODEITEM_FirstChild));
              }
            }
            int32_t i = 0;
            for (CXFA_Node* pValueNode =
                     pBind->GetNodeItem(XFA_NODEITEM_FirstChild);
                 pValueNode; pValueNode = pValueNode->GetNodeItem(
                                 XFA_NODEITEM_NextSibling)) {
              pValueNode->SetAttributeValue(wsSaveTextArray[i],
                                            wsSaveTextArray[i], FALSE);
              i++;
            }
          }
          CXFA_NodeArray nodeArray;
          pBind->GetBindItems(nodeArray);
          for (int32_t i = 0; i < nodeArray.GetSize(); i++) {
            CXFA_Node* pNode = nodeArray[i];
            if (pNode == this) {
              continue;
            }
            pNode->SetScriptContent(wsContent, wsContent, bNotify,
                                    bScriptModify, FALSE);
          }
        }
        break;
      } else if (GetClassID() == XFA_ELEMENT_ExclGroup) {
        pNode = this;
      } else {
        CXFA_Node* pValue = GetProperty(0, XFA_ELEMENT_Value);
        CXFA_Node* pChildValue = pValue->GetNodeItem(XFA_NODEITEM_FirstChild);
        FXSYS_assert(pChildValue);
        pChildValue->SetScriptContent(wsContent, wsContent, bNotify,
                                      bScriptModify, FALSE);
      }
      pBindNode = GetBindData();
      if (pBindNode && bSyncData) {
        pBindNode->SetScriptContent(wsContent, wsXMLValue, bNotify,
                                    bScriptModify, FALSE);
        CXFA_NodeArray nodeArray;
        pBindNode->GetBindItems(nodeArray);
        for (int32_t i = 0; i < nodeArray.GetSize(); i++) {
          CXFA_Node* pNode = nodeArray[i];
          if (pNode == this) {
            continue;
          }
          pNode->SetScriptContent(wsContent, wsContent, bNotify, TRUE, FALSE);
        }
      }
      pBindNode = NULL;
      break;
    }
    case XFA_OBJECTTYPE_ContentNode: {
      CFX_WideString wsContentType;
      if (GetClassID() == XFA_ELEMENT_ExData) {
        GetAttribute(XFA_ATTRIBUTE_ContentType, wsContentType, FALSE);
        if (wsContentType.Equal(FX_WSTRC(L"text/html"))) {
          wsContentType = FX_WSTRC(L"");
          SetAttribute(XFA_ATTRIBUTE_ContentType, wsContentType);
        }
      }
      CXFA_Node* pContentRawDataNode = GetNodeItem(XFA_NODEITEM_FirstChild);
      if (!pContentRawDataNode) {
        pContentRawDataNode =
            CreateSamePacketNode((wsContentType.Equal(FX_WSTRC(L"text/xml")))
                                     ? XFA_ELEMENT_Sharpxml
                                     : XFA_ELEMENT_Sharptext);
        InsertChild(pContentRawDataNode);
      }
      return pContentRawDataNode->SetScriptContent(
          wsContent, wsXMLValue, bNotify, bScriptModify, bSyncData);
    } break;
    case XFA_OBJECTTYPE_NodeC:
    case XFA_OBJECTTYPE_TextNode:
      pNode = this;
      break;
    case XFA_OBJECTTYPE_NodeV:
      pNode = this;
      if (bSyncData && GetPacketID() == XFA_XDPPACKET_Form) {
        CXFA_Node* pParent = GetNodeItem(XFA_NODEITEM_Parent);
        if (pParent) {
          pParent = pParent->GetNodeItem(XFA_NODEITEM_Parent);
        }
        if (pParent && pParent->GetClassID() == XFA_ELEMENT_Value) {
          pParent = pParent->GetNodeItem(XFA_NODEITEM_Parent);
          if (pParent && pParent->IsContainerNode()) {
            pBindNode = pParent->GetBindData();
            if (pBindNode) {
              pBindNode->SetScriptContent(wsContent, wsXMLValue, bNotify,
                                          bScriptModify, FALSE);
            }
          }
        }
      }
      break;
    default:
      if (GetClassID() == XFA_ELEMENT_DataValue) {
        pNode = this;
        pBindNode = this;
      }
      break;
  }
  if (pNode) {
    SetAttributeValue(wsContent, wsXMLValue, bNotify, bScriptModify);
    if (pBindNode && bSyncData) {
      CXFA_NodeArray nodeArray;
      pBindNode->GetBindItems(nodeArray);
      for (int32_t i = 0; i < nodeArray.GetSize(); i++) {
        CXFA_Node* pNode = nodeArray[i];
        pNode->SetScriptContent(wsContent, wsContent, bNotify, bScriptModify,
                                FALSE);
      }
    }
    return TRUE;
  }
  return FALSE;
}
FX_BOOL CXFA_Node::SetContent(const CFX_WideString& wsContent,
                              const CFX_WideString& wsXMLValue,
                              FX_BOOL bNotify,
                              FX_BOOL bScriptModify,
                              FX_BOOL bSyncData) {
  return SetScriptContent(wsContent, wsXMLValue, bNotify, bScriptModify,
                          bSyncData);
}
CFX_WideString CXFA_Node::GetScriptContent(FX_BOOL bScriptModify) {
  CFX_WideString wsContent;
  return TryContent(wsContent, bScriptModify) ? wsContent : CFX_WideString();
}
CFX_WideString CXFA_Node::GetContent() {
  return GetScriptContent();
}
FX_BOOL CXFA_Node::TryContent(CFX_WideString& wsContent,
                              FX_BOOL bScriptModify,
                              FX_BOOL bProto) {
  CXFA_Node* pNode = NULL;
  switch (GetObjectType()) {
    case XFA_OBJECTTYPE_ContainerNode:
      if (GetClassID() == XFA_ELEMENT_ExclGroup) {
        pNode = this;
      } else {
        CXFA_Node* pValue = GetChild(0, XFA_ELEMENT_Value);
        if (!pValue) {
          return FALSE;
        }
        CXFA_Node* pChildValue = pValue->GetNodeItem(XFA_NODEITEM_FirstChild);
        if (pChildValue && XFA_FieldIsMultiListBox(this)) {
          pChildValue->SetAttribute(XFA_ATTRIBUTE_ContentType,
                                    FX_WSTRC(L"text/xml"));
        }
        return pChildValue
                   ? pChildValue->TryContent(wsContent, bScriptModify, bProto)
                   : FALSE;
      }
      break;
    case XFA_OBJECTTYPE_ContentNode: {
      CXFA_Node* pContentRawDataNode = GetNodeItem(XFA_NODEITEM_FirstChild);
      if (!pContentRawDataNode) {
        XFA_ELEMENT element = XFA_ELEMENT_Sharptext;
        if (GetClassID() == XFA_ELEMENT_ExData) {
          CFX_WideString wsContentType;
          GetAttribute(XFA_ATTRIBUTE_ContentType, wsContentType, FALSE);
          if (wsContentType.Equal(FX_WSTRC(L"text/html"))) {
            element = XFA_ELEMENT_SharpxHTML;
          } else if (wsContentType.Equal(FX_WSTRC(L"text/xml"))) {
            element = XFA_ELEMENT_Sharpxml;
          }
        }
        pContentRawDataNode = CreateSamePacketNode(element);
        InsertChild(pContentRawDataNode);
      }
      return pContentRawDataNode->TryContent(wsContent, bScriptModify, bProto);
    }
    case XFA_OBJECTTYPE_NodeC:
    case XFA_OBJECTTYPE_NodeV:
    case XFA_OBJECTTYPE_TextNode:
      pNode = this;
    default:
      if (GetClassID() == XFA_ELEMENT_DataValue) {
        pNode = this;
      }
      break;
  }
  if (pNode) {
    if (bScriptModify) {
      IXFA_ScriptContext* pScriptContext = m_pDocument->GetScriptContext();
      if (pScriptContext) {
        m_pDocument->GetScriptContext()->AddNodesOfRunScript(this);
      }
    }
    return TryCData(XFA_ATTRIBUTE_Value, wsContent, FALSE, bProto);
  }
  return FALSE;
}
CXFA_Node* CXFA_Node::GetModelNode() {
  switch (GetPacketID()) {
    case XFA_XDPPACKET_XDP:
      return m_pDocument->GetRoot();
    case XFA_XDPPACKET_Config:
      return (CXFA_Node*)m_pDocument->GetXFANode(XFA_HASHCODE_Config);
    case XFA_XDPPACKET_Template:
      return (CXFA_Node*)m_pDocument->GetXFANode(XFA_HASHCODE_Template);
    case XFA_XDPPACKET_Form:
      return (CXFA_Node*)m_pDocument->GetXFANode(XFA_HASHCODE_Form);
    case XFA_XDPPACKET_Datasets:
      return (CXFA_Node*)m_pDocument->GetXFANode(XFA_HASHCODE_Datasets);
    case XFA_XDPPACKET_LocaleSet:
      return (CXFA_Node*)m_pDocument->GetXFANode(XFA_HASHCODE_LocaleSet);
    case XFA_XDPPACKET_ConnectionSet:
      return (CXFA_Node*)m_pDocument->GetXFANode(XFA_HASHCODE_ConnectionSet);
    case XFA_XDPPACKET_SourceSet:
      return (CXFA_Node*)m_pDocument->GetXFANode(XFA_HASHCODE_SourceSet);
    case XFA_XDPPACKET_Xdc:
      return (CXFA_Node*)m_pDocument->GetXFANode(XFA_HASHCODE_Xdc);
    default:
      return this;
  }
}
FX_BOOL CXFA_Node::TryNamespace(CFX_WideString& wsNamespace) {
  wsNamespace.Empty();
  if (this->GetObjectType() == XFA_OBJECTTYPE_ModelNode ||
      this->GetClassID() == XFA_ELEMENT_Packet) {
    IFDE_XMLNode* pXMLNode = this->GetXMLMappingNode();
    if (!pXMLNode || pXMLNode->GetType() != FDE_XMLNODE_Element) {
      return FALSE;
    }
    ((IFDE_XMLElement*)pXMLNode)->GetNamespaceURI(wsNamespace);
    return TRUE;
  } else if (this->GetPacketID() == XFA_XDPPACKET_Datasets) {
    IFDE_XMLNode* pXMLNode = this->GetXMLMappingNode();
    if (!pXMLNode) {
      return FALSE;
    }
    if (pXMLNode->GetType() != FDE_XMLNODE_Element) {
      return TRUE;
    }
    if (this->GetClassID() == XFA_ELEMENT_DataValue &&
        this->GetEnum(XFA_ATTRIBUTE_Contains) == XFA_ATTRIBUTEENUM_MetaData) {
      return XFA_FDEExtension_ResolveNamespaceQualifier(
          (IFDE_XMLElement*)pXMLNode,
          this->GetCData(XFA_ATTRIBUTE_QualifiedName), wsNamespace);
    }
    ((IFDE_XMLElement*)pXMLNode)->GetNamespaceURI(wsNamespace);
    return TRUE;
  } else {
    CXFA_Node* pModelNode = GetModelNode();
    return pModelNode->TryNamespace(wsNamespace);
  }
}
CXFA_Node* CXFA_Node::GetProperty(int32_t index,
                                  XFA_ELEMENT eProperty,
                                  FX_BOOL bCreateProperty) {
  XFA_ELEMENT eElement = GetClassID();
  FX_DWORD dwPacket = GetPacketID();
  XFA_LPCPROPERTY pProperty =
      XFA_GetPropertyOfElement(eElement, eProperty, dwPacket);
  if (pProperty == NULL || index >= pProperty->uOccur) {
    return NULL;
  }
  CXFA_Node* pNode = m_pChild;
  int32_t iCount = 0;
  for (; pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetClassID() == eProperty) {
      iCount++;
      if (iCount > index) {
        return pNode;
      }
    }
  }
  if (!bCreateProperty) {
    return NULL;
  }
  if (pProperty->uFlags & XFA_PROPERTYFLAG_OneOf) {
    pNode = m_pChild;
    for (; pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
      XFA_LPCPROPERTY pExistProterty =
          XFA_GetPropertyOfElement(eElement, pNode->GetClassID(), dwPacket);
      if (pExistProterty && (pExistProterty->uFlags & XFA_PROPERTYFLAG_OneOf)) {
        return NULL;
      }
    }
  }
  IXFA_ObjFactory* pFactory = m_pDocument->GetParser()->GetFactory();
  XFA_LPCPACKETINFO pPacket = XFA_GetPacketByID(dwPacket);
  CXFA_Node* pNewNode;
  for (; iCount <= index; iCount++) {
    pNewNode = pFactory->CreateNode(pPacket, eProperty);
    if (!pNewNode) {
      return NULL;
    }
    this->InsertChild(pNewNode, NULL);
    pNewNode->SetFlag(XFA_NODEFLAG_Initialized);
  }
  return pNewNode;
}
int32_t CXFA_Node::CountChildren(XFA_ELEMENT eElement, FX_BOOL bOnlyChild) {
  CXFA_Node* pNode = m_pChild;
  int32_t iCount = 0;
  for (; pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetClassID() == eElement || eElement == XFA_ELEMENT_UNKNOWN) {
      if (bOnlyChild) {
        XFA_LPCPROPERTY pPropert = XFA_GetPropertyOfElement(
            GetClassID(), pNode->GetClassID(), XFA_XDPPACKET_UNKNOWN);
        if (pPropert) {
          continue;
        }
      }
      iCount++;
    }
  }
  return iCount;
}
CXFA_Node* CXFA_Node::GetChild(int32_t index,
                               XFA_ELEMENT eElement,
                               FX_BOOL bOnlyChild) {
  FXSYS_assert(index > -1);
  CXFA_Node* pNode = m_pChild;
  int32_t iCount = 0;
  for (; pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetClassID() == eElement || eElement == XFA_ELEMENT_UNKNOWN) {
      if (bOnlyChild) {
        XFA_LPCPROPERTY pPropert = XFA_GetPropertyOfElement(
            GetClassID(), pNode->GetClassID(), XFA_XDPPACKET_UNKNOWN);
        if (pPropert) {
          continue;
        }
      }
      iCount++;
      if (iCount > index) {
        return pNode;
      }
    }
  }
  return NULL;
}
int32_t CXFA_Node::InsertChild(int32_t index, CXFA_Node* pNode) {
  ASSERT(pNode != NULL && pNode->m_pNext == NULL);
  pNode->m_pParent = this;
  FX_BOOL bWasPurgeNode = m_pDocument->RemovePurgeNode(pNode);
  FXSYS_assert(bWasPurgeNode == TRUE);
  if (m_pChild == NULL || index == 0) {
    if (index > 0) {
      return -1;
    }
    pNode->m_pNext = m_pChild;
    m_pChild = pNode;
    index = 0;
  } else if (index < 0) {
    m_pLastChild->m_pNext = pNode;
  } else {
    CXFA_Node* pPrev = m_pChild;
    int32_t iCount = 0;
    while (++iCount != index && pPrev->m_pNext) {
      pPrev = pPrev->m_pNext;
    }
    if (index > 0 && index != iCount) {
      return -1;
    }
    pNode->m_pNext = pPrev->m_pNext;
    pPrev->m_pNext = pNode;
    index = iCount;
  }
  if (pNode->m_pNext == NULL) {
    m_pLastChild = pNode;
  }
  ASSERT(m_pLastChild != NULL && m_pLastChild->m_pNext == NULL);
  pNode->SetFlag(XFA_NODEFLAG_HasRemoved, FALSE);
  IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
  if (pNotify) {
    pNotify->OnNodeEvent(this, XFA_NODEEVENT_ChildAdded, pNode);
  }
  if (IsNeedSavingXMLNode() && pNode->m_pXMLNode) {
    FXSYS_assert(pNode->m_pXMLNode->GetNodeItem(IFDE_XMLNode::Parent) == NULL);
    m_pXMLNode->InsertChildNode(pNode->m_pXMLNode, index);
    pNode->SetFlag(XFA_NODEFLAG_OwnXMLNode, FALSE, FALSE);
  }
  return index;
}
FX_BOOL CXFA_Node::InsertChild(CXFA_Node* pNode, CXFA_Node* pBeforeNode) {
  if (!pNode || pNode->m_pParent != NULL ||
      (pBeforeNode && pBeforeNode->m_pParent != this)) {
    FXSYS_assert(FALSE);
    return FALSE;
  }
  FX_BOOL bWasPurgeNode = m_pDocument->RemovePurgeNode(pNode);
  FXSYS_assert(bWasPurgeNode == TRUE);
  int32_t nIndex = -1;
  pNode->m_pParent = this;
  if (m_pChild == NULL || pBeforeNode == m_pChild) {
    pNode->m_pNext = m_pChild;
    m_pChild = pNode;
    nIndex = 0;
  } else if (!pBeforeNode) {
    pNode->m_pNext = m_pLastChild->m_pNext;
    m_pLastChild->m_pNext = pNode;
  } else {
    nIndex = 1;
    CXFA_Node* pPrev = m_pChild;
    while (pPrev->m_pNext != pBeforeNode) {
      pPrev = pPrev->m_pNext;
      nIndex++;
    }
    pNode->m_pNext = pPrev->m_pNext;
    pPrev->m_pNext = pNode;
  }
  if (pNode->m_pNext == NULL) {
    m_pLastChild = pNode;
  }
  ASSERT(m_pLastChild != NULL && m_pLastChild->m_pNext == NULL);
  pNode->SetFlag(XFA_NODEFLAG_HasRemoved, FALSE);
  IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
  if (pNotify) {
    pNotify->OnNodeEvent(this, XFA_NODEEVENT_ChildAdded, pNode);
  }
  if (IsNeedSavingXMLNode() && pNode->m_pXMLNode) {
    FXSYS_assert(pNode->m_pXMLNode->GetNodeItem(IFDE_XMLNode::Parent) == NULL);
    m_pXMLNode->InsertChildNode(pNode->m_pXMLNode, nIndex);
    pNode->SetFlag(XFA_NODEFLAG_OwnXMLNode, FALSE, FALSE);
  }
  return TRUE;
}
CXFA_Node* CXFA_Node::Deprecated_GetPrevSibling() {
  if (!m_pParent) {
    return NULL;
  }
  for (CXFA_Node* pSibling = m_pParent->m_pChild; pSibling;
       pSibling = pSibling->m_pNext) {
    if (pSibling->m_pNext == this) {
      return pSibling;
    }
  }
  return NULL;
}
FX_BOOL CXFA_Node::RemoveChild(CXFA_Node* pNode, FX_BOOL bNotify) {
  if (pNode == NULL || pNode->m_pParent != this) {
    FXSYS_assert(FALSE);
    return FALSE;
  }
  if (m_pChild == pNode) {
    m_pChild = pNode->m_pNext;
    if (m_pLastChild == pNode) {
      m_pLastChild = pNode->m_pNext;
    }
    pNode->m_pNext = NULL;
    pNode->m_pParent = NULL;
  } else {
    CXFA_Node* pPrev = pNode->Deprecated_GetPrevSibling();
    pPrev->m_pNext = pNode->m_pNext;
    if (m_pLastChild == pNode) {
      m_pLastChild = pNode->m_pNext ? pNode->m_pNext : pPrev;
    }
    pNode->m_pNext = NULL;
    pNode->m_pParent = NULL;
  }
  ASSERT(m_pLastChild == NULL || m_pLastChild->m_pNext == NULL);
  OnRemoved(this, pNode, bNotify);
  pNode->SetFlag(XFA_NODEFLAG_HasRemoved);
  m_pDocument->AddPurgeNode(pNode);
  if (IsNeedSavingXMLNode() && pNode->m_pXMLNode) {
    if (pNode->IsAttributeInXML()) {
      FXSYS_assert(pNode->m_pXMLNode == m_pXMLNode &&
                   m_pXMLNode->GetType() == FDE_XMLNODE_Element);
      if (pNode->m_pXMLNode->GetType() == FDE_XMLNODE_Element) {
        IFDE_XMLElement* pXMLElement = (IFDE_XMLElement*)(pNode->m_pXMLNode);
        CFX_WideStringC wsAttributeName =
            pNode->GetCData(XFA_ATTRIBUTE_QualifiedName);
        pXMLElement->RemoveAttribute(wsAttributeName.GetPtr());
      }
      CFX_WideString wsName;
      pNode->GetAttribute(XFA_ATTRIBUTE_Name, wsName, FALSE);
      IFDE_XMLElement* pNewXMLElement = IFDE_XMLElement::Create(wsName);
      CFX_WideStringC wsValue = this->GetCData(XFA_ATTRIBUTE_Value);
      if (!wsValue.IsEmpty()) {
        pNewXMLElement->SetTextData(wsValue);
      }
      pNode->m_pXMLNode = pNewXMLElement;
      pNode->SetEnum(XFA_ATTRIBUTE_Contains, XFA_ATTRIBUTEENUM_Unknown);
    } else {
      m_pXMLNode->RemoveChildNode(pNode->m_pXMLNode);
    }
    pNode->SetFlag(XFA_NODEFLAG_OwnXMLNode, TRUE, FALSE);
  }
  return TRUE;
}
CXFA_Node* CXFA_Node::GetFirstChildByName(const CFX_WideStringC& wsName) const {
  return GetFirstChildByName(
      wsName.IsEmpty() ? 0 : FX_HashCode_String_GetW(wsName.GetPtr(),
                                                     wsName.GetLength()));
}
CXFA_Node* CXFA_Node::GetFirstChildByName(FX_DWORD dwNameHash) const {
  for (CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_FirstChild); pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetNameHash() == dwNameHash) {
      return pNode;
    }
  }
  return NULL;
}
CXFA_Node* CXFA_Node::GetFirstChildByClass(XFA_ELEMENT eElement) const {
  for (CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_FirstChild); pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetClassID() == eElement) {
      return pNode;
    }
  }
  return NULL;
}
CXFA_Node* CXFA_Node::GetNextSameNameSibling(FX_DWORD dwNameHash) const {
  for (CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_NextSibling); pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetNameHash() == dwNameHash) {
      return pNode;
    }
  }
  return NULL;
}
CXFA_Node* CXFA_Node::GetNextSameNameSibling(
    const CFX_WideStringC& wsNodeName) const {
  return GetNextSameNameSibling(
      wsNodeName.IsEmpty() ? 0
                           : FX_HashCode_String_GetW(wsNodeName.GetPtr(),
                                                     wsNodeName.GetLength()));
}
CXFA_Node* CXFA_Node::GetNextSameClassSibling(XFA_ELEMENT eElement) const {
  for (CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_NextSibling); pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetClassID() == eElement) {
      return pNode;
    }
  }
  return NULL;
}
int32_t CXFA_Node::GetNodeSameNameIndex() const {
  IXFA_ScriptContext* pScriptContext = m_pDocument->GetScriptContext();
  if (!pScriptContext) {
    return -1;
  }
  return pScriptContext->GetIndexByName((CXFA_Node*)this);
}
int32_t CXFA_Node::GetNodeSameClassIndex() const {
  IXFA_ScriptContext* pScriptContext = m_pDocument->GetScriptContext();
  if (!pScriptContext) {
    return -1;
  }
  return pScriptContext->GetIndexByClassName((CXFA_Node*)this);
}
void CXFA_Node::GetSOMExpression(CFX_WideString& wsSOMExpression) {
  IXFA_ScriptContext* pScriptContext = m_pDocument->GetScriptContext();
  if (!pScriptContext) {
    return;
  }
  pScriptContext->GetSomExpression(this, wsSOMExpression);
}
CXFA_Node* CXFA_Node::GetInstanceMgrOfSubform() {
  CXFA_Node* pInstanceMgr = NULL;
  if (m_ePacket == XFA_XDPPACKET_Form) {
    CXFA_Node* pParentNode = GetNodeItem(XFA_NODEITEM_Parent);
    if (!pParentNode || pParentNode->GetClassID() == XFA_ELEMENT_Area) {
      return pInstanceMgr;
    }
    for (CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_PrevSibling);
         pNode != NULL; pNode = pNode->GetNodeItem(XFA_NODEITEM_PrevSibling)) {
      XFA_ELEMENT eType = pNode->GetClassID();
      if ((eType == XFA_ELEMENT_Subform || eType == XFA_ELEMENT_SubformSet) &&
          pNode->m_dwNameHash != m_dwNameHash) {
        break;
      }
      if (eType == XFA_ELEMENT_InstanceManager) {
        CFX_WideStringC wsName = GetCData(XFA_ATTRIBUTE_Name);
        CFX_WideStringC wsInstName = pNode->GetCData(XFA_ATTRIBUTE_Name);
        if (wsInstName.GetLength() > 0 && wsInstName.GetAt(0) == '_' &&
            wsInstName.Mid(1) == wsName) {
          pInstanceMgr = pNode;
        }
        break;
      }
    }
  }
  return pInstanceMgr;
}
CXFA_Node* CXFA_Node::GetOccurNode() {
  return GetFirstChildByClass(XFA_ELEMENT_Occur);
}
FX_BOOL CXFA_Node::HasFlag(FX_DWORD dwFlag) const {
  if (m_uFlags & dwFlag) {
    return TRUE;
  }
  switch (dwFlag) {
    case XFA_NODEFLAG_HasRemoved:
      return m_pParent && m_pParent->HasFlag(dwFlag);
    default:
      break;
  }
  return FALSE;
}
void CXFA_Node::SetFlag(FX_DWORD dwFlag, FX_BOOL bOn, FX_BOOL bNotify) {
  if (bOn) {
    switch (dwFlag) {
      case XFA_NODEFLAG_Initialized:
        if (bNotify && !HasFlag(XFA_NODEFLAG_Initialized)) {
          IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
          if (pNotify) {
            pNotify->OnNodeEvent(this, XFA_NODEEVENT_Ready);
          }
        }
        break;
      default:
        break;
    }
    m_uFlags |= dwFlag;
  } else {
    m_uFlags &= ~dwFlag;
  }
}
FX_BOOL CXFA_Node::IsAttributeInXML() {
  return GetEnum(XFA_ATTRIBUTE_Contains) == XFA_ATTRIBUTEENUM_MetaData;
}
void CXFA_Node::OnRemoved(CXFA_Node* pParent,
                          CXFA_Node* pRemoved,
                          FX_BOOL bNotify) {
  if (bNotify && (pParent != NULL)) {
    IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
    if (pNotify) {
      pNotify->OnNodeEvent(pParent, XFA_NODEEVENT_ChildRemoved, pRemoved);
    }
  }
}
void CXFA_Node::OnChanging(XFA_ATTRIBUTE eAttr,
                           void* pNewValue,
                           FX_BOOL bNotify) {
  if (bNotify && HasFlag(XFA_NODEFLAG_Initialized)) {
    IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
    if (pNotify) {
      pNotify->OnNodeEvent(this, XFA_NODEEVENT_ValueChanging,
                           (void*)(uintptr_t)eAttr, pNewValue);
    }
  }
}
void CXFA_Node::OnChanged(XFA_ATTRIBUTE eAttr,
                          void* pNewValue,
                          FX_BOOL bNotify,
                          FX_BOOL bScriptModify) {
  if (bNotify && HasFlag(XFA_NODEFLAG_Initialized)) {
    Script_Attribute_SendAttributeChangeMessage((void*)(uintptr_t)eAttr,
                                                pNewValue, bScriptModify);
  }
}
int32_t CXFA_Node::execSingleEventByName(const CFX_WideStringC& wsEventName,
                                         XFA_ELEMENT eElementType) {
  int32_t iRet = XFA_EVENTERROR_NotExist;
  const XFA_ExecEventParaInfo* eventParaInfo =
      GetEventParaInfoByName(wsEventName);
  if (eventParaInfo) {
    uint32_t validFlags = eventParaInfo->m_validFlags;
    IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
    if (!pNotify) {
      return iRet;
    }
    if (validFlags == 1) {
      iRet = pNotify->ExecEventByDeepFirst(this, eventParaInfo->m_eventType);
    } else if (validFlags == 2) {
      iRet = pNotify->ExecEventByDeepFirst(this, eventParaInfo->m_eventType,
                                           FALSE, FALSE);
    } else if (validFlags == 3) {
      if (eElementType == XFA_ELEMENT_Subform) {
        iRet = pNotify->ExecEventByDeepFirst(this, eventParaInfo->m_eventType,
                                             FALSE, FALSE);
      }
    } else if (validFlags == 4) {
      if (eElementType == XFA_ELEMENT_ExclGroup ||
          eElementType == XFA_ELEMENT_Field) {
        CXFA_Node* pParentNode = GetNodeItem(XFA_NODEITEM_Parent);
        if (pParentNode && pParentNode->GetClassID() == XFA_ELEMENT_ExclGroup) {
          iRet = pNotify->ExecEventByDeepFirst(this, eventParaInfo->m_eventType,
                                               FALSE, FALSE);
        }
        iRet = pNotify->ExecEventByDeepFirst(this, eventParaInfo->m_eventType,
                                             FALSE, FALSE);
      }
    } else if (validFlags == 5) {
      if (eElementType == XFA_ELEMENT_Field) {
        iRet = pNotify->ExecEventByDeepFirst(this, eventParaInfo->m_eventType,
                                             FALSE, FALSE);
      }
    } else if (validFlags == 6) {
      CXFA_WidgetData* pWidgetData = GetWidgetData();
      if (pWidgetData) {
        CXFA_Node* pUINode = pWidgetData->GetUIChild();
        if (pUINode->m_eNodeClass == XFA_ELEMENT_Signature) {
          iRet = pNotify->ExecEventByDeepFirst(this, eventParaInfo->m_eventType,
                                               FALSE, FALSE);
        }
      }
    } else if (validFlags == 7) {
      CXFA_WidgetData* pWidgetData = GetWidgetData();
      if (pWidgetData) {
        CXFA_Node* pUINode = pWidgetData->GetUIChild();
        if ((pUINode->m_eNodeClass == XFA_ELEMENT_ChoiceList) &&
            (!pWidgetData->IsListBox())) {
          iRet = pNotify->ExecEventByDeepFirst(this, eventParaInfo->m_eventType,
                                               FALSE, FALSE);
        }
      }
    }
  }
  return iRet;
}
void CXFA_Node::UpdateNameHash() {
  XFA_LPCNOTSUREATTRIBUTE pNotsure =
      XFA_GetNotsureAttribute(GetClassID(), XFA_ATTRIBUTE_Name);
  if (!pNotsure || pNotsure->eType == XFA_ATTRIBUTETYPE_Cdata) {
    CFX_WideStringC wsName = GetCData(XFA_ATTRIBUTE_Name);
    m_dwNameHash =
        wsName.IsEmpty() ? 0 : FX_HashCode_String_GetW(wsName.GetPtr(),
                                                       wsName.GetLength());
  } else if (pNotsure->eType == XFA_ATTRIBUTETYPE_Enum) {
    CFX_WideStringC wsName =
        XFA_GetAttributeEnumByID(GetEnum(XFA_ATTRIBUTE_Name))->pName;
    m_dwNameHash =
        wsName.IsEmpty() ? 0 : FX_HashCode_String_GetW(wsName.GetPtr(),
                                                       wsName.GetLength());
  }
}
IFDE_XMLNode* CXFA_Node::CreateXMLMappingNode() {
  if (!m_pXMLNode) {
    CFX_WideStringC wsTag = GetCData(XFA_ATTRIBUTE_Name);
    m_pXMLNode = IFDE_XMLElement::Create(wsTag);
    SetFlag(XFA_NODEFLAG_OwnXMLNode, TRUE, FALSE);
  }
  return m_pXMLNode;
}
FX_BOOL CXFA_Node::IsNeedSavingXMLNode() {
  return m_pXMLNode && (GetPacketID() == XFA_XDPPACKET_Datasets ||
                        GetClassID() == XFA_ELEMENT_Xfa);
}
XFA_LPMAPMODULEDATA CXFA_Node::GetMapModuleData(FX_BOOL bCreateNew) {
  if (!m_pMapModuleData && bCreateNew) {
    m_pMapModuleData = new XFA_MAPMODULEDATA;
  }
  return m_pMapModuleData;
}
void CXFA_Node::SetMapModuleValue(void* pKey, void* pValue) {
  XFA_LPMAPMODULEDATA pMoudle = this->GetMapModuleData(TRUE);
  if (!pMoudle) {
    return;
  }
  pMoudle->m_ValueMap.SetAt(pKey, pValue);
}
FX_BOOL CXFA_Node::GetMapModuleValue(void* pKey, void*& pValue) {
  CXFA_Node* pNode = this;
  while (pNode) {
    XFA_LPMAPMODULEDATA pMoudle = pNode->GetMapModuleData(FALSE);
    if (pMoudle && pMoudle->m_ValueMap.Lookup(pKey, pValue)) {
      return TRUE;
    }
    pNode = pNode->GetPacketID() != XFA_XDPPACKET_Datasets
                ? pNode->GetTemplateNode()
                : NULL;
  }
  return FALSE;
}
void CXFA_Node::SetMapModuleString(void* pKey, const CFX_WideStringC& wsValue) {
  SetMapModuleBuffer(pKey, (void*)wsValue.GetPtr(),
                     wsValue.GetLength() * sizeof(FX_WCHAR));
}
FX_BOOL CXFA_Node::GetMapModuleString(void* pKey, CFX_WideStringC& wsValue) {
  void* pValue;
  int32_t iBytes;
  if (!GetMapModuleBuffer(pKey, pValue, iBytes)) {
    return FALSE;
  }
  wsValue = CFX_WideStringC((const FX_WCHAR*)pValue, iBytes / sizeof(FX_WCHAR));
  return TRUE;
}
void CXFA_Node::SetMapModuleBuffer(
    void* pKey,
    void* pValue,
    int32_t iBytes,
    XFA_MAPDATABLOCKCALLBACKINFO* pCallbackInfo) {
  XFA_LPMAPMODULEDATA pMoudle = this->GetMapModuleData(TRUE);
  if (!pMoudle) {
    return;
  }
  XFA_LPMAPDATABLOCK& pBuffer = pMoudle->m_BufferMap[pKey];
  if (pBuffer == NULL) {
    pBuffer = (XFA_LPMAPDATABLOCK)FX_Alloc(uint8_t,
                                           sizeof(XFA_MAPDATABLOCK) + iBytes);
  } else if (pBuffer->iBytes != iBytes) {
    if (pBuffer->pCallbackInfo && pBuffer->pCallbackInfo->pFree) {
      pBuffer->pCallbackInfo->pFree(*(void**)pBuffer->GetData());
    }
    pBuffer = (XFA_LPMAPDATABLOCK)FX_Realloc(uint8_t, pBuffer,
                                             sizeof(XFA_MAPDATABLOCK) + iBytes);
  } else if (pBuffer->pCallbackInfo && pBuffer->pCallbackInfo->pFree) {
    pBuffer->pCallbackInfo->pFree(*(void**)pBuffer->GetData());
  }
  if (pBuffer == NULL) {
    return;
  }
  pBuffer->pCallbackInfo = pCallbackInfo;
  pBuffer->iBytes = iBytes;
  FXSYS_memcpy(pBuffer->GetData(), pValue, iBytes);
}
FX_BOOL CXFA_Node::GetMapModuleBuffer(void* pKey,
                                      void*& pValue,
                                      int32_t& iBytes,
                                      FX_BOOL bProtoAlso) {
  XFA_LPMAPDATABLOCK pBuffer = NULL;
  CXFA_Node* pNode = this;
  while (pNode) {
    XFA_LPMAPMODULEDATA pMoudle = pNode->GetMapModuleData(FALSE);
    if (pMoudle && pMoudle->m_BufferMap.Lookup(pKey, pBuffer)) {
      break;
    }
    pNode = (bProtoAlso && pNode->GetPacketID() != XFA_XDPPACKET_Datasets)
                ? pNode->GetTemplateNode()
                : NULL;
  }
  if (pBuffer == NULL) {
    return FALSE;
  }
  pValue = pBuffer->GetData();
  iBytes = pBuffer->iBytes;
  return TRUE;
}
FX_BOOL CXFA_Node::HasMapModuleKey(void* pKey, FX_BOOL bProtoAlso) {
  CXFA_Node* pNode = this;
  while (pNode) {
    void* pVal;
    XFA_LPMAPMODULEDATA pMoudle = pNode->GetMapModuleData(FALSE);
    if (pMoudle &&
        (pMoudle->m_ValueMap.Lookup(pKey, pVal) ||
         pMoudle->m_BufferMap.Lookup(pKey, (XFA_LPMAPDATABLOCK&)pVal))) {
      return TRUE;
    }
    pNode = (bProtoAlso && pNode->GetPacketID() != XFA_XDPPACKET_Datasets)
                ? pNode->GetTemplateNode()
                : NULL;
  }
  return FALSE;
}
void CXFA_Node::RemoveMapModuleKey(void* pKey) {
  XFA_LPMAPMODULEDATA pMoudle = this->GetMapModuleData(FALSE);
  if (!pMoudle) {
    return;
  }
  if (pKey) {
    XFA_LPMAPDATABLOCK pBuffer = NULL;
    pMoudle->m_BufferMap.Lookup(pKey, pBuffer);
    if (pBuffer) {
      if (pBuffer->pCallbackInfo && pBuffer->pCallbackInfo->pFree) {
        pBuffer->pCallbackInfo->pFree(*(void**)pBuffer->GetData());
      }
      FX_Free(pBuffer);
    }
    pMoudle->m_BufferMap.RemoveKey(pKey);
    pMoudle->m_ValueMap.RemoveKey(pKey);
  } else {
    XFA_LPMAPDATABLOCK pBuffer;
    FX_POSITION posBuffer = pMoudle->m_BufferMap.GetStartPosition();
    while (posBuffer) {
      pMoudle->m_BufferMap.GetNextAssoc(posBuffer, pKey, pBuffer);
      if (pBuffer) {
        if (pBuffer->pCallbackInfo && pBuffer->pCallbackInfo->pFree) {
          pBuffer->pCallbackInfo->pFree(*(void**)pBuffer->GetData());
        }
        FX_Free(pBuffer);
      }
    }
    pMoudle->m_BufferMap.RemoveAll();
    pMoudle->m_ValueMap.RemoveAll();
    if (pMoudle) {
      delete pMoudle;
      pMoudle = NULL;
    }
  }
}
void CXFA_Node::MergeAllData(void* pDstModule, FX_BOOL bUseSrcAttr) {
  XFA_LPMAPMODULEDATA pDstModuleData =
      ((CXFA_Node*)pDstModule)->GetMapModuleData(TRUE);
  if (!pDstModuleData) {
    return;
  }
  XFA_LPMAPMODULEDATA pSrcModuleData = this->GetMapModuleData(FALSE);
  if (!pSrcModuleData) {
    return;
  }
  FX_POSITION psValue = pSrcModuleData->m_ValueMap.GetStartPosition();
  while (psValue) {
    void* pKey;
    void* pValue;
    pSrcModuleData->m_ValueMap.GetNextAssoc(psValue, pKey, pValue);
    if (bUseSrcAttr || !pDstModuleData->m_ValueMap.GetValueAt(pKey)) {
      pDstModuleData->m_ValueMap.SetAt(pKey, pValue);
    }
  }
  FX_POSITION psBuffer = pSrcModuleData->m_BufferMap.GetStartPosition();
  while (psBuffer) {
    void* pKey;
    XFA_LPMAPDATABLOCK pSrcBuffer;
    pSrcModuleData->m_BufferMap.GetNextAssoc(psBuffer, pKey, pSrcBuffer);
    XFA_LPMAPDATABLOCK& pBuffer = pDstModuleData->m_BufferMap[pKey];
    if (pBuffer && !bUseSrcAttr) {
      continue;
    }
    if (pSrcBuffer->pCallbackInfo && pSrcBuffer->pCallbackInfo->pFree &&
        !pSrcBuffer->pCallbackInfo->pCopy) {
      if (pBuffer) {
        pBuffer->pCallbackInfo->pFree(*(void**)pBuffer->GetData());
        pDstModuleData->m_BufferMap.RemoveKey(pKey);
      }
      continue;
    }
    if (pBuffer == NULL) {
      pBuffer = (XFA_LPMAPDATABLOCK)FX_Alloc(
          uint8_t, sizeof(XFA_MAPDATABLOCK) + pSrcBuffer->iBytes);
    } else if (pBuffer->iBytes != pSrcBuffer->iBytes) {
      if (pBuffer->pCallbackInfo && pBuffer->pCallbackInfo->pFree) {
        pBuffer->pCallbackInfo->pFree(*(void**)pBuffer->GetData());
      }
      pBuffer = (XFA_LPMAPDATABLOCK)FX_Realloc(
          uint8_t, pBuffer, sizeof(XFA_MAPDATABLOCK) + pSrcBuffer->iBytes);
    } else if (pBuffer->pCallbackInfo && pBuffer->pCallbackInfo->pFree) {
      pBuffer->pCallbackInfo->pFree(*(void**)pBuffer->GetData());
    }
    if (pBuffer == NULL) {
      continue;
    }
    pBuffer->pCallbackInfo = pSrcBuffer->pCallbackInfo;
    pBuffer->iBytes = pSrcBuffer->iBytes;
    FXSYS_memcpy(pBuffer->GetData(), pSrcBuffer->GetData(), pSrcBuffer->iBytes);
    if (pBuffer->pCallbackInfo && pBuffer->pCallbackInfo->pCopy) {
      pBuffer->pCallbackInfo->pCopy(*(void**)pBuffer->GetData());
    }
  }
}
void CXFA_Node::MoveBufferMapData(CXFA_Node* pDstModule, void* pKey) {
  if (!pDstModule) {
    return;
  }
  FX_BOOL bNeedMove = TRUE;
  if (!pKey) {
    bNeedMove = FALSE;
  }
  if (pDstModule->GetClassID() != this->GetClassID()) {
    bNeedMove = FALSE;
  }
  XFA_LPMAPMODULEDATA pSrcModuleData = NULL;
  XFA_LPMAPMODULEDATA pDstModuleData = NULL;
  if (bNeedMove) {
    pSrcModuleData = this->GetMapModuleData(FALSE);
    if (!pSrcModuleData) {
      bNeedMove = FALSE;
    }
    pDstModuleData = pDstModule->GetMapModuleData(TRUE);
    if (!pDstModuleData) {
      bNeedMove = FALSE;
    }
  }
  if (bNeedMove) {
    void* pBufferBlockData = pSrcModuleData->m_BufferMap.GetValueAt(pKey);
    if (pBufferBlockData) {
      pSrcModuleData->m_BufferMap.RemoveKey(pKey);
      pDstModuleData->m_BufferMap.RemoveKey(pKey);
      pDstModuleData->m_BufferMap.SetAt(pKey,
                                        (XFA_LPMAPDATABLOCK)pBufferBlockData);
    }
  }
  if (pDstModule->GetObjectType() == XFA_OBJECTTYPE_NodeV) {
    CFX_WideString wsValue = pDstModule->GetScriptContent(FALSE);
    CFX_WideString wsFormatValue(wsValue);
    CXFA_WidgetData* pWidgetData = pDstModule->GetContainerWidgetData();
    if (pWidgetData) {
      pWidgetData->GetFormatDataValue(wsValue, wsFormatValue);
    }
    pDstModule->SetScriptContent(wsValue, wsFormatValue, TRUE, TRUE);
  }
}
void CXFA_Node::MoveBufferMapData(CXFA_Node* pSrcModule,
                                  CXFA_Node* pDstModule,
                                  void* pKey,
                                  FX_BOOL bRecursive) {
  if (!pSrcModule || !pDstModule || !pKey) {
    return;
  }
  if (bRecursive) {
    CXFA_Node* pSrcChild = pSrcModule->GetNodeItem(XFA_NODEITEM_FirstChild);
    CXFA_Node* pDstChild = pDstModule->GetNodeItem(XFA_NODEITEM_FirstChild);
    for (; pSrcChild && pDstChild;
         pSrcChild = pSrcChild->GetNodeItem(XFA_NODEITEM_NextSibling),
         pDstChild = pDstChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
      MoveBufferMapData(pSrcChild, pDstChild, pKey, TRUE);
    }
  }
  pSrcModule->MoveBufferMapData(pDstModule, pKey);
}
CXFA_NodeList::CXFA_NodeList(CXFA_Document* pDocument)
    : CXFA_Object(pDocument, XFA_OBJECTTYPE_NodeList) {
  m_pDocument->GetScriptContext()->CacheList(this);
}
CXFA_Node* CXFA_NodeList::NamedItem(const CFX_WideStringC& wsName) {
  int32_t iCount = GetLength();
  FX_DWORD dwHashCode =
      FX_HashCode_String_GetW(wsName.GetPtr(), wsName.GetLength());
  for (int32_t i = 0; i < iCount; i++) {
    CXFA_Node* ret = Item(i);
    if (dwHashCode == ret->GetNameHash()) {
      return ret;
    }
  }
  return NULL;
}
void CXFA_NodeList::Script_ListClass_Append(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 1) {
    CXFA_Node* pNode = (CXFA_Node*)pArguments->GetObject(0);
    if (pNode) {
      Append(pNode);
    } else {
      ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
    }
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"append");
  }
}
void CXFA_NodeList::Script_ListClass_Insert(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 2) {
    CXFA_Node* pNewNode = (CXFA_Node*)pArguments->GetObject(0);
    CXFA_Node* pBeforeNode = (CXFA_Node*)pArguments->GetObject(1);
    if (pNewNode) {
      Insert(pNewNode, pBeforeNode);
    } else {
      ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
    }
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"insert");
  }
}
void CXFA_NodeList::Script_ListClass_Remove(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 1) {
    CXFA_Node* pNode = (CXFA_Node*)pArguments->GetObject(0);
    if (pNode) {
      Remove(pNode);
    } else {
      ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
    }
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"remove");
  }
}
void CXFA_NodeList::Script_ListClass_Item(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 1) {
    int32_t iIndex = pArguments->GetInt32(0);
    if ((iIndex >= 0) && (iIndex + 1 <= GetLength())) {
      FXJSE_Value_Set(
          pArguments->GetReturnValue(),
          m_pDocument->GetScriptContext()->GetJSValueFromMap(Item(iIndex)));
    } else {
      ThrowScriptErrorMessage(XFA_IDS_INDEX_OUT_OF_BOUNDS);
    }
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"item");
  }
}
void CXFA_NodeList::Script_TreelistClass_NamedItem(
    CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc == 1) {
    CFX_ByteString szName = pArguments->GetUTF8String(0);
    CXFA_Node* pNode =
        NamedItem(CFX_WideString::FromUTF8(szName, szName.GetLength()));
    if (!pNode) {
      return;
    }
    FXJSE_Value_Set(pArguments->GetReturnValue(),
                    m_pDocument->GetScriptContext()->GetJSValueFromMap(pNode));
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"namedItem");
  }
}
void CXFA_NodeList::Script_ListClass_Length(FXJSE_HVALUE hValue,
                                            FX_BOOL bSetting,
                                            XFA_ATTRIBUTE eAttribute) {
  if (!bSetting) {
    FXJSE_Value_SetInteger(hValue, GetLength());
  } else {
    ThrowScriptErrorMessage(XFA_IDS_INVAlID_PROP_SET);
  }
}
CXFA_ArrayNodeList::CXFA_ArrayNodeList(CXFA_Document* pDocument)
    : CXFA_NodeList(pDocument) {}
void CXFA_ArrayNodeList::SetArrayNodeList(const CXFA_NodeArray& srcArray) {
  if (srcArray.GetSize() > 0) {
    m_array.Copy(srcArray);
  }
}
int32_t CXFA_ArrayNodeList::GetLength() {
  return m_array.GetSize();
}
FX_BOOL CXFA_ArrayNodeList::Append(CXFA_Node* pNode) {
  m_array.Add(pNode);
  return TRUE;
}
FX_BOOL CXFA_ArrayNodeList::Insert(CXFA_Node* pNewNode,
                                   CXFA_Node* pBeforeNode) {
  if (pBeforeNode == NULL) {
    m_array.Add(pNewNode);
  } else {
    int32_t iSize = m_array.GetSize();
    for (int32_t i = 0; i < iSize; ++i) {
      if (m_array[i] == pBeforeNode) {
        m_array.InsertAt(i, pNewNode);
        break;
      }
    }
  }
  return TRUE;
}
FX_BOOL CXFA_ArrayNodeList::Remove(CXFA_Node* pNode) {
  int32_t iSize = m_array.GetSize();
  for (int32_t i = 0; i < iSize; ++i) {
    if (m_array[i] == pNode) {
      m_array.RemoveAt(i);
      break;
    }
  }
  return TRUE;
}
CXFA_Node* CXFA_ArrayNodeList::Item(int32_t iIndex) {
  int32_t iSize = m_array.GetSize();
  if (iIndex >= 0 && iIndex < iSize) {
    return m_array[iIndex];
  }
  return NULL;
}
CXFA_AttachNodeList::CXFA_AttachNodeList(CXFA_Document* pDocument,
                                         CXFA_Node* pAttachNode)
    : CXFA_NodeList(pDocument) {
  m_pAttachNode = pAttachNode;
}
int32_t CXFA_AttachNodeList::GetLength() {
  return m_pAttachNode->CountChildren(
      XFA_ELEMENT_UNKNOWN, m_pAttachNode->GetClassID() == XFA_ELEMENT_Subform);
}
FX_BOOL CXFA_AttachNodeList::Append(CXFA_Node* pNode) {
  CXFA_Node* pParent = pNode->GetNodeItem(XFA_NODEITEM_Parent);
  if (pParent) {
    pParent->RemoveChild(pNode);
  }
  return m_pAttachNode->InsertChild(pNode);
}
FX_BOOL CXFA_AttachNodeList::Insert(CXFA_Node* pNewNode,
                                    CXFA_Node* pBeforeNode) {
  CXFA_Node* pParent = pNewNode->GetNodeItem(XFA_NODEITEM_Parent);
  if (pParent) {
    pParent->RemoveChild(pNewNode);
  }
  return m_pAttachNode->InsertChild(pNewNode, pBeforeNode);
}
FX_BOOL CXFA_AttachNodeList::Remove(CXFA_Node* pNode) {
  return m_pAttachNode->RemoveChild(pNode);
}
CXFA_Node* CXFA_AttachNodeList::Item(int32_t iIndex) {
  return m_pAttachNode->GetChild(
      iIndex, XFA_ELEMENT_UNKNOWN,
      m_pAttachNode->GetClassID() == XFA_ELEMENT_Subform);
}
