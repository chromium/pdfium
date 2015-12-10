// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_PICTUREBOX_H
#define _FWL_PICTUREBOX_H
class CFWL_WidgetImpProperties;
class IFWL_Widget;
class IFWL_PictureBoxDP;
class IFWL_PictureBox;
#define FWL_CLASS_PictureBox L"FWL_PICTUREBOX"
#define FWL_CLASSHASH_PictureBox 2974721741
#define FWL_STYLEEXT_PTB_Left 0L << 0
#define FWL_STYLEEXT_PTB_Center 1L << 0
#define FWL_STYLEEXT_PTB_Right 2L << 0
#define FWL_STYLEEXT_PTB_Top 0L << 2
#define FWL_STYLEEXT_PTB_Vcenter 1L << 2
#define FWL_STYLEEXT_PTB_Bottom 2L << 2
#define FWL_STYLEEXT_PTB_Normal 0L << 4
#define FWL_STYLEEXT_PTB_AutoSize 1L << 4
#define FWL_STYLEEXT_PTB_StretchImage 2L << 4
#define FWL_STYLEEXT_PTB_StretchHImage 3L << 4
#define FWL_STYLEEXT_PTB_StretchVImage 4L << 4
#define FWL_STYLEEXT_PTB_HAlignMask 3L << 0
#define FWL_STYLEEXT_PTB_VAlignMask 3L << 2
#define FWL_STYLEEXT_PTB_StretchAlignMask 7L << 4
#define FWL_PART_PTB_Border 1
#define FWL_PART_PTB_Edge 2
#define FWL_PART_PTB_Image 3
#define FWL_PARTSTATE_PTB_Normal (0L << 0)
#define FWL_PARTSTATE_PTB_Disabled (1L << 0)
class IFWL_PictureBoxDP : public IFWL_DataProvider {
 public:
  virtual CFX_DIBitmap* GetPicture(IFWL_Widget* pWidget) = 0;
  virtual CFX_DIBitmap* GetErrorPicture(IFWL_Widget* pWidget) = 0;
  virtual CFX_DIBitmap* GetInitialPicture(IFWL_Widget* pWidget) = 0;
  virtual int32_t GetOpacity(IFWL_Widget* pWidget) = 0;
  virtual int32_t GetFlipMode(IFWL_Widget* pWidget) = 0;
  virtual FWL_ERR GetMatrix(IFWL_Widget* pWidget, CFX_Matrix& matrix) = 0;
};

class IFWL_PictureBox : public IFWL_Widget {
 public:
  static IFWL_PictureBox* Create(const CFWL_WidgetImpProperties& properties,
                                 IFWL_Widget* pOuter);

 protected:
  IFWL_PictureBox();
};
#endif
