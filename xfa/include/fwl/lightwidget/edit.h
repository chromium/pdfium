// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_EDIT_LIGHT_H
#define _FWL_EDIT_LIGHT_H
class CFWL_Widget;
class CFWL_WidgetProperties;
class CFWL_Edit;
class CFWL_Edit : public CFWL_Widget {
 public:
  static CFWL_Edit* Create();
  FWL_ERR Initialize(const CFWL_WidgetProperties* pProperties = NULL);
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
  FX_BOOL Redo(const CFX_ByteStringC& bsRecord);
  FX_BOOL Undo(const CFX_ByteStringC& bsRecord);
  FWL_ERR SetTabWidth(FX_FLOAT fTabWidth, FX_BOOL bEquidistant);
  FWL_ERR SetNumberRange(int32_t iMin, int32_t iMax);
  FWL_ERR SetBackColor(FX_DWORD dwColor);
  FWL_ERR SetFont(const CFX_WideString& wsFont, FX_FLOAT fSize);
  FX_BOOL CanUndo();
  FX_BOOL CanRedo();
  FX_BOOL Undo();
  FX_BOOL Redo();
  FX_BOOL Copy(CFX_WideString& wsCopy);
  FX_BOOL Cut(CFX_WideString& wsCut);
  FX_BOOL Paste(const CFX_WideString& wsPaste);
  FX_BOOL Delete();
  void SetScrollOffset(FX_FLOAT fScrollOffset);
  FX_BOOL GetSuggestWords(CFX_PointF pointf, CFX_ByteStringArray& sSuggest);
  FX_BOOL ReplaceSpellCheckWord(CFX_PointF pointf,
                                const CFX_ByteStringC& bsReplace);
  CFWL_Edit();
  virtual ~CFWL_Edit();
};
#endif
