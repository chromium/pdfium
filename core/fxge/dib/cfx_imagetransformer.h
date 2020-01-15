// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_DIB_CFX_IMAGETRANSFORMER_H_
#define CORE_FXGE_DIB_CFX_IMAGETRANSFORMER_H_

#include <memory>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/dib/cfx_bitmapstorer.h"

class CFX_DIBBase;
class CFX_DIBitmap;
class CFX_ImageStretcher;
class PauseIndicatorIface;

class CFX_ImageTransformer {
 public:
  struct BilinearData {
    int res_x;
    int res_y;
    int src_col_l;
    int src_row_l;
    int src_col_r;
    int src_row_r;
    int row_offset_l;
    int row_offset_r;
  };

  struct BicubicData {
    int res_x;
    int res_y;
    int src_col_l;
    int src_row_l;
    int src_col_r;
    int src_row_r;
    int pos_pixel[8];
    int u_w[4];
    int v_w[4];
  };

  struct DownSampleData {
    int src_col;
    int src_row;
  };

  struct CalcData {
    CFX_DIBitmap* bitmap;
    const CFX_Matrix& matrix;
    const uint8_t* buf;
    uint32_t pitch;
  };

  CFX_ImageTransformer(const RetainPtr<CFX_DIBBase>& pSrc,
                       const CFX_Matrix& matrix,
                       const FXDIB_ResampleOptions& options,
                       const FX_RECT* pClip);
  ~CFX_ImageTransformer();

  bool Continue(PauseIndicatorIface* pPause);

  const FX_RECT& result() const { return m_result; }
  RetainPtr<CFX_DIBitmap> DetachBitmap();

 private:
  enum StretchType {
    kNone,
    kNormal,
    kRotate,
    kOther,
  };

  void ContinueRotate(PauseIndicatorIface* pPause);
  void ContinueOther(PauseIndicatorIface* pPause);

  void CalcMask(const CalcData& cdata);
  void CalcAlpha(const CalcData& cdata);
  void CalcMono(const CalcData& cdata, FXDIB_Format format);
  void CalcColor(const CalcData& cdata, FXDIB_Format format, int Bpp);

  bool IsBilinear() const;
  bool IsBiCubic() const;

  RetainPtr<CFX_DIBBase> const m_pSrc;
  const CFX_Matrix m_matrix;
  FX_RECT m_StretchClip;
  FX_RECT m_result;
  CFX_Matrix m_dest2stretch;
  std::unique_ptr<CFX_ImageStretcher> m_Stretcher;
  CFX_BitmapStorer m_Storer;
  const FXDIB_ResampleOptions m_ResampleOptions;
  StretchType m_type = kNone;
};

#endif  // CORE_FXGE_DIB_CFX_IMAGETRANSFORMER_H_
