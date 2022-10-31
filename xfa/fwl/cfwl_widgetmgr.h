// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_WIDGETMGR_H_
#define XFA_FWL_CFWL_WIDGETMGR_H_

#include <map>

#include "core/fxcrt/fx_coordinates.h"
#include "fxjs/gc/gced_tree_node.h"
#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/visitor.h"

class CFGAS_GEGraphics;
class CFWL_App;
class CFWL_Message;
class CFWL_Widget;

class CFWL_WidgetMgr final : public cppgc::GarbageCollected<CFWL_WidgetMgr> {
 public:
  class AdapterIface : public cppgc::GarbageCollectedMixin {
   public:
    virtual ~AdapterIface() = default;
    virtual void RepaintWidget(CFWL_Widget* pWidget) = 0;
    virtual bool GetPopupPos(CFWL_Widget* pWidget,
                             float fMinHeight,
                             float fMaxHeight,
                             const CFX_RectF& rtAnchor,
                             CFX_RectF* pPopupRect) = 0;
  };

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_WidgetMgr();

  void Trace(cppgc::Visitor* visitor) const;

  void OnProcessMessageToForm(CFWL_Message* pMessage);
  void OnDrawWidget(CFWL_Widget* pWidget,
                    CFGAS_GEGraphics* pGraphics,
                    const CFX_Matrix& matrix);

  CFWL_Widget* GetParentWidget(const CFWL_Widget* pWidget) const;
  CFWL_Widget* GetNextSiblingWidget(CFWL_Widget* pWidget) const;
  CFWL_Widget* GetFirstChildWidget(CFWL_Widget* pWidget) const;

  void RepaintWidget(CFWL_Widget* pWidget, const CFX_RectF& pRect);

  void InsertWidget(CFWL_Widget* pParent, CFWL_Widget* pChild);
  void RemoveWidget(CFWL_Widget* pWidget);

  CFWL_Widget* GetWidgetAtPoint(CFWL_Widget* pParent,
                                const CFX_PointF& point) const;

  CFWL_Widget* GetDefaultButton(CFWL_Widget* pParent) const;
  void GetAdapterPopupPos(CFWL_Widget* pWidget,
                          float fMinHeight,
                          float fMaxHeight,
                          const CFX_RectF& rtAnchor,
                          CFX_RectF* pPopupRect) const;

 private:
  class Item final : public GCedTreeNode<Item> {
   public:
    CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
    ~Item() final;

    // GcedTreeNode:
    void Trace(cppgc::Visitor* visitor) const override;

    cppgc::Member<CFWL_Widget> const pWidget;

   private:
    explicit Item(CFWL_Widget* widget);
  };

  CFWL_WidgetMgr(AdapterIface* pAdapter, CFWL_App* pApp);

  CFWL_Widget* GetPriorSiblingWidget(CFWL_Widget* pWidget) const;
  CFWL_Widget* GetLastChildWidget(CFWL_Widget* pWidget) const;

  Item* GetWidgetMgrRootItem() const;
  Item* GetWidgetMgrItem(const CFWL_Widget* pWidget) const;
  Item* CreateWidgetMgrItem(CFWL_Widget* pWidget);

  void DrawChildren(CFWL_Widget* pParent,
                    const CFX_RectF& rtClip,
                    CFGAS_GEGraphics* pGraphics,
                    const CFX_Matrix& mtMatrix);

  cppgc::Member<AdapterIface> const m_pAdapter;
  cppgc::Member<CFWL_App> const m_pApp;
  std::map<const CFWL_Widget*, cppgc::Member<Item>> m_mapWidgetItem;
};

#endif  // XFA_FWL_CFWL_WIDGETMGR_H_
