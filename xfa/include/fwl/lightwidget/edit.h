// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_EDIT_LIGHT_H
#define _FWL_EDIT_LIGHT_H
class CFWL_Widget;
class CFWL_WidgetProperties;
class CFWL_Edit;
class CFWL_Edit : public CFWL_Widget
{
public:
    static CFWL_Edit* Create();
    FWL_ERR     Initialize(const CFWL_WidgetProperties *pProperties = NULL);
    FWL_ERR		SetText(const CFX_WideString &wsText);
    FX_INT32	GetTextLength() const;
    FWL_ERR		GetText(CFX_WideString &wsText, FX_INT32 nStart = 0, FX_INT32 nCount = -1) const;
    FWL_ERR		ClearText();
    FX_INT32	GetCaretPos() const;
    FX_INT32	SetCaretPos(FX_INT32 nIndex, FX_BOOL bBefore = TRUE);
    FWL_ERR		AddSelRange(FX_INT32 nStart, FX_INT32 nCount = -1);
    FX_INT32	CountSelRanges();
    FX_INT32	GetSelRange(FX_INT32 nIndex, FX_INT32 &nStart);
    FWL_ERR		ClearSelections();
    FX_INT32	GetLimit();
    FWL_ERR		SetLimit(FX_INT32 nLimit);
    FWL_ERR		SetAliasChar(FX_WCHAR wAlias);
    FWL_ERR		SetFormatString(const CFX_WideString &wsFormat);
    FWL_ERR		Insert(FX_INT32 nStart, FX_LPCWSTR lpText, FX_INT32 nLen);
    FWL_ERR		DeleteSelections();
    FWL_ERR		DeleteRange(FX_INT32 nStart, FX_INT32 nCount = -1);
    FWL_ERR		ReplaceSelections(const CFX_WideStringC &wsReplace);
    FWL_ERR		Replace(FX_INT32 nStart, FX_INT32 nLen, const CFX_WideStringC &wsReplace);
    FWL_ERR		DoClipboard(FX_INT32 iCmd);
    FX_BOOL		Redo(FX_BSTR bsRecord);
    FX_BOOL		Undo(FX_BSTR bsRecord);
    FWL_ERR		SetTabWidth(FX_FLOAT fTabWidth, FX_BOOL bEquidistant);
    FWL_ERR		SetNumberRange(FX_INT32 iMin, FX_INT32 iMax);
    FWL_ERR     SetBackColor(FX_DWORD dwColor);
    FWL_ERR     SetFont(const CFX_WideString &wsFont, FX_FLOAT fSize);
    FX_BOOL		CanUndo();
    FX_BOOL		CanRedo();
    FX_BOOL		Undo();
    FX_BOOL		Redo();
    FX_BOOL		Copy(CFX_WideString &wsCopy);
    FX_BOOL		Cut(CFX_WideString &wsCut);
    FX_BOOL		Paste(const CFX_WideString &wsPaste);
    FX_BOOL		Delete();
    void		SetScrollOffset(FX_FLOAT fScrollOffset);
    FX_BOOL		GetSuggestWords(CFX_PointF pointf, CFX_ByteStringArray &sSuggest);
    FX_BOOL		ReplaceSpellCheckWord(CFX_PointF pointf, FX_BSTR bsReplace);
    CFWL_Edit();
    virtual ~CFWL_Edit();
};
#endif
