// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_FORMFILLER_PAGEVIEW_IMP_H
#define _FXFA_FORMFILLER_PAGEVIEW_IMP_H
class CXFA_FFWidget;
class CXFA_FFDocView;
class CXFA_FFPageView : public CXFA_ContainerLayoutItemImpl, public IXFA_PageView
{
public:
    CXFA_FFPageView(CXFA_FFDocView* pDocView, CXFA_Node* pPageArea);
    ~CXFA_FFPageView();
    virtual IXFA_DocView*	GetDocView();
    virtual FX_INT32		GetPageViewIndex();
    virtual void			GetPageViewRect(CFX_RectF &rtPage);
    virtual void			GetDisplayMatrix(CFX_Matrix &mt, const CFX_Rect &rtDisp, FX_INT32 iRotate);
    virtual FX_INT32		LoadPageView(IFX_Pause *pPause = NULL);
    virtual void			UnloadPageView();
    FX_BOOL					IsPageViewLoaded();

    virtual XFA_HWIDGET		GetWidgetByPos(FX_FLOAT fx, FX_FLOAT fy);
    virtual IXFA_WidgetIterator* CreateWidgetIterator(FX_DWORD dwTraverseWay = XFA_TRAVERSEWAY_Form,
            FX_DWORD dwWidgetFilter = XFA_WIDGETFILTER_Visible | XFA_WIDGETFILTER_Viewable | XFA_WIDGETFILTER_AllType);
    IXFA_LayoutPage*		GetLayoutPage()
    {
        return (IXFA_LayoutPage*)this;
    }
protected:
    CXFA_FFDocView*			m_pDocView;
    FX_BOOL					m_bLoaded;
};
typedef		CXFA_NodeIteratorTemplate<CXFA_LayoutItem, CXFA_TraverseStrategy_LayoutItem> CXFA_LayoutItemIterator;
class CXFA_FFPageWidgetIterator : public IXFA_WidgetIterator, public CFX_Object
{
public:
    CXFA_FFPageWidgetIterator(CXFA_FFPageView* pPageView, FX_DWORD dwFilter);
    virtual ~CXFA_FFPageWidgetIterator();
    virtual void				Release()
    {
        delete this;
    }

    virtual void				Reset();
    virtual XFA_HWIDGET			MoveToFirst();
    virtual XFA_HWIDGET			MoveToLast();
    virtual XFA_HWIDGET			MoveToNext();
    virtual XFA_HWIDGET			MoveToPrevious();
    virtual XFA_HWIDGET			GetCurrentWidget();
    virtual FX_BOOL				SetCurrentWidget(XFA_HWIDGET hWidget);
protected:
    XFA_HWIDGET					GetWidget(CXFA_LayoutItem* pLayoutItem);
    CXFA_FFPageView*			m_pPageView;
    XFA_HWIDGET					m_hCurWidget;
    FX_DWORD					m_dwFilter;
    FX_BOOL						m_bIgnorerelevant;
    CXFA_LayoutItemIterator		m_sIterator;
};
typedef 	CFX_ArrayTemplate<CXFA_FFWidget*> CXFA_WidgetArray;
class CXFA_TabParam : public CFX_Object
{
public:
    CXFA_TabParam() : m_pWidget(NULL) {}
    ~CXFA_TabParam() {}

    CXFA_FFWidget*		m_pWidget;
    CXFA_WidgetArray		m_Children;
};
class CXFA_FFTabOrderPageWidgetIterator : public IXFA_WidgetIterator, public CFX_Object
{
public:
    CXFA_FFTabOrderPageWidgetIterator(CXFA_FFPageView* pPageView, FX_DWORD dwFilter);
    virtual ~CXFA_FFTabOrderPageWidgetIterator();

    virtual void Release();

    virtual void Reset();
    virtual XFA_HWIDGET MoveToFirst();
    virtual XFA_HWIDGET MoveToLast();
    virtual XFA_HWIDGET MoveToNext();
    virtual XFA_HWIDGET MoveToPrevious();
    virtual XFA_HWIDGET GetCurrentWidget();
    virtual FX_BOOL		SetCurrentWidget(XFA_HWIDGET hWidget);
protected:
    CXFA_WidgetArray						m_TabOrderWidgetArray;
    CXFA_FFPageView*					m_pPageView;
    FX_DWORD							m_dwFilter;
    FX_INT32							m_iCurWidget;
    FX_BOOL								m_bIgnorerelevant;
    CXFA_FFWidget*			GetTraverseWidget(CXFA_FFWidget* pWidget);
    CXFA_FFWidget*			FindWidgetByName(FX_WSTR wsWidgetName, CXFA_FFWidget* pRefWidget);
    void					CreateTabOrderWidgetArray();
    void					CreateSpaceOrderWidgetArray(CXFA_WidgetArray& WidgetArray);
    CXFA_FFWidget*			GetWidget(CXFA_LayoutItem* pLayoutItem);
    void					OrderContainer(CXFA_LayoutItemIterator* sIterator, CXFA_LayoutItem* pContainerItem, CXFA_TabParam* pContainer, FX_BOOL &bCurrentItem, FX_BOOL& bContentArea, FX_BOOL bMarsterPage = FALSE);
};
#endif
