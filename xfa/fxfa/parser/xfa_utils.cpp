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
             XFA_AttributeEnum::MultiSelect;
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
