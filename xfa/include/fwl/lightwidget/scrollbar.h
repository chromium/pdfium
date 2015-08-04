// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_SCROLLBAR_LIGHT_H
#define _FWL_SCROLLBAR_LIGHT_H
class CFWL_Widget;
class CFWL_WidgetProperties;
class CFWL_ScrollBar;
class CFWL_ScrollBar : public CFWL_Widget {
 public:
  static CFWL_ScrollBar* Create();
  FWL_ERR Initialize(const CFWL_WidgetProperties* pProperties = NULL);
  FX_BOOL IsVertical();
  FWL_ERR GetRange(FX_FLOAT& fMin, FX_FLOAT& fMax);
  FWL_ERR SetRange(FX_FLOAT fMin, FX_FLOAT fMax);
  FX_FLOAT GetPageSize();
  FWL_ERR SetPageSize(FX_FLOAT fPageSize);
  FX_FLOAT GetStepSize();
  FWL_ERR SetStepSize(FX_FLOAT fStepSize);
  FX_FLOAT GetPos();
  FWL_ERR SetPos(FX_FLOAT fPos);
  FX_FLOAT GetTrackPos();
  FWL_ERR SetTrackPos(FX_FLOAT fTrackPos);
  FX_BOOL DoScroll(FX_DWORD dwCode, FX_FLOAT fPos = 0.0f);
  CFWL_ScrollBar();
  virtual ~CFWL_ScrollBar();
};
#endif
