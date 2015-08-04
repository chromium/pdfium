// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_CARET_LIGHT_H
#define _FWL_CARET_LIGHT_H
class CFWL_Widget;
class CFWL_WidgetProperties;
class CFWL_Caret;
class CFWL_Caret : public CFWL_Widget {
 public:
  static CFWL_Caret* Create();
  FWL_ERR Initialize(const CFWL_WidgetProperties* pProperties = NULL);
  FWL_ERR ShowCaret(FX_BOOL bFlag = TRUE);
  FWL_ERR GetFrequency(FX_DWORD& elapse);
  FWL_ERR SetFrequency(FX_DWORD elapse);
  FWL_ERR SetColor(CFX_Color crFill);

 protected:
  CFWL_Caret();
  virtual ~CFWL_Caret();
};
#endif
