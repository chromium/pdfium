// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_FORM_H
#define _FWL_FORM_H
class IFWL_Widget;
class IFWL_Panel;
class IFWL_Form;
#define FWL_CLASS_Form L"FWL_FORM"
#define FWL_CLASSHASH_Form 881567292
#define FWL_CLASS_FormProxy L"FWL_FORMPROXY"
#define FWL_CLASSHASH_FormProxy 881567291
#define FWL_STYLEEXT_FRM_Resize (1L << 0)
#define FWL_STYLEEXT_FRM_NativeBorder (1L << 1)
#define FWL_STYLEEXT_FRM_RoundCorner (2L << 1)
#define FWL_STYLEEXT_FRM_RoundCorner4 (3L << 1)
#define FWL_STYLEEXT_FRM_NoDrawClient (1L << 3)
#define FWL_STYLEEXT_FRM_BorderCornerMask (3L << 1)
#define FWL_STYLEEXT_FRM_Max (3)
#if (_FX_OS_ == _FX_MACOSX_)
#define FWL_UseMacSystemBorder
#endif
#define FWL_WGTCAPACITY_FRM_CYCaption (FWL_WGTCAPACITY_MAX + 1)
#define FWL_WGTCAPACITY_FRM_CYNarrowCaption (FWL_WGTCAPACITY_MAX + 2)
#define FWL_WGTCAPACITY_FRM_BigIcon (FWL_WGTCAPACITY_MAX + 3)
#define FWL_WGTCAPACITY_FRM_SmallIcon (FWL_WGTCAPACITY_MAX + 4)
#define FWL_PART_FRM_Border 1
#define FWL_PART_FRM_Edge 2
#define FWL_PART_FRM_Background 3
#define FWL_PART_FRM_Caption 4
#define FWL_PART_FRM_NarrowCaption 5
#define FWL_PART_FRM_CloseBox 6
#define FWL_PART_FRM_MinimizeBox 7
#define FWL_PART_FRM_MaximizeBox 8
#define FWL_PART_FRM_HeadText 9
#define FWL_PART_FRM_Icon 10
#define FWL_PARTSTATE_FRM_Normal 1
#define FWL_PARTSTATE_FRM_Hover 2
#define FWL_PARTSTATE_FRM_Pressed 3
#define FWL_PARTSTATE_FRM_Inactive 4
#define FWL_PARTSTATE_FRM_Disabled 5

class IFWL_FormDP : public IFWL_DataProvider {
 public:
  virtual CFX_DIBitmap* GetIcon(IFWL_Widget* pWidget, FX_BOOL bBig) = 0;
};
enum FWL_FORMSIZE {
  FWL_FORMSIZE_Manual = 0,
  FWL_FORMSIZE_Width,
  FWL_FORMSIZE_Height,
  FWL_FORMSIZE_All,
};
enum FWL_COMMANDID {
  FWL_COMMANDID_Close = 0,
  FWL_COMMANDID_Ok,
  FWL_COMMANDID_Cancel,
  FWL_COMMANDID_Abort,
  FWL_COMMANDID_Retry,
  FWL_COMMANDID_Ignore,
  FWL_COMMANDID_Yes,
  FWL_COMMANDID_No,
};

class IFWL_Form : public IFWL_Panel {
 public:
  static IFWL_Form* CreateFormProxy(CFWL_WidgetImpProperties& properties,
                                    CFX_WideString* classname,
                                    IFWL_Widget* pOuter);

  FWL_FORMSIZE GetFormSize();
  FWL_ERR SetFormSize(FWL_FORMSIZE eFormSize);
  IFWL_Widget* DoModal();
  IFWL_Widget* DoModal(FX_DWORD& dwCommandID);
  FWL_ERR EndDoModal();
  FWL_ERR SetBorderRegion(CFX_Path* pPath);

 protected:
  IFWL_Form();
};
#endif
