// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_DIB_H_
#define CORE_FPDFAPI_PAGE_CPDF_DIB_H_

#include <stdint.h>

#include <memory>
#include <vector>

#include "core/fpdfapi/page/cpdf_colorspace.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/dib/cfx_dibbase.h"

class CPDF_Dictionary;
class CPDF_Document;
class CPDF_Stream;
class CPDF_StreamAcc;

struct DIB_COMP_DATA {
  float decode_min_;
  float decode_step_;
  int color_key_min_;
  int color_key_max_;
};

namespace fxcodec {
class Jbig2Context;
class ScanlineDecoder;
}  // namespace fxcodec

constexpr size_t kHugeImageSize = 60000000;

class CPDF_DIB final : public CFX_DIBBase {
 public:
  enum class LoadState : uint8_t { kFail, kSuccess, kContinue };

  CONSTRUCT_VIA_MAKE_RETAIN;

  // CFX_DIBBase:
  pdfium::span<const uint8_t> GetScanline(int line) const override;
  bool SkipToScanline(int line, PauseIndicatorIface* pPause) const override;
  size_t GetEstimatedImageMemoryBurden() const override;

  RetainPtr<CPDF_ColorSpace> GetColorSpace() const { return color_space_; }
  uint32_t GetMatteColor() const { return matte_color_; }
  bool IsJBigImage() const;

  bool Load();
  LoadState StartLoadDIBBase(bool bHasMask,
                             const CPDF_Dictionary* pFormResources,
                             const CPDF_Dictionary* pPageResources,
                             bool bStdCS,
                             CPDF_ColorSpace::Family GroupFamily,
                             bool bLoadMask,
                             const CFX_Size& max_size_required);
  LoadState ContinueLoadDIBBase(PauseIndicatorIface* pPause);
  RetainPtr<CPDF_DIB> DetachMask();

 private:
  CPDF_DIB(CPDF_Document* pDoc, RetainPtr<const CPDF_Stream> pStream);
  ~CPDF_DIB() override;

  struct JpxSMaskInlineData {
    JpxSMaskInlineData();
    ~JpxSMaskInlineData();

    int width = 0;
    int height = 0;
    DataVector<uint8_t> data;
  };

  bool LoadInternal(const CPDF_Dictionary* pFormResources,
                    const CPDF_Dictionary* pPageResources);
  bool ContinueInternal();
  LoadState StartLoadMask();
  LoadState StartLoadMaskDIB(RetainPtr<const CPDF_Stream> mask_stream);
  bool ContinueToLoadMask();
  LoadState ContinueLoadMaskDIB(PauseIndicatorIface* pPause);
  bool LoadColorInfo(const CPDF_Dictionary* pFormResources,
                     const CPDF_Dictionary* pPageResources);
  bool GetDecodeAndMaskArray();
  RetainPtr<CFX_DIBitmap> LoadJpxBitmap(uint8_t resolution_levels_to_skip);
  RetainPtr<CFX_DIBitmap> ConvertArgbJpxBitmapToRgb(
      RetainPtr<CFX_DIBitmap> argb_bitmap,
      uint32_t width,
      uint32_t height);
  void LoadPalette();
  LoadState CreateDecoder(uint8_t resolution_levels_to_skip);
  bool CreateDCTDecoder(pdfium::span<const uint8_t> src_span,
                        const CPDF_Dictionary* pParams);
  void TranslateScanline24bpp(pdfium::span<uint8_t> dest_scan,
                              pdfium::span<const uint8_t> src_scan) const;
  bool TranslateScanline24bppDefaultDecode(
      pdfium::span<uint8_t> dest_scan,
      pdfium::span<const uint8_t> src_scan) const;
  bool ValidateDictParam(const ByteString& filter);
  bool TransMask() const;
  void SetMaskProperties();

  uint32_t Get1BitSetValue() const;
  uint32_t Get1BitResetValue() const;

  UnownedPtr<CPDF_Document> const document_;
  RetainPtr<const CPDF_Stream> const stream_;
  RetainPtr<const CPDF_Dictionary> dict_;
  RetainPtr<CPDF_StreamAcc> stream_acc_;
  RetainPtr<CPDF_ColorSpace> color_space_;
  uint32_t bpc_ = 0;
  uint32_t bpc_orig_ = 0;
  uint32_t components_ = 0;
  CPDF_ColorSpace::Family family_ = CPDF_ColorSpace::Family::kUnknown;
  CPDF_ColorSpace::Family group_family_ = CPDF_ColorSpace::Family::kUnknown;
  uint32_t matte_color_ = 0;
  LoadState status_ = LoadState::kFail;
  bool load_mask_ = false;
  bool default_decode_ = true;
  bool image_mask_ = false;
  bool do_bpc_check_ = true;
  bool color_key_ = false;
  bool has_mask_ = false;
  bool std_cs_ = false;
  std::vector<DIB_COMP_DATA> comp_data_;
  mutable DataVector<uint8_t> line_buf_;
  mutable DataVector<uint8_t> mask_buf_;
  RetainPtr<CFX_DIBitmap> cached_bitmap_;
  // Note: Must not create a cycle between CPDF_DIB instances.
  RetainPtr<CPDF_DIB> mask_;
  RetainPtr<CPDF_StreamAcc> global_acc_;
  std::unique_ptr<fxcodec::ScanlineDecoder> decoder_;
  JpxSMaskInlineData jpx_inline_data_;

  // Must come after |cached_bitmap_|.
  std::unique_ptr<fxcodec::Jbig2Context> jbig_2context_;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_DIB_H_
