// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_PDFWINDOW_PWL_EDITCTRL_H_
#define FPDFSDK_INCLUDE_PDFWINDOW_PWL_EDITCTRL_H_

#include "../../../core/include/fxcrt/fx_string.h"
#include "../fxedit/fx_edit.h"
#include "PWL_Wnd.h"

class CPWL_Caret;
class CPWL_Edit;
class CPWL_EditCtrl;
class IFX_Edit;
class IPWL_Edit_Notify;
struct CPVT_WordPlace;

enum PWL_EDIT_ALIGNFORMAT_H
{
	PEAH_LEFT = 0,
	PEAH_MIDDLE,
	PEAH_RIGHT
};

enum PWL_EDIT_ALIGNFORMAT_V
{
	PEAV_TOP = 0,
	PEAV_CENTER,
	PEAV_BOTTOM
};

class IPWL_Edit_Notify
{
public:
        virtual ~IPWL_Edit_Notify() { }
	//when the position of caret is changed in edit
	virtual void					OnCaretMove(FX_INT32 x1, FX_INT32 y1, FX_INT32 x2, FX_INT32 y2) {}
	virtual void					OnContentChange(const CPDF_Rect& rcContent){}
	//OprType: 0 InsertWord
	//1 InsertReturn
	//2 BackSpace
	//3 Delete
	//4 Clear
	//5 InsertText
	//6 SetText
	virtual void					OnInsertWord(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace){}
	virtual void					OnInsertReturn(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace){}
	virtual void					OnBackSpace(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace){}
	virtual void					OnDelete(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace){}
	virtual void					OnClear(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace){}
	virtual void					OnInsertText(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace){}
	virtual void					OnSetText(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace){}
	virtual void					OnAddUndo(CPWL_Edit* pEdit) {}
};

class PWL_CLASS CPWL_EditCtrl : public CPWL_Wnd, public IFX_Edit_Notify
{
	friend class CPWL_Edit_Notify;

public:
	CPWL_EditCtrl();
	virtual ~CPWL_EditCtrl();

public:
	virtual void					OnCreate(PWL_CREATEPARAM & cp);
	virtual void					OnCreated();

	virtual FX_BOOL					OnKeyDown(FX_WORD nChar, FX_DWORD nFlag);
	virtual FX_BOOL					OnChar(FX_WORD nChar, FX_DWORD nFlag);
	virtual FX_BOOL					OnLButtonDown(const CPDF_Point & point, FX_DWORD nFlag);	
	virtual FX_BOOL					OnLButtonUp(const CPDF_Point & point, FX_DWORD nFlag);	
	virtual FX_BOOL					OnMouseMove(const CPDF_Point & point, FX_DWORD nFlag);
	virtual void					OnNotify(CPWL_Wnd* pWnd, FX_DWORD msg, FX_INTPTR wParam = 0, FX_INTPTR lParam = 0);

	virtual void					CreateChildWnd(const PWL_CREATEPARAM & cp);
	virtual void					RePosChildWnd();	
	virtual void					SetFontSize(FX_FLOAT fFontSize);
	virtual FX_FLOAT				GetFontSize() const;
	
public:
	virtual void					SetText(FX_LPCWSTR csText);

	virtual void					CopyText();
	virtual void					PasteText();
	virtual void 					CutText();

	CPDF_Rect						GetContentRect() const;
	void							GetCaretPos(FX_INT32& x, FX_INT32& y) const;
	FX_BOOL							IsModified() const;

	CFX_WideString					GetText() const;
	void							SetSel(FX_INT32 nStartChar,FX_INT32 nEndChar);
	void							GetSel(FX_INT32 & nStartChar, FX_INT32 & nEndChar ) const;
	void							GetTextRange(const CPDF_Rect& rect, FX_INT32 & nStartChar, FX_INT32 & nEndChar) const;
	CFX_WideString					GetText(FX_INT32 & nStartChar, FX_INT32 & nEndChar) const;
	void							Clear();
	void							SelectAll();

	FX_INT32						GetCaret() const;
	void							SetCaret(FX_INT32 nPos);
	FX_INT32						GetTotalWords() const;

	void							Paint();

	void							EnableRefresh(FX_BOOL bRefresh);	
	CPDF_Point						GetScrollPos() const;
	void							SetScrollPos(const CPDF_Point& point);

	void							SetEditNotify(IPWL_Edit_Notify* pNotify) {m_pEditNotify = pNotify;}	

	void							SetCharSet(FX_BYTE nCharSet){m_nCharSet = nCharSet;}
	FX_INT32						GetCharSet() const;

	void							SetCodePage(FX_INT32 nCodePage){m_nCodePage = nCodePage;}
	FX_INT32						GetCodePage() const {return m_nCodePage;}

	CPDF_Font *						GetCaretFont() const;
	FX_FLOAT						GetCaretFontSize() const;

	FX_BOOL							CanUndo() const;
	FX_BOOL							CanRedo() const;
	void							Redo();
	void							Undo();

	void							SetReadyToInput();
protected:
	virtual void					ShowVScrollBar(FX_BOOL bShow);

	virtual void					InsertWord(FX_WORD word, FX_INT32 nCharset);
	virtual void					InsertReturn();
	virtual void					InsertText(FX_LPCWSTR csText);

	virtual void					SetCursor();
	FX_BOOL							IsWndHorV();

	void							Delete();
	void							Backspace();

protected:
	void							GetCaretInfo(CPDF_Point & ptHead, CPDF_Point & ptFoot) const;
	void							SetCaret(FX_BOOL bVisible, const CPDF_Point & ptHead, const CPDF_Point & ptFoot);
	
	void							SetEditCaret(FX_BOOL bVisible);

protected:
	virtual void					IOnSetScrollInfoX(FX_FLOAT fPlateMin, FX_FLOAT fPlateMax, 
												FX_FLOAT fContentMin, FX_FLOAT fContentMax, 
												FX_FLOAT fSmallStep, FX_FLOAT fBigStep){}
	virtual void					IOnSetScrollInfoY(FX_FLOAT fPlateMin, FX_FLOAT fPlateMax, 
												FX_FLOAT fContentMin, FX_FLOAT fContentMax, 
												FX_FLOAT fSmallStep, FX_FLOAT fBigStep);
	virtual void					IOnSetScrollPosX(FX_FLOAT fx){}
	virtual void					IOnSetScrollPosY(FX_FLOAT fy);
	virtual void					IOnSetCaret(FX_BOOL bVisible,const CPDF_Point & ptHead,const CPDF_Point & ptFoot, const CPVT_WordPlace& place);
	virtual void					IOnCaretChange(const CPVT_SecProps & secProps, const CPVT_WordProps & wordProps);
	virtual void					IOnContentChange(const CPDF_Rect& rcContent);
	virtual void					IOnInvalidateRect(CPDF_Rect * pRect);

private:
	void							CreateEditCaret(const PWL_CREATEPARAM & cp);

protected:
	IFX_Edit*						m_pEdit;
	CPWL_Caret*						m_pEditCaret;
	FX_BOOL							m_bMouseDown;
	IPWL_Edit_Notify*				m_pEditNotify;	

private:
	FX_INT32						m_nCharSet;
	FX_INT32						m_nCodePage;
};

#endif  // FPDFSDK_INCLUDE_PDFWINDOW_PWL_EDITCTRL_H_
