// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_PICTUREBOX_H_
#define XFA_FWL_CORE_IFWL_PICTUREBOX_H_

#include "xfa/fwl/core/cfwl_widgetimpproperties.h"
#include "xfa/fwl/core/fwl_error.h"
#include "xfa/fwl/core/ifwl_dataprovider.h"
#include "xfa/fwl/core/ifwl_widget.h"

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

class CFWL_PictureBoxImpDelegate;
class CFWL_WidgetImpProperties;
class CFX_DIBitmap;
class IFWL_Widget;

class IFWL_PictureBoxDP : public IFWL_DataProvider {
 public:
  virtual CFX_DIBitmap* GetPicture(IFWL_Widget* pWidget) = 0;
  virtual CFX_DIBitmap* GetErrorPicture(IFWL_Widget* pWidget) = 0;
  virtual CFX_DIBitmap* GetInitialPicture(IFWL_Widget* pWidget) = 0;
  virtual int32_t GetOpacity(IFWL_Widget* pWidget) = 0;
  virtual int32_t GetFlipMode(IFWL_Widget* pWidget) = 0;
  virtual FWL_Error GetMatrix(IFWL_Widget* pWidget, CFX_Matrix& matrix) = 0;
};

class IFWL_PictureBox : public IFWL_Widget {
 public:
  explicit IFWL_PictureBox(const CFWL_WidgetImpProperties& properties);
  ~IFWL_PictureBox() override;

  // IFWL_Widget
  FWL_Type GetClassID() const override;
  FWL_Error Initialize() override;
  void Finalize() override;
  FWL_Error GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE) override;
  FWL_Error Update() override;
  FWL_Error DrawWidget(CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix = nullptr) override;

 protected:
  friend class CFWL_PictureBoxImpDelegate;

  void DrawBkground(CFX_Graphics* pGraphics,
                    IFWL_ThemeProvider* pTheme,
                    const CFX_Matrix* pMatrix = nullptr);
  bool VStyle(uint32_t dwStyle);

  CFX_RectF m_rtClient;
  CFX_RectF m_rtImage;
  CFX_Matrix m_matrix;
  FX_BOOL m_bTop;
  FX_BOOL m_bVCenter;
  FX_BOOL m_bButton;
};

class CFWL_PictureBoxImpDelegate : public CFWL_WidgetImpDelegate {
 public:
  CFWL_PictureBoxImpDelegate(IFWL_PictureBox* pOwner);
  void OnDrawWidget(CFX_Graphics* pGraphics,
                    const CFX_Matrix* pMatrix = nullptr) override;

 protected:
  IFWL_PictureBox* m_pOwner;
};

#endif  // XFA_FWL_CORE_IFWL_PICTUREBOX_H_
