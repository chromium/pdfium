// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/cba_annotiterator.h"

#include "core/fpdfapi/fpdf_page/include/cpdf_page.h"
#include "fpdfsdk/include/cpdfsdk_annot.h"
#include "fpdfsdk/include/cpdfsdk_pageview.h"

// static
bool CBA_AnnotIterator::CompareByLeftAscending(const CPDFSDK_Annot* p1,
                                               const CPDFSDK_Annot* p2) {
  return GetAnnotRect(p1).left < GetAnnotRect(p2).left;
}

// static
bool CBA_AnnotIterator::CompareByTopDescending(const CPDFSDK_Annot* p1,
                                               const CPDFSDK_Annot* p2) {
  return GetAnnotRect(p1).top > GetAnnotRect(p2).top;
}

CBA_AnnotIterator::CBA_AnnotIterator(CPDFSDK_PageView* pPageView,
                                     CPDF_Annot::Subtype nAnnotSubtype)
    : m_eTabOrder(STRUCTURE),
      m_pPageView(pPageView),
      m_nAnnotSubtype(nAnnotSubtype) {
  CPDF_Page* pPDFPage = m_pPageView->GetPDFPage();
  CFX_ByteString sTabs = pPDFPage->m_pFormDict->GetStringFor("Tabs");
  if (sTabs == "R")
    m_eTabOrder = ROW;
  else if (sTabs == "C")
    m_eTabOrder = COLUMN;

  GenerateResults();
}

CBA_AnnotIterator::~CBA_AnnotIterator() {}

CPDFSDK_Annot* CBA_AnnotIterator::GetFirstAnnot() {
  return m_Annots.empty() ? nullptr : m_Annots.front();
}

CPDFSDK_Annot* CBA_AnnotIterator::GetLastAnnot() {
  return m_Annots.empty() ? nullptr : m_Annots.back();
}

CPDFSDK_Annot* CBA_AnnotIterator::GetNextAnnot(CPDFSDK_Annot* pAnnot) {
  auto iter = std::find(m_Annots.begin(), m_Annots.end(), pAnnot);
  if (iter == m_Annots.end())
    return nullptr;
  ++iter;
  if (iter == m_Annots.end())
    iter = m_Annots.begin();
  return *iter;
}

CPDFSDK_Annot* CBA_AnnotIterator::GetPrevAnnot(CPDFSDK_Annot* pAnnot) {
  auto iter = std::find(m_Annots.begin(), m_Annots.end(), pAnnot);
  if (iter == m_Annots.end())
    return nullptr;
  if (iter == m_Annots.begin())
    iter = m_Annots.end();
  return *(--iter);
}

void CBA_AnnotIterator::GenerateResults() {
  switch (m_eTabOrder) {
    case STRUCTURE: {
      for (size_t i = 0; i < m_pPageView->CountAnnots(); ++i) {
        CPDFSDK_Annot* pAnnot = m_pPageView->GetAnnot(i);
        if (pAnnot->GetAnnotSubtype() == m_nAnnotSubtype &&
            !pAnnot->IsSignatureWidget())
          m_Annots.push_back(pAnnot);
      }
      break;
    }
    case ROW: {
      std::vector<CPDFSDK_Annot*> sa;
      for (size_t i = 0; i < m_pPageView->CountAnnots(); ++i) {
        CPDFSDK_Annot* pAnnot = m_pPageView->GetAnnot(i);
        if (pAnnot->GetAnnotSubtype() == m_nAnnotSubtype &&
            !pAnnot->IsSignatureWidget())
          sa.push_back(pAnnot);
      }

      std::sort(sa.begin(), sa.end(), CompareByLeftAscending);
      while (!sa.empty()) {
        int nLeftTopIndex = -1;
        FX_FLOAT fTop = 0.0f;
        for (int i = sa.size() - 1; i >= 0; i--) {
          CFX_FloatRect rcAnnot = GetAnnotRect(sa[i]);
          if (rcAnnot.top > fTop) {
            nLeftTopIndex = i;
            fTop = rcAnnot.top;
          }
        }
        if (nLeftTopIndex >= 0) {
          CPDFSDK_Annot* pLeftTopAnnot = sa[nLeftTopIndex];
          CFX_FloatRect rcLeftTop = GetAnnotRect(pLeftTopAnnot);
          m_Annots.push_back(pLeftTopAnnot);
          sa.erase(sa.begin() + nLeftTopIndex);

          std::vector<int> aSelect;
          for (size_t i = 0; i < sa.size(); ++i) {
            CFX_FloatRect rcAnnot = GetAnnotRect(sa[i]);
            FX_FLOAT fCenterY = (rcAnnot.top + rcAnnot.bottom) / 2.0f;
            if (fCenterY > rcLeftTop.bottom && fCenterY < rcLeftTop.top)
              aSelect.push_back(i);
          }
          for (size_t i = 0; i < aSelect.size(); ++i)
            m_Annots.push_back(sa[aSelect[i]]);

          for (int i = aSelect.size() - 1; i >= 0; --i)
            sa.erase(sa.begin() + aSelect[i]);
        }
      }
      break;
    }
    case COLUMN: {
      std::vector<CPDFSDK_Annot*> sa;
      for (size_t i = 0; i < m_pPageView->CountAnnots(); ++i) {
        CPDFSDK_Annot* pAnnot = m_pPageView->GetAnnot(i);
        if (pAnnot->GetAnnotSubtype() == m_nAnnotSubtype &&
            !pAnnot->IsSignatureWidget())
          sa.push_back(pAnnot);
      }

      std::sort(sa.begin(), sa.end(), CompareByTopDescending);
      while (!sa.empty()) {
        int nLeftTopIndex = -1;
        FX_FLOAT fLeft = -1.0f;
        for (int i = sa.size() - 1; i >= 0; --i) {
          CFX_FloatRect rcAnnot = GetAnnotRect(sa[i]);
          if (fLeft < 0) {
            nLeftTopIndex = 0;
            fLeft = rcAnnot.left;
          } else if (rcAnnot.left < fLeft) {
            nLeftTopIndex = i;
            fLeft = rcAnnot.left;
          }
        }

        if (nLeftTopIndex >= 0) {
          CPDFSDK_Annot* pLeftTopAnnot = sa[nLeftTopIndex];
          CFX_FloatRect rcLeftTop = GetAnnotRect(pLeftTopAnnot);
          m_Annots.push_back(pLeftTopAnnot);
          sa.erase(sa.begin() + nLeftTopIndex);

          std::vector<int> aSelect;
          for (size_t i = 0; i < sa.size(); ++i) {
            CFX_FloatRect rcAnnot = GetAnnotRect(sa[i]);
            FX_FLOAT fCenterX = (rcAnnot.left + rcAnnot.right) / 2.0f;
            if (fCenterX > rcLeftTop.left && fCenterX < rcLeftTop.right)
              aSelect.push_back(i);
          }
          for (size_t i = 0; i < aSelect.size(); ++i)
            m_Annots.push_back(sa[aSelect[i]]);

          for (int i = aSelect.size() - 1; i >= 0; --i)
            sa.erase(sa.begin() + aSelect[i]);
        }
      }
      break;
    }
  }
}

CFX_FloatRect CBA_AnnotIterator::GetAnnotRect(const CPDFSDK_Annot* pAnnot) {
  return pAnnot->GetPDFAnnot()->GetRect();
}
