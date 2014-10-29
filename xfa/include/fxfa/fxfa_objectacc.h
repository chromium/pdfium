// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_NODEACC_H
#define _FXFA_NODEACC_H
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
inline FX_BOOL XFA_IsSpace(FX_WCHAR c)
{
    return (c == 0x20) || (c == 0x0d) || (c == 0x0a) || (c == 0x09);
}
inline FX_BOOL XFA_IsDigit(FX_WCHAR c)
{
    return c >= '0' && c <= '9';
}
typedef CFX_ArrayTemplate<CXFA_Node*> CXFA_NodeArray;
typedef CFX_ArrayTemplate<CXFA_Object*> CXFA_ObjArray;
class CXFA_Data : public CFX_Object
{
public:
    CXFA_Data(CXFA_Node* pNode) : m_pNode(pNode) {}
    operator CXFA_Node*() const
    {
        return m_pNode;
    }
    CXFA_Node*	GetNode()
    {
        return m_pNode;
    }

    FX_BOOL			IsExistInXML() const
    {
        return m_pNode != NULL;
    }

    XFA_ELEMENT		GetClassID() const;
protected:
    FX_BOOL TryMeasure(XFA_ATTRIBUTE eAttr, FX_FLOAT &fValue, FX_BOOL bUseDefault = FALSE) const;
    FX_BOOL	SetMeasure(XFA_ATTRIBUTE eAttr, FX_FLOAT fValue);
    CXFA_Node*				m_pNode;
};
class CXFA_Fill : public CXFA_Data
{
public:
    CXFA_Fill(CXFA_Node* pNode);
    ~CXFA_Fill();

    FX_INT32		GetPresence();

    FX_ARGB			GetColor(FX_BOOL bText = FALSE);

    void			SetColor(FX_ARGB color);


    FX_INT32		GetFillType();

    FX_INT32		GetPattern(FX_ARGB& foreColor);

    FX_INT32		GetStipple(FX_ARGB& stippleColor);

    FX_INT32		GetLinear(FX_ARGB& endColor);

    FX_INT32		GetRadial(FX_ARGB& endColor);
    FX_BOOL			SetPresence(FX_INT32 iPresence);
    FX_BOOL			SetFillType(FX_INT32 iType);
    FX_BOOL			SetPattern(FX_INT32 iPattern, FX_ARGB foreColor);
    FX_BOOL			SetStipple(FX_INT32 iStipple, FX_ARGB stippleColor);
    FX_BOOL			SetLinear(FX_INT32 iLinear, FX_ARGB endColor);
    FX_BOOL			SetRadial(FX_INT32 iRadial, FX_ARGB endColor);
};
class CXFA_Margin : public CXFA_Data
{
public:
    CXFA_Margin(CXFA_Node* pNode);
    FX_BOOL		GetLeftInset(FX_FLOAT &fInset, FX_FLOAT fDefInset = 0) const;
    FX_BOOL		GetTopInset(FX_FLOAT &fInset, FX_FLOAT fDefInset = 0) const;
    FX_BOOL		GetRightInset(FX_FLOAT &fInset, FX_FLOAT fDefInset = 0) const;
    FX_BOOL		GetBottomInset(FX_FLOAT &fInset, FX_FLOAT fDefInset = 0) const;
    FX_BOOL		SetLeftInset(FX_FLOAT fInset);
    FX_BOOL		SetTopInset(FX_FLOAT fInset);
    FX_BOOL		SetRightInset(FX_FLOAT fInset);
    FX_BOOL		SetBottomInset(FX_FLOAT fInset);
};
class CXFA_Font : public CXFA_Data
{
public:
    CXFA_Font(CXFA_Node* pNode);

    FX_FLOAT		GetBaselineShift();

    FX_FLOAT		GetHorizontalScale();

    FX_FLOAT		GetVerticalScale();

    FX_FLOAT		GetLetterSpacing();

    FX_INT32		GetLineThrough();

    FX_INT32		GetLineThroughPeriod();

    FX_INT32		GetOverline();

    FX_INT32		GetOverlinePeriod();

    FX_INT32		GetUnderline();

    FX_INT32		GetUnderlinePeriod();

    FX_FLOAT		GetFontSize();

    void			GetTypeface(CFX_WideStringC &wsTypeFace);

    FX_BOOL			IsBold();

    FX_BOOL			IsItalic();

    FX_BOOL			IsUseKerning();

    FX_ARGB			GetColor();

    void			SetColor(FX_ARGB color);
    FX_BOOL			SetBaselineShift(FX_FLOAT fBaselineShift);
    FX_BOOL			SetHorizontalScale(FX_FLOAT fHorizontalScale);
    FX_BOOL			SetVerticalScale(FX_FLOAT fVerticalScale);
    FX_BOOL			SetLetterSpacing(FX_FLOAT fLetterSpacing, XFA_UNIT eUnit);
    FX_BOOL			SetLineThrough(FX_INT32 iLineThrough);
    FX_BOOL			SetLineThroughPeriod(FX_INT32 iLineThroughPeriod);
    FX_BOOL			SetOverline(FX_INT32 iOverline);
    FX_BOOL			SetOverlinePeriod(FX_INT32 iOverlinePeriod);
    FX_BOOL			SetUnderline(FX_INT32 iUnderline);
    FX_BOOL			SetUnderlinePeriod(FX_INT32 iUnderlinePeriod);
};
class CXFA_Caption : public CXFA_Data
{
public:
    CXFA_Caption(CXFA_Node* pNode);

    FX_INT32		GetPresence();

    FX_INT32		GetPlacementType();

    FX_FLOAT		GetReserve();

    CXFA_Margin		GetMargin();

    CXFA_Font		GetFont();

    CXFA_Value		GetValue();

    CXFA_Para		GetPara();
    FX_BOOL			SetPresence(FX_INT32 iPresence);
    FX_BOOL			SetPlacementType(FX_INT32 iType);
    FX_BOOL			SetReserve(FX_FLOAT fReserve);
};
class CXFA_Para : public CXFA_Data
{
public:
    CXFA_Para(CXFA_Node* pNode);

    FX_INT32		GetHorizontalAlign();

    FX_INT32		GetVerticalAlign();

    FX_FLOAT		GetLineHeight();
    FX_FLOAT		GetMarginLeft();
    FX_FLOAT		GetMarginRight();
    FX_INT32		GetOrphans();
    FX_FLOAT		GetRadixOffset();
    FX_FLOAT		GetSpaceAbove();
    FX_FLOAT		GetSpaceBelow();
    FX_FLOAT		GetTextIndent();
    FX_INT32		GetWidows();
    FX_BOOL			SetHorizontalAlign(FX_INT32 iHorizontalAlign);
    FX_BOOL			SetVerticalAlign(FX_INT32 iVerticalAlign);
    FX_BOOL			SetLineHeight(FX_FLOAT fLineHeight);
    FX_BOOL			SetMarginLeft(FX_FLOAT fMarginLeft);
    FX_BOOL			SetMarginRight(FX_FLOAT fMarginRight);
    FX_BOOL			SetOrphans(FX_INT32 iOrphans);
    FX_BOOL			SetRadixOffset(FX_FLOAT fRadixOffset);
    FX_BOOL			SetSpaceAbove(FX_FLOAT fSpaceAbove);
    FX_BOOL			SetSpaceBelow(FX_FLOAT fSpaceBelow);
    FX_BOOL			SetTextIndent(FX_FLOAT fTextIndent);
    FX_BOOL			SetWidows(FX_INT32 iWidows);
};
class CXFA_Keep : public CXFA_Data
{
public:
    CXFA_Keep(CXFA_Node *pNode, CXFA_Node *pParent);

    FX_INT32		GetIntact();
    FX_INT32		GetNext();
    FX_INT32		GetPrevious();
    FX_BOOL			SetIntact(FX_INT32 iIntact);
    FX_BOOL			SetNext(FX_INT32 iNext);
    FX_BOOL			SetPrevious(FX_INT32 iPrevious);
private:
    CXFA_Node		*m_pParent;
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
class CXFA_Event : public CXFA_Data
{
public:
    CXFA_Event(CXFA_Node* pNode);


    FX_INT32			GetActivity();

    FX_INT32			GetEventType();
    void				GetRef(CFX_WideStringC &wsRef);

    FX_INT32			GetExecuteRunAt();
    FX_INT32			GetExecuteType();
    void				GetExecuteConnection(CFX_WideString &wsConnection);


    CXFA_Script			GetScript();


    CXFA_Submit			GetSubmit();


    FX_INT32			GetSignDataOperation();
    void				GetSignDataTarget(CFX_WideString &wsTarget);
    FX_BOOL				SetActivity(FX_INT32 iActivity);
    FX_BOOL				SetEventType(FX_INT32 iEventType);
    FX_BOOL				SetExecuteRunAt(FX_INT32 iExecuteRunAt);
    FX_BOOL				SetExecuteType(FX_INT32 iExecuteType);
    FX_BOOL				SetExecuteConnection(const CFX_WideString& wsConnection);
    FX_BOOL				SetSignDataOperation(FX_INT32 iOperation);
    FX_BOOL				SetSignDataTarget(const CFX_WideString& wsTarget);
};
enum XFA_SCRIPTTYPE {
    XFA_SCRIPTTYPE_Formcalc = 0,
    XFA_SCRIPTTYPE_Javascript,
    XFA_SCRIPTTYPE_Unkown,
};
class CXFA_Script : public CXFA_Data
{
public:
    CXFA_Script(CXFA_Node* pNode);
    void				GetBinding(CFX_WideString &wsBinding);

    XFA_SCRIPTTYPE		GetContentType();
    FX_INT32			GetRunAt();
    void				GetExpression(CFX_WideString &wsExpression);
    FX_BOOL				SetBinding(const CFX_WideString& wsBinding);
    FX_BOOL				SetContentType(XFA_SCRIPTTYPE eType);
    FX_BOOL				SetRunAt(FX_INT32 iRunAt);
    FX_BOOL				SetExpression(const CFX_WideString& wsExpression);
};
class CXFA_Submit : public CXFA_Data
{
public:
    CXFA_Submit(CXFA_Node* pNode);
    FX_BOOL				IsSubmitEmbedPDF();
    FX_INT32			GetSubmitFormat();
    void				GetSubmitTarget(CFX_WideStringC &wsTarget);
    XFA_TEXTENCODING	GetSubmitTextEncoding();
    void				GetSubmitXDPContent(CFX_WideStringC &wsContent);
    FX_BOOL				SetSubmitFormat(FX_INT32 iSubmitFormat);
    FX_BOOL				SetSubmitTarget(const CFX_WideString& wsTarget);
    FX_BOOL				SetSubmitTextEncoding(XFA_TEXTENCODING eTextEncoding);
    FX_BOOL				SetSubmitXDPContent(const CFX_WideString& wsContent);
};
class CXFA_Value : public CXFA_Data
{
public:
    CXFA_Value(CXFA_Node* pNode) : CXFA_Data(pNode) {}

    XFA_ELEMENT		GetChildValueClassID();

    FX_BOOL			GetChildValueContent(CFX_WideString &wsContent);
    CXFA_Arc		GetArc();
    CXFA_Line		GetLine();
    CXFA_Rectangle	GetRectangle();
    CXFA_Text		GetText();
    CXFA_ExData		GetExData();
    CXFA_Image		GetImage();
    FX_BOOL			SetChildValueContent(const CFX_WideString& wsContent, FX_BOOL bNotify = FALSE, XFA_ELEMENT iType = XFA_ELEMENT_UNKNOWN);
};
class CXFA_Line : public CXFA_Data
{
public:
    CXFA_Line(CXFA_Node* pNode) : CXFA_Data(pNode) {}
    FX_INT32		GetHand();
    FX_BOOL			GetSlop();
    CXFA_Edge		GetEdge();
    FX_BOOL			SetHand(FX_INT32 iHand);
    FX_BOOL			SetSlop(FX_INT32 iSlop);
};
class CXFA_Text : public CXFA_Data
{
public:
    CXFA_Text(CXFA_Node* pNode);
    void		GetName(CFX_WideStringC &wsName);
    FX_INT32	GetMaxChars();
    void		GetRid(CFX_WideStringC &wsRid);
    void		GetContent(CFX_WideString &wsText);
    void		SetContent(CFX_WideString wsText, FX_BOOL bNotify = TRUE);
    FX_BOOL		SetName(const CFX_WideString& wsName);
    FX_BOOL		SetMaxChars(FX_INT32 iMaxChars);
    FX_BOOL		SetRid(const CFX_WideString& wsRid);
};
class CXFA_ExData : public CXFA_Data
{
public:
    CXFA_ExData(CXFA_Node* pNode);
    void		GetContentType(CFX_WideStringC &wsContentType);
    void		GetHref(CFX_WideStringC &wsHref);
    FX_INT32	GetMaxLength();
    void		GetRid(CFX_WideStringC &wsRid);
    FX_INT32	GetTransferEncoding();
    void		GetContent(CFX_WideString &wsText);
    FX_BOOL		SetContentType(const CFX_WideString& wsContentType);
    FX_BOOL		SetHref(const CFX_WideString& wsHref);
    FX_BOOL		SetMaxLength(FX_INT32 iMaxLength);
    FX_BOOL		SetRid(const CFX_WideString& wsRid);
    FX_BOOL		SetTransferEncoding(FX_INT32 iTransferEncoding);
    FX_BOOL		SetContent(const CFX_WideString& wsText, FX_BOOL bNotify = FALSE,  FX_BOOL bScriptModify = FALSE, FX_BOOL bSyncData = TRUE);
};
class CXFA_Image : public CXFA_Data
{
public:
    CXFA_Image(CXFA_Node* pNode, FX_BOOL bDefValue);
    FX_INT32	GetAspect();
    FX_BOOL		GetContentType(CFX_WideString &wsContentType);
    FX_BOOL		GetHref(CFX_WideString &wsHref);
    FX_INT32	GetTransferEncoding();
    FX_BOOL		GetContent(CFX_WideString &wsText);
    FX_BOOL		SetAspect(FX_INT32 iAspect);
    FX_BOOL		SetContentType(const CFX_WideString& wsContentType);
    FX_BOOL		SetHref(const CFX_WideString& wsHref);
    FX_BOOL		SetTransferEncoding(FX_INT32 iTransferEncoding);
    FX_BOOL		SetContent(const CFX_WideString& wsText);
protected:
    FX_BOOL		m_bDefValue;
};
class CXFA_Calculate : public CXFA_Data
{
public:
    CXFA_Calculate(CXFA_Node* pNode);

    FX_INT32		GetOverride();
    CXFA_Script		GetScript();
    void			GetMessageText(CFX_WideString &wsMessage);
    FX_BOOL			SetOverride(FX_INT32 iOverride);
    FX_BOOL			SetMessageText(const CFX_WideString& wsMessage);
};
class CXFA_Validate : public CXFA_Data
{
public:
    CXFA_Validate(CXFA_Node* pNode);
    FX_INT32	GetFormatTest();
    FX_BOOL		SetFormatTest(CFX_WideString wsValue);
    FX_INT32	GetNullTest();
    FX_BOOL		SetNullTest(CFX_WideString wsValue);
    FX_INT32	GetScriptTest();
    void			GetFormatMessageText(CFX_WideString &wsMessage);
    void			SetFormatMessageText(CFX_WideString wsMessage);
    void			GetNullMessageText(CFX_WideString &wsMessage);
    void			SetNullMessageText(CFX_WideString wsMessage);
    void			GetScriptMessageText(CFX_WideString &wsMessage);
    void			SetScriptMessageText(CFX_WideString wsMessage);
    void			GetPicture(CFX_WideString &wsPicture);
    CXFA_Script		GetScript();
protected:
    void		GetMessageText(CFX_WideString &wsMessage, FX_WSTR wsMessageType);
    void		SetMessageText(CFX_WideString &wsMessage, FX_WSTR wsMessageType);
    FX_BOOL		SetTestValue(FX_INT32 iType, CFX_WideString &wsValue, XFA_ATTRIBUTEENUM eName);
};
class CXFA_Variables : public CXFA_Data
{
public:
    CXFA_Variables(CXFA_Node* pNode);

    FX_INT32	CountScripts();
    CXFA_Script	GetScript(FX_INT32 nIndex);
};
class CXFA_Bind : public CXFA_Data
{
public:
    CXFA_Bind(CXFA_Node* pNode);
    FX_INT32	GetMatch();
    void		GetRef(CFX_WideStringC &wsRef);
    void		GetPicture(CFX_WideString &wsPicture);
    FX_BOOL		SetMatch(FX_INT32 iMatch);
    FX_BOOL		SetRef(const CFX_WideString& wsRef);
    FX_BOOL		SetPicture(const CFX_WideString& wsPicture);
};
class CXFA_Assist : public CXFA_Data
{
public:
    CXFA_Assist(CXFA_Node* pNode);

    CXFA_ToolTip	GetToolTip();
};
class CXFA_ToolTip : public CXFA_Data
{
public:
    CXFA_ToolTip(CXFA_Node* pNode);
    FX_BOOL GetTip(CFX_WideString &wsTip);
    FX_BOOL	SetTip(const CFX_WideString& wsTip);
};
class CXFA_BindItems : public CXFA_Data
{
public:
    CXFA_BindItems(CXFA_Node* pNode);
    void GetConnection(CFX_WideStringC &wsConnection);
    void GetLabelRef(CFX_WideStringC &wsLabelRef);
    void GetValueRef(CFX_WideStringC &wsValueRef);
    void GetRef(CFX_WideStringC &wsRef);
    FX_BOOL SetConnection(const CFX_WideString& wsConnection);
    FX_BOOL SetLabelRef(const CFX_WideString& wsLabelRef);
    FX_BOOL SetValueRef(const CFX_WideString& wsValueRef);
    FX_BOOL SetRef(const CFX_WideString& wsRef);
};
#define XFA_STROKE_SAMESTYLE_NoPresence	1
#define XFA_STROKE_SAMESTYLE_Corner		2
class CXFA_Stroke : public CXFA_Data
{
public:

    CXFA_Stroke(CXFA_Node* pNode) : CXFA_Data(pNode) {}

    FX_BOOL			IsCorner() const
    {
        return GetClassID() == XFA_ELEMENT_Corner;
    }

    FX_BOOL			IsEdge() const
    {
        return GetClassID() == XFA_ELEMENT_Edge;
    }

    FX_INT32		GetPresence() const;
    FX_BOOL			IsVisible() const
    {
        return GetPresence() == XFA_ATTRIBUTEENUM_Visible;
    }

    FX_INT32		GetCapType() const;

    FX_INT32		GetStrokeType() const;

    FX_FLOAT		GetThickness() const;
    CXFA_Measurement GetMSThickness() const;

    void			SetThickness(FX_FLOAT fThickness);
    void			SetMSThickness(CXFA_Measurement msThinkness);

    FX_ARGB			GetColor() const;

    void			SetColor(FX_ARGB argb);

    FX_INT32		GetJoinType() const;

    FX_BOOL			IsInverted() const;

    FX_FLOAT		GetRadius() const;

    FX_BOOL			SameStyles(CXFA_Stroke stroke, FX_DWORD dwFlags = 0) const;
};
class CXFA_Corner : public CXFA_Stroke
{
public:
    CXFA_Corner(CXFA_Node* pNode) : CXFA_Stroke(pNode) {}
};
class CXFA_Edge : public CXFA_Stroke
{
public:
    CXFA_Edge(CXFA_Node* pNode) : CXFA_Stroke(pNode) {}
};
typedef CFX_ArrayTemplate<CXFA_Stroke>	CXFA_StrokeArray;
typedef CFX_ArrayTemplate<CXFA_Edge>	CXFA_EdgeArray;
typedef CFX_ArrayTemplate<CXFA_Corner>	CXFA_CornerArray;
class CXFA_Box : public CXFA_Data
{
public:

    CXFA_Box(CXFA_Node *pNode) : CXFA_Data(pNode) {}

    FX_BOOL			IsArc() const
    {
        return GetClassID() == XFA_ELEMENT_Arc;
    }

    FX_BOOL			IsBorder() const
    {
        return GetClassID() == XFA_ELEMENT_Border;
    }

    FX_BOOL			IsRectangle() const
    {
        return GetClassID() == XFA_ELEMENT_Rectangle;
    }

    FX_INT32		GetBreak() const;

    FX_INT32		GetHand() const;

    FX_INT32		GetPresence() const;

    FX_INT32		CountCorners() const;

    CXFA_Corner		GetCorner(FX_INT32 nIndex) const;

    FX_INT32		CountEdges() const;

    CXFA_Edge		GetEdge(FX_INT32 nIndex = 0) const;

    void			GetStrokes(CXFA_StrokeArray &strokes) const;

    FX_BOOL			IsCircular() const;

    FX_BOOL			GetStartAngle(FX_FLOAT &fStartAngle) const;
    FX_FLOAT		GetStartAngle() const
    {
        FX_FLOAT fStartAngle;
        GetStartAngle(fStartAngle);
        return fStartAngle;
    }

    FX_BOOL			GetSweepAngle(FX_FLOAT &fSweepAngle) const;
    FX_FLOAT		GetSweepAngle() const
    {
        FX_FLOAT fSweepAngle;
        GetSweepAngle(fSweepAngle);
        return fSweepAngle;
    }

    CXFA_Fill		GetFill(FX_BOOL bModified = FALSE) const;

    CXFA_Margin		GetMargin() const;

    FX_BOOL			SameStyles() const;

    FX_INT32		Get3DStyle(FX_BOOL &bVisible, FX_FLOAT &fThickness) const;
};
class CXFA_Arc : public CXFA_Box
{
public:
    CXFA_Arc(CXFA_Node *pNode) : CXFA_Box(pNode) {}
};
class CXFA_Border : public CXFA_Box
{
public:
    CXFA_Border(CXFA_Node *pNode) : CXFA_Box(pNode) {}
};
class CXFA_Rectangle : public CXFA_Box
{
public:
    CXFA_Rectangle(CXFA_Node *pNode) : CXFA_Box(pNode) {}
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
class CXFA_WidgetData : public CXFA_Data
{
public:
    CXFA_WidgetData(CXFA_Node *pNode);
    CXFA_Node*		GetUIChild();

    XFA_ELEMENT		GetUIType();
    CFX_WideString	GetRawValue();

    FX_INT32		GetAccess(FX_BOOL bTemplate = FALSE);

    FX_BOOL			GetAccessKey(CFX_WideStringC &wsAccessKey);

    FX_INT32		GetAnchorType();

    FX_INT32		GetColSpan();

    FX_INT32		GetPresence();

    FX_INT32		GetRotate();

    CXFA_Border		GetBorder(FX_BOOL bModified = FALSE);

    CXFA_Caption	GetCaption(FX_BOOL bModified = FALSE);

    CXFA_Font		GetFont(FX_BOOL bModified = FALSE);

    CXFA_Margin		GetMargin(FX_BOOL bModified = FALSE);

    CXFA_Para		GetPara(FX_BOOL bModified = FALSE);

    CXFA_Keep		GetKeep(FX_BOOL bModified = FALSE);

    void			GetEventList(CXFA_NodeArray &events);
    FX_INT32		GetEventByActivity(FX_INT32 iActivity, CXFA_NodeArray &events, FX_BOOL bIsFormReady = FALSE);

    CXFA_Value		GetDefaultValue(FX_BOOL bModified = FALSE);

    CXFA_Value		GetFormValue(FX_BOOL bModified = FALSE);

    CXFA_Calculate	GetCalculate(FX_BOOL bModified = FALSE);

    CXFA_Validate	GetValidate(FX_BOOL bModified = FALSE);

    CXFA_Variables	GetVariables(FX_BOOL bModified = FALSE);

    CXFA_Bind		GetBind(FX_BOOL bModified = FALSE);

    CXFA_Assist		GetAssist(FX_BOOL bModified = FALSE);

    void			GetRelevant(CFX_WideStringC &wsRelevant);
    FX_DWORD		GetRelevantStatus();

    FX_BOOL			GetWidth(FX_FLOAT &fWidth);
    FX_BOOL			GetHeight(FX_FLOAT &fHeight);

    FX_BOOL			GetMinWidth(FX_FLOAT &fMinWidth);
    FX_BOOL			GetMinHeight(FX_FLOAT &fMinHeight);

    FX_BOOL			GetMaxWidth(FX_FLOAT &fMaxWidth);
    FX_BOOL			GetMaxHeight(FX_FLOAT &fMaxHeight);

    CXFA_BindItems	GetBindItems();
    FX_BOOL			SetAccess(FX_INT32 iAccess, FX_BOOL bNotify = TRUE);
    FX_BOOL			SetAccessKey(const CFX_WideString& wsAccessKey);
    FX_BOOL			SetAnchorType(FX_INT32 iType);
    FX_BOOL			SetColSpan(FX_INT32 iColSpan);
    FX_BOOL			SetPresence(FX_INT32 iPresence);
    FX_BOOL			SetRotate(FX_INT32 iRotate);
    FX_BOOL			SetRelevant(const CFX_WideString& wsRelevant);
    FX_BOOL			SetStatus(FX_DWORD dwStatus);
    FX_BOOL			SetWidth(FX_FLOAT fWidth);
    FX_BOOL			SetHeight(FX_FLOAT fHeight);
    FX_BOOL			SetMinWidth(FX_FLOAT fMinWidth);
    FX_BOOL			SetMinHeight(FX_FLOAT fMinHeight);
    FX_BOOL			SetMaxWidth(FX_FLOAT fMaxWidth);
    FX_BOOL			SetMaxHeight(FX_FLOAT fMaxHeight);
    FX_BOOL			SetPos(FX_FLOAT x, FX_FLOAT y);
    FX_BOOL			SetName(const CFX_WideString& wsName);
    FX_BOOL			SetButtonHighlight(FX_INT32 iButtonHighlight);
    FX_BOOL			SetButtonRollover(const CFX_WideString &wsRollover, FX_BOOL bRichText);
    FX_BOOL			SetButtonDown(const CFX_WideString& wsDown, FX_BOOL bRichText);
    FX_BOOL			SetCheckButtonShape(FX_INT32 iCheckButtonShape);
    FX_BOOL			SetCheckButtonMark(FX_INT32 iCheckButtonMark);
    FX_BOOL			SetCheckButtonSize(FX_FLOAT fCheckButtonMark);

    CXFA_Border		GetUIBorder(FX_BOOL bModified = FALSE);

    CXFA_Margin		GetUIMargin(FX_BOOL bModified = FALSE);
    void			GetUIMargin(CFX_RectF &rtUIMargin);

    FX_INT32		GetButtonHighlight();
    FX_BOOL			GetButtonRollover(CFX_WideString &wsRollover, FX_BOOL &bRichText);
    FX_BOOL			GetButtonDown(CFX_WideString &wsDown, FX_BOOL &bRichText);


    FX_INT32		GetCheckButtonShape();

    FX_INT32		GetCheckButtonMark();

    FX_FLOAT		GetCheckButtonSize();

    FX_BOOL			IsAllowNeutral();
    FX_BOOL			IsRadioButton();
    XFA_CHECKSTATE	GetCheckState();
    void			SetCheckState(XFA_CHECKSTATE eCheckState, FX_BOOL bNotify = TRUE);

    CXFA_Node*		GetExclGroupNode();

    CXFA_Node*		GetSelectedMember();

    CXFA_Node*		SetSelectedMember(FX_WSTR wsName, FX_BOOL bNotify = TRUE);

    void			SetSelectedMemberByValue(FX_WSTR wsValue, FX_BOOL bNotify = TRUE, FX_BOOL bScriptModify = FALSE, FX_BOOL bSyncData = TRUE);
    CXFA_Node*		GetExclGroupFirstMember();
    CXFA_Node*		GetExclGroupNextMember(CXFA_Node* pNode);

    FX_INT32		GetChoiceListCommitOn();

    FX_BOOL			IsChoiceListAllowTextEntry();

    FX_INT32		GetChoiceListOpen();
    FX_BOOL			IsListBox();
    FX_INT32		CountChoiceListItems(FX_BOOL bSaveValue = FALSE);

    FX_BOOL			GetChoiceListItem(CFX_WideString &wsText, FX_INT32 nIndex, FX_BOOL bSaveValue = FALSE);
    void			GetChoiceListItems(CFX_WideStringArray &wsTextArray, FX_BOOL bSaveValue = FALSE);

    FX_INT32		CountSelectedItems();

    FX_INT32		GetSelectedItem(FX_INT32 nIndex = 0);
    void			GetSelectedItems(CFX_Int32Array &iSelArray);
    void			GetSelectedItemsValue(CFX_WideStringArray &wsSelTextArray);

    FX_BOOL			GetItemState(FX_INT32 nIndex);

    void			SetItemState(FX_INT32 nIndex, FX_BOOL bSelected, FX_BOOL bNotify = FALSE, FX_BOOL bScriptModify = FALSE, FX_BOOL bSyncData = TRUE);
    void			SetSelectdItems(CFX_Int32Array &iSelArray, FX_BOOL bNotify = FALSE, FX_BOOL bScriptModify = FALSE, FX_BOOL bSyncData = TRUE);
    void			ClearAllSelections();
    void			InsertItem(const CFX_WideString &wsLabel, const CFX_WideString &wsValue, FX_INT32 nIndex = -1, FX_BOOL bNotify = FALSE);
    void			GetItemLabel(FX_WSTR wsValue, CFX_WideString &wsLabel);
    void			GetItemValue(FX_WSTR wsLabel, CFX_WideString &wsValue);
    FX_BOOL			DeleteItem(FX_INT32 nIndex, FX_BOOL bNotify = FALSE, FX_BOOL bScriptModify = FALSE, FX_BOOL bSyncData = TRUE);

    FX_INT32		GetHorizontalScrollPolicy();

    FX_INT32		GetNumberOfCells();

    FX_BOOL			IsDateTimeEditUsePicker();

    FX_BOOL		SetValue(const CFX_WideString& wsValue, XFA_VALUEPICTURE eValueType);
    FX_BOOL		GetPictureContent(CFX_WideString &wsPicture, XFA_VALUEPICTURE ePicture);
    IFX_Locale* GetLocal();
    FX_BOOL		GetValue(CFX_WideString &wsValue, XFA_VALUEPICTURE eValueType);
    FX_BOOL		GetNormalizeDataValue(FX_WSTR wsValue, CFX_WideString &wsNormalizeValue);
    FX_BOOL		GetFormatDataValue(FX_WSTR wsValue, CFX_WideString &wsFormatedValue);
    void		NormalizeNumStr(const CFX_WideString& wsValue, CFX_WideString& wsOutput);

    CFX_WideString	GetBarcodeType();
    FX_BOOL			GetBarcodeAttribute_CharEncoding(FX_INT32& val);
    FX_BOOL			GetBarcodeAttribute_Checksum(FX_INT32& val);
    FX_BOOL			GetBarcodeAttribute_DataLength(FX_INT32& val);
    FX_BOOL			GetBarcodeAttribute_StartChar(FX_CHAR& val);
    FX_BOOL			GetBarcodeAttribute_EndChar(FX_CHAR& val);
    FX_BOOL			GetBarcodeAttribute_ECLevel(FX_INT32& val);
    FX_BOOL			GetBarcodeAttribute_ModuleWidth(FX_INT32& val);
    FX_BOOL			GetBarcodeAttribute_ModuleHeight(FX_INT32& val);
    FX_BOOL			GetBarcodeAttribute_PrintChecksum(FX_BOOL& val);
    FX_BOOL			GetBarcodeAttribute_TextLocation(FX_INT32& val);
    FX_BOOL			GetBarcodeAttribute_Truncate(FX_BOOL& val);
    FX_BOOL			GetBarcodeAttribute_WideNarrowRatio(FX_FLOAT& val);

    void			GetPasswordChar(CFX_WideString &wsPassWord);

    FX_BOOL			IsAllowRichText();

    FX_BOOL			IsMultiLine();

    FX_INT32		GetVerticalScrollPolicy();

    FX_INT32		GetMaxChars(XFA_ELEMENT& eType);

    FX_BOOL			GetFracDigits(FX_INT32 &iFracDigits);

    FX_BOOL			GetLeadDigits(FX_INT32 &iLeadDigits);

    CXFA_Filter		GetFilter(FX_BOOL bModified = FALSE);
    CXFA_Manifest	GetManifest(FX_BOOL bModified = FALSE);
    FX_BOOL			m_bIsNull;
    FX_BOOL			m_bPreNull;
protected:
    void			SyncValue(const CFX_WideString& wsValue, FX_BOOL bNotify);
    void			InsertListTextItem(CXFA_Node* pItems, FX_WSTR wsText, FX_INT32 nIndex = -1);
    void			FormatNumStr(const CFX_WideString& wsValue, IFX_Locale* pLocale, CFX_WideString& wsOutput);
    CXFA_Node*		m_pUiChildNode;
    XFA_ELEMENT		m_eUIType;
};
class CXFA_Occur : public CXFA_Data
{
public:
    CXFA_Occur(CXFA_Node* pNode);
    FX_INT32	GetMax();
    FX_INT32	GetMin();
    FX_INT32	GetInitial();
    FX_BOOL		GetOccurInfo(FX_INT32& iMin, FX_INT32& iMax, FX_INT32& iInit);
    void		SetMax(FX_INT32 iMax);
    void		SetMin(FX_INT32 iMin);
};
class CXFA_Filter : public CXFA_Data
{
public:
    CXFA_Filter(CXFA_Node* pNode) : CXFA_Data(pNode) {}
    CFX_WideString			GetFilterString(XFA_ATTRIBUTE eAttribute);
    XFA_ATTRIBUTEENUM		GetAppearanceFilterType();
    CFX_WideString			GetAppearanceFilterContent();
    XFA_ATTRIBUTEENUM		GetCertificatesCredentialServerPolicy();
    CFX_WideString			GetCertificatesURL();
    CFX_WideString			GetCertificatesURLPolicy();
    CXFA_WrapCertificate	GetCertificatesEncryption(FX_BOOL bModified = FALSE);
    CXFA_WrapCertificate	GetCertificatesIssuers(FX_BOOL bModified = FALSE);
    CFX_WideString			GetCertificatesKeyUsageString(XFA_ATTRIBUTE eAttribute);
    CXFA_Oids				GetCertificatesOids();
    CXFA_WrapCertificate	GetCertificatesSigning(FX_BOOL bModified = FALSE);
    CXFA_DigestMethods		GetDigestMethods(FX_BOOL bModified = FALSE);
    CXFA_Encodings			GetEncodings(FX_BOOL bModified = FALSE);
    CXFA_EncryptionMethods	GetEncryptionMethods(FX_BOOL bModified = FALSE);
    XFA_ATTRIBUTEENUM		GetHandlerType();
    CFX_WideString			GetHandlerContent();
    XFA_ATTRIBUTEENUM		GetlockDocumentType();
    CFX_WideString			GetlockDocumentContent();
    FX_INT32				GetMDPPermissions();
    XFA_ATTRIBUTEENUM		GetMDPSignatureType();

    CXFA_Reasons			GetReasons(FX_BOOL bModified = FALSE);
    CFX_WideString			GetTimeStampServer();
    XFA_ATTRIBUTEENUM		GetTimeStampType();
};
class CXFA_Certificate : public CXFA_Data
{
public:
    CXFA_Certificate(CXFA_Node* pNode) : CXFA_Data(pNode) {}
    CFX_WideString		GetCertificateName();
    CFX_WideString		GetCertificateContent();
};
class CXFA_WrapCertificate : public CXFA_Data
{
public:
    CXFA_WrapCertificate(CXFA_Node* pNode) : CXFA_Data(pNode) {}
    XFA_ATTRIBUTEENUM	GetType();
    FX_INT32			CountCertificates();
    CXFA_Certificate	GetCertificate(FX_INT32 nIndex);
};
class CXFA_Oids : public CXFA_Data
{
public:
    CXFA_Oids(CXFA_Node* pNode) : CXFA_Data(pNode) {}
    XFA_ATTRIBUTEENUM	GetOidsType();
    FX_INT32			CountOids();
    CFX_WideString		GetOidContent(FX_INT32 nIndex);
};
class CXFA_SubjectDNs : public CXFA_Data
{
public:
    CXFA_SubjectDNs(CXFA_Node* pNode) : CXFA_Data(pNode) {}
    XFA_ATTRIBUTEENUM	GetSubjectDNsType();
    FX_INT32				CountSubjectDNs();
    CFX_WideString			GetSubjectDNString(FX_INT32 nIndex, XFA_ATTRIBUTE eAttribute);
    CFX_WideString			GetSubjectDNContent(FX_INT32 nIndex);
};
class CXFA_DigestMethods : public CXFA_Data
{
public:
    CXFA_DigestMethods(CXFA_Node* pNode) : CXFA_Data(pNode) {}
    XFA_ATTRIBUTEENUM	GetDigestMethodsType();
    FX_INT32				CountDigestMethods();
    CFX_WideString			GetDigestMethodContent(FX_INT32 nIndex);
};
class CXFA_Encodings : public CXFA_Data
{
public:
    CXFA_Encodings(CXFA_Node* pNode) : CXFA_Data(pNode) {}
    XFA_ATTRIBUTEENUM		GetEncodingsType();
    FX_INT32				CountEncodings();
    CFX_WideString			GetEncodingContent(FX_INT32 nIndex);
};
class CXFA_EncryptionMethods : public CXFA_Data
{
public:
    CXFA_EncryptionMethods(CXFA_Node* pNode) : CXFA_Data(pNode) {}
    XFA_ATTRIBUTEENUM		GetEncryptionMethodsType();
    FX_INT32				CountEncryptionMethods();
    CFX_WideString			GetEncryptionMethodContent(FX_INT32 nIndex);
};
class CXFA_Reasons : public CXFA_Data
{
public:
    CXFA_Reasons(CXFA_Node* pNode) : CXFA_Data(pNode) {}
    XFA_ATTRIBUTEENUM		GetReasonsType();
    FX_INT32				CountReasons();
    CFX_WideString			GetReasonContent(FX_INT32 nIndex);
};
class CXFA_Manifest : public CXFA_Data
{
public:
    CXFA_Manifest(CXFA_Node* pNode) : CXFA_Data(pNode) {}
    XFA_ATTRIBUTEENUM	GetAction();
    FX_INT32			CountReives();
    CFX_WideString		GetRefContent(FX_INT32 nIndex);
};
#endif
