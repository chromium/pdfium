// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/pdfwindow/PDFWindow.h"
#include "../../include/pdfwindow/PWL_Wnd.h"
#include "../../include/pdfwindow/PWL_Button.h"
#include "../../include/pdfwindow/PWL_EditCtrl.h"
#include "../../include/pdfwindow/PWL_Edit.h"
#include "../../include/pdfwindow/PWL_ListCtrl.h"
#include "../../include/pdfwindow/PWL_ScrollBar.h"
#include "../../include/pdfwindow/PWL_Note.h"
#include "../../include/pdfwindow/PWL_Label.h"
#include "../../include/pdfwindow/PWL_Edit.h"
#include "../../include/pdfwindow/PWL_ScrollBar.h"
#include "../../include/pdfwindow/PWL_Utils.h"
#include "../../include/pdfwindow/PWL_Caret.h"

#define POPUP_ITEM_HEAD_BOTTOM					3.0f
#define POPUP_ITEM_BOTTOMWIDTH					1.0f
#define POPUP_ITEM_SIDEMARGIN					3.0f
#define POPUP_ITEM_SPACE						4.0f
#define POPUP_ITEM_TEXT_INDENT					2.0f
#define POPUP_ITEM_BORDERCOLOR					CPWL_Color(COLORTYPE_RGB, 80/255.0f, 80/255.0f, 80/255.0f)

#define IsFloatZero(f)						((f) < 0.0001 && (f) > -0.0001)
#define IsFloatBigger(fa,fb)				((fa) > (fb) && !IsFloatZero((fa) - (fb)))
#define IsFloatSmaller(fa,fb)				((fa) < (fb) && !IsFloatZero((fa) - (fb)))
#define IsFloatEqual(fa,fb)					IsFloatZero((fa)-(fb))


/* ------------------------------- CPWL_Note_Options ------------------------------- */

CPWL_Note_Options::CPWL_Note_Options() : m_pText(NULL)
{
}

CPWL_Note_Options::~CPWL_Note_Options()
{
}

void CPWL_Note_Options::SetTextColor(const CPWL_Color & color)
{
	CPWL_Wnd::SetTextColor(color);

	if (m_pText)
		m_pText->SetTextColor(color);
}

void CPWL_Note_Options::RePosChildWnd()
{
	if (this->IsValid())
	{
		ASSERT(m_pText != NULL);

		CPDF_Rect rcClient = GetClientRect();

		if (rcClient.Width() > 15.0f)
		{
			rcClient.right -= 15.0f;
			m_pText->Move(rcClient, TRUE, FALSE);
			m_pText->SetVisible(TRUE);
		}
		else
		{
			m_pText->Move(CPDF_Rect(0,0,0,0), TRUE, FALSE);
			m_pText->SetVisible(FALSE);
		}
	}
}

void CPWL_Note_Options::CreateChildWnd(const PWL_CREATEPARAM & cp)
{
	m_pText = new CPWL_Label;
	PWL_CREATEPARAM tcp = cp;
	tcp.pParentWnd = this;
	tcp.dwFlags = PWS_CHILD | PWS_VISIBLE;
	m_pText->Create(tcp);
}

void CPWL_Note_Options::SetText(const CFX_WideString& sText)
{
	m_pText->SetText(sText);
}

void CPWL_Note_Options::DrawThisAppearance(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device)
{
	CPWL_Wnd::DrawThisAppearance(pDevice, pUser2Device);

	CPDF_Rect rcClient = GetClientRect();
	rcClient.left = rcClient.right - 15.0f;

	CPDF_Point ptCenter = CPDF_Point((rcClient.left + rcClient.right) * 0.5f, (rcClient.top + rcClient.bottom) * 0.5f);

	CPDF_Point pt1(ptCenter.x - 2.0f, ptCenter.y + 2.0f * 0.5f);
	CPDF_Point pt2(ptCenter.x + 2.0f, ptCenter.y + 2.0f * 0.5f);
	CPDF_Point pt3(ptCenter.x, ptCenter.y - 3.0f * 0.5f);

	CFX_PathData path;

	path.SetPointCount(4);
	path.SetPoint(0, pt1.x, pt1.y, FXPT_MOVETO);
	path.SetPoint(1, pt2.x, pt2.y, FXPT_LINETO);
	path.SetPoint(2, pt3.x, pt3.y, FXPT_LINETO);
	path.SetPoint(3, pt1.x, pt1.y, FXPT_LINETO);

	pDevice->DrawPath(&path, pUser2Device, NULL, 
		CPWL_Utils::PWLColorToFXColor(GetTextColor(),GetTransparency()), 
		0, FXFILL_ALTERNATE);
}

CPDF_Rect CPWL_Note_Options::GetContentRect() const
{
	ASSERT(m_pText != NULL);

	CPDF_Rect rcText = m_pText->GetContentRect();
	rcText.right += 15.0f;
	return rcText;
}

/* ------------------------------- CPWL_Note_Edit ------------------------------ */

CPWL_Note_Edit::CPWL_Note_Edit() : m_bEnableNotify(TRUE),
	m_fOldItemHeight(0.0f),
	m_bSizeChanged(FALSE),
	m_fOldMin(0.0f),
	m_fOldMax(0.0f)
{
}

CPWL_Note_Edit::~CPWL_Note_Edit()
{
}

void CPWL_Note_Edit::RePosChildWnd()
{
	m_bEnableNotify = FALSE;	
	CPWL_Edit::RePosChildWnd();
	m_bEnableNotify = TRUE;

	m_fOldItemHeight = this->GetContentRect().Height();
}

void CPWL_Note_Edit::SetText(FX_LPCWSTR csText)
{
	m_bEnableNotify = FALSE;
	CPWL_Edit::SetText(csText);
	m_bEnableNotify = TRUE;	
	m_fOldItemHeight = this->GetContentRect().Height();
}

void CPWL_Note_Edit::OnSetFocus()
{
	m_bEnableNotify = FALSE;	
	CPWL_Edit::OnSetFocus();
	m_bEnableNotify = TRUE;	

	this->EnableSpellCheck(TRUE);
}

void CPWL_Note_Edit::OnKillFocus()
{
	this->EnableSpellCheck(FALSE);

	if (CPWL_Wnd* pParent = this->GetParentWindow())
	{
		if (CPWL_Wnd* pGrand = pParent->GetParentWindow())
		{
			ASSERT(pGrand->GetClassName() == "CPWL_NoteItem");

			CPWL_NoteItem* pNoteItem = (CPWL_NoteItem*)pGrand;

			pNoteItem->OnContentsValidate();
		}
	}

	CPWL_Edit::OnKillFocus();
}

void CPWL_Note_Edit::OnNotify(CPWL_Wnd* pWnd, FX_DWORD msg, FX_INTPTR wParam, FX_INTPTR lParam)
{
	if (m_bEnableNotify)
	{
		if (wParam == SBT_VSCROLL)
		{
			switch (msg)
			{
			case PNM_SETSCROLLINFO:	
				if (PWL_SCROLL_INFO* pInfo = (PWL_SCROLL_INFO*)lParam)
				{	
					if (!IsFloatEqual(pInfo->fContentMax, m_fOldMax) || 
						!IsFloatEqual(pInfo->fContentMin, m_fOldMin))
					{
						m_bSizeChanged = TRUE;				
						if (CPWL_Wnd * pParent = this->GetParentWindow())
						{
							pParent->OnNotify(this, PNM_NOTEEDITCHANGED, 0, 0);
						}

						m_fOldMax = pInfo->fContentMax;
						m_fOldMin = pInfo->fContentMin;
						return;
					}
				}
			}
		}		
	}

	CPWL_Edit::OnNotify(pWnd, msg, wParam, lParam);

	if (m_bEnableNotify)
	{
		switch (msg)
		{
		case PNM_SETCARETINFO:
			if (PWL_CARET_INFO * pInfo = (PWL_CARET_INFO*)wParam)
			{
				PWL_CARET_INFO newInfo = *pInfo;
				newInfo.bVisible = TRUE;
				newInfo.ptHead = this->ChildToParent(pInfo->ptHead);
				newInfo.ptFoot = this->ChildToParent(pInfo->ptFoot);

				if (CPWL_Wnd * pParent = this->GetParentWindow())
				{
					pParent->OnNotify(this, PNM_SETCARETINFO, (FX_INTPTR)&newInfo, 0);
				}
			}
			break;
		}
	}
}

FX_FLOAT CPWL_Note_Edit::GetItemHeight(FX_FLOAT fLimitWidth)
{
	if (fLimitWidth > 0)
	{
		if (!m_bSizeChanged)
			return m_fOldItemHeight;

		m_bSizeChanged = FALSE;	

		this->EnableNotify(FALSE);
		this->EnableRefresh(FALSE);
		m_pEdit->EnableNotify(FALSE);

		//CPDF_Rect rcOld = this->GetWindowRect();

		this->Move(CPDF_Rect(0,0,fLimitWidth,0), TRUE, FALSE);
		FX_FLOAT fRet = this->GetContentRect().Height();

		//this->Move(rcOld, TRUE, FALSE);

		m_pEdit->EnableNotify(TRUE);
		this->EnableNotify(TRUE);
		this->EnableRefresh(TRUE);

		return fRet;
	}

	return 0;
}

FX_FLOAT CPWL_Note_Edit::GetItemLeftMargin()
{
	return POPUP_ITEM_TEXT_INDENT;
}

FX_FLOAT CPWL_Note_Edit::GetItemRightMargin()
{
	return POPUP_ITEM_TEXT_INDENT;
}

/* -------------------------------- CPWL_Note_LBBox --------------------------------*/

CPWL_Note_LBBox::CPWL_Note_LBBox()
{
}

CPWL_Note_LBBox::~CPWL_Note_LBBox()
{
}

void CPWL_Note_LBBox::DrawThisAppearance(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device)
{
	CPDF_Rect rcClient = this->GetClientRect();

	CFX_GraphStateData gsd;
	gsd.m_LineWidth = 1.0f;

	CFX_PathData pathCross;

	pathCross.SetPointCount(4);
	pathCross.SetPoint(0, rcClient.left, rcClient.top, FXPT_MOVETO);
	pathCross.SetPoint(1, rcClient.right, rcClient.bottom, FXPT_LINETO);
	pathCross.SetPoint(2, rcClient.left, rcClient.bottom + rcClient.Height() * 0.5f, FXPT_MOVETO);
	pathCross.SetPoint(3, rcClient.left + rcClient.Width() * 0.5f, rcClient.bottom, FXPT_LINETO);
	
	pDevice->DrawPath(&pathCross, pUser2Device, &gsd, 
		0, CPWL_Utils::PWLColorToFXColor(GetTextColor(),this->GetTransparency()), FXFILL_ALTERNATE);
}

/* -------------------------------- CPWL_Note_RBBox --------------------------------*/

CPWL_Note_RBBox::CPWL_Note_RBBox()
{
}

CPWL_Note_RBBox::~CPWL_Note_RBBox()
{
}

void CPWL_Note_RBBox::DrawThisAppearance(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device)
{
	CPDF_Rect rcClient = this->GetClientRect();

	CFX_GraphStateData gsd;
	gsd.m_LineWidth = 1.0f;

	CFX_PathData pathCross;

	pathCross.SetPointCount(4);
	pathCross.SetPoint(0, rcClient.right, rcClient.top, FXPT_MOVETO);
	pathCross.SetPoint(1, rcClient.left, rcClient.bottom, FXPT_LINETO);
	pathCross.SetPoint(2, rcClient.right, rcClient.bottom + rcClient.Height() * 0.5f, FXPT_MOVETO);
	pathCross.SetPoint(3, rcClient.left + rcClient.Width() * 0.5f, rcClient.bottom, FXPT_LINETO);
	
	pDevice->DrawPath(&pathCross, pUser2Device, &gsd, 
		0, CPWL_Utils::PWLColorToFXColor(GetTextColor(),this->GetTransparency()), FXFILL_ALTERNATE);
}

/* --------------------------------- CPWL_Note_Icon ---------------------------------- */

CPWL_Note_Icon::CPWL_Note_Icon() : m_nType(0)
{
}

CPWL_Note_Icon::~CPWL_Note_Icon()
{
}

void CPWL_Note_Icon::SetIconType(FX_INT32 nType)
{
	m_nType = nType;
}

void CPWL_Note_Icon::DrawThisAppearance(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device)
{
	CPWL_Utils::DrawIconAppStream(pDevice, pUser2Device, m_nType, GetClientRect(), 
		this->GetBackgroundColor(), PWL_DEFAULT_BLACKCOLOR, this->GetTransparency());
}

/* --------------------------------- CPWL_Note_CloseBox ---------------------------------- */

CPWL_Note_CloseBox::CPWL_Note_CloseBox() : m_bMouseDown(FALSE)
{
}

CPWL_Note_CloseBox::~CPWL_Note_CloseBox()
{
}

void CPWL_Note_CloseBox::DrawThisAppearance(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device)
{
	CPWL_Button::DrawThisAppearance(pDevice, pUser2Device);

	CPDF_Rect rcClient = this->GetClientRect();
	rcClient = CPWL_Utils::DeflateRect(rcClient, 2.0f);

	CFX_GraphStateData gsd;
	gsd.m_LineWidth = 1.0f;

	CFX_PathData pathCross;

	if (m_bMouseDown)
	{
		rcClient.left += 0.5f;
		rcClient.right += 0.5f;
		rcClient.top -= 0.5f;
		rcClient.bottom -= 0.5f;
	}

	pathCross.SetPointCount(4);
	pathCross.SetPoint(0, rcClient.left, rcClient.bottom, FXPT_MOVETO);
	pathCross.SetPoint(1, rcClient.right, rcClient.top, FXPT_LINETO);
	pathCross.SetPoint(2, rcClient.left, rcClient.top, FXPT_MOVETO);
	pathCross.SetPoint(3, rcClient.right, rcClient.bottom, FXPT_LINETO);
	
	pDevice->DrawPath(&pathCross, pUser2Device, &gsd, 
		0, CPWL_Utils::PWLColorToFXColor(GetTextColor(),this->GetTransparency()), FXFILL_ALTERNATE);
}

FX_BOOL CPWL_Note_CloseBox::OnLButtonDown(const CPDF_Point & point, FX_DWORD nFlag)
{
	SetBorderStyle(PBS_INSET);
	InvalidateRect(NULL);

	m_bMouseDown = TRUE;

	return CPWL_Button::OnLButtonDown(point,nFlag);
}

FX_BOOL	CPWL_Note_CloseBox::OnLButtonUp(const CPDF_Point & point, FX_DWORD nFlag)
{
	m_bMouseDown = FALSE;

	SetBorderStyle(PBS_BEVELED);
	InvalidateRect(NULL);

	return CPWL_Button::OnLButtonUp(point,nFlag);
}

/* ------------------------------ CPWL_Note_Contents ------------------------------- */

CPWL_Note_Contents::CPWL_Note_Contents() : m_pEdit(NULL)
{
}

CPWL_Note_Contents::~CPWL_Note_Contents()
{
}

CFX_ByteString CPWL_Note_Contents::GetClassName() const
{
	return "CPWL_Note_Contents";
}

void CPWL_Note_Contents::CreateChildWnd(const PWL_CREATEPARAM & cp)
{
	m_pEdit = new CPWL_Note_Edit;
	PWL_CREATEPARAM ecp = cp;
	ecp.pParentWnd = this;
	ecp.dwFlags = PWS_VISIBLE | PWS_CHILD | PES_MULTILINE | PES_AUTORETURN | PES_TEXTOVERFLOW | PES_UNDO | PES_SPELLCHECK;

	m_pEdit->EnableNotify(FALSE);
	m_pEdit->Create(ecp);
	m_pEdit->EnableNotify(TRUE);
}

void CPWL_Note_Contents::SetText(const CFX_WideString& sText)
{
	if (m_pEdit)
	{
		m_pEdit->EnableNotify(FALSE);
		m_pEdit->SetText(sText);
		m_pEdit->EnableNotify(TRUE);
		OnNotify(m_pEdit, PNM_NOTEEDITCHANGED, 0, 0);		
	}
}

CFX_WideString CPWL_Note_Contents::GetText() const
{
	if (m_pEdit)
		return m_pEdit->GetText();

	return L"";
}

CPWL_NoteItem* CPWL_Note_Contents::CreateSubItem()
{
	CPWL_NoteItem* pNoteItem = new CPWL_NoteItem;
	PWL_CREATEPARAM icp = this->GetCreationParam();
	icp.pParentWnd = this;
	icp.dwFlags =  PWS_CHILD | PWS_VISIBLE | PWS_BACKGROUND;
	pNoteItem->Create(icp);

	pNoteItem->OnCreateNoteItem();
	
	pNoteItem->ResetSubjectName(m_aChildren.GetSize() - 1);

	FX_SYSTEMTIME st;
	if (IFX_SystemHandler* pSH = this->GetSystemHandler())
		st = pSH->GetLocalTime();
	pNoteItem->SetDateTime(st);

	pNoteItem->SetContents(L"");

	this->OnNotify(pNoteItem, PNM_NOTEEDITCHANGED, 0, 0);	

	return pNoteItem;
}

FX_INT32 CPWL_Note_Contents::CountSubItems() const
{
	return m_aChildren.GetSize() - 1;
}

IPWL_NoteItem* CPWL_Note_Contents::GetSubItems(FX_INT32 index) const
{
	FX_INT32 nIndex = index + 1;

	if (nIndex > 0 && nIndex < m_aChildren.GetSize())
		if (CPWL_Wnd* pChild = m_aChildren.GetAt(nIndex))
		{
			ASSERT(pChild->GetClassName() == "CPWL_NoteItem");
			CPWL_NoteItem* pItem = (CPWL_NoteItem*)pChild;
			return pItem;
		}
	return NULL;
}

void CPWL_Note_Contents::DeleteSubItem(IPWL_NoteItem* pNoteItem)
{
	FX_INT32 nIndex = this->GetItemIndex((CPWL_NoteItem*)pNoteItem);

	if (nIndex > 0)
	{
		if (CPWL_NoteItem* pPWLNoteItem = (CPWL_NoteItem*)pNoteItem)
		{
			pPWLNoteItem->KillFocus();
			pPWLNoteItem->Destroy();
			delete pPWLNoteItem;
		}

		for (FX_INT32 i=nIndex,sz=m_aChildren.GetSize(); i<sz; i++)
		{
			if (CPWL_Wnd* pChild = m_aChildren.GetAt(i))
			{
				ASSERT(pChild->GetClassName() == "CPWL_NoteItem");
				CPWL_NoteItem* pItem = (CPWL_NoteItem*)pChild;
				pItem->ResetSubjectName(i);
			}
		}

		this->OnNotify(this, PNM_NOTEEDITCHANGED, 0, 0);
	}
}

IPWL_NoteItem* CPWL_Note_Contents::GetHitNoteItem(const CPDF_Point& point)
{
	CPDF_Point pt = this->ParentToChild(point);

	for (FX_INT32 i=0,sz=m_aChildren.GetSize(); i<sz; i++)
	{
		if (CPWL_Wnd* pChild = m_aChildren.GetAt(i))
		{
			if (pChild->GetClassName() == "CPWL_NoteItem")
			{
				CPWL_NoteItem* pNoteItem = (CPWL_NoteItem*)pChild;
				if (IPWL_NoteItem* pRet = pNoteItem->GetHitNoteItem(pt))
					return pRet;
			}
		}
	}
	return NULL;
}

void CPWL_Note_Contents::OnNotify(CPWL_Wnd* pWnd, FX_DWORD msg, FX_INTPTR wParam, FX_INTPTR lParam)
{
	switch (msg)
	{
	case PNM_NOTEEDITCHANGED:
		{
			FX_INT32 nIndex = this->GetItemIndex(pWnd);
			if (nIndex < 0) nIndex = 0;

			m_pEdit->EnableNotify(FALSE);	
			this->ResetContent(nIndex);
			m_pEdit->EnableNotify(TRUE);

			for (FX_INT32 i=nIndex+1, sz=m_aChildren.GetSize(); i<sz; i++)
			{
				if (CPWL_Wnd* pChild = m_aChildren.GetAt(i))
					pChild->OnNotify(this, PNM_NOTERESET, 0, 0);
			}

			if (CPWL_Wnd * pParent = this->GetParentWindow())
			{
				pParent->OnNotify(this, PNM_NOTEEDITCHANGED, 0, 0);
			}
		}
		return;
	case PNM_SCROLLWINDOW:
		this->SetScrollPos(CPDF_Point(0.0f, *(FX_FLOAT*)lParam));
		this->ResetFace();
		InvalidateRect(NULL);
		return;		
	case PNM_SETCARETINFO:
		if (PWL_CARET_INFO * pInfo = (PWL_CARET_INFO*)wParam)
		{
			PWL_CARET_INFO newInfo = *pInfo;
			newInfo.bVisible = TRUE;
			newInfo.ptHead = this->ChildToParent(pInfo->ptHead);
			newInfo.ptFoot = this->ChildToParent(pInfo->ptFoot);

			if (CPWL_Wnd * pParent = this->GetParentWindow())
			{
				pParent->OnNotify(this, PNM_SETCARETINFO, (FX_INTPTR)&newInfo, 0);
			}
		}
		return;
	case PNM_NOTERESET:
		{
			m_pEdit->EnableNotify(FALSE);	
			this->ResetContent(0);
			m_pEdit->EnableNotify(TRUE);

			for (FX_INT32 i=1, sz=m_aChildren.GetSize(); i<sz; i++)
			{
				if (CPWL_Wnd* pChild = m_aChildren.GetAt(i))
					pChild->OnNotify(this, PNM_NOTERESET, 0, 0);
			}	

			m_pEdit->EnableNotify(FALSE);	
			this->ResetContent(0);
			m_pEdit->EnableNotify(TRUE);
		}
		return;
	}

	CPWL_Wnd::OnNotify(pWnd, msg, wParam, lParam);
}

FX_BOOL	CPWL_Note_Contents::OnLButtonDown(const CPDF_Point & point, FX_DWORD nFlag)
{
	if (CPWL_Wnd::OnLButtonDown(point,nFlag)) return TRUE;

	if (!m_pEdit->IsFocused())
	{
		m_pEdit->SetFocus();
	}

	return TRUE;
}

void CPWL_Note_Contents::SetEditFocus(FX_BOOL bLast)
{
	if (!m_pEdit->IsFocused())
	{
		m_pEdit->SetFocus();
		m_pEdit->SetCaret(bLast ? m_pEdit->GetTotalWords() : 0);
	}
}

CPWL_Edit* CPWL_Note_Contents::GetEdit() const
{
	return m_pEdit;
}

void CPWL_Note_Contents::EnableModify(FX_BOOL bEnabled)
{
	if (!bEnabled)
		m_pEdit->AddFlag(PWS_READONLY);
	else
		m_pEdit->RemoveFlag(PWS_READONLY);

	for (FX_INT32 i=0,sz=m_aChildren.GetSize(); i<sz; i++)
	{
		if (CPWL_Wnd* pChild = m_aChildren.GetAt(i))
		{
			if (pChild->GetClassName() == "CPWL_NoteItem")
			{
				CPWL_NoteItem* pNoteItem = (CPWL_NoteItem*)pChild;
				pNoteItem->EnableModify(bEnabled);
			}
		}
	}
}

void CPWL_Note_Contents::EnableRead(FX_BOOL bEnabled)
{
	if (!bEnabled)
		m_pEdit->AddFlag(PES_NOREAD);
	else
		m_pEdit->RemoveFlag(PES_NOREAD);

	for (FX_INT32 i=0,sz=m_aChildren.GetSize(); i<sz; i++)
	{
		if (CPWL_Wnd* pChild = m_aChildren.GetAt(i))
		{
			if (pChild->GetClassName() == "CPWL_NoteItem")
			{
				CPWL_NoteItem* pNoteItem = (CPWL_NoteItem*)pChild;
				pNoteItem->EnableRead(bEnabled);
			}
		}
	}
}

/* ---------------------------------- CPWL_NoteItem ---------------------------------- */

CPWL_NoteItem::CPWL_NoteItem() :
	m_pSubject(NULL),
	m_pDateTime(NULL),
	m_pContents(NULL),
	m_pPrivateData(NULL),
	m_sAuthor(L""),
	m_fOldItemHeight(0.0f),
	m_bSizeChanged(FALSE),
	m_bAllowModify(TRUE)
{
}

CPWL_NoteItem::~CPWL_NoteItem()
{
}

CFX_ByteString CPWL_NoteItem::GetClassName() const
{
	return "CPWL_NoteItem";
}

void CPWL_NoteItem::CreateChildWnd(const PWL_CREATEPARAM & cp)
{
	CPWL_Color sTextColor;

	if (CPWL_Utils::IsBlackOrWhite(this->GetBackgroundColor()))
		sTextColor = PWL_DEFAULT_WHITECOLOR;
	else
		sTextColor = PWL_DEFAULT_BLACKCOLOR;

	m_pSubject = new CPWL_Label;
	PWL_CREATEPARAM scp = cp;
	scp.pParentWnd = this;
	scp.dwFlags = PWS_VISIBLE | PWS_CHILD | PES_LEFT | PES_TOP;
	scp.sTextColor = sTextColor;
	m_pSubject->Create(scp);

	m_pDateTime = new CPWL_Label;
	PWL_CREATEPARAM dcp = cp;
	dcp.pParentWnd = this;
	dcp.dwFlags = PWS_VISIBLE | PWS_CHILD | PES_RIGHT | PES_TOP;
	dcp.sTextColor = sTextColor;
	m_pDateTime->Create(dcp);

	m_pContents = new CPWL_Note_Contents;
	PWL_CREATEPARAM ccp = cp;
	ccp.pParentWnd = this;
	//ccp.sBackgroundColor = PWL_DEFAULT_WHITECOLOR;
	ccp.sBackgroundColor = CPWL_Color(COLORTYPE_RGB, 240/255.0f, 240/255.0f, 240/255.0f);
	ccp.dwFlags = PWS_VISIBLE | PWS_CHILD | PWS_BACKGROUND;
	m_pContents->Create(ccp);
	m_pContents->SetItemSpace(POPUP_ITEM_SPACE);
	m_pContents->SetTopSpace(POPUP_ITEM_SPACE);
	m_pContents->SetBottomSpace(POPUP_ITEM_SPACE);
}

void CPWL_NoteItem::RePosChildWnd()
{
	if (this->IsValid())
	{
		ASSERT(m_pSubject != NULL);
		ASSERT(m_pDateTime != NULL);
		ASSERT(m_pContents != NULL);

		CPDF_Rect rcClient = GetClientRect();

		CPDF_Rect rcSubject = rcClient;
		rcSubject.left += POPUP_ITEM_TEXT_INDENT;
		rcSubject.top = rcClient.top;
		rcSubject.right = PWL_MIN(rcSubject.left + m_pSubject->GetContentRect().Width() + 1.0f, rcClient.right);
		rcSubject.bottom = rcSubject.top - m_pSubject->GetContentRect().Height();
		rcSubject.Normalize();
		m_pSubject->Move(rcSubject, TRUE, FALSE);
		m_pSubject->SetVisible(CPWL_Utils::ContainsRect(rcClient, rcSubject));

		CPDF_Rect rcDate = rcClient;
		rcDate.right -= POPUP_ITEM_TEXT_INDENT;
		rcDate.left = PWL_MAX(rcDate.right - m_pDateTime->GetContentRect().Width() - 1.0f, rcSubject.right);
		rcDate.bottom = rcDate.top - m_pDateTime->GetContentRect().Height();
		rcDate.Normalize();
		m_pDateTime->Move(rcDate, TRUE, FALSE);
		m_pDateTime->SetVisible(CPWL_Utils::ContainsRect(rcClient, rcDate));

		CPDF_Rect rcContents = rcClient;
		rcContents.left += 1.0f;
		rcContents.right -= 1.0f;
		rcContents.top = rcDate.bottom - POPUP_ITEM_HEAD_BOTTOM;
		rcContents.bottom += POPUP_ITEM_BOTTOMWIDTH;
		rcContents.Normalize();
		m_pContents->Move(rcContents, TRUE, FALSE);
		m_pContents->SetVisible(CPWL_Utils::ContainsRect(rcClient, rcContents));
	}

	SetClipRect(CPWL_Utils::InflateRect(GetWindowRect(),1.0f));
}

void CPWL_NoteItem::SetPrivateData(void* pData)
{
	m_pPrivateData = pData;
}

void CPWL_NoteItem::SetBkColor(const CPWL_Color& color)
{
	CPWL_Color sBK = color;
	this->SetBackgroundColor(sBK);

	CPWL_Color sTextColor;

	if (CPWL_Utils::IsBlackOrWhite(sBK))
		sTextColor = PWL_DEFAULT_WHITECOLOR;
	else
		sTextColor = PWL_DEFAULT_BLACKCOLOR;

	this->SetTextColor(sTextColor);
	if (m_pSubject)
		m_pSubject->SetTextColor(sTextColor);
	if (m_pDateTime)
		m_pDateTime->SetTextColor(sTextColor);

	this->InvalidateRect(NULL);

	if (IPWL_NoteNotify* pNotify = GetNoteNotify())
	{
		pNotify->OnSetBkColor(this);
	}
}

void CPWL_NoteItem::SetSubjectName(const CFX_WideString& sName)
{
	if (m_pSubject)
	{
		m_pSubject->SetText(sName);	
	}

	if (IPWL_NoteNotify* pNotify = GetNoteNotify())
	{
		pNotify->OnSetSubjectName(this);
	}
}

void CPWL_NoteItem::SetAuthorName(const CFX_WideString& sName)
{
	m_sAuthor = sName;
	ResetSubjectName(-1);	

	if (IPWL_NoteNotify* pNotify = GetNoteNotify())
	{
		pNotify->OnSetAuthorName(this);
	}
}

void CPWL_NoteItem::ResetSubjectName(FX_INT32 nItemIndex)
{
	if (nItemIndex < 0)
	{
		if (CPWL_Wnd* pParent = this->GetParentWindow())
		{
			ASSERT(pParent->GetClassName() == "CPWL_Note_Contents");

			CPWL_Note_Contents* pContents = (CPWL_Note_Contents*)pParent;
			nItemIndex = pContents->GetItemIndex(this);
		}
	}

	const CPWL_Note* pNote = GetNote();
	ASSERT(pNote != NULL);

	CFX_WideString sSubject;
	sSubject.Format(pNote->GetReplyString(), nItemIndex);

	if (!m_sAuthor.IsEmpty())
	{
		
		sSubject += L" - ";
		sSubject += m_sAuthor;
	}
	this->SetSubjectName(sSubject);
	this->RePosChildWnd();
}

void CPWL_NoteItem::SetDateTime(FX_SYSTEMTIME time)
{
	m_dtNote = time;
	
	CFX_WideString swTime;
	swTime.Format(L"%04d-%02d-%02d %02d:%02d:%02d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
	if (m_pDateTime)
	{
		m_pDateTime->SetText(swTime);
	}

	this->RePosChildWnd();

	if (IPWL_NoteNotify* pNotify = GetNoteNotify())
	{
		pNotify->OnSetDateTime(this);
	}
}

void CPWL_NoteItem::SetContents(const CFX_WideString& sContents)
{
	if (m_pContents)
	{
		m_pContents->SetText(sContents);
	}

	if (IPWL_NoteNotify* pNotify = GetNoteNotify())
	{
		pNotify->OnSetContents(this);
	}
}

CPWL_NoteItem* CPWL_NoteItem::GetParentNoteItem() const
{
	if (CPWL_Wnd* pParent = this->GetParentWindow())
	{
		if (CPWL_Wnd* pGrand = pParent->GetParentWindow())
		{
			ASSERT(pGrand->GetClassName() == "CPWL_NoteItem");
			return (CPWL_NoteItem*)pGrand;
		}
	}

	return NULL;
}

IPWL_NoteItem* CPWL_NoteItem::GetParentItem() const
{
	return GetParentNoteItem();
}

CPWL_Edit* CPWL_NoteItem::GetEdit() const
{
	if (m_pContents)
		return m_pContents->GetEdit();
	return NULL;
}

void* CPWL_NoteItem::GetPrivateData() const
{
	return m_pPrivateData;
}

CFX_WideString CPWL_NoteItem::GetAuthorName() const
{
	return m_sAuthor; 
}

CPWL_Color CPWL_NoteItem::GetBkColor() const
{
	return this->GetBackgroundColor();
}

CFX_WideString CPWL_NoteItem::GetContents() const
{
	if (m_pContents)
		return m_pContents->GetText();

	return L"";
}

FX_SYSTEMTIME CPWL_NoteItem::GetDateTime() const
{
	return m_dtNote;
}

CFX_WideString CPWL_NoteItem::GetSubjectName() const
{
	if (m_pSubject)
		return m_pSubject->GetText();

	return L"";
}

CPWL_NoteItem* CPWL_NoteItem::CreateNoteItem()
{
	if (m_pContents)
		return m_pContents->CreateSubItem();

	return NULL;
}

IPWL_NoteItem* CPWL_NoteItem::CreateSubItem()
{
	return CreateNoteItem();
}

FX_INT32 CPWL_NoteItem::CountSubItems() const
{
	if (m_pContents)
		return m_pContents->CountSubItems();

	return 0;
}

IPWL_NoteItem* CPWL_NoteItem::GetSubItems(FX_INT32 index) const
{
	if (m_pContents)
		return m_pContents->GetSubItems(index);

	return NULL;
}

void CPWL_NoteItem::DeleteSubItem(IPWL_NoteItem* pNoteItem)
{
	this->KillFocus();

	if (IPWL_NoteNotify* pNotify = GetNoteNotify())
	{
		pNotify->OnItemDelete(pNoteItem);
	}

	if (m_pContents)
		m_pContents->DeleteSubItem(pNoteItem);
}

IPWL_NoteItem* CPWL_NoteItem::GetHitNoteItem(const CPDF_Point& point)
{
	CPDF_Point pt = this->ParentToChild(point);

	if (this->WndHitTest(pt))
	{
		if (m_pContents)
		{
			if (IPWL_NoteItem* pNoteItem = m_pContents->GetHitNoteItem(pt))
				return pNoteItem;
		}

		return this;
	}

	return NULL;
}

IPWL_NoteItem* CPWL_NoteItem::GetFocusedNoteItem() const
{
	if (const CPWL_Wnd* pWnd = this->GetFocused())
	{
		if (pWnd->GetClassName() == "CPWL_Edit")
		{
			if (CPWL_Wnd* pParent = pWnd->GetParentWindow())
			{
				ASSERT(pParent->GetClassName() == "CPWL_Note_Contents");
				
				if (CPWL_Wnd* pGrand = pParent->GetParentWindow())
				{
					ASSERT(pGrand->GetClassName() == "CPWL_NoteItem");					
					return (CPWL_NoteItem*)pGrand;
				}
			}
		}
	}

	return NULL;
}

FX_FLOAT CPWL_NoteItem::GetItemHeight(FX_FLOAT fLimitWidth)
{
	if (fLimitWidth > 0)
	{
		if (!m_bSizeChanged)
			return m_fOldItemHeight;

		m_bSizeChanged = FALSE;

		ASSERT(m_pSubject != NULL);
		ASSERT(m_pDateTime != NULL);
		ASSERT(m_pContents != NULL);

		FX_FLOAT fRet = m_pDateTime->GetContentRect().Height();
		FX_FLOAT fBorderWidth = (FX_FLOAT)this->GetBorderWidth();
		if (fLimitWidth > fBorderWidth * 2)
			fRet += m_pContents->GetContentsHeight(fLimitWidth - fBorderWidth * 2);
		fRet += POPUP_ITEM_HEAD_BOTTOM + POPUP_ITEM_BOTTOMWIDTH + fBorderWidth * 2;

		return m_fOldItemHeight = fRet;
	}

	return 0;
}

FX_FLOAT CPWL_NoteItem::GetItemLeftMargin()
{
	return POPUP_ITEM_SIDEMARGIN;
}

FX_FLOAT CPWL_NoteItem::GetItemRightMargin()
{
	return POPUP_ITEM_SIDEMARGIN;
}

FX_BOOL	CPWL_NoteItem::OnLButtonDown(const CPDF_Point& point, FX_DWORD nFlag)
{
	if (!m_pContents->WndHitTest(m_pContents->ParentToChild(point)))
	{
		SetNoteFocus(FALSE);
	}

	CPWL_Wnd::OnLButtonDown(point,nFlag);

	return TRUE;
}

FX_BOOL	CPWL_NoteItem::OnRButtonUp(const CPDF_Point & point, FX_DWORD nFlag)
{
	if (!m_pContents->WndHitTest(m_pContents->ParentToChild(point)))
	{
		SetNoteFocus(FALSE);
		PopupNoteItemMenu(point);

		return TRUE;
	}

	return CPWL_Wnd::OnRButtonUp(point,nFlag);
}

void CPWL_NoteItem::OnNotify(CPWL_Wnd* pWnd, FX_DWORD msg, FX_INTPTR wParam, FX_INTPTR lParam)
{
	switch (msg)
	{
	case PNM_NOTEEDITCHANGED:	
		m_bSizeChanged = TRUE;

		if (CPWL_Wnd* pParent = this->GetParentWindow())
		{
			pParent->OnNotify(this, PNM_NOTEEDITCHANGED, 0, 0);
		}
		return;	
	case PNM_SETCARETINFO:
		if (PWL_CARET_INFO * pInfo = (PWL_CARET_INFO*)wParam)
		{
			PWL_CARET_INFO newInfo = *pInfo;
			newInfo.bVisible = TRUE;
			newInfo.ptHead = this->ChildToParent(pInfo->ptHead);
			newInfo.ptFoot = this->ChildToParent(pInfo->ptFoot);

			if (CPWL_Wnd * pParent = this->GetParentWindow())
			{
				pParent->OnNotify(this, PNM_SETCARETINFO, (FX_INTPTR)&newInfo, 0);
			}
		}
		return;
	case PNM_NOTERESET:
		m_bSizeChanged = TRUE;
		m_pContents->OnNotify(this, PNM_NOTERESET, 0, 0);
		
		return;
	}

	CPWL_Wnd::OnNotify(pWnd, msg, wParam, lParam);
}

void CPWL_NoteItem::PopupNoteItemMenu(const CPDF_Point& point)
{
	if (IPWL_NoteNotify* pNotify = GetNoteNotify())
	{
		FX_INT32 x,y;
		PWLtoWnd(point, x, y);
		if (IFX_SystemHandler* pSH = GetSystemHandler())
			pSH->ClientToScreen(GetAttachedHWnd(), x, y);
		pNotify->OnPopupMenu(this, x, y);
	}
}

const CPWL_Note* CPWL_NoteItem::GetNote() const
{
	if (const CPWL_Wnd* pRoot = this->GetRootWnd())
	{
		ASSERT(pRoot->GetClassName() == "CPWL_NoteItem");
		CPWL_NoteItem* pNoteItem = (CPWL_NoteItem*)pRoot;
		if (pNoteItem->IsTopItem())
		{
			return (CPWL_Note*)pNoteItem;
		}
	}

	return NULL;
}

IPWL_NoteNotify* CPWL_NoteItem::GetNoteNotify() const
{
	if (const CPWL_Note* pNote = GetNote())
		return pNote->GetNoteNotify();

	return NULL;
}

void CPWL_NoteItem::OnCreateNoteItem()
{
	if (IPWL_NoteNotify* pNotify = GetNoteNotify())
	{
		pNotify->OnItemCreate(this);
	}
}

void CPWL_NoteItem::OnContentsValidate()
{
	if (IPWL_NoteNotify* pNotify = GetNoteNotify())
	{
		pNotify->OnSetContents(this);
	}
}

void CPWL_NoteItem::SetNoteFocus(FX_BOOL bLast)
{
	m_pContents->SetEditFocus(bLast);
}

void CPWL_NoteItem::EnableModify(FX_BOOL bEnabled)
{
	m_pContents->EnableModify(bEnabled);
	m_bAllowModify = bEnabled;
}

void CPWL_NoteItem::EnableRead(FX_BOOL bEnabled)
{
	m_pContents->EnableRead(bEnabled);
}

/* ---------------------------------- CPWL_Note ---------------------------------- */

CPWL_Note::CPWL_Note(IPopup_Note* pPopupNote, IPWL_NoteNotify* pNoteNotify, IPWL_NoteHandler* pNoteHandler) : 
	m_pAuthor(NULL),
	m_pIcon(NULL),
	m_pCloseBox(NULL),
	m_pLBBox(NULL),
	m_pRBBox(NULL),
	m_pContentsBar(NULL),
	m_pOptions(NULL),
	m_pNoteNotify(pNoteNotify),
	m_bResizing(FALSE),
	m_rcCaption(0,0,0,0),
	m_bEnalbleNotify(TRUE),
	m_pPopupNote(pPopupNote)
{
}

CPWL_Note::~CPWL_Note()
{
}

IPWL_NoteItem* CPWL_Note::Reply()
{
	return CreateNoteItem();
}

void CPWL_Note::EnableNotify(FX_BOOL bEnabled)
{
	m_bEnalbleNotify = bEnabled;
}

void CPWL_Note::RePosChildWnd()
{
	RePosNoteChildren();
	m_pContents->OnNotify(this, PNM_NOTERESET, 0, 0);
	ResetScrollBar();
	m_pContents->OnNotify(this, PNM_NOTERESET, 0, 0);
	this->OnNotify(this, PNM_NOTEEDITCHANGED, 0, 0);
	//Í¬²½
	if (const CPWL_Wnd* pWnd = this->GetFocused())
	{
		if (pWnd->GetClassName() == "CPWL_Edit")
		{
			CPWL_Edit* pEdit = (CPWL_Edit*)pWnd;
			pEdit->SetCaret(pEdit->GetCaret());
		}
	}
	//CPDF_Point ptNew = m_pContents->GetScrollPos();
	//m_pContentsBar->OnNotify(this, PNM_SETSCROLLPOS, SBT_VSCROLL, (FX_INTPTR)&ptNew.y);
}

FX_BOOL CPWL_Note::ResetScrollBar()
{
	FX_BOOL bScrollChanged = FALSE;

	if (ScrollBarShouldVisible())
	{
		if (!m_pContentsBar->IsVisible())
		{
			m_pContentsBar->SetVisible(TRUE);
			if (m_pContentsBar->IsVisible())
			{
				m_pContentsBar->InvalidateRect(NULL);
				bScrollChanged = TRUE;
			}
		}	
	}
	else
	{
		if (m_pContentsBar->IsVisible())
		{
			m_pContentsBar->SetVisible(FALSE);
			m_pContentsBar->InvalidateRect(NULL);

			bScrollChanged = TRUE;
		}
	}

	if (bScrollChanged)
	{
		CPDF_Rect rcNote = this->GetClientRect();
		CPDF_Rect rcContents = m_pContents->GetWindowRect();
		rcContents.right = rcNote.right - 3.0f;
		if (m_pContentsBar->IsVisible())
			rcContents.right -= PWL_SCROLLBAR_WIDTH;
		m_pContents->Move(rcContents, TRUE, TRUE);
		m_pContents->SetScrollPos(CPDF_Point(0.0f,0.0f));
		m_pContents->InvalidateRect(NULL);
	}

	return bScrollChanged;
}

FX_BOOL CPWL_Note::ScrollBarShouldVisible()
{
	CPDF_Rect rcContentsFact = m_pContents->GetScrollArea();
	CPDF_Rect rcContentsClient = m_pContents->GetClientRect();

	return rcContentsFact.Height() > rcContentsClient.Height();	
}

void CPWL_Note::SetOptionsText(const CFX_WideString& sText)
{
	if (m_pOptions)
		m_pOptions->SetText(sText);

	RePosNoteChildren();
}

void CPWL_Note::RePosNoteChildren()
{
	if (m_bResizing) return;

	m_bResizing = TRUE;

	if (this->IsValid())
	{
		ASSERT(m_pSubject != NULL);
		ASSERT(m_pDateTime != NULL);
		ASSERT(m_pContents != NULL);
		ASSERT(m_pAuthor != NULL);
		ASSERT(m_pCloseBox != NULL);
		ASSERT(m_pIcon != NULL);
		ASSERT(m_pLBBox != NULL);
		ASSERT(m_pRBBox != NULL);
		ASSERT(m_pContentsBar != NULL);
		ASSERT(m_pOptions != NULL);

		CPDF_Rect rcClient = GetClientRect();

		CPDF_Rect rcIcon = rcClient;
		rcIcon.top -= 2.0f;
		rcIcon.right = rcIcon.left + 14.0f;
		rcIcon.bottom = rcIcon.top - 14.0f;
		rcIcon.Normalize();
		m_pIcon->Move(rcIcon, TRUE, FALSE);
		m_pIcon->SetVisible(CPWL_Utils::ContainsRect(rcClient, rcIcon));

		CPDF_Rect rcCloseBox = rcClient;
		rcCloseBox.right -= 1.0f;
		rcCloseBox.top -= 1.0f;
		rcCloseBox.left = rcCloseBox.right - 14.0f;
		rcCloseBox.bottom = rcCloseBox.top - 14.0f;
		rcCloseBox.Normalize();
		m_pCloseBox->Move(rcCloseBox, TRUE, FALSE);
		m_pCloseBox->SetVisible(CPWL_Utils::ContainsRect(rcClient, rcCloseBox));

		CPDF_Rect rcDate = rcClient;
		rcDate.right = rcCloseBox.left - POPUP_ITEM_TEXT_INDENT;
		rcDate.left = PWL_MAX(rcDate.right - m_pDateTime->GetContentRect().Width() - 1.0f, rcIcon.right + 1.0f);
		rcDate.top = rcClient.top - 2.0f;
		rcDate.bottom = rcDate.top - m_pDateTime->GetContentRect().Height();
		rcDate.Normalize();
		m_pDateTime->Move(rcDate, TRUE, FALSE);
		m_pDateTime->SetVisible(CPWL_Utils::ContainsRect(rcClient, rcDate));

		CPDF_Rect rcSubject = rcClient;
		rcSubject.top = rcClient.top - 2.0f;
		rcSubject.left = rcIcon.right + POPUP_ITEM_TEXT_INDENT;
		rcSubject.right = PWL_MIN(rcSubject.left + m_pSubject->GetContentRect().Width() + 1.0f, rcDate.left - 1.0f);
		rcSubject.bottom = rcSubject.top - m_pSubject->GetContentRect().Height();
		rcSubject.Normalize();
		m_pSubject->Move(rcSubject, TRUE, FALSE);	
		m_pSubject->SetVisible(CPWL_Utils::ContainsRect(rcClient, rcSubject));

		CPDF_Rect rcOptions = rcClient;
		rcOptions.left = PWL_MAX(rcOptions.right - m_pOptions->GetContentRect().Width(), rcIcon.right + 1.0f);
		rcOptions.top = rcSubject.bottom - 4.0f;
		rcOptions.bottom = rcOptions.top - m_pOptions->GetContentRect().Height();
		rcOptions.Normalize();
		m_pOptions->Move(rcOptions, TRUE, FALSE);
		m_pOptions->SetVisible(CPWL_Utils::ContainsRect(rcClient, rcOptions));

		CPDF_Rect rcAuthor = rcClient;
		rcAuthor.top = rcSubject.bottom - 4.0f;
		rcAuthor.left = rcSubject.left;
		rcAuthor.right = PWL_MIN(rcSubject.left + m_pAuthor->GetContentRect().Width() + 1.0f, rcOptions.left - 1.0f);
		rcAuthor.bottom = rcAuthor.top - m_pAuthor->GetContentRect().Height();
		rcAuthor.Normalize();
		m_pAuthor->Move(rcAuthor, TRUE, FALSE);
		m_pAuthor->SetVisible(CPWL_Utils::ContainsRect(rcClient, rcAuthor));

		CPDF_Rect rcLBBox = rcClient;
		rcLBBox.top = rcLBBox.bottom + 7.0f;
		rcLBBox.right = rcLBBox.left + 7.0f;
		rcLBBox.Normalize();
		m_pLBBox->Move(rcLBBox, TRUE, FALSE);
		m_pLBBox->SetVisible(CPWL_Utils::ContainsRect(rcClient, rcLBBox));

		CPDF_Rect rcRBBox = rcClient;
		rcRBBox.top = rcRBBox.bottom + 7.0f;
		rcRBBox.left = rcRBBox.right - 7.0f;
		rcRBBox.Normalize();
		m_pRBBox->Move(rcRBBox, TRUE, FALSE);
		m_pRBBox->SetVisible(CPWL_Utils::ContainsRect(rcClient, rcRBBox));

		CPDF_Rect rcContents = rcClient;
		rcContents.top = rcAuthor.bottom - POPUP_ITEM_HEAD_BOTTOM;
		rcContents.left += 3.0f;
		rcContents.right -= 3.0f;
		if (m_pContentsBar->IsVisible())
			rcContents.right -= PWL_SCROLLBAR_WIDTH;
		rcContents.bottom += 14.0f;
		rcContents.Normalize();
		m_pContents->Move(rcContents, FALSE, FALSE);
		m_pContents->SetVisible(CPWL_Utils::ContainsRect(rcClient, rcContents));

		CPDF_Rect rcContentsBar = rcContents;
		rcContentsBar.right = rcClient.right - 3.0f;
		rcContentsBar.left = rcContentsBar.right - PWL_SCROLLBAR_WIDTH;
		rcContentsBar.Normalize();
		m_pContentsBar->Move(rcContentsBar, TRUE, FALSE);
		
		m_rcCaption = rcClient;
		m_rcCaption.bottom = rcContents.top;
	}

	m_bResizing = FALSE;
}

//0-normal / 1-caption / 2-leftbottom corner / 3-rightbottom corner / 4-close / 5-options 
FX_INT32 CPWL_Note::NoteHitTest(const CPDF_Point& point) const
{
	ASSERT(m_pSubject != NULL);
	ASSERT(m_pDateTime != NULL);
	ASSERT(m_pContents != NULL);
	ASSERT(m_pAuthor != NULL);
	ASSERT(m_pIcon != NULL);
	ASSERT(m_pContentsBar != NULL);

	ASSERT(m_pCloseBox != NULL);
	ASSERT(m_pLBBox != NULL);
	ASSERT(m_pRBBox != NULL);
	ASSERT(m_pOptions != NULL);

	GetClientRect();

	if (m_pSubject->WndHitTest(m_pSubject->ParentToChild(point))) return 1;
	if (m_pDateTime->WndHitTest(m_pDateTime->ParentToChild(point))) return 1;
	if (m_pAuthor->WndHitTest(m_pAuthor->ParentToChild(point))) return 1;
	if (m_pIcon->WndHitTest(m_pIcon->ParentToChild(point))) return 1;

	if (m_pContents->WndHitTest(m_pContents->ParentToChild(point))) return 0;
	if (m_pContentsBar->WndHitTest(m_pContentsBar->ParentToChild(point))) return 0;

	if (m_pCloseBox->WndHitTest(m_pCloseBox->ParentToChild(point))) return 4;
	if (m_pLBBox->WndHitTest(m_pLBBox->ParentToChild(point))) return 2;
	if (m_pRBBox->WndHitTest(m_pRBBox->ParentToChild(point))) return 3;
	if (m_pOptions->WndHitTest(m_pOptions->ParentToChild(point))) return 5;

	return 1;
}

void CPWL_Note::CreateChildWnd(const PWL_CREATEPARAM & cp)
{
	CPWL_NoteItem::CreateChildWnd(cp);

	CPWL_Color sTextColor;

	if (CPWL_Utils::IsBlackOrWhite(this->GetBackgroundColor()))
		sTextColor = PWL_DEFAULT_WHITECOLOR;
	else
		sTextColor = PWL_DEFAULT_BLACKCOLOR;

	m_pAuthor = new CPWL_Label;
	PWL_CREATEPARAM acp = cp;
	acp.pParentWnd = this;
	acp.dwFlags = PWS_VISIBLE | PWS_CHILD | PES_LEFT | PES_TOP;
	acp.sTextColor = sTextColor;
	m_pAuthor->Create(acp);	 

	m_pCloseBox = new CPWL_Note_CloseBox;
	PWL_CREATEPARAM ccp = cp;
	ccp.pParentWnd = this;
	ccp.dwBorderWidth = 2;
	ccp.nBorderStyle = PBS_BEVELED;
	ccp.dwFlags = PWS_VISIBLE | PWS_CHILD | PWS_BORDER;
	ccp.sTextColor = sTextColor;
	m_pCloseBox->Create(ccp);

	m_pIcon = new CPWL_Note_Icon;
	PWL_CREATEPARAM icp = cp;
	icp.pParentWnd = this;
	icp.dwFlags = PWS_VISIBLE | PWS_CHILD;
	m_pIcon->Create(icp);

	m_pOptions = new CPWL_Note_Options;
	PWL_CREATEPARAM ocp = cp;
	ocp.pParentWnd = this;
	ocp.dwFlags = PWS_CHILD | PWS_VISIBLE;
	ocp.sTextColor = sTextColor;
	m_pOptions->Create(ocp);

	m_pLBBox = new CPWL_Note_LBBox;
	PWL_CREATEPARAM lcp = cp;
	lcp.pParentWnd = this;
	lcp.dwFlags = PWS_VISIBLE | PWS_CHILD;
	lcp.eCursorType = FXCT_NESW;
	lcp.sTextColor = sTextColor;
	m_pLBBox->Create(lcp);

	m_pRBBox = new CPWL_Note_RBBox;
	PWL_CREATEPARAM rcp = cp;
	rcp.pParentWnd = this;
	rcp.dwFlags = PWS_VISIBLE | PWS_CHILD;
	rcp.eCursorType = FXCT_NWSE;
	rcp.sTextColor = sTextColor;
	m_pRBBox->Create(rcp);

	m_pContentsBar = new CPWL_ScrollBar(SBT_VSCROLL);
	PWL_CREATEPARAM scp = cp;
	scp.pParentWnd = this;
	scp.sBackgroundColor = CPWL_Color(COLORTYPE_RGB, 240/255.0f, 240/255.0f, 240/255.0f);
	scp.dwFlags = PWS_CHILD | PWS_VISIBLE | PWS_BACKGROUND;
	m_pContentsBar->Create(scp);
	m_pContentsBar->SetNotifyForever(TRUE);
}

void CPWL_Note::SetSubjectName(const CFX_WideString& sName)
{
	CPWL_NoteItem::SetSubjectName(sName);
	RePosChildWnd();
}

void CPWL_Note::SetAuthorName(const CFX_WideString& sName)
{
	if (m_pAuthor)
	{
		m_pAuthor->SetText(sName);
		RePosChildWnd();
	}

	if (IPWL_NoteNotify* pNotify = GetNoteNotify())
	{
		pNotify->OnSetAuthorName(this);
	}	
}

CFX_WideString CPWL_Note::GetAuthorName() const
{
	if (m_pAuthor)
		return m_pAuthor->GetText();

	return L"";
}

FX_BOOL CPWL_Note::OnMouseWheel(short zDelta, const CPDF_Point & point, FX_DWORD nFlag)
{
	CPDF_Point ptScroll = m_pContents->GetScrollPos();
	CPDF_Rect rcScroll = m_pContents->GetScrollArea();
	CPDF_Rect rcContents = m_pContents->GetClientRect();

	if (rcScroll.top - rcScroll.bottom > rcContents.Height())
	{
		CPDF_Point ptNew = ptScroll;

		if (zDelta > 0)
			ptNew.y += 30;
		else
			ptNew.y -= 30;

		if (ptNew.y > rcScroll.top)
			ptNew.y = rcScroll.top;
		if (ptNew.y < rcScroll.bottom + rcContents.Height())
			ptNew.y = rcScroll.bottom + rcContents.Height();
		if (ptNew.y < rcScroll.bottom)
			ptNew.y = rcScroll.bottom;

		if (ptNew.y != ptScroll.y)
		{
			m_pContents->OnNotify(this, PNM_NOTERESET, 0, 0);
			m_pContents->OnNotify(this, PNM_SCROLLWINDOW, SBT_VSCROLL, (FX_INTPTR)&ptNew.y);			
			m_pContentsBar->OnNotify(this, PNM_SETSCROLLPOS, SBT_VSCROLL, (FX_INTPTR)&ptNew.y);

			return TRUE;
		}
	}

	return FALSE;
}

void CPWL_Note::OnNotify(CPWL_Wnd* pWnd, FX_DWORD msg, FX_INTPTR wParam, FX_INTPTR lParam)
{
	switch (msg)
	{
	case PNM_NOTEEDITCHANGED:
		{
			CPDF_Rect rcScroll = m_pContents->GetScrollArea();
			

			PWL_SCROLL_INFO sInfo;
			sInfo.fContentMin = rcScroll.bottom;
			sInfo.fContentMax = rcScroll.top;
			sInfo.fPlateWidth = m_pContents->GetClientRect().Height();
			sInfo.fSmallStep = 13.0f;
			sInfo.fBigStep = sInfo.fPlateWidth;

			if (FXSYS_memcmp(&m_OldScrollInfo, &sInfo, sizeof(PWL_SCROLL_INFO)) != 0)
			{
				FX_BOOL bScrollChanged = FALSE;

				if (lParam < 3) //·ÀÖ¹ËÀÑ­»· mantis:15759
				{
					bScrollChanged = ResetScrollBar();
					if (bScrollChanged)
					{
						lParam++;
						m_pContents->OnNotify(this, PNM_NOTERESET, 0, 0);
						this->OnNotify(this, PNM_NOTEEDITCHANGED, 0, lParam);
					}
				}
				
				if (!bScrollChanged)
				{
					if (m_pContentsBar->IsVisible())
					{
						m_pContentsBar->OnNotify(pWnd, PNM_SETSCROLLINFO, SBT_VSCROLL, (FX_INTPTR)&sInfo);
						m_OldScrollInfo = sInfo;

						CPDF_Point ptScroll = m_pContents->GetScrollPos();
						CPDF_Point ptOld = ptScroll;

						if (ptScroll.y > sInfo.fContentMax)
							ptScroll.y = sInfo.fContentMax;
						if (ptScroll.y < sInfo.fContentMin + sInfo.fPlateWidth)
							ptScroll.y = sInfo.fContentMin + sInfo.fPlateWidth;
						if (ptScroll.y < sInfo.fContentMin)
							ptScroll.y = sInfo.fContentMin;

						if (ptOld.y != ptScroll.y)
						{
							m_pContentsBar->OnNotify(this, PNM_SETSCROLLPOS, SBT_VSCROLL, (FX_INTPTR)&ptScroll.y);
							m_pContentsBar->InvalidateRect(NULL);
							m_pContents->OnNotify(this, PNM_SCROLLWINDOW, SBT_VSCROLL, (FX_INTPTR)&ptScroll.y);
						}
					}
				}
			}
		}

		m_pContents->InvalidateRect(NULL);

		return;
	case PNM_SCROLLWINDOW:
		if (m_pContents)
			m_pContents->OnNotify(pWnd, msg, wParam, lParam);
		return;
	case PNM_SETSCROLLPOS:
		if (m_pContentsBar)
			m_pContentsBar->OnNotify(pWnd,PNM_SETSCROLLPOS,wParam,lParam);
		return;
	}

	if (msg == PNM_SETCARETINFO && IsValid())
	{
		if (PWL_CARET_INFO * pInfo = (PWL_CARET_INFO*)wParam)
		{
			if (m_pContents)
			{
				CPDF_Rect rcClient = m_pContents->GetClientRect();
				if (pInfo->ptHead.y > rcClient.top)
				{
					CPDF_Point pt = m_pContents->OutToIn(pInfo->ptHead);
					m_pContents->OnNotify(this, PNM_SCROLLWINDOW, SBT_VSCROLL, (FX_INTPTR)&pt.y);

					CPDF_Point ptScroll = m_pContents->GetScrollPos();
					m_pContentsBar->OnNotify(this, PNM_SETSCROLLPOS, SBT_VSCROLL, (FX_INTPTR)&ptScroll.y);

					return;
				}
				
				if (pInfo->ptFoot.y < rcClient.bottom)
				{
					CPDF_Point pt = m_pContents->OutToIn(pInfo->ptFoot);
					pt.y += rcClient.Height();
					m_pContents->OnNotify(this, PNM_SCROLLWINDOW, SBT_VSCROLL, (FX_INTPTR)&pt.y);

					CPDF_Point ptScroll = m_pContents->GetScrollPos();
					m_pContentsBar->OnNotify(this, PNM_SETSCROLLPOS, SBT_VSCROLL, (FX_INTPTR)&ptScroll.y);

					return;
				}
			}
		}
	}

	CPWL_NoteItem::OnNotify(pWnd, msg, wParam, lParam);
}

void CPWL_Note::SetBkColor(const CPWL_Color& color)
{
	CPWL_NoteItem::SetBkColor(color);

	CPWL_Color sBK = color;
	CPWL_Color sTextColor;
	if (CPWL_Utils::IsBlackOrWhite(sBK))
		sTextColor = PWL_DEFAULT_WHITECOLOR;
	else
		sTextColor = PWL_DEFAULT_BLACKCOLOR;

	if (m_pCloseBox)
		m_pCloseBox->SetTextColor(sTextColor);
	if (m_pAuthor)
		m_pAuthor->SetTextColor(sTextColor);
	if (m_pOptions)
		m_pOptions->SetTextColor(sTextColor);
	if (m_pLBBox)
		m_pLBBox->SetTextColor(sTextColor);
	if (m_pRBBox)
		m_pRBBox->SetTextColor(sTextColor);
}

FX_BOOL	CPWL_Note::OnLButtonDown(const CPDF_Point& point, FX_DWORD nFlag)
{
	if (m_pOptions->WndHitTest(m_pOptions->ParentToChild(point)))
	{
		if (IPWL_NoteNotify* pNotify = this->GetNoteNotify())
		{
			FX_INT32 x, y;
			PWLtoWnd(point, x, y);
			if (IFX_SystemHandler* pSH = GetSystemHandler())
				pSH->ClientToScreen(GetAttachedHWnd(), x, y);
			this->KillFocus();
			pNotify->OnPopupMenu(x, y);

			return TRUE;
		}
	}

	return CPWL_Wnd::OnLButtonDown(point,nFlag);
}

FX_BOOL	CPWL_Note::OnRButtonUp(const CPDF_Point & point, FX_DWORD nFlag)
{
	return CPWL_Wnd::OnRButtonUp(point,nFlag);
}

const CPWL_Note* CPWL_Note::GetNote() const
{
	return this;
}

IPWL_NoteNotify* CPWL_Note::GetNoteNotify() const
{
	if (m_bEnalbleNotify)
		return m_pNoteNotify;

	return NULL;
}

void CPWL_Note::SetIconType(FX_INT32 nType)
{
	if (m_pIcon)
		m_pIcon->SetIconType(nType);
}

void CPWL_Note::EnableModify(FX_BOOL bEnabled)
{
	m_pContents->EnableModify(bEnabled);
}

void CPWL_Note::EnableRead(FX_BOOL bEnabled)
{
	m_pContents->EnableRead(bEnabled);
}

CFX_WideString CPWL_Note::GetReplyString() const
{
	return m_sReplyString;
}

void CPWL_Note::SetReplyString(const CFX_WideString& string)
{
	m_sReplyString = string;
}

