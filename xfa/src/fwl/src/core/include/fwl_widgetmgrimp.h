// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_WIDGETMGR_IMP_H
#define _FWL_WIDGETMGR_IMP_H
class CFWL_TargetImp;
class IFWL_Widget;
class IFWL_AdapterWidgetMgr;
class CFWL_WidgetMgrItem;
class CFWL_WidgetMgr;
class CFWL_WidgetMgrDelegate;
class IFWL_AdapterNative;
class CFWL_WidgetMgrItem {
 public:
  CFWL_WidgetMgrItem()
      : pParent(NULL),
        pOwner(NULL),
        pChild(NULL),
        pPrevious(NULL),
        pNext(NULL),
        pWidget(NULL),
        pOffscreen(NULL),
        iRedrawCounter(0)
#if (_FX_OS_ == _FX_WIN32_DESKTOP_) || (_FX_OS_ == _FX_WIN64_)
        ,
        bOutsideChanged(FALSE)
#endif
  {
  }
  ~CFWL_WidgetMgrItem() {
    if (pOffscreen) {
      delete pOffscreen;
      pOffscreen = NULL;
    }
  }
  CFWL_WidgetMgrItem* pParent;
  CFWL_WidgetMgrItem* pOwner;
  CFWL_WidgetMgrItem* pChild;
  CFWL_WidgetMgrItem* pPrevious;
  CFWL_WidgetMgrItem* pNext;
  IFWL_Widget* pWidget;
  CFX_Graphics* pOffscreen;
  int32_t iRedrawCounter;
#if (_FX_OS_ == _FX_WIN32_DESKTOP_) || (_FX_OS_ == _FX_WIN64_)
  FX_BOOL bOutsideChanged;
#endif
};

class CFWL_WidgetMgr : public IFWL_WidgetMgr {
 public:
  CFWL_WidgetMgr(IFWL_AdapterNative* pAdapterNative);
  ~CFWL_WidgetMgr() override;

  // IFWL_WidgetMgr:
  int32_t CountWidgets(IFWL_Widget* pParent = NULL) override;
  IFWL_Widget* GetWidget(int32_t nIndex, IFWL_Widget* pParent = NULL) override;
  IFWL_Widget* GetWidget(IFWL_Widget* pWidget,
                         FWL_WGTRELATION eRelation) override;
  int32_t GetWidgetIndex(IFWL_Widget* pWidget) override;
  FX_BOOL SetWidgetIndex(IFWL_Widget* pWidget, int32_t nIndex) override;
  FWL_ERR RepaintWidget(IFWL_Widget* pWidget,
                        const CFX_RectF* pRect = NULL) override;
  FX_DWORD GetCapability() override { return m_dwCapability; }

  void AddWidget(IFWL_Widget* pWidget);
  void InsertWidget(IFWL_Widget* pParent,
                    IFWL_Widget* pChild,
                    int32_t nIndex = -1);
  void RemoveWidget(IFWL_Widget* pWidget);
  void SetOwner(IFWL_Widget* pOwner, IFWL_Widget* pOwned);
  void SetParent(IFWL_Widget* pParent, IFWL_Widget* pChild);
  FX_BOOL IsChild(IFWL_Widget* pChild, IFWL_Widget* pParent);
  FWL_ERR CreateWidget_Native(IFWL_Widget* pWidget);
  FWL_ERR DestroyWidget_Native(IFWL_Widget* pWidget);
  FWL_ERR GetWidgetRect_Native(IFWL_Widget* pWidget, CFX_RectF& rect);
  FWL_ERR SetWidgetRect_Native(IFWL_Widget* pWidget, const CFX_RectF& rect);
  FWL_ERR SetWidgetPosition_Native(IFWL_Widget* pWidget,
                                   FX_FLOAT fx,
                                   FX_FLOAT fy);
  FWL_ERR SetWidgetIcon_Native(IFWL_Widget* pWidget,
                               const CFX_DIBitmap* pIcon,
                               FX_BOOL bBig);
  FWL_ERR SetWidgetCaption_Native(IFWL_Widget* pWidget,
                                  const CFX_WideStringC& wsCaption);
  FWL_ERR SetBorderRegion_Native(IFWL_Widget* pWidget, CFX_Path* pPath);
  FWL_ERR ShowWidget_Native(IFWL_Widget* pWidget);
  FWL_ERR HideWidget_Native(IFWL_Widget* pWidget);
  FWL_ERR SetNormal_Native(IFWL_Widget* pWidget);
  FWL_ERR SetMaximize_Native(IFWL_Widget* pWidget);
  FWL_ERR SetMinimize_Native(IFWL_Widget* pWidget);
  FX_BOOL CheckMessage_Native();
  FWL_ERR DispatchMessage_Native();
  FX_BOOL IsIdleMessage_Native();
  FWL_ERR Exit_Native(int32_t iExitCode);
  FWL_ERR CreateWidgetWithNativeId_Native(IFWL_Widget* pWidget, void* vp);
  IFWL_Widget* GetWidgetAtPoint(IFWL_Widget* pParent, FX_FLOAT fx, FX_FLOAT fy);
  void NotifySizeChanged(IFWL_Widget* pForm, FX_FLOAT fx, FX_FLOAT fy);
  IFWL_Widget* nextTab(IFWL_Widget* parent, IFWL_Widget* focus, FX_BOOL& bFind);
  int32_t CountRadioButtonGroup(IFWL_Widget* pFirst);
  IFWL_Widget* GetSiblingRadioButton(IFWL_Widget* pWidget, FX_BOOL bNext);
  IFWL_Widget* GetRadioButtonGroupHeader(IFWL_Widget* pRadioButton);
  void GetSameGroupRadioButton(IFWL_Widget* pRadioButton, CFX_PtrArray& group);
  IFWL_Widget* GetDefaultButton(IFWL_Widget* pParent);
  void AddRedrawCounts(IFWL_Widget* pWidget);
  void ResetRedrawCounts(IFWL_Widget* pWidget);
  IFWL_AdapterWidgetMgr* GetAdapterWidgetMgr() { return m_pAdapter; }
  CFWL_WidgetMgrDelegate* GetDelegate() { return m_pDelegate; }
  CFWL_WidgetMgrItem* GetWidgetMgrItem(IFWL_Widget* pWidget);
  FX_BOOL IsThreadEnabled();
  FX_BOOL IsFormDisabled();
  FX_BOOL GetAdapterPopupPos(IFWL_Widget* pWidget,
                             FX_FLOAT fMinHeight,
                             FX_FLOAT fMaxHeight,
                             const CFX_RectF& rtAnchor,
                             CFX_RectF& rtPopup);

 protected:
  int32_t TravelWidgetMgr(CFWL_WidgetMgrItem* pParent,
                          int32_t* pIndex,
                          CFWL_WidgetMgrItem* pItem,
                          IFWL_Widget** pWidget = NULL);
  FX_BOOL IsAbleNative(IFWL_Widget* pWidget);
  CFX_MapPtrToPtr m_mapWidgetItem;
  IFWL_AdapterWidgetMgr* m_pAdapter;
  CFWL_WidgetMgrDelegate* m_pDelegate;
  friend class CFWL_WidgetMgrDelegate;
  FX_DWORD m_dwCapability;
#if (_FX_OS_ == _FX_WIN32_DESKTOP_) || (_FX_OS_ == _FX_WIN64_)
  CFX_RectF m_rtScreen;
#endif
};

class CFWL_WidgetMgrDelegate : public IFWL_WidgetMgrDelegate {
 public:
  CFWL_WidgetMgrDelegate(CFWL_WidgetMgr* pWidgetMgr);
  ~CFWL_WidgetMgrDelegate() override {}

  // IFWL_WidgetMgrDelegate:
  FWL_ERR OnSetCapability(
      FX_DWORD dwCapability = FWL_WGTMGR_DisableThread) override;
  int32_t OnProcessMessageToForm(CFWL_Message* pMessage) override;
  FWL_ERR OnDrawWidget(IFWL_Widget* pWidget,
                       CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix) override;

 protected:
  void DrawChild(IFWL_Widget* pParent,
                 const CFX_RectF& rtClip,
                 CFX_Graphics* pGraphics,
                 const CFX_Matrix* pMatrix);
  CFX_Graphics* DrawWidgetBefore(IFWL_Widget* pWidget,
                                 CFX_Graphics* pGraphics,
                                 const CFX_Matrix* pMatrix);
  void DrawWidgetAfter(IFWL_Widget* pWidget,
                       CFX_Graphics* pGraphics,
                       CFX_RectF& rtClip,
                       const CFX_Matrix* pMatrix);
  FX_BOOL IsNeedRepaint(IFWL_Widget* pWidget,
                        CFX_Matrix* pMatrix,
                        const CFX_RectF& rtDirty);
  FX_BOOL bUseOffscreenDirect(IFWL_Widget* pWidget);

  CFWL_WidgetMgr* m_pWidgetMgr;
};
#endif
