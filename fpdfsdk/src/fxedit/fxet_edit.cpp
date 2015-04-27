// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fxedit/fxet_stub.h"
#include "../../include/fxedit/fxet_edit.h"

#define FX_EDIT_UNDO_MAXITEM				10000

/* ---------------------------- CFX_Edit_Iterator ---------------------------- */

CFX_Edit_Iterator::CFX_Edit_Iterator(CFX_Edit * pEdit,IPDF_VariableText_Iterator * pVTIterator) : 
	m_pEdit(pEdit),
	m_pVTIterator(pVTIterator)
{
}

CFX_Edit_Iterator::~CFX_Edit_Iterator()
{
}

FX_BOOL	CFX_Edit_Iterator::NextWord()
{
	ASSERT(m_pVTIterator != NULL);
	
	return m_pVTIterator->NextWord();
}

FX_BOOL CFX_Edit_Iterator::NextLine()
{
	ASSERT(m_pVTIterator != NULL);

	return m_pVTIterator->NextLine();
}

FX_BOOL CFX_Edit_Iterator::NextSection()
{
	ASSERT(m_pVTIterator != NULL);

	return m_pVTIterator->NextSection();
}

FX_BOOL	CFX_Edit_Iterator::PrevWord()
{
	ASSERT(m_pVTIterator != NULL);
	
	return m_pVTIterator->PrevWord();
}

FX_BOOL	CFX_Edit_Iterator::PrevLine()
{
	ASSERT(m_pVTIterator != NULL);

	return m_pVTIterator->PrevLine();
}

FX_BOOL	CFX_Edit_Iterator::PrevSection()
{
	ASSERT(m_pVTIterator != NULL);

	return m_pVTIterator->PrevSection();
}

FX_BOOL CFX_Edit_Iterator::GetWord(CPVT_Word & word) const
{
	ASSERT(m_pEdit != NULL);
	ASSERT(m_pVTIterator != NULL);

	if (m_pVTIterator->GetWord(word))
	{
		word.ptWord = m_pEdit->VTToEdit(word.ptWord);
		return TRUE;
	}

	return FALSE;
}

FX_BOOL CFX_Edit_Iterator::GetLine(CPVT_Line & line) const
{
	ASSERT(m_pEdit != NULL);
	ASSERT(m_pVTIterator != NULL);

	if (m_pVTIterator->GetLine(line))
	{		
		line.ptLine = m_pEdit->VTToEdit(line.ptLine);
		return TRUE;
	}

	return FALSE;
}

FX_BOOL CFX_Edit_Iterator::GetSection(CPVT_Section & section) const
{
	ASSERT(m_pEdit != NULL);
	ASSERT(m_pVTIterator != NULL);

	if (m_pVTIterator->GetSection(section))
	{
		section.rcSection = m_pEdit->VTToEdit(section.rcSection);
		return TRUE;
	}

	return FALSE;
}

void CFX_Edit_Iterator::SetAt(FX_INT32 nWordIndex)
{
	ASSERT(m_pVTIterator != NULL);

	m_pVTIterator->SetAt(nWordIndex);
}

void CFX_Edit_Iterator::SetAt(const CPVT_WordPlace & place)
{
	ASSERT(m_pVTIterator != NULL);

	m_pVTIterator->SetAt(place);
}

const CPVT_WordPlace & CFX_Edit_Iterator::GetAt() const
{
	ASSERT(m_pVTIterator != NULL);

	return m_pVTIterator->GetAt();
}

IFX_Edit* CFX_Edit_Iterator::GetEdit() const
{
	return m_pEdit;
}

/* --------------------------- CFX_Edit_Provider ------------------------------- */

CFX_Edit_Provider::CFX_Edit_Provider(IFX_Edit_FontMap * pFontMap) : m_pFontMap(pFontMap)
{
	ASSERT(m_pFontMap != NULL);
}

CFX_Edit_Provider::~CFX_Edit_Provider()
{
}

IFX_Edit_FontMap* CFX_Edit_Provider::GetFontMap()
{
	return m_pFontMap;
}

FX_INT32 CFX_Edit_Provider::GetCharWidth(FX_INT32 nFontIndex, FX_WORD word, FX_INT32 nWordStyle)
{
	if (CPDF_Font* pPDFFont = m_pFontMap->GetPDFFont(nFontIndex))
	{
		FX_DWORD charcode = word;

		if (pPDFFont->IsUnicodeCompatible())		
			charcode = pPDFFont->CharCodeFromUnicode(word);	
		else
			charcode = m_pFontMap->CharCodeFromUnicode(nFontIndex, word);

		if (charcode != -1)			
			return pPDFFont->GetCharWidthF(charcode);
	}

	return 0;
}

FX_INT32 CFX_Edit_Provider::GetTypeAscent(FX_INT32 nFontIndex)
{
	if (CPDF_Font* pPDFFont = m_pFontMap->GetPDFFont(nFontIndex))
		return pPDFFont->GetTypeAscent();

	return 0;
}

FX_INT32 CFX_Edit_Provider::GetTypeDescent(FX_INT32 nFontIndex)
{
	if (CPDF_Font* pPDFFont = m_pFontMap->GetPDFFont(nFontIndex))
		return pPDFFont->GetTypeDescent();

	return 0;
}

FX_INT32 CFX_Edit_Provider::GetWordFontIndex(FX_WORD word, FX_INT32 charset, FX_INT32 nFontIndex)
{
	return m_pFontMap->GetWordFontIndex(word,charset,nFontIndex);
}

FX_INT32 CFX_Edit_Provider::GetDefaultFontIndex()
{
	return 0;
}

FX_BOOL	CFX_Edit_Provider::IsLatinWord(FX_WORD word)
{
	return FX_EDIT_ISLATINWORD(word);
}

/* --------------------------------- CFX_Edit_Refresh --------------------------------- */

CFX_Edit_Refresh::CFX_Edit_Refresh()
{
}

CFX_Edit_Refresh::~CFX_Edit_Refresh()
{
}

void CFX_Edit_Refresh::BeginRefresh()
{
	m_RefreshRects.Empty();
	m_OldLineRects = m_NewLineRects;
}

void CFX_Edit_Refresh::Push(const CPVT_WordRange & linerange,const CPDF_Rect & rect)
{
	m_NewLineRects.Add(linerange,rect);
}

void CFX_Edit_Refresh::NoAnalyse()
{
	{
		for (FX_INT32 i = 0, sz = m_OldLineRects.GetSize(); i < sz; i++)
			if (CFX_Edit_LineRect * pOldRect = m_OldLineRects.GetAt(i))
				m_RefreshRects.Add(pOldRect->m_rcLine);
	}

	{
		for (FX_INT32 i = 0, sz = m_NewLineRects.GetSize(); i < sz; i++)
			if (CFX_Edit_LineRect * pNewRect = m_NewLineRects.GetAt(i))
				m_RefreshRects.Add(pNewRect->m_rcLine);
	}
}

void CFX_Edit_Refresh::Analyse(FX_INT32 nAlignment)
{
	FX_BOOL bLineTopChanged = FALSE;
	CPDF_Rect rcResult;
	FX_FLOAT fWidthDiff;

	FX_INT32 szMax = FX_EDIT_MAX(m_OldLineRects.GetSize(),m_NewLineRects.GetSize());
	FX_INT32 i = 0;

	while (i < szMax)
	{
		CFX_Edit_LineRect * pOldRect = m_OldLineRects.GetAt(i);
		CFX_Edit_LineRect * pNewRect = m_NewLineRects.GetAt(i);

		if (pOldRect)
		{
			if (pNewRect)
			{
				if (bLineTopChanged)
				{
					rcResult = pOldRect->m_rcLine;
					rcResult.Union(pNewRect->m_rcLine);
					m_RefreshRects.Add(rcResult);
				}
				else
				{
					if (*pNewRect != *pOldRect)
					{
						if (!pNewRect->IsSameTop(*pOldRect) || !pNewRect->IsSameHeight(*pOldRect))
						{
							bLineTopChanged = TRUE;
							continue;
						}

						if (nAlignment == 0)
						{
							if (pNewRect->m_wrLine.BeginPos != pOldRect->m_wrLine.BeginPos)
							{
								rcResult = pOldRect->m_rcLine;
								rcResult.Union(pNewRect->m_rcLine);
								m_RefreshRects.Add(rcResult);	
							}
							else
							{
								if (!pNewRect->IsSameLeft(*pOldRect)) 
								{
									rcResult = pOldRect->m_rcLine;
									rcResult.Union(pNewRect->m_rcLine);										
								}
								else
								{
									fWidthDiff = pNewRect->m_rcLine.Width() - pOldRect->m_rcLine.Width();
									rcResult = pNewRect->m_rcLine;
									if (fWidthDiff > 0.0f)
										rcResult.left = rcResult.right - fWidthDiff;
									else
									{
										rcResult.left = rcResult.right;
										rcResult.right += (-fWidthDiff);
									}
								}
								m_RefreshRects.Add(rcResult);
							}
						}
						else
						{
							rcResult = pOldRect->m_rcLine;
							rcResult.Union(pNewRect->m_rcLine);
							m_RefreshRects.Add(rcResult);
						}
					}
					else
					{
						//don't need to do anything
					}
				}
			}
			else
			{
				m_RefreshRects.Add(pOldRect->m_rcLine);
			}
		}
		else
		{
			if (pNewRect)
			{
				m_RefreshRects.Add(pNewRect->m_rcLine);
			}
			else
			{
				//error
			}
		}
		i++;
	}
}

void CFX_Edit_Refresh::AddRefresh(const CPDF_Rect & rect)
{
	m_RefreshRects.Add(rect);
}

const CFX_Edit_RectArray * CFX_Edit_Refresh::GetRefreshRects() const
{
	return &m_RefreshRects;
}

void CFX_Edit_Refresh::EndRefresh()
{
	m_RefreshRects.Empty();
}

/* ------------------------------------- CFX_Edit_Undo ------------------------------------- */

CFX_Edit_Undo::CFX_Edit_Undo(FX_INT32 nBufsize) : m_nCurUndoPos(0),
	m_nBufSize(nBufsize),
	m_bModified(FALSE),
	m_bVirgin(TRUE),
	m_bWorking(FALSE)
{
}

CFX_Edit_Undo::~CFX_Edit_Undo()
{
	Reset();
}

FX_BOOL CFX_Edit_Undo::CanUndo() const
{
	return m_nCurUndoPos > 0;
}

void CFX_Edit_Undo::Undo()
{
	m_bWorking = TRUE;

	if (m_nCurUndoPos > 0)
	{
		IFX_Edit_UndoItem * pItem = m_UndoItemStack.GetAt(m_nCurUndoPos-1);
		ASSERT(pItem != NULL);

		pItem->Undo();
	
		m_nCurUndoPos--;
		m_bModified = (m_nCurUndoPos != 0);		
	}

	m_bWorking = FALSE;
}

FX_BOOL	CFX_Edit_Undo::CanRedo() const
{
	return m_nCurUndoPos < m_UndoItemStack.GetSize();
}

void CFX_Edit_Undo::Redo()
{
	m_bWorking = TRUE;

	FX_INT32 nStackSize = m_UndoItemStack.GetSize();

	if (m_nCurUndoPos < nStackSize)
	{
		IFX_Edit_UndoItem * pItem = m_UndoItemStack.GetAt(m_nCurUndoPos);
		ASSERT(pItem != NULL);

		pItem->Redo();

		m_nCurUndoPos++;
		m_bModified = (m_nCurUndoPos != 0);		
	}

	m_bWorking = FALSE;
}

FX_BOOL	CFX_Edit_Undo::IsWorking() const
{
	return m_bWorking;
}

void CFX_Edit_Undo::AddItem(IFX_Edit_UndoItem* pItem)
{
	ASSERT(!m_bWorking);
	ASSERT(pItem != NULL);
	ASSERT(m_nBufSize > 1);
	
	if (m_nCurUndoPos < m_UndoItemStack.GetSize())
		RemoveTails();

	if (m_UndoItemStack.GetSize() >= m_nBufSize)
	{
		RemoveHeads();	
		m_bVirgin = FALSE;
	}

	m_UndoItemStack.Add(pItem);	
	m_nCurUndoPos = m_UndoItemStack.GetSize();

	m_bModified = (m_nCurUndoPos != 0);
}

FX_BOOL	CFX_Edit_Undo::IsModified() const
{
	if (m_bVirgin)
		return m_bModified;
	else
		return TRUE;
}

IFX_Edit_UndoItem* CFX_Edit_Undo::GetItem(FX_INT32 nIndex)
{
	if (nIndex>=0 && nIndex < m_UndoItemStack.GetSize())
		return m_UndoItemStack.GetAt(nIndex);

	return NULL;
}

void CFX_Edit_Undo::RemoveHeads()
{
	ASSERT(m_UndoItemStack.GetSize() > 1);

	delete m_UndoItemStack.GetAt(0);
	m_UndoItemStack.RemoveAt(0);
}

void CFX_Edit_Undo::RemoveTails()
{
	for (FX_INT32 i = m_UndoItemStack.GetSize()-1; i >= m_nCurUndoPos; i--)
	{
		delete m_UndoItemStack.GetAt(i);
		m_UndoItemStack.RemoveAt(i);
	}
}

void CFX_Edit_Undo::Reset()
{
	for (FX_INT32 i=0, sz=m_UndoItemStack.GetSize(); i < sz; i++)
	{
		delete m_UndoItemStack.GetAt(i);
	}
	m_nCurUndoPos = 0;
	m_UndoItemStack.RemoveAll();
}

/* -------------------------------- CFX_Edit_GroupUndoItem -------------------------------- */

CFX_Edit_GroupUndoItem::CFX_Edit_GroupUndoItem(const CFX_WideString& sTitle) : m_sTitle(sTitle)
{
}

CFX_Edit_GroupUndoItem::~CFX_Edit_GroupUndoItem()
{
	for (int i=0,sz=m_Items.GetSize(); i<sz; i++)
	{
		delete m_Items[i];
	}

	m_Items.RemoveAll();
}

void CFX_Edit_GroupUndoItem::AddUndoItem(CFX_Edit_UndoItem* pUndoItem)
{
	ASSERT(pUndoItem != NULL);

	pUndoItem->SetFirst(FALSE);
	pUndoItem->SetLast(FALSE);

	m_Items.Add(pUndoItem);

	if (m_sTitle.IsEmpty())
		m_sTitle = pUndoItem->GetUndoTitle();
}

void CFX_Edit_GroupUndoItem::UpdateItems()
{
	if (m_Items.GetSize() > 0)
	{
		CFX_Edit_UndoItem* pFirstItem = m_Items[0];
		ASSERT(pFirstItem != NULL);
		pFirstItem->SetFirst(TRUE);

		CFX_Edit_UndoItem* pLastItem = m_Items[m_Items.GetSize() - 1];
		ASSERT(pLastItem != NULL);
		pLastItem->SetLast(TRUE);
	}
}

void CFX_Edit_GroupUndoItem::Undo()
{
	for (int i=m_Items.GetSize()-1; i>=0; i--)
	{
		CFX_Edit_UndoItem* pUndoItem = m_Items[i];
		ASSERT(pUndoItem != NULL);

		pUndoItem->Undo();
	}
}

void CFX_Edit_GroupUndoItem::Redo()
{
	for (int i=0,sz=m_Items.GetSize(); i<sz; i++)
	{
		CFX_Edit_UndoItem* pUndoItem = m_Items[i];
		ASSERT(pUndoItem != NULL);

		pUndoItem->Redo();
	}
}

CFX_WideString CFX_Edit_GroupUndoItem::GetUndoTitle()
{
	return m_sTitle;
}

/* ------------------------------------- CFX_Edit_UndoItem derived classes ------------------------------------- */

CFXEU_InsertWord::CFXEU_InsertWord(CFX_Edit * pEdit, const CPVT_WordPlace & wpOldPlace, const CPVT_WordPlace & wpNewPlace,
								 FX_WORD word, FX_INT32 charset, const CPVT_WordProps * pWordProps) 
	: m_pEdit(pEdit), m_wpOld(wpOldPlace), m_wpNew(wpNewPlace), m_Word(word), m_nCharset(charset), m_WordProps()
{
	if (pWordProps)
		m_WordProps = *pWordProps;
}

CFXEU_InsertWord::~CFXEU_InsertWord()
{
}

void CFXEU_InsertWord::Redo()
{
	if (m_pEdit)
	{
		m_pEdit->SelectNone();
		m_pEdit->SetCaret(m_wpOld);		
		m_pEdit->InsertWord(m_Word,m_nCharset,&m_WordProps,FALSE,TRUE);
	}
}

void CFXEU_InsertWord::Undo()
{
	if (m_pEdit)
	{
		m_pEdit->SelectNone();
		m_pEdit->SetCaret(m_wpNew);
		m_pEdit->Backspace(FALSE,TRUE);
	}
}

/* -------------------------------------------------------------------------- */

CFXEU_InsertReturn::CFXEU_InsertReturn(CFX_Edit * pEdit, const CPVT_WordPlace & wpOldPlace, const CPVT_WordPlace & wpNewPlace,
			 const CPVT_SecProps * pSecProps, const CPVT_WordProps * pWordProps) :
			m_pEdit(pEdit),
			m_wpOld(wpOldPlace),
			m_wpNew(wpNewPlace),
			m_SecProps(),
			m_WordProps()									 
{
	if (pSecProps)
		m_SecProps = *pSecProps;
	if (pWordProps)
		m_WordProps = *pWordProps;
}

CFXEU_InsertReturn::~CFXEU_InsertReturn()
{
}

void CFXEU_InsertReturn::Redo()
{
	if (m_pEdit)
	{
		m_pEdit->SelectNone();
		m_pEdit->SetCaret(m_wpOld);
		m_pEdit->InsertReturn(&m_SecProps,&m_WordProps,FALSE,TRUE);
	}
}

void CFXEU_InsertReturn::Undo()
{
	if (m_pEdit)
	{
		m_pEdit->SelectNone();
		m_pEdit->SetCaret(m_wpNew);
		m_pEdit->Backspace(FALSE,TRUE);
	}
}

/* -------------------------------------------------------------------------- */
//CFXEU_Backspace

CFXEU_Backspace::CFXEU_Backspace(CFX_Edit * pEdit, const CPVT_WordPlace & wpOldPlace, const CPVT_WordPlace & wpNewPlace,
							   FX_WORD word, FX_INT32 charset,
							   const CPVT_SecProps & SecProps, const CPVT_WordProps & WordProps) :
			m_pEdit(pEdit),
			m_wpOld(wpOldPlace),
			m_wpNew(wpNewPlace),
			m_Word(word),
			m_nCharset(charset),
			m_SecProps(SecProps),
			m_WordProps(WordProps)									 
{
}

CFXEU_Backspace::~CFXEU_Backspace()
{
}

void CFXEU_Backspace::Redo()
{
	if (m_pEdit)
	{
		m_pEdit->SelectNone();
		m_pEdit->SetCaret(m_wpOld);
		m_pEdit->Backspace(FALSE,TRUE);
	}
}

void CFXEU_Backspace::Undo()
{
	if (m_pEdit)
	{
		m_pEdit->SelectNone();
		m_pEdit->SetCaret(m_wpNew);
		if (m_wpNew.SecCmp(m_wpOld) != 0)
		{
			m_pEdit->InsertReturn(&m_SecProps,&m_WordProps,FALSE,TRUE);
		}
		else
		{
			m_pEdit->InsertWord(m_Word,m_nCharset,&m_WordProps,FALSE,TRUE);
		}
	}
}

/* -------------------------------------------------------------------------- */
//CFXEU_Delete

CFXEU_Delete::CFXEU_Delete(CFX_Edit * pEdit, const CPVT_WordPlace & wpOldPlace, const CPVT_WordPlace & wpNewPlace,
							   FX_WORD word, FX_INT32 charset,
							   const CPVT_SecProps & SecProps, const CPVT_WordProps & WordProps, FX_BOOL bSecEnd) :
			m_pEdit(pEdit),
			m_wpOld(wpOldPlace),
			m_wpNew(wpNewPlace),
			m_Word(word),
			m_nCharset(charset),
			m_SecProps(SecProps),
			m_WordProps(WordProps),
			m_bSecEnd(bSecEnd)
{
}

CFXEU_Delete::~CFXEU_Delete()
{
}

void CFXEU_Delete::Redo()
{
	if (m_pEdit)
	{
		m_pEdit->SelectNone();
		m_pEdit->SetCaret(m_wpOld);
		m_pEdit->Delete(FALSE,TRUE);
	}
}

void CFXEU_Delete::Undo()
{
	if (m_pEdit)
	{
		m_pEdit->SelectNone();
		m_pEdit->SetCaret(m_wpNew);
		if (m_bSecEnd)
		{
			m_pEdit->InsertReturn(&m_SecProps,&m_WordProps,FALSE,TRUE);
		}
		else
		{
			m_pEdit->InsertWord(m_Word,m_nCharset,&m_WordProps,FALSE,TRUE);
		}
	}
}

/* -------------------------------------------------------------------------- */
//CFXEU_Clear

CFXEU_Clear::CFXEU_Clear(CFX_Edit * pEdit,  const CPVT_WordRange & wrSel, const CFX_WideString & swText) :
			m_pEdit(pEdit),
			m_wrSel(wrSel),
			m_swText(swText)
{
}

CFXEU_Clear::~CFXEU_Clear()
{
}

void CFXEU_Clear::Redo()
{
	if (m_pEdit)
	{
		m_pEdit->SelectNone();
		m_pEdit->SetSel(m_wrSel.BeginPos,m_wrSel.EndPos);
		m_pEdit->Clear(FALSE,TRUE);
	}
}

void CFXEU_Clear::Undo()
{
	if (m_pEdit)
	{
		m_pEdit->SelectNone();
		m_pEdit->SetCaret(m_wrSel.BeginPos);
		m_pEdit->InsertText(m_swText.c_str(), DEFAULT_CHARSET, NULL, NULL, FALSE, TRUE);
		m_pEdit->SetSel(m_wrSel.BeginPos,m_wrSel.EndPos);
	}
}

/* -------------------------------------------------------------------------- */
//CFXEU_ClearRich

CFXEU_ClearRich::CFXEU_ClearRich(CFX_Edit * pEdit, const CPVT_WordPlace & wpOldPlace, const CPVT_WordPlace & wpNewPlace,
							   const CPVT_WordRange & wrSel, FX_WORD word, FX_INT32 charset,
							   const CPVT_SecProps & SecProps, const CPVT_WordProps & WordProps) :
			m_pEdit(pEdit),
			m_wpOld(wpOldPlace),
			m_wpNew(wpNewPlace),
			m_wrSel(wrSel),
			m_Word(word),
			m_nCharset(charset),
			m_SecProps(SecProps),
			m_WordProps(WordProps)									 
{
}

CFXEU_ClearRich::~CFXEU_ClearRich()
{
}

void CFXEU_ClearRich::Redo()
{
	if (m_pEdit && IsLast())
	{
		m_pEdit->SelectNone();
		m_pEdit->SetSel(m_wrSel.BeginPos,m_wrSel.EndPos);
		m_pEdit->Clear(FALSE,TRUE);
	}
}

void CFXEU_ClearRich::Undo()
{
	if (m_pEdit)
	{
		m_pEdit->SelectNone();
		m_pEdit->SetCaret(m_wpOld);
		if (m_wpNew.SecCmp(m_wpOld) != 0)
		{
			m_pEdit->InsertReturn(&m_SecProps,&m_WordProps,FALSE,FALSE);
		}
		else
		{
			m_pEdit->InsertWord(m_Word,m_nCharset,&m_WordProps,FALSE,FALSE);
		}

		if (IsFirst())
		{
			m_pEdit->PaintInsertText(m_wrSel.BeginPos,m_wrSel.EndPos);
			m_pEdit->SetSel(m_wrSel.BeginPos,m_wrSel.EndPos);
		}
	}
}
/* -------------------------------------------------------------------------- */
//CFXEU_InsertText

CFXEU_InsertText::CFXEU_InsertText(CFX_Edit * pEdit, const CPVT_WordPlace & wpOldPlace, const CPVT_WordPlace & wpNewPlace,
							   const CFX_WideString & swText, FX_INT32 charset,
							   const CPVT_SecProps * pSecProps, const CPVT_WordProps * pWordProps) :
			m_pEdit(pEdit),
			m_wpOld(wpOldPlace),
			m_wpNew(wpNewPlace),
			m_swText(swText),
			m_nCharset(charset),
			m_SecProps(),
			m_WordProps()									 
{
	if (pSecProps)
		m_SecProps = *pSecProps;
	if (pWordProps)
		m_WordProps = *pWordProps;
}

CFXEU_InsertText::~CFXEU_InsertText()
{
}

void CFXEU_InsertText::Redo()
{
	if (m_pEdit && IsLast())
	{
		m_pEdit->SelectNone();
		m_pEdit->SetCaret(m_wpOld);
		m_pEdit->InsertText(m_swText.c_str(), m_nCharset, &m_SecProps, &m_WordProps, FALSE, TRUE);
	}
}

void CFXEU_InsertText::Undo()
{
	if (m_pEdit)
	{
		m_pEdit->SelectNone();
		m_pEdit->SetSel(m_wpOld,m_wpNew);
		m_pEdit->Clear(FALSE,TRUE);
	}
}

/* -------------------------------------------------------------------------- */

CFXEU_SetSecProps::CFXEU_SetSecProps(CFX_Edit * pEdit, const CPVT_WordPlace & place, EDIT_PROPS_E ep, 
		const CPVT_SecProps & oldsecprops, const CPVT_WordProps & oldwordprops, 
		const CPVT_SecProps & newsecprops, const CPVT_WordProps & newwordprops, const CPVT_WordRange & range)
		: m_pEdit(pEdit),
		m_wpPlace(place),
		m_wrPlace(range),
		m_eProps(ep),
		m_OldSecProps(oldsecprops),
		m_NewSecProps(newsecprops),
		m_OldWordProps(oldwordprops),
		m_NewWordProps(newwordprops)
{
}

CFXEU_SetSecProps::~CFXEU_SetSecProps()
{
}

void CFXEU_SetSecProps::Redo()
{
	if (m_pEdit)
	{
		m_pEdit->SetSecProps(m_eProps,m_wpPlace,&m_NewSecProps,&m_NewWordProps,m_wrPlace,FALSE);
		if (IsLast())
		{
			m_pEdit->SelectNone();
			m_pEdit->PaintSetProps(m_eProps,m_wrPlace);
			m_pEdit->SetSel(m_wrPlace.BeginPos,m_wrPlace.EndPos);
		}
	}
}

void CFXEU_SetSecProps::Undo()
{
	if (m_pEdit)
	{
		m_pEdit->SetSecProps(m_eProps,m_wpPlace,&m_OldSecProps,&m_OldWordProps,m_wrPlace,FALSE);
		if (IsFirst())
		{
			m_pEdit->SelectNone();
			m_pEdit->PaintSetProps(m_eProps,m_wrPlace);
			m_pEdit->SetSel(m_wrPlace.BeginPos,m_wrPlace.EndPos);
		}
	}
}

/* -------------------------------------------------------------------------- */

CFXEU_SetWordProps::CFXEU_SetWordProps(CFX_Edit * pEdit, const CPVT_WordPlace & place, EDIT_PROPS_E ep, 
		const CPVT_WordProps & oldprops, const CPVT_WordProps & newprops, const CPVT_WordRange & range)
		: m_pEdit(pEdit),
		m_wpPlace(place),
		m_wrPlace(range),
		m_eProps(ep),
		m_OldWordProps(oldprops),
		m_NewWordProps(newprops)
{
}

CFXEU_SetWordProps::~CFXEU_SetWordProps()
{
}

void CFXEU_SetWordProps::Redo()
{
	if (m_pEdit)
	{
		m_pEdit->SetWordProps(m_eProps,m_wpPlace,&m_NewWordProps,m_wrPlace,FALSE);
		if (IsLast())
		{
			m_pEdit->SelectNone();
			m_pEdit->PaintSetProps(m_eProps,m_wrPlace);
			m_pEdit->SetSel(m_wrPlace.BeginPos,m_wrPlace.EndPos);
		}
	}
}

void CFXEU_SetWordProps::Undo()
{
	if (m_pEdit)
	{
		m_pEdit->SetWordProps(m_eProps,m_wpPlace,&m_OldWordProps,m_wrPlace,FALSE);
		if (IsFirst())
		{
			m_pEdit->SelectNone();
			m_pEdit->PaintSetProps(m_eProps,m_wrPlace);
			m_pEdit->SetSel(m_wrPlace.BeginPos,m_wrPlace.EndPos);
		}
	}
}

/* ------------------------------------- CFX_Edit ------------------------------------- */

CFX_Edit::CFX_Edit(IPDF_VariableText * pVT) :
	m_pVT(pVT),
	m_pNotify(NULL),
	m_pOprNotify(NULL),
	m_pVTProvide(NULL),
	m_wpCaret(-1,-1,-1),
	m_wpOldCaret(-1,-1,-1),
	m_SelState(),
	m_ptScrollPos(0,0),
	m_ptRefreshScrollPos(0,0),
	m_bEnableScroll(FALSE),
	m_pIterator(NULL),
	m_ptCaret(0.0f,0.0f),
	m_Undo(FX_EDIT_UNDO_MAXITEM),
	m_nAlignment(0),
	m_bNotifyFlag(FALSE),
	m_bEnableOverflow(FALSE),
	m_bEnableRefresh(TRUE),
	m_rcOldContent(0.0f,0.0f,0.0f,0.0f),
	m_bEnableUndo(TRUE),
	m_bNotify(TRUE),
	m_bOprNotify(FALSE),
	m_pGroupUndoItem(NULL)
{	
	ASSERT(pVT != NULL);
}

CFX_Edit::~CFX_Edit()
{
	if (m_pVTProvide)
	{
		delete m_pVTProvide;
		m_pVTProvide = NULL;
	}

	if (m_pIterator)
	{
		delete m_pIterator;
		m_pIterator = NULL;
	}

	ASSERT(m_pGroupUndoItem == NULL);
}

// public methods

void CFX_Edit::Initialize()
{
	m_pVT->Initialize();
	SetCaret(m_pVT->GetBeginWordPlace());
	SetCaretOrigin();
}

void CFX_Edit::SetFontMap(IFX_Edit_FontMap * pFontMap)
{
	if (m_pVTProvide) 
		delete m_pVTProvide;

	m_pVT->SetProvider(m_pVTProvide = new CFX_Edit_Provider(pFontMap));
}

void CFX_Edit::SetVTProvider(IPDF_VariableText_Provider* pProvider)
{
	m_pVT->SetProvider(pProvider);
}

void CFX_Edit::SetNotify(IFX_Edit_Notify* pNotify)
{
	m_pNotify = pNotify;
}

void CFX_Edit::SetOprNotify(IFX_Edit_OprNotify* pOprNotify)
{
	m_pOprNotify = pOprNotify;
}

IFX_Edit_Iterator * CFX_Edit::GetIterator()
{
	if (!m_pIterator)
		m_pIterator = new CFX_Edit_Iterator(this,m_pVT->GetIterator());

	return m_pIterator;
}

IPDF_VariableText *	CFX_Edit::GetVariableText()
{
	return m_pVT;
}

IFX_Edit_FontMap* CFX_Edit::GetFontMap()
{
	if (m_pVTProvide)
		return m_pVTProvide->GetFontMap();

	return NULL;
}

void CFX_Edit::SetPlateRect(const CPDF_Rect & rect, FX_BOOL bPaint/* = TRUE*/)
{	
	m_pVT->SetPlateRect(rect);
	m_ptScrollPos = CPDF_Point(rect.left,rect.top);			
	if (bPaint) Paint();
}

void CFX_Edit::SetAlignmentH(FX_INT32 nFormat/* =0 */, FX_BOOL bPaint/* = TRUE*/)
{
	m_pVT->SetAlignment(nFormat);
	if (bPaint) Paint();
}

void CFX_Edit::SetAlignmentV(FX_INT32 nFormat/* =0 */, FX_BOOL bPaint/* = TRUE*/)
{
	m_nAlignment = nFormat;
	if (bPaint) Paint();
}

void CFX_Edit::SetPasswordChar(FX_WORD wSubWord/* ='*' */, FX_BOOL bPaint/* = TRUE*/)
{
	m_pVT->SetPasswordChar(wSubWord);
	if (bPaint) Paint();
}

void CFX_Edit::SetLimitChar(FX_INT32 nLimitChar/* =0 */, FX_BOOL bPaint/* = TRUE*/)
{
	m_pVT->SetLimitChar(nLimitChar);
	if (bPaint) Paint();
}

void CFX_Edit::SetCharArray(FX_INT32 nCharArray/* =0 */, FX_BOOL bPaint/* = TRUE*/)
{
	m_pVT->SetCharArray(nCharArray);
	if (bPaint) Paint();
}

void CFX_Edit::SetCharSpace(FX_FLOAT fCharSpace/* =0.0f */, FX_BOOL bPaint/* = TRUE*/)
{
	m_pVT->SetCharSpace(fCharSpace);
	if (bPaint) Paint();
}

void CFX_Edit::SetHorzScale(FX_INT32 nHorzScale/* =100 */, FX_BOOL bPaint/* = TRUE*/)
{
	m_pVT->SetHorzScale(nHorzScale);
	if (bPaint) Paint();
}

void CFX_Edit::SetMultiLine(FX_BOOL bMultiLine/* =TRUE */, FX_BOOL bPaint/* = TRUE*/)
{
	m_pVT->SetMultiLine(bMultiLine);
	if (bPaint) Paint();
}

void CFX_Edit::SetAutoReturn(FX_BOOL bAuto/* =TRUE */, FX_BOOL bPaint/* = TRUE*/)
{
	m_pVT->SetAutoReturn(bAuto);
	if (bPaint) Paint();
}

void CFX_Edit::SetLineLeading(FX_FLOAT fLineLeading/* =TRUE */, FX_BOOL bPaint/* = TRUE*/)
{
	m_pVT->SetLineLeading(fLineLeading);
	if (bPaint) Paint();
}

void CFX_Edit::SetAutoFontSize(FX_BOOL bAuto/* =TRUE */, FX_BOOL bPaint/* = TRUE*/)
{
	m_pVT->SetAutoFontSize(bAuto);
	if (bPaint) Paint();
}

void CFX_Edit::SetFontSize(FX_FLOAT fFontSize, FX_BOOL bPaint/* = TRUE*/)
{
	m_pVT->SetFontSize(fFontSize);
	if (bPaint) Paint();
}

void CFX_Edit::SetAutoScroll(FX_BOOL bAuto/* =TRUE */, FX_BOOL bPaint/* = TRUE*/)
{
	m_bEnableScroll = bAuto;
	if (bPaint) Paint();
}

void CFX_Edit::SetTextOverflow(FX_BOOL bAllowed /*= FALSE*/, FX_BOOL bPaint/* = TRUE*/)
{
	m_bEnableOverflow = bAllowed;
	if (bPaint) Paint();
}

void CFX_Edit::SetSel(FX_INT32 nStartChar,FX_INT32 nEndChar)
{
	if (m_pVT->IsValid())
	{
		if (nStartChar == 0 && nEndChar < 0)
		{
			SelectAll();
		}
		else if (nStartChar < 0)
		{	
			this->SelectNone();
		}
		else
		{		
			if (nStartChar < nEndChar)
			{
				SetSel(m_pVT->WordIndexToWordPlace(nStartChar),m_pVT->WordIndexToWordPlace(nEndChar));
			}
			else
			{
				SetSel(m_pVT->WordIndexToWordPlace(nEndChar),m_pVT->WordIndexToWordPlace(nStartChar));
			}
		}	
	}
}

void CFX_Edit::SetSel(const CPVT_WordPlace & begin,const CPVT_WordPlace & end)
{
	if (m_pVT->IsValid())
	{
		SelectNone();

		m_SelState.Set(begin,end);

		SetCaret(m_SelState.EndPos);

		if (m_SelState.IsExist())
		{
			ScrollToCaret();
			CPVT_WordRange wr(m_SelState.BeginPos,m_SelState.EndPos);
			Refresh(RP_OPTIONAL,&wr);
			SetCaretInfo();
		}
		else
		{		
			ScrollToCaret();
			SetCaretInfo();
		}
	}
}

void CFX_Edit::GetSel(FX_INT32 & nStartChar, FX_INT32 & nEndChar) const
{
	nStartChar = -1;
	nEndChar = -1;

	if (m_pVT->IsValid())
	{
		if (m_SelState.IsExist())
		{
			if (m_SelState.BeginPos.WordCmp(m_SelState.EndPos)<0)
			{
				nStartChar = m_pVT->WordPlaceToWordIndex(m_SelState.BeginPos);
				nEndChar = m_pVT->WordPlaceToWordIndex(m_SelState.EndPos);
			}
			else
			{
				nStartChar = m_pVT->WordPlaceToWordIndex(m_SelState.EndPos);
				nEndChar = m_pVT->WordPlaceToWordIndex(m_SelState.BeginPos);
			}
		}
		else
		{
			nStartChar = m_pVT->WordPlaceToWordIndex(m_wpCaret);
			nEndChar = m_pVT->WordPlaceToWordIndex(m_wpCaret);
		}
	}
}

FX_INT32 CFX_Edit::GetCaret() const
{
	if (m_pVT->IsValid())
		return m_pVT->WordPlaceToWordIndex(m_wpCaret);

	return -1;
}

CPVT_WordPlace CFX_Edit::GetCaretWordPlace() const
{
	return m_wpCaret;
}

CFX_WideString CFX_Edit::GetText() const
{
	CFX_WideString swRet;

	if (m_pVT->IsValid())
	{
		if (IPDF_VariableText_Iterator * pIterator = m_pVT->GetIterator())
		{
			FX_BOOL bRich = m_pVT->IsRichText();

			pIterator->SetAt(0);

			CPVT_Word wordinfo;	
			CPVT_WordPlace oldplace = pIterator->GetAt();
			while (pIterator->NextWord())
			{
				CPVT_WordPlace place = pIterator->GetAt();

				if (pIterator->GetWord(wordinfo))
				{
					if (bRich)
					{
						swRet += wordinfo.Word;
					}
					else
					{
						swRet += wordinfo.Word;
					}					
				}

				if (oldplace.SecCmp(place) != 0)
				{
					swRet += 0x0D;
					swRet += 0x0A;
				}
				
				oldplace = place;
			}
		}
	}

	return swRet;
}

CFX_WideString CFX_Edit::GetRangeText(const CPVT_WordRange & range) const
{
	CFX_WideString swRet;

	if (m_pVT->IsValid())
	{
		FX_BOOL bRich = m_pVT->IsRichText();

		if (IPDF_VariableText_Iterator * pIterator = m_pVT->GetIterator())
		{			
			CPVT_WordRange wrTemp = range;
			m_pVT->UpdateWordPlace(wrTemp.BeginPos);
			m_pVT->UpdateWordPlace(wrTemp.EndPos);
			pIterator->SetAt(wrTemp.BeginPos);

			CPVT_Word wordinfo;	
			CPVT_WordPlace oldplace = wrTemp.BeginPos;
			while (pIterator->NextWord())
			{
				CPVT_WordPlace place = pIterator->GetAt();
				if (place.WordCmp(wrTemp.EndPos) > 0)break;

				if (pIterator->GetWord(wordinfo))
				{
					if (bRich)
					{
						swRet += wordinfo.Word;
					}
					else
					{
						swRet += wordinfo.Word;
					}					
				}

				if (oldplace.SecCmp(place) != 0)
				{
					swRet += 0x0D;
					swRet += 0x0A;
				}
				
				oldplace = place;
			}
		}
	}

	return swRet;
}

CFX_WideString CFX_Edit::GetSelText() const
{
	return GetRangeText(m_SelState.ConvertToWordRange());
}

FX_INT32 CFX_Edit::GetTotalWords() const
{
	return m_pVT->GetTotalWords();
}

FX_INT32 CFX_Edit::GetTotalLines() const
{
	FX_INT32 nLines = 0;

	if (IPDF_VariableText_Iterator * pIterator = m_pVT->GetIterator())
	{
		pIterator->SetAt(0);
		while (pIterator->NextLine())
			nLines++;
	}

	return nLines+1;
}

CPVT_WordRange CFX_Edit::GetSelectWordRange() const
{
	return m_SelState.ConvertToWordRange();
}

CPVT_WordRange CFX_Edit::CombineWordRange(const CPVT_WordRange & wr1, const CPVT_WordRange & wr2)
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

FX_BOOL	CFX_Edit::IsRichText() const
{
	return m_pVT->IsRichText();
}

void CFX_Edit::SetRichText(FX_BOOL bRichText/* =TRUE */, FX_BOOL bPaint/* = TRUE*/)
{
	m_pVT->SetRichText(bRichText);
	if (bPaint) Paint();
}

FX_BOOL CFX_Edit::SetRichFontIndex(FX_INT32 nFontIndex)
{
	CPVT_WordProps WordProps;
	WordProps.nFontIndex = nFontIndex;
	return SetRichTextProps(EP_FONTINDEX,NULL,&WordProps);	
}

FX_BOOL CFX_Edit::SetRichFontSize(FX_FLOAT fFontSize)
{	
	CPVT_WordProps WordProps;
	WordProps.fFontSize = fFontSize;
	return SetRichTextProps(EP_FONTSIZE,NULL,&WordProps);	
}

FX_BOOL CFX_Edit::SetRichTextColor(FX_COLORREF dwColor)
{
	CPVT_WordProps WordProps;
	WordProps.dwWordColor = dwColor;
	return SetRichTextProps(EP_WORDCOLOR,NULL,&WordProps);	
}

FX_BOOL CFX_Edit::SetRichTextScript(FX_INT32 nScriptType)
{
	CPVT_WordProps WordProps;
	WordProps.nScriptType = nScriptType;
	return SetRichTextProps(EP_SCRIPTTYPE,NULL,&WordProps);	
}

FX_BOOL CFX_Edit::SetRichTextBold(FX_BOOL bBold)
{
	CPVT_WordProps WordProps;
	if (bBold)
		WordProps.nWordStyle |= PVTWORD_STYLE_BOLD;
	return SetRichTextProps(EP_BOLD,NULL,&WordProps);
}

FX_BOOL CFX_Edit::SetRichTextItalic(FX_BOOL bItalic)
{
	CPVT_WordProps WordProps;
	if (bItalic)
		WordProps.nWordStyle |= PVTWORD_STYLE_ITALIC;
	return SetRichTextProps(EP_ITALIC,NULL,&WordProps);
}

FX_BOOL CFX_Edit::SetRichTextUnderline(FX_BOOL bUnderline)
{
	CPVT_WordProps WordProps;
	if (bUnderline)
		WordProps.nWordStyle |= PVTWORD_STYLE_UNDERLINE;
	return SetRichTextProps(EP_UNDERLINE,NULL,&WordProps);
}

FX_BOOL CFX_Edit::SetRichTextCrossout(FX_BOOL bCrossout)
{
	CPVT_WordProps WordProps;
	if (bCrossout)
		WordProps.nWordStyle |= PVTWORD_STYLE_CROSSOUT;
	return SetRichTextProps(EP_CROSSOUT,NULL,&WordProps);
}

FX_BOOL CFX_Edit::SetRichTextCharSpace(FX_FLOAT fCharSpace)
{
	CPVT_WordProps WordProps;
	WordProps.fCharSpace = fCharSpace;
	return SetRichTextProps(EP_CHARSPACE,NULL,&WordProps);	
}

FX_BOOL CFX_Edit::SetRichTextHorzScale(FX_INT32 nHorzScale /*= 100*/)
{
	CPVT_WordProps WordProps;
	WordProps.nHorzScale = nHorzScale;
	return SetRichTextProps(EP_HORZSCALE,NULL,&WordProps);	
}

FX_BOOL CFX_Edit::SetRichTextLineLeading(FX_FLOAT fLineLeading)
{
	CPVT_SecProps SecProps;
	SecProps.fLineLeading = fLineLeading;
	return SetRichTextProps(EP_LINELEADING,&SecProps,NULL);	
}

FX_BOOL CFX_Edit::SetRichTextLineIndent(FX_FLOAT fLineIndent)
{
	CPVT_SecProps SecProps;
	SecProps.fLineIndent = fLineIndent;
	return SetRichTextProps(EP_LINEINDENT,&SecProps,NULL);
}

FX_BOOL	CFX_Edit::SetRichTextAlignment(FX_INT32 nAlignment)
{
	CPVT_SecProps SecProps;
	SecProps.nAlignment = nAlignment;
	return SetRichTextProps(EP_ALIGNMENT,&SecProps,NULL);
}

FX_BOOL CFX_Edit::SetRichTextProps(EDIT_PROPS_E eProps, const CPVT_SecProps * pSecProps, const CPVT_WordProps * pWordProps)
{
	FX_BOOL bSet = FALSE;
	FX_BOOL bSet1,bSet2;
	if (m_pVT->IsValid() && m_pVT->IsRichText())
	{
		if (IPDF_VariableText_Iterator * pIterator = m_pVT->GetIterator())
		{
			CPVT_WordRange wrTemp = m_SelState.ConvertToWordRange();
			
			m_pVT->UpdateWordPlace(wrTemp.BeginPos);
			m_pVT->UpdateWordPlace(wrTemp.EndPos);
			pIterator->SetAt(wrTemp.BeginPos);

			BeginGroupUndo(L"");;

			bSet = SetSecProps(eProps,wrTemp.BeginPos,pSecProps,pWordProps,wrTemp,TRUE);

			while (pIterator->NextWord())
			{
				CPVT_WordPlace place = pIterator->GetAt();
				if (place.WordCmp(wrTemp.EndPos) > 0) break;
				bSet1 = SetSecProps(eProps,place,pSecProps,pWordProps,wrTemp,TRUE);
				bSet2 = SetWordProps(eProps,place,pWordProps,wrTemp,TRUE);
				
				if (!bSet)
					bSet = (bSet1 || bSet2);
			}

			EndGroupUndo();

			if (bSet)
			{
				PaintSetProps(eProps,wrTemp);
			}
		}
	}	

	return bSet;
}

void CFX_Edit::PaintSetProps(EDIT_PROPS_E eProps, const CPVT_WordRange & wr)
{
	switch(eProps)
	{
	case EP_LINELEADING:
	case EP_LINEINDENT:
	case EP_ALIGNMENT:
		RearrangePart(wr);
		ScrollToCaret();
		Refresh(RP_ANALYSE);
		SetCaretOrigin();
		SetCaretInfo();	
		break;					
	case EP_WORDCOLOR:
	case EP_UNDERLINE:
	case EP_CROSSOUT:
		Refresh(RP_OPTIONAL,&wr);
		break;
	case EP_FONTINDEX:
	case EP_FONTSIZE:
	case EP_SCRIPTTYPE:					
	case EP_CHARSPACE:
	case EP_HORZSCALE:
	case EP_BOLD:
	case EP_ITALIC:
		RearrangePart(wr);
		ScrollToCaret();

		CPVT_WordRange wrRefresh(m_pVT->GetSectionBeginPlace(wr.BeginPos),
			m_pVT->GetSectionEndPlace(wr.EndPos));
		Refresh(RP_ANALYSE,&wrRefresh);

		SetCaretOrigin();
		SetCaretInfo();	
		break;
	}				
}

FX_BOOL CFX_Edit::SetSecProps(EDIT_PROPS_E eProps, const CPVT_WordPlace & place, 
							   const CPVT_SecProps * pSecProps, const CPVT_WordProps * pWordProps, 
							   const CPVT_WordRange & wr, FX_BOOL bAddUndo)
{
	if (m_pVT->IsValid() && m_pVT->IsRichText())
	{
		if (IPDF_VariableText_Iterator * pIterator = m_pVT->GetIterator())
		{
			FX_BOOL bSet = FALSE;
			CPVT_Section secinfo;
			CPVT_Section OldSecinfo;

			CPVT_WordPlace oldplace = pIterator->GetAt();

			if (eProps == EP_LINELEADING || eProps == EP_LINEINDENT || eProps == EP_ALIGNMENT)
			{
				if (pSecProps)
				{
					pIterator->SetAt(place);
					if (pIterator->GetSection(secinfo))
					{
						if (bAddUndo) OldSecinfo = secinfo;

						switch(eProps)
						{
						case EP_LINELEADING:				
							if (!FX_EDIT_IsFloatEqual(secinfo.SecProps.fLineLeading,pSecProps->fLineLeading))
							{
								secinfo.SecProps.fLineLeading = pSecProps->fLineLeading;							
								bSet = TRUE;
							}
							break;
						case EP_LINEINDENT:
							if (!FX_EDIT_IsFloatEqual(secinfo.SecProps.fLineIndent,pSecProps->fLineIndent))
							{
								secinfo.SecProps.fLineIndent = pSecProps->fLineIndent;
								bSet = TRUE;
							}
							break;
						case EP_ALIGNMENT:
							if (secinfo.SecProps.nAlignment != pSecProps->nAlignment)
							{
								secinfo.SecProps.nAlignment = pSecProps->nAlignment;
								bSet = TRUE;
							}
							break;
						default:
							break;
						}
					}
				}
			}
			else
			{
				if (pWordProps && place == m_pVT->GetSectionBeginPlace(place))
				{
					pIterator->SetAt(place);
					if (pIterator->GetSection(secinfo))
					{
						if (bAddUndo) OldSecinfo = secinfo;

						switch(eProps)
						{
						case EP_FONTINDEX:				
							if (secinfo.WordProps.nFontIndex != pWordProps->nFontIndex)
							{
								secinfo.WordProps.nFontIndex = pWordProps->nFontIndex;
								bSet = TRUE;
							}
							break;
						case EP_FONTSIZE:
							if (!FX_EDIT_IsFloatEqual(secinfo.WordProps.fFontSize,pWordProps->fFontSize))
							{
								secinfo.WordProps.fFontSize = pWordProps->fFontSize;
								bSet = TRUE;
							}
							break;
						case EP_WORDCOLOR:
							if (secinfo.WordProps.dwWordColor != pWordProps->dwWordColor)
							{
								secinfo.WordProps.dwWordColor = pWordProps->dwWordColor;
								bSet = TRUE;
							}
							break;
						case EP_SCRIPTTYPE:				
							if (secinfo.WordProps.nScriptType != pWordProps->nScriptType)
							{
								secinfo.WordProps.nScriptType = pWordProps->nScriptType;
								bSet = TRUE;
							}
							break;
						case EP_CHARSPACE:			
							if (!FX_EDIT_IsFloatEqual(secinfo.WordProps.fCharSpace,pWordProps->fCharSpace))
							{
								secinfo.WordProps.fCharSpace = pWordProps->fCharSpace;
								bSet = TRUE;
							}
							break;
						case EP_HORZSCALE:				
							if (secinfo.WordProps.nHorzScale != pWordProps->nHorzScale)
							{
								secinfo.WordProps.nHorzScale = pWordProps->nHorzScale;
								bSet = TRUE;
							}
							break;
						case EP_UNDERLINE:
							if (pWordProps->nWordStyle & PVTWORD_STYLE_UNDERLINE)
							{
								if ((secinfo.WordProps.nWordStyle & PVTWORD_STYLE_UNDERLINE) == 0)
								{
									secinfo.WordProps.nWordStyle |= PVTWORD_STYLE_UNDERLINE; 
									bSet = TRUE;
								}
							}
							else
							{
								if ((secinfo.WordProps.nWordStyle & PVTWORD_STYLE_UNDERLINE) != 0)
								{
									secinfo.WordProps.nWordStyle &= ~PVTWORD_STYLE_UNDERLINE;
									bSet = TRUE;
								}
							}
							break;
						case EP_CROSSOUT:
							if (pWordProps->nWordStyle & PVTWORD_STYLE_CROSSOUT)
							{
								if ((secinfo.WordProps.nWordStyle & PVTWORD_STYLE_CROSSOUT) == 0)
								{
									secinfo.WordProps.nWordStyle |= PVTWORD_STYLE_CROSSOUT; 
									bSet = TRUE;
								}
							}
							else
							{
								if ((secinfo.WordProps.nWordStyle & PVTWORD_STYLE_CROSSOUT) != 0)
								{
									secinfo.WordProps.nWordStyle &= ~PVTWORD_STYLE_CROSSOUT;
									bSet = TRUE;
								}
							}
							break;
						case EP_BOLD:
							if (pWordProps->nWordStyle & PVTWORD_STYLE_BOLD)
							{
								if ((secinfo.WordProps.nWordStyle & PVTWORD_STYLE_BOLD) == 0)
								{
									secinfo.WordProps.nWordStyle |= PVTWORD_STYLE_BOLD; 
									bSet = TRUE;
								}
							}
							else
							{
								if ((secinfo.WordProps.nWordStyle & PVTWORD_STYLE_BOLD) != 0)
								{
									secinfo.WordProps.nWordStyle &= ~PVTWORD_STYLE_BOLD;
									bSet = TRUE;
								}
							}
							break;
						case EP_ITALIC:
							if (pWordProps->nWordStyle & PVTWORD_STYLE_ITALIC)
							{
								if ((secinfo.WordProps.nWordStyle & PVTWORD_STYLE_ITALIC) == 0)
								{
									secinfo.WordProps.nWordStyle |= PVTWORD_STYLE_ITALIC; 
									bSet = TRUE;
								}
							}
							else
							{
								if ((secinfo.WordProps.nWordStyle & PVTWORD_STYLE_ITALIC) != 0)
								{
									secinfo.WordProps.nWordStyle &= ~PVTWORD_STYLE_ITALIC;
									bSet = TRUE;
								}
							}
							break;
						default:
							break;
						}
					}
				}
			}

			if (bSet)
			{
				pIterator->SetSection(secinfo);

				if (bAddUndo && m_bEnableUndo)
				{
					AddEditUndoItem(new CFXEU_SetSecProps
						(this,place,eProps,OldSecinfo.SecProps,OldSecinfo.WordProps,secinfo.SecProps,secinfo.WordProps,wr));
				}
			}

			pIterator->SetAt(oldplace);

			return bSet;
		}
	}
	
	return FALSE;
}

FX_BOOL CFX_Edit::SetWordProps(EDIT_PROPS_E eProps, const CPVT_WordPlace & place, 
								const CPVT_WordProps * pWordProps, const CPVT_WordRange & wr, FX_BOOL bAddUndo)
{
	if (m_pVT->IsValid() && m_pVT->IsRichText())
	{
		if (IPDF_VariableText_Iterator * pIterator = m_pVT->GetIterator())
		{
			FX_BOOL bSet = FALSE;
			CPVT_Word wordinfo;
			CPVT_Word OldWordinfo;

			CPVT_WordPlace oldplace = pIterator->GetAt();

			if (pWordProps)
			{
				pIterator->SetAt(place);
				if (pIterator->GetWord(wordinfo))
				{
					if (bAddUndo) OldWordinfo = wordinfo;

					switch(eProps)
					{
					case EP_FONTINDEX:				
						if (wordinfo.WordProps.nFontIndex != pWordProps->nFontIndex)
						{
							if (IFX_Edit_FontMap* pFontMap = this->GetFontMap())
							{
								wordinfo.WordProps.nFontIndex = pFontMap->GetWordFontIndex(wordinfo.Word,wordinfo.nCharset,pWordProps->nFontIndex);
							}
							bSet = TRUE;
						}
						break;
					case EP_FONTSIZE:
						if (!FX_EDIT_IsFloatEqual(wordinfo.WordProps.fFontSize,pWordProps->fFontSize))
						{
							wordinfo.WordProps.fFontSize = pWordProps->fFontSize;
							bSet = TRUE;
						}
						break;
					case EP_WORDCOLOR:
						if (wordinfo.WordProps.dwWordColor != pWordProps->dwWordColor)
						{
							wordinfo.WordProps.dwWordColor = pWordProps->dwWordColor;
							bSet = TRUE;
						}
						break;
					case EP_SCRIPTTYPE:
						if (wordinfo.WordProps.nScriptType != pWordProps->nScriptType)
						{
							wordinfo.WordProps.nScriptType = pWordProps->nScriptType;
							bSet = TRUE;
						}
						break;
					case EP_CHARSPACE:
						if (!FX_EDIT_IsFloatEqual(wordinfo.WordProps.fCharSpace,pWordProps->fCharSpace))
						{
							wordinfo.WordProps.fCharSpace = pWordProps->fCharSpace;
							bSet = TRUE;
						}
						break;
					case EP_HORZSCALE:
						if (wordinfo.WordProps.nHorzScale != pWordProps->nHorzScale)
						{
							wordinfo.WordProps.nHorzScale = pWordProps->nHorzScale;
							bSet = TRUE;
						}
						break;
					case EP_UNDERLINE:
						if (pWordProps->nWordStyle & PVTWORD_STYLE_UNDERLINE)
						{
							if ((wordinfo.WordProps.nWordStyle & PVTWORD_STYLE_UNDERLINE) == 0)
							{
								wordinfo.WordProps.nWordStyle |= PVTWORD_STYLE_UNDERLINE;
								bSet = TRUE;
							}
						}
						else
						{
							if ((wordinfo.WordProps.nWordStyle & PVTWORD_STYLE_UNDERLINE) != 0)
							{
								wordinfo.WordProps.nWordStyle &= ~PVTWORD_STYLE_UNDERLINE;
								bSet = TRUE;
							}
						}
						break;
					case EP_CROSSOUT:
						if (pWordProps->nWordStyle & PVTWORD_STYLE_CROSSOUT)
						{
							if ((wordinfo.WordProps.nWordStyle & PVTWORD_STYLE_CROSSOUT) == 0)
							{
								wordinfo.WordProps.nWordStyle |= PVTWORD_STYLE_CROSSOUT; 
								bSet = TRUE;
							}
						}
						else
						{
							if ((wordinfo.WordProps.nWordStyle & PVTWORD_STYLE_CROSSOUT) != 0)
							{
								wordinfo.WordProps.nWordStyle &= ~PVTWORD_STYLE_CROSSOUT;
								bSet = TRUE;
							}
						}
						break;
					case EP_BOLD:
						if (pWordProps->nWordStyle & PVTWORD_STYLE_BOLD)
						{
							if ((wordinfo.WordProps.nWordStyle & PVTWORD_STYLE_BOLD) == 0)
							{
								wordinfo.WordProps.nWordStyle |= PVTWORD_STYLE_BOLD; 
								bSet = TRUE;
							}
						}
						else
						{
							if ((wordinfo.WordProps.nWordStyle & PVTWORD_STYLE_BOLD) != 0)
							{
								wordinfo.WordProps.nWordStyle &= ~PVTWORD_STYLE_BOLD;
								bSet = TRUE;
							}
						}
						break;
					case EP_ITALIC:
						if (pWordProps->nWordStyle & PVTWORD_STYLE_ITALIC)
						{
							if ((wordinfo.WordProps.nWordStyle & PVTWORD_STYLE_ITALIC) == 0)
							{
								wordinfo.WordProps.nWordStyle |= PVTWORD_STYLE_ITALIC; 
								bSet = TRUE;
							}
						}
						else
						{
							if ((wordinfo.WordProps.nWordStyle & PVTWORD_STYLE_ITALIC) != 0)
							{
								wordinfo.WordProps.nWordStyle &= ~PVTWORD_STYLE_ITALIC;
								bSet = TRUE;
							}
						}
						break;
					default:
						break;
					}
				}
			}	

			if (bSet)
			{
				pIterator->SetWord(wordinfo);

				if (bAddUndo && m_bEnableUndo)
				{
					AddEditUndoItem(new CFXEU_SetWordProps
						(this,place,eProps,OldWordinfo.WordProps,wordinfo.WordProps,wr));
				}
			}
			
			pIterator->SetAt(oldplace);
			return bSet;
		}
	}

	return FALSE;
}

void CFX_Edit::SetText(FX_LPCWSTR text,FX_INT32 charset /*= DEFAULT_CHARSET*/,
						const CPVT_SecProps * pSecProps /*= NULL*/,const CPVT_WordProps * pWordProps /*= NULL*/)
{
	SetText(text,charset,pSecProps,pWordProps,TRUE,TRUE);
}

FX_BOOL CFX_Edit::InsertWord(FX_WORD word, FX_INT32 charset /*= DEFAULT_CHARSET*/, const CPVT_WordProps * pWordProps /*= NULL*/)
{
	return InsertWord(word,charset,pWordProps,TRUE,TRUE);
}

FX_BOOL CFX_Edit::InsertReturn(const CPVT_SecProps * pSecProps /*= NULL*/,const CPVT_WordProps * pWordProps /*= NULL*/)
{
	return InsertReturn(pSecProps,pWordProps,TRUE,TRUE);
}

FX_BOOL CFX_Edit::Backspace()
{
	return Backspace(TRUE,TRUE);
}

FX_BOOL CFX_Edit::Delete()
{
	return Delete(TRUE,TRUE);
}

FX_BOOL CFX_Edit::Clear()
{
	return Clear(TRUE,TRUE);
}

FX_BOOL CFX_Edit::InsertText(FX_LPCWSTR text, FX_INT32 charset /*= DEFAULT_CHARSET*/,
								const CPVT_SecProps * pSecProps /*= NULL*/,const CPVT_WordProps * pWordProps /*= NULL*/)
{
	return InsertText(text,charset,pSecProps,pWordProps,TRUE,TRUE);
}

FX_FLOAT CFX_Edit::GetFontSize() const
{
	return m_pVT->GetFontSize();
}

FX_WORD CFX_Edit::GetPasswordChar() const
{
	return m_pVT->GetPasswordChar();
}

FX_INT32 CFX_Edit::GetCharArray() const
{
	return m_pVT->GetCharArray();
}

CPDF_Rect CFX_Edit::GetPlateRect() const
{
	return m_pVT->GetPlateRect();
}

CPDF_Rect CFX_Edit::GetContentRect() const
{
	return VTToEdit(m_pVT->GetContentRect());
}

FX_INT32 CFX_Edit::GetHorzScale() const
{
	return m_pVT->GetHorzScale();
}

FX_FLOAT CFX_Edit::GetCharSpace() const
{
	return m_pVT->GetCharSpace();
}

// inner methods

CPVT_WordRange CFX_Edit::GetWholeWordRange() const
{
	if (m_pVT->IsValid())
		return CPVT_WordRange(m_pVT->GetBeginWordPlace(),m_pVT->GetEndWordPlace());

	return CPVT_WordRange();
}

CPVT_WordRange CFX_Edit::GetVisibleWordRange() const
{
	if (m_bEnableOverflow) return GetWholeWordRange();

	if (m_pVT->IsValid())
	{
		CPDF_Rect rcPlate = m_pVT->GetPlateRect();

		CPVT_WordPlace place1 = m_pVT->SearchWordPlace(EditToVT(CPDF_Point(rcPlate.left,rcPlate.top)));
		CPVT_WordPlace place2 = m_pVT->SearchWordPlace(EditToVT(CPDF_Point(rcPlate.right,rcPlate.bottom)));

		return CPVT_WordRange(place1,place2);
	}

	return CPVT_WordRange();
}

CPVT_WordPlace CFX_Edit::SearchWordPlace(const CPDF_Point& point) const
{
	if (m_pVT->IsValid())
	{
		return m_pVT->SearchWordPlace(EditToVT(point));
	}

	return CPVT_WordPlace();
}

void CFX_Edit::Paint()
{
	if (m_pVT->IsValid())
	{
		RearrangeAll();
		ScrollToCaret();
		Refresh(RP_NOANALYSE);
		SetCaretOrigin();
		SetCaretInfo();
	}
}

void CFX_Edit::RearrangeAll()
{
	if (m_pVT->IsValid())
	{
		m_pVT->UpdateWordPlace(m_wpCaret);
		m_pVT->RearrangeAll();
		m_pVT->UpdateWordPlace(m_wpCaret);
		SetScrollInfo();
		SetContentChanged();
	}
}

void CFX_Edit::RearrangePart(const CPVT_WordRange & range)
{
	if (m_pVT->IsValid())
	{
		m_pVT->UpdateWordPlace(m_wpCaret);
		m_pVT->RearrangePart(range);
		m_pVT->UpdateWordPlace(m_wpCaret);
		SetScrollInfo();
		SetContentChanged();
	}
}

void CFX_Edit::SetContentChanged()
{
	if (m_bNotify && m_pNotify)
	{
		CPDF_Rect rcContent = m_pVT->GetContentRect();
		if (rcContent.Width() != m_rcOldContent.Width() ||
			rcContent.Height() != m_rcOldContent.Height())
		{
			if (!m_bNotifyFlag)
			{
				m_bNotifyFlag = TRUE;
				m_pNotify->IOnContentChange(rcContent);
				m_bNotifyFlag = FALSE;
			}
			m_rcOldContent = rcContent;
		}
	}
}

void CFX_Edit::SelectAll()
{
	if (m_pVT->IsValid())
	{
		m_SelState = GetWholeWordRange();		
		SetCaret(m_SelState.EndPos);		
		
		ScrollToCaret();
		CPVT_WordRange wrVisible = GetVisibleWordRange();
		Refresh(RP_OPTIONAL,&wrVisible);
		SetCaretInfo();
	}
}

void CFX_Edit::SelectNone()
{
	if (m_pVT->IsValid())
	{
		if (m_SelState.IsExist())
		{
			CPVT_WordRange wrTemp = m_SelState.ConvertToWordRange();
			m_SelState.Default();
			Refresh(RP_OPTIONAL,&wrTemp);
		}
	}	
}

FX_BOOL	CFX_Edit::IsSelected() const
{
	return m_SelState.IsExist();
}

CPDF_Point CFX_Edit::VTToEdit(const CPDF_Point & point) const
{
	CPDF_Rect rcContent = m_pVT->GetContentRect();
	CPDF_Rect rcPlate = m_pVT->GetPlateRect();

	FX_FLOAT fPadding = 0.0f;

	switch (m_nAlignment)
	{
	case 0:
		fPadding = 0.0f;
		break;
	case 1:
		fPadding = (rcPlate.Height() - rcContent.Height()) * 0.5f;
		break;
	case 2:
		fPadding = rcPlate.Height() - rcContent.Height();
		break;
	}
	
	return CPDF_Point(point.x - (m_ptScrollPos.x - rcPlate.left),
		point.y - (m_ptScrollPos.y + fPadding - rcPlate.top));
}

CPDF_Point CFX_Edit::EditToVT(const CPDF_Point & point) const
{
	CPDF_Rect rcContent = m_pVT->GetContentRect();
	CPDF_Rect rcPlate = m_pVT->GetPlateRect();

	FX_FLOAT fPadding = 0.0f;

	switch (m_nAlignment)
	{
	case 0:
		fPadding = 0.0f;
		break;
	case 1:
		fPadding = (rcPlate.Height() - rcContent.Height()) * 0.5f;
		break;
	case 2:
		fPadding = rcPlate.Height() - rcContent.Height();
		break;
	}

	return CPDF_Point(point.x + (m_ptScrollPos.x - rcPlate.left),
		point.y + (m_ptScrollPos.y + fPadding - rcPlate.top));
}

CPDF_Rect CFX_Edit::VTToEdit(const CPDF_Rect & rect) const
{
	CPDF_Point ptLeftBottom = VTToEdit(CPDF_Point(rect.left,rect.bottom));
	CPDF_Point ptRightTop = VTToEdit(CPDF_Point(rect.right,rect.top));

	return CPDF_Rect(ptLeftBottom.x,ptLeftBottom.y,ptRightTop.x,ptRightTop.y);
}

CPDF_Rect CFX_Edit::EditToVT(const CPDF_Rect & rect) const
{
	CPDF_Point ptLeftBottom = EditToVT(CPDF_Point(rect.left,rect.bottom));
	CPDF_Point ptRightTop = EditToVT(CPDF_Point(rect.right,rect.top));

	return CPDF_Rect(ptLeftBottom.x,ptLeftBottom.y,ptRightTop.x,ptRightTop.y);
}

void CFX_Edit::SetScrollInfo()
{
	if (m_bNotify && m_pNotify)
	{
		CPDF_Rect rcPlate = m_pVT->GetPlateRect();
		CPDF_Rect rcContent = m_pVT->GetContentRect();

		if (!m_bNotifyFlag)
		{
			m_bNotifyFlag = TRUE;
			m_pNotify->IOnSetScrollInfoX(rcPlate.left, rcPlate.right, 
								rcContent.left, rcContent.right, rcPlate.Width() / 3, rcPlate.Width());
			
			m_pNotify->IOnSetScrollInfoY(rcPlate.bottom, rcPlate.top, 
					rcContent.bottom, rcContent.top, rcPlate.Height() / 3, rcPlate.Height());
			m_bNotifyFlag = FALSE;
		}
	}
}

void CFX_Edit::SetScrollPosX(FX_FLOAT fx)
{
	if (!m_bEnableScroll) return;

	if (m_pVT->IsValid())
	{
		if (!FX_EDIT_IsFloatEqual(m_ptScrollPos.x,fx))
		{
			m_ptScrollPos.x = fx;			
			Refresh(RP_NOANALYSE);

			if (m_bNotify && m_pNotify)
			{
				if (!m_bNotifyFlag)
				{
					m_bNotifyFlag = TRUE;
					m_pNotify->IOnSetScrollPosX(fx);
					m_bNotifyFlag = FALSE;
				}
			}
		}
	}
}

void CFX_Edit::SetScrollPosY(FX_FLOAT fy)
{
	if (!m_bEnableScroll) return;

	if (m_pVT->IsValid())
	{
		if (!FX_EDIT_IsFloatEqual(m_ptScrollPos.y,fy))
		{				
			m_ptScrollPos.y = fy;
			Refresh(RP_NOANALYSE);

			if (m_bNotify && m_pNotify)
			{
				if (!m_bNotifyFlag)
				{
					m_bNotifyFlag = TRUE;
					m_pNotify->IOnSetScrollPosY(fy);
					m_bNotifyFlag = FALSE;
				}
			}
		}
	}
}

void CFX_Edit::SetScrollPos(const CPDF_Point & point)
{
	SetScrollPosX(point.x);
	SetScrollPosY(point.y);
	SetScrollLimit();
	SetCaretInfo();
}

CPDF_Point CFX_Edit::GetScrollPos() const
{
	return m_ptScrollPos;
}

void CFX_Edit::SetScrollLimit()
{
	if (m_pVT->IsValid())
	{
		CPDF_Rect rcContent = m_pVT->GetContentRect();
		CPDF_Rect rcPlate = m_pVT->GetPlateRect();

		if (rcPlate.Width() > rcContent.Width())
		{
			SetScrollPosX(rcPlate.left);
		}
		else
		{
			if (FX_EDIT_IsFloatSmaller(m_ptScrollPos.x, rcContent.left))
			{
				SetScrollPosX(rcContent.left);			
			}
			else if (FX_EDIT_IsFloatBigger(m_ptScrollPos.x, rcContent.right - rcPlate.Width()))
			{
				SetScrollPosX(rcContent.right - rcPlate.Width());
			}
		}

		if (rcPlate.Height() > rcContent.Height())
		{
			SetScrollPosY(rcPlate.top);
		}
		else		
		{
			if (FX_EDIT_IsFloatSmaller(m_ptScrollPos.y, rcContent.bottom + rcPlate.Height()))
			{
				SetScrollPosY(rcContent.bottom + rcPlate.Height());
			}
			else if (FX_EDIT_IsFloatBigger(m_ptScrollPos.y, rcContent.top))
			{
				SetScrollPosY(rcContent.top);
			}
		}
	}
}

void CFX_Edit::ScrollToCaret()
{
	SetScrollLimit();

	if (m_pVT->IsValid())
	{
		CPDF_Point ptHead(0,0);
		CPDF_Point ptFoot(0,0);

		if (IPDF_VariableText_Iterator * pIterator = m_pVT->GetIterator())
		{
			pIterator->SetAt(m_wpCaret);

			CPVT_Word word;
			CPVT_Line line;
			if (pIterator->GetWord(word))
			{
				ptHead.x = word.ptWord.x + word.fWidth;
				ptHead.y = word.ptWord.y + word.fAscent;
				ptFoot.x = word.ptWord.x + word.fWidth;
				ptFoot.y = word.ptWord.y + word.fDescent;
			}
			else if (pIterator->GetLine(line))
			{				
				ptHead.x = line.ptLine.x;
				ptHead.y = line.ptLine.y + line.fLineAscent;
				ptFoot.x = line.ptLine.x;
				ptFoot.y = line.ptLine.y + line.fLineDescent;
			}
		}

		CPDF_Point ptHeadEdit = VTToEdit(ptHead);
		CPDF_Point ptFootEdit = VTToEdit(ptFoot);

		CPDF_Rect rcPlate = m_pVT->GetPlateRect();

		if (!FX_EDIT_IsFloatEqual(rcPlate.left,rcPlate.right))
		{
			if (FX_EDIT_IsFloatSmaller(ptHeadEdit.x, rcPlate.left) ||
				FX_EDIT_IsFloatEqual(ptHeadEdit.x, rcPlate.left))
			{
				SetScrollPosX(ptHead.x);
			}
			else if (FX_EDIT_IsFloatBigger(ptHeadEdit.x, rcPlate.right))
			{
				SetScrollPosX(ptHead.x - rcPlate.Width());
			}
		}

		if (!FX_EDIT_IsFloatEqual(rcPlate.top,rcPlate.bottom))
		{
			if (FX_EDIT_IsFloatSmaller(ptFootEdit.y, rcPlate.bottom) ||
				FX_EDIT_IsFloatEqual(ptFootEdit.y, rcPlate.bottom))
			{
				if (FX_EDIT_IsFloatSmaller(ptHeadEdit.y, rcPlate.top))
				{
					SetScrollPosY(ptFoot.y + rcPlate.Height());
				}
			}
			else if (FX_EDIT_IsFloatBigger(ptHeadEdit.y, rcPlate.top))
			{
				if (FX_EDIT_IsFloatBigger(ptFootEdit.y, rcPlate.bottom))
				{
					SetScrollPosY(ptHead.y);
				}
			}
		}
	}
}

void CFX_Edit::Refresh(REFRESH_PLAN_E ePlan,const CPVT_WordRange * pRange1,const CPVT_WordRange * pRange2)
{
	if (m_bEnableRefresh && m_pVT->IsValid())
	{
		m_Refresh.BeginRefresh();
		RefreshPushLineRects(GetVisibleWordRange());

// 		if (!FX_EDIT_IsFloatEqual(m_ptRefreshScrollPos.x,m_ptScrollPos.x) || 
// 			!FX_EDIT_IsFloatEqual(m_ptRefreshScrollPos.y,m_ptScrollPos.y))
// 		{
			m_Refresh.NoAnalyse();
			m_ptRefreshScrollPos = m_ptScrollPos;
// 		}
// 		else
// 		{
// 			switch (ePlan)
// 			{
// 			case RP_ANALYSE:
// 				m_Refresh.Analyse(m_pVT->GetAlignment());
// 
// 				if (pRange1) RefreshPushRandomRects(*pRange1);
// 				if (pRange2) RefreshPushRandomRects(*pRange2);
// 				break;
// 			case RP_NOANALYSE:
// 				m_Refresh.NoAnalyse();
// 				break;
// 			case RP_OPTIONAL:
// 				if (pRange1) RefreshPushRandomRects(*pRange1);
// 				if (pRange2) RefreshPushRandomRects(*pRange2);
// 				break;	
// 			}
// 		}		

		if (m_bNotify && m_pNotify)
		{
			if (!m_bNotifyFlag)
			{
				m_bNotifyFlag = TRUE;
				if (const CFX_Edit_RectArray * pRects = m_Refresh.GetRefreshRects())
				{
					for (FX_INT32 i = 0, sz = pRects->GetSize(); i < sz; i++)
						m_pNotify->IOnInvalidateRect(pRects->GetAt(i));
				}
				m_bNotifyFlag = FALSE;
			}
		}

		m_Refresh.EndRefresh();
	}
}

void CFX_Edit::RefreshPushLineRects(const CPVT_WordRange & wr)
{
	if (m_pVT->IsValid())
	{
		if (IPDF_VariableText_Iterator * pIterator = m_pVT->GetIterator())
		{
			CPVT_WordPlace wpBegin = wr.BeginPos;
			m_pVT->UpdateWordPlace(wpBegin);
			CPVT_WordPlace wpEnd = wr.EndPos;
			m_pVT->UpdateWordPlace(wpEnd);
			pIterator->SetAt(wpBegin);

			CPVT_Line lineinfo;	
			do
			{
				if (!pIterator->GetLine(lineinfo))break;
				if (lineinfo.lineplace.LineCmp(wpEnd) > 0)break;

				CPDF_Rect rcLine(lineinfo.ptLine.x,
									lineinfo.ptLine.y + lineinfo.fLineDescent,
									lineinfo.ptLine.x + lineinfo.fLineWidth,
									lineinfo.ptLine.y + lineinfo.fLineAscent);

				m_Refresh.Push(CPVT_WordRange(lineinfo.lineplace,lineinfo.lineEnd),VTToEdit(rcLine));

			}while (pIterator->NextLine());
		}
	}
}

void CFX_Edit::RefreshPushRandomRects(const CPVT_WordRange & wr)
{
	if (m_pVT->IsValid())
	{
		if (IPDF_VariableText_Iterator * pIterator = m_pVT->GetIterator())
		{
			CPVT_WordRange wrTemp = wr;

			m_pVT->UpdateWordPlace(wrTemp.BeginPos);
			m_pVT->UpdateWordPlace(wrTemp.EndPos);
			pIterator->SetAt(wrTemp.BeginPos);

			CPVT_Word wordinfo;	
			CPVT_Line lineinfo;	
			CPVT_WordPlace place;

			while (pIterator->NextWord())
			{
				place = pIterator->GetAt();
				if (place.WordCmp(wrTemp.EndPos) > 0) break;
						
				pIterator->GetWord(wordinfo);
				pIterator->GetLine(lineinfo);

				if (place.LineCmp(wrTemp.BeginPos) == 0 || place.LineCmp(wrTemp.EndPos) == 0)
				{
					CPDF_Rect rcWord(wordinfo.ptWord.x,
										lineinfo.ptLine.y + lineinfo.fLineDescent,
										wordinfo.ptWord.x + wordinfo.fWidth,
										lineinfo.ptLine.y + lineinfo.fLineAscent);

					m_Refresh.AddRefresh(VTToEdit(rcWord));
				}
				else
				{		
					CPDF_Rect rcLine(lineinfo.ptLine.x,
										lineinfo.ptLine.y + lineinfo.fLineDescent,
										lineinfo.ptLine.x + lineinfo.fLineWidth,
										lineinfo.ptLine.y + lineinfo.fLineAscent);

					m_Refresh.AddRefresh(VTToEdit(rcLine));

					pIterator->NextLine();
				}
			}
		}
	}
}

void CFX_Edit::RefreshWordRange(const CPVT_WordRange& wr)
{
	if (IPDF_VariableText_Iterator * pIterator = m_pVT->GetIterator())
	{
		CPVT_WordRange wrTemp = wr;

		m_pVT->UpdateWordPlace(wrTemp.BeginPos);
		m_pVT->UpdateWordPlace(wrTemp.EndPos);
		pIterator->SetAt(wrTemp.BeginPos);

		CPVT_Word wordinfo;	
		CPVT_Line lineinfo;	
		CPVT_WordPlace place;

		while (pIterator->NextWord())
		{
			place = pIterator->GetAt();
			if (place.WordCmp(wrTemp.EndPos) > 0) break;
					
			pIterator->GetWord(wordinfo);
			pIterator->GetLine(lineinfo);

			if (place.LineCmp(wrTemp.BeginPos) == 0 || place.LineCmp(wrTemp.EndPos) == 0)
			{
				CPDF_Rect rcWord(wordinfo.ptWord.x,
									lineinfo.ptLine.y + lineinfo.fLineDescent,
									wordinfo.ptWord.x + wordinfo.fWidth,
									lineinfo.ptLine.y + lineinfo.fLineAscent);

				if (m_bNotify && m_pNotify)
				{
					if (!m_bNotifyFlag)
					{
						m_bNotifyFlag = TRUE;
						CPDF_Rect rcRefresh = VTToEdit(rcWord);
						m_pNotify->IOnInvalidateRect(&rcRefresh);
						m_bNotifyFlag = FALSE;
					}
				}
			}
			else
			{		
				CPDF_Rect rcLine(lineinfo.ptLine.x,
									lineinfo.ptLine.y + lineinfo.fLineDescent,
									lineinfo.ptLine.x + lineinfo.fLineWidth,
									lineinfo.ptLine.y + lineinfo.fLineAscent);

				if (m_bNotify && m_pNotify)
				{
					if (!m_bNotifyFlag)
					{
						m_bNotifyFlag = TRUE;
						CPDF_Rect rcRefresh = VTToEdit(rcLine);
						m_pNotify->IOnInvalidateRect(&rcRefresh);
						m_bNotifyFlag = FALSE;
					}
				}

				pIterator->NextLine();
			}
		}
	}
}

void CFX_Edit::SetCaret(const CPVT_WordPlace & place)
{
	m_wpOldCaret = m_wpCaret; 
	m_wpCaret = place;	
}

void CFX_Edit::SetCaretInfo()
{
	if (m_bNotify && m_pNotify)
	{
		if (!m_bNotifyFlag)
		{
			CPDF_Point ptHead(0.0f,0.0f),ptFoot(0.0f,0.0f);

			if (IPDF_VariableText_Iterator * pIterator = m_pVT->GetIterator())
			{
				pIterator->SetAt(m_wpCaret);
				CPVT_Word word;
				CPVT_Line line;
				if (pIterator->GetWord(word))
				{
					ptHead.x = word.ptWord.x + word.fWidth;
					ptHead.y = word.ptWord.y + word.fAscent;
					ptFoot.x = word.ptWord.x + word.fWidth;
					ptFoot.y = word.ptWord.y + word.fDescent;
				}
				else if (pIterator->GetLine(line))
				{				
					ptHead.x = line.ptLine.x;
					ptHead.y = line.ptLine.y + line.fLineAscent;
					ptFoot.x = line.ptLine.x;
					ptFoot.y = line.ptLine.y + line.fLineDescent;
				}
			}		

			m_bNotifyFlag = TRUE;
			m_pNotify->IOnSetCaret(!m_SelState.IsExist(),VTToEdit(ptHead),VTToEdit(ptFoot), m_wpCaret);
			m_bNotifyFlag = FALSE;
		}
	}

	SetCaretChange();
}

void CFX_Edit::SetCaretChange()
{
	if (this->m_wpCaret == this->m_wpOldCaret) return;

	if (m_bNotify && m_pVT->IsRichText() && m_pNotify)
	{
		CPVT_SecProps SecProps;
		CPVT_WordProps WordProps;

		if (IPDF_VariableText_Iterator * pIterator = m_pVT->GetIterator())
		{
			pIterator->SetAt(m_wpCaret);
			CPVT_Word word;
			CPVT_Section section;

			if (pIterator->GetSection(section))
			{
				SecProps = section.SecProps;
				WordProps = section.WordProps;
			}
			
			if (pIterator->GetWord(word))
			{				
				WordProps = word.WordProps;				
			}
		}		
		
		if (!m_bNotifyFlag)
		{
			m_bNotifyFlag = TRUE;
			m_pNotify->IOnCaretChange(SecProps,WordProps);
			m_bNotifyFlag = FALSE;
		}
	}	
}

void CFX_Edit::SetCaret(FX_INT32 nPos)
{
	if (m_pVT->IsValid())
	{
		SelectNone();		
		SetCaret(m_pVT->WordIndexToWordPlace(nPos));
		m_SelState.Set(m_wpCaret,m_wpCaret);

		ScrollToCaret();
		SetCaretOrigin();
		SetCaretInfo();
	}
}

void CFX_Edit::OnMouseDown(const CPDF_Point & point,FX_BOOL bShift,FX_BOOL bCtrl)
{
	if (m_pVT->IsValid())
	{
		SelectNone();		
		SetCaret(m_pVT->SearchWordPlace(EditToVT(point)));
		m_SelState.Set(m_wpCaret,m_wpCaret);

		ScrollToCaret();
		SetCaretOrigin();
		SetCaretInfo();
	}
}

void CFX_Edit::OnMouseMove(const CPDF_Point & point,FX_BOOL bShift,FX_BOOL bCtrl)
{
	if (m_pVT->IsValid())
	{
		SetCaret(m_pVT->SearchWordPlace(EditToVT(point)));

		if (m_wpCaret != m_wpOldCaret)
		{
			m_SelState.SetEndPos(m_wpCaret);		

			ScrollToCaret();
			CPVT_WordRange wr(m_wpOldCaret,m_wpCaret);
			Refresh(RP_OPTIONAL,&wr);
			SetCaretOrigin();
			SetCaretInfo();
		}
	}
}

void CFX_Edit::OnVK_UP(FX_BOOL bShift,FX_BOOL bCtrl)
{	
	if (m_pVT->IsValid())
	{
		SetCaret(m_pVT->GetUpWordPlace(m_wpCaret,m_ptCaret));

		if (bShift)
		{
			if (m_SelState.IsExist())
				m_SelState.SetEndPos(m_wpCaret);
			else
				m_SelState.Set(m_wpOldCaret,m_wpCaret);

			if (m_wpOldCaret != m_wpCaret)
			{
				ScrollToCaret();
				CPVT_WordRange wr(m_wpOldCaret, m_wpCaret);
				Refresh(RP_OPTIONAL, &wr);
				SetCaretInfo();
			}
		}
		else
		{
			SelectNone();		
			
			ScrollToCaret();			
			SetCaretInfo();
		}
	}
}

void CFX_Edit::OnVK_DOWN(FX_BOOL bShift,FX_BOOL bCtrl)
{
	if (m_pVT->IsValid())
	{
		SetCaret(m_pVT->GetDownWordPlace(m_wpCaret,m_ptCaret));

		if (bShift)
		{
			if (m_SelState.IsExist())
				m_SelState.SetEndPos(m_wpCaret);
			else
				m_SelState.Set(m_wpOldCaret,m_wpCaret);

			if (m_wpOldCaret != m_wpCaret)
			{
				ScrollToCaret();
				CPVT_WordRange wr(m_wpOldCaret,m_wpCaret);
				Refresh(RP_OPTIONAL, &wr);
				SetCaretInfo();
			}
		}
		else
		{
			SelectNone();

			ScrollToCaret();		
			SetCaretInfo();
		}
	}
}

void CFX_Edit::OnVK_LEFT(FX_BOOL bShift,FX_BOOL bCtrl)
{
	if (m_pVT->IsValid())
	{
		if (bShift)
		{
			if (m_wpCaret == m_pVT->GetLineBeginPlace(m_wpCaret) &&
				m_wpCaret != m_pVT->GetSectionBeginPlace(m_wpCaret))
				SetCaret(m_pVT->GetPrevWordPlace(m_wpCaret));	

			SetCaret(m_pVT->GetPrevWordPlace(m_wpCaret));

			if (m_SelState.IsExist())
				m_SelState.SetEndPos(m_wpCaret);			
			else
				m_SelState.Set(m_wpOldCaret, m_wpCaret);

			if (m_wpOldCaret != m_wpCaret)
			{
				ScrollToCaret();
				CPVT_WordRange wr(m_wpOldCaret,m_wpCaret);
				Refresh(RP_OPTIONAL,&wr);
				SetCaretInfo();
			}
		}
		else
		{
			if (m_SelState.IsExist())
			{
				if (m_SelState.BeginPos.WordCmp(m_SelState.EndPos)<0)
					SetCaret(m_SelState.BeginPos);
				else
					SetCaret(m_SelState.EndPos);

				SelectNone();
				ScrollToCaret();
				SetCaretInfo();
			}
			else
			{
				if (m_wpCaret == m_pVT->GetLineBeginPlace(m_wpCaret) &&
					m_wpCaret != m_pVT->GetSectionBeginPlace(m_wpCaret))
					SetCaret(m_pVT->GetPrevWordPlace(m_wpCaret));	

				SetCaret(m_pVT->GetPrevWordPlace(m_wpCaret));

				ScrollToCaret();
				SetCaretOrigin();
				SetCaretInfo();
			}
		}
	}
}

void CFX_Edit::OnVK_RIGHT(FX_BOOL bShift,FX_BOOL bCtrl)
{
	if (m_pVT->IsValid())
	{
		if (bShift)
		{
			SetCaret(m_pVT->GetNextWordPlace(m_wpCaret));

			if (m_wpCaret == m_pVT->GetLineEndPlace(m_wpCaret) &&
				m_wpCaret != m_pVT->GetSectionEndPlace(m_wpCaret))
				SetCaret(m_pVT->GetNextWordPlace(m_wpCaret));

			if (m_SelState.IsExist())
				m_SelState.SetEndPos(m_wpCaret);			
			else
				m_SelState.Set(m_wpOldCaret,m_wpCaret);			

			if (m_wpOldCaret != m_wpCaret)
			{			
				ScrollToCaret();
				CPVT_WordRange wr(m_wpOldCaret,m_wpCaret);
				Refresh(RP_OPTIONAL,&wr);
				SetCaretInfo();
			}
		}
		else
		{
			if (m_SelState.IsExist())
			{
				if (m_SelState.BeginPos.WordCmp(m_SelState.EndPos)>0)
					SetCaret(m_SelState.BeginPos);
				else
					SetCaret(m_SelState.EndPos);

				SelectNone();
				ScrollToCaret();
				SetCaretInfo();
			}
			else
			{
				SetCaret(m_pVT->GetNextWordPlace(m_wpCaret));

				if (m_wpCaret == m_pVT->GetLineEndPlace(m_wpCaret) &&
					m_wpCaret != m_pVT->GetSectionEndPlace(m_wpCaret))
					SetCaret(m_pVT->GetNextWordPlace(m_wpCaret));				

				ScrollToCaret();
				SetCaretOrigin();
				SetCaretInfo();
			}
		}
	}
}

void CFX_Edit::OnVK_HOME(FX_BOOL bShift,FX_BOOL bCtrl)
{
	if (m_pVT->IsValid())
	{
		if (bShift)
		{
			if (bCtrl)
				SetCaret(m_pVT->GetBeginWordPlace());
			else
				SetCaret(m_pVT->GetLineBeginPlace(m_wpCaret));

			if (m_SelState.IsExist())
				m_SelState.SetEndPos(m_wpCaret);			
			else
				m_SelState.Set(m_wpOldCaret,m_wpCaret);

			ScrollToCaret();
			CPVT_WordRange wr(m_wpOldCaret, m_wpCaret);
			Refresh(RP_OPTIONAL, &wr);
			SetCaretInfo();
		}
		else
		{
			if (m_SelState.IsExist())
			{
				if (m_SelState.BeginPos.WordCmp(m_SelState.EndPos)<0)
					SetCaret(m_SelState.BeginPos);
				else
					SetCaret(m_SelState.EndPos);

				SelectNone();
				ScrollToCaret();
				SetCaretInfo();
			}
			else
			{
				if (bCtrl)
					SetCaret(m_pVT->GetBeginWordPlace());
				else
					SetCaret(m_pVT->GetLineBeginPlace(m_wpCaret));	
				
				ScrollToCaret();
				SetCaretOrigin();
				SetCaretInfo();
			}
		}
	}
}

void CFX_Edit::OnVK_END(FX_BOOL bShift,FX_BOOL bCtrl)
{
	if (m_pVT->IsValid())
	{
		if (bShift)
		{
			if (bCtrl)
				SetCaret(m_pVT->GetEndWordPlace());
			else
				SetCaret(m_pVT->GetLineEndPlace(m_wpCaret));

			if (m_SelState.IsExist())
				m_SelState.SetEndPos(m_wpCaret);
			else
				m_SelState.Set(m_wpOldCaret, m_wpCaret);

			ScrollToCaret();
			CPVT_WordRange wr(m_wpOldCaret, m_wpCaret);
			Refresh(RP_OPTIONAL, &wr);
			SetCaretInfo();
		}
		else
		{
			if (m_SelState.IsExist())
			{
				if (m_SelState.BeginPos.WordCmp(m_SelState.EndPos)>0)
					SetCaret(m_SelState.BeginPos);
				else
					SetCaret(m_SelState.EndPos);

				SelectNone();
				ScrollToCaret();
				SetCaretInfo();
			}
			else
			{
				if (bCtrl)
					SetCaret(m_pVT->GetEndWordPlace());
				else
					SetCaret(m_pVT->GetLineEndPlace(m_wpCaret));
						
				ScrollToCaret();			
				SetCaretOrigin();
				SetCaretInfo();
			}
		}
	}
}

void CFX_Edit::SetText(FX_LPCWSTR text,FX_INT32 charset,
						const CPVT_SecProps * pSecProps,const CPVT_WordProps * pWordProps, FX_BOOL bAddUndo, FX_BOOL bPaint)
{
	Empty();
	DoInsertText(CPVT_WordPlace(0,0,-1), text, charset, pSecProps, pWordProps);
	if (bPaint) Paint();
	if (m_bOprNotify && m_pOprNotify)
		m_pOprNotify->OnSetText(m_wpCaret, m_wpOldCaret);
	//if (bAddUndo)
}

FX_BOOL CFX_Edit::InsertWord(FX_WORD word, FX_INT32 charset, const CPVT_WordProps * pWordProps, FX_BOOL bAddUndo, FX_BOOL bPaint)
{
	if (IsTextOverflow()) return FALSE;

	if (m_pVT->IsValid())
	{
		m_pVT->UpdateWordPlace(m_wpCaret);

		SetCaret(m_pVT->InsertWord(m_wpCaret,word,GetCharSetFromUnicode(word, charset),pWordProps));
		m_SelState.Set(m_wpCaret,m_wpCaret);

		if (m_wpCaret != m_wpOldCaret)
		{
			if (bAddUndo && m_bEnableUndo)
			{
				AddEditUndoItem(new CFXEU_InsertWord(this,m_wpOldCaret,m_wpCaret,word,charset,pWordProps));
			}

			if (bPaint)
				PaintInsertText(m_wpOldCaret, m_wpCaret);

			if (m_bOprNotify && m_pOprNotify)
				m_pOprNotify->OnInsertWord(m_wpCaret, m_wpOldCaret);

			return TRUE;
		}
	}
	
	return FALSE;
}

FX_BOOL CFX_Edit::InsertReturn(const CPVT_SecProps * pSecProps,const CPVT_WordProps * pWordProps, 
							   FX_BOOL bAddUndo, FX_BOOL bPaint)
{
	if (IsTextOverflow()) return FALSE;

	if (m_pVT->IsValid())
	{
		m_pVT->UpdateWordPlace(m_wpCaret);
		SetCaret(m_pVT->InsertSection(m_wpCaret,pSecProps,pWordProps));
		m_SelState.Set(m_wpCaret,m_wpCaret);

		if (m_wpCaret != m_wpOldCaret)
		{
			if (bAddUndo && m_bEnableUndo)
			{
				AddEditUndoItem(new CFXEU_InsertReturn(this,m_wpOldCaret,m_wpCaret,pSecProps,pWordProps));
			}

			if (bPaint)
			{
				RearrangePart(CPVT_WordRange(m_wpOldCaret, m_wpCaret));
				ScrollToCaret();
				CPVT_WordRange wr(m_wpOldCaret, GetVisibleWordRange().EndPos);
				Refresh(RP_ANALYSE, &wr);		
				SetCaretOrigin();
				SetCaretInfo();			
			}

			if (m_bOprNotify && m_pOprNotify)
				m_pOprNotify->OnInsertReturn(m_wpCaret, m_wpOldCaret);

			return TRUE;
		}
	}

	return FALSE;
}

FX_BOOL CFX_Edit::Backspace(FX_BOOL bAddUndo, FX_BOOL bPaint)
{
	if (m_pVT->IsValid())
	{
		if (m_wpCaret == m_pVT->GetBeginWordPlace()) return FALSE;

		CPVT_Section section;
		CPVT_Word word;

		if (bAddUndo)
		{
			if (IPDF_VariableText_Iterator * pIterator = m_pVT->GetIterator())
			{				
				pIterator->SetAt(m_wpCaret);
				pIterator->GetSection(section);
				pIterator->GetWord(word);
			}
		}

		m_pVT->UpdateWordPlace(m_wpCaret);
		SetCaret(m_pVT->BackSpaceWord(m_wpCaret));
		m_SelState.Set(m_wpCaret,m_wpCaret);

		if (m_wpCaret != m_wpOldCaret)
		{
			if (bAddUndo && m_bEnableUndo)
			{
				if (m_wpCaret.SecCmp(m_wpOldCaret) != 0)			
					AddEditUndoItem(new CFXEU_Backspace(this,m_wpOldCaret,m_wpCaret,word.Word,word.nCharset,
						section.SecProps,section.WordProps));
				else
					AddEditUndoItem(new CFXEU_Backspace(this,m_wpOldCaret,m_wpCaret,word.Word,word.nCharset,
						section.SecProps,word.WordProps));
			}

			if (bPaint)
			{
				RearrangePart(CPVT_WordRange(m_wpCaret,m_wpOldCaret));
				ScrollToCaret();

				CPVT_WordRange wr;
				if (m_wpCaret.SecCmp(m_wpOldCaret) !=0)
					wr = CPVT_WordRange(m_pVT->GetPrevWordPlace(m_wpCaret),GetVisibleWordRange().EndPos);	
				else if (m_wpCaret.LineCmp(m_wpOldCaret) !=0)
					wr = CPVT_WordRange(m_pVT->GetLineBeginPlace(m_wpCaret),m_pVT->GetSectionEndPlace(m_wpCaret));
				else
					wr = CPVT_WordRange(m_pVT->GetPrevWordPlace(m_wpCaret),m_pVT->GetSectionEndPlace(m_wpCaret));

				Refresh(RP_ANALYSE, &wr);

				SetCaretOrigin();
				SetCaretInfo();
			}

			if (m_bOprNotify && m_pOprNotify)
				m_pOprNotify->OnBackSpace(m_wpCaret, m_wpOldCaret);

			return TRUE;
		}
	}

	return FALSE;
}

FX_BOOL CFX_Edit::Delete(FX_BOOL bAddUndo, FX_BOOL bPaint)
{
	if (m_pVT->IsValid())
	{
		if (m_wpCaret == m_pVT->GetEndWordPlace()) return FALSE;

		CPVT_Section section;
		CPVT_Word word;

		if (bAddUndo)
		{
			if (IPDF_VariableText_Iterator * pIterator = m_pVT->GetIterator())
			{				
				pIterator->SetAt(m_pVT->GetNextWordPlace(m_wpCaret));
				pIterator->GetSection(section);
				pIterator->GetWord(word);
			}
		}

		m_pVT->UpdateWordPlace(m_wpCaret);
		FX_BOOL bSecEnd = (m_wpCaret == m_pVT->GetSectionEndPlace(m_wpCaret));

		SetCaret(m_pVT->DeleteWord(m_wpCaret));
		m_SelState.Set(m_wpCaret,m_wpCaret);

		if (bAddUndo && m_bEnableUndo)
		{
			if (bSecEnd)		
				AddEditUndoItem(new CFXEU_Delete(this,m_wpOldCaret,m_wpCaret,word.Word,word.nCharset,
					section.SecProps,section.WordProps,bSecEnd));
			else
				AddEditUndoItem(new CFXEU_Delete(this,m_wpOldCaret,m_wpCaret,word.Word,word.nCharset,
					section.SecProps,word.WordProps,bSecEnd));
		}
		
		if (bPaint)
		{
			RearrangePart(CPVT_WordRange(m_wpOldCaret,m_wpCaret));
			ScrollToCaret();

			CPVT_WordRange wr;
			if (bSecEnd)
				wr = CPVT_WordRange(m_pVT->GetPrevWordPlace(m_wpOldCaret),GetVisibleWordRange().EndPos);
			else if (m_wpCaret.LineCmp(m_wpOldCaret) !=0)
				wr = CPVT_WordRange(m_pVT->GetLineBeginPlace(m_wpCaret),m_pVT->GetSectionEndPlace(m_wpCaret));
			else
				wr = CPVT_WordRange(m_pVT->GetPrevWordPlace(m_wpOldCaret),m_pVT->GetSectionEndPlace(m_wpCaret));			

			Refresh(RP_ANALYSE, &wr);
			
			SetCaretOrigin();
			SetCaretInfo();
		}

		if (m_bOprNotify && m_pOprNotify)
			m_pOprNotify->OnDelete(m_wpCaret, m_wpOldCaret);

		return TRUE;
	}

	return FALSE;
}

FX_BOOL	CFX_Edit::Empty()
{
	if (m_pVT->IsValid())
	{
		m_pVT->DeleteWords(GetWholeWordRange());
		SetCaret(m_pVT->GetBeginWordPlace());

		return TRUE;
	}

	return FALSE;
}

FX_BOOL CFX_Edit::Clear(FX_BOOL bAddUndo, FX_BOOL bPaint)
{
	if (m_pVT->IsValid())
	{
		if (m_SelState.IsExist())
		{
			CPVT_WordRange range = m_SelState.ConvertToWordRange();

			if (bAddUndo && m_bEnableUndo)
			{
				if (m_pVT->IsRichText())
				{
					BeginGroupUndo(L"");

					if (IPDF_VariableText_Iterator * pIterator = m_pVT->GetIterator())
					{
						pIterator->SetAt(range.EndPos);

						CPVT_Word wordinfo;	
						CPVT_Section secinfo;						
						do
						{							
							CPVT_WordPlace place = pIterator->GetAt();
							if (place.WordCmp(range.BeginPos) <= 0)break;

							CPVT_WordPlace oldplace = m_pVT->GetPrevWordPlace(place);							

							if (oldplace.SecCmp(place) != 0)
							{
								if (pIterator->GetSection(secinfo))
								{
									AddEditUndoItem(new CFXEU_ClearRich(this,oldplace,place,range,wordinfo.Word,
										wordinfo.nCharset,secinfo.SecProps,secinfo.WordProps));
								}
							}
							else
							{
								if (pIterator->GetWord(wordinfo))
								{									
									oldplace = m_pVT->AjustLineHeader(oldplace,TRUE);
									place = m_pVT->AjustLineHeader(place,TRUE);

									AddEditUndoItem(new CFXEU_ClearRich(this,oldplace,place,range,wordinfo.Word,
										wordinfo.nCharset,secinfo.SecProps,wordinfo.WordProps));
								}
							}
						}while (pIterator->PrevWord());
					}
					EndGroupUndo();
				}
				else
				{					
					AddEditUndoItem(new CFXEU_Clear(this,range,GetSelText()));					
				}
			}

			SelectNone();			
			SetCaret(m_pVT->DeleteWords(range));	
			m_SelState.Set(m_wpCaret,m_wpCaret);

			if (bPaint)
			{
				RearrangePart(range);	
				ScrollToCaret();

				CPVT_WordRange wr(m_wpOldCaret, GetVisibleWordRange().EndPos);
				Refresh(RP_ANALYSE, &wr);
				
				SetCaretOrigin();
				SetCaretInfo();
			}

			if (m_bOprNotify && m_pOprNotify)
				m_pOprNotify->OnClear(m_wpCaret, m_wpOldCaret);

			return TRUE;
		}
	}

	return FALSE;
}

FX_BOOL CFX_Edit::InsertText(FX_LPCWSTR text, FX_INT32 charset,
					const CPVT_SecProps * pSecProps, const CPVT_WordProps * pWordProps, FX_BOOL bAddUndo, FX_BOOL bPaint)
{
	if (IsTextOverflow()) return FALSE;

	m_pVT->UpdateWordPlace(m_wpCaret);
	SetCaret(DoInsertText(m_wpCaret, text, charset, pSecProps, pWordProps));
	m_SelState.Set(m_wpCaret,m_wpCaret);

	if (m_wpCaret != m_wpOldCaret)
	{
		if (bAddUndo && m_bEnableUndo)
		{
			AddEditUndoItem(new CFXEU_InsertText(this,m_wpOldCaret,m_wpCaret,text,charset,pSecProps,pWordProps));
		}

		if (bPaint)
			PaintInsertText(m_wpOldCaret, m_wpCaret);

		if (m_bOprNotify && m_pOprNotify)
			m_pOprNotify->OnInsertText(m_wpCaret, m_wpOldCaret);

		return TRUE;
	}
	return FALSE;
}

void CFX_Edit::PaintInsertText(const CPVT_WordPlace & wpOld, const CPVT_WordPlace & wpNew)
{
	if (m_pVT->IsValid())
	{
		RearrangePart(CPVT_WordRange(wpOld,wpNew));
		ScrollToCaret();
		
		CPVT_WordRange wr;
		if (m_wpCaret.LineCmp(wpOld) !=0)
			wr = CPVT_WordRange(m_pVT->GetLineBeginPlace(wpOld),m_pVT->GetSectionEndPlace(wpNew));	
		else
			wr = CPVT_WordRange(wpOld,m_pVT->GetSectionEndPlace(wpNew));	
		Refresh(RP_ANALYSE, &wr);
		SetCaretOrigin();
		SetCaretInfo();
	}
}

FX_BOOL CFX_Edit::Redo()
{
	if (m_bEnableUndo)
	{
		if (m_Undo.CanRedo())
		{
			m_Undo.Redo();
			return TRUE;
		}
	}

	return FALSE;
}

FX_BOOL CFX_Edit::Undo()
{
	if (m_bEnableUndo)
	{
		if (m_Undo.CanUndo())
		{
			m_Undo.Undo();
			return TRUE;
		}
	}

	return FALSE;
}

void CFX_Edit::SetCaretOrigin()
{
	if (m_pVT->IsValid())
	{
		if (IPDF_VariableText_Iterator * pIterator = m_pVT->GetIterator())
		{
			pIterator->SetAt(m_wpCaret);
			CPVT_Word word;
			CPVT_Line line;
			if (pIterator->GetWord(word))
			{
				m_ptCaret.x = word.ptWord.x + word.fWidth;
				m_ptCaret.y = word.ptWord.y;
			}
			else if (pIterator->GetLine(line))
			{				
				m_ptCaret.x = line.ptLine.x;
				m_ptCaret.y = line.ptLine.y;
			}
		}				
	}	
}

FX_INT32 CFX_Edit::WordPlaceToWordIndex(const CPVT_WordPlace & place) const
{
	if (m_pVT->IsValid())
		return m_pVT->WordPlaceToWordIndex(place);

	return -1;
}

CPVT_WordPlace CFX_Edit::WordIndexToWordPlace(FX_INT32 index) const
{
	if (m_pVT->IsValid())
		return m_pVT->WordIndexToWordPlace(index);

	return CPVT_WordPlace();
}

FX_BOOL	CFX_Edit::IsTextFull() const
{
	FX_INT32 nTotalWords = m_pVT->GetTotalWords();
	FX_INT32 nLimitChar = m_pVT->GetLimitChar();
	FX_INT32 nCharArray = m_pVT->GetCharArray();

	return IsTextOverflow() || (nLimitChar>0 && nTotalWords >= nLimitChar)
		|| (nCharArray>0 && nTotalWords >= nCharArray);
}

FX_BOOL	CFX_Edit::IsTextOverflow() const
{
	if (!m_bEnableScroll && !m_bEnableOverflow)
	{
		CPDF_Rect rcPlate = m_pVT->GetPlateRect();
		CPDF_Rect rcContent = m_pVT->GetContentRect();

		if (m_pVT->IsMultiLine() && GetTotalLines() > 1)
		{
			if (FX_EDIT_IsFloatBigger(rcContent.Height(),rcPlate.Height())) return TRUE;
		}

		if (FX_EDIT_IsFloatBigger(rcContent.Width(),rcPlate.Width())) return TRUE;
	}

	return FALSE;
}

CPVT_WordPlace CFX_Edit::GetLineBeginPlace(const CPVT_WordPlace & place) const
{
	return m_pVT->GetLineBeginPlace(place);
}

CPVT_WordPlace CFX_Edit::GetLineEndPlace(const CPVT_WordPlace & place) const
{
	return m_pVT->GetLineEndPlace(place);
}

CPVT_WordPlace CFX_Edit::GetSectionBeginPlace(const CPVT_WordPlace & place) const
{
	return m_pVT->GetSectionBeginPlace(place);
}

CPVT_WordPlace CFX_Edit::GetSectionEndPlace(const CPVT_WordPlace & place) const
{
	return m_pVT->GetSectionEndPlace(place);
}

FX_BOOL	CFX_Edit::CanUndo() const
{
	if (m_bEnableUndo)
	{
		return m_Undo.CanUndo();
	}

	return FALSE;
}

FX_BOOL	CFX_Edit::CanRedo() const
{
	if (m_bEnableUndo)
	{
		return m_Undo.CanRedo();
	}

	return FALSE;
}

FX_BOOL	CFX_Edit::IsModified() const
{
	if (m_bEnableUndo)
	{
		return m_Undo.IsModified();
	}

	return FALSE;
}

void CFX_Edit::EnableRefresh(FX_BOOL bRefresh)
{
	m_bEnableRefresh = bRefresh;
}

void CFX_Edit::EnableUndo(FX_BOOL bUndo)
{
	this->m_bEnableUndo = bUndo;
}

void CFX_Edit::EnableNotify(FX_BOOL bNotify)
{
	this->m_bNotify = bNotify;
}

void CFX_Edit::EnableOprNotify(FX_BOOL bNotify)
{
	this->m_bOprNotify = bNotify;
}

FX_FLOAT CFX_Edit::GetLineTop(const CPVT_WordPlace& place) const
{
	if (IPDF_VariableText_Iterator* pIterator = m_pVT->GetIterator())
	{
		CPVT_WordPlace wpOld = pIterator->GetAt();

		pIterator->SetAt(place);
		CPVT_Line line;
		pIterator->GetLine(line);

		pIterator->SetAt(wpOld);

		return line.ptLine.y + line.fLineAscent;
	}

	return 0.0f;
}

FX_FLOAT CFX_Edit::GetLineBottom(const CPVT_WordPlace& place) const
{
	if (IPDF_VariableText_Iterator* pIterator = m_pVT->GetIterator())
	{
		CPVT_WordPlace wpOld = pIterator->GetAt();

		pIterator->SetAt(place);
		CPVT_Line line;
		pIterator->GetLine(line);

		pIterator->SetAt(wpOld);

		return line.ptLine.y + line.fLineDescent;
	}

	return 0.0f;
}

CPVT_WordPlace CFX_Edit::DoInsertText(const CPVT_WordPlace& place, FX_LPCWSTR text, FX_INT32 charset, 
									  const CPVT_SecProps * pSecProps, const CPVT_WordProps * pWordProps)
{
	CPVT_WordPlace wp = place;

	if (m_pVT->IsValid())
	{
		CFX_WideString sText = text;

		for (FX_INT32 i = 0, sz = sText.GetLength(); i < sz; i++)
		{
			FX_WORD word = sText[i];
			switch (word)
			{
			case 0x0D:
				wp = m_pVT->InsertSection(wp,pSecProps,pWordProps);
				if (sText[i+1] == 0x0A)
					i++;
				break;
			case 0x0A:	
				wp = m_pVT->InsertSection(wp,pSecProps,pWordProps);
				if (sText[i+1] == 0x0D)
					i++;
				break;
			case 0x09:
				word = 0x20;
			default:
				wp = m_pVT->InsertWord(wp,word,GetCharSetFromUnicode(word, charset),pWordProps);
				break;
			}
		}
	}

	return wp;
}

FX_INT32 CFX_Edit::GetCharSetFromUnicode(FX_WORD word, FX_INT32 nOldCharset)
{
	if (IFX_Edit_FontMap* pFontMap = this->GetFontMap())
		return pFontMap->CharSetFromUnicode(word, nOldCharset);
	else
		return nOldCharset;
}

void CFX_Edit::BeginGroupUndo(const CFX_WideString& sTitle)
{
	ASSERT(m_pGroupUndoItem == NULL);

	m_pGroupUndoItem = new CFX_Edit_GroupUndoItem(sTitle);
}

void CFX_Edit::EndGroupUndo()
{
	ASSERT(m_pGroupUndoItem != NULL);

	m_pGroupUndoItem->UpdateItems();
	m_Undo.AddItem(m_pGroupUndoItem);
	if (m_bOprNotify && m_pOprNotify)
		m_pOprNotify->OnAddUndo(m_pGroupUndoItem);
	m_pGroupUndoItem = NULL;
}

void CFX_Edit::AddEditUndoItem(CFX_Edit_UndoItem* pEditUndoItem)
{
	if (m_pGroupUndoItem)
		m_pGroupUndoItem->AddUndoItem(pEditUndoItem);
	else
	{
		m_Undo.AddItem(pEditUndoItem);
		if (m_bOprNotify && m_pOprNotify)
			m_pOprNotify->OnAddUndo(pEditUndoItem);
	}
}

void CFX_Edit::AddUndoItem(IFX_Edit_UndoItem* pUndoItem)
{
	m_Undo.AddItem(pUndoItem);
	if (m_bOprNotify && m_pOprNotify)
		m_pOprNotify->OnAddUndo(pUndoItem);
}

