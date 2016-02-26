// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/pdfwindow/PWL_Caret.h"
#include "fpdfsdk/include/pdfwindow/PWL_Utils.h"
#include "fpdfsdk/include/pdfwindow/PWL_Wnd.h"

#define PWL_CARET_FLASHINTERVAL 500

CPWL_Caret::CPWL_Caret()
    : m_bFlash(FALSE),
      m_ptHead(0, 0),
      m_ptFoot(0, 0),
      m_fWidth(0.4f),
      m_nDelay(0) {}

CPWL_Caret::~CPWL_Caret() {}

CFX_ByteString CPWL_Caret::GetClassName() const {
  return "CPWL_Caret";
}

void CPWL_Caret::GetThisAppearanceStream(CFX_ByteTextBuf& sAppStream) {
  GetCaretApp(sAppStream, CPDF_Point(0.0f, 0.0f));
}

void CPWL_Caret::DrawThisAppearance(CFX_RenderDevice* pDevice,
                                    CFX_Matrix* pUser2Device) {
  if (IsVisible() && m_bFlash) {
    CPDF_Rect rcRect = GetCaretRect();
    CPDF_Rect rcClip = GetClipRect();

    CFX_PathData path;

    path.SetPointCount(2);

    FX_FLOAT fCaretX = rcRect.left + m_fWidth * 0.5f;
    FX_FLOAT fCaretTop = rcRect.top;
    FX_FLOAT fCaretBottom = rcRect.bottom;

    if (!rcClip.IsEmpty()) {
      rcRect.Intersect(rcClip);
      if (!rcRect.IsEmpty()) {
        fCaretTop = rcRect.top;
        fCaretBottom = rcRect.bottom;
        path.SetPoint(0, fCaretX, fCaretBottom, FXPT_MOVETO);
        path.SetPoint(1, fCaretX, fCaretTop, FXPT_LINETO);
      } else {
        return;
      }
    } else {
      path.SetPoint(0, fCaretX, fCaretBottom, FXPT_MOVETO);
      path.SetPoint(1, fCaretX, fCaretTop, FXPT_LINETO);
    }

    CFX_GraphStateData gsd;
    gsd.m_LineWidth = m_fWidth;

    pDevice->DrawPath(&path, pUser2Device, &gsd, 0, ArgbEncode(255, 0, 0, 0),
                      FXFILL_ALTERNATE);
  }
}

void CPWL_Caret::GetCaretApp(CFX_ByteTextBuf& sAppStream,
                             const CPDF_Point& ptOffset) {
  if (IsVisible() && m_bFlash) {
    CFX_ByteTextBuf sCaret;

    CPDF_Rect rcRect = GetCaretRect();
    CPDF_Rect rcClip = GetClipRect();

    rcRect = CPWL_Utils::OffsetRect(rcRect, ptOffset.x, ptOffset.y);
    rcClip = CPWL_Utils::OffsetRect(rcClip, ptOffset.x, ptOffset.y);

    sCaret << "q\n";
    if (!rcClip.IsEmpty()) {
      sCaret << rcClip.left << " " << rcClip.bottom + 2.5f << " "
             << rcClip.right - rcClip.left << " "
             << rcClip.top - rcClip.bottom - 4.5f << " re W n\n";
    }
    sCaret << m_fWidth << " w\n0 G\n";
    sCaret << rcRect.left + m_fWidth / 2 << " " << rcRect.bottom << " m\n";
    sCaret << rcRect.left + m_fWidth / 2 << " " << rcRect.top << " l S\nQ\n";

    sAppStream << sCaret;
  }
}

CFX_ByteString CPWL_Caret::GetCaretAppearanceStream(
    const CPDF_Point& ptOffset) {
  CFX_ByteTextBuf sCaret;
  GetCaretApp(sCaret, ptOffset);
  return sCaret.GetByteString();
}

void CPWL_Caret::TimerProc() {
  if (m_nDelay > 0) {
    m_nDelay--;
  } else {
    m_bFlash = !m_bFlash;
    InvalidateRect();
  }
}

CPDF_Rect CPWL_Caret::GetCaretRect() const {
  return CPDF_Rect(m_ptFoot.x, m_ptFoot.y, m_ptHead.x + m_fWidth, m_ptHead.y);
}

void CPWL_Caret::SetCaret(FX_BOOL bVisible,
                          const CPDF_Point& ptHead,
                          const CPDF_Point& ptFoot) {
  if (bVisible) {
    if (IsVisible()) {
      if (m_ptHead.x != ptHead.x || m_ptHead.y != ptHead.y ||
          m_ptFoot.x != ptFoot.x || m_ptFoot.y != ptFoot.y) {
        m_ptHead = ptHead;
        m_ptFoot = ptFoot;

        m_bFlash = TRUE;
        Move(m_rcInvalid, FALSE, TRUE);
      }
    } else {
      m_ptHead = ptHead;
      m_ptFoot = ptFoot;

      EndTimer();
      BeginTimer(PWL_CARET_FLASHINTERVAL);

      CPWL_Wnd::SetVisible(TRUE);
      m_bFlash = TRUE;

      Move(m_rcInvalid, FALSE, TRUE);
    }
  } else {
    m_ptHead = CPDF_Point(0, 0);
    m_ptFoot = CPDF_Point(0, 0);

    m_bFlash = FALSE;
    if (IsVisible()) {
      EndTimer();
      CPWL_Wnd::SetVisible(FALSE);
    }
  }
}

void CPWL_Caret::InvalidateRect(CPDF_Rect* pRect) {
  if (pRect) {
    CPDF_Rect rcRefresh = CPWL_Utils::InflateRect(*pRect, 0.5f);
    rcRefresh.top += 1;
    rcRefresh.bottom -= 1;

    CPWL_Wnd::InvalidateRect(&rcRefresh);
  } else {
    CPWL_Wnd::InvalidateRect(pRect);
  }
}
