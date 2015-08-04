// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_CHECKBOX_LIGHT_H
#define _FWL_CHECKBOX_LIGHT_H
class CFWL_Widget;
class CFWL_WidgetProperties;
class IFWL_CheckBoxDP;
class CFWL_CheckBox;
class CFWL_CheckBoxDP;
class CFWL_CheckBox : public CFWL_Widget {
 public:
  static CFWL_CheckBox* Create();

  FWL_ERR Initialize(const CFWL_WidgetProperties* pProperties = NULL);
  FWL_ERR GetCaption(CFX_WideString& wsCaption);
  FWL_ERR SetCaption(const CFX_WideStringC& wsCaption);
  FWL_ERR SetBoxSize(FX_FLOAT fHeight);
  int32_t GetCheckState();
  FWL_ERR SetCheckState(int32_t iCheck);
  CFWL_CheckBox();
  virtual ~CFWL_CheckBox();

 protected:
  class CFWL_CheckBoxDP : public IFWL_CheckBoxDP {
   public:
    CFWL_CheckBoxDP();
    virtual FWL_ERR GetCaption(IFWL_Widget* pWidget, CFX_WideString& wsCaption);
    virtual FX_FLOAT GetBoxSize(IFWL_Widget* pWidget);
    FX_FLOAT m_fBoxHeight;
    CFX_WideString m_wsCaption;
  };
  CFWL_CheckBoxDP m_checkboxData;
};
#endif
