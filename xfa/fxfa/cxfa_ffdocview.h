// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFDOCVIEW_H_
#define XFA_FXFA_CXFA_FFDOCVIEW_H_

#include <map>
#include <memory>
#include <vector>

#include "core/fxcrt/unowned_ptr.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffdoc.h"

class CXFA_BindItems;
class CXFA_FFWidgetHandler;
class CXFA_FFDoc;
class CXFA_FFWidget;
class CXFA_Subform;
class CXFA_WidgetAccIterator;

extern const XFA_AttributeEnum gs_EventActivity[];
enum XFA_DOCVIEW_LAYOUTSTATUS {
  XFA_DOCVIEW_LAYOUTSTATUS_None,
  XFA_DOCVIEW_LAYOUTSTATUS_Start,
  XFA_DOCVIEW_LAYOUTSTATUS_FormInitialize,
  XFA_DOCVIEW_LAYOUTSTATUS_FormInitCalculate,
  XFA_DOCVIEW_LAYOUTSTATUS_FormInitValidate,
  XFA_DOCVIEW_LAYOUTSTATUS_FormFormReady,
  XFA_DOCVIEW_LAYOUTSTATUS_Doing,
  XFA_DOCVIEW_LAYOUTSTATUS_PagesetInitialize,
  XFA_DOCVIEW_LAYOUTSTATUS_PagesetInitCalculate,
  XFA_DOCVIEW_LAYOUTSTATUS_PagesetInitValidate,
  XFA_DOCVIEW_LAYOUTSTATUS_PagesetFormReady,
  XFA_DOCVIEW_LAYOUTSTATUS_LayoutReady,
  XFA_DOCVIEW_LAYOUTSTATUS_DocReady,
  XFA_DOCVIEW_LAYOUTSTATUS_End
};

class CXFA_FFDocView {
 public:
  explicit CXFA_FFDocView(CXFA_FFDoc* pDoc);
  ~CXFA_FFDocView();

  CXFA_FFDoc* GetDoc() { return m_pDoc.Get(); }
  int32_t StartLayout(int32_t iStartPage = 0);
  int32_t DoLayout();
  void StopLayout();
  int32_t GetLayoutStatus() const { return m_iStatus; }
  void UpdateDocView();
  int32_t CountPageViews() const;
  CXFA_FFPageView* GetPageView(int32_t nIndex) const;

  void ResetWidgetAcc(CXFA_WidgetAcc* pWidgetAcc);
  int32_t ProcessWidgetEvent(CXFA_EventParam* pParam,
                             CXFA_WidgetAcc* pWidgetAcc);
  CXFA_FFWidgetHandler* GetWidgetHandler();
  std::unique_ptr<CXFA_WidgetAccIterator> CreateWidgetAccIterator();
  CXFA_FFWidget* GetFocusWidget() const { return m_pFocusWidget.Get(); }
  void KillFocus();
  bool SetFocus(CXFA_FFWidget* hWidget);
  CXFA_FFWidget* GetWidgetForNode(CXFA_Node* node);
  CXFA_FFWidget* GetWidgetByName(const WideString& wsName,
                                 CXFA_FFWidget* pRefWidget);
  CXFA_WidgetAcc* GetWidgetAccByName(const WideString& wsName,
                                     CXFA_WidgetAcc* pRefWidgetAcc);
  CXFA_LayoutProcessor* GetXFALayout() const;
  void OnPageEvent(CXFA_ContainerLayoutItem* pSender, uint32_t dwEvent);
  void LockUpdate() { m_iLock++; }
  void UnlockUpdate() { m_iLock--; }
  bool IsUpdateLocked() { return m_iLock > 0; }
  void ClearInvalidateList() { m_mapPageInvalidate.clear(); }
  void AddInvalidateRect(CXFA_FFWidget* pWidget, const CFX_RectF& rtInvalidate);
  void AddInvalidateRect(CXFA_FFPageView* pPageView,
                         const CFX_RectF& rtInvalidate);
  void RunInvalidate();
  void RunDocClose();
  void DestroyDocView();

  void ProcessValueChanged(CXFA_WidgetAcc* widgetAcc);

  bool InitValidate(CXFA_Node* pNode);
  bool RunValidate();

  void SetChangeMark();

  void AddValidateWidget(CXFA_WidgetAcc* pWidget);
  void AddCalculateNodeNotify(CXFA_Node* pNodeChange);
  void AddCalculateWidgetAcc(CXFA_WidgetAcc* pWidgetAcc);
  int32_t RunCalculateWidgets();
  bool IsStaticNotify() {
    return m_pDoc->GetFormType() == FormType::kXFAForeground;
  }
  bool RunLayout();
  void RunSubformIndexChange();
  void AddNewFormNode(CXFA_Node* pNode);
  void AddIndexChangedSubform(CXFA_Node* pNode);
  CXFA_WidgetAcc* GetFocusWidgetAcc() const { return m_pFocusAcc.Get(); }
  void SetFocusWidgetAcc(CXFA_WidgetAcc* pWidgetAcc);
  void DeleteLayoutItem(CXFA_FFWidget* pWidget);
  int32_t ExecEventActivityByDeepFirst(CXFA_Node* pFormNode,
                                       XFA_EVENTTYPE eEventType,
                                       bool bIsFormReady,
                                       bool bRecursive,
                                       CXFA_Node* pExclude);

  void AddBindItem(CXFA_BindItems* item) { m_BindItems.push_back(item); }

  bool m_bLayoutEvent;
  std::vector<WideString> m_arrNullTestMsg;
  CXFA_FFWidget* m_pListFocusWidget;
  bool m_bInLayoutStatus;

 private:
  bool RunEventLayoutReady();
  void RunBindItems();
  void InitCalculate(CXFA_Node* pNode);
  void InitLayout(CXFA_Node* pNode);
  size_t RunCalculateRecursive(size_t index);
  void ShowNullTestMsg();
  bool ResetSingleWidgetAccData(CXFA_WidgetAcc* pWidgetAcc);
  CXFA_Subform* GetRootSubform();

  UnownedPtr<CXFA_FFDoc> const m_pDoc;
  std::unique_ptr<CXFA_FFWidgetHandler> m_pWidgetHandler;
  CXFA_LayoutProcessor* m_pXFADocLayout;  // Not owned.
  UnownedPtr<CXFA_WidgetAcc> m_pFocusAcc;
  UnownedPtr<CXFA_FFWidget> m_pFocusWidget;
  UnownedPtr<CXFA_FFWidget> m_pOldFocusWidget;
  std::map<CXFA_FFPageView*, std::unique_ptr<CFX_RectF>> m_mapPageInvalidate;
  std::vector<CXFA_WidgetAcc*> m_ValidateAccs;
  std::vector<CXFA_WidgetAcc*> m_CalculateAccs;
  std::vector<CXFA_BindItems*> m_BindItems;
  std::vector<CXFA_Node*> m_NewAddedNodes;
  std::vector<CXFA_Node*> m_IndexChangedSubforms;
  XFA_DOCVIEW_LAYOUTSTATUS m_iStatus;
  int32_t m_iLock;
};

#endif  // XFA_FXFA_CXFA_FFDOCVIEW_H_
