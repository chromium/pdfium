// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_FWL_ADAPTER_IMP_H
#define _XFA_FWL_ADAPTER_IMP_H
class CXFA_FWLAdapterWidgetMgr : public CFWL_SDAdapterWidgetMgr {
 public:
  virtual FWL_ERR RepaintWidget(IFWL_Widget* pWidget, const CFX_RectF* pRect);
  virtual FX_BOOL GetPopupPos(IFWL_Widget* pWidget,
                              FX_FLOAT fMinHeight,
                              FX_FLOAT fMaxHeight,
                              const CFX_RectF& rtAnchor,
                              CFX_RectF& rtPopup);
};
#endif
