// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_CFWL_PUSHBUTTON_H_
#define XFA_FWL_CORE_CFWL_PUSHBUTTON_H_

#include "xfa/fwl/core/cfwl_widget.h"
#include "xfa/fwl/core/ifwl_pushbutton.h"

class CFWL_PushButton : public CFWL_Widget {
 public:
  CFWL_PushButton(const IFWL_App*);
  ~CFWL_PushButton() override;

  void Initialize();

  FWL_Error GetCaption(CFX_WideString& wsCaption);
  FWL_Error SetCaption(const CFX_WideStringC& wsCaption);
  CFX_DIBitmap* GetPicture();
  FWL_Error SetPicture(CFX_DIBitmap* pBitmap);

 protected:
  class CFWL_PushButtonDP : public IFWL_PushButtonDP {
   public:
    CFWL_PushButtonDP() : m_pBitmap(nullptr) {}

    // IFWL_DataProvider
    FWL_Error GetCaption(IFWL_Widget* pWidget,
                         CFX_WideString& wsCaption) override;

    // IFWL_PushButtonDP
    CFX_DIBitmap* GetPicture(IFWL_Widget* pWidget) override;

    CFX_WideString m_wsCaption;
    CFX_DIBitmap* m_pBitmap;
  };

  CFWL_PushButtonDP m_buttonData;
};

#endif  // XFA_FWL_CORE_CFWL_PUSHBUTTON_H_
