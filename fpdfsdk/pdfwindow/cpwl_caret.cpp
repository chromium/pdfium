// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pdfwindow/cpwl_caret.h"

#include <sstream>

#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_pathdata.h"
#include "core/fxge/cfx_renderdevice.h"
#include "fpdfsdk/pdfwindow/cpwl_wnd.h"

#define PWL_CARET_FLASHINTERVAL 500

CPWL_Caret::CPWL_Caret() : m_bFlash(false), m_fWidth(0.4f), m_nDelay(0) {}

CPWL_Caret::~CPWL_Caret() {}

CFX_ByteString CPWL_Caret::GetClassName() const {
  return "CPWL_Caret";
}

void CPWL_Caret::DrawThisAppearance(CFX_RenderDevice* pDevice,
                                    CFX_Matrix* pUser2Device) {
  if (!IsVisible() || !m_bFlash)
    return;

  CFX_FloatRect rcRect = GetCaretRect();
  CFX_FloatRect rcClip = GetClipRect();
  CFX_PathData path;

  float fCaretX = rcRect.left + m_fWidth * 0.5f;
  float fCaretTop = rcRect.top;
  float fCaretBottom = rcRect.bottom;
  if (!rcClip.IsEmpty()) {
    rcRect.Intersect(rcClip);
    if (rcRect.IsEmpty())
      return;

    fCaretTop = rcRect.top;
    fCaretBottom = rcRect.bottom;
  }

  path.AppendPoint(CFX_PointF(fCaretX, fCaretBottom), FXPT_TYPE::MoveTo, false);
  path.AppendPoint(CFX_PointF(fCaretX, fCaretTop), FXPT_TYPE::LineTo, false);

  CFX_GraphStateData gsd;
  gsd.m_LineWidth = m_fWidth;
  pDevice->DrawPath(&path, pUser2Device, &gsd, 0, ArgbEncode(255, 0, 0, 0),
                    FXFILL_ALTERNATE);
}

void CPWL_Caret::TimerProc() {
  if (m_nDelay > 0) {
    m_nDelay--;
  } else {
    m_bFlash = !m_bFlash;
    InvalidateRect();
  }
}

CFX_FloatRect CPWL_Caret::GetCaretRect() const {
  return CFX_FloatRect(m_ptFoot.x, m_ptFoot.y, m_ptHead.x + m_fWidth,
                       m_ptHead.y);
}

void CPWL_Caret::SetCaret(bool bVisible,
                          const CFX_PointF& ptHead,
                          const CFX_PointF& ptFoot) {
  if (bVisible) {
    if (IsVisible()) {
      if (m_ptHead != ptHead || m_ptFoot != ptFoot) {
        m_ptHead = ptHead;
        m_ptFoot = ptFoot;
        m_bFlash = true;
        Move(m_rcInvalid, false, true);
      }
    } else {
      m_ptHead = ptHead;
      m_ptFoot = ptFoot;
      EndTimer();
      BeginTimer(PWL_CARET_FLASHINTERVAL);
      CPWL_Wnd::SetVisible(true);
      m_bFlash = true;
      Move(m_rcInvalid, false, true);
    }
  } else {
    m_ptHead = CFX_PointF();
    m_ptFoot = CFX_PointF();
    m_bFlash = false;
    if (IsVisible()) {
      EndTimer();
      CPWL_Wnd::SetVisible(false);
    }
  }
}

void CPWL_Caret::InvalidateRect(CFX_FloatRect* pRect) {
  if (pRect) {
    CFX_FloatRect rcRefresh = *pRect;
    if (!rcRefresh.IsEmpty()) {
      rcRefresh.Inflate(0.5f, 0.5f);
      rcRefresh.Normalize();
    }
    rcRefresh.top += 1;
    rcRefresh.bottom -= 1;
    CPWL_Wnd::InvalidateRect(&rcRefresh);
  } else {
    CPWL_Wnd::InvalidateRect(pRect);
  }
}
