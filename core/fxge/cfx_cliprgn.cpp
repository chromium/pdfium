// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/cfx_cliprgn.h"

#include <string.h>

#include <utility>

#include "core/fxcrt/span_util.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "third_party/base/check_op.h"
#include "third_party/base/notreached.h"

CFX_ClipRgn::CFX_ClipRgn(int width, int height) : m_Box(0, 0, width, height) {}

CFX_ClipRgn::CFX_ClipRgn(const CFX_ClipRgn& src) = default;

CFX_ClipRgn::~CFX_ClipRgn() = default;

void CFX_ClipRgn::IntersectRect(const FX_RECT& rect) {
  if (m_Type == kRectI) {
    m_Box.Intersect(rect);
    return;
  }
  IntersectMaskRect(rect, m_Box, m_Mask);
}

void CFX_ClipRgn::IntersectMaskRect(FX_RECT rect,
                                    FX_RECT mask_rect,
                                    RetainPtr<CFX_DIBitmap> pOldMask) {
  m_Type = kMaskF;
  m_Box = rect;
  m_Box.Intersect(mask_rect);
  if (m_Box.IsEmpty()) {
    m_Type = kRectI;
    return;
  }
  if (m_Box == mask_rect) {
    m_Mask = std::move(pOldMask);
    return;
  }
  m_Mask = pdfium::MakeRetain<CFX_DIBitmap>();
  m_Mask->Create(m_Box.Width(), m_Box.Height(), FXDIB_Format::k8bppMask);
  const int offset = m_Box.left - mask_rect.left;
  for (int row = m_Box.top; row < m_Box.bottom; row++) {
    pdfium::span<uint8_t> dest_scan =
        m_Mask->GetWritableScanline(row - m_Box.top);
    pdfium::span<const uint8_t> src_scan =
        pOldMask->GetScanline(row - mask_rect.top);
    fxcrt::spancpy(dest_scan, src_scan.subspan(offset, m_Box.Width()));
  }
}

void CFX_ClipRgn::IntersectMaskF(int left,
                                 int top,
                                 RetainPtr<CFX_DIBitmap> pMask) {
  DCHECK_EQ(pMask->GetFormat(), FXDIB_Format::k8bppMask);
  FX_RECT mask_box(left, top, left + pMask->GetWidth(),
                   top + pMask->GetHeight());
  if (m_Type == kRectI) {
    IntersectMaskRect(m_Box, mask_box, std::move(pMask));
    return;
  }

  FX_RECT new_box = m_Box;
  new_box.Intersect(mask_box);
  if (new_box.IsEmpty()) {
    m_Type = kRectI;
    m_Mask = nullptr;
    m_Box = new_box;
    return;
  }
  auto new_dib = pdfium::MakeRetain<CFX_DIBitmap>();
  new_dib->Create(new_box.Width(), new_box.Height(), FXDIB_Format::k8bppMask);
  for (int row = new_box.top; row < new_box.bottom; row++) {
    pdfium::span<const uint8_t> old_scan = m_Mask->GetScanline(row - m_Box.top);
    pdfium::span<const uint8_t> mask_scan = pMask->GetScanline(row - top);
    uint8_t* new_scan = new_dib->GetWritableScanline(row - new_box.top).data();
    for (int col = new_box.left; col < new_box.right; col++) {
      new_scan[col - new_box.left] =
          old_scan[col - m_Box.left] * mask_scan[col - left] / 255;
    }
  }
  m_Box = new_box;
  m_Mask = std::move(new_dib);
}
