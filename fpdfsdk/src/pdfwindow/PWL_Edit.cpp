// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/pdfwindow/PDFWindow.h"
#include "../../include/pdfwindow/PWL_Wnd.h"
#include "../../include/pdfwindow/PWL_EditCtrl.h"
#include "../../include/pdfwindow/PWL_Edit.h"
#include "../../include/pdfwindow/PWL_ScrollBar.h"
#include "../../include/pdfwindow/PWL_Utils.h"
#include "../../include/pdfwindow/PWL_Caret.h"
#include "../../include/pdfwindow/PWL_FontMap.h"
#include "../../../core/src/fxcrt/fx_safe_types.h"

/* ---------------------------- CPWL_Edit ------------------------------ */

CPWL_Edit::CPWL_Edit() : m_pFillerNotify(NULL), 
	m_pSpellCheck(NULL),
	m_bFocus(FALSE)
{
	m_pFormFiller = NULL;
}

CPWL_Edit::~CPWL_Edit()
{
	ASSERT(m_bFocus == FALSE);
}

CFX_ByteString CPWL_Edit::GetClassName() const
{
	return PWL_CLASSNAME_EDIT;
}

void CPWL_Edit::OnDestroy()
{
}

void CPWL_Edit::SetText(FX_LPCWSTR csText)
{
	CFX_WideString swText = csText;

	if (HasFlag(PES_RICH))
	{		
		CFX_ByteString sValue = CFX_ByteString::FromUnicode(swText);
		
		if (CXML_Element * pXML = CXML_Element::Parse(sValue.c_str(), sValue.GetLength()))
		{
			FX_INT32 nCount = pXML->CountChildren();
			FX_BOOL bFirst = TRUE;

			swText.Empty();

			for (FX_INT32 i=0; i<nCount; i++)
			{
				if (CXML_Element * pSubElement = pXML->GetElement(i))
				{
					CFX_ByteString tag=pSubElement->GetTagName();
		   			if (tag.EqualNoCase("p"))
					{
						int nChild = pSubElement->CountChildren();
						CFX_WideString swSection;
						for(FX_INT32 j=0; j<nChild; j++)
						{
							swSection += pSubElement->GetContent(j);
						}
						
						if (bFirst)bFirst = FALSE;
						else
							swText += FWL_VKEY_Return;
						swText += swSection;
					}
				}
			}

			delete pXML;
		}
	}	

	m_pEdit->SetText(swText.c_str());
}

void CPWL_Edit::RePosChildWnd()
{
	if (CPWL_ScrollBar * pVSB = this->GetVScrollBar())
	{
		//if (pVSB->IsVisible())
		{
			CPDF_Rect rcWindow = m_rcOldWindow;		
			CPDF_Rect rcVScroll = CPDF_Rect(rcWindow.right,
								rcWindow.bottom,
								rcWindow.right + PWL_SCROLLBAR_WIDTH,
								rcWindow.top);
			pVSB->Move(rcVScroll, TRUE, FALSE);
		}
	}

	if (m_pEditCaret && !HasFlag(PES_TEXTOVERFLOW))
		m_pEditCaret->SetClipRect(CPWL_Utils::InflateRect(GetClientRect(),1.0f)); //+1 for caret beside border

	CPWL_EditCtrl::RePosChildWnd();
}

CPDF_Rect CPWL_Edit::GetClientRect() const
{
	CPDF_Rect rcClient = CPWL_Utils::DeflateRect(GetWindowRect(),(FX_FLOAT)(GetBorderWidth()+GetInnerBorderWidth()));
	
	if (CPWL_ScrollBar * pVSB = this->GetVScrollBar())
	{
		if (pVSB->IsVisible())
		{
			rcClient.right -= PWL_SCROLLBAR_WIDTH;
		}
	}

	return rcClient;
}

void CPWL_Edit::SetAlignFormatH(PWL_EDIT_ALIGNFORMAT_H nFormat, FX_BOOL bPaint/* = TRUE*/)
{
	m_pEdit->SetAlignmentH((FX_INT32)nFormat, bPaint);
}

void CPWL_Edit::SetAlignFormatV(PWL_EDIT_ALIGNFORMAT_V nFormat, FX_BOOL bPaint/* = TRUE*/)
{
	m_pEdit->SetAlignmentV((FX_INT32)nFormat, bPaint);
}

FX_BOOL	CPWL_Edit::CanSelectAll() const
{
	return  GetSelectWordRange() != m_pEdit->GetWholeWordRange();
}

FX_BOOL	CPWL_Edit::CanClear() const
{
	return !IsReadOnly() && m_pEdit->IsSelected();
}

FX_BOOL	CPWL_Edit::CanCopy() const
{
	return 	!HasFlag(PES_PASSWORD) && !HasFlag(PES_NOREAD) && m_pEdit->IsSelected();
}

FX_BOOL	CPWL_Edit::CanCut() const
{
	return 	CanCopy() && !IsReadOnly();
}

FX_BOOL	CPWL_Edit::CanPaste() const
{
	if (IsReadOnly()) return FALSE;

	CFX_WideString swClipboard;
	if (IFX_SystemHandler* pSH = GetSystemHandler())
		swClipboard = pSH->GetClipboardText(GetAttachedHWnd());

	return !swClipboard.IsEmpty();
}

void CPWL_Edit::CopyText()
{
	if (!CanCopy()) return;

	CFX_WideString str = m_pEdit->GetSelText();

	if (IFX_SystemHandler* pSH = GetSystemHandler())
		pSH->SetClipboardText(GetAttachedHWnd(), str);
}

void CPWL_Edit::PasteText()
{
	if (!CanPaste()) return;

	CFX_WideString swClipboard;
	if (IFX_SystemHandler* pSH = GetSystemHandler())
		swClipboard = pSH->GetClipboardText(GetAttachedHWnd());

	if (m_pFillerNotify)
	{
		FX_BOOL bRC = TRUE;
		FX_BOOL bExit = FALSE;
		CFX_WideString strChangeEx;
		int nSelStart = 0;
		int nSelEnd = 0;
		GetSel(nSelStart, nSelEnd);
		m_pFillerNotify->OnBeforeKeyStroke(TRUE, GetAttachedData(), 0 , swClipboard, strChangeEx, nSelStart, nSelEnd, TRUE, bRC, bExit, 0);
		if (!bRC) return;
		if (bExit) return;
	}

	if (swClipboard.GetLength() > 0)
	{
		Clear();
		InsertText(swClipboard.c_str());
	}

	if (m_pFillerNotify)
	{
		FX_BOOL bExit = FALSE;
		m_pFillerNotify->OnAfterKeyStroke(TRUE, GetAttachedData(), bExit,0);
		if (bExit) return;
	}
}

void CPWL_Edit::CutText()
{
	if (!CanCut()) return;

	CFX_WideString str = m_pEdit->GetSelText();

	if (IFX_SystemHandler* pSH = GetSystemHandler())
		pSH->SetClipboardText(GetAttachedHWnd(), str);

	m_pEdit->Clear();
}

void CPWL_Edit::OnCreated()
{
	CPWL_EditCtrl::OnCreated();

	if (CPWL_ScrollBar * pScroll = GetVScrollBar())
	{
		pScroll->RemoveFlag(PWS_AUTOTRANSPARENT);
		pScroll->SetTransparency(255);
	}

	SetParamByFlag();

	m_rcOldWindow = GetWindowRect();

	m_pEdit->SetOprNotify(this);
	m_pEdit->EnableOprNotify(TRUE);
}

void CPWL_Edit::SetParamByFlag()
{	
	if (HasFlag(PES_RIGHT))
	{
		m_pEdit->SetAlignmentH(2, FALSE);
	}
	else if (HasFlag(PES_MIDDLE))
	{
		m_pEdit->SetAlignmentH(1, FALSE);
	}
	else
	{
		m_pEdit->SetAlignmentH(0, FALSE);
	}

	if (HasFlag(PES_BOTTOM))
	{
		m_pEdit->SetAlignmentV(2, FALSE);
	}
	else if (HasFlag(PES_CENTER))
	{
		m_pEdit->SetAlignmentV(1, FALSE);
	}
	else
	{
		m_pEdit->SetAlignmentV(0, FALSE);
	}

	if (HasFlag(PES_PASSWORD))
	{
		m_pEdit->SetPasswordChar('*', FALSE);
	}

	m_pEdit->SetMultiLine(HasFlag(PES_MULTILINE), FALSE);
	m_pEdit->SetAutoReturn(HasFlag(PES_AUTORETURN), FALSE);
	m_pEdit->SetAutoFontSize(HasFlag(PWS_AUTOFONTSIZE), FALSE);
	m_pEdit->SetAutoScroll(HasFlag(PES_AUTOSCROLL), FALSE);
	m_pEdit->EnableUndo(HasFlag(PES_UNDO));

	if (HasFlag(PES_TEXTOVERFLOW))
	{
		SetClipRect(CPDF_Rect(0.0f,0.0f,0.0f,0.0f));
		m_pEdit->SetTextOverflow(TRUE, FALSE);
	}
	else
	{
		if (m_pEditCaret)
		{
			m_pEditCaret->SetClipRect(CPWL_Utils::InflateRect(GetClientRect(),1.0f)); //+1 for caret beside border
		}
	}

	if (HasFlag(PES_SPELLCHECK))
	{
		m_pSpellCheck = GetCreationParam().pSpellCheck;
	}
}

void CPWL_Edit::GetThisAppearanceStream(CFX_ByteTextBuf & sAppStream)
{
	CPWL_Wnd::GetThisAppearanceStream(sAppStream);

	CPDF_Rect rcClient = GetClientRect();
	CFX_ByteTextBuf sLine;

	FX_INT32 nCharArray = m_pEdit->GetCharArray();

	if (nCharArray > 0)
	{
		switch (GetBorderStyle())
		{
		case PBS_SOLID:
			{
				sLine << "q\n" << GetBorderWidth() << " w\n" 
					<< CPWL_Utils::GetColorAppStream(GetBorderColor(),FALSE) << " 2 J 0 j\n";					

				for (FX_INT32 i=1;i<nCharArray;i++)
				{
					sLine << rcClient.left + ((rcClient.right - rcClient.left)/nCharArray)*i << " "
						<< rcClient.bottom << " m\n"
						<< rcClient.left + ((rcClient.right - rcClient.left)/nCharArray)*i << " "
						<< rcClient.top << " l S\n";						
				}

				sLine << "Q\n";					
			}
			break; 
		case PBS_DASH:
			{
				sLine << "q\n" << GetBorderWidth() << " w\n" 
					<< CPWL_Utils::GetColorAppStream(GetBorderColor(),FALSE) << " 2 J 0 j\n"
					<< "[" << GetBorderDash().nDash << " " 
					<< GetBorderDash().nGap << "] " 
					<< GetBorderDash().nPhase << " d\n";

				for (FX_INT32 i=1;i<nCharArray;i++)					
				{
					sLine << rcClient.left + ((rcClient.right - rcClient.left)/nCharArray)*i << " "
						<< rcClient.bottom << " m\n"
						<< rcClient.left + ((rcClient.right - rcClient.left)/nCharArray)*i << " "
						<< rcClient.top << " l S\n";	
				}

				sLine << "Q\n";
			}
			break;
		}		
	}

	sAppStream << sLine;

	CFX_ByteTextBuf sText;

	CPDF_Point ptOffset = CPDF_Point(0.0f,0.0f);

	CPVT_WordRange wrWhole = m_pEdit->GetWholeWordRange();
	CPVT_WordRange wrSelect = GetSelectWordRange();
	CPVT_WordRange wrVisible = (HasFlag(PES_TEXTOVERFLOW) ? wrWhole : m_pEdit->GetVisibleWordRange());
	CPVT_WordRange wrSelBefore(wrWhole.BeginPos,wrSelect.BeginPos);
	CPVT_WordRange wrSelAfter(wrSelect.EndPos,wrWhole.EndPos);

	CPVT_WordRange wrTemp = CPWL_Utils::OverlapWordRange(GetSelectWordRange(),wrVisible);
	CFX_ByteString sEditSel = CPWL_Utils::GetEditSelAppStream(m_pEdit, ptOffset,
			&wrTemp);

	if (sEditSel.GetLength() > 0)
		sText << CPWL_Utils::GetColorAppStream(PWL_DEFAULT_SELBACKCOLOR) << sEditSel ;

	wrTemp = CPWL_Utils::OverlapWordRange(wrVisible,wrSelBefore);
	CFX_ByteString sEditBefore = CPWL_Utils::GetEditAppStream(m_pEdit, ptOffset,  
			&wrTemp, !HasFlag(PES_CHARARRAY), m_pEdit->GetPasswordChar());			

	if (sEditBefore.GetLength() > 0)
		sText << "BT\n" << CPWL_Utils::GetColorAppStream(GetTextColor()) << sEditBefore << "ET\n";

	wrTemp = CPWL_Utils::OverlapWordRange(wrVisible,wrSelect);
	CFX_ByteString sEditMid = CPWL_Utils::GetEditAppStream(m_pEdit, ptOffset, 
			&wrTemp, !HasFlag(PES_CHARARRAY), m_pEdit->GetPasswordChar());			

	if (sEditMid.GetLength() > 0)
		sText << "BT\n" << CPWL_Utils::GetColorAppStream(CPWL_Color(COLORTYPE_GRAY,1)) << sEditMid << "ET\n";

	wrTemp = CPWL_Utils::OverlapWordRange(wrVisible,wrSelAfter);
	CFX_ByteString sEditAfter = CPWL_Utils::GetEditAppStream(m_pEdit, ptOffset, 
			&wrTemp, !HasFlag(PES_CHARARRAY), m_pEdit->GetPasswordChar());			

	if (sEditAfter.GetLength() > 0)
		sText << "BT\n" << CPWL_Utils::GetColorAppStream(GetTextColor()) << sEditAfter<< "ET\n";

	if (HasFlag(PES_SPELLCHECK))
	{
		CFX_ByteString sSpellCheck = CPWL_Utils::GetSpellCheckAppStream(m_pEdit, m_pSpellCheck, ptOffset, &wrVisible);
		if (sSpellCheck.GetLength() > 0)
			sText << CPWL_Utils::GetColorAppStream(CPWL_Color(COLORTYPE_RGB,1,0,0),FALSE) << sSpellCheck;
	}

	if (sText.GetLength() > 0)
	{
		CPDF_Rect rcClient = this->GetClientRect();
		sAppStream << "q\n/Tx BMC\n";
		
		if (!HasFlag(PES_TEXTOVERFLOW))
			sAppStream << rcClient.left << " " << rcClient.bottom << " "
				<< rcClient.right - rcClient.left << " " << rcClient.top - rcClient.bottom << " re W n\n";

		sAppStream << sText;

		sAppStream << "EMC\nQ\n";
	}
}

void CPWL_Edit::DrawThisAppearance(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device)
{
	CPWL_Wnd::DrawThisAppearance(pDevice,pUser2Device);

	CPDF_Rect rcClient = GetClientRect();
	CFX_ByteTextBuf sLine;

	FX_INT32 nCharArray = m_pEdit->GetCharArray();
	FX_SAFE_INT32 nCharArraySafe = nCharArray;
	nCharArraySafe -= 1;
	nCharArraySafe *= 2;

	if (nCharArray > 0 && nCharArraySafe.IsValid())
	{
		switch (GetBorderStyle())
		{
		case PBS_SOLID:
			{
				CFX_GraphStateData gsd;
				gsd.m_LineWidth = (FX_FLOAT)GetBorderWidth();

				CFX_PathData path;
				path.SetPointCount(nCharArraySafe.ValueOrDie());

				for (FX_INT32 i=0; i<nCharArray-1; i++)
				{
					path.SetPoint(i*2, rcClient.left + ((rcClient.right - rcClient.left)/nCharArray)*(i+1),
						rcClient.bottom, FXPT_MOVETO);
					path.SetPoint(i*2+1, rcClient.left + ((rcClient.right - rcClient.left)/nCharArray)*(i+1),
						rcClient.top, FXPT_LINETO);
				}
				if (path.GetPointCount() > 0)
					pDevice->DrawPath(&path, pUser2Device, &gsd,0,  
						CPWL_Utils::PWLColorToFXColor(GetBorderColor(),255), FXFILL_ALTERNATE);
			}
			break; 
		case PBS_DASH:
			{
				CFX_GraphStateData gsd;
				gsd.m_LineWidth = (FX_FLOAT)GetBorderWidth();

				gsd.SetDashCount(2);
				gsd.m_DashArray[0] = (FX_FLOAT)GetBorderDash().nDash;
				gsd.m_DashArray[1] = (FX_FLOAT)GetBorderDash().nGap;
				gsd.m_DashPhase = (FX_FLOAT)GetBorderDash().nPhase;

				CFX_PathData path;
				path.SetPointCount(nCharArraySafe.ValueOrDie());

				for (FX_INT32 i=0; i<nCharArray-1; i++)
				{
					path.SetPoint(i*2, rcClient.left + ((rcClient.right - rcClient.left)/nCharArray)*(i+1),
						rcClient.bottom, FXPT_MOVETO);
					path.SetPoint(i*2+1, rcClient.left + ((rcClient.right - rcClient.left)/nCharArray)*(i+1),
						rcClient.top, FXPT_LINETO);
				}
				if (path.GetPointCount() > 0)
					pDevice->DrawPath(&path, pUser2Device, &gsd,0,
						CPWL_Utils::PWLColorToFXColor(GetBorderColor(),255), FXFILL_ALTERNATE);
			}
			break;
		}		
	}

	CPDF_Rect rcClip;
	CPVT_WordRange wrRange = m_pEdit->GetVisibleWordRange();
	CPVT_WordRange* pRange = NULL;

	if (!HasFlag(PES_TEXTOVERFLOW))
	{
		rcClip = GetClientRect();
		pRange = &wrRange;
	}
IFX_SystemHandler* pSysHandler = GetSystemHandler();
	IFX_Edit::DrawEdit(pDevice,pUser2Device,m_pEdit,
		CPWL_Utils::PWLColorToFXColor(GetTextColor(),this->GetTransparency()),
		CPWL_Utils::PWLColorToFXColor(GetTextStrokeColor(),this->GetTransparency()),
		rcClip,CPDF_Point(0.0f,0.0f),pRange, pSysHandler, m_pFormFiller);

	if (HasFlag(PES_SPELLCHECK))
	{
		CPWL_Utils::DrawEditSpellCheck(pDevice,pUser2Device,m_pEdit,rcClip,
			CPDF_Point(0.0f,0.0f),pRange, this->GetCreationParam().pSpellCheck);
	}
}

FX_BOOL CPWL_Edit::OnLButtonDown(const CPDF_Point & point, FX_DWORD nFlag)
{
	CPWL_Wnd::OnLButtonDown(point,nFlag);

	if (HasFlag(PES_TEXTOVERFLOW) || ClientHitTest(point))
	{
		if (m_bMouseDown)
			this->InvalidateRect();

		m_bMouseDown = TRUE;		
		SetCapture();

		m_pEdit->OnMouseDown(point,IsSHIFTpressed(nFlag),IsCTRLpressed(nFlag));
	}

	return TRUE;
}

FX_BOOL	CPWL_Edit::OnLButtonDblClk(const CPDF_Point & point, FX_DWORD nFlag)
{
	CPWL_Wnd::OnLButtonDblClk(point, nFlag);

	if (HasFlag(PES_TEXTOVERFLOW) || ClientHitTest(point))
	{
		m_pEdit->SelectAll();
	}

	return TRUE;
}

#define WM_PWLEDIT_UNDO					0x01
#define WM_PWLEDIT_REDO					0x02
#define WM_PWLEDIT_CUT					0x03
#define WM_PWLEDIT_COPY					0x04
#define WM_PWLEDIT_PASTE				0x05
#define WM_PWLEDIT_DELETE				0x06
#define WM_PWLEDIT_SELECTALL			0x07
#define WM_PWLEDIT_SUGGEST				0x08

FX_BOOL CPWL_Edit::OnRButtonUp(const CPDF_Point & point, FX_DWORD nFlag)
{
	if (m_bMouseDown) return FALSE;

	CPWL_Wnd::OnRButtonUp(point, nFlag);
	
	if (!HasFlag(PES_TEXTOVERFLOW) && !ClientHitTest(point)) return TRUE;

	IFX_SystemHandler* pSH = GetSystemHandler();
	if (!pSH) return FALSE;

	this->SetFocus();

	CPVT_WordRange wrLatin = GetLatinWordsRange(point);
	CFX_WideString swLatin = m_pEdit->GetRangeText(wrLatin);

	FX_HMENU hPopup = pSH->CreatePopupMenu();
	if (!hPopup) return FALSE;

	CFX_ByteStringArray sSuggestWords;
	CPDF_Point ptPopup = point;

	if (!IsReadOnly())
	{
		if (HasFlag(PES_SPELLCHECK) && !swLatin.IsEmpty())
		{
			if (m_pSpellCheck)
			{
				CFX_ByteString sLatin = CFX_ByteString::FromUnicode(swLatin);

				if (!m_pSpellCheck->CheckWord(sLatin))
				{						
					m_pSpellCheck->SuggestWords(sLatin,sSuggestWords);

					FX_INT32 nSuggest = sSuggestWords.GetSize();

					for (FX_INT32 nWord=0; nWord<nSuggest; nWord++)
					{	
						pSH->AppendMenuItem(hPopup, WM_PWLEDIT_SUGGEST+nWord, sSuggestWords[nWord].UTF8Decode());
					}

					if (nSuggest > 0)
						pSH->AppendMenuItem(hPopup, 0, L"");

					ptPopup = GetWordRightBottomPoint(wrLatin.EndPos);
				}
			}
		}
	}

	IPWL_Provider* pProvider = this->GetProvider();

	if (HasFlag(PES_UNDO))
	{
		pSH->AppendMenuItem(hPopup, WM_PWLEDIT_UNDO, 
			pProvider ? pProvider->LoadPopupMenuString(0) : L"&Undo");
		pSH->AppendMenuItem(hPopup, WM_PWLEDIT_REDO,
			pProvider ? pProvider->LoadPopupMenuString(1) : L"&Redo");
		pSH->AppendMenuItem(hPopup, 0, L"");

		if (!m_pEdit->CanUndo())
			pSH->EnableMenuItem(hPopup, WM_PWLEDIT_UNDO, FALSE);
		if (!m_pEdit->CanRedo())
			pSH->EnableMenuItem(hPopup, WM_PWLEDIT_REDO, FALSE);
	}

	pSH->AppendMenuItem(hPopup, WM_PWLEDIT_CUT, 
		pProvider ? pProvider->LoadPopupMenuString(2) : L"Cu&t");
	pSH->AppendMenuItem(hPopup, WM_PWLEDIT_COPY, 
		pProvider ? pProvider->LoadPopupMenuString(3) : L"&Copy");
	pSH->AppendMenuItem(hPopup, WM_PWLEDIT_PASTE, 
		pProvider ? pProvider->LoadPopupMenuString(4) : L"&Paste");
	pSH->AppendMenuItem(hPopup, WM_PWLEDIT_DELETE, 
		pProvider ? pProvider->LoadPopupMenuString(5) : L"&Delete");

	CFX_WideString swText = pSH->GetClipboardText(this->GetAttachedHWnd());
	if (swText.IsEmpty())
		pSH->EnableMenuItem(hPopup, WM_PWLEDIT_PASTE, FALSE);

	if (!m_pEdit->IsSelected())
	{
		pSH->EnableMenuItem(hPopup, WM_PWLEDIT_CUT, FALSE);
		pSH->EnableMenuItem(hPopup, WM_PWLEDIT_COPY, FALSE);
		pSH->EnableMenuItem(hPopup, WM_PWLEDIT_DELETE, FALSE);
	}

	if (IsReadOnly())
	{
		pSH->EnableMenuItem(hPopup, WM_PWLEDIT_CUT, FALSE);
		pSH->EnableMenuItem(hPopup, WM_PWLEDIT_DELETE, FALSE);
		pSH->EnableMenuItem(hPopup, WM_PWLEDIT_PASTE, FALSE);			
	}

	if (HasFlag(PES_PASSWORD))
	{
		pSH->EnableMenuItem(hPopup, WM_PWLEDIT_CUT, FALSE);
		pSH->EnableMenuItem(hPopup, WM_PWLEDIT_COPY, FALSE);
	}

	if (HasFlag(PES_NOREAD))
	{
		pSH->EnableMenuItem(hPopup, WM_PWLEDIT_CUT, FALSE);
		pSH->EnableMenuItem(hPopup, WM_PWLEDIT_COPY, FALSE);
	}

	pSH->AppendMenuItem(hPopup, 0, L"");
	pSH->AppendMenuItem(hPopup, WM_PWLEDIT_SELECTALL,
		pProvider ? pProvider->LoadPopupMenuString(6) : L"&Select All");

	if (m_pEdit->GetTotalWords() == 0)
	{
		pSH->EnableMenuItem(hPopup, WM_PWLEDIT_SELECTALL, FALSE);
	}

	FX_INT32 x, y;
	PWLtoWnd(ptPopup, x, y);
	pSH->ClientToScreen(GetAttachedHWnd(), x, y);
	pSH->SetCursor(FXCT_ARROW);
	FX_INT32 nCmd = pSH->TrackPopupMenu(hPopup,
					 x,
					 y,
					 GetAttachedHWnd());


	switch (nCmd)
	{
	case WM_PWLEDIT_UNDO:
		Undo();
		break;
	case WM_PWLEDIT_REDO:
		Redo();
		break;
	case WM_PWLEDIT_CUT:
		this->CutText();
		break;
	case WM_PWLEDIT_COPY:
		this->CopyText();
		break;
	case WM_PWLEDIT_PASTE:
		this->PasteText();
		break;
	case WM_PWLEDIT_DELETE:
		this->Clear();
		break;
	case WM_PWLEDIT_SELECTALL:
		this->SelectAll();
		break;
	case WM_PWLEDIT_SUGGEST + 0:
		SetSel(m_pEdit->WordPlaceToWordIndex(wrLatin.BeginPos),m_pEdit->WordPlaceToWordIndex(wrLatin.EndPos));
		ReplaceSel(sSuggestWords[0].UTF8Decode().c_str());
		break;
	case WM_PWLEDIT_SUGGEST + 1:
		SetSel(m_pEdit->WordPlaceToWordIndex(wrLatin.BeginPos),m_pEdit->WordPlaceToWordIndex(wrLatin.EndPos));
		ReplaceSel(sSuggestWords[1].UTF8Decode().c_str());
		break;
	case WM_PWLEDIT_SUGGEST + 2:
		SetSel(m_pEdit->WordPlaceToWordIndex(wrLatin.BeginPos),m_pEdit->WordPlaceToWordIndex(wrLatin.EndPos));
		ReplaceSel(sSuggestWords[2].UTF8Decode().c_str());
		break;
	case WM_PWLEDIT_SUGGEST + 3:
		SetSel(m_pEdit->WordPlaceToWordIndex(wrLatin.BeginPos),m_pEdit->WordPlaceToWordIndex(wrLatin.EndPos));
		ReplaceSel(sSuggestWords[3].UTF8Decode().c_str());
		break;
	case WM_PWLEDIT_SUGGEST + 4:
		SetSel(m_pEdit->WordPlaceToWordIndex(wrLatin.BeginPos),m_pEdit->WordPlaceToWordIndex(wrLatin.EndPos));
		ReplaceSel(sSuggestWords[4].UTF8Decode().c_str());
		break;
	default:
		break;
	}

	pSH->DestroyMenu(hPopup);

	return TRUE;
}

void CPWL_Edit::OnSetFocus()
{
	SetEditCaret(TRUE);

	if (!IsReadOnly())
	{
		if (IPWL_FocusHandler* pFocusHandler = GetFocusHandler())
			pFocusHandler->OnSetFocus(this);
	}

	m_bFocus = TRUE;
}

void CPWL_Edit::OnKillFocus()
{
	ShowVScrollBar(FALSE);
	
	m_pEdit->SelectNone();
	SetCaret(FALSE, CPDF_Point(0.0f,0.0f), CPDF_Point(0.0f,0.0f));
	
	SetCharSet(0);

	if (!IsReadOnly())
	{
		if (IPWL_FocusHandler* pFocusHandler = GetFocusHandler())
			pFocusHandler->OnKillFocus(this);
	}

	m_bFocus = FALSE;
}

void CPWL_Edit::SetHorzScale(FX_INT32 nHorzScale, FX_BOOL bPaint/* = TRUE*/)
{
	m_pEdit->SetHorzScale(nHorzScale, bPaint);
}

void CPWL_Edit::SetCharSpace(FX_FLOAT fCharSpace, FX_BOOL bPaint/* = TRUE*/)
{
	m_pEdit->SetCharSpace(fCharSpace, bPaint);
}

void CPWL_Edit::SetLineLeading(FX_FLOAT fLineLeading, FX_BOOL bPaint/* = TRUE*/)
{
	m_pEdit->SetLineLeading(fLineLeading, bPaint);
}

CFX_ByteString CPWL_Edit::GetSelectAppearanceStream(const CPDF_Point & ptOffset) const
{
	CPVT_WordRange wr = GetSelectWordRange();
	return CPWL_Utils::GetEditSelAppStream(m_pEdit,ptOffset,&wr);
}

CPVT_WordRange CPWL_Edit::GetSelectWordRange() const
{
	if (m_pEdit->IsSelected())
	{
		FX_INT32 nStart = -1;
		FX_INT32 nEnd = -1;

		m_pEdit->GetSel(nStart, nEnd);

		CPVT_WordPlace wpStart = m_pEdit->WordIndexToWordPlace(nStart);
		CPVT_WordPlace wpEnd = m_pEdit->WordIndexToWordPlace(nEnd);

		return CPVT_WordRange(wpStart,wpEnd);
	}

	return CPVT_WordRange();
}

CFX_ByteString CPWL_Edit::GetTextAppearanceStream(const CPDF_Point & ptOffset) const
{
	CFX_ByteTextBuf sRet;
	CFX_ByteString sEdit = CPWL_Utils::GetEditAppStream(m_pEdit,ptOffset);
	
	if (sEdit.GetLength() > 0)
	{
		sRet << "BT\n" << CPWL_Utils::GetColorAppStream(GetTextColor()) << sEdit << "ET\n";
	}

	return sRet.GetByteString();
}

CFX_ByteString CPWL_Edit::GetCaretAppearanceStream(const CPDF_Point & ptOffset) const
{
	if (m_pEditCaret)
		return m_pEditCaret->GetCaretAppearanceStream(ptOffset);

	return CFX_ByteString();
}

CPDF_Point CPWL_Edit::GetWordRightBottomPoint(const CPVT_WordPlace& wpWord)
{
	CPDF_Point pt(0.0f, 0.0f);

	if (IFX_Edit_Iterator * pIterator = m_pEdit->GetIterator())
	{
		CPVT_WordPlace wpOld = pIterator->GetAt();
		pIterator->SetAt(wpWord);
		CPVT_Word word;
		if (pIterator->GetWord(word))
		{
			pt = CPDF_Point(word.ptWord.x + word.fWidth, word.ptWord.y + word.fDescent);
		}

		pIterator->SetAt(wpOld);
	}

	return pt;
}

FX_BOOL	CPWL_Edit::IsTextFull() const
{
	return m_pEdit->IsTextFull();
}

FX_FLOAT CPWL_Edit::GetCharArrayAutoFontSize(CPDF_Font* pFont, const CPDF_Rect& rcPlate, FX_INT32 nCharArray)
{
	if (pFont && !pFont->IsStandardFont())
	{
		FX_RECT rcBBox;
		pFont->GetFontBBox(rcBBox);

		CPDF_Rect rcCell = rcPlate;
		FX_FLOAT xdiv = rcCell.Width() / nCharArray * 1000.0f / rcBBox.Width();
		FX_FLOAT ydiv = - rcCell.Height() * 1000.0f / rcBBox.Height();

		return xdiv < ydiv ? xdiv : ydiv;
	}

	return 0.0f;
}

void CPWL_Edit::SetCharArray(FX_INT32 nCharArray)
{
	if (HasFlag(PES_CHARARRAY) && nCharArray > 0)
	{
		m_pEdit->SetCharArray(nCharArray);	
		m_pEdit->SetTextOverflow(TRUE);

		if (HasFlag(PWS_AUTOFONTSIZE))
		{
			if (IFX_Edit_FontMap* pFontMap = this->GetFontMap())
			{
				FX_FLOAT fFontSize = GetCharArrayAutoFontSize(pFontMap->GetPDFFont(0), GetClientRect(), nCharArray);
				if (fFontSize > 0.0f)
				{
					m_pEdit->SetAutoFontSize(FALSE);
					m_pEdit->SetFontSize(fFontSize);
				}
			}
		}
	}
}

void CPWL_Edit::SetLimitChar(FX_INT32 nLimitChar)
{
	m_pEdit->SetLimitChar(nLimitChar);
}

void CPWL_Edit::ReplaceSel(FX_LPCWSTR csText)
{
	m_pEdit->Clear();
	m_pEdit->InsertText(csText);
}

CPDF_Rect CPWL_Edit::GetFocusRect() const
{
	return CPDF_Rect();
}

void CPWL_Edit::ShowVScrollBar(FX_BOOL bShow)
{
	if (CPWL_ScrollBar * pScroll = GetVScrollBar())
	{
		if (bShow)
		{
			if (!pScroll->IsVisible())
			{
				pScroll->SetVisible(TRUE);
				CPDF_Rect rcWindow = GetWindowRect();
				m_rcOldWindow = rcWindow;
				rcWindow.right += PWL_SCROLLBAR_WIDTH;			
				Move(rcWindow, TRUE, TRUE);
			}
		}
		else
		{
			if (pScroll->IsVisible())
			{
				pScroll->SetVisible(FALSE);
				Move(m_rcOldWindow, TRUE, TRUE);
			}	
		}
	}
}

FX_BOOL	CPWL_Edit::IsVScrollBarVisible() const
{
	if (CPWL_ScrollBar * pScroll = GetVScrollBar())
	{
		return pScroll->IsVisible();
	}

	return FALSE;
}

void CPWL_Edit::EnableSpellCheck(FX_BOOL bEnabled)
{
	if (bEnabled)
		AddFlag(PES_SPELLCHECK);
	else
		RemoveFlag(PES_SPELLCHECK);
}

FX_BOOL CPWL_Edit::OnKeyDown(FX_WORD nChar, FX_DWORD nFlag)
{
	if (m_bMouseDown) return TRUE;

	if (nChar == FWL_VKEY_Delete)
	{
		if (m_pFillerNotify)
		{
			FX_BOOL bRC = TRUE;
			FX_BOOL bExit = FALSE;
			CFX_WideString strChange;
			CFX_WideString strChangeEx;

			int nSelStart = 0;
			int nSelEnd = 0;
			GetSel(nSelStart, nSelEnd);

			if (nSelStart == nSelEnd)
				nSelEnd = nSelStart + 1;
			m_pFillerNotify->OnBeforeKeyStroke(TRUE, GetAttachedData(), FWL_VKEY_Delete, strChange, strChangeEx, nSelStart, nSelEnd, TRUE, bRC, bExit, nFlag);
			if (!bRC) return FALSE;				
			if (bExit) return FALSE;
		}
	}

	FX_BOOL bRet = CPWL_EditCtrl::OnKeyDown(nChar,  nFlag);

	if (nChar == FWL_VKEY_Delete)
	{
		if (m_pFillerNotify)
		{
			FX_BOOL bExit = FALSE;
			m_pFillerNotify->OnAfterKeyStroke(TRUE, GetAttachedData(), bExit,nFlag);
			if (bExit) return FALSE;
		}
	}

	//In case of implementation swallow the OnKeyDown event.
	if(IsProceedtoOnChar(nChar, nFlag))
			return TRUE;

	return bRet;
}

/**
*In case of implementation swallow the OnKeyDown event. 
*If the event is swallowed, implementation may do other unexpected things, which is not the control means to do.
*/
FX_BOOL CPWL_Edit::IsProceedtoOnChar(FX_WORD nKeyCode, FX_DWORD nFlag)
{

	FX_BOOL bCtrl = IsCTRLpressed(nFlag);
	FX_BOOL bAlt = IsALTpressed(nFlag);
	if(bCtrl && !bAlt)
	{
	//hot keys for edit control.	
		switch(nKeyCode)
		{
		case 'C':
		case 'V':
		case 'X':
		case 'A':
		case 'Z':
			return TRUE;
		default:
			break;
		}
	}
	//control characters.
	switch(nKeyCode)
	{
	case FWL_VKEY_Escape:
	case FWL_VKEY_Back:
	case FWL_VKEY_Return:
	case FWL_VKEY_Space:
		return TRUE;
	default:
		break;
	}
	return FALSE;

}

FX_BOOL CPWL_Edit::OnChar(FX_WORD nChar, FX_DWORD nFlag)
{
	if (m_bMouseDown) return TRUE;

	FX_BOOL bRC = TRUE;
	FX_BOOL bExit = FALSE;

	FX_BOOL bCtrl = IsCTRLpressed(nFlag);
	if (!bCtrl)
	{
		if (m_pFillerNotify)
		{
			CFX_WideString swChange;
			FX_INT32 nKeyCode;

			int nSelStart = 0;
			int nSelEnd = 0;
			GetSel(nSelStart, nSelEnd);

			switch (nChar)
			{
			case FWL_VKEY_Back:
				nKeyCode = nChar;
				if (nSelStart == nSelEnd)
					nSelStart = nSelEnd - 1;
				break;
			case FWL_VKEY_Return:
				nKeyCode = nChar;
				break;
			default:
				nKeyCode = 0;
				swChange += nChar;
				break;
			}
		
			CFX_WideString strChangeEx;
			m_pFillerNotify->OnBeforeKeyStroke(TRUE, GetAttachedData(), nKeyCode, swChange, strChangeEx, nSelStart, nSelEnd, TRUE, bRC, bExit, nFlag);
		}
	}

	if (!bRC) return TRUE;
	if (bExit) return FALSE;

	if (IFX_Edit_FontMap * pFontMap = GetFontMap())
	{
		FX_INT32 nOldCharSet = GetCharSet();
		FX_INT32 nNewCharSet = pFontMap->CharSetFromUnicode(nChar, DEFAULT_CHARSET);
		if(nOldCharSet != nNewCharSet)
		{
			SetCharSet(nNewCharSet);
		}
	}
	FX_BOOL bRet = CPWL_EditCtrl::OnChar(nChar,nFlag);

	if (!bCtrl)
	{
		if (m_pFillerNotify)
		{
			m_pFillerNotify->OnAfterKeyStroke(TRUE, GetAttachedData(), bExit,nFlag);
			if (bExit) return FALSE;
		}
	}

	return bRet;
}

FX_BOOL	CPWL_Edit::OnMouseWheel(short zDelta, const CPDF_Point & point, FX_DWORD nFlag)
{
	if (HasFlag(PES_MULTILINE))
	{
		CPDF_Point ptScroll = GetScrollPos();

		if (zDelta > 0)
		{
			ptScroll.y += this->GetFontSize();
		}
		else
		{
			ptScroll.y -= this->GetFontSize();
		}
		this->SetScrollPos(ptScroll);

		return TRUE;
	}

	return FALSE;
}

void CPWL_Edit::OnInsertReturn(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace)
{
	if (HasFlag(PES_SPELLCHECK))
	{
		m_pEdit->RefreshWordRange(CombineWordRange(GetLatinWordsRange(oldplace),GetLatinWordsRange(place)));
	}

	if (m_pEditNotify)
	{
		m_pEditNotify->OnInsertReturn(place, oldplace);
	}
}

void CPWL_Edit::OnBackSpace(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace)
{
	if (HasFlag(PES_SPELLCHECK))
	{
		m_pEdit->RefreshWordRange(CombineWordRange(GetLatinWordsRange(oldplace),GetLatinWordsRange(place)));
	}

	if (m_pEditNotify)
	{
		m_pEditNotify->OnBackSpace(place, oldplace);
	}
}

void CPWL_Edit::OnDelete(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace)
{
	if (HasFlag(PES_SPELLCHECK))
	{
		m_pEdit->RefreshWordRange(CombineWordRange(GetLatinWordsRange(oldplace),GetLatinWordsRange(place)));
	}

	if (m_pEditNotify)
	{
		m_pEditNotify->OnDelete(place, oldplace);
	}
}

void CPWL_Edit::OnClear(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace)
{
	if (HasFlag(PES_SPELLCHECK))
	{
		m_pEdit->RefreshWordRange(CombineWordRange(GetLatinWordsRange(oldplace),GetLatinWordsRange(place)));
	}

	if (m_pEditNotify)
	{
		m_pEditNotify->OnClear(place, oldplace);
	}
}

void CPWL_Edit::OnInsertWord(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace)
{
	if (HasFlag(PES_SPELLCHECK))
	{
		m_pEdit->RefreshWordRange(CombineWordRange(GetLatinWordsRange(oldplace),GetLatinWordsRange(place)));
	}

	if (m_pEditNotify)
	{
		m_pEditNotify->OnInsertWord(place, oldplace);
	}
}

void CPWL_Edit::OnSetText(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace)
{
}

void CPWL_Edit::OnInsertText(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace)
{
	if (HasFlag(PES_SPELLCHECK))
	{
		m_pEdit->RefreshWordRange(CombineWordRange(GetLatinWordsRange(oldplace),GetLatinWordsRange(place)));
	}

	if (m_pEditNotify)
	{
		m_pEditNotify->OnInsertText(place, oldplace);
	}
}

void CPWL_Edit::OnAddUndo(IFX_Edit_UndoItem* pUndoItem)
{
	if (m_pEditNotify)
	{
		m_pEditNotify->OnAddUndo(this);
	}
}

CPVT_WordRange CPWL_Edit::CombineWordRange(const CPVT_WordRange& wr1, const CPVT_WordRange& wr2)
{
	CPVT_WordRange wrRet;

	if (wr1.BeginPos.WordCmp(wr2.BeginPos) < 0)
	{
		wrRet.BeginPos = wr1.BeginPos;
	}
	else
	{
		wrRet.BeginPos = wr2.BeginPos;
	}

	if (wr1.EndPos.WordCmp(wr2.EndPos) < 0)
	{
		wrRet.EndPos = wr2.EndPos;
	}
	else
	{
		wrRet.EndPos = wr1.EndPos;
	}

	return wrRet;
}

CPVT_WordRange CPWL_Edit::GetLatinWordsRange(const CPDF_Point& point) const
{
	return GetSameWordsRange(m_pEdit->SearchWordPlace(point), TRUE, FALSE);
}

CPVT_WordRange CPWL_Edit::GetLatinWordsRange(const CPVT_WordPlace & place) const
{
	return GetSameWordsRange(place, TRUE, FALSE);
}

CPVT_WordRange CPWL_Edit::GetArabicWordsRange(const CPVT_WordPlace & place) const
{
	return GetSameWordsRange(place, FALSE, TRUE);
}

#define PWL_ISARABICWORD(word) ((word >= 0x0600 && word <= 0x06FF) || (word >= 0xFB50 && word <= 0xFEFC))

CPVT_WordRange CPWL_Edit::GetSameWordsRange(const CPVT_WordPlace & place, FX_BOOL bLatin, FX_BOOL bArabic) const
{
	CPVT_WordRange range;

	if (IFX_Edit_Iterator* pIterator = m_pEdit->GetIterator())
	{
		CPVT_Word wordinfo;	
		CPVT_WordPlace wpStart(place),wpEnd(place);			
		pIterator->SetAt(place);			
		
		if (bLatin)
		{
			while (pIterator->NextWord())
			{
				if (pIterator->GetWord(wordinfo) && FX_EDIT_ISLATINWORD(wordinfo.Word))
				{
					wpEnd = pIterator->GetAt();
					continue;
				}
				else 
					break;
			};
		}
		else if (bArabic)
		{
			while (pIterator->NextWord())
			{
				if (pIterator->GetWord(wordinfo) && PWL_ISARABICWORD(wordinfo.Word))
				{
					wpEnd = pIterator->GetAt();
					continue;
				}
				else 
					break;
			};
		}

		pIterator->SetAt(place);

		if (bLatin)
		{
			do
			{
				if (pIterator->GetWord(wordinfo) && FX_EDIT_ISLATINWORD(wordinfo.Word))
				{					
					continue;
				}
				else
				{
					wpStart = pIterator->GetAt();
					break;
				}
			}
			while (pIterator->PrevWord());
		}
		else if (bArabic)
		{
			do
			{
				if (pIterator->GetWord(wordinfo) && PWL_ISARABICWORD(wordinfo.Word))
				{					
					continue;
				}
				else
				{
					wpStart = pIterator->GetAt();
					break;
				}
			}
			while (pIterator->PrevWord());
		}

		range.Set(wpStart,wpEnd);
	}	

	return range;
}

void CPWL_Edit::AjustArabicWords(const CPVT_WordRange& wr)
{
}

void CPWL_Edit::GeneratePageObjects(CPDF_PageObjects* pPageObjects, 
										const CPDF_Point& ptOffset, CFX_ArrayTemplate<CPDF_TextObject*>& ObjArray)
{
	IFX_Edit::GeneratePageObjects(pPageObjects, m_pEdit, ptOffset, NULL, CPWL_Utils::PWLColorToFXColor(GetTextColor(),GetTransparency()), ObjArray);
}

void CPWL_Edit::GeneratePageObjects(CPDF_PageObjects* pPageObjects, 
									const CPDF_Point& ptOffset)
{
	CFX_ArrayTemplate<CPDF_TextObject*> ObjArray;
	IFX_Edit::GeneratePageObjects(pPageObjects, m_pEdit, ptOffset, NULL, CPWL_Utils::PWLColorToFXColor(GetTextColor(),GetTransparency()), ObjArray);
}

