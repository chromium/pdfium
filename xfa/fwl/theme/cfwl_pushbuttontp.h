// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_THEME_CFWL_PUSHBUTTONTP_H_
#define XFA_FWL_THEME_CFWL_PUSHBUTTONTP_H_

#include "xfa/fwl/theme/cfwl_widgettp.h"

class CFWL_PushButtonTP : public CFWL_WidgetTP {
 public:
  CFWL_PushButtonTP();
  virtual ~CFWL_PushButtonTP();
  virtual FX_BOOL IsValidWidget(IFWL_Widget* pWidget);
  virtual uint32_t SetThemeID(IFWL_Widget* pWidget,
                              uint32_t dwThemeID,
                              FX_BOOL bChildren = TRUE);
  virtual FX_BOOL DrawBackground(CFWL_ThemeBackground* pParams);
  virtual void* GetCapacity(CFWL_ThemePart* pThemePart, uint32_t dwCapacity);
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();

 protected:
  struct PBThemeData {
    FX_ARGB clrBorder[5];
    FX_ARGB clrStart[5];
    FX_ARGB clrEnd[5];
    FX_ARGB clrFill[5];
  };

  void SetThemeData(uint32_t dwID);
  void SetTopLineColor(uint32_t* pData);
  void SetLeftLineColor(uint32_t* pData);
  void SetRightLineColor(uint32_t* pData);
  void SetBottomLineColor(uint32_t* pData);
  void SetBackgroudColor(uint32_t* pData);
  void SetCaptionColor(uint32_t* pData);
  void SetCornerColor(uint32_t* pData);
  int32_t GetColorID(uint32_t dwStates);

  struct PBThemeData* m_pThemeData;
};

#endif  // XFA_FWL_THEME_CFWL_PUSHBUTTONTP_H_
