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
										int32_t & nRet, FX_FLOAT & fPopupRet) = 0; //nRet: (0:bottom 1:top)
	virtual void					OnBeforeKeyStroke(bool bEditOrList, void* pPrivateData, int32_t nKeyCode,
										CFX_WideString & strChange, const CFX_WideString& strChangeEx,
										int nSelStart, int nSelEnd,
										bool bKeyDown, bool & bRC, bool & bExit, FX_DWORD nFlag) = 0;
	virtual void					OnAfterKeyStroke(bool bEditOrList, void* pPrivateData, bool & bExit, FX_DWORD nFlag) = 0;
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

	virtual bool					OnLButtonDown(const CPDF_Point & point, FX_DWORD nFlag);
	virtual bool					OnLButtonDblClk(const CPDF_Point & point, FX_DWORD nFlag);
	virtual bool					OnRButtonUp(const CPDF_Point & point, FX_DWORD nFlag);
	virtual bool					OnMouseWheel(short zDelta, const CPDF_Point & point, FX_DWORD nFlag);

	virtual bool					OnKeyDown(FX_WORD nChar, FX_DWORD nFlag);
	virtual bool					OnChar(FX_WORD nChar, FX_DWORD nFlag);

	virtual CPDF_Rect				GetFocusRect() const;

public:
	void							SetAlignFormatH(PWL_EDIT_ALIGNFORMAT_H nFormat = PEAH_LEFT, bool bPaint = true);	//0:left 1:right 2:middle
	void							SetAlignFormatV(PWL_EDIT_ALIGNFORMAT_V nFormat = PEAV_TOP, bool bPaint = true);	//0:top 1:bottom 2:center

	void							SetCharArray(int32_t nCharArray);
	void							SetLimitChar(int32_t nLimitChar);

	void							SetHorzScale(int32_t nHorzScale, bool bPaint = true);
	void							SetCharSpace(FX_FLOAT fCharSpace, bool bPaint = true);

	void							SetLineLeading(FX_FLOAT fLineLeading, bool bPaint = true);

	void							EnableSpellCheck(bool bEnabled);

	bool							CanSelectAll() const;
	bool							CanClear() const;
	bool							CanCopy() const;
	bool							CanCut() const;
	bool							CanPaste() const;

	virtual void					CopyText();
	virtual void					PasteText();
	virtual void 					CutText();

	virtual void					SetText(const FX_WCHAR* csText);
	void							ReplaceSel(const FX_WCHAR* csText);

	CFX_ByteString					GetTextAppearanceStream(const CPDF_Point & ptOffset) const;
	CFX_ByteString					GetCaretAppearanceStream(const CPDF_Point & ptOffset) const;
	CFX_ByteString					GetSelectAppearanceStream(const CPDF_Point & ptOffset) const;

	bool							IsTextFull() const;

	static FX_FLOAT					GetCharArrayAutoFontSize(CPDF_Font* pFont, const CPDF_Rect& rcPlate, int32_t nCharArray);

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
	virtual void					ShowVScrollBar(bool bShow);
	bool							IsVScrollBarVisible() const;
	void							SetParamByFlag();

	FX_FLOAT						GetCharArrayAutoFontSize(int32_t nCharArray);
	CPDF_Point						GetWordRightBottomPoint(const CPVT_WordPlace& wpWord);

	CPVT_WordRange					CombineWordRange(const CPVT_WordRange& wr1, const CPVT_WordRange& wr2);
	CPVT_WordRange					GetLatinWordsRange(const CPDF_Point & point) const;
	CPVT_WordRange					GetLatinWordsRange(const CPVT_WordPlace & place) const;
	CPVT_WordRange					GetArabicWordsRange(const CPVT_WordPlace & place) const;
	CPVT_WordRange					GetSameWordsRange(const CPVT_WordPlace & place, bool bLatin, bool bArabic) const;

	void							AjustArabicWords(const CPVT_WordRange& wr);
public:
	bool							IsProceedtoOnChar(FX_WORD nKeyCode, FX_DWORD nFlag);
private:
	IPWL_Filler_Notify*				m_pFillerNotify;
	IPWL_SpellCheck*				m_pSpellCheck;
	bool							m_bFocus;
	CPDF_Rect						m_rcOldWindow;
public:
	void							AttachFFLData(void* pData) {m_pFormFiller = pData;}
private:
	void*							m_pFormFiller;
};

#endif  // FPDFSDK_INCLUDE_PDFWINDOW_PWL_EDIT_H_
