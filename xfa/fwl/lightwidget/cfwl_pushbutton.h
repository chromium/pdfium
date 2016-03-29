// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_LIGHTWIDGET_CFWL_PUSHBUTTON_H_
#define XFA_FWL_LIGHTWIDGET_CFWL_PUSHBUTTON_H_

#include "xfa/fwl/basewidget/ifwl_pushbutton.h"
#include "xfa/fwl/lightwidget/cfwl_widget.h"

class CFWL_PushButton : public CFWL_Widget {
 public:
  static CFWL_PushButton* Create();
  FWL_ERR Initialize(const CFWL_WidgetProperties* pProperties = NULL);
  FWL_ERR GetCaption(CFX_WideString& wsCaption);
  FWL_ERR SetCaption(const CFX_WideStringC& wsCaption);
  CFX_DIBitmap* GetPicture();
  FWL_ERR SetPicture(CFX_DIBitmap* pBitmap);
  CFWL_PushButton();
  virtual ~CFWL_PushButton();

 protected:
  class CFWL_PushButtonDP : public IFWL_PushButtonDP {
   public:
    CFWL_PushButtonDP() : m_pBitmap(NULL) {}
    FWL_ERR GetCaption(IFWL_Widget* pWidget, CFX_WideString& wsCaption);
    virtual CFX_DIBitmap* GetPicture(IFWL_Widget* pWidget);
    CFX_WideString m_wsCaption;
    CFX_DIBitmap* m_pBitmap;
  };
  CFWL_PushButtonDP m_buttonData;
};

#endif  // XFA_FWL_LIGHTWIDGET_CFWL_PUSHBUTTON_H_
