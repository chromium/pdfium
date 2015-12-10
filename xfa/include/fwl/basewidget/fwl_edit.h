// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_EDIT_H
#define _FWL_EDIT_H
class CFWL_WidgetImpProperties;
class IFWL_Widget;
class IFWL_EditDP;
class IFWL_Edit;
#define FWL_CLASS_Edit L"FWL_EDIT"
#define FWL_CLASSHASH_Edit 2893987822
#define FWL_STYLEEXT_EDT_ReadOnly (1L << 0)
#define FWL_STYLEEXT_EDT_MultiLine (1L << 1)
#define FWL_STYLEEXT_EDT_WantReturn (1L << 2)
#define FWL_STYLEEXT_EDT_NoHideSel (1L << 3)
#define FWL_STYLEEXT_EDT_AutoHScroll (1L << 4)
#define FWL_STYLEEXT_EDT_AutoVScroll (1L << 5)
#define FWL_STYLEEXT_EDT_NoRedoUndo (1L << 6)
#define FWL_STYLEEXT_EDT_Validate (1L << 7)
#define FWL_STYLEEXT_EDT_Password (1L << 8)
#define FWL_STYLEEXT_EDT_Number (1L << 9)
#define FWL_STYLEEXT_EDT_HSelfAdaption (1L << 10)
#define FWL_STYLEEXT_EDT_VSelfAdaption (1L << 11)
#define FWL_STYLEEXT_EDT_VerticalLayout (1L << 12)
#define FWL_STYLEEXT_EDT_VerticalChars (1L << 13)
#define FWL_STYLEEXT_EDT_ReverseLine (1L << 14)
#define FWL_STYLEEXT_EDT_ArabicShapes (1L << 15)
#define FWL_STYLEEXT_EDT_ExpandTab (1L << 16)
#define FWL_STYLEEXT_EDT_CombText (1L << 17)
#define FWL_STYLEEXT_EDT_HNear (0L << 18)
#define FWL_STYLEEXT_EDT_HCenter (1L << 18)
#define FWL_STYLEEXT_EDT_HFar (2L << 18)
#define FWL_STYLEEXT_EDT_VNear (0L << 20)
#define FWL_STYLEEXT_EDT_VCenter (1L << 20)
#define FWL_STYLEEXT_EDT_VFar (2L << 20)
#define FWL_STYLEEXT_EDT_Justified (1L << 22)
#define FWL_STYLEEXT_EDT_Distributed (2L << 22)
#define FWL_STYLEEXT_EDT_HAlignMask (3L << 18)
#define FWL_STYLEEXT_EDT_VAlignMask (3L << 20)
#define FWL_STYLEEXT_EDT_HAlignModeMask (3L << 22)
#define FWL_STYLEEXT_EDT_InnerCaret (1L << 24)
#define FWL_STYLEEXT_EDT_ShowScrollbarFocus (1L << 25)
#define FWL_STYLEEXT_EDT_OuterScrollbar (1L << 26)
#define FWL_STYLEEXT_EDT_LastLineHeight (1L << 27)
#define FWL_STATE_EDT_Editing (1 << FWL_WGTSTATE_MAX)
#define FWL_PART_EDT_Border 1
#define FWL_PART_EDT_Edge 2
#define FWL_PART_EDT_Background 3
#define FWL_PART_EDT_CombTextLine 4
#define FWL_PARTDATA_EDT_Background 0
#define FWL_PARTDATA_EDT_StaticBackground 1
#define FWL_PARTSTATE_EDT_Normal (0L << 0)
#define FWL_PARTSTATE_EDT_ReadOnly (1L << 0)
#define FWL_PARTSTATE_EDT_Disable (2L << 0)
enum FWL_EDT_TEXTCHANGED {
  FWL_EDT_TEXTCHANGED_Insert = 0,
  FWL_EDT_TEXTCHANGED_Delete,
  FWL_EDT_TEXTCHANGED_Replace,
};
#define FWL_EVT_EDT_AddDoRecord L"FWL_EVENT_EDT_AddDoRecord"
#define FWL_EVTHASH_EDT_AddDoRecord 3701672224
#define FWL_EVT_EDT_TextChanged L"FWL_EVENT_EDT_TextChanged"
#define FWL_EVTHASH_EDT_TextChanged 1064022132
#define FWL_EVT_EDT_PreSelfAdaption L"FWL_EVENT_PreSelfAdaption"
#define FWL_EVTHASH_EDT_PreSelfAdaption 1001979178
#define FWL_EVT_EDT_Validate L"FWL_EVTHASH_EDT_Validate"
#define FWL_EVTHASH_EDT_Validate 3373308608
#define FWL_EVT_EDT_CheckWord L"FWL_EVTHASH_EDT_CheckWord"
#define FWL_EVTHASH_EDT_CheckWord 2897181520
#define FWL_EVT_EDT_GetSuggestWords L"FWL_EVTHASH_EDT_GetSuggestWords"
#define FWL_EVTHASH_EDT_GetSuggestWords 315782791
#define FWL_EVT_EDT_TextFull L"FWL_EVTHASH_EDT_TextFull"
#define FWL_EVTHASH_EDT_TextFull 2158580174
BEGIN_FWL_EVENT_DEF(CFWL_EvtEdtAddDoRecord, FWL_EVTHASH_EDT_AddDoRecord)
CFX_ByteString m_wsDoRecord;
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtEdtTextChanged, FWL_EVTHASH_EDT_TextChanged)
int32_t nChangeType;
CFX_WideString wsInsert;
CFX_WideString wsDelete;
CFX_WideString wsPrevText;
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtEdtTextFull, FWL_EVTHASH_EDT_TextFull)
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtEdtPreSelfAdaption, FWL_EVTHASH_EDT_PreSelfAdaption)
FX_BOOL bHSelfAdaption;
FX_BOOL bVSelfAdaption;
CFX_RectF rtAfterChange;
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtEdtValidate, FWL_EVTHASH_EDT_Validate)
IFWL_Widget* pDstWidget;
CFX_WideString wsInsert;
FX_BOOL bValidate;
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtEdtCheckWord, FWL_EVTHASH_EDT_CheckWord)
CFX_ByteString bsWord;
FX_BOOL bCheckWord;
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtEdtGetSuggestWords, FWL_EVTHASH_EDT_GetSuggestWords)
FX_BOOL bSuggestWords;
CFX_ByteString bsWord;
CFX_ByteStringArray bsArraySuggestWords;
END_FWL_EVENT_DEF
class IFWL_EditDP : public IFWL_DataProvider {};
#define FWL_EDT_FIND_FLAGS_Prev (0L << 0)
#define FWL_EDT_FIND_FLAGS_Next (1L << 0)
#define FWL_EDT_FIND_FLAGS_WholeWord (1L << 1)
#define FWL_EDT_FIND_FLAGS_NoCase (1L << 2)
typedef struct _FWL_HEDTFIND { void* pData; } * FWL_HEDTFIND;
class IFWL_Edit : public IFWL_Widget {
 public:
  static IFWL_Edit* Create(const CFWL_WidgetImpProperties& properties,
                           IFWL_Widget* pOuter);
  static IFWL_Edit* CreateComboEdit(const CFWL_WidgetImpProperties& properties,
                                    IFWL_Widget* pOuter);

  FWL_ERR SetText(const CFX_WideString& wsText);
  int32_t GetTextLength() const;
  FWL_ERR GetText(CFX_WideString& wsText,
                  int32_t nStart = 0,
                  int32_t nCount = -1) const;
  FWL_ERR ClearText();
  int32_t GetCaretPos() const;
  int32_t SetCaretPos(int32_t nIndex, FX_BOOL bBefore = TRUE);
  FWL_ERR AddSelRange(int32_t nStart, int32_t nCount = -1);
  int32_t CountSelRanges();
  int32_t GetSelRange(int32_t nIndex, int32_t& nStart);
  FWL_ERR ClearSelections();
  int32_t GetLimit();
  FWL_ERR SetLimit(int32_t nLimit);
  FWL_ERR SetAliasChar(FX_WCHAR wAlias);
  FWL_ERR SetFormatString(const CFX_WideString& wsFormat);
  FWL_ERR Insert(int32_t nStart, const FX_WCHAR* lpText, int32_t nLen);
  FWL_ERR DeleteSelections();
  FWL_ERR DeleteRange(int32_t nStart, int32_t nCount = -1);
  FWL_ERR ReplaceSelections(const CFX_WideStringC& wsReplace);
  FWL_ERR Replace(int32_t nStart,
                  int32_t nLen,
                  const CFX_WideStringC& wsReplace);
  FWL_ERR DoClipboard(int32_t iCmd);
  FX_BOOL Copy(CFX_WideString& wsCopy);
  FX_BOOL Cut(CFX_WideString& wsCut);
  FX_BOOL Paste(const CFX_WideString& wsPaste);
  FX_BOOL Delete();
  FX_BOOL Redo(const CFX_ByteStringC& bsRecord);
  FX_BOOL Undo(const CFX_ByteStringC& bsRecord);
  FX_BOOL Undo();
  FX_BOOL Redo();
  FX_BOOL CanUndo();
  FX_BOOL CanRedo();
  FWL_ERR SetTabWidth(FX_FLOAT fTabWidth, FX_BOOL bEquidistant);
  FWL_ERR SetOuter(IFWL_Widget* pOuter);
  FWL_ERR SetNumberRange(int32_t iMin, int32_t iMax);
  FWL_ERR SetBackColor(FX_DWORD dwColor);
  FWL_ERR SetFont(const CFX_WideString& wsFont, FX_FLOAT fSize);
  void SetScrollOffset(FX_FLOAT fScrollOffset);
  FX_BOOL GetSuggestWords(CFX_PointF pointf, CFX_ByteStringArray& sSuggest);
  FX_BOOL ReplaceSpellCheckWord(CFX_PointF pointf,
                                const CFX_ByteStringC& bsReplace);

 protected:
  IFWL_Edit();
};
#endif
