// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_DIB_CSTRETCHENGINE_H_
#define CORE_FXGE_DIB_CSTRETCHENGINE_H_

#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/fx_dib.h"

class IFX_PauseIndicator;
class IFX_ScanlineComposer;

class CStretchEngine {
 public:
  CStretchEngine(IFX_ScanlineComposer* pDestBitmap,
                 FXDIB_Format dest_format,
                 int dest_width,
                 int dest_height,
                 const FX_RECT& clip_rect,
                 const RetainPtr<CFX_DIBSource>& pSrcBitmap,
                 int flags);
  ~CStretchEngine();

  bool Continue(IFX_PauseIndicator* pPause);

  bool StartStretchHorz();
  bool ContinueStretchHorz(IFX_PauseIndicator* pPause);
  void StretchVert();

  class CWeightTable {
   public:
    CWeightTable();
    ~CWeightTable();

    bool Calc(int dest_len,
              int dest_min,
              int dest_max,
              int src_len,
              int src_min,
              int src_max,
              int flags);
    PixelWeight* GetPixelWeight(int pixel) const;
    int* GetValueFromPixelWeight(PixelWeight* pWeight, int index) const;
    size_t GetPixelWeightSize() const;

   private:
    int m_DestMin;
    int m_ItemSize;
    std::vector<uint8_t> m_WeightTables;
    size_t m_dwWeightTablesSize;
  };

  FXDIB_Format m_DestFormat;
  int m_DestBpp;
  int m_SrcBpp;
  int m_bHasAlpha;
  UnownedPtr<IFX_ScanlineComposer> m_pDestBitmap;
  int m_DestWidth;
  int m_DestHeight;
  FX_RECT m_DestClip;
  std::vector<uint8_t> m_DestScanline;
  std::vector<uint8_t> m_DestMaskScanline;
  FX_RECT m_SrcClip;
  RetainPtr<CFX_DIBSource> m_pSource;
  uint32_t* m_pSrcPalette;
  int m_SrcWidth;
  int m_SrcHeight;
  int m_SrcPitch;
  int m_InterPitch;
  int m_ExtraMaskPitch;
  std::vector<uint8_t> m_InterBuf;
  std::vector<uint8_t> m_ExtraAlphaBuf;
  int m_TransMethod;
  int m_Flags;
  CWeightTable m_WeightTable;
  int m_CurRow;
  int m_State;
};

#endif  // CORE_FXGE_DIB_CSTRETCHENGINE_H_
