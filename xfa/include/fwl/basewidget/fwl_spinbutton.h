// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_SPINBUTTON_H
#define _FWL_SPINBUTTON_H
class CFWL_WidgetImpProperties;
class IFWL_Widget;
class IFWL_SpinButton;
#define FWL_CLASS_SpinButton L"FWL_SPINBUTTON"
#define FWL_CLASSHASH_SpinButton 3793043646
#define FWL_STYLEEXE_SPB_Vert (1L << 0)
#define FWL_PART_SPB_Border 1
#define FWL_PART_SPB_Edge 2
#define FWL_PART_SPB_UpButton 3
#define FWL_PART_SPB_DownButton 4
#define FWL_PARTSTATE_SPB_Normal (0L << 0)
#define FWL_PARTSTATE_SPB_Hovered (1L << 0)
#define FWL_PARTSTATE_SPB_Pressed (2L << 0)
#define FWL_PARTSTATE_SPB_Disabled (3L << 0)
#define FWL_WGTHITTEST_SPB_UpButton (FWL_WGTHITTEST_MAX + 1)
#define FWL_WGTHITTEST_SPB_DownButton (FWL_WGTHITTEST_MAX + 2)
#define FWL_EVT_SPB_Click L"FWL_EVENT_SPB_Click"
#define FWL_EVTHASH_SPB_Click 2927651187
BEGIN_FWL_EVENT_DEF(CFWL_EvtSpbClick, FWL_EVTHASH_SPB_Click)
FX_BOOL m_bUp;
END_FWL_EVENT_DEF

class IFWL_SpinButton : public IFWL_Widget {
 public:
  static IFWL_SpinButton* Create(const CFWL_WidgetImpProperties& properties,
                                 IFWL_Widget* pOuter);

  FWL_ERR EnableButton(FX_BOOL bEnable, FX_BOOL bUp = TRUE);
  FX_BOOL IsButtonEnable(FX_BOOL bUp = TRUE);

 protected:
  IFWL_SpinButton();
};
#endif
