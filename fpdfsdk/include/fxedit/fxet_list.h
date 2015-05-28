// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FXEDIT_FXET_LIST_H_
#define FPDFSDK_INCLUDE_FXEDIT_FXET_LIST_H_

#include "../../../core/include/fpdfapi/fpdf_parser.h"  // For CPDF_Point.
#include "fx_edit.h"

class IFX_Edit;

class CLST_Size
{
public:
	CLST_Size() : x(0.0f), y(0.0f)
	{
	}

	CLST_Size(FX_FLOAT x,FX_FLOAT y)
	{
		this->x = x;
		this->y = y;
	}

	void Default()
	{
		x = 0.0f;
		y = 0.0f;
	}

	FX_BOOL operator != (const CLST_Size & size) const
	{
		return FXSYS_memcmp(this, &size, sizeof(CLST_Size)) != 0;
	}

	FX_FLOAT x,y;
};

class CLST_Rect : public CPDF_Rect
{
public:
	CLST_Rect()
	{
		left = top = right = bottom = 0.0f;
	}

	CLST_Rect(FX_FLOAT left,FX_FLOAT top,
						FX_FLOAT right,FX_FLOAT bottom)
	{
		this->left = left;
		this->top = top;
		this->right = right;
		this->bottom = bottom;
	}

	CLST_Rect(const CPDF_Rect & rect)
	{
		this->left = rect.left;
		this->top = rect.top;
		this->right = rect.right;
		this->bottom = rect.bottom;
	}

	void Default()
	{
		left = top = right = bottom = 0.0f;
	}

	const CLST_Rect operator = (const CPDF_Rect & rect)
	{
		this->left = rect.left;
		this->top = rect.top;
		this->right = rect.right;
		this->bottom = rect.bottom;

		return *this;
	}

	FX_BOOL operator == (const CLST_Rect & rect) const
	{
		return FXSYS_memcmp(this, &rect, sizeof(CLST_Rect)) == 0;
	}

	FX_BOOL operator != (const CLST_Rect & rect) const
	{
		return FXSYS_memcmp(this, &rect, sizeof(CLST_Rect)) != 0;
	}

	FX_FLOAT Width() const
	{
		return this->right - this->left;
	}

	FX_FLOAT Height() const
	{
		if (this->top > this->bottom)
			return this->top - this->bottom;
		else
			return this->bottom - this->top;
	}

	CPDF_Point LeftTop() const
	{
		return CPDF_Point(left,top);
	}

	CPDF_Point RightBottom() const
	{
		return CPDF_Point(right,bottom);
	}

	const CLST_Rect operator += (const CPDF_Point & point)
	{
		this->left += point.x;
		this->right += point.x;
		this->top += point.y;
		this->bottom += point.y;

		return *this;
	}

	const CLST_Rect operator -= (const CPDF_Point & point)
	{
		this->left -= point.x;
		this->right -= point.x;
		this->top -= point.y;
		this->bottom -= point.y;

		return *this;
	}

	CLST_Rect operator + (const CPDF_Point & point) const
	{
		return CLST_Rect(left + point.x,
					top + point.y,
					right + point.x,
					bottom + point.y);
	}

	CLST_Rect operator - (const CPDF_Point & point) const
	{
		return CLST_Rect(left - point.x,
					top - point.y,
					right - point.x,
					bottom - point.y);
	}
};

class CFX_ListItem
{
public:
	CFX_ListItem();
	virtual ~CFX_ListItem();

	void							SetFontMap(IFX_Edit_FontMap * pFontMap);
	IFX_Edit_Iterator*				GetIterator() const;
	IFX_Edit*						GetEdit() const;

public:
	void							SetRect(const CLST_Rect & rect);
	void							SetSelect(FX_BOOL bSelected);
	void							SetCaret(FX_BOOL bCaret);
	void							SetText(FX_LPCWSTR text);
	void							SetFontSize(FX_FLOAT fFontSize);
	CFX_WideString					GetText() const;

	CLST_Rect						GetRect() const;
	FX_BOOL							IsSelected() const;
	FX_BOOL							IsCaret() const;
	FX_FLOAT						GetItemHeight() const;
	FX_WORD							GetFirstChar() const;

private:
	IFX_Edit*						m_pEdit;
	FX_BOOL							m_bSelected;
	FX_BOOL							m_bCaret;
	CLST_Rect						m_rcListItem;
};

class CFX_ListContainer
{
public:
	CFX_ListContainer() : m_rcPlate(0.0f,0.0f,0.0f,0.0f), m_rcContent(0.0f,0.0f,0.0f,0.0f){}
	virtual ~CFX_ListContainer(){}
	virtual void					SetPlateRect(const CPDF_Rect & rect){m_rcPlate = rect;}
	CPDF_Rect						GetPlateRect() const{return m_rcPlate;}
	void							SetContentRect(const CLST_Rect & rect){m_rcContent = rect;}
	CLST_Rect						GetContentRect() const{return m_rcContent;}
	CPDF_Point						GetBTPoint() const{return CPDF_Point(m_rcPlate.left,m_rcPlate.top);}
	CPDF_Point						GetETPoint() const{return CPDF_Point(m_rcPlate.right,m_rcPlate.bottom);}
public:
	CPDF_Point						InnerToOuter(const CPDF_Point & point) const{return CPDF_Point(point.x + GetBTPoint().x,GetBTPoint().y - point.y);}
	CPDF_Point						OuterToInner(const CPDF_Point & point) const{return CPDF_Point(point.x - GetBTPoint().x,GetBTPoint().y - point.y);}
	CPDF_Rect						InnerToOuter(const CLST_Rect & rect) const{CPDF_Point ptLeftTop = InnerToOuter(CPDF_Point(rect.left,rect.top));
																			CPDF_Point ptRightBottom = InnerToOuter(CPDF_Point(rect.right,rect.bottom));
																			return CPDF_Rect(ptLeftTop.x,ptRightBottom.y,ptRightBottom.x,ptLeftTop.y);}
	CLST_Rect						OuterToInner(const CPDF_Rect & rect) const{CPDF_Point ptLeftTop = OuterToInner(CPDF_Point(rect.left,rect.top));
																			CPDF_Point ptRightBottom = OuterToInner(CPDF_Point(rect.right,rect.bottom));
																			return CLST_Rect(ptLeftTop.x,ptLeftTop.y,ptRightBottom.x,ptRightBottom.y);}
private:
	CPDF_Rect						m_rcPlate;
	CLST_Rect						m_rcContent;		//positive forever!
};

template<class TYPE> class CLST_ArrayTemplate : public CFX_ArrayTemplate<TYPE>
{
public:
	FX_BOOL IsEmpty() { return CFX_ArrayTemplate<TYPE>::GetSize() <= 0; }
	TYPE GetAt(FX_INT32 nIndex) const { if (nIndex >= 0 && nIndex < CFX_ArrayTemplate<TYPE>::GetSize()) return CFX_ArrayTemplate<TYPE>::GetAt(nIndex); return NULL;}
	void RemoveAt(FX_INT32 nIndex){if (nIndex >= 0 && nIndex < CFX_ArrayTemplate<TYPE>::GetSize()) CFX_ArrayTemplate<TYPE>::RemoveAt(nIndex);}
};

class CFX_List : protected CFX_ListContainer , public IFX_List
{
public:
	CFX_List();
	virtual ~CFX_List();

public:
	virtual void					SetFontMap(IFX_Edit_FontMap * pFontMap);
	virtual void					SetFontSize(FX_FLOAT fFontSize);

	virtual CPDF_Rect				GetPlateRect() const;
	virtual CPDF_Rect				GetContentRect() const;

	virtual FX_FLOAT				GetFontSize() const;
	virtual IFX_Edit*				GetItemEdit(FX_INT32 nIndex) const;
	virtual FX_INT32				GetCount() const;
	virtual FX_BOOL					IsItemSelected(FX_INT32 nIndex) const;
	virtual FX_FLOAT				GetFirstHeight() const;

	virtual void					SetMultipleSel(FX_BOOL bMultiple);
	virtual FX_BOOL					IsMultipleSel() const;
	virtual FX_BOOL					IsValid(FX_INT32 nItemIndex) const;
	virtual FX_INT32				FindNext(FX_INT32 nIndex,FX_WCHAR nChar) const;

protected:
	virtual void					Empty();

	void							AddItem(FX_LPCWSTR str);
	virtual void					ReArrange(FX_INT32 nItemIndex);

	virtual CPDF_Rect				GetItemRect(FX_INT32 nIndex) const;
	CFX_WideString					GetItemText(FX_INT32 nIndex) const;

	void							SetItemSelect(FX_INT32 nItemIndex, FX_BOOL bSelected);
	void							SetItemCaret(FX_INT32 nItemIndex, FX_BOOL bCaret);

	virtual FX_INT32				GetItemIndex(const CPDF_Point & point) const;
	FX_INT32						GetFirstSelected() const;
	FX_INT32						GetLastSelected() const;
	FX_WCHAR						Toupper(FX_WCHAR c) const;

private:
	CLST_ArrayTemplate<CFX_ListItem*>	m_aListItems;
	FX_FLOAT							m_fFontSize;
	IFX_Edit_FontMap*					m_pFontMap;
	FX_BOOL								m_bMultiple;
};

struct CPLST_Select_Item
{
	CPLST_Select_Item(FX_INT32 nItemIndex,FX_INT32 nState)
	{
		this->nItemIndex = nItemIndex;
		this->nState = nState;
	}

	FX_INT32		nItemIndex;
	FX_INT32		nState; //0:normal select -1:to deselect 1: to select
};

class CPLST_Select
{
public:
	CPLST_Select();
	virtual ~CPLST_Select();

public:
	void							Add(FX_INT32 nItemIndex);
	void							Add(FX_INT32 nBeginIndex, FX_INT32 nEndIndex);
	void							Sub(FX_INT32 nItemIndex);
	void							Sub(FX_INT32 nBeginIndex, FX_INT32 nEndIndex);
	FX_BOOL							IsExist(FX_INT32 nItemIndex) const;
	FX_INT32						Find(FX_INT32 nItemIndex) const;
	FX_INT32						GetCount() const;
	FX_INT32						GetItemIndex(FX_INT32 nIndex) const;
	FX_INT32						GetState(FX_INT32 nIndex) const;
	void							Done();
	void							DeselectAll();

private:
	CFX_ArrayTemplate<CPLST_Select_Item*>	m_aItems;
};

class CFX_ListCtrl : public CFX_List
{
public:
	CFX_ListCtrl();
	virtual ~CFX_ListCtrl();

public:
	void							SetNotify(IFX_List_Notify * pNotify);

	void							OnMouseDown(const CPDF_Point & point,FX_BOOL bShift,FX_BOOL bCtrl);
	void							OnMouseMove(const CPDF_Point & point,FX_BOOL bShift,FX_BOOL bCtrl);
	void							OnVK_UP(FX_BOOL bShift,FX_BOOL bCtrl);
	void							OnVK_DOWN(FX_BOOL bShift,FX_BOOL bCtrl);
	void							OnVK_LEFT(FX_BOOL bShift,FX_BOOL bCtrl);
	void							OnVK_RIGHT(FX_BOOL bShift,FX_BOOL bCtrl);
	void							OnVK_HOME(FX_BOOL bShift,FX_BOOL bCtrl);
	void							OnVK_END(FX_BOOL bShift,FX_BOOL bCtrl);
	void							OnVK(FX_INT32 nItemIndex,FX_BOOL bShift,FX_BOOL bCtrl);
	FX_BOOL							OnChar(FX_WORD nChar,FX_BOOL bShift,FX_BOOL bCtrl);

	virtual CPDF_Point				InToOut(const CPDF_Point & point) const;
	virtual CPDF_Point				OutToIn(const CPDF_Point & point) const;
	virtual CPDF_Rect				InToOut(const CPDF_Rect & rect) const;
	virtual CPDF_Rect				OutToIn(const CPDF_Rect & rect) const;

	virtual void					SetPlateRect(const CPDF_Rect & rect);
	void							SetScrollPos(const CPDF_Point & point);
	void							ScrollToListItem(FX_INT32 nItemIndex);
	virtual CPDF_Rect				GetItemRect(FX_INT32 nIndex) const;
	FX_INT32						GetCaret() const {return m_nCaretIndex;}
	FX_INT32						GetSelect() const {return m_nSelItem;}
	FX_INT32						GetTopItem() const;
	virtual CPDF_Rect				GetContentRect() const;
	virtual FX_INT32				GetItemIndex(const CPDF_Point & point) const;

	void							AddString(FX_LPCWSTR string);
	void							SetTopItem(FX_INT32 nIndex);
	void							Select(FX_INT32 nItemIndex);
	virtual void					SetCaret(FX_INT32 nItemIndex);
	virtual void					Empty();
	virtual void					Cancel();
	CFX_WideString					GetText() const;

private:
	void							SetMultipleSelect(FX_INT32 nItemIndex, FX_BOOL bSelected);
	void							SetSingleSelect(FX_INT32 nItemIndex);
	void							InvalidateItem(FX_INT32 nItemIndex);
	void							SelectItems();
	FX_BOOL							IsItemVisible(FX_INT32 nItemIndex) const;
	void							SetScrollInfo();
	void							SetScrollPosY(FX_FLOAT fy);
	virtual void					ReArrange(FX_INT32 nItemIndex);

private:
	IFX_List_Notify*				m_pNotify;
	FX_BOOL							m_bNotifyFlag;
	CPDF_Point						m_ptScrollPos;
	CPLST_Select					m_aSelItems;	//for multiple
	FX_INT32						m_nSelItem;		//for single
	FX_INT32						m_nFootIndex;	//for multiple
	FX_BOOL							m_bCtrlSel;		//for multiple
	FX_INT32						m_nCaretIndex;	//for multiple
};

#endif  // FPDFSDK_INCLUDE_FXEDIT_FXET_LIST_H_
