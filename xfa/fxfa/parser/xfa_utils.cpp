// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/xfa_utils.h"

#include <algorithm>

#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/xml/cfx_xmlchardata.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"
#include "xfa/fxfa/parser/cxfa_localevalue.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/xfa_basic_data.h"

namespace {

const double fraction_scales[] = {0.1,
                                  0.01,
                                  0.001,
                                  0.0001,
                                  0.00001,
                                  0.000001,
                                  0.0000001,
                                  0.00000001,
                                  0.000000001,
                                  0.0000000001,
                                  0.00000000001,
                                  0.000000000001,
                                  0.0000000000001,
                                  0.00000000000001,
                                  0.000000000000001,
                                  0.0000000000000001};

}  // namespace

double XFA_GetFractionalScale(uint32_t idx) {
  return fraction_scales[idx];
}

int XFA_GetMaxFractionalScale() {
  return FX_ArraySize(fraction_scales);
}

CXFA_LocaleValue XFA_GetLocaleValue(CXFA_WidgetData* pWidgetData) {
  CXFA_Node* pNodeValue =
      pWidgetData->GetNode()->GetChild(0, XFA_Element::Value, false);
  if (!pNodeValue) {
    return CXFA_LocaleValue();
  }
  CXFA_Node* pValueChild = pNodeValue->GetNodeItem(XFA_NODEITEM_FirstChild);
  if (!pValueChild) {
    return CXFA_LocaleValue();
  }
  int32_t iVTType = XFA_VT_NULL;
  switch (pValueChild->GetElementType()) {
    case XFA_Element::Decimal:
      iVTType = XFA_VT_DECIMAL;
      break;
    case XFA_Element::Float:
      iVTType = XFA_VT_FLOAT;
      break;
    case XFA_Element::Date:
      iVTType = XFA_VT_DATE;
      break;
    case XFA_Element::Time:
      iVTType = XFA_VT_TIME;
      break;
    case XFA_Element::DateTime:
      iVTType = XFA_VT_DATETIME;
      break;
    case XFA_Element::Boolean:
      iVTType = XFA_VT_BOOLEAN;
      break;
    case XFA_Element::Integer:
      iVTType = XFA_VT_INTEGER;
      break;
    case XFA_Element::Text:
      iVTType = XFA_VT_TEXT;
      break;
    default:
      iVTType = XFA_VT_NULL;
      break;
  }
  return CXFA_LocaleValue(iVTType, pWidgetData->GetRawValue(),
                          pWidgetData->GetNode()->GetDocument()->GetLocalMgr());
}
void XFA_GetPlainTextFromRichText(CFX_XMLNode* pXMLNode,
                                  WideString& wsPlainText) {
  if (!pXMLNode) {
    return;
  }
  switch (pXMLNode->GetType()) {
    case FX_XMLNODE_Element: {
      CFX_XMLElement* pXMLElement = static_cast<CFX_XMLElement*>(pXMLNode);
      WideString wsTag = pXMLElement->GetLocalTagName();
      uint32_t uTag = FX_HashCode_GetW(wsTag.AsStringView(), true);
      if (uTag == 0x0001f714) {
        wsPlainText += L"\n";
      } else if (uTag == 0x00000070) {
        if (!wsPlainText.IsEmpty()) {
          wsPlainText += L"\n";
        }
      } else if (uTag == 0xa48ac63) {
        if (!wsPlainText.IsEmpty() &&
            wsPlainText[wsPlainText.GetLength() - 1] != '\n') {
          wsPlainText += L"\n";
        }
      }
      break;
    }
    case FX_XMLNODE_Text:
    case FX_XMLNODE_CharData: {
      WideString wsContent = static_cast<CFX_XMLText*>(pXMLNode)->GetText();
      wsPlainText += wsContent;
      break;
    }
    default:
      break;
  }
  for (CFX_XMLNode* pChildXML = pXMLNode->GetNodeItem(CFX_XMLNode::FirstChild);
       pChildXML;
       pChildXML = pChildXML->GetNodeItem(CFX_XMLNode::NextSibling)) {
    XFA_GetPlainTextFromRichText(pChildXML, wsPlainText);
  }
}

bool XFA_FieldIsMultiListBox(CXFA_Node* pFieldNode) {
  bool bRet = false;
  if (!pFieldNode)
    return bRet;

  CXFA_Node* pUIChild = pFieldNode->GetChild(0, XFA_Element::Ui, false);
  if (pUIChild) {
    CXFA_Node* pFirstChild = pUIChild->GetNodeItem(XFA_NODEITEM_FirstChild);
    if (pFirstChild &&
        pFirstChild->GetElementType() == XFA_Element::ChoiceList) {
      bRet = pFirstChild->JSNode()->GetEnum(XFA_Attribute::Open) ==
             XFA_ATTRIBUTEENUM_MultiSelect;
    }
  }
  return bRet;
}

int32_t XFA_MapRotation(int32_t nRotation) {
  nRotation = nRotation % 360;
  nRotation = nRotation < 0 ? nRotation + 360 : nRotation;
  return nRotation;
}

const XFA_SCRIPTATTRIBUTEINFO* XFA_GetScriptAttributeByName(
    XFA_Element eElement,
    const WideStringView& wsAttributeName) {
  if (wsAttributeName.IsEmpty())
    return nullptr;

  int32_t iElementIndex = static_cast<int32_t>(eElement);
  while (iElementIndex != -1) {
    const XFA_SCRIPTHIERARCHY* scriptIndex = g_XFAScriptIndex + iElementIndex;
    int32_t icount = scriptIndex->wAttributeCount;
    if (icount == 0) {
      iElementIndex = scriptIndex->wParentIndex;
      continue;
    }
    uint32_t uHash = FX_HashCode_GetW(wsAttributeName, false);
    int32_t iStart = scriptIndex->wAttributeStart, iEnd = iStart + icount - 1;
    do {
      int32_t iMid = (iStart + iEnd) / 2;
      const XFA_SCRIPTATTRIBUTEINFO* pInfo = g_SomAttributeData + iMid;
      if (uHash == pInfo->uHash)
        return pInfo;
      if (uHash < pInfo->uHash)
        iEnd = iMid - 1;
      else
        iStart = iMid + 1;
    } while (iStart <= iEnd);
    iElementIndex = scriptIndex->wParentIndex;
  }
  return nullptr;
}

const XFA_NOTSUREATTRIBUTE* XFA_GetNotsureAttribute(XFA_Element eElement,
                                                    XFA_Attribute eAttribute,
                                                    XFA_AttributeType eType) {
  int32_t iStart = 0, iEnd = g_iXFANotsureCount - 1;
  do {
    int32_t iMid = (iStart + iEnd) / 2;
    const XFA_NOTSUREATTRIBUTE* pAttr = g_XFANotsureAttributes + iMid;
    if (eElement == pAttr->eElement) {
      if (pAttr->attribute == eAttribute) {
        if (eType == XFA_AttributeType::NotSure || eType == pAttr->eType)
          return pAttr;
        return nullptr;
      }
      int32_t iBefore = iMid - 1;
      if (iBefore >= 0) {
        pAttr = g_XFANotsureAttributes + iBefore;
        while (eElement == pAttr->eElement) {
          if (pAttr->attribute == eAttribute) {
            if (eType == XFA_AttributeType::NotSure || eType == pAttr->eType)
              return pAttr;
            return nullptr;
          }
          iBefore--;
          if (iBefore < 0)
            break;

          pAttr = g_XFANotsureAttributes + iBefore;
        }
      }

      int32_t iAfter = iMid + 1;
      if (iAfter <= g_iXFANotsureCount - 1) {
        pAttr = g_XFANotsureAttributes + iAfter;
        while (eElement == pAttr->eElement) {
          if (pAttr->attribute == eAttribute) {
            if (eType == XFA_AttributeType::NotSure || eType == pAttr->eType)
              return pAttr;
            return nullptr;
          }
          iAfter++;
          if (iAfter > g_iXFANotsureCount - 1)
            break;

          pAttr = g_XFANotsureAttributes + iAfter;
        }
      }
      return nullptr;
    }

    if (eElement < pAttr->eElement)
      iEnd = iMid - 1;
    else
      iStart = iMid + 1;
  } while (iStart <= iEnd);
  return nullptr;
}

const XFA_PROPERTY* XFA_GetPropertyOfElement(XFA_Element eElement,
                                             XFA_Element eProperty,
                                             uint32_t dwPacket) {
  int32_t iCount = 0;
  const XFA_PROPERTY* pProperties = XFA_GetElementProperties(eElement, iCount);
  if (!pProperties || iCount < 1)
    return nullptr;

  auto* it = std::find_if(pProperties, pProperties + iCount,
                          [eProperty](const XFA_PROPERTY& prop) {
                            return prop.eName == eProperty;
                          });
  if (it == pProperties + iCount)
    return nullptr;

  const XFA_ELEMENTINFO* pInfo = XFA_GetElementByID(eProperty);
  ASSERT(pInfo);
  if (dwPacket != XFA_XDPPACKET_UNKNOWN && !(dwPacket & pInfo->dwPackets))
    return nullptr;
  return it;
}

const XFA_PROPERTY* XFA_GetElementProperties(XFA_Element eElement,
                                             int32_t& iCount) {
  if (eElement == XFA_Element::Unknown)
    return nullptr;

  const XFA_ELEMENTHIERARCHY* pElement =
      g_XFAElementPropertyIndex + static_cast<int32_t>(eElement);
  iCount = pElement->wCount;
  return g_XFAElementPropertyData + pElement->wStart;
}

const XFA_Attribute* XFA_GetElementAttributes(XFA_Element eElement,
                                              int32_t& iCount) {
  if (eElement == XFA_Element::Unknown)
    return nullptr;

  const XFA_ELEMENTHIERARCHY* pElement =
      g_XFAElementAttributeIndex + static_cast<int32_t>(eElement);
  iCount = pElement->wCount;
  return g_XFAElementAttributeData + pElement->wStart;
}

const XFA_ELEMENTINFO* XFA_GetElementByID(XFA_Element eName) {
  return eName != XFA_Element::Unknown
             ? g_XFAElementData + static_cast<int32_t>(eName)
             : nullptr;
}

XFA_Element XFA_GetElementTypeForName(const WideStringView& wsName) {
  if (wsName.IsEmpty())
    return XFA_Element::Unknown;

  uint32_t uHash = FX_HashCode_GetW(wsName, false);
  const XFA_ELEMENTINFO* pEnd = g_XFAElementData + g_iXFAElementCount;
  auto* pInfo =
      std::lower_bound(g_XFAElementData, pEnd, uHash,
                       [](const XFA_ELEMENTINFO& info, uint32_t hash) {
                         return info.uHash < hash;
                       });
  if (pInfo < pEnd && pInfo->uHash == uHash)
    return pInfo->eName;
  return XFA_Element::Unknown;
}

bool XFA_GetAttributeDefaultValue(void*& pValue,
                                  XFA_Element eElement,
                                  XFA_Attribute eAttribute,
                                  XFA_AttributeType eType,
                                  uint32_t dwPacket) {
  const XFA_ATTRIBUTEINFO* pInfo = XFA_GetAttributeByID(eAttribute);
  if (!pInfo)
    return false;
  if (dwPacket && (dwPacket & pInfo->dwPackets) == 0)
    return false;
  if (pInfo->eType == eType) {
    pValue = pInfo->pDefValue;
    return true;
  }
  if (pInfo->eType == XFA_AttributeType::NotSure) {
    const XFA_NOTSUREATTRIBUTE* pAttr =
        XFA_GetNotsureAttribute(eElement, eAttribute, eType);
    if (pAttr) {
      pValue = pAttr->pValue;
      return true;
    }
  }
  return false;
}

const XFA_ATTRIBUTEINFO* XFA_GetAttributeByName(const WideStringView& wsName) {
  if (wsName.IsEmpty())
    return nullptr;

  auto* it = std::lower_bound(g_XFAAttributeData,
                              g_XFAAttributeData + g_iXFAAttributeCount,
                              FX_HashCode_GetW(wsName, false),
                              [](const XFA_ATTRIBUTEINFO& arg, uint32_t hash) {
                                return arg.uHash < hash;
                              });
  if (it != g_XFAAttributeData + g_iXFAAttributeCount && wsName == it->pName)
    return it;
  return nullptr;
}

const XFA_ATTRIBUTEINFO* XFA_GetAttributeByID(XFA_Attribute eName) {
  return (static_cast<uint8_t>(eName) < g_iXFAAttributeCount)
             ? (g_XFAAttributeData + static_cast<uint8_t>(eName))
             : nullptr;
}

const XFA_ATTRIBUTEENUMINFO* XFA_GetAttributeEnumByName(
    const WideStringView& wsName) {
  if (wsName.IsEmpty())
    return nullptr;

  auto* it = std::lower_bound(g_XFAEnumData, g_XFAEnumData + g_iXFAEnumCount,
                              FX_HashCode_GetW(wsName, false),
                              [](const XFA_ATTRIBUTEENUMINFO& arg,
                                 uint32_t hash) { return arg.uHash < hash; });
  if (it != g_XFAEnumData + g_iXFAEnumCount && wsName == it->pName)
    return it;
  return nullptr;
}

const XFA_PACKETINFO* XFA_GetPacketByIndex(XFA_PacketType ePacket) {
  return g_XFAPacketData + static_cast<uint8_t>(ePacket);
}

const XFA_PACKETINFO* XFA_GetPacketByID(uint32_t dwPacket) {
  int32_t iStart = 0, iEnd = g_iXFAPacketCount - 1;
  do {
    int32_t iMid = (iStart + iEnd) / 2;
    uint32_t dwFind = (g_XFAPacketData + iMid)->eName;
    if (dwPacket == dwFind)
      return g_XFAPacketData + iMid;
    if (dwPacket < dwFind)
      iEnd = iMid - 1;
    else
      iStart = iMid + 1;
  } while (iStart <= iEnd);
  return nullptr;
}
