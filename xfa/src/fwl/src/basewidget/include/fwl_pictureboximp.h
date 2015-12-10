// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_PICTUREBOX_IMP_H
#define _FWL_PICTUREBOX_IMP_H
class CFWL_WidgetImp;
class CFWL_WidgetImpProperties;
class CFWL_WidgetImpDelegate;
class IFWL_Widget;
class CFWL_PictureBoxImp;
class CFWL_PictureBoxImpDelegate;
class CFWL_PictureBoxImp : public CFWL_WidgetImp {
 public:
  CFWL_PictureBoxImp(const CFWL_WidgetImpProperties& properties,
                     IFWL_Widget* pOuter);
  ~CFWL_PictureBoxImp();
  virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const;
  virtual FX_DWORD GetClassID() const;
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();
  virtual FWL_ERR GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE);
  virtual FWL_ERR Update();
  virtual FWL_ERR DrawWidget(CFX_Graphics* pGraphics,
                             const CFX_Matrix* pMatrix = NULL);

 protected:
  void DrawBkground(CFX_Graphics* pGraphics,
                    IFWL_ThemeProvider* pTheme,
                    const CFX_Matrix* pMatrix = NULL);
  FX_BOOL VStyle(FX_BOOL dwStyle);
  CFX_RectF m_rtClient;
  CFX_RectF m_rtImage;
  CFX_Matrix m_matrix;
  FX_BOOL m_bTop;
  FX_BOOL m_bVCenter;
  FX_BOOL m_bButton;
  friend class CFWL_PictureBoxImpDelegate;
};
class CFWL_PictureBoxImpDelegate : public CFWL_WidgetImpDelegate {
 public:
  CFWL_PictureBoxImpDelegate(CFWL_PictureBoxImp* pOwner);
  FWL_ERR OnDrawWidget(CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix = NULL) override;

 protected:
  CFWL_PictureBoxImp* m_pOwner;
};
#endif
