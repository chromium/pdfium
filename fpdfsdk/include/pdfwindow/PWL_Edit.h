// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_PDFWINDOW_PWL_EDIT_H_
#define FPDFSDK_INCLUDE_PDFWINDOW_PWL_EDIT_H_

#include "../../../core/include/fxcrt/fx_basic.h"
#include "PWL_EditCtrl.h"
#include "PWL_Wnd.h"

class CPWL_Edit;
class IPWL_Filler_Notify;
class IPWL_SpellCheck;

class IPWL_Filler_Notify
{
public:
        virtual ~IPWL_Filler_Notify() { }
	virtual void					QueryWherePopup(void* pPrivateData, FX_FLOAT fPopupMin,FX_FLOAT fPopupMax, 
										FX_INT32 & nRet, FX_FLOAT & fPopupRet) = 0; //nRet: (0:bottom 1:top)
	virtual void					OnBeforeKeyStroke(FX_BOOL bEditOrList, void* pPrivateData, FX_INT32 nKeyCode,
										CFX_WideString & strChange, const CFX_WideString& strChangeEx, 
										int nSelStart, int nSelEnd,
										FX_BOOL bKeyDown, FX_BOOL & bRC, FX_BOOL & bExit, FX_DWORD nFlag) = 0;
	virtual void					OnAfterKeyStroke(FX_BOOL bEditOrList, void* pPrivateData, FX_BOOL & bExit, FX_DWORD nFlag) = 0;
};

class PWL_CLASS CPWL_Edit : public CPWL_EditCtrl, public IFX_Edit_OprNotify
{
public:
	CPWL_Edit();
	virtual ~CPWL_Edit();

public:
	virtual CFX_ByteString			GetClassName() const;
	virtual void					OnDestroy();
	virtual void					OnCreated();
	virtual void					RePosChildWnd();
	virtual CPDF_Rect				GetClientRect() const;

	virtual void					GetThisAppearanceStream(CFX_ByteTextBuf & sAppStream);
	virtual void					DrawThisAppearance(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device);

	virtual FX_BOOL					OnLButtonDown(const CPDF_Point & point, FX_DWORD nFlag);
	virtual FX_BOOL					OnLButtonDblClk(const CPDF_Point & point, FX_DWORD nFlag);
	virtual FX_BOOL					OnRButtonUp(const CPDF_Point & point, FX_DWORD nFlag);
	virtual FX_BOOL					OnMouseWheel(short zDelta, const CPDF_Point & point, FX_DWORD nFlag);

	virtual FX_BOOL					OnKeyDown(FX_WORD nChar, FX_DWORD nFlag);
	virtual FX_BOOL					OnChar(FX_WORD nChar, FX_DWORD nFlag);

	virtual CPDF_Rect				GetFocusRect() const;

public:		
	void							SetAlignFormatH(PWL_EDIT_ALIGNFORMAT_H nFormat = PEAH_LEFT, FX_BOOL bPaint = TRUE);	//0:left 1:right 2:middle 
	void							SetAlignFormatV(PWL_EDIT_ALIGNFORMAT_V nFormat = PEAV_TOP, FX_BOOL bPaint = TRUE);	//0:top 1:bottom 2:center

	void							SetCharArray(FX_INT32 nCharArray);
	void							SetLimitChar(FX_INT32 nLimitChar);

	void							SetHorzScale(FX_INT32 nHorzScale, FX_BOOL bPaint = TRUE);
	void							SetCharSpace(FX_FLOAT fCharSpace, FX_BOOL bPaint = TRUE);

	void							SetLineLeading(FX_FLOAT fLineLeading, FX_BOOL bPaint = TRUE);

	void							EnableSpellCheck(FX_BOOL bEnabled);

	FX_BOOL							CanSelectAll() const;
	FX_BOOL							CanClear() const;
	FX_BOOL							CanCopy() const;
	FX_BOOL							CanCut() const;
	FX_BOOL							CanPaste() const;

	virtual void					CopyText();
	virtual void					PasteText();
	virtual void 					CutText();

	virtual void					SetText(FX_LPCWSTR csText);
	void							ReplaceSel(FX_LPCWSTR csText);

	CFX_ByteString					GetTextAppearanceStream(const CPDF_Point & ptOffset) const;
	CFX_ByteString					GetCaretAppearanceStream(const CPDF_Point & ptOffset) const;	
	CFX_ByteString					GetSelectAppearanceStream(const CPDF_Point & ptOffset) const;

	FX_BOOL							IsTextFull() const;	

	static FX_FLOAT					GetCharArrayAutoFontSize(CPDF_Font* pFont, const CPDF_Rect& rcPlate, FX_INT32 nCharArray);

	void							SetFillerNotify(IPWL_Filler_Notify* pNotify) {m_pFillerNotify = pNotify;}

	void							GeneratePageObjects(CPDF_PageObjects* pPageObjects, 
										const CPDF_Point& ptOffset, CFX_ArrayTemplate<CPDF_TextObject*>& ObjArray);
	void							GeneratePageObjects(CPDF_PageObjects* pPageObjects, 
										const CPDF_Point& ptOffset);

protected:
	virtual void					OnSetFocus();
	virtual void					OnKillFocus();

protected:
	virtual void					OnInsertWord(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace);
	virtual void					OnInsertReturn(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace);
	virtual void					OnBackSpace(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace);
	virtual void					OnDelete(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace);
	virtual void					OnClear(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace);
	virtual void					OnSetText(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace);
	virtual void					OnInsertText(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace);
	virtual void					OnAddUndo(IFX_Edit_UndoItem* pUndoItem);

private:	
	CPVT_WordRange					GetSelectWordRange() const;
	virtual void					ShowVScrollBar(FX_BOOL bShow);
	FX_BOOL							IsVScrollBarVisible() const;
	void							SetParamByFlag();

	FX_FLOAT						GetCharArrayAutoFontSize(FX_INT32 nCharArray);
	CPDF_Point						GetWordRightBottomPoint(const CPVT_WordPlace& wpWord);

	CPVT_WordRange					CombineWordRange(const CPVT_WordRange& wr1, const CPVT_WordRange& wr2);
	CPVT_WordRange					GetLatinWordsRange(const CPDF_Point & point) const;
	CPVT_WordRange					GetLatinWordsRange(const CPVT_WordPlace & place) const;
	CPVT_WordRange					GetArabicWordsRange(const CPVT_WordPlace & place) const;
	CPVT_WordRange					GetSameWordsRange(const CPVT_WordPlace & place, FX_BOOL bLatin, FX_BOOL bArabic) const;

	void							AjustArabicWords(const CPVT_WordRange& wr);
public:
	FX_BOOL							IsProceedtoOnChar(FX_WORD nKeyCode, FX_DWORD nFlag);
private:
	IPWL_Filler_Notify*				m_pFillerNotify;
	IPWL_SpellCheck*				m_pSpellCheck;
	FX_BOOL							m_bFocus;
	CPDF_Rect						m_rcOldWindow;
public:
	void							AttachFFLData(void* pData) {m_pFormFiller = pData;}
private:
	void*							m_pFormFiller;
};

#endif  // FPDFSDK_INCLUDE_PDFWINDOW_PWL_EDIT_H_
