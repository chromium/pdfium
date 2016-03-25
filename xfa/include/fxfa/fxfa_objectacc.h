// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_INCLUDE_FXFA_FXFA_OBJECTACC_H_
#define XFA_INCLUDE_FXFA_FXFA_OBJECTACC_H_

#include "core/include/fxge/fx_dib.h"
#include "xfa/fxfa/parser/xfa_object.h"

class CXFA_Node;
class IFX_Locale;
class CXFA_Margin;
class CXFA_Caption;
class CXFA_Para;
class CXFA_Event;
class CXFA_Script;
class CXFA_Value;
class CXFA_Calculate;
class CXFA_Line;
class CXFA_Text;
class CXFA_ExData;
class CXFA_Image;
class CXFA_Validate;
class CXFA_Bind;
class CXFA_Assist;
class CXFA_ToolTip;
class CXFA_Submit;
class CXFA_BindItems;
class CXFA_Stroke;
class CXFA_Corner;
class CXFA_Edge;
class CXFA_Box;
class CXFA_Arc;
class CXFA_Border;
class CXFA_Rectangle;
class CXFA_WidgetData;
class CXFA_Occur;

inline FX_BOOL XFA_IsSpace(FX_WCHAR c) {
  return (c == 0x20) || (c == 0x0d) || (c == 0x0a) || (c == 0x09);
}
inline FX_BOOL XFA_IsDigit(FX_WCHAR c) {
  return c >= '0' && c <= '9';
}
typedef CFX_ArrayTemplate<CXFA_Object*> CXFA_ObjArray;

class CXFA_Data {
 public:
  explicit CXFA_Data(CXFA_Node* pNode) : m_pNode(pNode) {}

  operator bool() const { return !!m_pNode; }
  CXFA_Node* GetNode() const { return m_pNode; }
  XFA_ELEMENT GetClassID() const;

 protected:
  FX_BOOL TryMeasure(XFA_ATTRIBUTE eAttr,
                     FX_FLOAT& fValue,
                     FX_BOOL bUseDefault = FALSE) const;
  FX_BOOL SetMeasure(XFA_ATTRIBUTE eAttr, FX_FLOAT fValue);

  CXFA_Node* m_pNode;
};

class CXFA_Fill : public CXFA_Data {
 public:
  explicit CXFA_Fill(CXFA_Node* pNode);
  ~CXFA_Fill();

  int32_t GetPresence();
  FX_ARGB GetColor(FX_BOOL bText = FALSE);
  int32_t GetFillType();
  int32_t GetPattern(FX_ARGB& foreColor);
  int32_t GetStipple(FX_ARGB& stippleColor);
  int32_t GetLinear(FX_ARGB& endColor);
  int32_t GetRadial(FX_ARGB& endColor);
  void SetColor(FX_ARGB color);
};

class CXFA_Margin : public CXFA_Data {
 public:
  explicit CXFA_Margin(CXFA_Node* pNode);
  FX_BOOL GetLeftInset(FX_FLOAT& fInset, FX_FLOAT fDefInset = 0) const;
  FX_BOOL GetTopInset(FX_FLOAT& fInset, FX_FLOAT fDefInset = 0) const;
  FX_BOOL GetRightInset(FX_FLOAT& fInset, FX_FLOAT fDefInset = 0) const;
  FX_BOOL GetBottomInset(FX_FLOAT& fInset, FX_FLOAT fDefInset = 0) const;
};

class CXFA_Font : public CXFA_Data {
 public:
  explicit CXFA_Font(CXFA_Node* pNode);

  FX_FLOAT GetBaselineShift();
  FX_FLOAT GetHorizontalScale();
  FX_FLOAT GetVerticalScale();
  FX_FLOAT GetLetterSpacing();
  int32_t GetLineThrough();
  int32_t GetUnderline();
  int32_t GetUnderlinePeriod();
  FX_FLOAT GetFontSize();
  void GetTypeface(CFX_WideStringC& wsTypeFace);

  FX_BOOL IsBold();
  FX_BOOL IsItalic();

  FX_ARGB GetColor();
  void SetColor(FX_ARGB color);

};

class CXFA_Caption : public CXFA_Data {
 public:
  explicit CXFA_Caption(CXFA_Node* pNode);

  int32_t GetPresence();
  int32_t GetPlacementType();
  FX_FLOAT GetReserve();
  CXFA_Margin GetMargin();
  CXFA_Font GetFont();
  CXFA_Value GetValue();
};

class CXFA_Para : public CXFA_Data {
 public:
  explicit CXFA_Para(CXFA_Node* pNode);

  int32_t GetHorizontalAlign();
  int32_t GetVerticalAlign();
  FX_FLOAT GetLineHeight();
  FX_FLOAT GetMarginLeft();
  FX_FLOAT GetMarginRight();
  FX_FLOAT GetSpaceAbove();
  FX_FLOAT GetSpaceBelow();
  FX_FLOAT GetTextIndent();
};

enum XFA_TEXTENCODING {
  XFA_TEXTENCODING_None,
  XFA_TEXTENCODING_Big5,
  XFA_TEXTENCODING_FontSpecific,
  XFA_TEXTENCODING_GBK,
  XFA_TEXTENCODING_GB18030,
  XFA_TEXTENCODING_GB2312,
  XFA_TEXTENCODING_ISO8859NN,
  XFA_TEXTENCODING_KSC5601,
  XFA_TEXTENCODING_ShiftJIS,
  XFA_TEXTENCODING_UCS2,
  XFA_TEXTENCODING_UTF16,
  XFA_TEXTENCODING_UTF8
};

class CXFA_Event : public CXFA_Data {
 public:
  explicit CXFA_Event(CXFA_Node* pNode);

  int32_t GetActivity();
  int32_t GetEventType();
  void GetRef(CFX_WideStringC& wsRef);


  CXFA_Script GetScript();
  CXFA_Submit GetSubmit();

  void GetSignDataTarget(CFX_WideString& wsTarget);
};

enum XFA_SCRIPTTYPE {
  XFA_SCRIPTTYPE_Formcalc = 0,
  XFA_SCRIPTTYPE_Javascript,
  XFA_SCRIPTTYPE_Unkown,
};

class CXFA_Script : public CXFA_Data {
 public:
  explicit CXFA_Script(CXFA_Node* pNode);

  XFA_SCRIPTTYPE GetContentType();
  int32_t GetRunAt();
  void GetExpression(CFX_WideString& wsExpression);
};

class CXFA_Submit : public CXFA_Data {
 public:
  explicit CXFA_Submit(CXFA_Node* pNode);

  FX_BOOL IsSubmitEmbedPDF();
  int32_t GetSubmitFormat();
  void GetSubmitTarget(CFX_WideStringC& wsTarget);
  void GetSubmitXDPContent(CFX_WideStringC& wsContent);
};

class CXFA_Value : public CXFA_Data {
 public:
  explicit CXFA_Value(CXFA_Node* pNode) : CXFA_Data(pNode) {}

  XFA_ELEMENT GetChildValueClassID();
  FX_BOOL GetChildValueContent(CFX_WideString& wsContent);
  CXFA_Arc GetArc();
  CXFA_Line GetLine();
  CXFA_Rectangle GetRectangle();
  CXFA_Text GetText();
  CXFA_ExData GetExData();
  CXFA_Image GetImage();
};

class CXFA_Line : public CXFA_Data {
 public:
  explicit CXFA_Line(CXFA_Node* pNode) : CXFA_Data(pNode) {}

  int32_t GetHand();
  FX_BOOL GetSlop();
  CXFA_Edge GetEdge();
};

class CXFA_Text : public CXFA_Data {
 public:
  explicit CXFA_Text(CXFA_Node* pNode);

  void GetContent(CFX_WideString& wsText);
};

class CXFA_ExData : public CXFA_Data {
 public:
  explicit CXFA_ExData(CXFA_Node* pNode);

  FX_BOOL SetContentType(const CFX_WideString& wsContentType);
};

class CXFA_Image : public CXFA_Data {
 public:
  CXFA_Image(CXFA_Node* pNode, FX_BOOL bDefValue);

  int32_t GetAspect();
  FX_BOOL GetContentType(CFX_WideString& wsContentType);
  FX_BOOL GetHref(CFX_WideString& wsHref);
  int32_t GetTransferEncoding();
  FX_BOOL GetContent(CFX_WideString& wsText);
  FX_BOOL SetContentType(const CFX_WideString& wsContentType);
  FX_BOOL SetHref(const CFX_WideString& wsHref);
  FX_BOOL SetTransferEncoding(int32_t iTransferEncoding);

 protected:
  FX_BOOL m_bDefValue;
};

class CXFA_Calculate : public CXFA_Data {
 public:
  explicit CXFA_Calculate(CXFA_Node* pNode);

  int32_t GetOverride();
  CXFA_Script GetScript();
  void GetMessageText(CFX_WideString& wsMessage);
};

class CXFA_Validate : public CXFA_Data {
 public:
  explicit CXFA_Validate(CXFA_Node* pNode);

  int32_t GetFormatTest();
  int32_t GetNullTest();
  FX_BOOL SetNullTest(CFX_WideString wsValue);
  int32_t GetScriptTest();
  void GetFormatMessageText(CFX_WideString& wsMessage);
  void SetFormatMessageText(CFX_WideString wsMessage);
  void GetNullMessageText(CFX_WideString& wsMessage);
  void SetNullMessageText(CFX_WideString wsMessage);
  void GetScriptMessageText(CFX_WideString& wsMessage);
  void SetScriptMessageText(CFX_WideString wsMessage);
  void GetPicture(CFX_WideString& wsPicture);
  CXFA_Script GetScript();

 protected:
  void GetMessageText(CFX_WideString& wsMessage,
                      const CFX_WideStringC& wsMessageType);
  void SetMessageText(CFX_WideString& wsMessage,
                      const CFX_WideStringC& wsMessageType);
  FX_BOOL SetTestValue(int32_t iType,
                       CFX_WideString& wsValue,
                       XFA_ATTRIBUTEENUM eName);
};

class CXFA_Bind : public CXFA_Data {
 public:
  explicit CXFA_Bind(CXFA_Node* pNode);

  void GetPicture(CFX_WideString& wsPicture);
};

class CXFA_Assist : public CXFA_Data {
 public:
  explicit CXFA_Assist(CXFA_Node* pNode);

  CXFA_ToolTip GetToolTip();
};

class CXFA_ToolTip : public CXFA_Data {
 public:
  explicit CXFA_ToolTip(CXFA_Node* pNode);

  FX_BOOL GetTip(CFX_WideString& wsTip);
};

class CXFA_BindItems : public CXFA_Data {
 public:
  explicit CXFA_BindItems(CXFA_Node* pNode);

  void GetLabelRef(CFX_WideStringC& wsLabelRef);
  void GetValueRef(CFX_WideStringC& wsValueRef);
  void GetRef(CFX_WideStringC& wsRef);
  FX_BOOL SetConnection(const CFX_WideString& wsConnection);
};

#define XFA_STROKE_SAMESTYLE_NoPresence 1
#define XFA_STROKE_SAMESTYLE_Corner 2

class CXFA_Stroke : public CXFA_Data {
 public:
  explicit CXFA_Stroke(CXFA_Node* pNode) : CXFA_Data(pNode) {}

  bool IsCorner() const { return GetClassID() == XFA_ELEMENT_Corner; }
  bool IsEdge() const { return GetClassID() == XFA_ELEMENT_Edge; }
  bool IsVisible() const { return GetPresence() == XFA_ATTRIBUTEENUM_Visible; }
  int32_t GetPresence() const;
  int32_t GetCapType() const;
  int32_t GetStrokeType() const;
  FX_FLOAT GetThickness() const;
  CXFA_Measurement GetMSThickness() const;
  void SetMSThickness(CXFA_Measurement msThinkness);
  FX_ARGB GetColor() const;
  void SetColor(FX_ARGB argb);
  int32_t GetJoinType() const;
  FX_BOOL IsInverted() const;
  FX_FLOAT GetRadius() const;
  FX_BOOL SameStyles(CXFA_Stroke stroke, uint32_t dwFlags = 0) const;
};

class CXFA_Corner : public CXFA_Stroke {
 public:
  explicit CXFA_Corner(CXFA_Node* pNode) : CXFA_Stroke(pNode) {}
};

class CXFA_Edge : public CXFA_Stroke {
 public:
  explicit CXFA_Edge(CXFA_Node* pNode) : CXFA_Stroke(pNode) {}
};

typedef CFX_ArrayTemplate<CXFA_Stroke> CXFA_StrokeArray;
typedef CFX_ArrayTemplate<CXFA_Edge> CXFA_EdgeArray;
typedef CFX_ArrayTemplate<CXFA_Corner> CXFA_CornerArray;

class CXFA_Box : public CXFA_Data {
 public:
  explicit CXFA_Box(CXFA_Node* pNode) : CXFA_Data(pNode) {}

  bool IsArc() const { return GetClassID() == XFA_ELEMENT_Arc; }
  bool IsBorder() const { return GetClassID() == XFA_ELEMENT_Border; }
  bool IsRectangle() const { return GetClassID() == XFA_ELEMENT_Rectangle; }
  int32_t GetHand() const;
  int32_t GetPresence() const;
  int32_t CountEdges() const;
  CXFA_Edge GetEdge(int32_t nIndex = 0) const;
  void GetStrokes(CXFA_StrokeArray& strokes) const;
  FX_BOOL IsCircular() const;
  FX_BOOL GetStartAngle(FX_FLOAT& fStartAngle) const;
  FX_FLOAT GetStartAngle() const {
    FX_FLOAT fStartAngle;
    GetStartAngle(fStartAngle);
    return fStartAngle;
  }

  FX_BOOL GetSweepAngle(FX_FLOAT& fSweepAngle) const;
  FX_FLOAT GetSweepAngle() const {
    FX_FLOAT fSweepAngle;
    GetSweepAngle(fSweepAngle);
    return fSweepAngle;
  }

  CXFA_Fill GetFill(FX_BOOL bModified = FALSE) const;
  CXFA_Margin GetMargin() const;
  int32_t Get3DStyle(FX_BOOL& bVisible, FX_FLOAT& fThickness) const;
};

class CXFA_Arc : public CXFA_Box {
 public:
  explicit CXFA_Arc(CXFA_Node* pNode) : CXFA_Box(pNode) {}
};

class CXFA_Border : public CXFA_Box {
 public:
  explicit CXFA_Border(CXFA_Node* pNode) : CXFA_Box(pNode) {}
};

class CXFA_Rectangle : public CXFA_Box {
 public:
  explicit CXFA_Rectangle(CXFA_Node* pNode) : CXFA_Box(pNode) {}
};

enum XFA_CHECKSTATE {
  XFA_CHECKSTATE_On = 0,
  XFA_CHECKSTATE_Off = 1,
  XFA_CHECKSTATE_Neutral = 2,
};

enum XFA_VALUEPICTURE {
  XFA_VALUEPICTURE_Raw = 0,
  XFA_VALUEPICTURE_Display,
  XFA_VALUEPICTURE_Edit,
  XFA_VALUEPICTURE_DataBind,
};

class CXFA_WidgetData : public CXFA_Data {
 public:
  explicit CXFA_WidgetData(CXFA_Node* pNode);

  CXFA_Node* GetUIChild();
  XFA_ELEMENT GetUIType();
  CFX_WideString GetRawValue();
  int32_t GetAccess(FX_BOOL bTemplate = FALSE);
  int32_t GetRotate();
  CXFA_Border GetBorder(FX_BOOL bModified = FALSE);
  CXFA_Caption GetCaption(FX_BOOL bModified = FALSE);
  CXFA_Font GetFont(FX_BOOL bModified = FALSE);
  CXFA_Margin GetMargin(FX_BOOL bModified = FALSE);
  CXFA_Para GetPara(FX_BOOL bModified = FALSE);
  void GetEventList(CXFA_NodeArray& events);
  int32_t GetEventByActivity(int32_t iActivity,
                             CXFA_NodeArray& events,
                             FX_BOOL bIsFormReady = FALSE);
  CXFA_Value GetDefaultValue(FX_BOOL bModified = FALSE);
  CXFA_Value GetFormValue(FX_BOOL bModified = FALSE);
  CXFA_Calculate GetCalculate(FX_BOOL bModified = FALSE);
  CXFA_Validate GetValidate(FX_BOOL bModified = FALSE);
  CXFA_Bind GetBind(FX_BOOL bModified = FALSE);
  CXFA_Assist GetAssist(FX_BOOL bModified = FALSE);
  uint32_t GetRelevantStatus();
  FX_BOOL GetWidth(FX_FLOAT& fWidth);
  FX_BOOL GetHeight(FX_FLOAT& fHeight);
  FX_BOOL GetMinWidth(FX_FLOAT& fMinWidth);
  FX_BOOL GetMinHeight(FX_FLOAT& fMinHeight);
  FX_BOOL GetMaxWidth(FX_FLOAT& fMaxWidth);
  FX_BOOL GetMaxHeight(FX_FLOAT& fMaxHeight);
  CXFA_Border GetUIBorder(FX_BOOL bModified = FALSE);
  CXFA_Margin GetUIMargin(FX_BOOL bModified = FALSE);
  void GetUIMargin(CFX_RectF& rtUIMargin);
  int32_t GetButtonHighlight();
  FX_BOOL GetButtonRollover(CFX_WideString& wsRollover, FX_BOOL& bRichText);
  FX_BOOL GetButtonDown(CFX_WideString& wsDown, FX_BOOL& bRichText);
  int32_t GetCheckButtonShape();
  int32_t GetCheckButtonMark();
  FX_FLOAT GetCheckButtonSize();
  FX_BOOL IsAllowNeutral();
  FX_BOOL IsRadioButton();
  XFA_CHECKSTATE GetCheckState();
  void SetCheckState(XFA_CHECKSTATE eCheckState, FX_BOOL bNotify = TRUE);
  CXFA_Node* GetExclGroupNode();
  CXFA_Node* GetSelectedMember();
  CXFA_Node* SetSelectedMember(const CFX_WideStringC& wsName,
                               FX_BOOL bNotify = TRUE);
  void SetSelectedMemberByValue(const CFX_WideStringC& wsValue,
                                FX_BOOL bNotify = TRUE,
                                FX_BOOL bScriptModify = FALSE,
                                FX_BOOL bSyncData = TRUE);
  CXFA_Node* GetExclGroupFirstMember();
  CXFA_Node* GetExclGroupNextMember(CXFA_Node* pNode);
  int32_t GetChoiceListCommitOn();
  FX_BOOL IsChoiceListAllowTextEntry();
  int32_t GetChoiceListOpen();
  FX_BOOL IsListBox();
  int32_t CountChoiceListItems(FX_BOOL bSaveValue = FALSE);
  FX_BOOL GetChoiceListItem(CFX_WideString& wsText,
                            int32_t nIndex,
                            FX_BOOL bSaveValue = FALSE);
  void GetChoiceListItems(CFX_WideStringArray& wsTextArray,
                          FX_BOOL bSaveValue = FALSE);
  int32_t CountSelectedItems();
  int32_t GetSelectedItem(int32_t nIndex = 0);
  void GetSelectedItems(CFX_Int32Array& iSelArray);
  void GetSelectedItemsValue(CFX_WideStringArray& wsSelTextArray);
  FX_BOOL GetItemState(int32_t nIndex);
  void SetItemState(int32_t nIndex,
                    FX_BOOL bSelected,
                    FX_BOOL bNotify = FALSE,
                    FX_BOOL bScriptModify = FALSE,
                    FX_BOOL bSyncData = TRUE);
  void SetSelectdItems(CFX_Int32Array& iSelArray,
                       FX_BOOL bNotify = FALSE,
                       FX_BOOL bScriptModify = FALSE,
                       FX_BOOL bSyncData = TRUE);
  void ClearAllSelections();
  void InsertItem(const CFX_WideString& wsLabel,
                  const CFX_WideString& wsValue,
                  int32_t nIndex = -1,
                  FX_BOOL bNotify = FALSE);
  void GetItemLabel(const CFX_WideStringC& wsValue, CFX_WideString& wsLabel);
  void GetItemValue(const CFX_WideStringC& wsLabel, CFX_WideString& wsValue);
  FX_BOOL DeleteItem(int32_t nIndex,
                     FX_BOOL bNotify = FALSE,
                     FX_BOOL bScriptModify = FALSE,
                     FX_BOOL bSyncData = TRUE);
  int32_t GetHorizontalScrollPolicy();
  int32_t GetNumberOfCells();
  FX_BOOL SetValue(const CFX_WideString& wsValue, XFA_VALUEPICTURE eValueType);
  FX_BOOL GetPictureContent(CFX_WideString& wsPicture,
                            XFA_VALUEPICTURE ePicture);
  IFX_Locale* GetLocal();
  FX_BOOL GetValue(CFX_WideString& wsValue, XFA_VALUEPICTURE eValueType);
  FX_BOOL GetNormalizeDataValue(const CFX_WideStringC& wsValue,
                                CFX_WideString& wsNormalizeValue);
  FX_BOOL GetFormatDataValue(const CFX_WideStringC& wsValue,
                             CFX_WideString& wsFormatedValue);
  void NormalizeNumStr(const CFX_WideString& wsValue, CFX_WideString& wsOutput);
  CFX_WideString GetBarcodeType();
  FX_BOOL GetBarcodeAttribute_CharEncoding(int32_t& val);
  FX_BOOL GetBarcodeAttribute_Checksum(int32_t& val);
  FX_BOOL GetBarcodeAttribute_DataLength(int32_t& val);
  FX_BOOL GetBarcodeAttribute_StartChar(FX_CHAR& val);
  FX_BOOL GetBarcodeAttribute_EndChar(FX_CHAR& val);
  FX_BOOL GetBarcodeAttribute_ECLevel(int32_t& val);
  FX_BOOL GetBarcodeAttribute_ModuleWidth(int32_t& val);
  FX_BOOL GetBarcodeAttribute_ModuleHeight(int32_t& val);
  FX_BOOL GetBarcodeAttribute_PrintChecksum(FX_BOOL& val);
  FX_BOOL GetBarcodeAttribute_TextLocation(int32_t& val);
  FX_BOOL GetBarcodeAttribute_Truncate(FX_BOOL& val);
  FX_BOOL GetBarcodeAttribute_WideNarrowRatio(FX_FLOAT& val);
  void GetPasswordChar(CFX_WideString& wsPassWord);
  FX_BOOL IsMultiLine();
  int32_t GetVerticalScrollPolicy();
  int32_t GetMaxChars(XFA_ELEMENT& eType);
  FX_BOOL GetFracDigits(int32_t& iFracDigits);
  FX_BOOL GetLeadDigits(int32_t& iLeadDigits);

  FX_BOOL m_bIsNull;
  FX_BOOL m_bPreNull;

 protected:
  void SyncValue(const CFX_WideString& wsValue, FX_BOOL bNotify);
  void InsertListTextItem(CXFA_Node* pItems,
                          const CFX_WideStringC& wsText,
                          int32_t nIndex = -1);
  void FormatNumStr(const CFX_WideString& wsValue,
                    IFX_Locale* pLocale,
                    CFX_WideString& wsOutput);

  CXFA_Node* m_pUiChildNode;
  XFA_ELEMENT m_eUIType;
};

class CXFA_Occur : public CXFA_Data {
 public:
  explicit CXFA_Occur(CXFA_Node* pNode);

  int32_t GetMax();
  int32_t GetMin();
  FX_BOOL GetOccurInfo(int32_t& iMin, int32_t& iMax, int32_t& iInit);
  void SetMax(int32_t iMax);
  void SetMin(int32_t iMin);
};

#endif  // XFA_INCLUDE_FXFA_FXFA_OBJECTACC_H_
