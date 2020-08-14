// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_WIDGETMGR_H_
#define XFA_FWL_CFWL_WIDGETMGR_H_

#include <map>
#include <memory>

#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/tree_node.h"
#include "xfa/fxgraphics/cxfa_graphics.h"

class CFWL_Message;
class CXFA_Graphics;
class CFX_Matrix;
class CFWL_Widget;

class CFWL_WidgetMgr {
 public:
  class AdapterIface {
   public:
    virtual ~AdapterIface() {}
    virtual void RepaintWidget(CFWL_Widget* pWidget) = 0;
    virtual bool GetPopupPos(CFWL_Widget* pWidget,
                             float fMinHeight,
                             float fMaxHeight,
                             const CFX_RectF& rtAnchor,
                             CFX_RectF* pPopupRect) = 0;
  };

  explicit CFWL_WidgetMgr(AdapterIface* pAdapterNative);
  ~CFWL_WidgetMgr();

  void OnProcessMessageToForm(std::unique_ptr<CFWL_Message> pMessage);
  void OnDrawWidget(CFWL_Widget* pWidget,
                    CXFA_Graphics* pGraphics,
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
  class Item : public TreeNode<Item> {
   public:
    Item();
    explicit Item(CFWL_Widget* widget);
    ~Item() final;

    CFWL_Widget* const pWidget;
  };

  CFWL_Widget* GetPriorSiblingWidget(CFWL_Widget* pWidget) const;
  CFWL_Widget* GetLastChildWidget(CFWL_Widget* pWidget) const;

  Item* GetWidgetMgrRootItem() const;
  Item* GetWidgetMgrItem(const CFWL_Widget* pWidget) const;
  Item* CreateWidgetMgrItem(CFWL_Widget* pWidget);

  void DrawChildren(CFWL_Widget* pParent,
                    const CFX_RectF& rtClip,
                    CXFA_Graphics* pGraphics,
                    const CFX_Matrix* pMatrix);

  std::map<const CFWL_Widget*, std::unique_ptr<Item>> m_mapWidgetItem;
  UnownedPtr<AdapterIface> const m_pAdapter;
};

#endif  // XFA_FWL_CFWL_WIDGETMGR_H_
