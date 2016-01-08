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
CXFA_Node* XFA_CreateUIChild(CXFA_Node* pNode, XFA_ELEMENT& eWidgetType) {
  XFA_ELEMENT eType = pNode->GetClassID();
  eWidgetType = eType;
  if (eType != XFA_ELEMENT_Field && eType != XFA_ELEMENT_Draw) {
    return NULL;
  }
  eWidgetType = XFA_ELEMENT_UNKNOWN;
  XFA_ELEMENT eUIType = XFA_ELEMENT_UNKNOWN;
  CXFA_Value defValue = pNode->GetProperty(0, XFA_ELEMENT_Value, TRUE);
  XFA_ELEMENT eValueType = (XFA_ELEMENT)defValue.GetChildValueClassID();
  switch (eValueType) {
    case XFA_ELEMENT_Boolean:
      eUIType = XFA_ELEMENT_CheckButton;
      break;
    case XFA_ELEMENT_Integer:
    case XFA_ELEMENT_Decimal:
    case XFA_ELEMENT_Float:
      eUIType = XFA_ELEMENT_NumericEdit;
      break;
    case XFA_ELEMENT_ExData:
    case XFA_ELEMENT_Text:
      eUIType = XFA_ELEMENT_TextEdit;
      eWidgetType = XFA_ELEMENT_Text;
      break;
    case XFA_ELEMENT_Date:
    case XFA_ELEMENT_Time:
    case XFA_ELEMENT_DateTime:
      eUIType = XFA_ELEMENT_DateTimeEdit;
      break;
    case XFA_ELEMENT_Image:
      eUIType = XFA_ELEMENT_ImageEdit;
      eWidgetType = XFA_ELEMENT_Image;
      break;
      ;
    case XFA_ELEMENT_Arc:
    case XFA_ELEMENT_Line:
    case XFA_ELEMENT_Rectangle:
      eUIType = XFA_ELEMENT_DefaultUi;
      eWidgetType = eValueType;
      break;
    default:
      break;
  }
  CXFA_Node* pUIChild = NULL;
  CXFA_Node* pUI = pNode->GetProperty(0, XFA_ELEMENT_Ui, TRUE);
  CXFA_Node* pChild = pUI->GetNodeItem(XFA_NODEITEM_FirstChild);
  for (; pChild; pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    XFA_ELEMENT eChild = pChild->GetClassID();
    if (eChild == XFA_ELEMENT_Extras || eChild == XFA_ELEMENT_Picture) {
      continue;
    }
    XFA_LPCPROPERTY pProterty =
        XFA_GetPropertyOfElement(XFA_ELEMENT_Ui, eChild, XFA_XDPPACKET_Form);
    if (pProterty && (pProterty->uFlags & XFA_PROPERTYFLAG_OneOf)) {
      pUIChild = pChild;
      break;
    }
  }
  if (eType == XFA_ELEMENT_Draw) {
    XFA_ELEMENT eDraw = pUIChild ? pUIChild->GetClassID() : XFA_ELEMENT_UNKNOWN;
    switch (eDraw) {
      case XFA_ELEMENT_TextEdit:
        eWidgetType = XFA_ELEMENT_Text;
        break;
      case XFA_ELEMENT_ImageEdit:
        eWidgetType = XFA_ELEMENT_Image;
        break;
      default:
        eWidgetType =
            eWidgetType == XFA_ELEMENT_UNKNOWN ? XFA_ELEMENT_Text : eWidgetType;
        break;
    }
  } else {
    if (pUIChild && pUIChild->GetClassID() == XFA_ELEMENT_DefaultUi) {
      eWidgetType = XFA_ELEMENT_TextEdit;
    } else {
      eWidgetType = pUIChild
                        ? pUIChild->GetClassID()
                        : (eUIType == XFA_ELEMENT_UNKNOWN ? XFA_ELEMENT_TextEdit
                                                          : eUIType);
    }
  }
  if (!pUIChild) {
    if (eUIType == XFA_ELEMENT_UNKNOWN) {
      eUIType = XFA_ELEMENT_TextEdit;
      ((CXFA_Node*)defValue)->GetProperty(0, XFA_ELEMENT_Text, TRUE);
    }
    pUIChild = pUI->GetProperty(0, eUIType, TRUE);
  } else if (eUIType == XFA_ELEMENT_UNKNOWN) {
    switch (pUIChild->GetClassID()) {
      case XFA_ELEMENT_CheckButton: {
        eValueType = XFA_ELEMENT_Text;
        if (CXFA_Node* pItems = pNode->GetChild(0, XFA_ELEMENT_Items)) {
          if (CXFA_Node* pItem = pItems->GetChild(0, XFA_ELEMENT_UNKNOWN)) {
            eValueType = pItem->GetClassID();
          }
        }
      } break;
      case XFA_ELEMENT_DateTimeEdit:
        eValueType = XFA_ELEMENT_DateTime;
        break;
      case XFA_ELEMENT_ImageEdit:
        eValueType = XFA_ELEMENT_Image;
        break;
      case XFA_ELEMENT_NumericEdit:
        eValueType = XFA_ELEMENT_Float;
        break;
      case XFA_ELEMENT_ChoiceList: {
        eValueType = (pUIChild->GetEnum(XFA_ATTRIBUTE_Open) ==
                      XFA_ATTRIBUTEENUM_MultiSelect)
                         ? XFA_ELEMENT_ExData
                         : XFA_ELEMENT_Text;
      } break;
      case XFA_ELEMENT_Barcode:
      case XFA_ELEMENT_Button:
      case XFA_ELEMENT_PasswordEdit:
      case XFA_ELEMENT_Signature:
      case XFA_ELEMENT_TextEdit:
      default:
        eValueType = XFA_ELEMENT_Text;
        break;
    }
    ((CXFA_Node*)defValue)->GetProperty(0, eValueType, TRUE);
  }
  return pUIChild;
}
CXFA_LocaleValue XFA_GetLocaleValue(CXFA_WidgetData* pWidgetData) {
  CXFA_Node* pNodeValue =
      pWidgetData->GetNode()->GetChild(0, XFA_ELEMENT_Value);
  if (!pNodeValue) {
    return CXFA_LocaleValue();
  }
  CXFA_Node* pValueChild = pNodeValue->GetNodeItem(XFA_NODEITEM_FirstChild);
  if (!pValueChild) {
    return CXFA_LocaleValue();
  }
  int32_t iVTType = XFA_VT_NULL;
  XFA_ELEMENT eType = pValueChild->GetClassID();
  switch (eType) {
    case XFA_ELEMENT_Decimal:
      iVTType = XFA_VT_DECIMAL;
      break;
    case XFA_ELEMENT_Float:
      iVTType = XFA_VT_FLOAT;
      break;
    case XFA_ELEMENT_Date:
      iVTType = XFA_VT_DATE;
      break;
    case XFA_ELEMENT_Time:
      iVTType = XFA_VT_TIME;
      break;
    case XFA_ELEMENT_DateTime:
      iVTType = XFA_VT_DATETIME;
      break;
    case XFA_ELEMENT_Boolean:
      iVTType = XFA_VT_BOOLEAN;
      break;
    case XFA_ELEMENT_Integer:
      iVTType = XFA_VT_INTEGER;
      break;
    case XFA_ELEMENT_Text:
      iVTType = XFA_VT_TEXT;
      break;
    default:
      iVTType = XFA_VT_NULL;
      break;
  }
  return CXFA_LocaleValue(iVTType, pWidgetData->GetRawValue(),
                          pWidgetData->GetNode()->GetDocument()->GetLocalMgr());
}
void XFA_GetPlainTextFromRichText(IFDE_XMLNode* pXMLNode,
                                  CFX_WideString& wsPlainText) {
  if (pXMLNode == NULL) {
    return;
  }
  switch (pXMLNode->GetType()) {
    case FDE_XMLNODE_Element: {
      IFDE_XMLElement* pXMLElement = (IFDE_XMLElement*)pXMLNode;
      CFX_WideString wsTag;
      pXMLElement->GetLocalTagName(wsTag);
      uint32_t uTag = FX_HashCode_String_GetW(wsTag, wsTag.GetLength(), TRUE);
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
    } break;
    case FDE_XMLNODE_Text: {
      CFX_WideString wsContent;
      ((IFDE_XMLText*)pXMLNode)->GetText(wsContent);
      wsPlainText += wsContent;
    } break;
    case FDE_XMLNODE_CharData: {
      CFX_WideString wsCharData;
      ((IFDE_XMLCharData*)pXMLNode)->GetCharData(wsCharData);
      wsPlainText += wsCharData;
    } break;
    default:
      break;
  }
  for (IFDE_XMLNode* pChildXML =
           pXMLNode->GetNodeItem(IFDE_XMLNode::FirstChild);
       pChildXML;
       pChildXML = pChildXML->GetNodeItem(IFDE_XMLNode::NextSibling)) {
    XFA_GetPlainTextFromRichText(pChildXML, wsPlainText);
  }
}
FX_BOOL XFA_FieldIsMultiListBox(CXFA_Node* pFieldNode) {
  FX_BOOL bRet = FALSE;
  if (!pFieldNode) {
    return bRet;
  }
  CXFA_Node* pUIChild = pFieldNode->GetChild(0, XFA_ELEMENT_Ui);
  if (pUIChild) {
    CXFA_Node* pFirstChild = pUIChild->GetNodeItem(XFA_NODEITEM_FirstChild);
    if (pFirstChild && pFirstChild->GetClassID() == XFA_ELEMENT_ChoiceList) {
      bRet = pFirstChild->GetEnum(XFA_ATTRIBUTE_Open) ==
             XFA_ATTRIBUTEENUM_MultiSelect;
    }
  }
  return bRet;
}
FX_BOOL XFA_IsLayoutElement(XFA_ELEMENT eElement, FX_BOOL bLayoutContainer) {
  switch (eElement) {
    case XFA_ELEMENT_Draw:
    case XFA_ELEMENT_Field:
    case XFA_ELEMENT_InstanceManager:
      return !bLayoutContainer;
    case XFA_ELEMENT_Area:
    case XFA_ELEMENT_Subform:
    case XFA_ELEMENT_ExclGroup:
    case XFA_ELEMENT_SubformSet:
      return TRUE;
    case XFA_ELEMENT_PageArea:
    case XFA_ELEMENT_Form:
      return TRUE;
    default:
      return FALSE;
  }
  return FALSE;
}
FX_BOOL XFA_IsTakingupSpace(XFA_ATTRIBUTEENUM ePresence) {
  switch (ePresence) {
    case XFA_ATTRIBUTEENUM_Visible:
    case XFA_ATTRIBUTEENUM_Invisible:
      return TRUE;
    default:
      return FALSE;
  }
  return FALSE;
}
FX_BOOL XFA_IsFlowingLayout(XFA_ATTRIBUTEENUM eLayout) {
  switch (eLayout) {
    case XFA_ATTRIBUTEENUM_Tb:
    case XFA_ATTRIBUTEENUM_Lr_tb:
    case XFA_ATTRIBUTEENUM_Rl_tb:
      return TRUE;
    default:
      return FALSE;
  }
  return FALSE;
}
FX_BOOL XFA_IsHorizontalFlow(XFA_ATTRIBUTEENUM eLayout) {
  switch (eLayout) {
    case XFA_ATTRIBUTEENUM_Lr_tb:
    case XFA_ATTRIBUTEENUM_Rl_tb:
      return TRUE;
    default:
      return FALSE;
  }
  return FALSE;
}
static const FX_DOUBLE fraction_scales[] = {0.1,
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
FX_DOUBLE XFA_WideStringToDouble(const CFX_WideString& wsStringVal) {
  CFX_WideString wsValue = wsStringVal;
  wsValue.TrimLeft();
  wsValue.TrimRight();
  int64_t nIntegral = 0;
  FX_DWORD dwFractional = 0;
  int32_t nExponent = 0;
  int32_t cc = 0;
  FX_BOOL bNegative = FALSE, bExpSign = FALSE;
  const FX_WCHAR* str = (const FX_WCHAR*)wsValue;
  int32_t len = wsValue.GetLength();
  if (str[0] == '+') {
    cc++;
  } else if (str[0] == '-') {
    bNegative = TRUE;
    cc++;
  }
  int32_t nIntegralLen = 0;
  while (cc < len) {
    if (str[cc] == '.' || str[cc] == 'E' || str[cc] == 'e' ||
        nIntegralLen > 17) {
      break;
    }
    if (!XFA_IsDigit(str[cc])) {
      return 0;
    }
    nIntegral = nIntegral * 10 + str[cc] - '0';
    cc++;
    nIntegralLen++;
  }
  nIntegral = bNegative ? -nIntegral : nIntegral;
  int32_t scale = 0;
  FX_DOUBLE fraction = 0.0;
  if (cc < len && str[cc] == '.') {
    cc++;
    while (cc < len) {
      fraction += fraction_scales[scale] * (str[cc] - '0');
      scale++;
      cc++;
      if (cc == len) {
        break;
      }
      if (scale == sizeof(fraction_scales) / sizeof(FX_DOUBLE) ||
          str[cc] == 'E' || str[cc] == 'e') {
        break;
      }
      if (!XFA_IsDigit(str[cc])) {
        return 0;
      }
    }
    dwFractional = (FX_DWORD)(fraction * 4294967296.0);
  }
  if (cc < len && (str[cc] == 'E' || str[cc] == 'e')) {
    cc++;
    if (cc < len) {
      if (str[cc] == '+') {
        cc++;
      } else if (str[cc] == '-') {
        bExpSign = TRUE;
        cc++;
      }
    }
    while (cc < len) {
      if (str[cc] == '.' || !XFA_IsDigit(str[cc])) {
        return 0;
      }
      nExponent = nExponent * 10 + str[cc] - '0';
      cc++;
    }
    nExponent = bExpSign ? -nExponent : nExponent;
  }
  FX_DOUBLE dValue = (dwFractional / 4294967296.0);
  dValue = nIntegral + (nIntegral >= 0 ? dValue : -dValue);
  if (nExponent != 0) {
    dValue *= FXSYS_pow(10, (FX_FLOAT)nExponent);
  }
  return dValue;
}

FX_DOUBLE XFA_ByteStringToDouble(const CFX_ByteStringC& szStringVal) {
  CFX_WideString wsValue =
      CFX_WideString::FromUTF8(szStringVal.GetCStr(), szStringVal.GetLength());
  return XFA_WideStringToDouble(wsValue);
}

int32_t XFA_MapRotation(int32_t nRotation) {
  nRotation = nRotation % 360;
  nRotation = nRotation < 0 ? nRotation + 360 : nRotation;
  return nRotation;
}
