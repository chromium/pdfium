// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "../core/include/fwl_threadimp.h"
#include "../core/include/fwl_appimp.h"
#include "../core/include/fwl_targetimp.h"
#include "../core/include/fwl_noteimp.h"
#include "../core/include/fwl_widgetimp.h"
#include "../core/include/fwl_widgetmgrimp.h"
#include "include/fwl_scrollbarimp.h"
#include "include/fwl_editimp.h"
#include "include/fwl_caretimp.h"
IFWL_Edit* IFWL_Edit::Create()
{
    return new IFWL_Edit;
}
IFWL_Edit::IFWL_Edit()
{
    m_pData = NULL;
}
IFWL_Edit::~IFWL_Edit()
{
    if (m_pData) {
        delete (CFWL_EditImp*)m_pData;
        m_pData = NULL;
    }
}
FWL_ERR IFWL_Edit::Initialize(IFWL_Widget *pOuter)
{
    m_pData = FX_NEW CFWL_EditImp(pOuter);
    ((CFWL_EditImp*)m_pData)->SetInterface(this);
    return ((CFWL_EditImp*)m_pData)->Initialize();
}
FWL_ERR	IFWL_Edit::Initialize(const CFWL_WidgetImpProperties &properties, IFWL_Widget *pOuter)
{
    m_pData = FX_NEW CFWL_EditImp(properties, pOuter);
    ((CFWL_EditImp*)m_pData)->SetInterface(this);
    return ((CFWL_EditImp*)m_pData)->Initialize();
}
FWL_ERR	IFWL_Edit::SetText(const CFX_WideString &wsText)
{
    return ((CFWL_EditImp*)m_pData)->SetText(wsText);
}
FX_INT32 IFWL_Edit::GetTextLength() const
{
    return ((CFWL_EditImp*)m_pData)->GetTextLength();
}
FWL_ERR	IFWL_Edit::GetText(CFX_WideString &wsText, FX_INT32 nStart, FX_INT32 nCount) const
{
    return ((CFWL_EditImp*)m_pData)->GetText(wsText, nStart, nCount);
}
FWL_ERR	IFWL_Edit::ClearText()
{
    return ((CFWL_EditImp*)m_pData)->ClearText();
}
FX_INT32 IFWL_Edit::GetCaretPos() const
{
    return ((CFWL_EditImp*)m_pData)->GetCaretPos();
}
FX_INT32 IFWL_Edit::SetCaretPos(FX_INT32 nIndex, FX_BOOL bBefore)
{
    return ((CFWL_EditImp*)m_pData)->SetCaretPos(nIndex, bBefore);
}
FWL_ERR IFWL_Edit::AddSelRange(FX_INT32 nStart, FX_INT32 nCount)
{
    return ((CFWL_EditImp*)m_pData)->AddSelRange(nStart, nCount);
}
FX_INT32 IFWL_Edit::CountSelRanges()
{
    return ((CFWL_EditImp*)m_pData)->CountSelRanges();
}
FX_INT32 IFWL_Edit::GetSelRange(FX_INT32 nIndex, FX_INT32 &nStart)
{
    return ((CFWL_EditImp*)m_pData)->GetSelRange(nIndex, nStart);
}
FWL_ERR	IFWL_Edit::ClearSelections()
{
    return ((CFWL_EditImp*)m_pData)->ClearSelections();
}
FX_INT32 IFWL_Edit::GetLimit()
{
    return ((CFWL_EditImp*)m_pData)->GetLimit();
}
FWL_ERR	IFWL_Edit::SetLimit(FX_INT32 nLimit)
{
    return ((CFWL_EditImp*)m_pData)->SetLimit(nLimit);
}
FWL_ERR	IFWL_Edit::SetAliasChar(FX_WCHAR wAlias)
{
    return ((CFWL_EditImp*)m_pData)->SetAliasChar(wAlias);
}
FWL_ERR	IFWL_Edit::SetFormatString(const CFX_WideString &wsFormat)
{
    return ((CFWL_EditImp*)m_pData)->SetFormatString(wsFormat);
}
FWL_ERR IFWL_Edit::Insert(FX_INT32 nStart, FX_LPCWSTR lpText, FX_INT32 nLen)
{
    return ((CFWL_EditImp*)m_pData)->Insert(nStart, lpText, nLen);
}
FWL_ERR IFWL_Edit::DeleteSelections()
{
    return ((CFWL_EditImp*)m_pData)->DeleteSelections();
}
FWL_ERR IFWL_Edit::DeleteRange(FX_INT32 nStart, FX_INT32 nCount)
{
    return ((CFWL_EditImp*)m_pData)->DeleteRange(nStart, nCount);
}
FWL_ERR IFWL_Edit::ReplaceSelections(const CFX_WideStringC &wsReplace)
{
    return ((CFWL_EditImp*)m_pData)->ReplaceSelections(wsReplace);
}
FWL_ERR IFWL_Edit::Replace(FX_INT32 nStart, FX_INT32 nLen, const CFX_WideStringC &wsReplace)
{
    return ((CFWL_EditImp*)m_pData)->Replace(nStart, nLen, wsReplace);
}
FWL_ERR	IFWL_Edit::DoClipboard(FX_INT32 iCmd)
{
    return ((CFWL_EditImp*)m_pData)->DoClipboard(iCmd);
}
FX_BOOL	IFWL_Edit::Copy(CFX_WideString &wsCopy)
{
    return ((CFWL_EditImp*)m_pData)->Copy(wsCopy);
}
FX_BOOL	IFWL_Edit::Cut(CFX_WideString &wsCut)
{
    return ((CFWL_EditImp*)m_pData)->Cut(wsCut);
}
FX_BOOL	IFWL_Edit::Paste(const CFX_WideString &wsPaste)
{
    return ((CFWL_EditImp*)m_pData)->Paste(wsPaste);
}
FX_BOOL	IFWL_Edit::Delete()
{
    return ((CFWL_EditImp*)m_pData)->Delete();
}
FX_BOOL	IFWL_Edit::Redo(FX_BSTR bsRecord)
{
    return ((CFWL_EditImp*)m_pData)->Redo(bsRecord);
}
FX_BOOL	IFWL_Edit::Undo(FX_BSTR bsRecord)
{
    return ((CFWL_EditImp*)m_pData)->Undo(bsRecord);
}
FX_BOOL	IFWL_Edit::Undo()
{
    return ((CFWL_EditImp*)m_pData)->Undo();
}
FX_BOOL	IFWL_Edit::Redo()
{
    return ((CFWL_EditImp*)m_pData)->Redo();
}
FX_BOOL	IFWL_Edit::CanUndo()
{
    return ((CFWL_EditImp*)m_pData)->CanUndo();
}
FX_BOOL	IFWL_Edit::CanRedo()
{
    return ((CFWL_EditImp*)m_pData)->CanRedo();
}
FWL_ERR IFWL_Edit::SetTabWidth(FX_FLOAT fTabWidth, FX_BOOL bEquidistant)
{
    return ((CFWL_EditImp*)m_pData)->SetTabWidth(fTabWidth, bEquidistant);
}
FWL_ERR	IFWL_Edit::SetOuter(IFWL_Widget *pOuter)
{
    return ((CFWL_EditImp*)m_pData)->SetOuter(pOuter);
}
FWL_ERR	IFWL_Edit::SetNumberRange(FX_INT32 iMin, FX_INT32 iMax)
{
    return ((CFWL_EditImp*)m_pData)->SetNumberRange(iMin, iMax);
}
FWL_ERR	IFWL_Edit::SetBackColor(FX_DWORD dwColor)
{
    return ((CFWL_EditImp*)m_pData)->SetBackgroundColor(dwColor);
}
FWL_ERR IFWL_Edit::SetFont(const CFX_WideString &wsFont, FX_FLOAT fSize)
{
    return ((CFWL_EditImp*)m_pData)->SetFont(wsFont, fSize);
}
void IFWL_Edit::SetScrollOffset(FX_FLOAT fScrollOffset)
{
    return ((CFWL_EditImp*)m_pData)->SetScrollOffset(fScrollOffset);
}
FX_BOOL IFWL_Edit::GetSuggestWords(CFX_PointF pointf, CFX_ByteStringArray &sSuggest)
{
    return ((CFWL_EditImp*)m_pData)->GetSuggestWords(pointf, sSuggest);
}
FX_BOOL IFWL_Edit::ReplaceSpellCheckWord(CFX_PointF pointf, FX_BSTR bsReplace)
{
    return ((CFWL_EditImp*)m_pData)->ReplaceSpellCheckWord(pointf, bsReplace);
}
#define FWL_EDIT_Margin 3
CFWL_EditImp::CFWL_EditImp(IFWL_Widget *pOuter )
    : CFWL_WidgetImp(pOuter)
    , m_fVAlignOffset(0.0f)
    , m_fScrollOffsetX(0.0f)
    , m_fScrollOffsetY(0.0f)
    , m_pEdtEngine(NULL)
    , m_bLButtonDown(FALSE)
    , m_nSelStart(0)
    , m_nLimit(-1)
    , m_fFontSize(0)
    , m_pVertScrollBar(NULL)
    , m_pHorzScrollBar(NULL)
    , m_pCaret(NULL)
    , m_iMin(-1)
    , m_iMax(0xFFFFFFF)
    , m_bSetRange(FALSE)
    , m_pTextField(NULL)
    , m_backColor(0)
    , m_updateBackColor(FALSE)
    , m_iCurRecord(-1)
    , m_iMaxRecord(128)
    , m_fSpaceAbove(0)
    , m_fSpaceBelow(0)
{
    m_rtClient.Reset();
    m_rtEngine.Reset();
    m_rtStatic.Reset();
}
CFWL_EditImp::CFWL_EditImp(const CFWL_WidgetImpProperties &properties, IFWL_Widget *pOuter )
    : CFWL_WidgetImp(properties, pOuter)
    , m_fVAlignOffset(0.0f)
    , m_fScrollOffsetX(0.0f)
    , m_fScrollOffsetY(0.0f)
    , m_pEdtEngine(NULL)
    , m_bLButtonDown(FALSE)
    , m_nSelStart(0)
    , m_nLimit(-1)
    , m_fFontSize(0)
    , m_pVertScrollBar(NULL)
    , m_pHorzScrollBar(NULL)
    , m_pCaret(NULL)
    , m_iMin(-1)
    , m_iMax(0xFFFFFFF)
    , m_bSetRange(FALSE)
    , m_pTextField(NULL)
    , m_backColor(0)
    , m_updateBackColor(FALSE)
    , m_iCurRecord(-1)
    , m_iMaxRecord(128)
    , m_fSpaceAbove(0)
    , m_fSpaceBelow(0)
{
    m_rtClient.Reset();
    m_rtEngine.Reset();
    m_rtStatic.Reset();
}
CFWL_EditImp::~CFWL_EditImp()
{
    if (m_pEdtEngine) {
        m_pEdtEngine->Release();
        m_pEdtEngine = NULL;
    }
    if (m_pHorzScrollBar) {
        m_pHorzScrollBar->Release();
        m_pHorzScrollBar = NULL;
    }
    if (m_pVertScrollBar) {
        m_pVertScrollBar->Release();
        m_pVertScrollBar = NULL;
    }
    if (m_pCaret) {
        m_pCaret->Release();
        m_pCaret = NULL;
    }
    ClearRecord();
}
FWL_ERR CFWL_EditImp::GetClassName(CFX_WideString &wsClass) const
{
    wsClass = FWL_CLASS_Edit;
    return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_EditImp::GetClassID() const
{
    return FWL_CLASSHASH_Edit;
}
FWL_ERR	CFWL_EditImp::Initialize()
{
    _FWL_ERR_CHECK_RETURN_VALUE_IF_FAIL(CFWL_WidgetImp::Initialize(), FWL_ERR_Indefinite);
    if (!m_pDelegate) {
        m_pDelegate = (IFWL_WidgetDelegate*)FX_NEW CFWL_EditImpDelegate(this);
    }
    InitCaret();
    if (!m_pEdtEngine) {
        InitEngine();
    }
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_EditImp::Finalize()
{
    if (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) {
        ShowCaret(FALSE);
    }
    if (m_pHorzScrollBar) {
        m_pHorzScrollBar->Finalize();
    }
    if (m_pVertScrollBar) {
        m_pVertScrollBar->Finalize();
    }
    if ( m_pDelegate) {
        delete (CFWL_EditImpDelegate*)m_pDelegate;
        m_pDelegate = NULL;
    }
    return CFWL_WidgetImp::Finalize();
}
FWL_ERR CFWL_EditImp::GetWidgetRect(CFX_RectF &rect, FX_BOOL bAutoSize )
{
    if (bAutoSize) {
        rect.Set(0, 0, 0, 0);
        if (m_pEdtEngine) {
            FX_INT32 iTextLen = m_pEdtEngine->GetTextLength();
            if (iTextLen > 0) {
                CFX_WideString wsText;
                m_pEdtEngine->GetText(wsText, 0);
                CFX_SizeF sz = CalcTextSize(wsText,
                                            m_pProperties->m_pThemeProvider,
                                            m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_MultiLine);
                rect.Set(0, 0, sz.x, sz.y);
            }
        }
        CFWL_WidgetImp::GetWidgetRect(rect, TRUE);
    } else {
        rect = m_pProperties->m_rtWidget;
        if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_OuterScrollbar) {
            if (IsShowScrollBar(TRUE)) {
                FX_FLOAT *pfWidth = (FX_FLOAT*)GetThemeCapacity(FWL_WGTCAPACITY_ScrollBarWidth);
                rect.width += *pfWidth;
                rect.width += FWL_EDIT_Margin;
            }
            if (IsShowScrollBar(FALSE)) {
                FX_FLOAT *pfWidth = (FX_FLOAT*)GetThemeCapacity(FWL_WGTCAPACITY_ScrollBarWidth);
                rect.height += *pfWidth;
                rect.height += FWL_EDIT_Margin;
            }
        }
    }
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_EditImp::SetStates(FX_DWORD dwStates, FX_BOOL bSet )
{
    if ((m_pProperties->m_dwStates & FWL_WGTSTATE_Invisible) || (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled)) {
        ShowCaret(FALSE);
    }
    return CFWL_WidgetImp::SetStates(dwStates, bSet);
}
FWL_ERR	CFWL_EditImp::SetWidgetRect(const CFX_RectF &rect)
{
    return	CFWL_WidgetImp::SetWidgetRect(rect);
}
FWL_ERR CFWL_EditImp::Update()
{
    if (IsLocked()) {
        return FWL_ERR_Indefinite;
    }
    if (!m_pProperties->m_pThemeProvider) {
        m_pProperties->m_pThemeProvider = GetAvailableTheme();
    }
    Layout();
    if (m_rtClient.IsEmpty()) {
        return FWL_ERR_Indefinite;
    }
    UpdateEditEngine();
    UpdateVAlignment();
    UpdateScroll();
    InitCaret();
    return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_EditImp::HitTest(FX_FLOAT fx, FX_FLOAT fy)
{
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_OuterScrollbar) {
        if (IsShowScrollBar(TRUE)) {
            CFX_RectF rect;
            m_pVertScrollBar->GetWidgetRect(rect);
            if (rect.Contains(fx, fy)) {
                return FWL_WGTHITTEST_VScrollBar;
            }
        }
        if (IsShowScrollBar(FALSE)) {
            CFX_RectF rect;
            m_pHorzScrollBar->GetWidgetRect(rect);
            if (rect.Contains(fx, fy)) {
                return FWL_WGTHITTEST_HScrollBar;
            }
        }
    }
    if (m_rtClient.Contains(fx, fy)) {
        return FWL_WGTHITTEST_Edit;
    }
    return FWL_WGTHITTEST_Unknown;
}
#define FX_EDIT_ISLATINWORD(u)	(u == 0x2D || (u <= 0x005A && u >= 0x0041) || (u <= 0x007A && u >= 0x0061) || (u <= 0x02AF && u >= 0x00C0) || u == 0x0027)
static void AddSquigglyPath(CFX_Path& PathData, FX_FLOAT fStartX, FX_FLOAT fEndX, FX_FLOAT fY, FX_FLOAT fStep)
{
    PathData.MoveTo(fStartX, fY);
    FX_FLOAT fx;
    FX_INT32 i;
    for (i = 1, fx = fStartX + fStep; fx < fEndX; fx += fStep, i++) {
        PathData.LineTo(fx, fY + (i & 1)*fStep);
    }
}
void CFWL_EditImp::AddSpellCheckObj(CFX_Path& PathData, FX_INT32 nStart, FX_INT32 nCount, FX_FLOAT fOffSetX, FX_FLOAT fOffSetY)
{
    FX_FLOAT fStartX = 0.0f;
    FX_FLOAT fEndX = 0.0f;
    FX_FLOAT fY = 0.0f;
    FX_FLOAT fStep = 0.0f;
    IFDE_TxtEdtPage *pPage = m_pEdtEngine->GetPage(0);
    CFX_RectFArray rectArray;
    CFX_RectF rectText;
    const FDE_TXTEDTPARAMS* txtEdtParams = m_pEdtEngine->GetEditParams();
    FX_FLOAT fAsent = (FX_FLOAT)txtEdtParams->pFont->GetAscent() * txtEdtParams->fFontSize / 1000;
    pPage->CalcRangeRectArray(nStart, nCount, rectArray);
    for (int i = 0; i < rectArray.GetSize(); i++) {
        rectText = rectArray.GetAt(i);
        fY = rectText.top + fAsent + fOffSetY;
        fStep = txtEdtParams->fFontSize / 16.0f;
        fStartX = rectText.left + fOffSetX;
        fEndX = fStartX + rectText.Width();
        AddSquigglyPath(PathData, fStartX, fEndX, fY, fStep);
    }
}
FX_INT32 CFWL_EditImp::GetWordAtPoint(CFX_PointF pointf, FX_INT32& nCount)
{
    return 0;
}
FX_BOOL CFWL_EditImp::GetSuggestWords(CFX_PointF pointf, CFX_ByteStringArray &sSuggest)
{
    FX_INT32 nWordCount = 0;
    FX_INT32 nWordStart = GetWordAtPoint(pointf, nWordCount);
    if (nWordCount < 1) {
        return FALSE;
    }
    CFX_WideString wsSpell;
    GetText(wsSpell, nWordStart, nWordCount);
    CFX_ByteString sLatinWord;
    for (int i = 0; i < nWordCount; i++) {
        if (!FX_EDIT_ISLATINWORD(wsSpell[i])) {
            break;
        }
        sLatinWord += (FX_CHAR)wsSpell[i];
    }
    if (sLatinWord.IsEmpty()) {
        return FALSE;
    }
    CFWL_EvtEdtCheckWord checkWordEvent;
    checkWordEvent.m_pSrcTarget = m_pInterface;
    checkWordEvent.bsWord = sLatinWord;
    checkWordEvent.bCheckWord = TRUE;
    DispatchEvent(&checkWordEvent);
    if (checkWordEvent.bCheckWord) {
        return FALSE;
    }
    CFWL_EvtEdtGetSuggestWords suggestWordsEvent;
    suggestWordsEvent.m_pSrcTarget = m_pInterface;
    suggestWordsEvent.bsWord = sLatinWord;
    suggestWordsEvent.bsArraySuggestWords = sSuggest;
    suggestWordsEvent.bSuggestWords = FALSE;
    DispatchEvent(&checkWordEvent);
    return suggestWordsEvent.bSuggestWords;
}
FX_BOOL CFWL_EditImp::ReplaceSpellCheckWord(CFX_PointF pointf, FX_BSTR bsReplace)
{
    FX_INT32 nWordCount = 0;
    FX_INT32 nWordStart = GetWordAtPoint(pointf, nWordCount);
    if (nWordCount < 1) {
        return FALSE;
    }
    CFX_WideString wsSpell;
    GetText(wsSpell, nWordStart, nWordCount);
    for (int i = 0; i < nWordCount; i++) {
        if (!FX_EDIT_ISLATINWORD(wsSpell[i])) {
            nWordCount = i;
            break;
        }
    }
    FX_INT32 nDestLen = bsReplace.GetLength();
    CFX_WideString wsDest;
    FX_LPWSTR pBuffer = wsDest.GetBuffer(nDestLen);
    for (FX_INT32 i = 0; i < nDestLen; i++) {
        pBuffer[i] = bsReplace[i];
    }
    wsDest.ReleaseBuffer(nDestLen);
    Replace(nWordStart, nWordCount, wsDest);
    return TRUE;
}
void CFWL_EditImp::DrawSpellCheck(CFX_Graphics *pGraphics, const CFX_Matrix *pMatrix )
{
    pGraphics->SaveGraphState();
    if (pMatrix) {
        pGraphics->ConcatMatrix((CFX_Matrix*)pMatrix);
    }
    FX_ARGB cr = 0xFFFF0000;
    CFX_Color crLine(cr);
    FX_FLOAT fWidth = 1.0f;
    CFWL_EvtEdtCheckWord checkWordEvent;
    checkWordEvent.m_pSrcTarget = m_pInterface;
    CFX_ByteString sLatinWord;
    CFX_Path pathSpell;
    pathSpell.Create();
    FX_INT32 nStart = 0;
    FX_FLOAT fOffSetX = m_rtEngine.left - m_fScrollOffsetX;
    FX_FLOAT fOffSetY = m_rtEngine.top - m_fScrollOffsetY + m_fVAlignOffset;
    CFX_WideString wsSpell;
    this->GetText(wsSpell);
    FX_INT32 nContentLen = wsSpell.GetLength();
    for (int i = 0; i < nContentLen; i++) {
        if (FX_EDIT_ISLATINWORD(wsSpell[i])) {
            if (sLatinWord.IsEmpty()) {
                nStart = i;
            }
            sLatinWord += (FX_CHAR)wsSpell[i];
        } else {
            checkWordEvent.bsWord = sLatinWord;
            checkWordEvent.bCheckWord = TRUE;
            DispatchEvent(&checkWordEvent);
            if (!sLatinWord.IsEmpty() && !checkWordEvent.bCheckWord) {
                AddSpellCheckObj(pathSpell, nStart, sLatinWord.GetLength(), fOffSetX, fOffSetY);
            }
            sLatinWord.Empty();
        }
    }
    checkWordEvent.bsWord = sLatinWord;
    checkWordEvent.bCheckWord = TRUE;
    DispatchEvent(&checkWordEvent);
    if (!sLatinWord.IsEmpty() && !checkWordEvent.bCheckWord) {
        AddSpellCheckObj(pathSpell, nStart, sLatinWord.GetLength(), fOffSetX, fOffSetY);
    }
    if (!pathSpell.IsEmpty()) {
        CFX_RectF rtClip = m_rtEngine;
        CFX_Matrix mt;
        mt.Set(1, 0, 0, 1, fOffSetX, fOffSetY);
        if (pMatrix) {
            pMatrix->TransformRect(rtClip);
            mt.Concat(*pMatrix);
        }
        pGraphics->SetClipRect(rtClip);
        pGraphics->SetStrokeColor(&crLine);
        pGraphics->SetLineWidth(0);
        pGraphics->StrokePath(&pathSpell, NULL);
    }
    pGraphics->RestoreGraphState();
}
FWL_ERR CFWL_EditImp::DrawWidget(CFX_Graphics *pGraphics, const CFX_Matrix *pMatrix )
{
    _FWL_RETURN_VALUE_IF_FAIL(pGraphics, FWL_ERR_Indefinite);
    _FWL_RETURN_VALUE_IF_FAIL(m_pProperties->m_pThemeProvider, FWL_ERR_Indefinite);
    if (m_rtClient.IsEmpty()) {
        return FWL_ERR_Indefinite;
    }
    IFWL_ThemeProvider *pTheme = m_pProperties->m_pThemeProvider;
    if (!m_pWidgetMgr->IsFormDisabled()) {
        DrawTextBk(pGraphics, pTheme, pMatrix);
    }
    if (m_pEdtEngine) {
        DrawContent(pGraphics, pTheme, pMatrix);
    }
    if ((m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) && !(m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_ReadOnly)) {
        DrawSpellCheck(pGraphics, pMatrix);
    }
    if (HasBorder()) {
        DrawBorder(pGraphics, FWL_PART_EDT_Border, pTheme, pMatrix);
    }
    if (HasEdge()) {
        DrawEdge(pGraphics, FWL_PART_EDT_Edge, pTheme, pMatrix);
    }
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_EditImp::SetThemeProvider(IFWL_ThemeProvider *pThemeProvider)
{
    _FWL_RETURN_VALUE_IF_FAIL(pThemeProvider, FWL_ERR_Indefinite);
    if (m_pHorzScrollBar) {
        m_pHorzScrollBar->SetThemeProvider(pThemeProvider);
    }
    if (m_pVertScrollBar) {
        m_pVertScrollBar->SetThemeProvider(pThemeProvider);
    }
    if (m_pCaret) {
        m_pCaret->SetThemeProvider(pThemeProvider);
    }
    m_pProperties->m_pThemeProvider = pThemeProvider;
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_EditImp::SetText(const CFX_WideString &wsText)
{
    m_pEdtEngine->SetText(wsText);
    return FWL_ERR_Succeeded;
}
FX_INT32 CFWL_EditImp::GetTextLength() const
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, -1);
    return m_pEdtEngine->GetTextLength();
}
FWL_ERR CFWL_EditImp::GetText(CFX_WideString &wsText, FX_INT32 nStart , FX_INT32 nCount ) const
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, FWL_ERR_Succeeded);
    m_pEdtEngine->GetText(wsText, nStart, nCount);
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_EditImp::ClearText()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, FWL_ERR_Succeeded);
    m_pEdtEngine->ClearText();
    return FWL_ERR_Succeeded;
}
FX_INT32 CFWL_EditImp::GetCaretPos() const
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, -1);
    return m_pEdtEngine->GetCaretPos();
}
FX_INT32 CFWL_EditImp::SetCaretPos(FX_INT32 nIndex, FX_BOOL bBefore )
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, -1);
    return m_pEdtEngine->SetCaretPos(nIndex, bBefore);
}
FWL_ERR CFWL_EditImp::AddSelRange(FX_INT32 nStart, FX_INT32 nCount )
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, FWL_ERR_Succeeded);
    m_pEdtEngine->AddSelRange(nStart, nCount);
    return FWL_ERR_Succeeded;
}
FX_INT32 CFWL_EditImp::CountSelRanges()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, 0);
    return m_pEdtEngine->CountSelRanges();
    return FWL_ERR_Succeeded;
}
FX_INT32 CFWL_EditImp::GetSelRange(FX_INT32 nIndex, FX_INT32 &nStart)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, -1);
    return m_pEdtEngine->GetSelRange(nIndex, nStart);
}
FWL_ERR CFWL_EditImp::ClearSelections()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, FWL_ERR_Succeeded);
    m_pEdtEngine->ClearSelection();
    return FWL_ERR_Succeeded;
}
FX_INT32 CFWL_EditImp::GetLimit()
{
    return m_nLimit;
}
FWL_ERR CFWL_EditImp::SetLimit(FX_INT32 nLimit)
{
    m_nLimit = nLimit;
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, FWL_ERR_Succeeded);
    m_pEdtEngine->SetLimit(nLimit);
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_EditImp::SetAliasChar(FX_WCHAR wAlias)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, FWL_ERR_Indefinite);
    m_pEdtEngine->SetAliasChar(wAlias);
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_EditImp::SetFormatString(const CFX_WideString &wsFormat)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, FWL_ERR_Succeeded);
    m_pEdtEngine->SetFormatBlock(0, wsFormat);
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_EditImp::Insert(FX_INT32 nStart, FX_LPCWSTR lpText, FX_INT32 nLen)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, FWL_ERR_Succeeded);
    if (	(m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_ReadOnly)
            || (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled)) {
        return FWL_ERR_Indefinite;
    }
    m_pEdtEngine->Insert(nStart, lpText, nLen);
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_EditImp::DeleteSelections()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, FWL_ERR_Succeeded);
    FX_INT32 iCount = m_pEdtEngine->CountSelRanges();
    if (iCount > 0) {
        m_pEdtEngine->Delete(-1);
    }
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_EditImp::DeleteRange(FX_INT32 nStart, FX_INT32 nCount )
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, FWL_ERR_Succeeded);
    m_pEdtEngine->DeleteRange(nStart, nCount);
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_EditImp::ReplaceSelections(const CFX_WideStringC &wsReplace)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, FWL_ERR_Succeeded);
    FX_INT32 iCount = m_pEdtEngine->CountSelRanges();
    for (int i = 0; i < iCount; i ++) {
        FX_INT32 nStart;
        FX_INT32 nCount = m_pEdtEngine->GetSelRange(i, nStart);
        m_pEdtEngine->Replace(nStart, nCount, wsReplace);
    }
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_EditImp::Replace(FX_INT32 nStart, FX_INT32 nLen, const CFX_WideStringC &wsReplace)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, FWL_ERR_Succeeded);
    m_pEdtEngine->Replace(nStart, nLen, wsReplace);
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_EditImp::DoClipboard(FX_INT32 iCmd)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, FWL_ERR_Succeeded);
    if (	(m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_ReadOnly)
            ||	(m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled)) {
        return FWL_ERR_Succeeded;
    }
    IFWL_AdapterNative *pNative = FWL_GetAdapterNative();
    _FWL_RETURN_VALUE_IF_FAIL(pNative, FWL_ERR_Indefinite);
    IFWL_AdapterClipboardMgr *pClipBorder = pNative->GetClipboardMgr();
    _FWL_RETURN_VALUE_IF_FAIL(pClipBorder, FWL_ERR_Indefinite);
    CFX_WideString wsText;
    switch(iCmd) {
        case 1: {
                FX_INT32 nStart;
                FX_INT32 nCount = m_pEdtEngine->GetSelRange(0, nStart);
                if (nCount < 1) {
                    break;
                }
                m_pEdtEngine->GetText(wsText, nStart, nCount);
                pClipBorder->SetStringData(wsText);
                break;
            }
        case 2: {
                FX_INT32 nStart;
                FX_INT32 nCount = m_pEdtEngine->GetSelRange(0, nStart);
                if (nCount < 1) {
                    break;
                }
                m_pEdtEngine->GetText(wsText, nStart, nCount);
                m_pEdtEngine->DeleteRange(nStart, nCount);
                m_pEdtEngine->ClearSelection();
                pClipBorder->SetStringData(wsText);
                break;
            }
        case 3: {
                pClipBorder->GetStringData(wsText);
                FX_INT32 iLen = wsText.GetLength();
                if (iLen < 0) {
                    break;
                }
                if (wsText[iLen] == L'\0') {
                    if (iLen == 1) {
                        break;
                    }
                    iLen --;
                    wsText = wsText.Left(iLen);
                }
                FX_INT32 nPos = m_pEdtEngine->GetCaretPos();
                m_pEdtEngine->Insert(nPos, wsText, iLen);
                break;
            }
        default: {
            }
    }
    return FWL_ERR_Succeeded;
}
FX_BOOL	CFWL_EditImp::Copy(CFX_WideString &wsCopy)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, FALSE);
    FX_INT32 nCount = m_pEdtEngine->CountSelRanges();
    if (nCount == 0) {
        return FALSE;
    }
    wsCopy.Empty();
    CFX_WideString wsTemp;
    FX_INT32 nStart, nLength;
    for (FX_INT32 i = 0; i < nCount; i ++) {
        nLength = m_pEdtEngine->GetSelRange(i, nStart);
        m_pEdtEngine->GetText(wsTemp, nStart, nLength);
        wsCopy += wsTemp;
        wsTemp.Empty();
    }
    return TRUE;
}
FX_BOOL	CFWL_EditImp::Cut(CFX_WideString &wsCut)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, FALSE);
    FX_INT32 nCount = m_pEdtEngine->CountSelRanges();
    if (nCount == 0) {
        return FALSE;
    }
    wsCut.Empty();
    CFX_WideString wsTemp;
    FX_INT32 nStart, nLength;
    for (FX_INT32 i = 0; i < nCount; i ++) {
        nLength = m_pEdtEngine->GetSelRange(i, nStart);
        m_pEdtEngine->GetText(wsTemp, nStart, nLength);
        wsCut += wsTemp;
        wsTemp.Empty();
    }
    m_pEdtEngine->Delete(0);
    return TRUE;
}
FX_BOOL	CFWL_EditImp::Paste(const CFX_WideString &wsPaste)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, FALSE);
    FX_INT32 nCaret = m_pEdtEngine->GetCaretPos();
    FX_INT32 iError = m_pEdtEngine->Insert(nCaret, FX_LPCWSTR(wsPaste), wsPaste.GetLength());
    if (iError < 0) {
        ProcessInsertError(iError);
        return FALSE;
    }
    return TRUE;
}
FX_BOOL CFWL_EditImp::Delete()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, FALSE);
    FX_INT32 nCount = m_pEdtEngine->CountSelRanges();
    if (nCount < 1) {
        return FALSE;
    }
    m_pEdtEngine->Delete(0);
    return TRUE;
}
FX_BOOL CFWL_EditImp::Redo(FX_BSTR bsRecord)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, FALSE);
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_NoRedoUndo) {
        return TRUE;
    }
    return m_pEdtEngine->Redo(bsRecord);
}
FX_BOOL CFWL_EditImp::Undo(FX_BSTR bsRecord)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, FALSE);
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_NoRedoUndo) {
        return TRUE;
    }
    return m_pEdtEngine->Undo(bsRecord);
}
FX_BOOL	CFWL_EditImp::Undo()
{
    if (!CanUndo()) {
        return FALSE;
    }
    CFX_ByteString bsRecord = m_RecordArr[m_iCurRecord--];
    return Undo(bsRecord);
}
FX_BOOL	CFWL_EditImp::Redo()
{
    if (!CanRedo()) {
        return FALSE;
    }
    CFX_ByteString bsRecord = m_RecordArr[++m_iCurRecord];
    return Redo(bsRecord);
}
FX_BOOL	CFWL_EditImp::CanUndo()
{
    return m_iCurRecord >= 0;
}
FX_BOOL	CFWL_EditImp::CanRedo()
{
    return m_iCurRecord < m_RecordArr.GetSize() - 1;
}
FWL_ERR CFWL_EditImp::SetTabWidth(FX_FLOAT fTabWidth, FX_BOOL bEquidistant)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, FWL_ERR_Succeeded);
    FDE_LPTXTEDTPARAMS pParams = (FDE_LPTXTEDTPARAMS)m_pEdtEngine->GetEditParams();
    pParams->fTabWidth = fTabWidth;
    pParams->bTabEquidistant = bEquidistant;
    return FWL_ERR_Succeeded;
}
FWL_ERR	CFWL_EditImp::SetOuter(IFWL_Widget *pOuter)
{
    m_pOuter = pOuter;
    return FWL_ERR_Succeeded;
}
FWL_ERR	CFWL_EditImp::SetNumberRange(FX_INT32 iMin, FX_INT32 iMax)
{
    m_iMin = iMin;
    m_iMax = iMax;
    m_bSetRange = TRUE;
    return FWL_ERR_Succeeded;
}
void CFWL_EditImp::On_CaretChanged(IFDE_TxtEdtEngine *pEdit, FX_INT32 nPage, FX_BOOL bVisible )
{
    if (m_rtEngine.IsEmpty()) {
        return;
    }
    if ((m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) == 0) {
        return;
    }
    FX_BOOL bRepaintContent = UpdateOffset();
    UpdateCaret();
    CFX_RectF rtInvalid;
    rtInvalid.Set(0, 0, 0, 0);
    FX_BOOL bRepaintScroll = FALSE;
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_MultiLine) {
        IFWL_ScrollBar *pScroll = UpdateScroll();
        if (pScroll) {
            pScroll->GetWidgetRect(rtInvalid);
            bRepaintScroll = TRUE;
        }
    }
    if (bRepaintContent || bRepaintScroll) {
        if (bRepaintContent) {
            rtInvalid.Union(m_rtEngine);
        }
        Repaint(&rtInvalid);
    }
}
void CFWL_EditImp::On_TextChanged(IFDE_TxtEdtEngine *pEdit, FDE_TXTEDT_TEXTCHANGE_INFO &ChangeInfo)
{
    FX_DWORD dwStyleEx = m_pProperties->m_dwStyleExes;
    if (dwStyleEx & FWL_STYLEEXT_EDT_VAlignMask) {
        UpdateVAlignment();
    }
    IFDE_TxtEdtPage *page = m_pEdtEngine->GetPage(0);
    FX_FLOAT fContentWidth = page->GetContentsBox().width;
    FX_FLOAT fContentHeight = page->GetContentsBox().height;
    CFX_RectF rtTemp;
    GetClientRect(rtTemp);
    FX_BOOL bHSelfAdaption = m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_HSelfAdaption;
    FX_BOOL bVSelfAdaption = m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_VSelfAdaption;
    FX_BOOL bNeedUpdate = FALSE;
    if (bHSelfAdaption || bVSelfAdaption) {
        CFWL_EvtEdtPreSelfAdaption evt;
        evt.m_pSrcTarget = m_pInterface;
        evt.bHSelfAdaption = TRUE;
        evt.bVSelfAdaption = TRUE;
        FX_FLOAT fWidth;
        FX_FLOAT fHight;
        fWidth = bHSelfAdaption ? fContentWidth : m_pProperties->m_rtWidget.width;
        fHight = bVSelfAdaption ? fContentHeight : m_pProperties->m_rtWidget.height;
        evt.rtAfterChange.Set(0, 0, fWidth, fHight);
        DispatchEvent(&evt);
        if (!evt.bHSelfAdaption) {
            ModifyStylesEx(0, FWL_STYLEEXT_EDT_HSelfAdaption | FWL_STYLEEXT_EDT_AutoHScroll);
        }
        if (!evt.bVSelfAdaption) {
            ModifyStylesEx(0, FWL_STYLEEXT_EDT_VSelfAdaption | FWL_STYLEEXT_EDT_AutoVScroll);
        }
        bNeedUpdate = (bHSelfAdaption && !evt.bHSelfAdaption) || (bVSelfAdaption && !evt.bVSelfAdaption);
    }
    FX_FLOAT fContentWidth1 = fContentWidth;
    FX_FLOAT fContentHeight1 = fContentHeight;
    if (bNeedUpdate) {
        UpdateEditParams();
        UpdateEditLayout();
        IFDE_TxtEdtPage *page1 = m_pEdtEngine->GetPage(0);
        fContentWidth1 = page1->GetContentsBox().width;
        fContentHeight1 = page1->GetContentsBox().height;
    }
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_HSelfAdaption) {
        rtTemp.width = FX_MAX(m_pProperties->m_rtWidget.width, fContentWidth1);
        m_pProperties->m_rtWidget.width = fContentWidth1;
    }
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_VSelfAdaption) {
        rtTemp.height = FX_MAX(m_pProperties->m_rtWidget.height, fContentHeight1);
        m_pProperties->m_rtWidget.height = fContentHeight1;
    }
    CFWL_EvtEdtTextChanged event;
    event.m_pSrcTarget = m_pInterface;
    event.nChangeType = ChangeInfo.nChangeType;
    event.wsInsert = ChangeInfo.wsInsert;
    event.wsDelete = ChangeInfo.wsDelete;
    event.wsPrevText = ChangeInfo.wsPrevText;
    DispatchEvent(&event);
    LayoutScrollBar();
    Repaint(&rtTemp);
}
void CFWL_EditImp::On_SelChanged(IFDE_TxtEdtEngine *pEdit)
{
    CFX_RectF rtTemp;
    GetClientRect(rtTemp);
    Repaint(&rtTemp);
}
FX_BOOL	CFWL_EditImp::On_PageLoad(IFDE_TxtEdtEngine *pEdit, FX_INT32 nPageIndex, FX_INT32 nPurpose)
{
    IFDE_TxtEdtEngine *pEdtEngine = m_pEdtEngine;
    IFDE_TxtEdtPage *pPage = pEdtEngine->GetPage(nPageIndex);
    _FWL_RETURN_VALUE_IF_FAIL(pPage, FALSE);
    pPage->LoadPage();
    return TRUE;
}
FX_BOOL CFWL_EditImp::On_PageUnload(IFDE_TxtEdtEngine *pEdit, FX_INT32 nPageIndex, FX_INT32 nPurpose)
{
    IFDE_TxtEdtEngine *pEdtEngine = m_pEdtEngine;
    IFDE_TxtEdtPage *pPage = pEdtEngine->GetPage(nPageIndex);
    _FWL_RETURN_VALUE_IF_FAIL(pPage, FALSE);
    pPage->UnloadPage();
    return TRUE;
}
void CFWL_EditImp::On_AddDoRecord(IFDE_TxtEdtEngine *pEdit, FX_BSTR bsDoRecord)
{
    AddDoRecord(bsDoRecord);
    CFWL_WidgetImp *pSrcTarget = GetRootOuter();
    if (!pSrcTarget) {
        pSrcTarget = this;
    }
    CFWL_EvtEdtAddDoRecord evt;
    evt.m_pSrcTarget = m_pInterface;
    evt.m_wsDoRecord = bsDoRecord;
    m_pDelegate->OnProcessEvent(&evt);
}
FX_BOOL CFWL_EditImp::On_ValidateField(IFDE_TxtEdtEngine *pEdit, FX_INT32 nBlockIndex, FX_INT32 nFieldIndex, \
                                       const CFX_WideString &wsFieldText, FX_INT32 nCharIndex)
{
    return TRUE;
}
FX_BOOL CFWL_EditImp::On_ValidateBlock(IFDE_TxtEdtEngine *pEdit, FX_INT32 nBlockIndex)
{
    return TRUE;
}
FX_BOOL CFWL_EditImp::On_GetBlockFormatText(IFDE_TxtEdtEngine *pEdit, FX_INT32 nBlockIndex, CFX_WideString &wsBlockText)
{
    return FALSE;
}
FX_BOOL	CFWL_EditImp::On_Validate(IFDE_TxtEdtEngine * pEdit, CFX_WideString &wsText)
{
    IFWL_Widget *pDst = GetOuter();
    if (!pDst) {
        pDst = (IFWL_Widget*)this;
    }
    CFWL_EvtEdtValidate event;
    event.pDstWidget = pDst;
    event.m_pSrcTarget = m_pInterface;
    event.wsInsert = wsText;
    event.bValidate = TRUE;
    DispatchEvent(&event);
    return event.bValidate;
}
FWL_ERR CFWL_EditImp::SetBackgroundColor(FX_DWORD color)
{
    m_backColor = color;
    m_updateBackColor = TRUE;
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_EditImp::SetFont(const CFX_WideString &wsFont, FX_FLOAT fSize)
{
    m_wsFont = wsFont;
    m_fFontSize = fSize;
    return FWL_ERR_Succeeded;
}
void CFWL_EditImp::SetScrollOffset(FX_FLOAT fScrollOffset)
{
    m_fScrollOffsetY = fScrollOffset;
}
void CFWL_EditImp::DrawTextBk(CFX_Graphics *pGraphics, IFWL_ThemeProvider *pTheme, const CFX_Matrix *pMatrix )
{
    CFWL_ThemeBackground param;
    param.m_pWidget = m_pInterface;
    param.m_iPart = FWL_PART_EDT_Background;
    param.m_dwData = FWL_PARTDATA_EDT_Background;
    param.m_dwStates = m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_ReadOnly ?
                       FWL_PARTSTATE_EDT_ReadOnly : FWL_PARTSTATE_EDT_Normal;
    FX_DWORD dwStates = (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled);
    if ( dwStates) {
        param.m_dwStates = FWL_PARTSTATE_EDT_Disable;
    }
    param.m_pGraphics = pGraphics;
    param.m_matrix = *pMatrix;
    param.m_rtPart = m_rtClient;
    pTheme->DrawBackground(&param);
    if (!IsShowScrollBar(TRUE) || !IsShowScrollBar(FALSE)) {
        return;
    }
    CFX_RectF rtScorll;
    m_pHorzScrollBar->GetWidgetRect(rtScorll);
    FX_FLOAT fStaticWidth = rtScorll.height;
    CFX_RectF rtStatic;
    rtStatic.Set(m_rtClient.right() - rtScorll.height,
                 m_rtClient.bottom() - rtScorll.height,
                 rtScorll.height,
                 rtScorll.height);
    param.m_dwData = FWL_PARTDATA_EDT_StaticBackground;
    param.m_rtPart = rtStatic;
    pTheme->DrawBackground(&param);
}
void CFWL_EditImp::DrawContent(CFX_Graphics *pGraphics, IFWL_ThemeProvider *pTheme, const CFX_Matrix *pMatrix )
{
    _FWL_RETURN_IF_FAIL(m_pEdtEngine);
    IFDE_TxtEdtPage *pPage = m_pEdtEngine->GetPage(0);
    _FWL_RETURN_IF_FAIL(pPage);
    pGraphics->SaveGraphState();
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_CombText) {
        pGraphics->SaveGraphState();
    }
    CFX_RectF rtClip = m_rtEngine;
    FX_FLOAT fOffSetX = m_rtEngine.left - m_fScrollOffsetX;
    FX_FLOAT fOffSetY = m_rtEngine.top - m_fScrollOffsetY + m_fVAlignOffset;
    CFX_Matrix mt;
    mt.Set(1, 0, 0, 1, fOffSetX, fOffSetY);
    if (pMatrix) {
        pMatrix->TransformRect(rtClip);
        mt.Concat(*pMatrix);
    }
    FX_BOOL bShowSel = (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_NoHideSel) || (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused);
    if (bShowSel) {
        IFWL_Widget *pForm = m_pWidgetMgr->GetWidget(m_pInterface, FWL_WGTRELATION_SystemForm);
        if (pForm) {
            bShowSel = (pForm->GetStates() & FWL_WGTSTATE_Deactivated) != FWL_WGTSTATE_Deactivated;
        }
    }
    FX_INT32 nSelCount = m_pEdtEngine->CountSelRanges();
    if (bShowSel && nSelCount > 0) {
        FX_INT32 nPageCharStart = pPage->GetCharStart();
        FX_INT32 nPageCharCount = pPage->GetCharCount();
        FX_INT32 nPageCharEnd	= nPageCharStart + nPageCharCount - 1;
        FX_INT32 nCharCount;
        FX_INT32 nCharStart;
        CFX_RectFArray rectArr;
        FX_INT32 i = 0;
        for (i = 0; i < nSelCount; i ++) {
            nCharCount = m_pEdtEngine->GetSelRange(i, nCharStart);
            FX_INT32 nCharEnd = nCharStart + nCharCount - 1;
            if (nCharEnd < nPageCharStart || nCharStart > nPageCharEnd) {
                continue;
            }
            FX_INT32 nBgn = FX_MAX(nCharStart, nPageCharStart);
            FX_INT32 nEnd = FX_MIN(nCharEnd, nPageCharEnd);
            pPage->CalcRangeRectArray(nBgn - nPageCharStart, nEnd - nBgn + 1, rectArr);
        }
        FX_INT32 nCount = rectArr.GetSize();
        CFX_Path path;
        path.Create();
        for (i = 0; i < nCount; i ++) {
            rectArr[i].left += fOffSetX;
            rectArr[i].top += fOffSetY;
            path.AddRectangle(rectArr[i].left,
                              rectArr[i].top,
                              rectArr[i].width,
                              rectArr[i].height);
        }
        pGraphics->SetClipRect(rtClip);
        CFWL_ThemeBackground param;
        param.m_pGraphics = pGraphics;
        param.m_matrix = *pMatrix;
        param.m_pWidget = m_pInterface;
        param.m_iPart = FWL_PART_EDT_Background;
        param.m_pPath = &path;
        pTheme->DrawBackground(&param);
    }
    CFX_RenderDevice *pRenderDev = pGraphics->GetRenderDevice();
    _FWL_RETURN_IF_FAIL(pRenderDev);
    IFDE_RenderDevice *pRenderDevice = IFDE_RenderDevice::Create(pRenderDev);
    _FWL_RETURN_IF_FAIL(pRenderDevice);
    IFDE_RenderContext *pRenderContext = IFDE_RenderContext::Create();
    _FWL_RETURN_IF_FAIL(pRenderContext);
    pRenderDevice->SetClipRect(rtClip);
    pRenderContext->StartRender(pRenderDevice, (IFDE_CanvasSet*)pPage, mt);
    pRenderContext->DoRender(NULL);
    pRenderContext->Release();
    pRenderDevice->Release();
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_CombText) {
        pGraphics->RestoreGraphState();
        CFX_Path path;
        path.Create();
        FX_INT32 iLimit = m_nLimit > 0 ? m_nLimit : 1;
        FX_FLOAT fStep = m_rtEngine.width / iLimit;
        FX_FLOAT fLeft = m_rtEngine.left + 1;
        for (FX_INT32 i = 1; i < iLimit; i++) {
            fLeft += fStep;
            path.AddLine(fLeft, m_rtClient.top, fLeft, m_rtClient.bottom());
        }
        CFWL_ThemeBackground param;
        param.m_pGraphics = pGraphics;
        param.m_matrix = *pMatrix;
        param.m_pWidget = m_pInterface;
        param.m_iPart = FWL_PART_EDT_CombTextLine;
        param.m_pPath = &path;
        pTheme->DrawBackground(&param);
    }
    pGraphics->RestoreGraphState();
}
void CFWL_EditImp::UpdateEditEngine()
{
    UpdateEditParams();
    UpdateEditLayout();
    if (m_nLimit > -1) {
        m_pEdtEngine->SetLimit(m_nLimit);
    }
}
void CFWL_EditImp::UpdateEditParams()
{
    FDE_TXTEDTPARAMS params;
    params.nHorzScale = 100;
    params.fPlateWidth = m_rtEngine.width;
    params.fPlateHeight = m_rtEngine.height;
    if (m_pProperties->m_dwStyles & FWL_WGTSTYLE_RTLLayout) {
        params.dwLayoutStyles |= FDE_TEXTEDITLAYOUT_RTL;
    }
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_VerticalLayout) {
        params.dwLayoutStyles |= FDE_TEXTEDITLAYOUT_DocVertical;
    }
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_VerticalChars) {
        params.dwLayoutStyles |= FDE_TEXTEDITLAYOUT_CharVertial;
    }
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_ReverseLine) {
        params.dwLayoutStyles |= FDE_TEXTEDITLAYOUT_LineReserve;
    }
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_ArabicShapes) {
        params.dwLayoutStyles |= FDE_TEXTEDITLAYOUT_ArabicShapes;
    }
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_ExpandTab) {
        params.dwLayoutStyles |= FDE_TEXTEDITLAYOUT_ExpandTab;
    }
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_CombText) {
        params.dwLayoutStyles |= FDE_TEXTEDITLAYOUT_CombText;
    }
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_LastLineHeight) {
        params.dwLayoutStyles |= FDE_TEXTEDITLAYOUT_LastLineHeight;
    }
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_Validate) {
        params.dwMode |= FDE_TEXTEDITMODE_Validate;
    }
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_Password) {
        params.dwMode |= FDE_TEXTEDITMODE_Password;
    }
    switch (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_HAlignMask) {
        case FWL_STYLEEXT_EDT_HNear: {
                params.dwAlignment |= FDE_TEXTEDITALIGN_Left;
                break;
            }
        case FWL_STYLEEXT_EDT_HCenter: {
                params.dwAlignment |= FDE_TEXTEDITALIGN_Center;
                break;
            }
        case FWL_STYLEEXT_EDT_HFar: {
                params.dwAlignment |= FDE_TEXTEDITALIGN_Right;
                break;
            }
        default: {
            }
    }
    switch (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_HAlignModeMask) {
        case FWL_STYLEEXT_EDT_Justified: {
                params.dwAlignment |= FDE_TEXTEDITALIGN_Justified;
                break;
            }
        case FWL_STYLEEXT_EDT_Distributed: {
                params.dwAlignment |= FDE_TEXTEDITALIGN_Distributed;
                break;
            }
        default: {
                params.dwAlignment |= FDE_TEXTEDITALIGN_Normal;
            }
    }
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_MultiLine) {
        params.dwMode |= FDE_TEXTEDITMODE_MultiLines;
        if ((m_pProperties->m_dwStyles & FWL_WGTSTYLE_HScroll) == 0 && (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_AutoHScroll) == 0) {
            params.dwMode |= FDE_TEXTEDITMODE_AutoLineWrap | FDE_TEXTEDITMODE_LimitArea_Horz;
        }
        if ((m_pProperties->m_dwStyles & FWL_WGTSTYLE_VScroll) == 0 && (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_AutoVScroll) == 0) {
            params.dwMode |= FDE_TEXTEDITMODE_LimitArea_Vert;
        } else {
            params.fPlateHeight = 0x00FFFFFF;
        }
    } else {
        if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_AutoHScroll) == 0) {
            params.dwMode |= FDE_TEXTEDITMODE_LimitArea_Horz;
        }
    }
    if (	(m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_ReadOnly)
            ||	(m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled)) {
        params.dwMode |= FDE_TEXTEDITMODE_ReadOnly;
    }
    FX_FLOAT *pFontSize = (FX_FLOAT*)GetThemeCapacity(FWL_WGTCAPACITY_FontSize);
    _FWL_RETURN_IF_FAIL(pFontSize);
    m_fFontSize = *pFontSize;
    FX_DWORD *pFontColor = (FX_DWORD*)GetThemeCapacity(FWL_WGTCAPACITY_TextColor);
    _FWL_RETURN_IF_FAIL(pFontColor);
    params.dwFontColor = *pFontColor;
    FX_FLOAT *pLineHeight = (FX_FLOAT*)GetThemeCapacity(FWL_WGTCAPACITY_LineHeight);
    _FWL_RETURN_IF_FAIL(pLineHeight);
    params.fLineSpace = *pLineHeight;
    IFX_Font *pFont = (IFX_Font*)GetThemeCapacity(FWL_WGTCAPACITY_Font);
    _FWL_RETURN_IF_FAIL(pFont);
    params.pFont			= pFont;
    params.fFontSize		= m_fFontSize;
    params.nLineCount		= (FX_INT32)(params.fPlateHeight / params.fLineSpace);
    if (params.nLineCount <= 0) {
        params.nLineCount = 1;
    }
    params.fTabWidth		= params.fFontSize * 1;
    params.bTabEquidistant	= TRUE;
    params.wLineBreakChar	= L'\n';
    params.nCharRotation	= 0;
    params.pEventSink		= this;
    m_pEdtEngine->SetEditParams(params);
}
void CFWL_EditImp::UpdateEditLayout()
{
    if (m_pEdtEngine->GetTextLength() <= 0) {
        m_pEdtEngine->SetTextByStream(NULL);
    }
    IFDE_TxtEdtPage *pPage = m_pEdtEngine->GetPage(0);
    if (pPage) {
        pPage->UnloadPage();
        pPage = NULL;
    }
    m_pEdtEngine->StartLayout();
    m_pEdtEngine->DoLayout(NULL);
    m_pEdtEngine->EndLayout();
    pPage = m_pEdtEngine->GetPage(0);
    if (pPage) {
        pPage->LoadPage();
    }
}
FX_BOOL CFWL_EditImp::UpdateOffset()
{
    CFX_RectF rtCaret;
    m_pEdtEngine->GetCaretRect(rtCaret);
    FX_FLOAT fOffSetX = m_rtEngine.left - m_fScrollOffsetX;
    FX_FLOAT fOffSetY = m_rtEngine.top - m_fScrollOffsetY + m_fVAlignOffset;
    rtCaret.Offset(fOffSetX, fOffSetY);
    const CFX_RectF& rtEidt = m_rtEngine;
    if (rtEidt.Contains(rtCaret)) {
        IFDE_TxtEdtPage *pPage = m_pEdtEngine->GetPage(0);
        _FWL_RETURN_VALUE_IF_FAIL(pPage, FALSE);
        CFX_RectF rtFDE = pPage->GetContentsBox();
        rtFDE.Offset(fOffSetX, fOffSetY);
        if (rtFDE.right() < rtEidt.right() && m_fScrollOffsetX > 0) {
            m_fScrollOffsetX += rtFDE.right() - rtEidt.right();
            if (m_fScrollOffsetX < 0) {
                m_fScrollOffsetX = 0;
            }
        }
        if (rtFDE.bottom() < rtEidt.bottom() && m_fScrollOffsetY > 0) {
            m_fScrollOffsetY += rtFDE.bottom() - rtEidt.bottom();
            if (m_fScrollOffsetY < 0) {
                m_fScrollOffsetY = 0;
            }
        }
        return FALSE;
    } else {
        FX_FLOAT offsetX = 0.0;
        FX_FLOAT offsetY = 0.0;
        if (rtCaret.left < rtEidt.left) {
            offsetX = rtCaret.left - rtEidt.left;
        }
        if (rtCaret.right() > rtEidt.right()) {
            offsetX = rtCaret.right() - rtEidt.right();
        }
        if (rtCaret.top < rtEidt.top) {
            offsetY =  rtCaret.top - rtEidt.top;
        }
        if (rtCaret.bottom() > rtEidt.bottom()) {
            offsetY = rtCaret.bottom() - rtEidt.bottom();
        }
        if (!(m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_HSelfAdaption)) {
            m_fScrollOffsetX += offsetX;
        }
        if (!(m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_VSelfAdaption)) {
            m_fScrollOffsetY += offsetY;
        }
        if (m_fFontSize > m_rtEngine.height) {
            m_fScrollOffsetY = 0;
        }
        return TRUE;
    }
}
FX_BOOL	CFWL_EditImp::UpdateOffset(IFWL_ScrollBar *pScrollBar, FX_FLOAT fPosChanged)
{
    if (pScrollBar == (IFWL_ScrollBar*)m_pHorzScrollBar) {
        m_fScrollOffsetX += fPosChanged;
    } else {
        m_fScrollOffsetY += fPosChanged;
    }
    return TRUE;
}
void CFWL_EditImp::UpdateVAlignment()
{
    IFDE_TxtEdtPage *pPage = m_pEdtEngine->GetPage(0);
    _FWL_RETURN_IF_FAIL(pPage);
    const CFX_RectF& rtFDE = pPage->GetContentsBox();
    FX_FLOAT fOffsetY = 0.0f;
    FX_FLOAT fSpaceAbove = 0.0f;
    FX_FLOAT fSpaceBelow = 0.0f;
    CFX_SizeF* pSpace = (CFX_SizeF*)GetThemeCapacity(FWL_WGTCAPACITY_SpaceAboveBelow);
    if (pSpace) {
        fSpaceAbove = pSpace->x;
        fSpaceBelow = pSpace->y;
    }
    if (fSpaceAbove < 0.1f) {
        fSpaceAbove = 0;
    }
    if (fSpaceBelow < 0.1f) {
        fSpaceBelow = 0;
    }
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_VCenter) {
        fOffsetY = (m_rtEngine.height - rtFDE.height) / 2;
        if (fOffsetY < (fSpaceAbove + fSpaceBelow) / 2 && fSpaceAbove < fSpaceBelow) {
            return;
        }
        fOffsetY += (fSpaceAbove - fSpaceBelow) / 2;
    } else if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_VFar) {
        fOffsetY = (m_rtEngine.height - rtFDE.height);
        fOffsetY -= fSpaceBelow;
    } else {
        fOffsetY += fSpaceAbove;
    }
    m_fVAlignOffset = fOffsetY;
    if (m_fVAlignOffset < 0) {
        m_fVAlignOffset = 0;
    }
}
void CFWL_EditImp::UpdateCaret()
{
    CFX_RectF rtFDE;
    m_pEdtEngine->GetCaretRect(rtFDE);
    rtFDE.Offset(m_rtEngine.left - m_fScrollOffsetX, m_rtEngine.top - m_fScrollOffsetY + m_fVAlignOffset);
    CFX_RectF rtCaret;
    rtCaret.Set(rtFDE.left, rtFDE.top, rtFDE.width, rtFDE.height);
    CFX_RectF temp = rtCaret;
    CFX_RectF rtClient;
    GetClientRect(rtClient);
    rtCaret.Intersect(rtClient);
    if (rtCaret.left > rtClient.right()) {
        FX_FLOAT right = rtCaret.right();
        rtCaret.left = rtClient.right() - 1;
        rtCaret.width = right - rtCaret.left;
    }
    FX_BOOL bIntersect = !rtCaret.IsEmpty();
    FX_BOOL bShow = TRUE;
    FX_BOOL bShowWhole = FALSE;
    if (!(m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) || !bIntersect) {
        bShow = FALSE;
    }
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_HSelfAdaption && temp.right() > m_rtEngine.right()) {
        bShowWhole = TRUE;
    }
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_VSelfAdaption && temp.bottom() > m_rtEngine.bottom()) {
        bShowWhole = TRUE;
    } else {
        bShow = (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused && bIntersect);
    }
    if (bShowWhole) {
        rtCaret = temp;
    }
    ShowCaret(bShow, &rtCaret);
}
IFWL_ScrollBar* CFWL_EditImp::UpdateScroll()
{
    FX_BOOL bShowHorz = m_pHorzScrollBar && ((m_pHorzScrollBar->GetStates() & FWL_WGTSTATE_Invisible) == 0);
    FX_BOOL bShowVert = m_pVertScrollBar && ((m_pVertScrollBar->GetStates() & FWL_WGTSTATE_Invisible) == 0);
    if (!bShowHorz && !bShowVert) {
        return NULL;
    }
    IFDE_TxtEdtPage *pPage = m_pEdtEngine->GetPage(0);
    _FWL_RETURN_VALUE_IF_FAIL(pPage, NULL);
    const CFX_RectF &rtFDE = pPage->GetContentsBox();
    IFWL_ScrollBar *pRepaint = NULL;
    if (bShowHorz) {
        CFX_RectF rtScroll;
        m_pHorzScrollBar->GetWidgetRect(rtScroll);
        if (rtScroll.width < rtFDE.width) {
            m_pHorzScrollBar->LockUpdate();
            FX_FLOAT fRange = rtFDE.width - rtScroll.width;
            m_pHorzScrollBar->SetRange(0.0f, fRange);
            FX_FLOAT fPos = m_fScrollOffsetX;
            if (fPos < 0.0f) {
                fPos = 0.0f;
            }
            if (fPos > fRange) {
                fPos = fRange;
            }
            m_pHorzScrollBar->SetPos(fPos);
            m_pHorzScrollBar->SetTrackPos(fPos);
            m_pHorzScrollBar->SetPageSize(rtScroll.width);
            m_pHorzScrollBar->SetStepSize(rtScroll.width / 10);
            m_pHorzScrollBar->SetStates(FWL_WGTSTATE_Disabled, FALSE);
            m_pHorzScrollBar->UnlockUpdate();
            m_pHorzScrollBar->Update();
            pRepaint = (IFWL_ScrollBar*)m_pHorzScrollBar;
        } else if ((m_pHorzScrollBar->GetStates() & FWL_WGTSTATE_Disabled) == 0) {
            m_pHorzScrollBar->LockUpdate();
            m_pHorzScrollBar->SetRange(0, -1);
            m_pHorzScrollBar->SetStates(FWL_WGTSTATE_Disabled, TRUE);
            m_pHorzScrollBar->UnlockUpdate();
            m_pHorzScrollBar->Update();
            pRepaint = (IFWL_ScrollBar*)m_pHorzScrollBar;
        }
    }
    if (bShowVert) {
        CFX_RectF rtScroll;
        m_pVertScrollBar->GetWidgetRect(rtScroll);
        if (rtScroll.height < rtFDE.height) {
            m_pVertScrollBar->LockUpdate();
            FX_FLOAT fStep = m_pEdtEngine->GetEditParams()->fLineSpace;
            FX_FLOAT fRange = rtFDE.height - m_rtEngine.height;
            if (fRange < fStep) {
                fRange = fStep;
            }
            m_pVertScrollBar->SetRange(0.0f, fRange);
            FX_FLOAT fPos = m_fScrollOffsetY;
            if (fPos < 0.0f) {
                fPos = 0.0f;
            }
            if (fPos > fRange) {
                fPos = fRange;
            }
            m_pVertScrollBar->SetPos(fPos);
            m_pVertScrollBar->SetTrackPos(fPos);
            m_pVertScrollBar->SetPageSize(rtScroll.height);
            m_pVertScrollBar->SetStepSize(fStep);
            m_pVertScrollBar->SetStates(FWL_WGTSTATE_Disabled, FALSE);
            m_pVertScrollBar->UnlockUpdate();
            m_pVertScrollBar->Update();
            pRepaint = (IFWL_ScrollBar*)m_pVertScrollBar;
        } else if ((m_pVertScrollBar->GetStates() & FWL_WGTSTATE_Disabled) == 0) {
            m_pVertScrollBar->LockUpdate();
            m_pVertScrollBar->SetRange(0, -1);
            m_pVertScrollBar->SetStates(FWL_WGTSTATE_Disabled, TRUE);
            m_pVertScrollBar->UnlockUpdate();
            m_pVertScrollBar->Update();
            pRepaint = (IFWL_ScrollBar*)m_pVertScrollBar;
        }
    }
    return pRepaint;
}
FX_BOOL	CFWL_EditImp::IsShowScrollBar(FX_BOOL bVert)
{
    FX_BOOL bShow = (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_ShowScrollbarFocus) ? (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) == FWL_WGTSTATE_Focused : TRUE;
    if (bVert) {
        return bShow && (m_pProperties->m_dwStyles & FWL_WGTSTYLE_VScroll) &&
               (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_MultiLine) && IsContentHeightOverflow();
    }
    return  bShow && (m_pProperties->m_dwStyles & FWL_WGTSTYLE_HScroll) && (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_MultiLine);
}
FX_BOOL	CFWL_EditImp::IsContentHeightOverflow()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pEdtEngine, FALSE);
    IFDE_TxtEdtPage *pPage = m_pEdtEngine->GetPage(0);
    _FWL_RETURN_VALUE_IF_FAIL(pPage, FALSE);
    return pPage->GetContentsBox().height > m_rtEngine.height + 1.0f;
}
FX_INT32 CFWL_EditImp::AddDoRecord(FX_BSTR bsDoRecord)
{
    FX_INT32 nCount = m_RecordArr.GetSize();
    if (m_iCurRecord == nCount - 1) {
        if (nCount == m_iMaxRecord) {
            m_RecordArr.RemoveAt(0);
            m_iCurRecord --;
        }
    } else {
        for (FX_INT32 i = nCount - 1; i > m_iCurRecord; i --) {
            m_RecordArr.RemoveAt(i);
        }
    }
    m_RecordArr.Add(bsDoRecord);
    return m_iCurRecord = m_RecordArr.GetSize() - 1;
}
void CFWL_EditImp::Layout()
{
    GetClientRect(m_rtClient);
    m_rtEngine = m_rtClient;
    FX_FLOAT *pfWidth = (FX_FLOAT*)GetThemeCapacity(FWL_WGTCAPACITY_ScrollBarWidth);
    _FWL_RETURN_IF_FAIL(pfWidth);
    FX_FLOAT fWidth = *pfWidth;
    if (!m_pOuter) {
        CFX_RectF* pUIMargin = (CFX_RectF*)GetThemeCapacity(FWL_WGTCAPACITY_UIMargin);
        if (pUIMargin) {
            m_rtEngine.Deflate(pUIMargin->left, pUIMargin->top, pUIMargin->width, pUIMargin->height);
        }
    } else if (m_pOuter->GetClassID() == FWL_CLASSHASH_DateTimePicker) {
        CFWL_ThemePart part;
        part.m_pWidget = m_pOuter;
        CFX_RectF* pUIMargin = (CFX_RectF*)m_pOuter->GetThemeProvider()->GetCapacity(&part, FWL_WGTCAPACITY_UIMargin);
        if (pUIMargin) {
            m_rtEngine.Deflate(pUIMargin->left, pUIMargin->top, pUIMargin->width, pUIMargin->height);
        }
    }
    FX_BOOL bShowVertScrollbar = IsShowScrollBar(TRUE);
    FX_BOOL bShowHorzScrollbar = IsShowScrollBar(FALSE);
    if (bShowVertScrollbar) {
        InitScrollBar();
        CFX_RectF rtVertScr;
        if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_OuterScrollbar) {
            rtVertScr.Set(m_rtClient.right() + FWL_EDIT_Margin,
                          m_rtClient.top, fWidth, m_rtClient.height);
        } else {
            rtVertScr.Set(m_rtClient.right() - fWidth, m_rtClient.top, fWidth, m_rtClient.height);
            if (bShowHorzScrollbar) {
                rtVertScr.height -= fWidth;
            }
            m_rtEngine.width -= fWidth;
        }
        m_pVertScrollBar->SetWidgetRect(rtVertScr);
        m_pVertScrollBar->SetStates(FWL_WGTSTATE_Invisible, FALSE);
        m_pVertScrollBar->Update();
    } else if (m_pVertScrollBar) {
        m_pVertScrollBar->SetStates(FWL_WGTSTATE_Invisible, TRUE);
    }
    if (bShowHorzScrollbar) {
        InitScrollBar(FALSE);
        CFX_RectF rtHoriScr;
        if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_OuterScrollbar) {
            rtHoriScr.Set(m_rtClient.left, m_rtClient.bottom() + FWL_EDIT_Margin, m_rtClient.width, fWidth);
        } else {
            rtHoriScr.Set(m_rtClient.left, m_rtClient.bottom() - fWidth, m_rtClient.width, fWidth);
            if (bShowVertScrollbar) {
                rtHoriScr.width -= fWidth;
            }
            m_rtEngine.height -= fWidth;
        }
        m_pHorzScrollBar->SetWidgetRect(rtHoriScr);
        m_pHorzScrollBar->SetStates(FWL_WGTSTATE_Invisible, FALSE);
        m_pHorzScrollBar->Update();
    } else if (m_pHorzScrollBar) {
        m_pHorzScrollBar->SetStates(FWL_WGTSTATE_Invisible, TRUE);
    }
}
void CFWL_EditImp::LayoutScrollBar()
{
    if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_ShowScrollbarFocus) == 0) {
        return;
    }
    FX_FLOAT* pfWidth = NULL;
    FX_BOOL bShowVertScrollbar = IsShowScrollBar(TRUE);
    FX_BOOL bShowHorzScrollbar = IsShowScrollBar(FALSE);
    if (bShowVertScrollbar) {
        if (!m_pVertScrollBar) {
            pfWidth = (FX_FLOAT*)GetThemeCapacity(FWL_WGTCAPACITY_ScrollBarWidth);
            FX_FLOAT fWidth = pfWidth ? *pfWidth : 0;
            InitScrollBar();
            CFX_RectF rtVertScr;
            if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_OuterScrollbar) {
                rtVertScr.Set(m_rtClient.right() + FWL_EDIT_Margin,
                              m_rtClient.top, fWidth, m_rtClient.height);
            } else {
                rtVertScr.Set(m_rtClient.right() - fWidth, m_rtClient.top, fWidth, m_rtClient.height);
                if (bShowHorzScrollbar) {
                    rtVertScr.height -= fWidth;
                }
            }
            m_pVertScrollBar->SetWidgetRect(rtVertScr);
            m_pVertScrollBar->Update();
        }
        m_pVertScrollBar->SetStates(FWL_WGTSTATE_Invisible, FALSE);
    } else if (m_pVertScrollBar) {
        m_pVertScrollBar->SetStates(FWL_WGTSTATE_Invisible, TRUE);
    }
    if (bShowHorzScrollbar) {
        if (!m_pHorzScrollBar) {
            pfWidth = !pfWidth ? (FX_FLOAT*)GetThemeCapacity(FWL_WGTCAPACITY_ScrollBarWidth) : pfWidth;
            FX_FLOAT fWidth = pfWidth ? *pfWidth : 0;
            InitScrollBar(FALSE);
            CFX_RectF rtHoriScr;
            if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_OuterScrollbar) {
                rtHoriScr.Set(m_rtClient.left, m_rtClient.bottom() + FWL_EDIT_Margin, m_rtClient.width, fWidth);
            } else {
                rtHoriScr.Set(m_rtClient.left, m_rtClient.bottom() - fWidth, m_rtClient.width, fWidth);
                if (bShowVertScrollbar) {
                    rtHoriScr.width -= (fWidth);
                }
            }
            m_pHorzScrollBar->SetWidgetRect(rtHoriScr);
            m_pHorzScrollBar->Update();
        }
        m_pHorzScrollBar->SetStates(FWL_WGTSTATE_Invisible, FALSE);
    } else if (m_pHorzScrollBar) {
        m_pHorzScrollBar->SetStates(FWL_WGTSTATE_Invisible, TRUE);
    }
    if (bShowVertScrollbar || bShowHorzScrollbar) {
        UpdateScroll();
    }
}
void CFWL_EditImp::DeviceToEngine(CFX_PointF &pt)
{
    pt.x += -m_rtEngine.left + m_fScrollOffsetX;
    pt.y += -m_rtEngine.top - m_fVAlignOffset + m_fScrollOffsetY;
}
void CFWL_EditImp::InitScrollBar(FX_BOOL bVert )
{
    if ((bVert && m_pVertScrollBar) || (!bVert && m_pHorzScrollBar)) {
        return;
    }
    CFWL_WidgetImpProperties prop;
    prop.m_dwStyleExes = bVert ? FWL_STYLEEXT_SCB_Vert : FWL_STYLEEXT_SCB_Horz;
    prop.m_dwStates = FWL_WGTSTATE_Disabled | FWL_WGTSTATE_Invisible;
    prop.m_pParent = m_pInterface;
    prop.m_pThemeProvider = m_pProperties->m_pThemeProvider;
    IFWL_ScrollBar *pScrollBar = IFWL_ScrollBar::Create();
    pScrollBar->Initialize(prop, m_pInterface);
    bVert ? (m_pVertScrollBar = pScrollBar) : (m_pHorzScrollBar = pScrollBar);
}
void CFWL_EditImp::InitEngine()
{
    if (m_pEdtEngine) {
        return;
    }
    m_pEdtEngine = IFDE_TxtEdtEngine::Create();
}
extern 	FX_BOOL	FWL_ShowCaret(IFWL_Widget *pWidget, FX_BOOL bVisible, const CFX_RectF *pRtAnchor);
void CFWL_EditImp::ShowCaret(FX_BOOL bVisible, CFX_RectF *pRect )
{
    if (m_pCaret) {
        m_pCaret->ShowCaret(bVisible);
        if (bVisible && !pRect->IsEmpty()) {
            m_pCaret->SetWidgetRect(*pRect);
        }
        Repaint(&m_rtEngine);
    } else {
        IFWL_Widget *pOuter = (IFWL_Widget*)m_pInterface;
        if (bVisible) {
            pRect->Offset(m_pProperties->m_rtWidget.left, m_pProperties->m_rtWidget.top);
        }
        while (pOuter->GetOuter()) {
            pOuter = pOuter->GetOuter();
            if (bVisible) {
                CFX_RectF rtOuter;
                pOuter->GetWidgetRect(rtOuter);
                pRect->Offset(rtOuter.left, rtOuter.top);
            }
        }
        FWL_ShowCaret(pOuter, bVisible, pRect);
    }
}
FX_BOOL	CFWL_EditImp::ValidateNumberChar(FX_WCHAR cNum)
{
    if (!m_pEdtEngine) {
        return FALSE;
    }
    if (!m_bSetRange) {
        return TRUE;
    }
    CFX_WideString wsOld, wsText;
    m_pEdtEngine->GetText(wsText, 0);
    if (wsText.IsEmpty()) {
        if (cNum == L'0') {
            return FALSE;
        }
        return TRUE;
    }
    FX_INT32 caretPos = m_pEdtEngine->GetCaretPos();
    FX_INT32 iSel =	CountSelRanges();
    if (iSel == 0) {
        if (cNum == L'0' && caretPos == 0) {
            return FALSE;
        }
        FX_INT32 nLen = wsText.GetLength();
        CFX_WideString l = wsText.Mid(0, caretPos);
        CFX_WideString r = wsText.Mid(caretPos, nLen - caretPos);
        CFX_WideString wsNew = l + cNum + r;
        if (wsNew.GetInteger() <= m_iMax) {
            return TRUE;
        }
    } else {
        if (wsText.GetInteger() <= m_iMax) {
            return TRUE;
        }
    }
    return FALSE;
}
void CFWL_EditImp::InitCaret()
{
    if (!m_pCaret) {
        if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_InnerCaret)) {
            m_pCaret = IFWL_Caret::Create();
            m_pCaret->Initialize(m_pInterface);
            m_pCaret->SetParent(m_pInterface);
            m_pCaret->SetStates(m_pProperties->m_dwStates);
        }
    } else if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_InnerCaret) == 0) {
        m_pCaret->Release();
        m_pCaret = NULL;
    }
}
void CFWL_EditImp::ClearRecord()
{
    m_iCurRecord = -1;
    m_RecordArr.RemoveAll();
}
void CFWL_EditImp::ProcessInsertError(FX_INT32 iError)
{
    switch (iError) {
        case -2: {
                CFWL_EvtEdtTextFull textFullEvent;
                textFullEvent.m_pSrcTarget = m_pInterface;
                DispatchEvent(&textFullEvent);
                break;
            }
        default: {
            }
    }
}
CFWL_EditImpDelegate::CFWL_EditImpDelegate(CFWL_EditImp *pOwner)
    : m_pOwner(pOwner)
{
}
FX_INT32 CFWL_EditImpDelegate::OnProcessMessage(CFWL_Message *pMessage)
{
    _FWL_RETURN_VALUE_IF_FAIL(pMessage, 0);
    FX_DWORD dwMsgCode = pMessage->GetClassID();
    FX_INT32 iRet = 1;
    switch (dwMsgCode) {
        case FWL_MSGHASH_Activate: {
                DoActivate((CFWL_MsgActivate*)pMessage);
                break;
            }
        case FWL_MSGHASH_Deactivate: {
                DoDeactivate((CFWL_MsgDeactivate*)pMessage);
                break;
            }
        case FWL_MSGHASH_SetFocus:
        case FWL_MSGHASH_KillFocus: {
                OnFocusChanged(pMessage, dwMsgCode == FWL_MSGHASH_SetFocus);
                break;
            }
        case FWL_MSGHASH_Mouse: {
                CFWL_MsgMouse *pMsg = (CFWL_MsgMouse*)pMessage;
                FX_DWORD dwCmd = pMsg->m_dwCmd;
                switch(dwCmd) {
                    case FWL_MSGMOUSECMD_LButtonDown: {
                            OnLButtonDown(pMsg);
                            break;
                        }
                    case FWL_MSGMOUSECMD_LButtonUp: {
                            OnLButtonUp(pMsg);
                            break;
                        }
                    case FWL_MSGMOUSECMD_LButtonDblClk: {
                            OnButtonDblClk(pMsg);
                            break;
                        }
                    case FWL_MSGMOUSECMD_MouseMove: {
                            OnMouseMove(pMsg);
                            break;
                        }
                    case FWL_MSGMOUSECMD_RButtonDown: {
                            DoButtonDown(pMsg);
                            break;
                        }
                    default: {
                        }
                }
                break;
            }
        case FWL_MSGHASH_Key: {
                CFWL_MsgKey *pKey = (CFWL_MsgKey*)pMessage;
                FX_DWORD dwCmd = pKey->m_dwCmd;
                if (dwCmd == FWL_MSGKEYCMD_KeyDown) {
                    OnKeyDown(pKey);
                } else if (dwCmd == FWL_MSGKEYCMD_Char) {
                    OnChar(pKey);
                }
                break;
            }
        default: {
                iRet = 0;
            }
    }
    CFWL_WidgetImpDelegate::OnProcessMessage(pMessage);
    return iRet;
}
FWL_ERR CFWL_EditImpDelegate::OnProcessEvent(CFWL_Event *pEvent)
{
    _FWL_RETURN_VALUE_IF_FAIL(pEvent, FWL_ERR_Indefinite);
    FX_DWORD dwHashCode = pEvent->GetClassID();
    if (dwHashCode != FWL_EVTHASH_Scroll) {
        return FWL_ERR_Succeeded;
    }
    IFWL_Widget *pSrcTarget = pEvent->m_pSrcTarget;
    if ((pSrcTarget == (IFWL_Widget*)m_pOwner->m_pVertScrollBar && m_pOwner->m_pVertScrollBar) || (pSrcTarget == (IFWL_Widget*)m_pOwner->m_pHorzScrollBar && m_pOwner->m_pHorzScrollBar)) {
        FX_DWORD dwScrollCode = ((CFWL_EvtScroll*)pEvent)->m_iScrollCode;
        FX_FLOAT fPos = ((CFWL_EvtScroll*)pEvent)->m_fPos;
        OnScroll((IFWL_ScrollBar*)pSrcTarget, dwScrollCode, fPos);
    }
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_EditImpDelegate::OnDrawWidget(CFX_Graphics *pGraphics, const CFX_Matrix *pMatrix)
{
    return m_pOwner->DrawWidget(pGraphics, pMatrix);
}
void CFWL_EditImpDelegate::DoActivate(CFWL_MsgActivate *pMsg)
{
    m_pOwner->m_pProperties->m_dwStates |= ~FWL_WGTSTATE_Deactivated;
    m_pOwner->Repaint(&m_pOwner->m_rtClient);
}
void CFWL_EditImpDelegate::DoDeactivate(CFWL_MsgDeactivate *pMsg)
{
    m_pOwner->m_pProperties->m_dwStates &= FWL_WGTSTATE_Deactivated;
    m_pOwner->Repaint(&m_pOwner->m_rtClient);
}
void CFWL_EditImpDelegate::DoButtonDown(CFWL_MsgMouse *pMsg)
{
    if ((m_pOwner->m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) == 0) {
        m_pOwner->SetFocus(TRUE);
    }
    if (!m_pOwner->m_pEdtEngine) {
        m_pOwner->UpdateEditEngine();
    }
    IFDE_TxtEdtPage *pPage = m_pOwner->m_pEdtEngine->GetPage(0);
    _FWL_RETURN_IF_FAIL(pPage);
    CFX_PointF pt;
    pt.Set(pMsg->m_fx, pMsg->m_fy);
    m_pOwner->DeviceToEngine(pt);
    FX_BOOL bBefore = TRUE;
    FX_INT32 nIndex = pPage->GetCharIndex(pt, bBefore);
    if (nIndex < 0) {
        nIndex = 0;
    }
    m_pOwner->m_pEdtEngine->SetCaretPos(nIndex, bBefore);
}
void CFWL_EditImpDelegate::OnFocusChanged(CFWL_Message *pMsg, FX_BOOL bSet)
{
    FX_DWORD dwStyleEx = m_pOwner->GetStylesEx();
    FX_BOOL bRepaint = dwStyleEx & FWL_STYLEEXT_EDT_InnerCaret;
    if (bSet) {
        m_pOwner->m_pProperties->m_dwStates |= FWL_WGTSTATE_Focused;
        if (!m_pOwner->m_pEdtEngine) {
            m_pOwner->UpdateEditEngine();
        }
        m_pOwner->UpdateVAlignment();
        m_pOwner->UpdateOffset();
        m_pOwner->UpdateCaret();
    } else if (m_pOwner->m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) {
        m_pOwner->m_pProperties->m_dwStates &= ~FWL_WGTSTATE_Focused;
        m_pOwner->ShowCaret(FALSE);
        if (m_pOwner->m_pEdtEngine && (dwStyleEx & FWL_STYLEEXT_EDT_NoHideSel) == 0) {
            FX_INT32 nSel = m_pOwner->CountSelRanges();
            if (nSel > 0) {
                m_pOwner->ClearSelections();
                bRepaint = TRUE;
            }
            m_pOwner->SetCaretPos(0);
            m_pOwner->UpdateOffset();
        }
        m_pOwner->ClearRecord();
    }
    m_pOwner->LayoutScrollBar();
    if (bRepaint) {
        CFX_RectF rtInvalidate;
        rtInvalidate.Set(0,
                         0,
                         m_pOwner->m_pProperties->m_rtWidget.width,
                         m_pOwner->m_pProperties->m_rtWidget.height);
        m_pOwner->Repaint(&rtInvalidate);
    }
}
void CFWL_EditImpDelegate::OnLButtonDown(CFWL_MsgMouse *pMsg)
{
    DoCursor(pMsg);
    if(m_pOwner->m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled) {
        return;
    }
    m_pOwner->m_bLButtonDown = TRUE;
    m_pOwner->SetGrab(TRUE);
    DoButtonDown(pMsg);
    FX_INT32 nIndex = m_pOwner->m_pEdtEngine->GetCaretPos();
    FX_BOOL bRepaint = FALSE;
    FX_INT32 iCount = m_pOwner->m_pEdtEngine->CountSelRanges();
    if (iCount > 0) {
        m_pOwner->m_pEdtEngine->ClearSelection();
        bRepaint = TRUE;
    }
    FX_BOOL bShift = pMsg->m_dwFlags & FWL_KEYFLAG_Shift;
    if (bShift && m_pOwner->m_nSelStart != nIndex) {
        FX_INT32 iStart = FX_MIN(m_pOwner->m_nSelStart, nIndex);
        FX_INT32 iEnd = FX_MAX(m_pOwner->m_nSelStart, nIndex);
        m_pOwner->m_pEdtEngine->AddSelRange(iStart, iEnd - iStart);
        bRepaint = TRUE;
    } else {
        m_pOwner->m_nSelStart = nIndex;
    }
    if (bRepaint) {
        m_pOwner->Repaint(&m_pOwner->m_rtEngine);
    }
}
void CFWL_EditImpDelegate::OnLButtonUp(CFWL_MsgMouse *pMsg)
{
    DoCursor(pMsg);
    m_pOwner->m_bLButtonDown = FALSE;
    m_pOwner->SetGrab(FALSE);
}
void CFWL_EditImpDelegate::OnButtonDblClk(CFWL_MsgMouse *pMsg)
{
    _FWL_RETURN_IF_FAIL(m_pOwner->m_pEdtEngine);
    DoCursor(pMsg);
    IFDE_TxtEdtPage *pPage = m_pOwner->m_pEdtEngine->GetPage(0);
    _FWL_RETURN_IF_FAIL(pPage);
    CFX_PointF pt;
    pt.Set(pMsg->m_fx, pMsg->m_fy);
    m_pOwner->DeviceToEngine(pt);
    FX_INT32 nCount = 0;
    FX_INT32 nIndex = pPage->SelectWord(pt, nCount);
    if (nIndex < 0) {
        return;
    }
    m_pOwner->m_pEdtEngine->AddSelRange(nIndex, nCount);
    m_pOwner->m_pEdtEngine->SetCaretPos(nIndex + nCount - 1, FALSE);
    m_pOwner->Repaint(&m_pOwner->m_rtEngine);
}
void CFWL_EditImpDelegate::OnMouseMove(CFWL_MsgMouse *pMsg)
{
    _FWL_RETURN_IF_FAIL(m_pOwner->m_pEdtEngine);
    DoCursor(pMsg);
    if (m_pOwner->m_nSelStart == -1 || !m_pOwner->m_bLButtonDown) {
        return;
    }
    IFDE_TxtEdtPage *pPage = m_pOwner->m_pEdtEngine->GetPage(0);
    _FWL_RETURN_IF_FAIL(pPage);
    CFX_PointF pt;
    pt.Set(pMsg->m_fx, pMsg->m_fy);
    m_pOwner->DeviceToEngine(pt);
    FX_BOOL bBefore = TRUE;
    FX_INT32 nIndex = pPage->GetCharIndex(pt, bBefore);
    m_pOwner->m_pEdtEngine->SetCaretPos(nIndex, bBefore);
    nIndex = m_pOwner->m_pEdtEngine->GetCaretPos();
    m_pOwner->m_pEdtEngine->ClearSelection();
    if (nIndex != m_pOwner->m_nSelStart) {
        FX_INT32 nLen = m_pOwner->m_pEdtEngine->GetTextLength();
        if (m_pOwner->m_nSelStart >= nLen) {
            m_pOwner->m_nSelStart = nLen;
        }
        m_pOwner->m_pEdtEngine->AddSelRange(FX_MIN(m_pOwner->m_nSelStart, nIndex), FXSYS_abs(nIndex - m_pOwner->m_nSelStart));
    }
}
void CFWL_EditImpDelegate::OnKeyDown(CFWL_MsgKey *pMsg)
{
    _FWL_RETURN_IF_FAIL(m_pOwner->m_pEdtEngine);
    FDE_TXTEDTMOVECARET MoveCaret = MC_MoveNone;
    FX_BOOL bShift = pMsg->m_dwFlags & FWL_KEYFLAG_Shift;
    FX_BOOL bCtrl = pMsg->m_dwFlags & FWL_KEYFLAG_Ctrl;
    FX_DWORD dwKeyCode = pMsg->m_dwKeyCode;
    switch (dwKeyCode) {
        case FWL_VKEY_Left: {
                MoveCaret = MC_Left;
                break;
            }
        case FWL_VKEY_Right: {
                MoveCaret = MC_Right;
                break;
            }
        case FWL_VKEY_Up: {
                MoveCaret = MC_Up;
                break;
            }
        case FWL_VKEY_Down: {
                MoveCaret = MC_Down;
                break;
            }
        case FWL_VKEY_Home: {
                if (bCtrl) {
                    MoveCaret = MC_Home;
                } else {
                    MoveCaret = MC_LineStart;
                }
                break;
            }
        case FWL_VKEY_End: {
                if (bCtrl) {
                    MoveCaret = MC_End;
                } else {
                    MoveCaret = MC_LineEnd;
                }
                break;
            }
        case FWL_VKEY_Insert: {
                break;
            }
        case FWL_VKEY_Delete: {
                if (	(m_pOwner->m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_ReadOnly)
                        ||	(m_pOwner->m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled)) {
                    break;
                }
                FX_INT32 nCaret = m_pOwner->m_pEdtEngine->GetCaretPos();
#if (_FX_OS_==_FX_MACOSX_)
                m_pOwner->m_pEdtEngine->Delete(nCaret, TRUE);
#else
                m_pOwner->m_pEdtEngine->Delete(nCaret);
#endif
                break;
            }
        case FWL_VKEY_F2: {
                break;
            }
        case FWL_VKEY_Tab: {
                m_pOwner->DispatchKeyEvent(pMsg);
                break;
            }
        default: {
#if (_FX_OS_==_FX_MACOSX_)
                if (pMsg->m_dwFlags & FWL_KEYFLAG_Command)
#else
                if (pMsg->m_dwFlags & FWL_KEYFLAG_Ctrl)
#endif
                {
                    if (dwKeyCode == 0x43 || dwKeyCode == 0x63) {
                        m_pOwner->DoClipboard(1);
                        return;
                    }
                    if (dwKeyCode == 0x58 || dwKeyCode == 0x78) {
                        m_pOwner->DoClipboard(2);
                        return;
                    }
                    if (dwKeyCode == 0x56 || dwKeyCode == 0x76) {
                        m_pOwner->DoClipboard(3);
                        return;
                    }
                }
            }
    }
    if (MoveCaret != MC_MoveNone) {
        m_pOwner->m_pEdtEngine->MoveCaretPos(MoveCaret, bShift, bCtrl);
    }
}
void CFWL_EditImpDelegate::OnChar(CFWL_MsgKey *pMsg)
{
    if (	(m_pOwner->m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_ReadOnly)
            ||	( m_pOwner->m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled)) {
        return;
    }
    _FWL_RETURN_IF_FAIL(m_pOwner->m_pEdtEngine);
    FX_INT32 iError = 0;
    FX_WCHAR c = (FX_WCHAR)pMsg->m_dwKeyCode;
    FX_INT32 nCaret = m_pOwner->m_pEdtEngine->GetCaretPos();
    switch (c) {
        case FWL_VKEY_Back: {
                m_pOwner->m_pEdtEngine->Delete(nCaret, TRUE);
                break;
            }
        case 0x0A: {
                break;
            }
        case FWL_VKEY_Escape: {
                break;
            }
        case FWL_VKEY_Tab: {
                iError = m_pOwner->m_pEdtEngine->Insert(nCaret, FX_LPCWSTR(L"\t"), 1);
                break;
            }
        case FWL_VKEY_Return: {
                if (m_pOwner->m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_WantReturn) {
                    iError = m_pOwner->m_pEdtEngine->Insert(nCaret, FX_LPCWSTR(L"\n"), 1);
                }
                break;
            }
        default: {
                if (!m_pOwner->m_pWidgetMgr->IsFormDisabled()) {
                    if (m_pOwner->m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_Number) {
                        if (((pMsg->m_dwKeyCode < FWL_VKEY_0) && (pMsg->m_dwKeyCode != 0x2E && pMsg->m_dwKeyCode != 0x2D)) || pMsg->m_dwKeyCode > FWL_VKEY_9) {
                            break;
                        }
                        if (!m_pOwner->ValidateNumberChar(c)) {
                            break;
                        }
                    }
                }
#if (_FX_OS_==_FX_MACOSX_)
                if (pMsg->m_dwFlags & FWL_KEYFLAG_Command)
#else
                if (pMsg->m_dwFlags & FWL_KEYFLAG_Ctrl)
#endif
                {
                    break;
                }
                iError = m_pOwner->m_pEdtEngine->Insert(nCaret, &c, 1);
                break;
            }
    }
    if (iError < 0) {
        m_pOwner->ProcessInsertError(iError);
    }
}
FX_BOOL CFWL_EditImpDelegate::OnScroll(IFWL_ScrollBar *pScrollBar, FX_DWORD dwCode, FX_FLOAT fPos)
{
    CFX_SizeF fs;
    pScrollBar->GetRange(fs.x, fs.y);
    FX_FLOAT iCurPos = pScrollBar->GetPos();
    FX_FLOAT fStep = pScrollBar->GetStepSize();
    switch (dwCode) {
        case FWL_SCBCODE_Min: {
                fPos = fs.x;
                break;
            }
        case FWL_SCBCODE_Max: {
                fPos = fs.y;
                break;
            }
        case FWL_SCBCODE_StepBackward: {
                fPos -= fStep;
                if (fPos < fs.x + fStep / 2) {
                    fPos = fs.x;
                }
                break;
            }
        case FWL_SCBCODE_StepForward: {
                fPos += fStep;
                if (fPos > fs.y - fStep / 2) {
                    fPos = fs.y;
                }
                break;
            }
        case FWL_SCBCODE_PageBackward: {
                fPos -= pScrollBar->GetPageSize();
                if (fPos < fs.x) {
                    fPos = fs.x;
                }
                break;
            }
        case FWL_SCBCODE_PageForward: {
                fPos += pScrollBar->GetPageSize();
                if (fPos > fs.y) {
                    fPos = fs.y;
                }
                break;
            }
        case FWL_SCBCODE_Pos:
        case FWL_SCBCODE_TrackPos: {
                break;
            }
        case FWL_SCBCODE_EndScroll: {
                return FALSE;
            }
        default: {
            }
    }
    if (iCurPos != fPos) {
        pScrollBar->SetPos(fPos);
        pScrollBar->SetTrackPos(fPos);
        m_pOwner->UpdateOffset(pScrollBar, fPos - iCurPos);
        if (m_pOwner->m_pEdtEngine) {
            m_pOwner->UpdateCaret();
        }
        CFX_RectF rect;
        m_pOwner->GetWidgetRect(rect);
        CFX_RectF rtInvalidate;
        rtInvalidate.Set(0, 0, rect.width + 2, rect.height + 2);
        m_pOwner->Repaint(&rtInvalidate);
    }
    return TRUE;
}
void CFWL_EditImpDelegate::DoCursor(CFWL_MsgMouse *pMsg)
{
    if (m_pOwner->m_rtClient.Contains(pMsg->m_fx, pMsg->m_fy)) {
        IFWL_AdapterNative *pNative = FWL_GetAdapterNative();
        IFWL_AdapterCursorMgr *pCursorMgr = pNative->GetCursorMgr();
        if (NULL != pCursorMgr) {
            FWL_HCURSOR hCursor = pCursorMgr->GetSystemCursor(FWL_CURSORTYPE_InputBeam);
            pCursorMgr->SetCursor(hCursor);
            pCursorMgr->ShowCursor(TRUE);
        }
    }
}
