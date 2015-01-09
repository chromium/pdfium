// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _PWL_IconList_H_
#define _PWL_IconList_H_

class IPWL_IconList_Notify;
class CPWL_IconList_Item;
class CPWL_IconList_Content;
class CPWL_IconList;
class CPWL_Label;

class IPWL_IconList_Notify
{
public:
        virtual ~IPWL_IconList_Notify() { }
	virtual void						OnNoteListSelChanged(FX_INT32 nItemIndex) = 0;
};

class CPWL_IconList_Item : public CPWL_Wnd
{
public:
	CPWL_IconList_Item();
	virtual ~CPWL_IconList_Item();

	virtual CFX_ByteString				GetClassName() const;
	virtual void						CreateChildWnd(const PWL_CREATEPARAM & cp);
	virtual void						RePosChildWnd();

	void								SetSelect(FX_BOOL bSelected);
	FX_BOOL								IsSelected() const;
	void								SetData(void* pData);
	void								SetIcon(FX_INT32 nIconIndex);
	void								SetText(const CFX_WideString& str);
	void								SetIconFillColor(const CPWL_Color& color);
	CFX_WideString						GetText() const;

protected:
	virtual FX_FLOAT					GetItemHeight(FX_FLOAT fLimitWidth);
	virtual void						DrawThisAppearance(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device);

	virtual void						OnEnabled();
	virtual void						OnDisabled();

private:
	FX_INT32							m_nIconIndex;
	void*								m_pData;
	FX_BOOL								m_bSelected;
	CPWL_Label*							m_pText;
	CPWL_Color							m_crIcon;
};

class CPWL_IconList_Content : public CPWL_ListCtrl
{
public:
	CPWL_IconList_Content(FX_INT32 nListCount);
	virtual ~CPWL_IconList_Content();

	void								SetSelect(FX_INT32 nIndex);
	FX_INT32							GetSelect() const;
	void								SetNotify(IPWL_IconList_Notify* pNotify);
	void								EnableNotify(FX_BOOL bNotify);
	void								SetListData(FX_INT32 nItemIndex, void* pData);
	void								SetListIcon(FX_INT32 nItemIndex, FX_INT32 nIconIndex);
	void								SetListString(FX_INT32 nItemIndex, const CFX_WideString& str);
	void								SetIconFillColor(const CPWL_Color& color);
	CFX_WideString						GetListString(FX_INT32 nItemIndex) const;
	IPWL_IconList_Notify*				GetNotify() const;
	void								ScrollToItem(FX_INT32 nItemIndex);

protected:
	virtual void						CreateChildWnd(const PWL_CREATEPARAM & cp);
	virtual FX_BOOL						OnLButtonDown(const CPDF_Point & point, FX_DWORD nFlag);
	virtual FX_BOOL						OnLButtonUp(const CPDF_Point & point, FX_DWORD nFlag);
	virtual FX_BOOL						OnMouseMove(const CPDF_Point & point, FX_DWORD nFlag);
	virtual FX_BOOL						OnKeyDown(FX_WORD nChar, FX_DWORD nFlag);

private:
	CPWL_IconList_Item*					GetListItem(FX_INT32 nItemIndex) const;
	void								SelectItem(FX_INT32 nItemIndex, FX_BOOL bSelect);
	FX_INT32							FindItemIndex(const CPDF_Point& point);
	
	FX_BOOL								m_nSelectIndex;
	IPWL_IconList_Notify*				m_pNotify;
	FX_BOOL								m_bEnableNotify;
	FX_BOOL								m_bMouseDown;
	FX_INT32							m_nListCount;
};

class PWL_CLASS CPWL_IconList : public CPWL_Wnd
{
public:
	CPWL_IconList(FX_INT32 nListCount);
	virtual ~CPWL_IconList();

	virtual FX_BOOL						OnMouseWheel(short zDelta, const CPDF_Point & point, FX_DWORD nFlag);

	void								SetSelect(FX_INT32 nIndex);
	void								SetTopItem(FX_INT32 nIndex);
	FX_INT32							GetSelect() const;
	void								SetNotify(IPWL_IconList_Notify* pNotify);
	void								EnableNotify(FX_BOOL bNotify);
	void								SetListData(FX_INT32 nItemIndex, void* pData);
	void								SetListIcon(FX_INT32 nItemIndex, FX_INT32 nIconIndex);
	void								SetListString(FX_INT32 nItemIndex, const CFX_WideString& str);
	void								SetIconFillColor(const CPWL_Color& color);
	CFX_WideString						GetListString(FX_INT32 nItemIndex) const;

protected:
	virtual void						OnCreated();
	virtual void						RePosChildWnd();
	virtual void						CreateChildWnd(const PWL_CREATEPARAM & cp);
	
	virtual void						OnNotify(CPWL_Wnd* pWnd, FX_DWORD msg, FX_INTPTR wParam = 0, FX_INTPTR lParam = 0);

private:
	CPWL_IconList_Content*				m_pListContent;
	FX_INT32							m_nListCount;
};
 
#endif //_PWL_IconList_H_


