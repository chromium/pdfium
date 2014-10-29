// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_FWL_ADAPTER_IMP_H
#define _XFA_FWL_ADAPTER_IMP_H
class CXFA_FWLAdapterWidgetMgr : public CFWL_SDAdapterWidgetMgr
{
public:
    virtual FWL_ERR RepaintWidget(IFWL_Widget *pWidget, const CFX_RectF *pRect);
    virtual FX_BOOL	 GetPopupPos(IFWL_Widget* pWidget, FX_FLOAT fMinHeight, FX_FLOAT fMaxHeight, const CFX_RectF &rtAnchor, CFX_RectF &rtPopup);
};
class IXFA_FWLEventHandler
{
public:
    virtual FX_BOOL		GetPopupPos(IFWL_Widget *pWidget, FX_FLOAT fMinPopup, FX_FLOAT fMaxPopup,
                                    const CFX_RectF &rtAnchor, CFX_RectF& rtPopup)
    {
        return FALSE;
    }
    virtual void		OnPreOpen(IFWL_Widget *pWidget) {}
    virtual void		OnPostOpen(IFWL_Widget *pWidget) {}
    virtual	void		OnSelectChanged(IFWL_Widget *pWidget, FX_INT32 iYear, FX_INT32 iMonth, FX_INT32 iDay) {}
    virtual	void		OnTextChanged(IFWL_Widget *pWidget, const CFX_WideString &wsChanged) {}
    virtual void		OnTextFull(IFWL_Widget *pWidget) {}
    virtual FX_BOOL		OnValidate(IFWL_Widget *pWidget, CFX_WideString &wsText)
    {
        return FALSE;
    }
    virtual	void		OnSelectChanged(IFWL_Widget *pWidget, const CFX_Int32Array &arrSels) {}
    virtual void		OnAddDoRecord(IFWL_Widget *pWidget) {}
    virtual CXFA_Edge	GetComboSplitColor(IFWL_Widget *pWidget)
    {
        return CXFA_Edge(NULL);
    }
    virtual void		GetUIMargin(CFX_RectF &rtMargin) {}
    virtual FX_BOOL		CheckWord(FX_BSTR sWord)
    {
        return TRUE;
    }
    virtual FX_BOOL		GetSuggestWords(FX_BSTR sWord, CFX_ByteStringArray &sSuggest)
    {
        return FALSE;
    }
};
#endif
