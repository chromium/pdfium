// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_PUSHBUTTON_LIGHT_H
#define _FWL_PUSHBUTTON_LIGHT_H
class CFWL_Widget;
class CFWL_WidgetProperties;
class IFWL_PushButtonDP;
class CFWL_PushButton;
class CFWL_PushButtonDP;
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
#endif
