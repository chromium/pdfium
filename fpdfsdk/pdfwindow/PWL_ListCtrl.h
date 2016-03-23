// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PDFWINDOW_PWL_LISTCTRL_H_
#define FPDFSDK_PDFWINDOW_PWL_LISTCTRL_H_

#include "fpdfsdk/pdfwindow/PWL_Wnd.h"

class CPWL_ListCtrl : public CPWL_Wnd {
 public:
  CPWL_ListCtrl();
  ~CPWL_ListCtrl() override;

  void SetScrollPos(const CFX_FloatPoint& point);
  CFX_FloatPoint GetScrollPos() const;
  CFX_FloatRect GetScrollArea() const;
  void SetItemSpace(FX_FLOAT fSpace);
  void SetTopSpace(FX_FLOAT fSpace);
  void SetBottomSpace(FX_FLOAT fSpace);
  void ResetFace();
  void ResetContent(int32_t nStart);
  int32_t GetItemIndex(CPWL_Wnd* pItem);
  FX_FLOAT GetContentsHeight(FX_FLOAT fLimitWidth);
  CFX_FloatPoint InToOut(const CFX_FloatPoint& point) const;
  CFX_FloatPoint OutToIn(const CFX_FloatPoint& point) const;
  CFX_FloatRect InToOut(const CFX_FloatRect& rect) const;
  CFX_FloatRect OutToIn(const CFX_FloatRect& rect) const;

 protected:
  // CPWL_Wnd
  void RePosChildWnd() override;
  void DrawChildAppearance(CFX_RenderDevice* pDevice,
                           CFX_Matrix* pUser2Device) override;

 private:
  void ResetAll(FX_BOOL bMove, int32_t nStart);

  CFX_FloatRect m_rcContent;
  CFX_FloatPoint m_ptScroll;
  FX_FLOAT m_fItemSpace;
  FX_FLOAT m_fTopSpace;
  FX_FLOAT m_fBottomSpace;
};

#endif  // FPDFSDK_PDFWINDOW_PWL_LISTCTRL_H_
