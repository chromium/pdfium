// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_PUSHBUTTONTP_H
#define _FWL_PUSHBUTTONTP_H
class CFWL_WidgetTP;
class CFWL_PushButtonTP;
class CFWL_PushButtonTP : public CFWL_WidgetTP {
 public:
  CFWL_PushButtonTP();
  virtual ~CFWL_PushButtonTP();
  virtual FX_BOOL IsValidWidget(IFWL_Widget* pWidget);
  virtual FX_DWORD SetThemeID(IFWL_Widget* pWidget,
                              FX_DWORD dwThemeID,
                              FX_BOOL bChildren = TRUE);
  virtual FX_BOOL DrawBackground(CFWL_ThemeBackground* pParams);
  virtual void* GetCapacity(CFWL_ThemePart* pThemePart, FX_DWORD dwCapacity);
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();

 protected:
  void SetThemeData(FX_DWORD dwID);
  void SetTopLineColor(FX_DWORD* pData);
  void SetLeftLineColor(FX_DWORD* pData);
  void SetRightLineColor(FX_DWORD* pData);
  void SetBottomLineColor(FX_DWORD* pData);
  void SetBackgroudColor(FX_DWORD* pData);
  void SetCaptionColor(FX_DWORD* pData);
  void SetCornerColor(FX_DWORD* pData);
  int32_t GetColorID(FX_DWORD dwStates);

  struct PBThemeData {
    FX_ARGB clrBorder[5];
    FX_ARGB clrStart[5];
    FX_ARGB clrEnd[5];
    FX_ARGB clrFill[5];
  } * m_pThemeData;
};
#endif
