// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXFA_OBJECTACC_H_
#define FXFA_OBJECTACC_H_

#include "core/include/fxge/fx_dib.h"  // For FX_ARGB.

class CXFA_Node;
class IFX_Locale;
class CXFA_Data;
class CXFA_Font;
class CXFA_Fill;
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
class CXFA_Variables;
class CXFA_Bind;
class CXFA_Assist;
class CXFA_ToolTip;
class CXFA_Keep;
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
class CXFA_Filter;
class CXFA_Certificate;
class CXFA_WrapCertificate;
class CXFA_Oids;
class CXFA_SubjectDNs;
class CXFA_DigestMethods;
class CXFA_Encodings;
class CXFA_EncryptionMethods;
class CXFA_Reasons;
class CXFA_Manifest;
inline FX_BOOL XFA_IsSpace(FX_WCHAR c) {
  return (c == 0x20) || (c == 0x0d) || (c == 0x0a) || (c == 0x09);
}
inline FX_BOOL XFA_IsDigit(FX_WCHAR c) {
  return c >= '0' && c <= '9';
}
typedef CFX_ArrayTemplate<CXFA_Node*> CXFA_NodeArray;
typedef CFX_ArrayTemplate<CXFA_Object*> CXFA_ObjArray;
class CXFA_Data {
 public:
  CXFA_Data(CXFA_Node* pNode) : m_pNode(pNode) {}
  operator CXFA_Node*() const { return m_pNode; }
  CXFA_Node* GetNode() { return m_pNode; }

  FX_BOOL IsExistInXML() const { return m_pNode != NULL; }

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
  CXFA_Fill(CXFA_Node* pNode);
  ~CXFA_Fill();

  int32_t GetPresence();

  FX_ARGB GetColor(FX_BOOL bText = FALSE);

  void SetColor(FX_ARGB color);

  int32_t GetFillType();

  int32_t GetPattern(FX_ARGB& foreColor);

  int32_t GetStipple(FX_ARGB& stippleColor);

  int32_t GetLinear(FX_ARGB& endColor);

  int32_t GetRadial(FX_ARGB& endColor);
  FX_BOOL SetPresence(int32_t iPresence);
  FX_BOOL SetFillType(int32_t iType);
  FX_BOOL SetPattern(int32_t iPattern, FX_ARGB foreColor);
  FX_BOOL SetStipple(int32_t iStipple, FX_ARGB stippleColor);
  FX_BOOL SetLinear(int32_t iLinear, FX_ARGB endColor);
  FX_BOOL SetRadial(int32_t iRadial, FX_ARGB endColor);
};
class CXFA_Margin : public CXFA_Data {
 public:
  CXFA_Margin(CXFA_Node* pNode);
  FX_BOOL GetLeftInset(FX_FLOAT& fInset, FX_FLOAT fDefInset = 0) const;
  FX_BOOL GetTopInset(FX_FLOAT& fInset, FX_FLOAT fDefInset = 0) const;
  FX_BOOL GetRightInset(FX_FLOAT& fInset, FX_FLOAT fDefInset = 0) const;
  FX_BOOL GetBottomInset(FX_FLOAT& fInset, FX_FLOAT fDefInset = 0) const;
  FX_BOOL SetLeftInset(FX_FLOAT fInset);
  FX_BOOL SetTopInset(FX_FLOAT fInset);
  FX_BOOL SetRightInset(FX_FLOAT fInset);
  FX_BOOL SetBottomInset(FX_FLOAT fInset);
};
class CXFA_Font : public CXFA_Data {
 public:
  CXFA_Font(CXFA_Node* pNode);

  FX_FLOAT GetBaselineShift();

  FX_FLOAT GetHorizontalScale();

  FX_FLOAT GetVerticalScale();

  FX_FLOAT GetLetterSpacing();

  int32_t GetLineThrough();

  int32_t GetLineThroughPeriod();

  int32_t GetOverline();

  int32_t GetOverlinePeriod();

  int32_t GetUnderline();

  int32_t GetUnderlinePeriod();

  FX_FLOAT GetFontSize();

  void GetTypeface(CFX_WideStringC& wsTypeFace);

  FX_BOOL IsBold();

  FX_BOOL IsItalic();

  FX_BOOL IsUseKerning();

  FX_ARGB GetColor();

  void SetColor(FX_ARGB color);
  FX_BOOL SetBaselineShift(FX_FLOAT fBaselineShift);
  FX_BOOL SetHorizontalScale(FX_FLOAT fHorizontalScale);
  FX_BOOL SetVerticalScale(FX_FLOAT fVerticalScale);
  FX_BOOL SetLetterSpacing(FX_FLOAT fLetterSpacing, XFA_UNIT eUnit);
  FX_BOOL SetLineThrough(int32_t iLineThrough);
  FX_BOOL SetLineThroughPeriod(int32_t iLineThroughPeriod);
  FX_BOOL SetOverline(int32_t iOverline);
  FX_BOOL SetOverlinePeriod(int32_t iOverlinePeriod);
  FX_BOOL SetUnderline(int32_t iUnderline);
  FX_BOOL SetUnderlinePeriod(int32_t iUnderlinePeriod);
};
class CXFA_Caption : public CXFA_Data {
 public:
  CXFA_Caption(CXFA_Node* pNode);

  int32_t GetPresence();

  int32_t GetPlacementType();

  FX_FLOAT GetReserve();

  CXFA_Margin GetMargin();

  CXFA_Font GetFont();

  CXFA_Value GetValue();

  CXFA_Para GetPara();
  FX_BOOL SetPresence(int32_t iPresence);
  FX_BOOL SetPlacementType(int32_t iType);
  FX_BOOL SetReserve(FX_FLOAT fReserve);
};
class CXFA_Para : public CXFA_Data {
 public:
  CXFA_Para(CXFA_Node* pNode);

  int32_t GetHorizontalAlign();

  int32_t GetVerticalAlign();

  FX_FLOAT GetLineHeight();
  FX_FLOAT GetMarginLeft();
  FX_FLOAT GetMarginRight();
  int32_t GetOrphans();
  FX_FLOAT GetRadixOffset();
  FX_FLOAT GetSpaceAbove();
  FX_FLOAT GetSpaceBelow();
  FX_FLOAT GetTextIndent();
  int32_t GetWidows();
  FX_BOOL SetHorizontalAlign(int32_t iHorizontalAlign);
  FX_BOOL SetVerticalAlign(int32_t iVerticalAlign);
  FX_BOOL SetLineHeight(FX_FLOAT fLineHeight);
  FX_BOOL SetMarginLeft(FX_FLOAT fMarginLeft);
  FX_BOOL SetMarginRight(FX_FLOAT fMarginRight);
  FX_BOOL SetOrphans(int32_t iOrphans);
  FX_BOOL SetRadixOffset(FX_FLOAT fRadixOffset);
  FX_BOOL SetSpaceAbove(FX_FLOAT fSpaceAbove);
  FX_BOOL SetSpaceBelow(FX_FLOAT fSpaceBelow);
  FX_BOOL SetTextIndent(FX_FLOAT fTextIndent);
  FX_BOOL SetWidows(int32_t iWidows);
};
class CXFA_Keep : public CXFA_Data {
 public:
  CXFA_Keep(CXFA_Node* pNode, CXFA_Node* pParent);

  int32_t GetIntact();
  int32_t GetNext();
  int32_t GetPrevious();
  FX_BOOL SetIntact(int32_t iIntact);
  FX_BOOL SetNext(int32_t iNext);
  FX_BOOL SetPrevious(int32_t iPrevious);

 private:
  CXFA_Node* m_pParent;
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
  CXFA_Event(CXFA_Node* pNode);

  int32_t GetActivity();

  int32_t GetEventType();
  void GetRef(CFX_WideStringC& wsRef);

  int32_t GetExecuteRunAt();
  int32_t GetExecuteType();
  void GetExecuteConnection(CFX_WideString& wsConnection);

  CXFA_Script GetScript();

  CXFA_Submit GetSubmit();

  int32_t GetSignDataOperation();
  void GetSignDataTarget(CFX_WideString& wsTarget);
  FX_BOOL SetActivity(int32_t iActivity);
  FX_BOOL SetEventType(int32_t iEventType);
  FX_BOOL SetExecuteRunAt(int32_t iExecuteRunAt);
  FX_BOOL SetExecuteType(int32_t iExecuteType);
  FX_BOOL SetExecuteConnection(const CFX_WideString& wsConnection);
  FX_BOOL SetSignDataOperation(int32_t iOperation);
  FX_BOOL SetSignDataTarget(const CFX_WideString& wsTarget);
};
enum XFA_SCRIPTTYPE {
  XFA_SCRIPTTYPE_Formcalc = 0,
  XFA_SCRIPTTYPE_Javascript,
  XFA_SCRIPTTYPE_Unkown,
};
class CXFA_Script : public CXFA_Data {
 public:
  CXFA_Script(CXFA_Node* pNode);
  void GetBinding(CFX_WideString& wsBinding);

  XFA_SCRIPTTYPE GetContentType();
  int32_t GetRunAt();
  void GetExpression(CFX_WideString& wsExpression);
  FX_BOOL SetBinding(const CFX_WideString& wsBinding);
  FX_BOOL SetContentType(XFA_SCRIPTTYPE eType);
  FX_BOOL SetRunAt(int32_t iRunAt);
  FX_BOOL SetExpression(const CFX_WideString& wsExpression);
};
class CXFA_Submit : public CXFA_Data {
 public:
  CXFA_Submit(CXFA_Node* pNode);
  FX_BOOL IsSubmitEmbedPDF();
  int32_t GetSubmitFormat();
  void GetSubmitTarget(CFX_WideStringC& wsTarget);
  XFA_TEXTENCODING GetSubmitTextEncoding();
  void GetSubmitXDPContent(CFX_WideStringC& wsContent);
  FX_BOOL SetSubmitFormat(int32_t iSubmitFormat);
  FX_BOOL SetSubmitTarget(const CFX_WideString& wsTarget);
  FX_BOOL SetSubmitTextEncoding(XFA_TEXTENCODING eTextEncoding);
  FX_BOOL SetSubmitXDPContent(const CFX_WideString& wsContent);
};
class CXFA_Value : public CXFA_Data {
 public:
  CXFA_Value(CXFA_Node* pNode) : CXFA_Data(pNode) {}

  XFA_ELEMENT GetChildValueClassID();

  FX_BOOL GetChildValueContent(CFX_WideString& wsContent);
  CXFA_Arc GetArc();
  CXFA_Line GetLine();
  CXFA_Rectangle GetRectangle();
  CXFA_Text GetText();
  CXFA_ExData GetExData();
  CXFA_Image GetImage();
  FX_BOOL SetChildValueContent(const CFX_WideString& wsContent,
                               FX_BOOL bNotify = FALSE,
                               XFA_ELEMENT iType = XFA_ELEMENT_UNKNOWN);
};
class CXFA_Line : public CXFA_Data {
 public:
  CXFA_Line(CXFA_Node* pNode) : CXFA_Data(pNode) {}
  int32_t GetHand();
  FX_BOOL GetSlop();
  CXFA_Edge GetEdge();
  FX_BOOL SetHand(int32_t iHand);
  FX_BOOL SetSlop(int32_t iSlop);
};
class CXFA_Text : public CXFA_Data {
 public:
  CXFA_Text(CXFA_Node* pNode);
  void GetName(CFX_WideStringC& wsName);
  int32_t GetMaxChars();
  void GetRid(CFX_WideStringC& wsRid);
  void GetContent(CFX_WideString& wsText);
  void SetContent(CFX_WideString wsText, FX_BOOL bNotify = TRUE);
  FX_BOOL SetName(const CFX_WideString& wsName);
  FX_BOOL SetMaxChars(int32_t iMaxChars);
  FX_BOOL SetRid(const CFX_WideString& wsRid);
};
class CXFA_ExData : public CXFA_Data {
 public:
  CXFA_ExData(CXFA_Node* pNode);
  void GetContentType(CFX_WideStringC& wsContentType);
  void GetHref(CFX_WideStringC& wsHref);
  int32_t GetMaxLength();
  void GetRid(CFX_WideStringC& wsRid);
  int32_t GetTransferEncoding();
  void GetContent(CFX_WideString& wsText);
  FX_BOOL SetContentType(const CFX_WideString& wsContentType);
  FX_BOOL SetHref(const CFX_WideString& wsHref);
  FX_BOOL SetMaxLength(int32_t iMaxLength);
  FX_BOOL SetRid(const CFX_WideString& wsRid);
  FX_BOOL SetTransferEncoding(int32_t iTransferEncoding);
  FX_BOOL SetContent(const CFX_WideString& wsText,
                     FX_BOOL bNotify = FALSE,
                     FX_BOOL bScriptModify = FALSE,
                     FX_BOOL bSyncData = TRUE);
};
class CXFA_Image : public CXFA_Data {
 public:
  CXFA_Image(CXFA_Node* pNode, FX_BOOL bDefValue);
  int32_t GetAspect();
  FX_BOOL GetContentType(CFX_WideString& wsContentType);
  FX_BOOL GetHref(CFX_WideString& wsHref);
  int32_t GetTransferEncoding();
  FX_BOOL GetContent(CFX_WideString& wsText);
  FX_BOOL SetAspect(int32_t iAspect);
  FX_BOOL SetContentType(const CFX_WideString& wsContentType);
  FX_BOOL SetHref(const CFX_WideString& wsHref);
  FX_BOOL SetTransferEncoding(int32_t iTransferEncoding);
  FX_BOOL SetContent(const CFX_WideString& wsText);

 protected:
  FX_BOOL m_bDefValue;
};
class CXFA_Calculate : public CXFA_Data {
 public:
  CXFA_Calculate(CXFA_Node* pNode);

  int32_t GetOverride();
  CXFA_Script GetScript();
  void GetMessageText(CFX_WideString& wsMessage);
  FX_BOOL SetOverride(int32_t iOverride);
  FX_BOOL SetMessageText(const CFX_WideString& wsMessage);
};
class CXFA_Validate : public CXFA_Data {
 public:
  CXFA_Validate(CXFA_Node* pNode);
  int32_t GetFormatTest();
  FX_BOOL SetFormatTest(CFX_WideString wsValue);
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
class CXFA_Variables : public CXFA_Data {
 public:
  CXFA_Variables(CXFA_Node* pNode);

  int32_t CountScripts();
  CXFA_Script GetScript(int32_t nIndex);
};
class CXFA_Bind : public CXFA_Data {
 public:
  CXFA_Bind(CXFA_Node* pNode);
  int32_t GetMatch();
  void GetRef(CFX_WideStringC& wsRef);
  void GetPicture(CFX_WideString& wsPicture);
  FX_BOOL SetMatch(int32_t iMatch);
  FX_BOOL SetRef(const CFX_WideString& wsRef);
  FX_BOOL SetPicture(const CFX_WideString& wsPicture);
};
class CXFA_Assist : public CXFA_Data {
 public:
  CXFA_Assist(CXFA_Node* pNode);

  CXFA_ToolTip GetToolTip();
};
class CXFA_ToolTip : public CXFA_Data {
 public:
  CXFA_ToolTip(CXFA_Node* pNode);
  FX_BOOL GetTip(CFX_WideString& wsTip);
  FX_BOOL SetTip(const CFX_WideString& wsTip);
};
class CXFA_BindItems : public CXFA_Data {
 public:
  CXFA_BindItems(CXFA_Node* pNode);
  void GetConnection(CFX_WideStringC& wsConnection);
  void GetLabelRef(CFX_WideStringC& wsLabelRef);
  void GetValueRef(CFX_WideStringC& wsValueRef);
  void GetRef(CFX_WideStringC& wsRef);
  FX_BOOL SetConnection(const CFX_WideString& wsConnection);
  FX_BOOL SetLabelRef(const CFX_WideString& wsLabelRef);
  FX_BOOL SetValueRef(const CFX_WideString& wsValueRef);
  FX_BOOL SetRef(const CFX_WideString& wsRef);
};
#define XFA_STROKE_SAMESTYLE_NoPresence 1
#define XFA_STROKE_SAMESTYLE_Corner 2
class CXFA_Stroke : public CXFA_Data {
 public:
  CXFA_Stroke(CXFA_Node* pNode) : CXFA_Data(pNode) {}

  FX_BOOL IsCorner() const { return GetClassID() == XFA_ELEMENT_Corner; }

  FX_BOOL IsEdge() const { return GetClassID() == XFA_ELEMENT_Edge; }

  int32_t GetPresence() const;
  FX_BOOL IsVisible() const {
    return GetPresence() == XFA_ATTRIBUTEENUM_Visible;
  }

  int32_t GetCapType() const;

  int32_t GetStrokeType() const;

  FX_FLOAT GetThickness() const;
  CXFA_Measurement GetMSThickness() const;

  void SetThickness(FX_FLOAT fThickness);
  void SetMSThickness(CXFA_Measurement msThinkness);

  FX_ARGB GetColor() const;

  void SetColor(FX_ARGB argb);

  int32_t GetJoinType() const;

  FX_BOOL IsInverted() const;

  FX_FLOAT GetRadius() const;

  FX_BOOL SameStyles(CXFA_Stroke stroke, FX_DWORD dwFlags = 0) const;
};
class CXFA_Corner : public CXFA_Stroke {
 public:
  CXFA_Corner(CXFA_Node* pNode) : CXFA_Stroke(pNode) {}
};
class CXFA_Edge : public CXFA_Stroke {
 public:
  CXFA_Edge(CXFA_Node* pNode) : CXFA_Stroke(pNode) {}
};
typedef CFX_ArrayTemplate<CXFA_Stroke> CXFA_StrokeArray;
typedef CFX_ArrayTemplate<CXFA_Edge> CXFA_EdgeArray;
typedef CFX_ArrayTemplate<CXFA_Corner> CXFA_CornerArray;
class CXFA_Box : public CXFA_Data {
 public:
  CXFA_Box(CXFA_Node* pNode) : CXFA_Data(pNode) {}

  FX_BOOL IsArc() const { return GetClassID() == XFA_ELEMENT_Arc; }

  FX_BOOL IsBorder() const { return GetClassID() == XFA_ELEMENT_Border; }

  FX_BOOL IsRectangle() const { return GetClassID() == XFA_ELEMENT_Rectangle; }

  int32_t GetBreak() const;

  int32_t GetHand() const;

  int32_t GetPresence() const;

  int32_t CountCorners() const;

  CXFA_Corner GetCorner(int32_t nIndex) const;

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

  FX_BOOL SameStyles() const;

  int32_t Get3DStyle(FX_BOOL& bVisible, FX_FLOAT& fThickness) const;
};
class CXFA_Arc : public CXFA_Box {
 public:
  CXFA_Arc(CXFA_Node* pNode) : CXFA_Box(pNode) {}
};
class CXFA_Border : public CXFA_Box {
 public:
  CXFA_Border(CXFA_Node* pNode) : CXFA_Box(pNode) {}
};
class CXFA_Rectangle : public CXFA_Box {
 public:
  CXFA_Rectangle(CXFA_Node* pNode) : CXFA_Box(pNode) {}
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
  CXFA_WidgetData(CXFA_Node* pNode);
  CXFA_Node* GetUIChild();

  XFA_ELEMENT GetUIType();
  CFX_WideString GetRawValue();
  int32_t GetAccess(FX_BOOL bTemplate = FALSE);
  FX_BOOL GetAccessKey(CFX_WideStringC& wsAccessKey);
  int32_t GetAnchorType();
  int32_t GetColSpan();
  int32_t GetPresence();
  int32_t GetRotate();
  CXFA_Border GetBorder(FX_BOOL bModified = FALSE);
  CXFA_Caption GetCaption(FX_BOOL bModified = FALSE);
  CXFA_Font GetFont(FX_BOOL bModified = FALSE);
  CXFA_Margin GetMargin(FX_BOOL bModified = FALSE);
  CXFA_Para GetPara(FX_BOOL bModified = FALSE);
  CXFA_Keep GetKeep(FX_BOOL bModified = FALSE);
  void GetEventList(CXFA_NodeArray& events);
  int32_t GetEventByActivity(int32_t iActivity,
                             CXFA_NodeArray& events,
                             FX_BOOL bIsFormReady = FALSE);
  CXFA_Value GetDefaultValue(FX_BOOL bModified = FALSE);
  CXFA_Value GetFormValue(FX_BOOL bModified = FALSE);
  CXFA_Calculate GetCalculate(FX_BOOL bModified = FALSE);
  CXFA_Validate GetValidate(FX_BOOL bModified = FALSE);
  CXFA_Variables GetVariables(FX_BOOL bModified = FALSE);
  CXFA_Bind GetBind(FX_BOOL bModified = FALSE);
  CXFA_Assist GetAssist(FX_BOOL bModified = FALSE);
  void GetRelevant(CFX_WideStringC& wsRelevant);
  FX_DWORD GetRelevantStatus();
  FX_BOOL GetWidth(FX_FLOAT& fWidth);
  FX_BOOL GetHeight(FX_FLOAT& fHeight);
  FX_BOOL GetMinWidth(FX_FLOAT& fMinWidth);
  FX_BOOL GetMinHeight(FX_FLOAT& fMinHeight);
  FX_BOOL GetMaxWidth(FX_FLOAT& fMaxWidth);
  FX_BOOL GetMaxHeight(FX_FLOAT& fMaxHeight);
  CXFA_BindItems GetBindItems();
  FX_BOOL SetAccess(int32_t iAccess, FX_BOOL bNotify = TRUE);
  FX_BOOL SetAccessKey(const CFX_WideString& wsAccessKey);
  FX_BOOL SetAnchorType(int32_t iType);
  FX_BOOL SetColSpan(int32_t iColSpan);
  FX_BOOL SetPresence(int32_t iPresence);
  FX_BOOL SetRotate(int32_t iRotate);
  FX_BOOL SetRelevant(const CFX_WideString& wsRelevant);
  FX_BOOL SetStatus(FX_DWORD dwStatus);
  FX_BOOL SetWidth(FX_FLOAT fWidth);
  FX_BOOL SetHeight(FX_FLOAT fHeight);
  FX_BOOL SetMinWidth(FX_FLOAT fMinWidth);
  FX_BOOL SetMinHeight(FX_FLOAT fMinHeight);
  FX_BOOL SetMaxWidth(FX_FLOAT fMaxWidth);
  FX_BOOL SetMaxHeight(FX_FLOAT fMaxHeight);
  FX_BOOL SetPos(FX_FLOAT x, FX_FLOAT y);
  FX_BOOL SetName(const CFX_WideString& wsName);
  FX_BOOL SetButtonHighlight(int32_t iButtonHighlight);
  FX_BOOL SetButtonRollover(const CFX_WideString& wsRollover,
                            FX_BOOL bRichText);
  FX_BOOL SetButtonDown(const CFX_WideString& wsDown, FX_BOOL bRichText);
  FX_BOOL SetCheckButtonShape(int32_t iCheckButtonShape);
  FX_BOOL SetCheckButtonMark(int32_t iCheckButtonMark);
  FX_BOOL SetCheckButtonSize(FX_FLOAT fCheckButtonMark);
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
  FX_BOOL IsDateTimeEditUsePicker();
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
  FX_BOOL IsAllowRichText();
  FX_BOOL IsMultiLine();
  int32_t GetVerticalScrollPolicy();
  int32_t GetMaxChars(XFA_ELEMENT& eType);
  FX_BOOL GetFracDigits(int32_t& iFracDigits);
  FX_BOOL GetLeadDigits(int32_t& iLeadDigits);
  CXFA_Filter GetFilter(FX_BOOL bModified = FALSE);
  CXFA_Manifest GetManifest(FX_BOOL bModified = FALSE);

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
  CXFA_Occur(CXFA_Node* pNode);
  int32_t GetMax();
  int32_t GetMin();
  int32_t GetInitial();
  FX_BOOL GetOccurInfo(int32_t& iMin, int32_t& iMax, int32_t& iInit);
  void SetMax(int32_t iMax);
  void SetMin(int32_t iMin);
};
class CXFA_Filter : public CXFA_Data {
 public:
  CXFA_Filter(CXFA_Node* pNode) : CXFA_Data(pNode) {}
  CFX_WideString GetFilterString(XFA_ATTRIBUTE eAttribute);
  XFA_ATTRIBUTEENUM GetAppearanceFilterType();
  CFX_WideString GetAppearanceFilterContent();
  XFA_ATTRIBUTEENUM GetCertificatesCredentialServerPolicy();
  CFX_WideString GetCertificatesURL();
  CFX_WideString GetCertificatesURLPolicy();
  CXFA_WrapCertificate GetCertificatesEncryption(FX_BOOL bModified = FALSE);
  CXFA_WrapCertificate GetCertificatesIssuers(FX_BOOL bModified = FALSE);
  CFX_WideString GetCertificatesKeyUsageString(XFA_ATTRIBUTE eAttribute);
  CXFA_Oids GetCertificatesOids();
  CXFA_WrapCertificate GetCertificatesSigning(FX_BOOL bModified = FALSE);
  CXFA_DigestMethods GetDigestMethods(FX_BOOL bModified = FALSE);
  CXFA_Encodings GetEncodings(FX_BOOL bModified = FALSE);
  CXFA_EncryptionMethods GetEncryptionMethods(FX_BOOL bModified = FALSE);
  XFA_ATTRIBUTEENUM GetHandlerType();
  CFX_WideString GetHandlerContent();
  XFA_ATTRIBUTEENUM GetlockDocumentType();
  CFX_WideString GetlockDocumentContent();
  int32_t GetMDPPermissions();
  XFA_ATTRIBUTEENUM GetMDPSignatureType();

  CXFA_Reasons GetReasons(FX_BOOL bModified = FALSE);
  CFX_WideString GetTimeStampServer();
  XFA_ATTRIBUTEENUM GetTimeStampType();
};
class CXFA_Certificate : public CXFA_Data {
 public:
  CXFA_Certificate(CXFA_Node* pNode) : CXFA_Data(pNode) {}
  CFX_WideString GetCertificateName();
  CFX_WideString GetCertificateContent();
};
class CXFA_WrapCertificate : public CXFA_Data {
 public:
  CXFA_WrapCertificate(CXFA_Node* pNode) : CXFA_Data(pNode) {}
  XFA_ATTRIBUTEENUM GetType();
  int32_t CountCertificates();
  CXFA_Certificate GetCertificate(int32_t nIndex);
};
class CXFA_Oids : public CXFA_Data {
 public:
  CXFA_Oids(CXFA_Node* pNode) : CXFA_Data(pNode) {}
  XFA_ATTRIBUTEENUM GetOidsType();
  int32_t CountOids();
  CFX_WideString GetOidContent(int32_t nIndex);
};
class CXFA_SubjectDNs : public CXFA_Data {
 public:
  CXFA_SubjectDNs(CXFA_Node* pNode) : CXFA_Data(pNode) {}
  XFA_ATTRIBUTEENUM GetSubjectDNsType();
  int32_t CountSubjectDNs();
  CFX_WideString GetSubjectDNString(int32_t nIndex, XFA_ATTRIBUTE eAttribute);
  CFX_WideString GetSubjectDNContent(int32_t nIndex);
};
class CXFA_DigestMethods : public CXFA_Data {
 public:
  CXFA_DigestMethods(CXFA_Node* pNode) : CXFA_Data(pNode) {}
  XFA_ATTRIBUTEENUM GetDigestMethodsType();
  int32_t CountDigestMethods();
  CFX_WideString GetDigestMethodContent(int32_t nIndex);
};
class CXFA_Encodings : public CXFA_Data {
 public:
  CXFA_Encodings(CXFA_Node* pNode) : CXFA_Data(pNode) {}
  XFA_ATTRIBUTEENUM GetEncodingsType();
  int32_t CountEncodings();
  CFX_WideString GetEncodingContent(int32_t nIndex);
};
class CXFA_EncryptionMethods : public CXFA_Data {
 public:
  CXFA_EncryptionMethods(CXFA_Node* pNode) : CXFA_Data(pNode) {}
  XFA_ATTRIBUTEENUM GetEncryptionMethodsType();
  int32_t CountEncryptionMethods();
  CFX_WideString GetEncryptionMethodContent(int32_t nIndex);
};
class CXFA_Reasons : public CXFA_Data {
 public:
  CXFA_Reasons(CXFA_Node* pNode) : CXFA_Data(pNode) {}
  XFA_ATTRIBUTEENUM GetReasonsType();
  int32_t CountReasons();
  CFX_WideString GetReasonContent(int32_t nIndex);
};
class CXFA_Manifest : public CXFA_Data {
 public:
  CXFA_Manifest(CXFA_Node* pNode) : CXFA_Data(pNode) {}
  XFA_ATTRIBUTEENUM GetAction();
  int32_t CountReives();
  CFX_WideString GetRefContent(int32_t nIndex);
};

#endif  // FXFA_OBJECTACC_H_
