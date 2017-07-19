// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PDFWINDOW_CPWL_CARET_H_
#define FPDFSDK_PDFWINDOW_CPWL_CARET_H_

#include "fpdfsdk/pdfwindow/cpwl_wnd.h"

class CPWL_Caret : public CPWL_Wnd {
 public:
  CPWL_Caret();
  ~CPWL_Caret() override;

  // CPWL_Wnd
  CFX_ByteString GetClassName() const override;
  void DrawThisAppearance(CFX_RenderDevice* pDevice,
                          CFX_Matrix* pUser2Device) override;
  void InvalidateRect(CFX_FloatRect* pRect = nullptr) override;
  void SetVisible(bool bVisible) override {}
  void TimerProc() override;

  void SetCaret(bool bVisible,
                const CFX_PointF& ptHead,
                const CFX_PointF& ptFoot);
  void SetInvalidRect(CFX_FloatRect rc) { m_rcInvalid = rc; }

 private:
  CFX_FloatRect GetCaretRect() const;

  bool m_bFlash;
  CFX_PointF m_ptHead;
  CFX_PointF m_ptFoot;
  float m_fWidth;
  int32_t m_nDelay;
  CFX_FloatRect m_rcInvalid;
};

#endif  // FPDFSDK_PDFWINDOW_CPWL_CARET_H_
