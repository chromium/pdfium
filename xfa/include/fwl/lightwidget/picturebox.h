// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_PICTUREBOX_LIGHT_H
#define _FWL_PICTUREBOX_LIGHT_H
class CFWL_Widget;
class CFWL_WidgetProperties;
class IFWL_PictureBoxDP;
class CFWL_PictureBox;
class CFWL_PictureBoxDP;
class CFWL_PictureBox : public CFWL_Widget {
 public:
  static CFWL_PictureBox* Create();
  FWL_ERR Initialize(const CFWL_WidgetProperties* pProperties = NULL);
  CFX_DIBitmap* GetPicture();
  FWL_ERR SetPicture(CFX_DIBitmap* pBitmap);
  FX_FLOAT GetRotation();
  FWL_ERR SetRotation(FX_FLOAT fRotation);
  int32_t GetFlipMode();
  FWL_ERR SetFlipMode(int32_t iFlipMode);
  int32_t GetOpacity();
  FWL_ERR SetOpacity(int32_t iOpacity);
  FWL_ERR GetScale(FX_FLOAT& fScaleX, FX_FLOAT& fScaleY);
  FWL_ERR SetScale(FX_FLOAT fScaleX, FX_FLOAT fScaleY);
  FWL_ERR GetOffset(FX_FLOAT& fx, FX_FLOAT& fy);
  FWL_ERR SetOffset(FX_FLOAT fx, FX_FLOAT fy);
  CFWL_PictureBox();
  virtual ~CFWL_PictureBox();

 protected:
  class CFWL_PictureBoxDP : public IFWL_PictureBoxDP {
   public:
    CFWL_PictureBoxDP() {
      m_fRotation = 0.0f;
      m_fScaleX = 1.0f;
      m_fScaleY = 1.0f;
      m_fOffSetX = 0.0f;
      m_fOffSetY = 0.0f;
      m_pBitmap = NULL;
    };
    virtual FWL_ERR GetCaption(IFWL_Widget* pWidget, CFX_WideString& wsCaption);
    virtual CFX_DIBitmap* GetPicture(IFWL_Widget* pWidget);
    virtual CFX_DIBitmap* GetErrorPicture(IFWL_Widget* pWidget);
    virtual CFX_DIBitmap* GetInitialPicture(IFWL_Widget* pWidget);
    virtual int32_t GetOpacity(IFWL_Widget* pWidget);
    virtual int32_t GetFlipMode(IFWL_Widget* pWidget);
    virtual FWL_ERR GetMatrix(IFWL_Widget* pWidget, CFX_Matrix& matrix);
    CFX_DIBitmap* m_pBitmap;
    int32_t m_iOpacity;
    int32_t m_iFlipMode;
    FX_FLOAT m_fRotation;
    FX_FLOAT m_fScaleX;
    FX_FLOAT m_fScaleY;
    FX_FLOAT m_fOffSetX;
    FX_FLOAT m_fOffSetY;
    CFX_WideString m_wsData;
  };
  CFWL_PictureBoxDP m_PictureBoxDP;
};
#endif
