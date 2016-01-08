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
static FX_ARGB XFA_WStringToColor(const CFX_WideStringC& wsValue) {
  uint8_t r = 0, g = 0, b = 0;
  if (wsValue.GetLength() == 0) {
    return 0xff000000;
  }
  int cc = 0;
  const FX_WCHAR* str = wsValue.GetPtr();
  int len = wsValue.GetLength();
  while (XFA_IsSpace(str[cc]) && cc < len) {
    cc++;
  }
  if (cc >= len) {
    return 0xff000000;
  }
  while (cc < len) {
    if (str[cc] == ',' || !XFA_IsDigit(str[cc])) {
      break;
    }
    r = r * 10 + str[cc] - '0';
    cc++;
  }
  if (cc < len && str[cc] == ',') {
    cc++;
    while (XFA_IsSpace(str[cc]) && cc < len) {
      cc++;
    }
    while (cc < len) {
      if (str[cc] == ',' || !XFA_IsDigit(str[cc])) {
        break;
      }
      g = g * 10 + str[cc] - '0';
      cc++;
    }
    if (cc < len && str[cc] == ',') {
      cc++;
      while (XFA_IsSpace(str[cc]) && cc < len) {
        cc++;
      }
      while (cc < len) {
        if (str[cc] == ',' || !XFA_IsDigit(str[cc])) {
          break;
        }
        b = b * 10 + str[cc] - '0';
        cc++;
      }
    }
  }
  return (0xff << 24) | (r << 16) | (g << 8) | b;
}
XFA_ELEMENT CXFA_Data::GetClassID() const {
  return m_pNode ? m_pNode->GetClassID() : XFA_ELEMENT_UNKNOWN;
}
FX_BOOL CXFA_Data::TryMeasure(XFA_ATTRIBUTE eAttr,
                              FX_FLOAT& fValue,
                              FX_BOOL bUseDefault) const {
  CXFA_Measurement ms;
  if (m_pNode->TryMeasure(eAttr, ms, bUseDefault)) {
    fValue = ms.ToUnit(XFA_UNIT_Pt);
    return TRUE;
  }
  return FALSE;
}
FX_BOOL CXFA_Data::SetMeasure(XFA_ATTRIBUTE eAttr, FX_FLOAT fValue) {
  CXFA_Measurement ms(fValue, XFA_UNIT_Pt);
  return m_pNode->SetMeasure(eAttr, ms);
}
CXFA_Fill::CXFA_Fill(CXFA_Node* pNode) : CXFA_Data(pNode) {}
CXFA_Fill::~CXFA_Fill() {}
int32_t CXFA_Fill::GetPresence() {
  return m_pNode->GetEnum(XFA_ATTRIBUTE_Presence);
}
void CXFA_Fill::SetColor(FX_ARGB color) {
  CXFA_Node* pNode = m_pNode->GetProperty(0, XFA_ELEMENT_Color);
  CFX_WideString wsColor;
  int a, r, g, b;
  ArgbDecode(color, a, r, g, b);
  wsColor.Format(L"%d,%d,%d", r, g, b);
  pNode->SetCData(XFA_ATTRIBUTE_Value, wsColor);
}
FX_ARGB CXFA_Fill::GetColor(FX_BOOL bText) {
  if (CXFA_Node* pNode = m_pNode->GetChild(0, XFA_ELEMENT_Color)) {
    CFX_WideStringC wsColor;
    if (pNode->TryCData(XFA_ATTRIBUTE_Value, wsColor, FALSE)) {
      return XFA_WStringToColor(wsColor);
    }
  }
  if (bText) {
    return 0xFF000000;
  }
  return 0xFFFFFFFF;
}
int32_t CXFA_Fill::GetFillType() {
  CXFA_Node* pChild = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  while (pChild) {
    int32_t eType = pChild->GetClassID();
    if (eType != XFA_ELEMENT_Color && eType != XFA_ELEMENT_Extras) {
      return eType;
    }
    pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling);
  }
  return XFA_ELEMENT_Solid;
}
int32_t CXFA_Fill::GetPattern(FX_ARGB& foreColor) {
  CXFA_Node* pNode = m_pNode->GetProperty(0, XFA_ELEMENT_Pattern);
  if (CXFA_Node* pColor = pNode->GetChild(0, XFA_ELEMENT_Color)) {
    CFX_WideStringC wsColor;
    pColor->TryCData(XFA_ATTRIBUTE_Value, wsColor, FALSE);
    foreColor = XFA_WStringToColor(wsColor);
  } else {
    foreColor = 0xFF000000;
  }
  return pNode->GetEnum(XFA_ATTRIBUTE_Type);
}
int32_t CXFA_Fill::GetStipple(FX_ARGB& stippleColor) {
  CXFA_Node* pNode = m_pNode->GetProperty(0, XFA_ELEMENT_Stipple);
  int32_t eAttr = 50;
  pNode->TryInteger(XFA_ATTRIBUTE_Rate, eAttr);
  if (CXFA_Node* pColor = pNode->GetChild(0, XFA_ELEMENT_Color)) {
    CFX_WideStringC wsColor;
    pColor->TryCData(XFA_ATTRIBUTE_Value, wsColor, FALSE);
    stippleColor = XFA_WStringToColor(wsColor);
  } else {
    stippleColor = 0xFF000000;
  }
  return eAttr;
}
int32_t CXFA_Fill::GetLinear(FX_ARGB& endColor) {
  CXFA_Node* pNode = m_pNode->GetProperty(0, XFA_ELEMENT_Linear);
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_ToRight;
  pNode->TryEnum(XFA_ATTRIBUTE_Type, eAttr);
  if (CXFA_Node* pColor = pNode->GetChild(0, XFA_ELEMENT_Color)) {
    CFX_WideStringC wsColor;
    pColor->TryCData(XFA_ATTRIBUTE_Value, wsColor, FALSE);
    endColor = XFA_WStringToColor(wsColor);
  } else {
    endColor = 0xFF000000;
  }
  return eAttr;
}
int32_t CXFA_Fill::GetRadial(FX_ARGB& endColor) {
  CXFA_Node* pNode = m_pNode->GetProperty(0, XFA_ELEMENT_Radial);
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_ToEdge;
  pNode->TryEnum(XFA_ATTRIBUTE_Type, eAttr);
  if (CXFA_Node* pColor = pNode->GetChild(0, XFA_ELEMENT_Color)) {
    CFX_WideStringC wsColor;
    pColor->TryCData(XFA_ATTRIBUTE_Value, wsColor, FALSE);
    endColor = XFA_WStringToColor(wsColor);
  } else {
    endColor = 0xFF000000;
  }
  return eAttr;
}
FX_BOOL CXFA_Fill::SetPresence(int32_t iPresence) {
  return m_pNode->SetEnum(XFA_ATTRIBUTE_Presence, (XFA_ATTRIBUTEENUM)iPresence);
}
FX_BOOL CXFA_Fill::SetFillType(int32_t iType) {
  return FALSE;
}
FX_BOOL CXFA_Fill::SetPattern(int32_t iPattern, FX_ARGB foreColor) {
  CXFA_Node* pNode = m_pNode->GetProperty(0, XFA_ELEMENT_Pattern);
  CXFA_Node* pColor = pNode->GetProperty(0, XFA_ELEMENT_Color);
  CFX_WideString wsColor;
  int a, r, g, b;
  ArgbDecode(foreColor, a, r, g, b);
  wsColor.Format(L"%d,%d,%d", r, g, b);
  pColor->SetCData(XFA_ATTRIBUTE_Value, wsColor);
  return pNode->SetEnum(XFA_ATTRIBUTE_Type, (XFA_ATTRIBUTEENUM)iPattern);
}
FX_BOOL CXFA_Fill::SetStipple(int32_t iStipple, FX_ARGB stippleColor) {
  CXFA_Node* pNode = m_pNode->GetProperty(0, XFA_ELEMENT_Stipple);
  CXFA_Node* pColor = pNode->GetProperty(0, XFA_ELEMENT_Color);
  CFX_WideString wsColor;
  int a, r, g, b;
  ArgbDecode(stippleColor, a, r, g, b);
  wsColor.Format(L"%d,%d,%d", r, g, b);
  pColor->SetCData(XFA_ATTRIBUTE_Value, wsColor);
  return pNode->SetEnum(XFA_ATTRIBUTE_Rate, (XFA_ATTRIBUTEENUM)iStipple);
}
FX_BOOL CXFA_Fill::SetLinear(int32_t iLinear, FX_ARGB endColor) {
  CXFA_Node* pNode = m_pNode->GetProperty(0, XFA_ELEMENT_Linear);
  CXFA_Node* pColor = pNode->GetProperty(0, XFA_ELEMENT_Color);
  CFX_WideString wsColor;
  int a, r, g, b;
  ArgbDecode(endColor, a, r, g, b);
  wsColor.Format(L"%d,%d,%d", r, g, b);
  pColor->SetCData(XFA_ATTRIBUTE_Value, wsColor);
  return pNode->SetEnum(XFA_ATTRIBUTE_Type, (XFA_ATTRIBUTEENUM)iLinear);
}
FX_BOOL CXFA_Fill::SetRadial(int32_t iRadial, FX_ARGB endColor) {
  CXFA_Node* pNode = m_pNode->GetProperty(0, XFA_ELEMENT_Radial);
  CXFA_Node* pColor = pNode->GetProperty(0, XFA_ELEMENT_Color);
  CFX_WideString wsColor;
  int a, r, g, b;
  ArgbDecode(endColor, a, r, g, b);
  wsColor.Format(L"%d,%d,%d", r, g, b);
  pColor->SetCData(XFA_ATTRIBUTE_Value, wsColor);
  return pNode->SetEnum(XFA_ATTRIBUTE_Type, (XFA_ATTRIBUTEENUM)iRadial);
}
CXFA_Margin::CXFA_Margin(CXFA_Node* pNode) : CXFA_Data(pNode) {}
FX_BOOL CXFA_Margin::GetLeftInset(FX_FLOAT& fInset, FX_FLOAT fDefInset) const {
  fInset = fDefInset;
  return TryMeasure(XFA_ATTRIBUTE_LeftInset, fInset);
}
FX_BOOL CXFA_Margin::GetTopInset(FX_FLOAT& fInset, FX_FLOAT fDefInset) const {
  fInset = fDefInset;
  return TryMeasure(XFA_ATTRIBUTE_TopInset, fInset);
}
FX_BOOL CXFA_Margin::GetRightInset(FX_FLOAT& fInset, FX_FLOAT fDefInset) const {
  fInset = fDefInset;
  return TryMeasure(XFA_ATTRIBUTE_RightInset, fInset);
}
FX_BOOL CXFA_Margin::GetBottomInset(FX_FLOAT& fInset,
                                    FX_FLOAT fDefInset) const {
  fInset = fDefInset;
  return TryMeasure(XFA_ATTRIBUTE_BottomInset, fInset);
}
FX_BOOL CXFA_Margin::SetLeftInset(FX_FLOAT fInset) {
  return SetMeasure(XFA_ATTRIBUTE_LeftInset, fInset);
}
FX_BOOL CXFA_Margin::SetTopInset(FX_FLOAT fInset) {
  return SetMeasure(XFA_ATTRIBUTE_TopInset, fInset);
}
FX_BOOL CXFA_Margin::SetRightInset(FX_FLOAT fInset) {
  return SetMeasure(XFA_ATTRIBUTE_RightInset, fInset);
}
FX_BOOL CXFA_Margin::SetBottomInset(FX_FLOAT fInset) {
  return SetMeasure(XFA_ATTRIBUTE_BottomInset, fInset);
}
CXFA_Font::CXFA_Font(CXFA_Node* pNode) : CXFA_Data(pNode) {}
FX_FLOAT CXFA_Font::GetBaselineShift() {
  return m_pNode->GetMeasure(XFA_ATTRIBUTE_BaselineShift).ToUnit(XFA_UNIT_Pt);
}
FX_FLOAT CXFA_Font::GetHorizontalScale() {
  CFX_WideString wsValue;
  m_pNode->TryCData(XFA_ATTRIBUTE_FontHorizontalScale, wsValue);
  int32_t iScale = FXSYS_wtoi((const FX_WCHAR*)wsValue);
  return iScale > 0 ? (FX_FLOAT)iScale : 100.0f;
}
FX_FLOAT CXFA_Font::GetVerticalScale() {
  CFX_WideString wsValue;
  m_pNode->TryCData(XFA_ATTRIBUTE_FontVerticalScale, wsValue);
  int32_t iScale = FXSYS_wtoi((const FX_WCHAR*)wsValue);
  return iScale > 0 ? (FX_FLOAT)iScale : 100.0f;
}
FX_FLOAT CXFA_Font::GetLetterSpacing() {
  CFX_WideStringC wsValue;
  if (!m_pNode->TryCData(XFA_ATTRIBUTE_LetterSpacing, wsValue)) {
    return 0;
  }
  CXFA_Measurement ms(wsValue);
  if (ms.GetUnit() == XFA_UNIT_Em) {
    return ms.GetValue() * GetFontSize();
  }
  return ms.ToUnit(XFA_UNIT_Pt);
}
int32_t CXFA_Font::GetLineThrough() {
  int32_t iValue = 0;
  m_pNode->TryInteger(XFA_ATTRIBUTE_LineThrough, iValue);
  return iValue;
}
int32_t CXFA_Font::GetLineThroughPeriod() {
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_All;
  m_pNode->TryEnum(XFA_ATTRIBUTE_LineThroughPeriod, eAttr);
  return eAttr;
}
int32_t CXFA_Font::GetOverline() {
  int32_t iValue = 0;
  m_pNode->TryInteger(XFA_ATTRIBUTE_Overline, iValue);
  return iValue;
}
int32_t CXFA_Font::GetOverlinePeriod() {
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_All;
  m_pNode->TryEnum(XFA_ATTRIBUTE_OverlinePeriod, eAttr);
  return eAttr;
}
int32_t CXFA_Font::GetUnderline() {
  int32_t iValue = 0;
  m_pNode->TryInteger(XFA_ATTRIBUTE_Underline, iValue);
  return iValue;
}
int32_t CXFA_Font::GetUnderlinePeriod() {
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_All;
  m_pNode->TryEnum(XFA_ATTRIBUTE_UnderlinePeriod, eAttr);
  return eAttr;
}
FX_FLOAT CXFA_Font::GetFontSize() {
  CXFA_Measurement ms;
  m_pNode->TryMeasure(XFA_ATTRIBUTE_Size, ms);
  return ms.ToUnit(XFA_UNIT_Pt);
}
void CXFA_Font::GetTypeface(CFX_WideStringC& wsTypeFace) {
  m_pNode->TryCData(XFA_ATTRIBUTE_Typeface, wsTypeFace);
}
FX_BOOL CXFA_Font::IsBold() {
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_Normal;
  m_pNode->TryEnum(XFA_ATTRIBUTE_Weight, eAttr);
  return eAttr == XFA_ATTRIBUTEENUM_Bold;
}
FX_BOOL CXFA_Font::IsItalic() {
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_Normal;
  m_pNode->TryEnum(XFA_ATTRIBUTE_Posture, eAttr);
  return eAttr == XFA_ATTRIBUTEENUM_Italic;
}
FX_BOOL CXFA_Font::IsUseKerning() {
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_None;
  m_pNode->TryEnum(XFA_ATTRIBUTE_KerningMode, eAttr);
  return eAttr == XFA_ATTRIBUTEENUM_Pair;
}
void CXFA_Font::SetColor(FX_ARGB color) {
  CXFA_Fill fill = m_pNode->GetProperty(0, XFA_ELEMENT_Fill);
  fill.SetColor(color);
}
FX_ARGB CXFA_Font::GetColor() {
  if (CXFA_Fill fill = m_pNode->GetChild(0, XFA_ELEMENT_Fill)) {
    return fill.GetColor(TRUE);
  }
  return 0xFF000000;
}
FX_BOOL CXFA_Font::SetBaselineShift(FX_FLOAT fBaselineShift) {
  CXFA_Measurement ms(fBaselineShift, XFA_UNIT_Pt);
  return m_pNode->SetMeasure(XFA_ATTRIBUTE_BaselineShift, ms);
}
FX_BOOL CXFA_Font::SetHorizontalScale(FX_FLOAT fHorizontalScale) {
  CFX_WideString wsValue;
  wsValue.Format(L"%d", (int32_t)fHorizontalScale);
  return m_pNode->SetCData(XFA_ATTRIBUTE_FontHorizontalScale, wsValue);
}
FX_BOOL CXFA_Font::SetVerticalScale(FX_FLOAT fVerticalScale) {
  CFX_WideString wsValue;
  wsValue.Format(L"%d", (int32_t)fVerticalScale);
  return m_pNode->SetCData(XFA_ATTRIBUTE_FontVerticalScale, wsValue);
}
FX_BOOL CXFA_Font::SetLetterSpacing(FX_FLOAT fLetterSpacing, XFA_UNIT eUnit) {
  return FALSE;
}
FX_BOOL CXFA_Font::SetLineThrough(int32_t iLineThrough) {
  return m_pNode->SetInteger(XFA_ATTRIBUTE_LineThrough, iLineThrough);
}
FX_BOOL CXFA_Font::SetLineThroughPeriod(int32_t iLineThroughPeriod) {
  return m_pNode->SetEnum(XFA_ATTRIBUTE_LineThroughPeriod,
                          (XFA_ATTRIBUTEENUM)iLineThroughPeriod);
}
FX_BOOL CXFA_Font::SetOverline(int32_t iOverline) {
  return m_pNode->SetInteger(XFA_ATTRIBUTE_Overline, iOverline);
}
FX_BOOL CXFA_Font::SetOverlinePeriod(int32_t iOverlinePeriod) {
  return m_pNode->SetEnum(XFA_ATTRIBUTE_OverlinePeriod,
                          (XFA_ATTRIBUTEENUM)iOverlinePeriod);
}
FX_BOOL CXFA_Font::SetUnderline(int32_t iUnderline) {
  return m_pNode->SetInteger(XFA_ATTRIBUTE_Underline, iUnderline);
}
FX_BOOL CXFA_Font::SetUnderlinePeriod(int32_t iUnderlinePeriod) {
  return m_pNode->SetEnum(XFA_ATTRIBUTE_UnderlinePeriod,
                          (XFA_ATTRIBUTEENUM)iUnderlinePeriod);
}
CXFA_Caption::CXFA_Caption(CXFA_Node* pNode) : CXFA_Data(pNode) {}
int32_t CXFA_Caption::GetPresence() {
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_Visible;
  m_pNode->TryEnum(XFA_ATTRIBUTE_Presence, eAttr);
  return eAttr;
}
int32_t CXFA_Caption::GetPlacementType() {
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_Left;
  m_pNode->TryEnum(XFA_ATTRIBUTE_Placement, eAttr);
  return eAttr;
}
FX_FLOAT CXFA_Caption::GetReserve() {
  CXFA_Measurement ms;
  m_pNode->TryMeasure(XFA_ATTRIBUTE_Reserve, ms);
  return ms.ToUnit(XFA_UNIT_Pt);
}
CXFA_Margin CXFA_Caption::GetMargin() {
  return CXFA_Margin(m_pNode ? m_pNode->GetChild(0, XFA_ELEMENT_Margin) : NULL);
}
CXFA_Font CXFA_Caption::GetFont() {
  return CXFA_Font(m_pNode ? m_pNode->GetChild(0, XFA_ELEMENT_Font) : NULL);
}
CXFA_Value CXFA_Caption::GetValue() {
  return CXFA_Value(m_pNode ? m_pNode->GetChild(0, XFA_ELEMENT_Value) : NULL);
}
CXFA_Para CXFA_Caption::GetPara() {
  return CXFA_Para(m_pNode ? m_pNode->GetChild(0, XFA_ELEMENT_Para) : NULL);
}
FX_BOOL CXFA_Caption::SetPresence(int32_t iPresence) {
  return m_pNode->SetEnum(XFA_ATTRIBUTE_Presence, (XFA_ATTRIBUTEENUM)iPresence);
}
FX_BOOL CXFA_Caption::SetPlacementType(int32_t iType) {
  return m_pNode->SetEnum(XFA_ATTRIBUTE_Placement, (XFA_ATTRIBUTEENUM)iType);
}
FX_BOOL CXFA_Caption::SetReserve(FX_FLOAT fReserve) {
  CXFA_Measurement ms(fReserve, XFA_UNIT_Pt);
  return m_pNode->SetMeasure(XFA_ATTRIBUTE_Reserve, ms);
}
CXFA_Para::CXFA_Para(CXFA_Node* pNode) : CXFA_Data(pNode) {}
int32_t CXFA_Para::GetHorizontalAlign() {
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_Left;
  m_pNode->TryEnum(XFA_ATTRIBUTE_HAlign, eAttr);
  return eAttr;
}
int32_t CXFA_Para::GetVerticalAlign() {
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_Top;
  m_pNode->TryEnum(XFA_ATTRIBUTE_VAlign, eAttr);
  return eAttr;
}
FX_FLOAT CXFA_Para::GetLineHeight() {
  CXFA_Measurement ms;
  m_pNode->TryMeasure(XFA_ATTRIBUTE_LineHeight, ms);
  return ms.ToUnit(XFA_UNIT_Pt);
}
FX_FLOAT CXFA_Para::GetMarginLeft() {
  CXFA_Measurement ms;
  m_pNode->TryMeasure(XFA_ATTRIBUTE_MarginLeft, ms);
  return ms.ToUnit(XFA_UNIT_Pt);
}
FX_FLOAT CXFA_Para::GetMarginRight() {
  CXFA_Measurement ms;
  m_pNode->TryMeasure(XFA_ATTRIBUTE_MarginRight, ms);
  return ms.ToUnit(XFA_UNIT_Pt);
}
int32_t CXFA_Para::GetOrphans() {
  int32_t iValue = 0;
  m_pNode->TryInteger(XFA_ATTRIBUTE_Orphans, iValue);
  return iValue;
}
FX_FLOAT CXFA_Para::GetRadixOffset() {
  CXFA_Measurement ms;
  m_pNode->TryMeasure(XFA_ATTRIBUTE_RadixOffset, ms);
  return ms.ToUnit(XFA_UNIT_Pt);
}
FX_FLOAT CXFA_Para::GetSpaceAbove() {
  CXFA_Measurement ms;
  m_pNode->TryMeasure(XFA_ATTRIBUTE_SpaceAbove, ms);
  return ms.ToUnit(XFA_UNIT_Pt);
}
FX_FLOAT CXFA_Para::GetSpaceBelow() {
  CXFA_Measurement ms;
  m_pNode->TryMeasure(XFA_ATTRIBUTE_SpaceBelow, ms);
  return ms.ToUnit(XFA_UNIT_Pt);
}
FX_FLOAT CXFA_Para::GetTextIndent() {
  CXFA_Measurement ms;
  m_pNode->TryMeasure(XFA_ATTRIBUTE_TextIndent, ms);
  return ms.ToUnit(XFA_UNIT_Pt);
}
int32_t CXFA_Para::GetWidows() {
  int32_t iValue = 0;
  m_pNode->TryInteger(XFA_ATTRIBUTE_Widows, iValue);
  return iValue;
}
FX_BOOL CXFA_Para::SetHorizontalAlign(int32_t iHorizontalAlign) {
  return m_pNode->SetEnum(XFA_ATTRIBUTE_HAlign,
                          (XFA_ATTRIBUTEENUM)iHorizontalAlign);
}
FX_BOOL CXFA_Para::SetVerticalAlign(int32_t iVerticalAlign) {
  return m_pNode->SetEnum(XFA_ATTRIBUTE_VAlign,
                          (XFA_ATTRIBUTEENUM)iVerticalAlign);
}
FX_BOOL CXFA_Para::SetLineHeight(FX_FLOAT fLineHeight) {
  CXFA_Measurement ms;
  return m_pNode->SetMeasure(XFA_ATTRIBUTE_LineHeight, ms);
}
FX_BOOL CXFA_Para::SetMarginLeft(FX_FLOAT fMarginLeft) {
  CXFA_Measurement ms(fMarginLeft, XFA_UNIT_Pt);
  return m_pNode->SetMeasure(XFA_ATTRIBUTE_MarginLeft, ms);
}
FX_BOOL CXFA_Para::SetMarginRight(FX_FLOAT fMarginRight) {
  CXFA_Measurement ms(fMarginRight, XFA_UNIT_Pt);
  return m_pNode->SetMeasure(XFA_ATTRIBUTE_MarginRight, ms);
}
FX_BOOL CXFA_Para::SetOrphans(int32_t iOrphans) {
  return m_pNode->SetInteger(XFA_ATTRIBUTE_Orphans, iOrphans);
}
FX_BOOL CXFA_Para::SetRadixOffset(FX_FLOAT fRadixOffset) {
  CXFA_Measurement ms(fRadixOffset, XFA_UNIT_Pt);
  return m_pNode->SetMeasure(XFA_ATTRIBUTE_RadixOffset, ms);
}
FX_BOOL CXFA_Para::SetSpaceAbove(FX_FLOAT fSpaceAbove) {
  CXFA_Measurement ms(fSpaceAbove, XFA_UNIT_Pt);
  return m_pNode->SetMeasure(XFA_ATTRIBUTE_SpaceAbove, ms);
}
FX_BOOL CXFA_Para::SetSpaceBelow(FX_FLOAT fSpaceBelow) {
  CXFA_Measurement ms(fSpaceBelow, XFA_UNIT_Pt);
  return m_pNode->SetMeasure(XFA_ATTRIBUTE_SpaceBelow, ms);
}
FX_BOOL CXFA_Para::SetTextIndent(FX_FLOAT fTextIndent) {
  CXFA_Measurement ms(fTextIndent, XFA_UNIT_Pt);
  return m_pNode->SetMeasure(XFA_ATTRIBUTE_TextIndent, ms);
}
FX_BOOL CXFA_Para::SetWidows(int32_t iWidows) {
  return m_pNode->SetInteger(XFA_ATTRIBUTE_Widows, iWidows);
}
CXFA_Keep::CXFA_Keep(CXFA_Node* pNode, CXFA_Node* pParent)
    : CXFA_Data(pNode), m_pParent(pParent) {}
int32_t CXFA_Keep::GetIntact() {
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_None;
  switch (m_pParent->GetClassID()) {
    case XFA_ELEMENT_Subform: {
      XFA_ATTRIBUTEENUM eAttrSubForm;
      m_pParent->TryEnum(XFA_ATTRIBUTE_Layout, eAttrSubForm);
      if (eAttrSubForm == XFA_ATTRIBUTEENUM_Position ||
          eAttrSubForm == XFA_ATTRIBUTEENUM_Row) {
        eAttr = XFA_ATTRIBUTEENUM_ContentArea;
      }
    } break;
    case XFA_ELEMENT_Draw:
      eAttr = XFA_ATTRIBUTEENUM_ContentArea;
      break;
    default:
      break;
  }
  m_pNode->TryEnum(XFA_ATTRIBUTE_Intact, eAttr, FALSE);
  return eAttr;
}
int32_t CXFA_Keep::GetNext() {
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_None;
  m_pNode->TryEnum(XFA_ATTRIBUTE_Next, eAttr);
  return eAttr;
}
int32_t CXFA_Keep::GetPrevious() {
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_None;
  m_pNode->TryEnum(XFA_ATTRIBUTE_Previous, eAttr);
  return eAttr;
}
FX_BOOL CXFA_Keep::SetIntact(int32_t iIntact) {
  return m_pNode->SetEnum(XFA_ATTRIBUTE_Intact, (XFA_ATTRIBUTEENUM)iIntact);
}
FX_BOOL CXFA_Keep::SetNext(int32_t iNext) {
  return m_pNode->SetEnum(XFA_ATTRIBUTE_Next, (XFA_ATTRIBUTEENUM)iNext);
}
FX_BOOL CXFA_Keep::SetPrevious(int32_t iPrevious) {
  return m_pNode->SetEnum(XFA_ATTRIBUTE_Previous, (XFA_ATTRIBUTEENUM)iPrevious);
}
CXFA_Event::CXFA_Event(CXFA_Node* pNode) : CXFA_Data(pNode) {}
int32_t CXFA_Event::GetActivity() {
  return m_pNode->GetEnum(XFA_ATTRIBUTE_Activity);
}
int32_t CXFA_Event::GetEventType() {
  CXFA_Node* pChild = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  while (pChild) {
    int32_t eType = pChild->GetClassID();
    if (eType != XFA_ELEMENT_Extras) {
      return eType;
    }
    pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling);
  }
  return XFA_ELEMENT_UNKNOWN;
}
void CXFA_Event::GetRef(CFX_WideStringC& wsRef) {
  m_pNode->TryCData(XFA_ATTRIBUTE_Ref, wsRef);
}
int32_t CXFA_Event::GetExecuteRunAt() {
  CXFA_Node* pNode = m_pNode->GetProperty(0, XFA_ELEMENT_Execute);
  return pNode->GetEnum(XFA_ATTRIBUTE_RunAt);
}
int32_t CXFA_Event::GetExecuteType() {
  CXFA_Node* pNode = m_pNode->GetProperty(0, XFA_ELEMENT_Execute);
  return pNode->GetEnum(XFA_ATTRIBUTE_ExecuteType);
}
void CXFA_Event::GetExecuteConnection(CFX_WideString& wsConnection) {
  CXFA_Node* pNode = m_pNode->GetProperty(0, XFA_ELEMENT_Execute);
  CFX_WideStringC cData;
  pNode->TryCData(XFA_ATTRIBUTE_Connection, cData);
  wsConnection = cData;
}
CXFA_Script CXFA_Event::GetScript() {
  return m_pNode->GetChild(0, XFA_ELEMENT_Script);
}
CXFA_Submit CXFA_Event::GetSubmit() {
  return m_pNode->GetChild(0, XFA_ELEMENT_Submit);
}
int32_t CXFA_Event::GetSignDataOperation() {
  CXFA_Node* pNode = m_pNode->GetProperty(0, XFA_ELEMENT_SignData);
  return pNode->GetEnum(XFA_ATTRIBUTE_Operation);
}
void CXFA_Event::GetSignDataTarget(CFX_WideString& wsTarget) {
  if (CXFA_Node* pNode = m_pNode->GetProperty(0, XFA_ELEMENT_SignData)) {
    CFX_WideStringC wsCData;
    pNode->TryCData(XFA_ATTRIBUTE_Target, wsCData);
    wsTarget = wsCData;
  }
}
FX_BOOL CXFA_Event::SetActivity(int32_t iActivity) {
  return m_pNode->SetEnum(XFA_ATTRIBUTE_Activity, (XFA_ATTRIBUTEENUM)iActivity);
}
FX_BOOL CXFA_Event::SetEventType(int32_t iEventType) {
  return FALSE;
}
FX_BOOL CXFA_Event::SetExecuteRunAt(int32_t iExecuteRunAt) {
  CXFA_Node* pNode = m_pNode->GetProperty(0, XFA_ELEMENT_Execute);
  return pNode->SetEnum(XFA_ATTRIBUTE_RunAt, (XFA_ATTRIBUTEENUM)iExecuteRunAt);
}
FX_BOOL CXFA_Event::SetExecuteType(int32_t iExecuteType) {
  CXFA_Node* pNode = m_pNode->GetProperty(0, XFA_ELEMENT_Execute);
  return pNode->SetEnum(XFA_ATTRIBUTE_ExecuteType,
                        (XFA_ATTRIBUTEENUM)iExecuteType);
}
FX_BOOL CXFA_Event::SetExecuteConnection(const CFX_WideString& wsConnection) {
  CXFA_Node* pNode = m_pNode->GetProperty(0, XFA_ELEMENT_Execute);
  return pNode->SetCData(XFA_ATTRIBUTE_Connection, wsConnection);
}
FX_BOOL CXFA_Event::SetSignDataOperation(int32_t iOperation) {
  CXFA_Node* pNode = m_pNode->GetProperty(0, XFA_ELEMENT_SignData);
  return pNode->SetEnum(XFA_ATTRIBUTE_Operation, (XFA_ATTRIBUTEENUM)iOperation);
}
FX_BOOL CXFA_Event::SetSignDataTarget(const CFX_WideString& wsTarget) {
  if (CXFA_Node* pNode = m_pNode->GetProperty(0, XFA_ELEMENT_SignData)) {
    return pNode->SetCData(XFA_ATTRIBUTE_Target, wsTarget);
  }
  return FALSE;
}
CXFA_Script::CXFA_Script(CXFA_Node* pNode) : CXFA_Data(pNode) {}
void CXFA_Script::GetBinding(CFX_WideString& wsBinding) {
  CFX_WideStringC cData;
  m_pNode->TryCData(XFA_ATTRIBUTE_Binding, cData);
  wsBinding = cData;
}
XFA_SCRIPTTYPE CXFA_Script::GetContentType() {
  CFX_WideStringC cData;
  if (m_pNode->TryCData(XFA_ATTRIBUTE_ContentType, cData, FALSE)) {
    if (cData == FX_WSTRC(L"application/x-javascript")) {
      return XFA_SCRIPTTYPE_Javascript;
    } else if (cData == FX_WSTRC(L"application/x-formcalc")) {
      return XFA_SCRIPTTYPE_Formcalc;
    } else {
      return XFA_SCRIPTTYPE_Unkown;
    }
  }
  return XFA_SCRIPTTYPE_Formcalc;
}
int32_t CXFA_Script::GetRunAt() {
  return m_pNode->GetEnum(XFA_ATTRIBUTE_RunAt);
}
void CXFA_Script::GetExpression(CFX_WideString& wsExpression) {
  m_pNode->TryContent(wsExpression);
}
FX_BOOL CXFA_Script::SetBinding(const CFX_WideString& wsBinding) {
  return m_pNode->SetCData(XFA_ATTRIBUTE_Binding, wsBinding);
}
FX_BOOL CXFA_Script::SetContentType(XFA_SCRIPTTYPE eType) {
  CFX_WideString wsType;
  switch (eType) {
    case XFA_SCRIPTTYPE_Javascript:
      wsType = L"application/x-javascript";
      break;
    case XFA_SCRIPTTYPE_Formcalc:
      wsType = L"application/x-formcalc";
      break;
    default:
      break;
  }
  return m_pNode->SetCData(XFA_ATTRIBUTE_ContentType, wsType);
}
FX_BOOL CXFA_Script::SetRunAt(int32_t iRunAt) {
  return m_pNode->SetEnum(XFA_ATTRIBUTE_RunAt, (XFA_ATTRIBUTEENUM)iRunAt);
}
FX_BOOL CXFA_Script::SetExpression(const CFX_WideString& wsExpression) {
  return m_pNode->SetContent(wsExpression, wsExpression);
}
CXFA_Submit::CXFA_Submit(CXFA_Node* pNode) : CXFA_Data(pNode) {}
FX_BOOL CXFA_Submit::IsSubmitEmbedPDF() {
  return m_pNode->GetBoolean(XFA_ATTRIBUTE_EmbedPDF);
}
int32_t CXFA_Submit::GetSubmitFormat() {
  return m_pNode->GetEnum(XFA_ATTRIBUTE_Format);
}
void CXFA_Submit::GetSubmitTarget(CFX_WideStringC& wsTarget) {
  m_pNode->TryCData(XFA_ATTRIBUTE_Target, wsTarget);
}
XFA_TEXTENCODING CXFA_Submit::GetSubmitTextEncoding() {
  CFX_WideStringC wsCData;
  if (!m_pNode->TryCData(XFA_ATTRIBUTE_TextEncoding, wsCData)) {
    return XFA_TEXTENCODING_None;
  }
  CFX_WideString wsValue(wsCData);
  if (wsValue == L"Big-Five") {
    return XFA_TEXTENCODING_Big5;
  } else if (wsValue == L"fontSpecific") {
    return XFA_TEXTENCODING_FontSpecific;
  } else if (wsValue == L"GBK") {
    return XFA_TEXTENCODING_GBK;
  } else if (wsValue == L"GB-18030") {
    return XFA_TEXTENCODING_GB18030;
  } else if (wsValue == L"GB-2312") {
    return XFA_TEXTENCODING_GB2312;
  } else if (wsValue == L"ISO-8859-NN") {
    return XFA_TEXTENCODING_ISO8859NN;
  } else if (wsValue == L"KSC-5601") {
    return XFA_TEXTENCODING_KSC5601;
  } else if (wsValue == L"Shift-JIS") {
    return XFA_TEXTENCODING_ShiftJIS;
  } else if (wsValue == L"UCS-2") {
    return XFA_TEXTENCODING_UCS2;
  } else if (wsValue == L"UTF-16") {
    return XFA_TEXTENCODING_UTF16;
  } else if (wsValue == L"UTF-8") {
    return XFA_TEXTENCODING_UTF8;
  }
  return XFA_TEXTENCODING_None;
}
void CXFA_Submit::GetSubmitXDPContent(CFX_WideStringC& wsContent) {
  m_pNode->TryCData(XFA_ATTRIBUTE_XdpContent, wsContent);
}
FX_BOOL CXFA_Submit::SetSubmitFormat(int32_t iSubmitFormat) {
  return m_pNode->SetEnum(XFA_ATTRIBUTE_Format,
                          (XFA_ATTRIBUTEENUM)iSubmitFormat);
}
FX_BOOL CXFA_Submit::SetSubmitTarget(const CFX_WideString& wsTarget) {
  return m_pNode->SetCData(XFA_ATTRIBUTE_Target, wsTarget);
}
FX_BOOL CXFA_Submit::SetSubmitTextEncoding(XFA_TEXTENCODING eTextEncoding) {
  CFX_WideString wsValue;
  switch (eTextEncoding) {
    case XFA_TEXTENCODING_Big5:
      wsValue = L"Big-Five";
      break;
    case XFA_TEXTENCODING_FontSpecific:
      wsValue = L"fontSpecific";
      break;
    case XFA_TEXTENCODING_GBK:
      wsValue = L"GBK";
      break;
    case XFA_TEXTENCODING_GB18030:
      wsValue = L"GB-18030";
      break;
    case XFA_TEXTENCODING_GB2312:
      wsValue = L"GB-2312";
      break;
    case XFA_TEXTENCODING_ISO8859NN:
      wsValue = L"ISO-8859-NN";
      break;
    case XFA_TEXTENCODING_KSC5601:
      wsValue = L"KSC-5601";
      break;
    case XFA_TEXTENCODING_ShiftJIS:
      wsValue = L"Shift-JIS";
      break;
    case XFA_TEXTENCODING_UCS2:
      wsValue = L"UCS-2";
      break;
    case XFA_TEXTENCODING_UTF16:
      wsValue = L"UTF-16";
      break;
    case XFA_TEXTENCODING_UTF8:
      wsValue = L"UTF-8";
      break;
    default:
      break;
  }
  return m_pNode->SetCData(XFA_ATTRIBUTE_TextEncoding, wsValue);
}
FX_BOOL CXFA_Submit::SetSubmitXDPContent(const CFX_WideString& wsContent) {
  return m_pNode->SetCData(XFA_ATTRIBUTE_XdpContent, wsContent);
}
XFA_ELEMENT CXFA_Value::GetChildValueClassID() {
  if (!m_pNode) {
    return XFA_ELEMENT_UNKNOWN;
  }
  if (CXFA_Node* pNode = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild)) {
    return pNode->GetClassID();
  }
  return XFA_ELEMENT_UNKNOWN;
}
FX_BOOL CXFA_Value::GetChildValueContent(CFX_WideString& wsContent) {
  if (!m_pNode) {
    return FALSE;
  }
  if (CXFA_Node* pNode = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild)) {
    return pNode->TryContent(wsContent);
  }
  return FALSE;
}
CXFA_Arc CXFA_Value::GetArc() {
  return m_pNode ? CXFA_Arc(m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild))
                 : NULL;
}
CXFA_Line CXFA_Value::GetLine() {
  return m_pNode ? CXFA_Line(m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild))
                 : NULL;
}
CXFA_Rectangle CXFA_Value::GetRectangle() {
  return m_pNode ? CXFA_Rectangle(m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild))
                 : NULL;
}
CXFA_Text CXFA_Value::GetText() {
  return m_pNode ? CXFA_Text(m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild))
                 : NULL;
}
CXFA_ExData CXFA_Value::GetExData() {
  return m_pNode ? CXFA_ExData(m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild))
                 : NULL;
}
CXFA_Image CXFA_Value::GetImage() {
  return CXFA_Image(
      m_pNode ? (m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild)) : NULL, TRUE);
}
FX_BOOL CXFA_Value::SetChildValueContent(const CFX_WideString& wsContent,
                                         FX_BOOL bNotify,
                                         XFA_ELEMENT iType) {
  if (!m_pNode) {
    return FALSE;
  }
  CXFA_Node* pNode = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  if (!pNode) {
    if (iType == XFA_ELEMENT_UNKNOWN) {
      return FALSE;
    }
    pNode = m_pNode->GetProperty(0, iType);
  }
  CFX_WideString wsFormatContent(wsContent);
  CXFA_WidgetData* pContainerWidgetData = pNode->GetContainerWidgetData();
  if (pContainerWidgetData) {
    pContainerWidgetData->GetFormatDataValue(wsContent, wsFormatContent);
  }
  return pNode->SetContent(wsContent, wsFormatContent, bNotify);
}
int32_t CXFA_Line::GetHand() {
  return m_pNode->GetEnum(XFA_ATTRIBUTE_Hand);
}
FX_BOOL CXFA_Line::GetSlop() {
  XFA_ATTRIBUTEENUM eSlop = m_pNode->GetEnum(XFA_ATTRIBUTE_Slope);
  return eSlop == XFA_ATTRIBUTEENUM_Slash;
}
CXFA_Edge CXFA_Line::GetEdge() {
  return CXFA_Edge(m_pNode->GetChild(0, XFA_ELEMENT_Edge));
}
FX_BOOL CXFA_Line::SetHand(int32_t iHand) {
  return m_pNode->SetEnum(XFA_ATTRIBUTE_Hand, (XFA_ATTRIBUTEENUM)iHand);
}
FX_BOOL CXFA_Line::SetSlop(int32_t iSlop) {
  return m_pNode->SetEnum(XFA_ATTRIBUTE_Slope, (XFA_ATTRIBUTEENUM)iSlop);
}
CXFA_Text::CXFA_Text(CXFA_Node* pNode) : CXFA_Data(pNode) {}
void CXFA_Text::GetName(CFX_WideStringC& wsName) {
  m_pNode->TryCData(XFA_ATTRIBUTE_Name, wsName);
}
int32_t CXFA_Text::GetMaxChars() {
  return m_pNode->GetInteger(XFA_ATTRIBUTE_MaxChars);
}
void CXFA_Text::GetRid(CFX_WideStringC& wsRid) {
  m_pNode->TryCData(XFA_ATTRIBUTE_Rid, wsRid);
}
void CXFA_Text::GetContent(CFX_WideString& wsText) {
  m_pNode->TryContent(wsText);
}
void CXFA_Text::SetContent(CFX_WideString wsText, FX_BOOL bNotify) {
  CFX_WideString wsFormatValue(wsText);
  CXFA_WidgetData* pContainerWidgetData = m_pNode->GetContainerWidgetData();
  if (pContainerWidgetData) {
    pContainerWidgetData->GetFormatDataValue(wsText, wsFormatValue);
  }
  m_pNode->SetContent(wsText, wsFormatValue, bNotify);
}
FX_BOOL CXFA_Text::SetName(const CFX_WideString& wsName) {
  return m_pNode->SetCData(XFA_ATTRIBUTE_Name, wsName);
}
FX_BOOL CXFA_Text::SetMaxChars(int32_t iMaxChars) {
  return m_pNode->SetInteger(XFA_ATTRIBUTE_MaxChars, iMaxChars);
}
FX_BOOL CXFA_Text::SetRid(const CFX_WideString& wsRid) {
  return m_pNode->SetCData(XFA_ATTRIBUTE_Rid, wsRid);
}
CXFA_ExData::CXFA_ExData(CXFA_Node* pNode) : CXFA_Data(pNode) {}
void CXFA_ExData::GetContentType(CFX_WideStringC& wsContentType) {
  m_pNode->TryCData(XFA_ATTRIBUTE_ContentType, wsContentType);
}
void CXFA_ExData::GetHref(CFX_WideStringC& wsHref) {
  m_pNode->TryCData(XFA_ATTRIBUTE_Href, wsHref);
}
int32_t CXFA_ExData::GetMaxLength() {
  return m_pNode->GetInteger(XFA_ATTRIBUTE_MaxLength);
}
void CXFA_ExData::GetRid(CFX_WideStringC& wsRid) {
  m_pNode->TryCData(XFA_ATTRIBUTE_Rid, wsRid);
}
int32_t CXFA_ExData::GetTransferEncoding() {
  return m_pNode->GetEnum(XFA_ATTRIBUTE_TransferEncoding);
}
void CXFA_ExData::GetContent(CFX_WideString& wsText) {
  m_pNode->TryContent(wsText);
}
FX_BOOL CXFA_ExData::SetContentType(const CFX_WideString& wsContentType) {
  return m_pNode->SetCData(XFA_ATTRIBUTE_ContentType, wsContentType);
}
FX_BOOL CXFA_ExData::SetHref(const CFX_WideString& wsHref) {
  return m_pNode->SetCData(XFA_ATTRIBUTE_Href, wsHref);
}
FX_BOOL CXFA_ExData::SetMaxLength(int32_t iMaxLength) {
  return m_pNode->SetInteger(XFA_ATTRIBUTE_MaxLength, iMaxLength);
}
FX_BOOL CXFA_ExData::SetRid(const CFX_WideString& wsRid) {
  return m_pNode->SetCData(XFA_ATTRIBUTE_Rid, wsRid);
}
FX_BOOL CXFA_ExData::SetTransferEncoding(int32_t iTransferEncoding) {
  return m_pNode->SetEnum(XFA_ATTRIBUTE_TransferEncoding,
                          (XFA_ATTRIBUTEENUM)iTransferEncoding);
}
FX_BOOL CXFA_ExData::SetContent(const CFX_WideString& wsText,
                                FX_BOOL bNotify,
                                FX_BOOL bScriptModify,
                                FX_BOOL bSyncData) {
  CFX_WideString wsFormatValue(wsText);
  CXFA_WidgetData* pContainerWidgetData = m_pNode->GetContainerWidgetData();
  if (pContainerWidgetData) {
    pContainerWidgetData->GetFormatDataValue(wsText, wsFormatValue);
  }
  return m_pNode->SetContent(wsText, wsFormatValue, bNotify, bScriptModify,
                             bSyncData);
}
CXFA_Image::CXFA_Image(CXFA_Node* pNode, FX_BOOL bDefValue)
    : CXFA_Data(pNode), m_bDefValue(bDefValue) {}
int32_t CXFA_Image::GetAspect() {
  return m_pNode->GetEnum(XFA_ATTRIBUTE_Aspect);
}
FX_BOOL CXFA_Image::GetContentType(CFX_WideString& wsContentType) {
  return m_pNode->TryCData(XFA_ATTRIBUTE_ContentType, wsContentType);
}
FX_BOOL CXFA_Image::GetHref(CFX_WideString& wsHref) {
  if (m_bDefValue) {
    return m_pNode->TryCData(XFA_ATTRIBUTE_Href, wsHref);
  }
  return m_pNode->GetAttribute(FX_WSTRC(L"href"), wsHref);
}
int32_t CXFA_Image::GetTransferEncoding() {
  if (m_bDefValue) {
    return m_pNode->GetEnum(XFA_ATTRIBUTE_TransferEncoding);
  }
  return XFA_ATTRIBUTEENUM_Base64;
}
FX_BOOL CXFA_Image::GetContent(CFX_WideString& wsText) {
  return m_pNode->TryContent(wsText);
}
FX_BOOL CXFA_Image::SetAspect(int32_t iAspect) {
  return m_pNode->SetEnum(XFA_ATTRIBUTE_Aspect, (XFA_ATTRIBUTEENUM)iAspect);
}
FX_BOOL CXFA_Image::SetContentType(const CFX_WideString& wsContentType) {
  return m_pNode->SetCData(XFA_ATTRIBUTE_ContentType, wsContentType);
}
FX_BOOL CXFA_Image::SetHref(const CFX_WideString& wsHref) {
  if (m_bDefValue) {
    return m_pNode->SetCData(XFA_ATTRIBUTE_Href, wsHref);
  }
  return m_pNode->SetAttribute(XFA_ATTRIBUTE_Href, wsHref);
}
FX_BOOL CXFA_Image::SetTransferEncoding(int32_t iTransferEncoding) {
  if (m_bDefValue) {
    return m_pNode->SetEnum(XFA_ATTRIBUTE_TransferEncoding,
                            (XFA_ATTRIBUTEENUM)iTransferEncoding);
  }
  return TRUE;
}
FX_BOOL CXFA_Image::SetContent(const CFX_WideString& wsText) {
  CFX_WideString wsFormatValue(wsText);
  CXFA_WidgetData* pContainerWidgetData = m_pNode->GetContainerWidgetData();
  if (pContainerWidgetData) {
    pContainerWidgetData->GetFormatDataValue(wsText, wsFormatValue);
  }
  return m_pNode->SetContent(wsText, wsFormatValue);
}
CXFA_Calculate::CXFA_Calculate(CXFA_Node* pNode) : CXFA_Data(pNode) {}
int32_t CXFA_Calculate::GetOverride() {
  XFA_ATTRIBUTEENUM eAtt = XFA_ATTRIBUTEENUM_Error;
  m_pNode->TryEnum(XFA_ATTRIBUTE_Override, eAtt, FALSE);
  return eAtt;
}
CXFA_Script CXFA_Calculate::GetScript() {
  return m_pNode->GetChild(0, XFA_ELEMENT_Script);
}
void CXFA_Calculate::GetMessageText(CFX_WideString& wsMessage) {
  if (CXFA_Node* pNode = m_pNode->GetChild(0, XFA_ELEMENT_Message)) {
    CXFA_Text text(pNode->GetChild(0, XFA_ELEMENT_Text));
    if (text) {
      text.GetContent(wsMessage);
    }
  }
}
FX_BOOL CXFA_Calculate::SetOverride(int32_t iOverride) {
  return m_pNode->SetEnum(XFA_ATTRIBUTE_Override, (XFA_ATTRIBUTEENUM)iOverride);
}
FX_BOOL CXFA_Calculate::SetMessageText(const CFX_WideString& wsMessage) {
  if (CXFA_Node* pNode = m_pNode->GetChild(0, XFA_ELEMENT_Message)) {
    CXFA_Node* pChildNode = pNode->GetProperty(0, XFA_ELEMENT_Text);
    return pChildNode->SetContent(wsMessage, wsMessage);
  }
  return FALSE;
}
CXFA_Validate::CXFA_Validate(CXFA_Node* pNode) : CXFA_Data(pNode) {}
int32_t CXFA_Validate::GetFormatTest() {
  return m_pNode->GetEnum(XFA_ATTRIBUTE_FormatTest);
}
FX_BOOL CXFA_Validate::SetTestValue(int32_t iType,
                                    CFX_WideString& wsValue,
                                    XFA_ATTRIBUTEENUM eName) {
  XFA_LPCATTRIBUTEENUMINFO pInfo = XFA_GetAttributeEnumByName(wsValue);
  if (pInfo) {
    eName = pInfo->eName;
  }
  m_pNode->SetEnum((XFA_ATTRIBUTE)iType, eName, FALSE);
  return TRUE;
}
FX_BOOL CXFA_Validate::SetFormatTest(CFX_WideString wsValue) {
  return SetTestValue(XFA_ATTRIBUTE_FormatTest, wsValue,
                      XFA_ATTRIBUTEENUM_Warning);
}
FX_BOOL CXFA_Validate::SetNullTest(CFX_WideString wsValue) {
  return SetTestValue(XFA_ATTRIBUTE_NullTest, wsValue,
                      XFA_ATTRIBUTEENUM_Disabled);
}
int32_t CXFA_Validate::GetNullTest() {
  return m_pNode->GetEnum(XFA_ATTRIBUTE_NullTest);
}
int32_t CXFA_Validate::GetScriptTest() {
  return m_pNode->GetEnum(XFA_ATTRIBUTE_ScriptTest);
}
void CXFA_Validate::GetMessageText(CFX_WideString& wsMessage,
                                   const CFX_WideStringC& wsMessageType) {
  if (CXFA_Node* pNode = m_pNode->GetProperty(0, XFA_ELEMENT_Message, FALSE)) {
    CXFA_Node* pItemNode = pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
    for (; pItemNode;
         pItemNode = pItemNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
      if (pItemNode->GetClassID() != XFA_ELEMENT_Text) {
        continue;
      }
      CFX_WideStringC wsName;
      pItemNode->TryCData(XFA_ATTRIBUTE_Name, wsName);
      if (wsName.IsEmpty() || wsName == wsMessageType) {
        pItemNode->TryContent(wsMessage);
        return;
      }
    }
  }
}
void CXFA_Validate::SetFormatMessageText(CFX_WideString wsMessage) {
  SetMessageText(wsMessage, FX_WSTRC(L"formatTest"));
}
void CXFA_Validate::GetFormatMessageText(CFX_WideString& wsMessage) {
  GetMessageText(wsMessage, FX_WSTRC(L"formatTest"));
}
void CXFA_Validate::SetNullMessageText(CFX_WideString wsMessage) {
  SetMessageText(wsMessage, FX_WSTRC(L"nullTest"));
}
void CXFA_Validate::GetNullMessageText(CFX_WideString& wsMessage) {
  GetMessageText(wsMessage, FX_WSTRC(L"nullTest"));
}
void CXFA_Validate::SetMessageText(CFX_WideString& wsMessage,
                                   const CFX_WideStringC& wsMessageType) {
  if (CXFA_Node* pNode = m_pNode->GetProperty(0, XFA_ELEMENT_Message, TRUE)) {
    CXFA_Node* pItemNode = pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
    for (; pItemNode;
         pItemNode = pItemNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
      if (pItemNode->GetClassID() != XFA_ELEMENT_Text) {
        continue;
      }
      CFX_WideStringC wsName;
      pItemNode->TryCData(XFA_ATTRIBUTE_Name, wsName);
      if (wsName.IsEmpty() || wsName == wsMessageType) {
        pItemNode->SetContent(wsMessage, wsMessage, FALSE);
        return;
      }
    }
    CXFA_Node* pTextNode = pNode->CreateSamePacketNode(XFA_ELEMENT_Text);
    pNode->InsertChild(pTextNode);
    pTextNode->SetCData(XFA_ATTRIBUTE_Name, wsMessageType, FALSE);
    pTextNode->SetContent(wsMessage, wsMessage, FALSE);
  }
}
void CXFA_Validate::GetScriptMessageText(CFX_WideString& wsMessage) {
  GetMessageText(wsMessage, FX_WSTRC(L"scriptTest"));
}
void CXFA_Validate::SetScriptMessageText(CFX_WideString wsMessage) {
  SetMessageText(wsMessage, FX_WSTRC(L"scriptTest"));
}
void CXFA_Validate::GetPicture(CFX_WideString& wsPicture) {
  if (CXFA_Node* pNode = m_pNode->GetChild(0, XFA_ELEMENT_Picture)) {
    pNode->TryContent(wsPicture);
  }
}
CXFA_Script CXFA_Validate::GetScript() {
  return m_pNode->GetChild(0, XFA_ELEMENT_Script);
}
CXFA_Variables::CXFA_Variables(CXFA_Node* pNode) : CXFA_Data(pNode) {}
int32_t CXFA_Variables::CountScripts() {
  return m_pNode->CountChildren(XFA_ELEMENT_Script);
}
CXFA_Script CXFA_Variables::GetScript(int32_t nIndex) {
  return m_pNode->GetChild(nIndex, XFA_ELEMENT_Script);
}
CXFA_Bind::CXFA_Bind(CXFA_Node* pNode) : CXFA_Data(pNode) {}
int32_t CXFA_Bind::GetMatch() {
  return m_pNode->GetEnum(XFA_ATTRIBUTE_Match);
}
void CXFA_Bind::GetRef(CFX_WideStringC& wsRef) {
  m_pNode->TryCData(XFA_ATTRIBUTE_Ref, wsRef);
}
void CXFA_Bind::GetPicture(CFX_WideString& wsPicture) {
  if (CXFA_Node* pPicture = m_pNode->GetChild(0, XFA_ELEMENT_Picture)) {
    pPicture->TryContent(wsPicture);
  }
}
FX_BOOL CXFA_Bind::SetMatch(int32_t iMatch) {
  return m_pNode->SetEnum(XFA_ATTRIBUTE_Match, (XFA_ATTRIBUTEENUM)iMatch);
}
FX_BOOL CXFA_Bind::SetRef(const CFX_WideString& wsRef) {
  return m_pNode->SetCData(XFA_ATTRIBUTE_Ref, wsRef);
}
FX_BOOL CXFA_Bind::SetPicture(const CFX_WideString& wsPicture) {
  if (CXFA_Node* pPicture = m_pNode->GetChild(0, XFA_ELEMENT_Picture)) {
    return pPicture->SetContent(wsPicture, wsPicture);
  }
  return FALSE;
}
CXFA_Assist::CXFA_Assist(CXFA_Node* pNode) : CXFA_Data(pNode) {}
CXFA_ToolTip CXFA_Assist::GetToolTip() {
  return m_pNode->GetChild(0, XFA_ELEMENT_ToolTip);
}
CXFA_ToolTip::CXFA_ToolTip(CXFA_Node* pNode) : CXFA_Data(pNode) {}
FX_BOOL CXFA_ToolTip::GetTip(CFX_WideString& wsTip) {
  return m_pNode->TryContent(wsTip);
}
FX_BOOL CXFA_ToolTip::SetTip(const CFX_WideString& wsTip) {
  return m_pNode->SetContent(wsTip, wsTip);
}
CXFA_BindItems::CXFA_BindItems(CXFA_Node* pNode) : CXFA_Data(pNode) {}
void CXFA_BindItems::GetConnection(CFX_WideStringC& wsConnection) {
  m_pNode->TryCData(XFA_ATTRIBUTE_Connection, wsConnection);
}
void CXFA_BindItems::GetLabelRef(CFX_WideStringC& wsLabelRef) {
  m_pNode->TryCData(XFA_ATTRIBUTE_LabelRef, wsLabelRef);
}
void CXFA_BindItems::GetValueRef(CFX_WideStringC& wsValueRef) {
  m_pNode->TryCData(XFA_ATTRIBUTE_ValueRef, wsValueRef);
}
void CXFA_BindItems::GetRef(CFX_WideStringC& wsRef) {
  m_pNode->TryCData(XFA_ATTRIBUTE_Ref, wsRef);
}
FX_BOOL CXFA_BindItems::SetConnection(const CFX_WideString& wsConnection) {
  return m_pNode->SetCData(XFA_ATTRIBUTE_Connection, wsConnection);
}
FX_BOOL CXFA_BindItems::SetLabelRef(const CFX_WideString& wsLabelRef) {
  return m_pNode->SetCData(XFA_ATTRIBUTE_LabelRef, wsLabelRef);
}
FX_BOOL CXFA_BindItems::SetValueRef(const CFX_WideString& wsValueRef) {
  return m_pNode->SetCData(XFA_ATTRIBUTE_ValueRef, wsValueRef);
}
FX_BOOL CXFA_BindItems::SetRef(const CFX_WideString& wsRef) {
  return m_pNode->SetCData(XFA_ATTRIBUTE_Ref, wsRef);
}
int32_t CXFA_Box::GetBreak() const {
  if (!m_pNode) {
    return XFA_ATTRIBUTEENUM_Close;
  }
  return m_pNode->GetEnum(XFA_ATTRIBUTE_Break);
}
int32_t CXFA_Box::GetHand() const {
  if (!m_pNode) {
    return XFA_ATTRIBUTEENUM_Even;
  }
  return m_pNode->GetEnum(XFA_ATTRIBUTE_Hand);
}
int32_t CXFA_Box::GetPresence() const {
  if (!m_pNode) {
    return XFA_ATTRIBUTEENUM_Hidden;
  }
  return m_pNode->GetEnum(XFA_ATTRIBUTE_Presence);
}
int32_t CXFA_Box::CountCorners() const {
  if (!m_pNode) {
    return 0;
  }
  return m_pNode->CountChildren(XFA_ELEMENT_Corner);
}
CXFA_Corner CXFA_Box::GetCorner(int32_t nIndex) const {
  if (!m_pNode) {
    return NULL;
  }
  return CXFA_Corner(
      m_pNode->GetProperty(nIndex, XFA_ELEMENT_Corner, nIndex == 0));
}
int32_t CXFA_Box::CountEdges() const {
  if (!m_pNode) {
    return 0;
  }
  return m_pNode->CountChildren(XFA_ELEMENT_Edge);
}
CXFA_Edge CXFA_Box::GetEdge(int32_t nIndex) const {
  if (!m_pNode) {
    return NULL;
  }
  return CXFA_Edge(m_pNode->GetProperty(nIndex, XFA_ELEMENT_Edge, nIndex == 0));
}
static void XFA_BOX_GetStrokes(CXFA_Node* pNode,
                               CXFA_StrokeArray& strokes,
                               FX_BOOL bNULL) {
  strokes.RemoveAll();
  if (!pNode) {
    return;
  }
  strokes.SetSize(8);
  int32_t i, j;
  for (i = 0, j = 0; i < 4; i++) {
    CXFA_Corner corner =
        CXFA_Corner(pNode->GetProperty(i, XFA_ELEMENT_Corner, i == 0));
    if (corner.IsExistInXML() || i == 0) {
      strokes.SetAt(j, corner);
    } else if (bNULL) {
      strokes.SetAt(j, NULL);
    } else if (i == 1) {
      strokes.SetAt(j, strokes[0]);
    } else if (i == 2) {
      strokes.SetAt(j, strokes[0]);
    } else {
      strokes.SetAt(j, strokes[2]);
    }
    j++;
    CXFA_Edge edge = CXFA_Edge(pNode->GetProperty(i, XFA_ELEMENT_Edge, i == 0));
    if (edge.IsExistInXML() || i == 0) {
      strokes.SetAt(j, edge);
    } else if (bNULL) {
      strokes.SetAt(j, NULL);
    } else if (i == 1) {
      strokes.SetAt(j, strokes[1]);
    } else if (i == 2) {
      strokes.SetAt(j, strokes[1]);
    } else {
      strokes.SetAt(j, strokes[3]);
    }
    j++;
  }
}
void CXFA_Box::GetStrokes(CXFA_StrokeArray& strokes) const {
  XFA_BOX_GetStrokes(m_pNode, strokes, FALSE);
}
FX_BOOL CXFA_Box::IsCircular() const {
  if (!m_pNode) {
    return FALSE;
  }
  return m_pNode->GetBoolean(XFA_ATTRIBUTE_Circular);
}
FX_BOOL CXFA_Box::GetStartAngle(FX_FLOAT& fStartAngle) const {
  fStartAngle = 0;
  if (!m_pNode) {
    return FALSE;
  }
  CXFA_Measurement ms;
  FX_BOOL bRet = m_pNode->TryMeasure(XFA_ATTRIBUTE_StartAngle, ms, FALSE);
  if (bRet) {
    fStartAngle = ms.GetValue();
  }
  return bRet;
}
FX_BOOL CXFA_Box::GetSweepAngle(FX_FLOAT& fSweepAngle) const {
  fSweepAngle = 360;
  if (!m_pNode) {
    return FALSE;
  }
  CXFA_Measurement ms;
  FX_BOOL bRet = m_pNode->TryMeasure(XFA_ATTRIBUTE_SweepAngle, ms, FALSE);
  if (bRet) {
    fSweepAngle = ms.GetValue();
  }
  return bRet;
}
CXFA_Fill CXFA_Box::GetFill(FX_BOOL bModified) const {
  if (!m_pNode) {
    return NULL;
  }
  CXFA_Node* pFillNode = m_pNode->GetProperty(0, XFA_ELEMENT_Fill, bModified);
  return CXFA_Fill(pFillNode);
}
CXFA_Margin CXFA_Box::GetMargin() const {
  if (!m_pNode) {
    return NULL;
  }
  return CXFA_Margin(m_pNode->GetChild(0, XFA_ELEMENT_Margin));
}
static FX_BOOL XFA_BOX_SameStyles(const CXFA_StrokeArray& strokes) {
  int32_t iCount = strokes.GetSize();
  if (iCount < 1) {
    return TRUE;
  }
  CXFA_Stroke stroke1 = strokes[0];
  for (int32_t i = 1; i < iCount; i++) {
    CXFA_Stroke stroke2 = strokes[i];
    if (!stroke2.IsExistInXML()) {
      continue;
    }
    if (!stroke1.IsExistInXML()) {
      stroke1 = stroke2;
    } else if (!stroke1.SameStyles(stroke2)) {
      return FALSE;
    }
  }
  return TRUE;
}
FX_BOOL CXFA_Box::SameStyles() const {
  if (IsArc()) {
    return TRUE;
  }
  CXFA_StrokeArray strokes;
  XFA_BOX_GetStrokes(m_pNode, strokes, TRUE);
  return XFA_BOX_SameStyles(strokes);
}
static int32_t XFA_BOX_3DStyle(const CXFA_StrokeArray& strokes,
                               CXFA_Stroke& stroke) {
  int32_t iCount = strokes.GetSize();
  if (iCount < 1) {
    return 0;
  }
  stroke = strokes[0];
  for (int32_t i = 1; i < iCount; i++) {
    CXFA_Stroke find = strokes[i];
    if (!find.IsExistInXML()) {
      continue;
    }
    if (!stroke.IsExistInXML()) {
      stroke = find;
    } else if (stroke.GetStrokeType() != find.GetStrokeType()) {
      stroke = find;
      break;
    }
  }
  int32_t iType = stroke.GetStrokeType();
  if (iType == XFA_ATTRIBUTEENUM_Lowered || iType == XFA_ATTRIBUTEENUM_Raised ||
      iType == XFA_ATTRIBUTEENUM_Etched ||
      iType == XFA_ATTRIBUTEENUM_Embossed) {
    return iType;
  }
  return 0;
}
int32_t CXFA_Box::Get3DStyle(FX_BOOL& bVisible, FX_FLOAT& fThickness) const {
  if (IsArc()) {
    return 0;
  }
  CXFA_StrokeArray strokes;
  XFA_BOX_GetStrokes(m_pNode, strokes, TRUE);
  CXFA_Stroke stroke(NULL);
  int32_t iType = XFA_BOX_3DStyle(strokes, stroke);
  if (iType) {
    bVisible = stroke.IsVisible();
    fThickness = stroke.GetThickness();
  }
  return iType;
}
int32_t CXFA_Stroke::GetPresence() const {
  return m_pNode ? m_pNode->GetEnum(XFA_ATTRIBUTE_Presence)
                 : XFA_ATTRIBUTEENUM_Invisible;
}
int32_t CXFA_Stroke::GetCapType() const {
  if (!m_pNode) {
    return XFA_ATTRIBUTEENUM_Square;
  }
  return m_pNode->GetEnum(XFA_ATTRIBUTE_Cap);
}
int32_t CXFA_Stroke::GetStrokeType() const {
  return m_pNode ? m_pNode->GetEnum(XFA_ATTRIBUTE_Stroke)
                 : XFA_ATTRIBUTEENUM_Solid;
}
FX_FLOAT CXFA_Stroke::GetThickness() const {
  return GetMSThickness().ToUnit(XFA_UNIT_Pt);
}
CXFA_Measurement CXFA_Stroke::GetMSThickness() const {
  return m_pNode ? m_pNode->GetMeasure(XFA_ATTRIBUTE_Thickness)
                 : XFA_GetAttributeDefaultValue_Measure(XFA_ELEMENT_Edge,
                                                        XFA_ATTRIBUTE_Thickness,
                                                        XFA_XDPPACKET_Form);
}
void CXFA_Stroke::SetThickness(FX_FLOAT fThickness) {
  if (!m_pNode) {
    return;
  }
  CXFA_Measurement thickness(fThickness, XFA_UNIT_Pt);
  m_pNode->SetMeasure(XFA_ATTRIBUTE_Thickness, thickness);
}
void CXFA_Stroke::SetMSThickness(CXFA_Measurement msThinkness) {
  if (!m_pNode) {
    return;
  }
  m_pNode->SetMeasure(XFA_ATTRIBUTE_Thickness, msThinkness);
}
FX_ARGB CXFA_Stroke::GetColor() const {
  if (!m_pNode) {
    return 0xFF000000;
  }
  CXFA_Node* pNode = m_pNode->GetChild(0, XFA_ELEMENT_Color);
  if (!pNode) {
    return 0xFF000000;
  }
  CFX_WideStringC wsColor;
  pNode->TryCData(XFA_ATTRIBUTE_Value, wsColor);
  return XFA_WStringToColor(wsColor);
}
void CXFA_Stroke::SetColor(FX_ARGB argb) {
  if (!m_pNode) {
    return;
  }
  CXFA_Node* pNode = m_pNode->GetProperty(0, XFA_ELEMENT_Color);
  CFX_WideString wsColor;
  int a, r, g, b;
  ArgbDecode(argb, a, r, g, b);
  wsColor.Format(L"%d,%d,%d", r, g, b);
  pNode->SetCData(XFA_ATTRIBUTE_Value, wsColor);
}
int32_t CXFA_Stroke::GetJoinType() const {
  return m_pNode ? m_pNode->GetEnum(XFA_ATTRIBUTE_Join)
                 : XFA_ATTRIBUTEENUM_Square;
}
FX_BOOL CXFA_Stroke::IsInverted() const {
  return m_pNode ? m_pNode->GetBoolean(XFA_ATTRIBUTE_Inverted) : FALSE;
}
FX_FLOAT CXFA_Stroke::GetRadius() const {
  return m_pNode ? m_pNode->GetMeasure(XFA_ATTRIBUTE_Radius).ToUnit(XFA_UNIT_Pt)
                 : 0;
}
FX_BOOL CXFA_Stroke::SameStyles(CXFA_Stroke stroke, FX_DWORD dwFlags) const {
  if (m_pNode == (CXFA_Node*)stroke) {
    return TRUE;
  }
  if (FXSYS_fabs(GetThickness() - stroke.GetThickness()) >= 0.01f) {
    return FALSE;
  }
  if ((dwFlags & XFA_STROKE_SAMESTYLE_NoPresence) == 0 &&
      IsVisible() != stroke.IsVisible()) {
    return FALSE;
  }
  if (GetStrokeType() != stroke.GetStrokeType()) {
    return FALSE;
  }
  if (GetColor() != stroke.GetColor()) {
    return FALSE;
  }
  if ((dwFlags & XFA_STROKE_SAMESTYLE_Corner) != 0 &&
      FXSYS_fabs(GetRadius() - stroke.GetRadius()) >= 0.01f) {
    return FALSE;
  }
  return TRUE;
}
FX_FLOAT XFA_GetEdgeThickness(const CXFA_StrokeArray& strokes,
                              FX_BOOL b3DStyle,
                              int32_t nIndex) {
  FX_FLOAT fThickness = 0;
  {
    if (strokes[nIndex * 2 + 1].GetPresence() == XFA_ATTRIBUTEENUM_Visible) {
      if (nIndex == 0) {
        fThickness += 2.5f;
      }
      fThickness += strokes[nIndex * 2 + 1].GetThickness() * (b3DStyle ? 4 : 2);
    }
  }
  return fThickness;
}
CXFA_WidgetData::CXFA_WidgetData(CXFA_Node* pNode)
    : CXFA_Data(pNode),
      m_bIsNull(TRUE),
      m_bPreNull(TRUE),
      m_pUiChildNode(NULL),
      m_eUIType(XFA_ELEMENT_UNKNOWN) {}
CXFA_Node* CXFA_WidgetData::GetUIChild() {
  if (m_eUIType == XFA_ELEMENT_UNKNOWN) {
    m_pUiChildNode = XFA_CreateUIChild(m_pNode, m_eUIType);
  }
  return m_pUiChildNode;
}
XFA_ELEMENT CXFA_WidgetData::GetUIType() {
  GetUIChild();
  return m_eUIType;
}
CFX_WideString CXFA_WidgetData::GetRawValue() {
  return m_pNode->GetContent();
}
int32_t CXFA_WidgetData::GetAccess(FX_BOOL bTemplate) {
  if (bTemplate) {
    CXFA_Node* pNode = m_pNode->GetTemplateNode();
    if (pNode) {
      return pNode->GetEnum(XFA_ATTRIBUTE_Access);
    }
    return XFA_ATTRIBUTEENUM_Open;
  }
  CXFA_Node* pNode = m_pNode;
  while (pNode) {
    int32_t iAcc = pNode->GetEnum(XFA_ATTRIBUTE_Access);
    if (iAcc != XFA_ATTRIBUTEENUM_Open) {
      return iAcc;
    }
    pNode =
        pNode->GetNodeItem(XFA_NODEITEM_Parent, XFA_OBJECTTYPE_ContainerNode);
  }
  return XFA_ATTRIBUTEENUM_Open;
}
FX_BOOL CXFA_WidgetData::GetAccessKey(CFX_WideStringC& wsAccessKey) {
  return m_pNode->TryCData(XFA_ATTRIBUTE_AccessKey, wsAccessKey);
}
int32_t CXFA_WidgetData::GetAnchorType() {
  return m_pNode->GetEnum(XFA_ATTRIBUTE_AnchorType);
}
int32_t CXFA_WidgetData::GetColSpan() {
  return m_pNode->GetInteger(XFA_ATTRIBUTE_ColSpan);
}
int32_t CXFA_WidgetData::GetPresence() {
  return m_pNode->GetEnum(XFA_ATTRIBUTE_Presence);
  CXFA_Node* pNode = m_pNode;
  while (pNode && pNode->GetObjectType() == XFA_OBJECTTYPE_ContainerNode) {
    int32_t iAcc = pNode->GetEnum(XFA_ATTRIBUTE_Presence);
    if (iAcc != XFA_ATTRIBUTEENUM_Visible) {
      return iAcc;
    }
    pNode = pNode->GetNodeItem(XFA_NODEITEM_Parent);
  }
  return XFA_ATTRIBUTEENUM_Visible;
}
int32_t CXFA_WidgetData::GetRotate() {
  CXFA_Measurement ms;
  if (!m_pNode->TryMeasure(XFA_ATTRIBUTE_Rotate, ms, FALSE)) {
    return 0;
  }
  int32_t iRotate = FXSYS_round(ms.GetValue());
  iRotate = XFA_MapRotation(iRotate);
  return iRotate / 90 * 90;
}
CXFA_Border CXFA_WidgetData::GetBorder(FX_BOOL bModified) {
  return CXFA_Border(m_pNode->GetProperty(0, XFA_ELEMENT_Border, bModified));
}
CXFA_Caption CXFA_WidgetData::GetCaption(FX_BOOL bModified) {
  return CXFA_Caption(m_pNode->GetProperty(0, XFA_ELEMENT_Caption, bModified));
}
CXFA_Font CXFA_WidgetData::GetFont(FX_BOOL bModified) {
  return CXFA_Font(m_pNode->GetProperty(0, XFA_ELEMENT_Font, bModified));
}
CXFA_Margin CXFA_WidgetData::GetMargin(FX_BOOL bModified) {
  return CXFA_Margin(m_pNode->GetProperty(0, XFA_ELEMENT_Margin, bModified));
}
CXFA_Para CXFA_WidgetData::GetPara(FX_BOOL bModified) {
  return CXFA_Para(m_pNode->GetProperty(0, XFA_ELEMENT_Para, bModified));
}
CXFA_Keep CXFA_WidgetData::GetKeep(FX_BOOL bModified) {
  return CXFA_Keep(m_pNode->GetProperty(0, XFA_ELEMENT_Keep, bModified),
                   m_pNode);
}
void CXFA_WidgetData::GetEventList(CXFA_NodeArray& events) {
  m_pNode->GetNodeList(events, 0, XFA_ELEMENT_Event);
}
int32_t CXFA_WidgetData::GetEventByActivity(int32_t iActivity,
                                            CXFA_NodeArray& events,
                                            FX_BOOL bIsFormReady) {
  CXFA_NodeArray allEvents;
  GetEventList(allEvents);
  int32_t iCount = allEvents.GetSize();
  for (int32_t i = 0; i < iCount; i++) {
    CXFA_Event event(allEvents[i]);
    if (event.GetActivity() == iActivity) {
      if (iActivity == XFA_ATTRIBUTEENUM_Ready) {
        CFX_WideStringC wsRef;
        event.GetRef(wsRef);
        if (bIsFormReady) {
          if (wsRef == CFX_WideStringC(L"$form")) {
            events.Add(allEvents[i]);
          }
        } else {
          if (wsRef == CFX_WideStringC(L"$layout")) {
            events.Add(allEvents[i]);
          }
        }
      } else {
        events.Add(allEvents[i]);
      }
    }
  }
  return events.GetSize();
}
CXFA_Value CXFA_WidgetData::GetDefaultValue(FX_BOOL bModified) {
  CXFA_Node* pTemNode = m_pNode->GetTemplateNode();
  return pTemNode ? pTemNode->GetProperty(0, XFA_ELEMENT_Value, bModified)
                  : NULL;
}
CXFA_Value CXFA_WidgetData::GetFormValue(FX_BOOL bModified) {
  return m_pNode->GetProperty(0, XFA_ELEMENT_Value, bModified);
}
CXFA_Calculate CXFA_WidgetData::GetCalculate(FX_BOOL bModified) {
  return m_pNode->GetProperty(0, XFA_ELEMENT_Calculate, bModified);
}
CXFA_Validate CXFA_WidgetData::GetValidate(FX_BOOL bModified) {
  return m_pNode->GetProperty(0, XFA_ELEMENT_Validate, bModified);
}
CXFA_Variables CXFA_WidgetData::GetVariables(FX_BOOL bModified) {
  return m_pNode->GetProperty(0, XFA_ELEMENT_Variables, bModified);
}
CXFA_Bind CXFA_WidgetData::GetBind(FX_BOOL bModified) {
  return m_pNode->GetProperty(0, XFA_ELEMENT_Bind, bModified);
}
CXFA_Assist CXFA_WidgetData::GetAssist(FX_BOOL bModified) {
  return m_pNode->GetProperty(0, XFA_ELEMENT_Assist, bModified);
}
void CXFA_WidgetData::GetRelevant(CFX_WideStringC& wsRelevant) {
  m_pNode->TryCData(XFA_ATTRIBUTE_Relevant, wsRelevant);
}
FX_BOOL CXFA_WidgetData::GetWidth(FX_FLOAT& fWidth) {
  return TryMeasure(XFA_ATTRIBUTE_W, fWidth);
}
FX_BOOL CXFA_WidgetData::GetHeight(FX_FLOAT& fHeight) {
  return TryMeasure(XFA_ATTRIBUTE_H, fHeight);
}
FX_BOOL CXFA_WidgetData::GetMinWidth(FX_FLOAT& fMinWidth) {
  return TryMeasure(XFA_ATTRIBUTE_MinW, fMinWidth);
}
FX_BOOL CXFA_WidgetData::GetMinHeight(FX_FLOAT& fMinHeight) {
  return TryMeasure(XFA_ATTRIBUTE_MinH, fMinHeight);
}
FX_BOOL CXFA_WidgetData::GetMaxWidth(FX_FLOAT& fMaxWidth) {
  return TryMeasure(XFA_ATTRIBUTE_MaxW, fMaxWidth);
}
FX_BOOL CXFA_WidgetData::GetMaxHeight(FX_FLOAT& fMaxHeight) {
  return TryMeasure(XFA_ATTRIBUTE_MaxH, fMaxHeight);
}
CXFA_BindItems CXFA_WidgetData::GetBindItems() {
  return m_pNode->GetChild(0, XFA_ELEMENT_BindItems);
}
FX_BOOL CXFA_WidgetData::SetAccess(int32_t iAccess, FX_BOOL bNotify) {
  return m_pNode->SetEnum(XFA_ATTRIBUTE_Access, (XFA_ATTRIBUTEENUM)iAccess,
                          bNotify);
}
FX_BOOL CXFA_WidgetData::SetAccessKey(const CFX_WideString& wsAccessKey) {
  return m_pNode->SetCData(XFA_ATTRIBUTE_AccessKey, wsAccessKey);
}
FX_BOOL CXFA_WidgetData::SetAnchorType(int32_t iType) {
  return m_pNode->SetEnum(XFA_ATTRIBUTE_AnchorType, (XFA_ATTRIBUTEENUM)iType);
}
FX_BOOL CXFA_WidgetData::SetColSpan(int32_t iColSpan) {
  return m_pNode->SetInteger(XFA_ATTRIBUTE_ColSpan,
                             (XFA_ATTRIBUTEENUM)iColSpan);
}
FX_BOOL CXFA_WidgetData::SetPresence(int32_t iPresence) {
  return m_pNode->SetEnum(XFA_ATTRIBUTE_Presence, (XFA_ATTRIBUTEENUM)iPresence);
}
FX_BOOL CXFA_WidgetData::SetRotate(int32_t iRotate) {
  iRotate = XFA_MapRotation(iRotate);
  CXFA_Measurement ms((FX_FLOAT)iRotate, XFA_UNIT_Angle);
  return m_pNode->SetMeasure(XFA_ATTRIBUTE_Rotate, ms);
}
FX_BOOL CXFA_WidgetData::SetRelevant(const CFX_WideString& wsRelevant) {
  return m_pNode->SetCData(XFA_ATTRIBUTE_Relevant, wsRelevant);
}
FX_BOOL CXFA_WidgetData::SetStatus(FX_DWORD dwStatus) {
  return FALSE;
}
FX_BOOL CXFA_WidgetData::SetWidth(FX_FLOAT fWidth) {
  return SetMeasure(XFA_ATTRIBUTE_W, fWidth);
}
FX_BOOL CXFA_WidgetData::SetHeight(FX_FLOAT fHeight) {
  return SetMeasure(XFA_ATTRIBUTE_H, fHeight);
}
FX_BOOL CXFA_WidgetData::SetMinWidth(FX_FLOAT fMinWidth) {
  return SetMeasure(XFA_ATTRIBUTE_MinW, fMinWidth);
}
FX_BOOL CXFA_WidgetData::SetMinHeight(FX_FLOAT fMinHeight) {
  return SetMeasure(XFA_ATTRIBUTE_MinH, fMinHeight);
}
FX_BOOL CXFA_WidgetData::SetMaxWidth(FX_FLOAT fMaxWidth) {
  return SetMeasure(XFA_ATTRIBUTE_MaxW, fMaxWidth);
}
FX_BOOL CXFA_WidgetData::SetMaxHeight(FX_FLOAT fMaxHeight) {
  return SetMeasure(XFA_ATTRIBUTE_MaxH, fMaxHeight);
}
FX_BOOL CXFA_WidgetData::SetPos(FX_FLOAT x, FX_FLOAT y) {
  return SetMeasure(XFA_ATTRIBUTE_X, x) && SetMeasure(XFA_ATTRIBUTE_Y, y);
}
FX_BOOL CXFA_WidgetData::SetName(const CFX_WideString& wsName) {
  return m_pNode->SetCData(XFA_ATTRIBUTE_Name, wsName);
}
FX_BOOL CXFA_WidgetData::SetButtonHighlight(int32_t iButtonHighlight) {
  CXFA_Node* pUiChildNode = GetUIChild();
  return pUiChildNode->SetEnum(XFA_ATTRIBUTE_Highlight,
                               (XFA_ATTRIBUTEENUM)iButtonHighlight);
}
FX_BOOL CXFA_WidgetData::SetButtonRollover(const CFX_WideString& wsRollover,
                                           FX_BOOL bRichText) {
  return FALSE;
}
FX_BOOL CXFA_WidgetData::SetButtonDown(const CFX_WideString& wsDown,
                                       FX_BOOL bRichText) {
  return FALSE;
}
FX_BOOL CXFA_WidgetData::SetCheckButtonShape(int32_t iCheckButtonShape) {
  CXFA_Node* pUiChildNode = GetUIChild();
  return pUiChildNode->SetEnum(XFA_ATTRIBUTE_Shape,
                               (XFA_ATTRIBUTEENUM)iCheckButtonShape);
}
FX_BOOL CXFA_WidgetData::SetCheckButtonMark(int32_t iCheckButtonMark) {
  CXFA_Node* pUiChildNode = GetUIChild();
  return pUiChildNode->SetEnum(XFA_ATTRIBUTE_Mark,
                               (XFA_ATTRIBUTEENUM)iCheckButtonMark);
}
FX_BOOL CXFA_WidgetData::SetCheckButtonSize(FX_FLOAT fCheckButtonMark) {
  CXFA_Node* pUiChildNode = GetUIChild();
  if (pUiChildNode) {
    CXFA_Measurement ms(fCheckButtonMark, XFA_UNIT_Pt);
    return pUiChildNode->SetMeasure(XFA_ATTRIBUTE_Size, ms);
  }
  return FALSE;
}
CXFA_Border CXFA_WidgetData::GetUIBorder(FX_BOOL bModified) {
  CXFA_Node* pUIChild = GetUIChild();
  return pUIChild ? pUIChild->GetProperty(0, XFA_ELEMENT_Border, bModified)
                  : NULL;
}
CXFA_Margin CXFA_WidgetData::GetUIMargin(FX_BOOL bModified) {
  CXFA_Node* pUIChild = GetUIChild();
  return pUIChild ? pUIChild->GetProperty(0, XFA_ELEMENT_Margin, bModified)
                  : NULL;
}
void CXFA_WidgetData::GetUIMargin(CFX_RectF& rtUIMargin) {
  rtUIMargin.Reset();
  CXFA_Margin mgUI = GetUIMargin();
  if (!mgUI) {
    return;
  }
  CXFA_Border border = GetUIBorder();
  if (border && border.GetPresence() != XFA_ATTRIBUTEENUM_Visible) {
    return;
  }
  FX_FLOAT fLeftInset, fTopInset, fRightInset, fBottomInset;
  FX_BOOL bLeft = mgUI.GetLeftInset(fLeftInset);
  FX_BOOL bTop = mgUI.GetTopInset(fTopInset);
  FX_BOOL bRight = mgUI.GetRightInset(fRightInset);
  FX_BOOL bBottom = mgUI.GetBottomInset(fBottomInset);
  if (border) {
    FX_BOOL bVisible = FALSE;
    FX_FLOAT fThickness = 0;
    border.Get3DStyle(bVisible, fThickness);
    if (!bLeft || !bTop || !bRight || !bBottom) {
      CXFA_StrokeArray strokes;
      border.GetStrokes(strokes);
      if (!bTop) {
        fTopInset = XFA_GetEdgeThickness(strokes, bVisible, 0);
      }
      if (!bRight) {
        fRightInset = XFA_GetEdgeThickness(strokes, bVisible, 1);
      }
      if (!bBottom) {
        fBottomInset = XFA_GetEdgeThickness(strokes, bVisible, 2);
      }
      if (!bLeft) {
        fLeftInset = XFA_GetEdgeThickness(strokes, bVisible, 3);
      }
    }
  }
  rtUIMargin.Set(fLeftInset, fTopInset, fRightInset, fBottomInset);
}
int32_t CXFA_WidgetData::GetButtonHighlight() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->GetEnum(XFA_ATTRIBUTE_Highlight);
  }
  return XFA_GetAttributeDefaultValue_Enum(
      XFA_ELEMENT_Button, XFA_ATTRIBUTE_Highlight, XFA_XDPPACKET_Form);
}
FX_BOOL CXFA_WidgetData::GetButtonRollover(CFX_WideString& wsRollover,
                                           FX_BOOL& bRichText) {
  if (CXFA_Node* pItems = m_pNode->GetChild(0, XFA_ELEMENT_Items)) {
    CXFA_Node* pText = pItems->GetNodeItem(XFA_NODEITEM_FirstChild);
    while (pText) {
      CFX_WideStringC wsName;
      pText->TryCData(XFA_ATTRIBUTE_Name, wsName);
      if (wsName == FX_WSTRC(L"rollover")) {
        pText->TryContent(wsRollover);
        bRichText = pText->GetClassID() == XFA_ELEMENT_ExData;
        return !wsRollover.IsEmpty();
      }
      pText = pText->GetNodeItem(XFA_NODEITEM_NextSibling);
    }
  }
  return FALSE;
}
FX_BOOL CXFA_WidgetData::GetButtonDown(CFX_WideString& wsDown,
                                       FX_BOOL& bRichText) {
  if (CXFA_Node* pItems = m_pNode->GetChild(0, XFA_ELEMENT_Items)) {
    CXFA_Node* pText = pItems->GetNodeItem(XFA_NODEITEM_FirstChild);
    while (pText) {
      CFX_WideStringC wsName;
      pText->TryCData(XFA_ATTRIBUTE_Name, wsName);
      if (wsName == FX_WSTRC(L"down")) {
        pText->TryContent(wsDown);
        bRichText = pText->GetClassID() == XFA_ELEMENT_ExData;
        return !wsDown.IsEmpty();
      }
      pText = pText->GetNodeItem(XFA_NODEITEM_NextSibling);
    }
  }
  return FALSE;
}
int32_t CXFA_WidgetData::GetCheckButtonShape() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->GetEnum(XFA_ATTRIBUTE_Shape);
  }
  return XFA_GetAttributeDefaultValue_Enum(
      XFA_ELEMENT_CheckButton, XFA_ATTRIBUTE_Shape, XFA_XDPPACKET_Form);
}
int32_t CXFA_WidgetData::GetCheckButtonMark() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->GetEnum(XFA_ATTRIBUTE_Mark);
  }
  return XFA_GetAttributeDefaultValue_Enum(
      XFA_ELEMENT_CheckButton, XFA_ATTRIBUTE_Mark, XFA_XDPPACKET_Form);
}
FX_BOOL CXFA_WidgetData::IsRadioButton() {
  if (CXFA_Node* pParent = m_pNode->GetNodeItem(XFA_NODEITEM_Parent)) {
    return pParent->GetClassID() == XFA_ELEMENT_ExclGroup;
  }
  return FALSE;
}
FX_FLOAT CXFA_WidgetData::GetCheckButtonSize() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->GetMeasure(XFA_ATTRIBUTE_Size).ToUnit(XFA_UNIT_Pt);
  }
  return XFA_GetAttributeDefaultValue_Measure(
             XFA_ELEMENT_CheckButton, XFA_ATTRIBUTE_Size, XFA_XDPPACKET_Form)
      .ToUnit(XFA_UNIT_Pt);
}
FX_BOOL CXFA_WidgetData::IsAllowNeutral() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->GetBoolean(XFA_ATTRIBUTE_AllowNeutral);
  }
  return XFA_GetAttributeDefaultValue_Boolean(
      XFA_ELEMENT_CheckButton, XFA_ATTRIBUTE_AllowNeutral, XFA_XDPPACKET_Form);
}
XFA_CHECKSTATE CXFA_WidgetData::GetCheckState() {
  CFX_WideString wsValue = GetRawValue();
  if (wsValue.IsEmpty()) {
    return XFA_CHECKSTATE_Off;
  }
  if (CXFA_Node* pItems = m_pNode->GetChild(0, XFA_ELEMENT_Items)) {
    CXFA_Node* pText = pItems->GetNodeItem(XFA_NODEITEM_FirstChild);
    int32_t i = 0;
    while (pText) {
      CFX_WideString wsContent;
      if (pText->TryContent(wsContent) && (wsContent == wsValue)) {
        return (XFA_CHECKSTATE)i;
      }
      i++;
      pText = pText->GetNodeItem(XFA_NODEITEM_NextSibling);
    }
  }
  return XFA_CHECKSTATE_Off;
}
void CXFA_WidgetData::SetCheckState(XFA_CHECKSTATE eCheckState,
                                    FX_BOOL bNotify) {
  if (CXFA_WidgetData exclGroup = GetExclGroupNode()) {
    CFX_WideString wsValue;
    if (eCheckState != XFA_CHECKSTATE_Off) {
      if (CXFA_Node* pItems = m_pNode->GetChild(0, XFA_ELEMENT_Items)) {
        CXFA_Node* pText = pItems->GetNodeItem(XFA_NODEITEM_FirstChild);
        if (pText) {
          pText->TryContent(wsValue);
        }
      }
    }
    CXFA_Node* pChild =
        ((CXFA_Node*)exclGroup)->GetNodeItem(XFA_NODEITEM_FirstChild);
    for (; pChild; pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
      if (pChild->GetClassID() != XFA_ELEMENT_Field) {
        continue;
      }
      CXFA_Node* pItem = pChild->GetChild(0, XFA_ELEMENT_Items);
      if (!pItem) {
        continue;
      }
      CXFA_Node* pItemchild = pItem->GetNodeItem(XFA_NODEITEM_FirstChild);
      if (!pItemchild) {
        continue;
      }
      CFX_WideString text = pItemchild->GetContent();
      CFX_WideString wsChildValue = text;
      if (wsValue != text) {
        pItemchild = pItemchild->GetNodeItem(XFA_NODEITEM_NextSibling);
        if (pItemchild) {
          wsChildValue = pItemchild->GetContent();
        } else {
          wsChildValue.Empty();
        }
      }
      CXFA_WidgetData ch(pChild);
      ch.SyncValue(wsChildValue, bNotify);
    }
    exclGroup.SyncValue(wsValue, bNotify);
  } else {
    CXFA_Node* pItems = m_pNode->GetChild(0, XFA_ELEMENT_Items);
    if (!pItems) {
      return;
    }
    int32_t i = -1;
    CXFA_Node* pText = pItems->GetNodeItem(XFA_NODEITEM_FirstChild);
    CFX_WideString wsContent;
    while (pText) {
      i++;
      if (i == eCheckState) {
        pText->TryContent(wsContent);
        break;
      }
      pText = pText->GetNodeItem(XFA_NODEITEM_NextSibling);
    }
    SyncValue(wsContent, bNotify);
  }
}
CXFA_Node* CXFA_WidgetData::GetExclGroupNode() {
  CXFA_Node* pExcl = (CXFA_Node*)m_pNode->GetNodeItem(XFA_NODEITEM_Parent);
  if (!pExcl || pExcl->GetClassID() != XFA_ELEMENT_ExclGroup) {
    return NULL;
  }
  return pExcl;
}
CXFA_Node* CXFA_WidgetData::GetSelectedMember() {
  CXFA_Node* pSelectedMember = NULL;
  CFX_WideString wsState = GetRawValue();
  if (wsState.IsEmpty()) {
    return pSelectedMember;
  }
  for (CXFA_Node* pNode =
           (CXFA_Node*)m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
       pNode != NULL; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    CXFA_WidgetData widgetData(pNode);
    if (widgetData.GetCheckState() == XFA_CHECKSTATE_On) {
      pSelectedMember = pNode;
      break;
    }
  }
  return pSelectedMember;
}
CXFA_Node* CXFA_WidgetData::SetSelectedMember(const CFX_WideStringC& wsName,
                                              FX_BOOL bNotify) {
  CXFA_Node* pSelectedMember = NULL;
  FX_DWORD nameHash =
      FX_HashCode_String_GetW(wsName.GetPtr(), wsName.GetLength());
  for (CXFA_Node* pNode =
           (CXFA_Node*)m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
       pNode != NULL; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetNameHash() == nameHash) {
      CXFA_WidgetData widgetData(pNode);
      widgetData.SetCheckState(XFA_CHECKSTATE_On, bNotify);
      pSelectedMember = pNode;
      break;
    }
  }
  return pSelectedMember;
}
void CXFA_WidgetData::SetSelectedMemberByValue(const CFX_WideStringC& wsValue,
                                               FX_BOOL bNotify,
                                               FX_BOOL bScriptModify,
                                               FX_BOOL bSyncData) {
  CFX_WideString wsExclGroup;
  for (CXFA_Node* pNode = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild); pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetClassID() != XFA_ELEMENT_Field) {
      continue;
    }
    CXFA_Node* pItem = pNode->GetChild(0, XFA_ELEMENT_Items);
    if (!pItem) {
      continue;
    }
    CXFA_Node* pItemchild = pItem->GetNodeItem(XFA_NODEITEM_FirstChild);
    if (!pItemchild) {
      continue;
    }
    CFX_WideString wsChildValue = pItemchild->GetContent();
    if (wsValue != wsChildValue) {
      pItemchild = pItemchild->GetNodeItem(XFA_NODEITEM_NextSibling);
      if (pItemchild) {
        wsChildValue = pItemchild->GetContent();
      } else {
        wsChildValue.Empty();
      }
    } else {
      wsExclGroup = wsValue;
    }
    pNode->SetContent(wsChildValue, wsChildValue, bNotify, bScriptModify,
                      FALSE);
  }
  if (m_pNode) {
    m_pNode->SetContent(wsExclGroup, wsExclGroup, bNotify, bScriptModify,
                        bSyncData);
  }
}
CXFA_Node* CXFA_WidgetData::GetExclGroupFirstMember() {
  CXFA_Node* pExcl = GetNode();
  if (!pExcl) {
    return NULL;
  }
  CXFA_Node* pNode = pExcl->GetNodeItem(XFA_NODEITEM_FirstChild);
  while (pNode) {
    if (pNode->GetClassID() == XFA_ELEMENT_Field) {
      return pNode;
    }
    pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling);
  }
  return NULL;
}
CXFA_Node* CXFA_WidgetData::GetExclGroupNextMember(CXFA_Node* pNode) {
  if (!pNode) {
    return NULL;
  }
  CXFA_Node* pNodeField = pNode->GetNodeItem(XFA_NODEITEM_NextSibling);
  while (pNodeField) {
    if (pNodeField->GetClassID() == XFA_ELEMENT_Field) {
      return pNodeField;
    }
    pNodeField = pNodeField->GetNodeItem(XFA_NODEITEM_NextSibling);
  }
  return NULL;
}
int32_t CXFA_WidgetData::GetChoiceListCommitOn() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->GetEnum(XFA_ATTRIBUTE_CommitOn);
  }
  return XFA_GetAttributeDefaultValue_Enum(
      XFA_ELEMENT_ChoiceList, XFA_ATTRIBUTE_CommitOn, XFA_XDPPACKET_Form);
}
FX_BOOL CXFA_WidgetData::IsChoiceListAllowTextEntry() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->GetBoolean(XFA_ATTRIBUTE_TextEntry);
  }
  return XFA_GetAttributeDefaultValue_Boolean(
      XFA_ELEMENT_ChoiceList, XFA_ATTRIBUTE_TextEntry, XFA_XDPPACKET_Form);
}
int32_t CXFA_WidgetData::GetChoiceListOpen() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->GetEnum(XFA_ATTRIBUTE_Open);
  }
  return XFA_GetAttributeDefaultValue_Enum(
      XFA_ELEMENT_ChoiceList, XFA_ATTRIBUTE_Open, XFA_XDPPACKET_Form);
}
FX_BOOL CXFA_WidgetData::IsListBox() {
  int32_t iOpenMode = GetChoiceListOpen();
  return (iOpenMode == XFA_ATTRIBUTEENUM_Always ||
          iOpenMode == XFA_ATTRIBUTEENUM_MultiSelect);
}
int32_t CXFA_WidgetData::CountChoiceListItems(FX_BOOL bSaveValue) {
  CXFA_NodeArray pItems;
  CXFA_Node* pItem = NULL;
  int32_t iCount = 0;
  CXFA_Node* pNode = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  for (; pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetClassID() != XFA_ELEMENT_Items) {
      continue;
    }
    iCount++;
    pItems.Add(pNode);
    if (iCount == 2) {
      break;
    }
  }
  if (iCount == 0) {
    return 0;
  }
  pItem = pItems[0];
  if (iCount > 1) {
    FX_BOOL bItemOneHasSave = pItems[0]->GetBoolean(XFA_ATTRIBUTE_Save);
    FX_BOOL bItemTwoHasSave = pItems[1]->GetBoolean(XFA_ATTRIBUTE_Save);
    if (bItemOneHasSave != bItemTwoHasSave && bSaveValue == bItemTwoHasSave) {
      pItem = pItems[1];
    }
  }
  pItems.RemoveAll();
  return pItem->CountChildren(XFA_ELEMENT_UNKNOWN);
}
FX_BOOL CXFA_WidgetData::GetChoiceListItem(CFX_WideString& wsText,
                                           int32_t nIndex,
                                           FX_BOOL bSaveValue) {
  wsText.Empty();
  CXFA_NodeArray pItemsArray;
  CXFA_Node* pItems = NULL;
  int32_t iCount = 0;
  CXFA_Node* pNode = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  for (; pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetClassID() != XFA_ELEMENT_Items) {
      continue;
    }
    iCount++;
    pItemsArray.Add(pNode);
    if (iCount == 2) {
      break;
    }
  }
  if (iCount == 0) {
    return FALSE;
  }
  pItems = pItemsArray[0];
  if (iCount > 1) {
    FX_BOOL bItemOneHasSave = pItemsArray[0]->GetBoolean(XFA_ATTRIBUTE_Save);
    FX_BOOL bItemTwoHasSave = pItemsArray[1]->GetBoolean(XFA_ATTRIBUTE_Save);
    if (bItemOneHasSave != bItemTwoHasSave && bSaveValue == bItemTwoHasSave) {
      pItems = pItemsArray[1];
    }
  }
  if (pItems) {
    CXFA_Node* pItem = pItems->GetChild(nIndex, XFA_ELEMENT_UNKNOWN);
    if (pItem != NULL) {
      pItem->TryContent(wsText);
      return TRUE;
    }
  }
  return FALSE;
}
void CXFA_WidgetData::GetChoiceListItems(CFX_WideStringArray& wsTextArray,
                                         FX_BOOL bSaveValue) {
  CXFA_NodeArray pItems;
  CXFA_Node* pItem = NULL;
  int32_t iCount = 0;
  CXFA_Node* pNode = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  for (; pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetClassID() != XFA_ELEMENT_Items) {
      continue;
    }
    iCount++;
    pItems.Add(pNode);
    if (iCount == 2) {
      break;
    }
  }
  if (iCount == 0) {
    return;
  }
  pItem = pItems[0];
  if (iCount > 1) {
    FX_BOOL bItemOneHasSave = pItems[0]->GetBoolean(XFA_ATTRIBUTE_Save);
    FX_BOOL bItemTwoHasSave = pItems[1]->GetBoolean(XFA_ATTRIBUTE_Save);
    if (bItemOneHasSave != bItemTwoHasSave && bSaveValue == bItemTwoHasSave) {
      pItem = pItems[1];
    }
  }
  pItems.RemoveAll();
  pNode = pItem->GetNodeItem(XFA_NODEITEM_FirstChild);
  for (; pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    pNode->TryContent(wsTextArray.Add());
  }
}
int32_t CXFA_WidgetData::CountSelectedItems() {
  CFX_WideStringArray wsValueArray;
  GetSelectedItemsValue(wsValueArray);
  if (IsListBox() || !IsChoiceListAllowTextEntry()) {
    return wsValueArray.GetSize();
  }
  int32_t iSelected = 0;
  CFX_WideStringArray wsSaveTextArray;
  GetChoiceListItems(wsSaveTextArray, TRUE);
  int32_t iValues = wsValueArray.GetSize();
  for (int32_t i = 0; i < iValues; i++) {
    int32_t iSaves = wsSaveTextArray.GetSize();
    for (int32_t j = 0; j < iSaves; j++) {
      if (wsValueArray[i] == wsSaveTextArray[j]) {
        iSelected++;
        break;
      }
    }
  }
  return iSelected;
}
int32_t CXFA_WidgetData::GetSelectedItem(int32_t nIndex) {
  CFX_WideStringArray wsValueArray;
  GetSelectedItemsValue(wsValueArray);
  CFX_WideStringArray wsSaveTextArray;
  GetChoiceListItems(wsSaveTextArray, TRUE);
  int32_t iSaves = wsSaveTextArray.GetSize();
  for (int32_t j = 0; j < iSaves; j++) {
    if (wsValueArray[nIndex] == wsSaveTextArray[j]) {
      return j;
    }
  }
  return -1;
}
void CXFA_WidgetData::GetSelectedItems(CFX_Int32Array& iSelArray) {
  CFX_WideStringArray wsValueArray;
  GetSelectedItemsValue(wsValueArray);
  int32_t iValues = wsValueArray.GetSize();
  if (iValues < 1) {
    return;
  }
  CFX_WideStringArray wsSaveTextArray;
  GetChoiceListItems(wsSaveTextArray, TRUE);
  int32_t iSaves = wsSaveTextArray.GetSize();
  for (int32_t i = 0; i < iValues; i++) {
    for (int32_t j = 0; j < iSaves; j++) {
      if (wsValueArray[i] == wsSaveTextArray[j]) {
        iSelArray.Add(j);
        break;
      }
    }
  }
}
void CXFA_WidgetData::GetSelectedItemsValue(
    CFX_WideStringArray& wsSelTextArray) {
  CFX_WideString wsValue = GetRawValue();
  if (GetChoiceListOpen() == XFA_ATTRIBUTEENUM_MultiSelect) {
    if (!wsValue.IsEmpty()) {
      int32_t iStart = 0;
      int32_t iLength = wsValue.GetLength();
      int32_t iEnd = wsValue.Find(L'\n', iStart);
      iEnd = (iEnd == -1) ? iLength : iEnd;
      while (iEnd >= iStart) {
        wsSelTextArray.Add(wsValue.Mid(iStart, iEnd - iStart));
        iStart = iEnd + 1;
        if (iStart >= iLength) {
          break;
        }
        iEnd = wsValue.Find(L'\n', iStart);
        if (iEnd < 0) {
          wsSelTextArray.Add(wsValue.Mid(iStart, iLength - iStart));
        }
      }
    }
  } else {
    wsSelTextArray.Add(wsValue);
  }
}
FX_BOOL CXFA_WidgetData::GetItemState(int32_t nIndex) {
  if (nIndex < 0) {
    return FALSE;
  }
  CFX_WideStringArray wsSaveTextArray;
  GetChoiceListItems(wsSaveTextArray, TRUE);
  if (wsSaveTextArray.GetSize() <= nIndex) {
    return FALSE;
  }
  CFX_WideStringArray wsValueArray;
  GetSelectedItemsValue(wsValueArray);
  int32_t iValues = wsValueArray.GetSize();
  for (int32_t j = 0; j < iValues; j++) {
    if (wsValueArray[j] == wsSaveTextArray[nIndex]) {
      return TRUE;
    }
  }
  return FALSE;
}
void CXFA_WidgetData::SetItemState(int32_t nIndex,
                                   FX_BOOL bSelected,
                                   FX_BOOL bNotify,
                                   FX_BOOL bScriptModify,
                                   FX_BOOL bSyncData) {
  if (nIndex < 0) {
    return;
  }
  CFX_WideStringArray wsSaveTextArray;
  GetChoiceListItems(wsSaveTextArray, TRUE);
  if (wsSaveTextArray.GetSize() <= nIndex) {
    return;
  }
  int32_t iSel = -1;
  CFX_WideStringArray wsValueArray;
  GetSelectedItemsValue(wsValueArray);
  int32_t iValues = wsValueArray.GetSize();
  for (int32_t j = 0; j < iValues; j++) {
    if (wsValueArray[j] == wsSaveTextArray[nIndex]) {
      iSel = j;
      break;
    }
  }
  if (GetChoiceListOpen() == XFA_ATTRIBUTEENUM_MultiSelect) {
    if (bSelected) {
      if (iSel < 0) {
        CFX_WideString wsValue = GetRawValue();
        if (!wsValue.IsEmpty()) {
          wsValue += L"\n";
        }
        wsValue += wsSaveTextArray[nIndex];
        m_pNode->SetContent(wsValue, wsValue, bNotify, bScriptModify,
                            bSyncData);
      }
    } else if (iSel >= 0) {
      CFX_Int32Array iSelArray;
      GetSelectedItems(iSelArray);
      for (int32_t i = 0; i < iSelArray.GetSize(); i++) {
        if (iSelArray[i] == nIndex) {
          iSelArray.RemoveAt(i);
          break;
        }
      }
      SetSelectdItems(iSelArray, bNotify, bScriptModify, bSyncData);
    }
  } else {
    if (bSelected) {
      if (iSel < 0) {
        CFX_WideString wsSaveText = wsSaveTextArray[nIndex];
        CFX_WideString wsFormatText(wsSaveText);
        GetFormatDataValue(wsSaveText, wsFormatText);
        m_pNode->SetContent(wsSaveText, wsFormatText, bNotify, bScriptModify,
                            bSyncData);
      }
    } else if (iSel >= 0) {
      m_pNode->SetContent(CFX_WideString(), CFX_WideString(), bNotify,
                          bScriptModify, bSyncData);
    }
  }
}
void CXFA_WidgetData::SetSelectdItems(CFX_Int32Array& iSelArray,
                                      FX_BOOL bNotify,
                                      FX_BOOL bScriptModify,
                                      FX_BOOL bSyncData) {
  CFX_WideString wsValue;
  int32_t iSize = iSelArray.GetSize();
  if (iSize >= 1) {
    CFX_WideStringArray wsSaveTextArray;
    GetChoiceListItems(wsSaveTextArray, TRUE);
    CFX_WideString wsItemValue;
    for (int32_t i = 0; i < iSize; i++) {
      wsItemValue = (iSize == 1)
                        ? wsSaveTextArray[iSelArray[i]]
                        : wsSaveTextArray[iSelArray[i]] + FX_WSTRC(L"\n");
      wsValue += wsItemValue;
    }
  }
  CFX_WideString wsFormat(wsValue);
  if (GetChoiceListOpen() != XFA_ATTRIBUTEENUM_MultiSelect) {
    GetFormatDataValue(wsValue, wsFormat);
  }
  m_pNode->SetContent(wsValue, wsFormat, bNotify, bScriptModify, bSyncData);
}
void CXFA_WidgetData::ClearAllSelections() {
  CXFA_Node* pBind = m_pNode->GetBindData();
  if (pBind && GetChoiceListOpen() == XFA_ATTRIBUTEENUM_MultiSelect) {
    while (CXFA_Node* pChildNode =
               pBind->GetNodeItem(XFA_NODEITEM_FirstChild)) {
      pBind->RemoveChild(pChildNode);
    }
  } else {
    SyncValue(CFX_WideString(), FALSE);
  }
}
void CXFA_WidgetData::InsertItem(const CFX_WideString& wsLabel,
                                 const CFX_WideString& wsValue,
                                 int32_t nIndex,
                                 FX_BOOL bNotify) {
  CFX_WideString wsNewValue(wsValue);
  if (wsNewValue.IsEmpty()) {
    wsNewValue = wsLabel;
  }
  CXFA_NodeArray listitems;
  int32_t iCount = 0;
  CXFA_Node* pItemNode = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  for (; pItemNode;
       pItemNode = pItemNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pItemNode->GetClassID() != XFA_ELEMENT_Items) {
      continue;
    }
    listitems.Add(pItemNode);
    iCount++;
  }
  if (iCount < 1) {
    CXFA_Node* pItems = m_pNode->CreateSamePacketNode(XFA_ELEMENT_Items);
    m_pNode->InsertChild(-1, pItems);
    InsertListTextItem(pItems, wsLabel, nIndex);
    CXFA_Node* pSaveItems = m_pNode->CreateSamePacketNode(XFA_ELEMENT_Items);
    m_pNode->InsertChild(-1, pSaveItems);
    pSaveItems->SetBoolean(XFA_ATTRIBUTE_Save, TRUE);
    InsertListTextItem(pSaveItems, wsNewValue, nIndex);
  } else if (iCount > 1) {
    for (int32_t i = 0; i < 2; i++) {
      CXFA_Node* pNode = listitems[i];
      FX_BOOL bHasSave = pNode->GetBoolean(XFA_ATTRIBUTE_Save);
      if (bHasSave) {
        InsertListTextItem(pNode, wsNewValue, nIndex);
      } else {
        InsertListTextItem(pNode, wsLabel, nIndex);
      }
    }
  } else {
    CXFA_Node* pNode = listitems[0];
    pNode->SetBoolean(XFA_ATTRIBUTE_Save, FALSE);
    pNode->SetEnum(XFA_ATTRIBUTE_Presence, XFA_ATTRIBUTEENUM_Visible);
    CXFA_Node* pSaveItems = m_pNode->CreateSamePacketNode(XFA_ELEMENT_Items);
    m_pNode->InsertChild(-1, pSaveItems);
    pSaveItems->SetBoolean(XFA_ATTRIBUTE_Save, TRUE);
    pSaveItems->SetEnum(XFA_ATTRIBUTE_Presence, XFA_ATTRIBUTEENUM_Hidden);
    listitems.RemoveAll();
    CXFA_Node* pListNode = pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
    int32_t i = 0;
    while (pListNode) {
      CFX_WideString wsOldValue;
      pListNode->TryContent(wsOldValue);
      InsertListTextItem(pSaveItems, wsOldValue, i);
      i++;
      pListNode = pListNode->GetNodeItem(XFA_NODEITEM_NextSibling);
    }
    InsertListTextItem(pNode, wsLabel, nIndex);
    InsertListTextItem(pSaveItems, wsNewValue, nIndex);
  }
  if (!bNotify) {
    return;
  }
  m_pNode->GetDocument()->GetNotify()->OnWidgetDataEvent(
      this, XFA_WIDGETEVENT_ListItemAdded, (void*)(const FX_WCHAR*)wsLabel,
      (void*)(const FX_WCHAR*)wsValue, (void*)(uintptr_t)nIndex);
}
void CXFA_WidgetData::GetItemLabel(const CFX_WideStringC& wsValue,
                                   CFX_WideString& wsLabel) {
  int32_t iCount = 0;
  CXFA_NodeArray listitems;
  CXFA_Node* pItems = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  for (; pItems; pItems = pItems->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pItems->GetClassID() != XFA_ELEMENT_Items) {
      continue;
    }
    iCount++;
    listitems.Add(pItems);
  }
  if (iCount <= 1) {
    wsLabel = wsValue;
  } else {
    CXFA_Node* pLabelItems = listitems[0];
    FX_BOOL bSave = pLabelItems->GetBoolean(XFA_ATTRIBUTE_Save);
    CXFA_Node* pSaveItems = NULL;
    if (bSave) {
      pSaveItems = pLabelItems;
      pLabelItems = listitems[1];
    } else {
      pSaveItems = listitems[1];
    }
    iCount = 0;
    int32_t iSearch = -1;
    CFX_WideString wsContent;
    CXFA_Node* pChildItem = pSaveItems->GetNodeItem(XFA_NODEITEM_FirstChild);
    for (; pChildItem;
         pChildItem = pChildItem->GetNodeItem(XFA_NODEITEM_NextSibling)) {
      pChildItem->TryContent(wsContent);
      if (wsContent == wsValue) {
        iSearch = iCount;
        break;
      }
      iCount++;
    }
    if (iSearch < 0) {
      return;
    }
    if (CXFA_Node* pText =
            pLabelItems->GetChild(iSearch, XFA_ELEMENT_UNKNOWN)) {
      pText->TryContent(wsLabel);
    }
  }
}
void CXFA_WidgetData::GetItemValue(const CFX_WideStringC& wsLabel,
                                   CFX_WideString& wsValue) {
  int32_t iCount = 0;
  CXFA_NodeArray listitems;
  CXFA_Node* pItems = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  for (; pItems; pItems = pItems->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pItems->GetClassID() != XFA_ELEMENT_Items) {
      continue;
    }
    iCount++;
    listitems.Add(pItems);
  }
  if (iCount <= 1) {
    wsValue = wsLabel;
  } else {
    CXFA_Node* pLabelItems = listitems[0];
    FX_BOOL bSave = pLabelItems->GetBoolean(XFA_ATTRIBUTE_Save);
    CXFA_Node* pSaveItems = NULL;
    if (bSave) {
      pSaveItems = pLabelItems;
      pLabelItems = listitems[1];
    } else {
      pSaveItems = listitems[1];
    }
    iCount = 0;
    int32_t iSearch = -1;
    CFX_WideString wsContent;
    CXFA_Node* pChildItem = pLabelItems->GetNodeItem(XFA_NODEITEM_FirstChild);
    for (; pChildItem;
         pChildItem = pChildItem->GetNodeItem(XFA_NODEITEM_NextSibling)) {
      pChildItem->TryContent(wsContent);
      if (wsContent == wsLabel) {
        iSearch = iCount;
        break;
      }
      iCount++;
    }
    if (iSearch < 0) {
      return;
    }
    if (CXFA_Node* pText = pSaveItems->GetChild(iSearch, XFA_ELEMENT_UNKNOWN)) {
      pText->TryContent(wsValue);
    }
  }
}
FX_BOOL CXFA_WidgetData::DeleteItem(int32_t nIndex,
                                    FX_BOOL bNotify,
                                    FX_BOOL bScriptModify,
                                    FX_BOOL bSyncData) {
  FX_BOOL bSetValue = FALSE;
  CXFA_Node* pItems = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  for (; pItems; pItems = pItems->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pItems->GetClassID() != XFA_ELEMENT_Items) {
      continue;
    }
    if (nIndex < 0) {
      while (CXFA_Node* pNode = pItems->GetNodeItem(XFA_NODEITEM_FirstChild)) {
        pItems->RemoveChild(pNode);
      }
    } else {
      if (!bSetValue && pItems->GetBoolean(XFA_ATTRIBUTE_Save)) {
        SetItemState(nIndex, FALSE, TRUE, bScriptModify, bSyncData);
        bSetValue = TRUE;
      }
      int32_t i = 0;
      CXFA_Node* pNode = pItems->GetNodeItem(XFA_NODEITEM_FirstChild);
      while (pNode) {
        if (i == nIndex) {
          pItems->RemoveChild(pNode);
          break;
        }
        i++;
        pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling);
      }
    }
  }
  if (!bNotify) {
    return TRUE;
  }
  m_pNode->GetDocument()->GetNotify()->OnWidgetDataEvent(
      this, XFA_WIDGETEVENT_ListItemRemoved, (void*)(uintptr_t)nIndex);
  return TRUE;
}
int32_t CXFA_WidgetData::GetHorizontalScrollPolicy() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->GetEnum(XFA_ATTRIBUTE_HScrollPolicy);
  }
  return XFA_ATTRIBUTEENUM_Auto;
}
int32_t CXFA_WidgetData::GetNumberOfCells() {
  CXFA_Node* pUIChild = GetUIChild();
  if (!pUIChild) {
    return -1;
  }
  if (CXFA_Node* pNode = pUIChild->GetChild(0, XFA_ELEMENT_Comb)) {
    return pNode->GetInteger(XFA_ATTRIBUTE_NumberOfCells);
  }
  return -1;
}
FX_BOOL CXFA_WidgetData::IsDateTimeEditUsePicker() {
  return TRUE;
}
CFX_WideString CXFA_WidgetData::GetBarcodeType() {
  CXFA_Node* pUIChild = GetUIChild();
  return pUIChild ? pUIChild->GetCData(XFA_ATTRIBUTE_Type) : NULL;
}
FX_BOOL CXFA_WidgetData::GetBarcodeAttribute_CharEncoding(int32_t& val) {
  CXFA_Node* pUIChild = GetUIChild();
  CFX_WideString wsCharEncoding;
  if (pUIChild->TryCData(XFA_ATTRIBUTE_CharEncoding, wsCharEncoding)) {
    if (wsCharEncoding.CompareNoCase(L"UTF-16")) {
      val = CHAR_ENCODING_UNICODE;
      return TRUE;
    } else if (wsCharEncoding.CompareNoCase(L"UTF-8")) {
      val = CHAR_ENCODING_UTF8;
      return TRUE;
    }
  }
  return FALSE;
}
FX_BOOL CXFA_WidgetData::GetBarcodeAttribute_Checksum(int32_t& val) {
  CXFA_Node* pUIChild = GetUIChild();
  XFA_ATTRIBUTEENUM eChecksum;
  if (pUIChild->TryEnum(XFA_ATTRIBUTE_Checksum, eChecksum)) {
    switch (eChecksum) {
      case XFA_ATTRIBUTEENUM_None:
        val = 0;
        return TRUE;
      case XFA_ATTRIBUTEENUM_Auto:
        val = 1;
        return TRUE;
      case XFA_ATTRIBUTEENUM_1mod10:
        break;
      case XFA_ATTRIBUTEENUM_1mod10_1mod11:
        break;
      case XFA_ATTRIBUTEENUM_2mod10:
        break;
      default:
        break;
    }
  }
  return FALSE;
}
FX_BOOL CXFA_WidgetData::GetBarcodeAttribute_DataLength(int32_t& val) {
  CXFA_Node* pUIChild = GetUIChild();
  CFX_WideString wsDataLength;
  if (pUIChild->TryCData(XFA_ATTRIBUTE_DataLength, wsDataLength)) {
    val = FXSYS_wtoi(wsDataLength);
    return TRUE;
  }
  return FALSE;
}
FX_BOOL CXFA_WidgetData::GetBarcodeAttribute_StartChar(FX_CHAR& val) {
  CXFA_Node* pUIChild = GetUIChild();
  CFX_WideStringC wsStartEndChar;
  if (pUIChild->TryCData(XFA_ATTRIBUTE_StartChar, wsStartEndChar)) {
    if (wsStartEndChar.GetLength()) {
      val = (FX_CHAR)wsStartEndChar.GetAt(0);
      return TRUE;
    }
  }
  return FALSE;
}
FX_BOOL CXFA_WidgetData::GetBarcodeAttribute_EndChar(FX_CHAR& val) {
  CXFA_Node* pUIChild = GetUIChild();
  CFX_WideStringC wsStartEndChar;
  if (pUIChild->TryCData(XFA_ATTRIBUTE_EndChar, wsStartEndChar)) {
    if (wsStartEndChar.GetLength()) {
      val = (FX_CHAR)wsStartEndChar.GetAt(0);
      return TRUE;
    }
  }
  return FALSE;
}
FX_BOOL CXFA_WidgetData::GetBarcodeAttribute_ECLevel(int32_t& val) {
  CXFA_Node* pUIChild = GetUIChild();
  CFX_WideString wsECLevel;
  if (pUIChild->TryCData(XFA_ATTRIBUTE_ErrorCorrectionLevel, wsECLevel)) {
    val = FXSYS_wtoi(wsECLevel);
    return TRUE;
  }
  return FALSE;
}
FX_BOOL CXFA_WidgetData::GetBarcodeAttribute_ModuleWidth(int32_t& val) {
  CXFA_Node* pUIChild = GetUIChild();
  CXFA_Measurement mModuleWidthHeight;
  if (pUIChild->TryMeasure(XFA_ATTRIBUTE_ModuleWidth, mModuleWidthHeight)) {
    val = (int32_t)mModuleWidthHeight.ToUnit(XFA_UNIT_Pt);
    return TRUE;
  }
  return FALSE;
}
FX_BOOL CXFA_WidgetData::GetBarcodeAttribute_ModuleHeight(int32_t& val) {
  CXFA_Node* pUIChild = GetUIChild();
  CXFA_Measurement mModuleWidthHeight;
  if (pUIChild->TryMeasure(XFA_ATTRIBUTE_ModuleHeight, mModuleWidthHeight)) {
    val = (int32_t)mModuleWidthHeight.ToUnit(XFA_UNIT_Pt);
    return TRUE;
  }
  return FALSE;
}
FX_BOOL CXFA_WidgetData::GetBarcodeAttribute_PrintChecksum(FX_BOOL& val) {
  CXFA_Node* pUIChild = GetUIChild();
  FX_BOOL bPrintCheckDigit;
  if (pUIChild->TryBoolean(XFA_ATTRIBUTE_PrintCheckDigit, bPrintCheckDigit)) {
    val = bPrintCheckDigit;
    return TRUE;
  }
  return FALSE;
}
FX_BOOL CXFA_WidgetData::GetBarcodeAttribute_TextLocation(int32_t& val) {
  CXFA_Node* pUIChild = GetUIChild();
  XFA_ATTRIBUTEENUM eTextLocation;
  if (pUIChild->TryEnum(XFA_ATTRIBUTE_TextLocation, eTextLocation)) {
    switch (eTextLocation) {
      case XFA_ATTRIBUTEENUM_None:
        val = BC_TEXT_LOC_NONE;
        return TRUE;
      case XFA_ATTRIBUTEENUM_Above:
        val = BC_TEXT_LOC_ABOVE;
        return TRUE;
      case XFA_ATTRIBUTEENUM_Below:
        val = BC_TEXT_LOC_BELOW;
        return TRUE;
      case XFA_ATTRIBUTEENUM_AboveEmbedded:
        val = BC_TEXT_LOC_ABOVEEMBED;
        return TRUE;
      case XFA_ATTRIBUTEENUM_BelowEmbedded:
        val = BC_TEXT_LOC_BELOWEMBED;
        return TRUE;
      default:
        break;
    }
  }
  return FALSE;
}
FX_BOOL CXFA_WidgetData::GetBarcodeAttribute_Truncate(FX_BOOL& val) {
  CXFA_Node* pUIChild = GetUIChild();
  FX_BOOL bTruncate;
  if (pUIChild->TryBoolean(XFA_ATTRIBUTE_Truncate, bTruncate)) {
    val = bTruncate;
    return TRUE;
  }
  return FALSE;
}
FX_BOOL CXFA_WidgetData::GetBarcodeAttribute_WideNarrowRatio(FX_FLOAT& val) {
  CXFA_Node* pUIChild = GetUIChild();
  CFX_WideString wsWideNarrowRatio;
  if (pUIChild->TryCData(XFA_ATTRIBUTE_WideNarrowRatio, wsWideNarrowRatio)) {
    FX_STRSIZE ptPos = wsWideNarrowRatio.Find(':');
    FX_FLOAT fRatio = 0;
    if (ptPos >= 0) {
      fRatio = (FX_FLOAT)FXSYS_wtoi(wsWideNarrowRatio);
    } else {
      int32_t fA, fB;
      fA = FXSYS_wtoi(wsWideNarrowRatio.Left(ptPos));
      fB = FXSYS_wtoi(wsWideNarrowRatio.Mid(ptPos + 1));
      if (fB) {
        fRatio = (FX_FLOAT)fA / fB;
      }
    }
    val = fRatio;
    return TRUE;
  }
  return FALSE;
}
void CXFA_WidgetData::GetPasswordChar(CFX_WideString& wsPassWord) {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    pUIChild->TryCData(XFA_ATTRIBUTE_PasswordChar, wsPassWord);
  } else {
    wsPassWord = XFA_GetAttributeDefaultValue_Cdata(XFA_ELEMENT_PasswordEdit,
                                                    XFA_ATTRIBUTE_PasswordChar,
                                                    XFA_XDPPACKET_Form);
  }
}
FX_BOOL CXFA_WidgetData::IsAllowRichText() {
  CXFA_Node* pUIChild = GetUIChild();
  FX_BOOL bValue = FALSE;
  if (pUIChild &&
      pUIChild->TryBoolean(XFA_ATTRIBUTE_AllowRichText, bValue, FALSE)) {
    return bValue;
  }
  if (CXFA_Node* pNode = m_pNode->GetChild(0, XFA_ELEMENT_Value)) {
    if (CXFA_Node* pChild = pNode->GetNodeItem(XFA_NODEITEM_FirstChild)) {
      return pChild->GetClassID() == XFA_ELEMENT_ExData;
    }
  }
  return FALSE;
}
FX_BOOL CXFA_WidgetData::IsMultiLine() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->GetBoolean(XFA_ATTRIBUTE_MultiLine);
  }
  return XFA_GetAttributeDefaultValue_Boolean(
      XFA_ELEMENT_TextEdit, XFA_ATTRIBUTE_MultiLine, XFA_XDPPACKET_Form);
}
int32_t CXFA_WidgetData::GetVerticalScrollPolicy() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->GetEnum(XFA_ATTRIBUTE_VScrollPolicy);
  }
  return XFA_GetAttributeDefaultValue_Enum(
      XFA_ELEMENT_TextEdit, XFA_ATTRIBUTE_VScrollPolicy, XFA_XDPPACKET_Form);
}
int32_t CXFA_WidgetData::GetMaxChars(XFA_ELEMENT& eType) {
  if (CXFA_Node* pNode = m_pNode->GetChild(0, XFA_ELEMENT_Value)) {
    if (CXFA_Node* pChild = pNode->GetNodeItem(XFA_NODEITEM_FirstChild)) {
      switch (pChild->GetClassID()) {
        case XFA_ELEMENT_Text:
          eType = XFA_ELEMENT_Text;
          return pChild->GetInteger(XFA_ATTRIBUTE_MaxChars);
        case XFA_ELEMENT_ExData: {
          eType = XFA_ELEMENT_ExData;
          int32_t iMax = pChild->GetInteger(XFA_ATTRIBUTE_MaxLength);
          return iMax < 0 ? 0 : iMax;
        }
        default:
          break;
      }
    }
  }
  return 0;
}
FX_BOOL CXFA_WidgetData::GetFracDigits(int32_t& iFracDigits) {
  if (CXFA_Node* pNode = m_pNode->GetChild(0, XFA_ELEMENT_Value)) {
    if (CXFA_Node* pChild = pNode->GetChild(0, XFA_ELEMENT_Decimal)) {
      return pChild->TryInteger(XFA_ATTRIBUTE_FracDigits, iFracDigits);
    }
  }
  iFracDigits = -1;
  return FALSE;
}
FX_BOOL CXFA_WidgetData::GetLeadDigits(int32_t& iLeadDigits) {
  if (CXFA_Node* pNode = m_pNode->GetChild(0, XFA_ELEMENT_Value)) {
    if (CXFA_Node* pChild = pNode->GetChild(0, XFA_ELEMENT_Decimal)) {
      return pChild->TryInteger(XFA_ATTRIBUTE_LeadDigits, iLeadDigits);
    }
  }
  iLeadDigits = -1;
  return FALSE;
}
CFX_WideString XFA_NumericLimit(const CFX_WideString& wsValue,
                                int32_t iLead,
                                int32_t iTread) {
  if ((iLead == -1) && (iTread == -1)) {
    return wsValue;
  }
  CFX_WideString wsRet;
  int32_t iLead_ = 0, iTread_ = -1;
  int32_t iCount = wsValue.GetLength();
  if (iCount == 0) {
    return wsValue;
  }
  int32_t i = 0;
  if (wsValue[i] == L'-') {
    wsRet += L'-';
    i++;
  }
  for (; i < iCount; i++) {
    FX_WCHAR wc = wsValue[i];
    if (XFA_IsDigit(wc)) {
      if (iLead >= 0) {
        iLead_++;
        if (iLead_ > iLead) {
          return L"0";
        }
      } else if (iTread_ >= 0) {
        iTread_++;
        if (iTread_ > iTread) {
          if (iTread != -1) {
            CFX_Decimal wsDeci = CFX_Decimal(wsValue);
            wsDeci.SetScale(iTread);
            wsRet = wsDeci;
          }
          return wsRet;
        }
      }
    } else if (wc == L'.') {
      iTread_ = 0;
      iLead = -1;
    }
    wsRet += wc;
  }
  return wsRet;
}
FX_BOOL CXFA_WidgetData::SetValue(const CFX_WideString& wsValue,
                                  XFA_VALUEPICTURE eValueType) {
  if (wsValue.IsEmpty()) {
    SyncValue(wsValue, TRUE);
    return TRUE;
  }
  this->m_bPreNull = this->m_bIsNull;
  this->m_bIsNull = FALSE;
  CFX_WideString wsNewText(wsValue);
  CFX_WideString wsPicture;
  GetPictureContent(wsPicture, eValueType);
  FX_BOOL bValidate = TRUE;
  FX_BOOL bSyncData = FALSE;
  CXFA_Node* pNode = GetUIChild();
  if (!pNode) {
    return TRUE;
  }
  XFA_ELEMENT uiType = pNode->GetClassID();
  if (!wsPicture.IsEmpty()) {
    CXFA_LocaleMgr* pLocalMgr = m_pNode->GetDocument()->GetLocalMgr();
    IFX_Locale* pLocale = GetLocal();
    CXFA_LocaleValue widgetValue = XFA_GetLocaleValue(this);
    bValidate =
        widgetValue.ValidateValue(wsValue, wsPicture, pLocale, &wsPicture);
    if (bValidate) {
      widgetValue = CXFA_LocaleValue(widgetValue.GetType(), wsNewText,
                                     wsPicture, pLocale, pLocalMgr);
      wsNewText = widgetValue.GetValue();
      if (uiType == XFA_ELEMENT_NumericEdit) {
        int32_t iLeadDigits = 0;
        int32_t iFracDigits = 0;
        GetLeadDigits(iLeadDigits);
        GetFracDigits(iFracDigits);
        wsNewText = XFA_NumericLimit(wsNewText, iLeadDigits, iFracDigits);
      }
      bSyncData = TRUE;
    }
  } else {
    if (uiType == XFA_ELEMENT_NumericEdit) {
      if (wsNewText != FX_WSTRC(L"0")) {
        int32_t iLeadDigits = 0;
        int32_t iFracDigits = 0;
        GetLeadDigits(iLeadDigits);
        GetFracDigits(iFracDigits);
        wsNewText = XFA_NumericLimit(wsNewText, iLeadDigits, iFracDigits);
      }
      bSyncData = TRUE;
    }
  }
  if (uiType != XFA_ELEMENT_NumericEdit || bSyncData) {
    SyncValue(wsNewText, TRUE);
  }
  return bValidate;
}
FX_BOOL CXFA_WidgetData::GetPictureContent(CFX_WideString& wsPicture,
                                           XFA_VALUEPICTURE ePicture) {
  if (ePicture == XFA_VALUEPICTURE_Raw) {
    return FALSE;
  }
  CXFA_LocaleValue widgetValue = XFA_GetLocaleValue(this);
  switch (ePicture) {
    case XFA_VALUEPICTURE_Display: {
      if (CXFA_Node* pFormat = m_pNode->GetChild(0, XFA_ELEMENT_Format)) {
        if (CXFA_Node* pPicture = pFormat->GetChild(0, XFA_ELEMENT_Picture)) {
          if (pPicture->TryContent(wsPicture)) {
            return TRUE;
          }
        }
      }
      CFX_WideString wsDataPicture, wsTimePicture;
      IFX_Locale* pLocale = GetLocal();
      if (!pLocale) {
        return FALSE;
      }
      FX_DWORD dwType = widgetValue.GetType();
      switch (dwType) {
        case XFA_VT_DATE:
          pLocale->GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY_Medium,
                                  wsPicture);
          break;
        case XFA_VT_TIME:
          pLocale->GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY_Medium,
                                  wsPicture);
          break;
        case XFA_VT_DATETIME:
          pLocale->GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY_Medium,
                                  wsDataPicture);
          pLocale->GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY_Medium,
                                  wsTimePicture);
          wsPicture = wsDataPicture + FX_WSTRC(L"T") + wsTimePicture;
          break;
        case XFA_VT_DECIMAL:
        case XFA_VT_FLOAT:
          break;
        default:
          break;
      }
    }
      return TRUE;
    case XFA_VALUEPICTURE_Edit: {
      CXFA_Node* pUI = m_pNode->GetChild(0, XFA_ELEMENT_Ui);
      if (pUI) {
        if (CXFA_Node* pPicture = pUI->GetChild(0, XFA_ELEMENT_Picture)) {
          if (pPicture->TryContent(wsPicture)) {
            return TRUE;
          }
        }
      }
      {
        CFX_WideString wsDataPicture, wsTimePicture;
        IFX_Locale* pLocale = GetLocal();
        if (!pLocale) {
          return FALSE;
        }
        FX_DWORD dwType = widgetValue.GetType();
        switch (dwType) {
          case XFA_VT_DATE:
            pLocale->GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY_Short,
                                    wsPicture);
            break;
          case XFA_VT_TIME:
            pLocale->GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY_Short,
                                    wsPicture);
            break;
          case XFA_VT_DATETIME:
            pLocale->GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY_Short,
                                    wsDataPicture);
            pLocale->GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY_Short,
                                    wsTimePicture);
            wsPicture = wsDataPicture + L"T" + wsTimePicture;
            break;
          default:
            break;
        }
      }
    }
      return TRUE;
    case XFA_VALUEPICTURE_DataBind: {
      if (CXFA_Bind bind = GetBind()) {
        bind.GetPicture(wsPicture);
        return TRUE;
      }
    } break;
    default:
      break;
  }
  return FALSE;
}
IFX_Locale* CXFA_WidgetData::GetLocal() {
  IFX_Locale* pLocale = NULL;
  if (!m_pNode) {
    return pLocale;
  }
  FX_BOOL bLocale = FALSE;
  CFX_WideString wsLocaleName;
  bLocale = m_pNode->GetLocaleName(wsLocaleName);
  if (bLocale) {
    if (wsLocaleName.Equal(FX_WSTRC(L"ambient"))) {
      pLocale = m_pNode->GetDocument()->GetLocalMgr()->GetDefLocale();
    } else {
      pLocale =
          m_pNode->GetDocument()->GetLocalMgr()->GetLocaleByName(wsLocaleName);
    }
  }
  return pLocale;
}
static FX_BOOL XFA_SplitDateTime(const CFX_WideString& wsDateTime,
                                 CFX_WideString& wsDate,
                                 CFX_WideString& wsTime) {
  wsDate = L"";
  wsTime = L"";
  if (wsDateTime.IsEmpty()) {
    return FALSE;
  }
  int nSplitIndex = -1;
  nSplitIndex = wsDateTime.Find('T');
  if (nSplitIndex < 0) {
    nSplitIndex = wsDateTime.Find(' ');
  }
  if (nSplitIndex < 0) {
    return FALSE;
  }
  wsDate = wsDateTime.Left(nSplitIndex);
  if (!wsDate.IsEmpty()) {
    int32_t iCount = wsDate.GetLength();
    int32_t i = 0;
    for (i = 0; i < iCount; i++) {
      if (wsDate[i] >= '0' && wsDate[i] <= '9') {
        break;
      }
    }
    if (i == iCount) {
      return FALSE;
    }
  }
  wsTime = wsDateTime.Right(wsDateTime.GetLength() - nSplitIndex - 1);
  if (!wsTime.IsEmpty()) {
    int32_t iCount = wsTime.GetLength();
    int32_t i = 0;
    for (i = 0; i < iCount; i++) {
      if (wsTime[i] >= '0' && wsTime[i] <= '9') {
        break;
      }
    }
    if (i == iCount) {
      return FALSE;
    }
  }
  return TRUE;
}
#ifndef XFA_PARSE_HAS_LINEIDENTIFIER
FX_BOOL CXFA_FieldNode_IsRichTextEdit(CXFA_Node* pFieldNode,
                                      IFDE_XMLNode*& pXMLNode) {
  FX_BOOL bRichTextEdit = FALSE;
  pXMLNode = NULL;
  if (pFieldNode->GetClassID() == XFA_ELEMENT_Field) {
    CXFA_Node* pValue = pFieldNode->GetChild(0, XFA_ELEMENT_Value);
    if (!pValue) {
      return bRichTextEdit;
    }
    CXFA_Node* pChildValue = pValue->GetNodeItem(XFA_NODEITEM_FirstChild);
    if (!pChildValue) {
      return bRichTextEdit;
    }
    if (pChildValue->GetClassID() == XFA_ELEMENT_ExData) {
      CFX_WideString wsContentType;
      pChildValue->GetAttribute(XFA_ATTRIBUTE_ContentType, wsContentType,
                                FALSE);
      bRichTextEdit = wsContentType.Equal(FX_WSTRC(L"text/html"));
      if (bRichTextEdit) {
        FX_BOOL bXMLInData = FALSE;
        CXFA_Node* pDataNode = pFieldNode->GetBindData();
        if (pDataNode) {
          IFDE_XMLNode* pBindXML = pDataNode->GetXMLMappingNode();
          FXSYS_assert(pBindXML);
          IFDE_XMLNode* pValueXML =
              pBindXML->GetNodeItem(IFDE_XMLNode::FirstChild);
          if (pValueXML && pValueXML->GetType() == FDE_XMLNODE_Element) {
            pXMLNode = pValueXML;
            bXMLInData = TRUE;
          }
        }
        if (!bXMLInData) {
          pXMLNode = pChildValue->GetXMLMappingNode();
        }
      }
    }
  }
  return bRichTextEdit;
}
#endif
FX_BOOL CXFA_WidgetData::GetValue(CFX_WideString& wsValue,
                                  XFA_VALUEPICTURE eValueType) {
#ifdef XFA_PARSE_HAS_LINEIDENTIFIER
  wsValue = m_pNode->GetContent();
#else
  IFDE_XMLNode* pXMLNode = NULL;
  FX_BOOL bRichTextEdit = CXFA_FieldNode_IsRichTextEdit(m_pNode, pXMLNode);
  if (bRichTextEdit) {
    XFA_GetPlainTextFromRichText(pXMLNode, wsValue);
  } else {
    wsValue = m_pNode->GetContent();
  }
#endif
  if (eValueType == XFA_VALUEPICTURE_Display) {
    GetItemLabel(wsValue, wsValue);
  }
  CFX_WideString wsPicture;
  GetPictureContent(wsPicture, eValueType);
  CXFA_Node* pNode = GetUIChild();
  if (!pNode) {
    return TRUE;
  }
  XFA_ELEMENT uiType = GetUIChild()->GetClassID();
  switch (uiType) {
    case XFA_ELEMENT_ChoiceList: {
      if (eValueType == XFA_VALUEPICTURE_Display) {
        int32_t iSelItemIndex = GetSelectedItem(0);
        if (iSelItemIndex >= 0) {
          GetChoiceListItem(wsValue, iSelItemIndex);
          wsPicture.Empty();
        }
      }
    } break;
    case XFA_ELEMENT_NumericEdit:
      if (eValueType != XFA_VALUEPICTURE_Raw && wsPicture.IsEmpty()) {
        IFX_Locale* pLocale = GetLocal();
        if (eValueType == XFA_VALUEPICTURE_Display && pLocale) {
          CFX_WideString wsOutput;
          NormalizeNumStr(wsValue, wsOutput);
          FormatNumStr(wsOutput, pLocale, wsOutput);
          wsValue = wsOutput;
        }
      }
      break;
    default:
      break;
  }
  if (wsPicture.IsEmpty()) {
    return TRUE;
  }
  if (IFX_Locale* pLocale = GetLocal()) {
    CXFA_LocaleValue widgetValue = XFA_GetLocaleValue(this);
    CXFA_LocaleMgr* pLocalMgr = m_pNode->GetDocument()->GetLocalMgr();
    switch (widgetValue.GetType()) {
      case XFA_VT_DATE: {
        CFX_WideString wsDate, wsTime;
        if (XFA_SplitDateTime(wsValue, wsDate, wsTime)) {
          CXFA_LocaleValue date(XFA_VT_DATE, wsDate, pLocalMgr);
          if (date.FormatPatterns(wsValue, wsPicture, pLocale, eValueType)) {
            return TRUE;
          }
        }
        break;
      }
      case XFA_VT_TIME: {
        CFX_WideString wsDate, wsTime;
        if (XFA_SplitDateTime(wsValue, wsDate, wsTime)) {
          CXFA_LocaleValue time(XFA_VT_TIME, wsTime, pLocalMgr);
          if (time.FormatPatterns(wsValue, wsPicture, pLocale, eValueType)) {
            return TRUE;
          }
        }
        break;
      }
      default:
        break;
    }
    widgetValue.FormatPatterns(wsValue, wsPicture, pLocale, eValueType);
  }
  return TRUE;
}
FX_BOOL CXFA_WidgetData::GetNormalizeDataValue(
    const CFX_WideStringC& wsValue,
    CFX_WideString& wsNormalizeValue) {
  wsNormalizeValue = wsValue;
  if (wsValue.IsEmpty()) {
    return TRUE;
  }
  CFX_WideString wsPicture;
  GetPictureContent(wsPicture, XFA_VALUEPICTURE_DataBind);
  if (wsPicture.IsEmpty()) {
    return TRUE;
  }
  FXSYS_assert(GetNode());
  CXFA_LocaleMgr* pLocalMgr = GetNode()->GetDocument()->GetLocalMgr();
  IFX_Locale* pLocale = GetLocal();
  CXFA_LocaleValue widgetValue = XFA_GetLocaleValue(this);
  if (widgetValue.ValidateValue(wsValue, wsPicture, pLocale, &wsPicture)) {
    widgetValue = CXFA_LocaleValue(widgetValue.GetType(), wsNormalizeValue,
                                   wsPicture, pLocale, pLocalMgr);
    wsNormalizeValue = widgetValue.GetValue();
    return TRUE;
  }
  return FALSE;
}
FX_BOOL CXFA_WidgetData::GetFormatDataValue(const CFX_WideStringC& wsValue,
                                            CFX_WideString& wsFormatedValue) {
  wsFormatedValue = wsValue;
  if (wsValue.IsEmpty()) {
    return TRUE;
  }
  CFX_WideString wsPicture;
  GetPictureContent(wsPicture, XFA_VALUEPICTURE_DataBind);
  if (wsPicture.IsEmpty()) {
    return TRUE;
  }
  if (IFX_Locale* pLocale = GetLocal()) {
    FXSYS_assert(GetNode());
    CXFA_Node* pNodeValue = GetNode()->GetChild(0, XFA_ELEMENT_Value);
    if (!pNodeValue) {
      return FALSE;
    }
    CXFA_Node* pValueChild = pNodeValue->GetNodeItem(XFA_NODEITEM_FirstChild);
    if (!pValueChild) {
      return FALSE;
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
    CXFA_LocaleMgr* pLocalMgr = GetNode()->GetDocument()->GetLocalMgr();
    CXFA_LocaleValue widgetValue(iVTType, wsValue, pLocalMgr);
    switch (widgetValue.GetType()) {
      case XFA_VT_DATE: {
        CFX_WideString wsDate, wsTime;
        if (XFA_SplitDateTime(wsValue, wsDate, wsTime)) {
          CXFA_LocaleValue date(XFA_VT_DATE, wsDate, pLocalMgr);
          if (date.FormatPatterns(wsFormatedValue, wsPicture, pLocale,
                                  XFA_VALUEPICTURE_DataBind)) {
            return TRUE;
          }
        }
        break;
      }
      case XFA_VT_TIME: {
        CFX_WideString wsDate, wsTime;
        if (XFA_SplitDateTime(wsValue, wsDate, wsTime)) {
          CXFA_LocaleValue time(XFA_VT_TIME, wsTime, pLocalMgr);
          if (time.FormatPatterns(wsFormatedValue, wsPicture, pLocale,
                                  XFA_VALUEPICTURE_DataBind)) {
            return TRUE;
          }
        }
        break;
      }
      default:
        break;
    }
    widgetValue.FormatPatterns(wsFormatedValue, wsPicture, pLocale,
                               XFA_VALUEPICTURE_DataBind);
  }
  return FALSE;
}
void CXFA_WidgetData::NormalizeNumStr(const CFX_WideString& wsValue,
                                      CFX_WideString& wsOutput) {
  if (wsValue.IsEmpty()) {
    return;
  }
  wsOutput = wsValue;
  wsOutput.TrimLeft('0');
  int32_t dot_index = wsOutput.Find('.');
  int32_t iFracDigits = 0;
  if (!wsOutput.IsEmpty() && dot_index >= 0 &&
      (!GetFracDigits(iFracDigits) || iFracDigits != -1)) {
    wsOutput.TrimRight(L"0");
    wsOutput.TrimRight(L".");
  }
  if (wsOutput.IsEmpty() || wsOutput[0] == '.') {
    wsOutput.Insert(0, '0');
  }
}
void CXFA_WidgetData::FormatNumStr(const CFX_WideString& wsValue,
                                   IFX_Locale* pLocale,
                                   CFX_WideString& wsOutput) {
  if (wsValue.IsEmpty()) {
    return;
  }
  CFX_WideString wsSrcNum = wsValue;
  CFX_WideString wsGroupSymbol;
  pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Grouping, wsGroupSymbol);
  FX_BOOL bNeg = FALSE;
  if (wsSrcNum[0] == '-') {
    bNeg = TRUE;
    wsSrcNum.Delete(0, 1);
  }
  int32_t len = wsSrcNum.GetLength();
  int32_t dot_index = wsSrcNum.Find('.');
  if (dot_index == -1) {
    dot_index = len;
  }
  int32_t cc = dot_index - 1;
  if (cc >= 0) {
    int nPos = dot_index % 3;
    wsOutput.Empty();
    for (int32_t i = 0; i < dot_index; i++) {
      if (i % 3 == nPos && i != 0) {
        wsOutput += wsGroupSymbol;
      }
      wsOutput += wsSrcNum[i];
    }
    if (dot_index < len) {
      CFX_WideString wsSymbol;
      pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Decimal, wsSymbol);
      wsOutput += wsSymbol;
      wsOutput += wsSrcNum.Right(len - dot_index - 1);
    }
    if (bNeg) {
      CFX_WideString wsMinusymbol;
      pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Minus, wsMinusymbol);
      wsOutput = wsMinusymbol + wsOutput;
    }
  }
}
void CXFA_WidgetData::SyncValue(const CFX_WideString& wsValue,
                                FX_BOOL bNotify) {
  if (!m_pNode) {
    return;
  }
  CFX_WideString wsFormatValue(wsValue);
  CXFA_WidgetData* pContainerWidgetData = m_pNode->GetContainerWidgetData();
  if (pContainerWidgetData) {
    pContainerWidgetData->GetFormatDataValue(wsValue, wsFormatValue);
  }
  m_pNode->SetContent(wsValue, wsFormatValue, bNotify);
}
void CXFA_WidgetData::InsertListTextItem(CXFA_Node* pItems,
                                         const CFX_WideStringC& wsText,
                                         int32_t nIndex) {
  CXFA_Node* pText = pItems->CreateSamePacketNode(XFA_ELEMENT_Text);
  pItems->InsertChild(nIndex, pText);
  pText->SetContent(wsText, wsText, FALSE, FALSE, FALSE);
}
CXFA_Filter CXFA_WidgetData::GetFilter(FX_BOOL bModified) {
  if (!m_pUiChildNode) {
    return CXFA_Filter(NULL);
  }
  return m_pUiChildNode->GetProperty(0, XFA_ELEMENT_Filter, bModified);
}
CXFA_Manifest CXFA_WidgetData::GetManifest(FX_BOOL bModified) {
  if (!m_pUiChildNode) {
    return CXFA_Manifest(NULL);
  }
  return m_pUiChildNode->GetProperty(0, XFA_ELEMENT_Manifest, bModified);
}
CXFA_Occur::CXFA_Occur(CXFA_Node* pNode) : CXFA_Data(pNode) {}
int32_t CXFA_Occur::GetMax() {
  int32_t iMax = 1;
  if (m_pNode) {
    if (!m_pNode->TryInteger(XFA_ATTRIBUTE_Max, iMax, TRUE)) {
      iMax = GetMin();
    }
  }
  return iMax;
}
int32_t CXFA_Occur::GetMin() {
  int32_t iMin = 1;
  if (m_pNode) {
    if (!m_pNode->TryInteger(XFA_ATTRIBUTE_Min, iMin, TRUE) || iMin < 0) {
      iMin = 1;
    }
  }
  return iMin;
}
int32_t CXFA_Occur::GetInitial() {
  int32_t iInit = 1;
  if (m_pNode) {
    int32_t iMin = GetMin();
    if (!m_pNode->TryInteger(XFA_ATTRIBUTE_Initial, iInit, TRUE) ||
        iInit < iMin) {
      iInit = iMin;
    }
  }
  return iInit;
}
FX_BOOL CXFA_Occur::GetOccurInfo(int32_t& iMin, int32_t& iMax, int32_t& iInit) {
  if (!m_pNode) {
    return FALSE;
  }
  if (!m_pNode->TryInteger(XFA_ATTRIBUTE_Min, iMin, FALSE) || iMin < 0) {
    iMin = 1;
  }
  if (!m_pNode->TryInteger(XFA_ATTRIBUTE_Max, iMax, FALSE)) {
    if (iMin == 0) {
      iMax = 1;
    } else {
      iMax = iMin;
    }
  }
  if (!m_pNode->TryInteger(XFA_ATTRIBUTE_Initial, iInit, FALSE) ||
      iInit < iMin) {
    iInit = iMin;
  }
  return TRUE;
}
void CXFA_Occur::SetMax(int32_t iMax) {
  iMax = (iMax != -1 && iMax < 1) ? 1 : iMax;
  m_pNode->SetInteger(XFA_ATTRIBUTE_Max, iMax, FALSE);
  int32_t iMin = GetMin();
  if (iMax != -1 && iMax < iMin) {
    iMin = iMax;
    m_pNode->SetInteger(XFA_ATTRIBUTE_Min, iMin, FALSE);
  }
}
void CXFA_Occur::SetMin(int32_t iMin) {
  iMin = (iMin < 0) ? 1 : iMin;
  m_pNode->SetInteger(XFA_ATTRIBUTE_Min, iMin, FALSE);
  int32_t iMax = GetMax();
  if (iMax > 0 && iMax < iMin) {
    iMax = iMin;
    m_pNode->SetInteger(XFA_ATTRIBUTE_Max, iMax, FALSE);
  }
}
XFA_ATTRIBUTEENUM XFA_GetEnumTypeAttribute(
    CXFA_Node* pNode,
    XFA_ATTRIBUTE attributeValue = XFA_ATTRIBUTE_Type,
    XFA_ATTRIBUTEENUM eDefaultValue = XFA_ATTRIBUTEENUM_Optional) {
  XFA_ATTRIBUTEENUM eType = eDefaultValue;
  if (pNode) {
    if (!pNode->TryEnum(attributeValue, eType, TRUE)) {
      eType = eDefaultValue;
    }
  }
  return eType;
}
CFX_WideString CXFA_Filter::GetFilterString(XFA_ATTRIBUTE eAttribute) {
  CFX_WideString wsStringValue;
  if (m_pNode) {
    m_pNode->GetAttribute(eAttribute, wsStringValue, FALSE);
  }
  return wsStringValue;
}
XFA_ATTRIBUTEENUM CXFA_Filter::GetAppearanceFilterType() {
  if (!m_pNode) {
    return XFA_ATTRIBUTEENUM_Optional;
  }
  CXFA_Node* pAppearanceFilterNode =
      m_pNode->GetProperty(0, XFA_ELEMENT_AppearanceFilter);
  return XFA_GetEnumTypeAttribute(pAppearanceFilterNode);
}
CFX_WideString CXFA_Filter::GetAppearanceFilterContent() {
  CFX_WideString wsContent;
  if (m_pNode) {
    CXFA_Node* pAppearanceFilterNode =
        m_pNode->GetProperty(0, XFA_ELEMENT_AppearanceFilter);
    pAppearanceFilterNode->TryContent(wsContent);
  }
  return wsContent;
}
XFA_ATTRIBUTEENUM CXFA_Filter::GetCertificatesCredentialServerPolicy() {
  if (!m_pNode) {
    return XFA_ATTRIBUTEENUM_Optional;
  }
  CXFA_Node* pCertsNode = m_pNode->GetProperty(0, XFA_ELEMENT_Certificates);
  return XFA_GetEnumTypeAttribute(pCertsNode,
                                  XFA_ATTRIBUTE_CredentialServerPolicy);
}
CFX_WideString CXFA_Filter::GetCertificatesURL() {
  CFX_WideString wsURL;
  if (m_pNode) {
    CXFA_Node* pCertsNode = m_pNode->GetProperty(0, XFA_ELEMENT_Certificates);
    pCertsNode->GetAttribute(XFA_ATTRIBUTE_Url, wsURL, FALSE);
  }
  return wsURL;
}
CFX_WideString CXFA_Filter::GetCertificatesURLPolicy() {
  CFX_WideString wsURLPolicy;
  if (m_pNode) {
    CXFA_Node* pCertsNode = m_pNode->GetProperty(0, XFA_ELEMENT_Certificates);
    pCertsNode->GetAttribute(XFA_ATTRIBUTE_UrlPolicy, wsURLPolicy, FALSE);
  }
  return wsURLPolicy;
}
CXFA_WrapCertificate CXFA_Filter::GetCertificatesEncryption(FX_BOOL bModified) {
  if (!m_pNode) {
    return CXFA_WrapCertificate(NULL);
  }
  CXFA_Node* pCertsNode =
      m_pNode->GetProperty(0, XFA_ELEMENT_Certificates, bModified);
  return CXFA_WrapCertificate(
      pCertsNode ? pCertsNode->GetProperty(0, XFA_ELEMENT_Encryption, bModified)
                 : NULL);
}
CXFA_WrapCertificate CXFA_Filter::GetCertificatesIssuers(FX_BOOL bModified) {
  if (!m_pNode) {
    return CXFA_WrapCertificate(NULL);
  }
  CXFA_Node* pCertsNode =
      m_pNode->GetProperty(0, XFA_ELEMENT_Certificates, bModified);
  return CXFA_WrapCertificate(
      pCertsNode ? pCertsNode->GetProperty(0, XFA_ELEMENT_Issuers, bModified)
                 : NULL);
}
CFX_WideString CXFA_Filter::GetCertificatesKeyUsageString(
    XFA_ATTRIBUTE eAttribute) {
  if (!m_pNode) {
    return FX_WSTRC(L"");
  }
  CXFA_Node* pCertsNode = m_pNode->GetProperty(0, XFA_ELEMENT_Certificates);
  CXFA_Node* pKeyUsageNode = pCertsNode->GetProperty(0, XFA_ELEMENT_KeyUsage);
  CFX_WideString wsAttributeValue;
  pKeyUsageNode->GetAttribute(eAttribute, wsAttributeValue, FALSE);
  return wsAttributeValue;
}
CXFA_Oids CXFA_Filter::GetCertificatesOids() {
  if (!m_pNode) {
    return CXFA_Oids(NULL);
  }
  CXFA_Node* pCertsNode = m_pNode->GetProperty(0, XFA_ELEMENT_Certificates);
  return CXFA_Oids(pCertsNode ? pCertsNode->GetProperty(0, XFA_ELEMENT_Oids)
                              : NULL);
}
CXFA_WrapCertificate CXFA_Filter::GetCertificatesSigning(FX_BOOL bModified) {
  if (!m_pNode) {
    return CXFA_WrapCertificate(NULL);
  }
  CXFA_Node* pCertsNode =
      m_pNode->GetProperty(0, XFA_ELEMENT_Certificates, bModified);
  return CXFA_WrapCertificate(
      pCertsNode ? pCertsNode->GetProperty(0, XFA_ELEMENT_Signing, bModified)
                 : NULL);
}
CXFA_DigestMethods CXFA_Filter::GetDigestMethods(FX_BOOL bModified) {
  return CXFA_DigestMethods(
      m_pNode ? m_pNode->GetProperty(0, XFA_ELEMENT_DigestMethods, bModified)
              : NULL);
}
CXFA_Encodings CXFA_Filter::GetEncodings(FX_BOOL bModified) {
  return CXFA_Encodings(
      m_pNode ? m_pNode->GetProperty(0, XFA_ELEMENT_Encodings, bModified)
              : NULL);
}
CXFA_EncryptionMethods CXFA_Filter::GetEncryptionMethods(FX_BOOL bModified) {
  return CXFA_EncryptionMethods(
      m_pNode
          ? m_pNode->GetProperty(0, XFA_ELEMENT_EncryptionMethods, bModified)
          : NULL);
}
XFA_ATTRIBUTEENUM CXFA_Filter::GetHandlerType() {
  if (!m_pNode) {
    return XFA_ATTRIBUTEENUM_Optional;
  }
  CXFA_Node* pHandlerNode = m_pNode->GetProperty(0, XFA_ELEMENT_Handler);
  return XFA_GetEnumTypeAttribute(pHandlerNode);
}
CFX_WideString CXFA_Filter::GetHandlerContent() {
  CFX_WideString wsContent;
  if (m_pNode) {
    CXFA_Node* pHandlerNode = m_pNode->GetProperty(0, XFA_ELEMENT_Handler);
    pHandlerNode->TryContent(wsContent);
  }
  return wsContent;
}
XFA_ATTRIBUTEENUM CXFA_Filter::GetlockDocumentType() {
  if (!m_pNode) {
    return XFA_ATTRIBUTEENUM_Optional;
  }
  CXFA_Node* pLockDocNode = m_pNode->GetProperty(0, XFA_ELEMENT_LockDocument);
  return XFA_GetEnumTypeAttribute(pLockDocNode);
}
CFX_WideString CXFA_Filter::GetlockDocumentContent() {
  CFX_WideString wsContent = FX_WSTRC(L"auto");
  if (m_pNode) {
    CXFA_Node* pLockDocNode = m_pNode->GetProperty(0, XFA_ELEMENT_LockDocument);
    pLockDocNode->TryContent(wsContent);
  }
  return wsContent;
}
int32_t CXFA_Filter::GetMDPPermissions() {
  int32_t iPermissions = 2;
  if (m_pNode) {
    CXFA_Node* pMDPNode = m_pNode->GetProperty(0, XFA_ELEMENT_Mdp);
    if (!pMDPNode->TryInteger(XFA_ATTRIBUTE_Permissions, iPermissions, TRUE)) {
      iPermissions = 2;
    }
  }
  return iPermissions;
}
XFA_ATTRIBUTEENUM CXFA_Filter::GetMDPSignatureType() {
  if (!m_pNode) {
    return XFA_ATTRIBUTEENUM_Filter;
  }
  CXFA_Node* pMDPNode = m_pNode->GetProperty(0, XFA_ELEMENT_Mdp);
  return XFA_GetEnumTypeAttribute(pMDPNode, XFA_ATTRIBUTE_SignatureType,
                                  XFA_ATTRIBUTEENUM_Filter);
}
CXFA_Reasons CXFA_Filter::GetReasons(FX_BOOL bModified) {
  return CXFA_Reasons(m_pNode ? m_pNode->GetProperty(0, XFA_ELEMENT_Reasons)
                              : NULL);
}
CFX_WideString CXFA_Filter::GetTimeStampServer() {
  CFX_WideString wsServerURI;
  if (m_pNode) {
    CXFA_Node* pTimeStampNode = m_pNode->GetProperty(0, XFA_ELEMENT_TimeStamp);
    pTimeStampNode->GetAttribute(XFA_ATTRIBUTE_Server, wsServerURI, FALSE);
  }
  return wsServerURI;
}
XFA_ATTRIBUTEENUM CXFA_Filter::GetTimeStampType() {
  if (!m_pNode) {
    return XFA_ATTRIBUTEENUM_Optional;
  }
  CXFA_Node* pTimeStampNode = m_pNode->GetProperty(0, XFA_ELEMENT_TimeStamp);
  return XFA_GetEnumTypeAttribute(pTimeStampNode);
}
CFX_WideString CXFA_Certificate::GetCertificateName() {
  CFX_WideString wsName;
  if (m_pNode) {
    m_pNode->GetAttribute(XFA_ATTRIBUTE_Name, wsName, FALSE);
  }
  return wsName;
}
CFX_WideString CXFA_Certificate::GetCertificateContent() {
  CFX_WideString wsContent;
  if (m_pNode) {
    m_pNode->TryContent(wsContent);
  }
  return wsContent;
}
XFA_ATTRIBUTEENUM CXFA_WrapCertificate::GetType() {
  return XFA_GetEnumTypeAttribute(m_pNode);
}
int32_t CXFA_WrapCertificate::CountCertificates() {
  return m_pNode ? m_pNode->CountChildren(XFA_ELEMENT_Certificate) : 0;
}
CXFA_Certificate CXFA_WrapCertificate::GetCertificate(int32_t nIndex) {
  return CXFA_Certificate(
      (nIndex > -1 && m_pNode)
          ? m_pNode->GetChild(nIndex, XFA_ELEMENT_Certificate)
          : NULL);
}
XFA_ATTRIBUTEENUM CXFA_Oids::GetOidsType() {
  return XFA_GetEnumTypeAttribute(m_pNode);
}
int32_t CXFA_Oids::CountOids() {
  return m_pNode ? m_pNode->CountChildren(XFA_ELEMENT_Oid) : 0;
}
CFX_WideString CXFA_Oids::GetOidContent(int32_t nIndex) {
  if (nIndex <= -1 || !m_pNode) {
    return FX_WSTRC(L"");
  }
  CXFA_Node* pOidNode = m_pNode->GetChild(nIndex, XFA_ELEMENT_Oid);
  if (!pOidNode) {
    return FX_WSTRC(L"");
  }
  CFX_WideString wsContent;
  pOidNode->TryContent(wsContent);
  return wsContent;
}
XFA_ATTRIBUTEENUM CXFA_SubjectDNs::GetSubjectDNsType() {
  return XFA_GetEnumTypeAttribute(m_pNode);
}
int32_t CXFA_SubjectDNs::CountSubjectDNs() {
  return m_pNode ? m_pNode->CountChildren(XFA_ELEMENT_SubjectDN) : 0;
}
CFX_WideString CXFA_SubjectDNs::GetSubjectDNString(int32_t nIndex,
                                                   XFA_ATTRIBUTE eAttribute) {
  if (nIndex <= -1 || !m_pNode) {
    return FX_WSTRC(L"");
  }
  CXFA_Node* pSubjectDNNode = m_pNode->GetChild(nIndex, XFA_ELEMENT_SubjectDN);
  if (!pSubjectDNNode) {
    return FX_WSTRC(L"");
  }
  CFX_WideString wsAttributeValue;
  pSubjectDNNode->GetAttribute(eAttribute, wsAttributeValue, FALSE);
  return wsAttributeValue;
}
CFX_WideString CXFA_SubjectDNs::GetSubjectDNContent(int32_t nIndex) {
  if (nIndex <= -1 || !m_pNode) {
    return FX_WSTRC(L"");
  }
  CXFA_Node* pSubjectDNNode = m_pNode->GetChild(nIndex, XFA_ELEMENT_SubjectDN);
  if (!pSubjectDNNode) {
    return FX_WSTRC(L"");
  }
  CFX_WideString wsContent;
  pSubjectDNNode->TryContent(wsContent);
  return wsContent;
}
XFA_ATTRIBUTEENUM CXFA_DigestMethods::GetDigestMethodsType() {
  return XFA_GetEnumTypeAttribute(m_pNode);
}
int32_t CXFA_DigestMethods::CountDigestMethods() {
  return m_pNode ? m_pNode->CountChildren(XFA_ELEMENT_DigestMethod) : 0;
}
CFX_WideString CXFA_DigestMethods::GetDigestMethodContent(int32_t nIndex) {
  if (nIndex <= -1 || !m_pNode) {
    return FX_WSTRC(L"");
  }
  CXFA_Node* pDigestMethodNode =
      m_pNode->GetChild(nIndex, XFA_ELEMENT_DigestMethod);
  if (!pDigestMethodNode) {
    return FX_WSTRC(L"");
  }
  CFX_WideString wsContent;
  pDigestMethodNode->TryContent(wsContent);
  return wsContent;
}
XFA_ATTRIBUTEENUM CXFA_Encodings::GetEncodingsType() {
  return XFA_GetEnumTypeAttribute(m_pNode);
}
int32_t CXFA_Encodings::CountEncodings() {
  return m_pNode ? m_pNode->CountChildren(XFA_ELEMENT_Encoding) : 0;
}
CFX_WideString CXFA_Encodings::GetEncodingContent(int32_t nIndex) {
  if (nIndex <= -1 || !m_pNode) {
    return FX_WSTRC(L"");
  }
  CXFA_Node* pEncodingNode = m_pNode->GetChild(nIndex, XFA_ELEMENT_Encoding);
  if (!pEncodingNode) {
    return FX_WSTRC(L"");
  }
  CFX_WideString wsContent;
  pEncodingNode->TryContent(wsContent);
  return wsContent;
}
XFA_ATTRIBUTEENUM CXFA_EncryptionMethods::GetEncryptionMethodsType() {
  return XFA_GetEnumTypeAttribute(m_pNode);
}
int32_t CXFA_EncryptionMethods::CountEncryptionMethods() {
  return m_pNode ? m_pNode->CountChildren(XFA_ELEMENT_EncryptionMethod) : 0;
}
CFX_WideString CXFA_EncryptionMethods::GetEncryptionMethodContent(
    int32_t nIndex) {
  if (nIndex <= -1 || !m_pNode) {
    return FX_WSTRC(L"");
  }
  CXFA_Node* pEncryMethodNode =
      m_pNode->GetChild(nIndex, XFA_ELEMENT_EncryptionMethod);
  if (!pEncryMethodNode) {
    return FX_WSTRC(L"");
  }
  CFX_WideString wsContent;
  pEncryMethodNode->TryContent(wsContent);
  return wsContent;
}
XFA_ATTRIBUTEENUM CXFA_Reasons::GetReasonsType() {
  return XFA_GetEnumTypeAttribute(m_pNode);
}
int32_t CXFA_Reasons::CountReasons() {
  return m_pNode ? m_pNode->CountChildren(XFA_ELEMENT_Reason) : 0;
}
CFX_WideString CXFA_Reasons::GetReasonContent(int32_t nIndex) {
  if (nIndex <= -1 || !m_pNode) {
    return FX_WSTRC(L"");
  }
  CXFA_Node* pReasonNode = m_pNode->GetChild(nIndex, XFA_ELEMENT_Reason);
  if (!pReasonNode) {
    return FX_WSTRC(L"");
  }
  CFX_WideString wsContent;
  pReasonNode->TryContent(wsContent);
  return wsContent;
}
XFA_ATTRIBUTEENUM CXFA_Manifest::GetAction() {
  return XFA_GetEnumTypeAttribute(m_pNode, XFA_ATTRIBUTE_Action,
                                  XFA_ATTRIBUTEENUM_Include);
}
int32_t CXFA_Manifest::CountReives() {
  return m_pNode ? m_pNode->CountChildren(XFA_ELEMENT_Ref) : 0;
}
CFX_WideString CXFA_Manifest::GetRefContent(int32_t nIndex) {
  if (nIndex <= -1 || !m_pNode) {
    return FX_WSTRC(L"");
  }
  CXFA_Node* pRefNode = m_pNode->GetChild(nIndex, XFA_ELEMENT_Ref);
  if (!pRefNode) {
    return FX_WSTRC(L"");
  }
  CFX_WideString wsContent;
  pRefNode->TryContent(wsContent);
  return wsContent;
}
