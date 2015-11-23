// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_FORMFILLER_PAGEVIEW_IMP_H
#define _FXFA_FORMFILLER_PAGEVIEW_IMP_H
class CXFA_FFWidget;
class CXFA_FFDocView;
class CXFA_FFPageView : public CXFA_ContainerLayoutItem, public IXFA_PageView {
 public:
  CXFA_FFPageView(CXFA_FFDocView* pDocView, CXFA_Node* pPageArea);
  ~CXFA_FFPageView() override;

  // IFXA_PageView:
  IXFA_DocView* GetDocView() override;
  int32_t GetPageViewIndex() override;
  void GetPageViewRect(CFX_RectF& rtPage) override;
  void GetDisplayMatrix(CFX_Matrix& mt,
                        const CFX_Rect& rtDisp,
                        int32_t iRotate) override;
  int32_t LoadPageView(IFX_Pause* pPause = NULL) override;
  void UnloadPageView() override;
  IXFA_Widget* GetWidgetByPos(FX_FLOAT fx, FX_FLOAT fy) override;
  IXFA_WidgetIterator* CreateWidgetIterator(
      FX_DWORD dwTraverseWay = XFA_TRAVERSEWAY_Form,
      FX_DWORD dwWidgetFilter = XFA_WIDGETFILTER_Visible |
                                XFA_WIDGETFILTER_Viewable |
                                XFA_WIDGETFILTER_AllType) override;

  FX_BOOL IsPageViewLoaded();

 protected:
  CXFA_FFDocView* m_pDocView;
  FX_BOOL m_bLoaded;
};
typedef CXFA_NodeIteratorTemplate<CXFA_LayoutItem,
                                  CXFA_TraverseStrategy_LayoutItem>
    CXFA_LayoutItemIterator;
class CXFA_FFPageWidgetIterator : public IXFA_WidgetIterator {
 public:
  CXFA_FFPageWidgetIterator(CXFA_FFPageView* pPageView, FX_DWORD dwFilter);
  virtual ~CXFA_FFPageWidgetIterator();
  virtual void Release() { delete this; }

  virtual void Reset();
  virtual IXFA_Widget* MoveToFirst();
  virtual IXFA_Widget* MoveToLast();
  virtual IXFA_Widget* MoveToNext();
  virtual IXFA_Widget* MoveToPrevious();
  virtual IXFA_Widget* GetCurrentWidget();
  virtual FX_BOOL SetCurrentWidget(IXFA_Widget* hWidget);

 protected:
  IXFA_Widget* GetWidget(CXFA_LayoutItem* pLayoutItem);
  CXFA_FFPageView* m_pPageView;
  IXFA_Widget* m_hCurWidget;
  FX_DWORD m_dwFilter;
  FX_BOOL m_bIgnorerelevant;
  CXFA_LayoutItemIterator m_sIterator;
};
typedef CFX_ArrayTemplate<CXFA_FFWidget*> CXFA_WidgetArray;
class CXFA_TabParam {
 public:
  CXFA_TabParam() : m_pWidget(NULL) {}
  ~CXFA_TabParam() {}

  CXFA_FFWidget* m_pWidget;
  CXFA_WidgetArray m_Children;
};
class CXFA_FFTabOrderPageWidgetIterator : public IXFA_WidgetIterator {
 public:
  CXFA_FFTabOrderPageWidgetIterator(CXFA_FFPageView* pPageView,
                                    FX_DWORD dwFilter);
  virtual ~CXFA_FFTabOrderPageWidgetIterator();

  virtual void Release();

  virtual void Reset();
  virtual IXFA_Widget* MoveToFirst();
  virtual IXFA_Widget* MoveToLast();
  virtual IXFA_Widget* MoveToNext();
  virtual IXFA_Widget* MoveToPrevious();
  virtual IXFA_Widget* GetCurrentWidget();
  virtual FX_BOOL SetCurrentWidget(IXFA_Widget* hWidget);

 protected:
  CXFA_WidgetArray m_TabOrderWidgetArray;
  CXFA_FFPageView* m_pPageView;
  FX_DWORD m_dwFilter;
  int32_t m_iCurWidget;
  FX_BOOL m_bIgnorerelevant;
  CXFA_FFWidget* GetTraverseWidget(CXFA_FFWidget* pWidget);
  CXFA_FFWidget* FindWidgetByName(const CFX_WideStringC& wsWidgetName,
                                  CXFA_FFWidget* pRefWidget);
  void CreateTabOrderWidgetArray();
  void CreateSpaceOrderWidgetArray(CXFA_WidgetArray& WidgetArray);
  CXFA_FFWidget* GetWidget(CXFA_LayoutItem* pLayoutItem);
  void OrderContainer(CXFA_LayoutItemIterator* sIterator,
                      CXFA_LayoutItem* pContainerItem,
                      CXFA_TabParam* pContainer,
                      FX_BOOL& bCurrentItem,
                      FX_BOOL& bContentArea,
                      FX_BOOL bMarsterPage = FALSE);
};
#endif
