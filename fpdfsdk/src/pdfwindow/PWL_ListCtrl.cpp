// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/pdfwindow/PWL_ListCtrl.h"
#include "fpdfsdk/include/pdfwindow/PWL_Wnd.h"

CPWL_ListCtrl::CPWL_ListCtrl()
    : m_rcContent(0, 0, 0, 0),
      m_ptScroll(0, 0),
      m_fItemSpace(0.0f),
      m_fTopSpace(0.0f),
      m_fBottomSpace(0.0f) {}

CPWL_ListCtrl::~CPWL_ListCtrl() {}

void CPWL_ListCtrl::SetScrollPos(const CPDF_Point& point) {
  m_ptScroll = point;

  if (m_ptScroll.x < m_rcContent.left)
    m_ptScroll.x = m_rcContent.left;

  if (m_ptScroll.x > m_rcContent.right)
    m_ptScroll.x = m_rcContent.right;

  if (m_ptScroll.y > m_rcContent.top)
    m_ptScroll.y = m_rcContent.top;

  if (m_ptScroll.y < m_rcContent.bottom)
    m_ptScroll.y = m_rcContent.bottom;
}

CPDF_Point CPWL_ListCtrl::GetScrollPos() const {
  return m_ptScroll;
}

CPDF_Rect CPWL_ListCtrl::GetScrollArea() const {
  return m_rcContent;
}

void CPWL_ListCtrl::ResetFace() {
  ResetAll(FALSE, 0);
}

void CPWL_ListCtrl::ResetContent(int32_t nStart) {
  if (nStart < 0)
    nStart = 0;
  if (nStart >= 0 && nStart < m_aChildren.GetSize())
    ResetAll(TRUE, nStart);
}

FX_FLOAT CPWL_ListCtrl::GetContentsHeight(FX_FLOAT fLimitWidth) {
  FX_FLOAT fRet = m_fTopSpace;

  FX_FLOAT fBorderWidth = (FX_FLOAT)GetBorderWidth();

  if (fLimitWidth > fBorderWidth * 2) {
    for (int32_t i = 0, sz = m_aChildren.GetSize(); i < sz; i++) {
      if (CPWL_Wnd* pChild = m_aChildren.GetAt(i)) {
        FX_FLOAT fLeft = pChild->GetItemLeftMargin();
        FX_FLOAT fRight = pChild->GetItemRightMargin();

        fRet += pChild->GetItemHeight(fLimitWidth - fBorderWidth * 2 - fLeft -
                                      fRight);
        fRet += m_fItemSpace;
      }
    }

    fRet -= m_fItemSpace;
  }

  fRet += m_fBottomSpace;

  return fRet;
}

void CPWL_ListCtrl::ResetAll(FX_BOOL bMove, int32_t nStart) {
  CPDF_Rect rcClient = GetClientRect();

  FX_FLOAT fWidth = rcClient.Width();

  FX_FLOAT fy = 0.0f - m_fTopSpace;

  if (nStart - 1 >= 0 && nStart - 1 < m_aChildren.GetSize())
    if (CPWL_Wnd* pChild = m_aChildren.GetAt(nStart - 1))
      fy = pChild->GetWindowRect().bottom - m_fItemSpace;

  for (int32_t i = nStart, sz = m_aChildren.GetSize(); i < sz; i++) {
    if (CPWL_Wnd* pChild = m_aChildren.GetAt(i)) {
      FX_FLOAT fLeft = pChild->GetItemLeftMargin();
      FX_FLOAT fRight = pChild->GetItemRightMargin();

      pChild->SetChildMatrix(CFX_Matrix(1, 0, 0, 1,
                                        rcClient.left - m_ptScroll.x,
                                        rcClient.top - m_ptScroll.y));

      if (bMove) {
        FX_FLOAT fItemHeight = pChild->GetItemHeight(fWidth - fLeft - fRight);
        pChild->Move(CPDF_Rect(fLeft, fy - fItemHeight, fWidth - fRight, fy),
                     TRUE, FALSE);
        fy -= fItemHeight;
        fy -= m_fItemSpace;
      }
    }
  }

  fy += m_fItemSpace;

  fy -= m_fBottomSpace;

  if (bMove) {
    m_rcContent.left = 0;
    m_rcContent.top = 0;
    m_rcContent.right = fWidth;
    m_rcContent.bottom = fy;
  }
}

void CPWL_ListCtrl::SetItemSpace(FX_FLOAT fSpace) {
  m_fItemSpace = fSpace;
}

void CPWL_ListCtrl::SetTopSpace(FX_FLOAT fSpace) {
  m_fTopSpace = fSpace;
}

void CPWL_ListCtrl::SetBottomSpace(FX_FLOAT fSpace) {
  m_fBottomSpace = fSpace;
}

void CPWL_ListCtrl::RePosChildWnd() {
  ResetFace();
}

void CPWL_ListCtrl::DrawChildAppearance(CFX_RenderDevice* pDevice,
                                        CFX_Matrix* pUser2Device) {
  pDevice->SaveState();
  CPDF_Rect rcClient = GetClientRect();
  CPDF_Rect rcTemp = rcClient;
  pUser2Device->TransformRect(rcTemp);
  FX_RECT rcClip((int32_t)rcTemp.left, (int32_t)rcTemp.bottom,
                 (int32_t)rcTemp.right, (int32_t)rcTemp.top);

  pDevice->SetClip_Rect(&rcClip);

  for (int32_t i = 0, sz = m_aChildren.GetSize(); i < sz; i++) {
    if (CPWL_Wnd* pChild = m_aChildren.GetAt(i)) {
      CPDF_Rect rcChild = pChild->ChildToParent(pChild->GetWindowRect());
      if (!(rcChild.top < rcClient.bottom || rcChild.bottom > rcClient.top)) {
        CFX_Matrix mt = pChild->GetChildMatrix();
        if (mt.IsIdentity()) {
          pChild->DrawAppearance(pDevice, pUser2Device);
        } else {
          mt.Concat(*pUser2Device);
          pChild->DrawAppearance(pDevice, &mt);
        }
      }
    }
  }

  pDevice->RestoreState();
}

int32_t CPWL_ListCtrl::GetItemIndex(CPWL_Wnd* pItem) {
  for (int32_t i = 0, sz = m_aChildren.GetSize(); i < sz; i++) {
    if (pItem == m_aChildren.GetAt(i))
      return i;
  }

  return -1;
}

CPDF_Point CPWL_ListCtrl::InToOut(const CPDF_Point& point) const {
  CPDF_Rect rcClient = GetClientRect();

  return CPDF_Point(point.x + rcClient.left - m_ptScroll.x,
                    point.y + rcClient.top - m_ptScroll.y);
}

CPDF_Point CPWL_ListCtrl::OutToIn(const CPDF_Point& point) const {
  CPDF_Rect rcClient = GetClientRect();

  return CPDF_Point(point.x - rcClient.left + m_ptScroll.x,
                    point.y - rcClient.top + m_ptScroll.y);
}

CPDF_Rect CPWL_ListCtrl::InToOut(const CPDF_Rect& rect) const {
  CPDF_Rect rcClient = GetClientRect();

  return CPDF_Rect(rect.left + rcClient.left - m_ptScroll.x,
                   rect.bottom + rcClient.top - m_ptScroll.y,
                   rect.right + rcClient.left - m_ptScroll.x,
                   rect.top + rcClient.top - m_ptScroll.y);
}

CPDF_Rect CPWL_ListCtrl::OutToIn(const CPDF_Rect& rect) const {
  CPDF_Rect rcClient = GetClientRect();

  return CPDF_Rect(rect.left - rcClient.left + m_ptScroll.x,
                   rect.bottom - rcClient.top + m_ptScroll.y,
                   rect.right - rcClient.left + m_ptScroll.x,
                   rect.top - rcClient.top + m_ptScroll.y);
}
