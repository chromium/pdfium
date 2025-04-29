// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_image.h"

#include <stdint.h>

#include <algorithm>
#include <array>
#include <memory>
#include <utility>

#include "constants/stream_dict_common.h"
#include "core/fpdfapi/page/cpdf_dib.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_pageimagecache.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_boolean.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fxcodec/jpeg/jpegmodule.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_2d_size.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxcrt/span_util.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/fx_dib.h"

// static
bool CPDF_Image::IsValidJpegComponent(int32_t comps) {
  return comps == 1 || comps == 3 || comps == 4;
}

// static
bool CPDF_Image::IsValidJpegBitsPerComponent(int32_t bpc) {
  return bpc == 1 || bpc == 2 || bpc == 4 || bpc == 8 || bpc == 16;
}

CPDF_Image::CPDF_Image(CPDF_Document* pDoc) : document_(pDoc) {
  DCHECK(document_);
}

CPDF_Image::CPDF_Image(CPDF_Document* pDoc, RetainPtr<CPDF_Stream> pStream)
    : is_inline_(true), document_(pDoc), stream_(std::move(pStream)) {
  DCHECK(document_);
  FinishInitialization();
}

CPDF_Image::CPDF_Image(CPDF_Document* pDoc, uint32_t dwStreamObjNum)
    : document_(pDoc),
      stream_(ToStream(pDoc->GetMutableIndirectObject(dwStreamObjNum))) {
  DCHECK(document_);
  FinishInitialization();
}

CPDF_Image::~CPDF_Image() = default;

void CPDF_Image::FinishInitialization() {
  RetainPtr<const CPDF_Dictionary> pStreamDict = stream_->GetDict();
  oc_ = pStreamDict->GetDictFor("OC");
  is_mask_ = !pStreamDict->KeyExist("ColorSpace") ||
             pStreamDict->GetBooleanFor("ImageMask", /*bDefault=*/false);
  interpolate_ = !!pStreamDict->GetIntegerFor("Interpolate");
  height_ = pStreamDict->GetIntegerFor("Height");
  width_ = pStreamDict->GetIntegerFor("Width");
}

void CPDF_Image::ConvertStreamToIndirectObject() {
  CHECK(stream_->IsInline());
  document_->AddIndirectObject(stream_);
}

RetainPtr<const CPDF_Dictionary> CPDF_Image::GetDict() const {
  return stream_ ? stream_->GetDict() : nullptr;
}

RetainPtr<const CPDF_Stream> CPDF_Image::GetStream() const {
  return stream_;
}

RetainPtr<const CPDF_Dictionary> CPDF_Image::GetOC() const {
  return oc_;
}

RetainPtr<CPDF_Dictionary> CPDF_Image::InitJPEG(
    pdfium::span<uint8_t> src_span) {
  std::optional<JpegModule::ImageInfo> info_opt =
      JpegModule::LoadInfo(src_span);
  if (!info_opt.has_value()) {
    return nullptr;
  }

  const JpegModule::ImageInfo& info = info_opt.value();
  if (!IsValidJpegComponent(info.num_components) ||
      !IsValidJpegBitsPerComponent(info.bits_per_components)) {
    return nullptr;
  }

  RetainPtr<CPDF_Dictionary> pDict =
      CreateXObjectImageDict(info.width, info.height);
  const char* csname = nullptr;
  if (info.num_components == 1) {
    csname = "DeviceGray";
  } else if (info.num_components == 3) {
    csname = "DeviceRGB";
  } else if (info.num_components == 4) {
    csname = "DeviceCMYK";
    auto pDecode = pDict->SetNewFor<CPDF_Array>("Decode");
    for (int n = 0; n < 4; n++) {
      pDecode->AppendNew<CPDF_Number>(1);
      pDecode->AppendNew<CPDF_Number>(0);
    }
  }
  pDict->SetNewFor<CPDF_Name>("ColorSpace", csname);
  pDict->SetNewFor<CPDF_Number>("BitsPerComponent", info.bits_per_components);
  pDict->SetNewFor<CPDF_Name>("Filter", "DCTDecode");
  if (!info.color_transform) {
    auto pParms =
        pDict->SetNewFor<CPDF_Dictionary>(pdfium::stream::kDecodeParms);
    pParms->SetNewFor<CPDF_Number>("ColorTransform", 0);
  }
  is_mask_ = false;
  width_ = info.width;
  height_ = info.height;
  return pDict;
}

void CPDF_Image::SetJpegImage(RetainPtr<IFX_SeekableReadStream> pFile) {
  uint32_t size = pdfium::checked_cast<uint32_t>(pFile->GetSize());
  if (!size) {
    return;
  }

  uint32_t dwEstimateSize = std::min(size, 8192U);
  DataVector<uint8_t> data(dwEstimateSize);
  if (!pFile->ReadBlockAtOffset(data, 0)) {
    return;
  }

  RetainPtr<CPDF_Dictionary> dict = InitJPEG(data);
  if (!dict && size > dwEstimateSize) {
    data.resize(size);
    if (pFile->ReadBlockAtOffset(data, 0)) {
      dict = InitJPEG(data);
    }
  }
  if (!dict) {
    return;
  }

  stream_ = pdfium::MakeRetain<CPDF_Stream>(std::move(pFile), std::move(dict));
}

void CPDF_Image::SetJpegImageInline(RetainPtr<IFX_SeekableReadStream> pFile) {
  uint32_t size = pdfium::checked_cast<uint32_t>(pFile->GetSize());
  if (!size) {
    return;
  }

  DataVector<uint8_t> data(size);
  if (!pFile->ReadBlockAtOffset(data, 0)) {
    return;
  }

  RetainPtr<CPDF_Dictionary> dict = InitJPEG(data);
  if (!dict) {
    return;
  }

  stream_ = pdfium::MakeRetain<CPDF_Stream>(std::move(data), std::move(dict));
}

void CPDF_Image::SetImage(const RetainPtr<CFX_DIBitmap>& pBitmap) {
  int32_t BitmapWidth = pBitmap->GetWidth();
  int32_t BitmapHeight = pBitmap->GetHeight();
  if (BitmapWidth < 1 || BitmapHeight < 1) {
    return;
  }

  RetainPtr<CPDF_Dictionary> pDict =
      CreateXObjectImageDict(BitmapWidth, BitmapHeight);
  const int32_t bpp = pBitmap->GetBPP();
  size_t dest_pitch = 0;
  bool bCopyWithoutAlpha = true;
  if (bpp == 1) {
    FX_BGRA_STRUCT<uint8_t> reset_bgra;
    FX_BGRA_STRUCT<uint8_t> set_bgra;
    if (!pBitmap->IsMaskFormat()) {
      reset_bgra = ArgbToBGRAStruct(pBitmap->GetPaletteArgb(0));
      set_bgra = ArgbToBGRAStruct(pBitmap->GetPaletteArgb(1));
    }
    if (set_bgra.alpha == 0 || reset_bgra.alpha == 0) {
      pDict->SetNewFor<CPDF_Boolean>("ImageMask", true);
      if (reset_bgra.alpha == 0) {
        auto pArray = pDict->SetNewFor<CPDF_Array>("Decode");
        pArray->AppendNew<CPDF_Number>(1);
        pArray->AppendNew<CPDF_Number>(0);
      }
    } else {
      auto pCS = pDict->SetNewFor<CPDF_Array>("ColorSpace");
      pCS->AppendNew<CPDF_Name>("Indexed");
      pCS->AppendNew<CPDF_Name>("DeviceRGB");
      pCS->AppendNew<CPDF_Number>(1);
      const uint8_t ct[6] = {reset_bgra.red, reset_bgra.green, reset_bgra.blue,
                             set_bgra.red,   set_bgra.green,   set_bgra.blue};
      pCS->AppendNew<CPDF_String>(ct, CPDF_String::DataType::kIsHex);
    }
    pDict->SetNewFor<CPDF_Number>("BitsPerComponent", 1);
    dest_pitch = (BitmapWidth + 7) / 8;
  } else if (bpp == 8) {
    size_t palette_size = pBitmap->GetRequiredPaletteSize();
    if (palette_size > 0) {
      DCHECK(palette_size <= 256);
      auto pCS = document_->NewIndirect<CPDF_Array>();
      pCS->AppendNew<CPDF_Name>("Indexed");
      pCS->AppendNew<CPDF_Name>("DeviceRGB");
      pCS->AppendNew<CPDF_Number>(static_cast<int>(palette_size - 1));
      DataVector<uint8_t> color_table(Fx2DSizeOrDie(palette_size, 3));
      auto color_table_span = pdfium::span(color_table);
      for (size_t i = 0; i < palette_size; i++) {
        uint32_t argb = pBitmap->GetPaletteArgb(i);
        color_table_span[0] = FXARGB_R(argb);
        color_table_span[1] = FXARGB_G(argb);
        color_table_span[2] = FXARGB_B(argb);
        color_table_span = color_table_span.subspan<3u>();
      }
      auto pNewDict = document_->New<CPDF_Dictionary>();
      auto pCTS = document_->NewIndirect<CPDF_Stream>(std::move(color_table),
                                                      std::move(pNewDict));
      pCS->AppendNew<CPDF_Reference>(document_, pCTS->GetObjNum());
      pDict->SetNewFor<CPDF_Reference>("ColorSpace", document_,
                                       pCS->GetObjNum());
    } else {
      pDict->SetNewFor<CPDF_Name>("ColorSpace", "DeviceGray");
    }
    pDict->SetNewFor<CPDF_Number>("BitsPerComponent", 8);
    dest_pitch = BitmapWidth;
  } else {
    pDict->SetNewFor<CPDF_Name>("ColorSpace", "DeviceRGB");
    pDict->SetNewFor<CPDF_Number>("BitsPerComponent", 8);
    dest_pitch = BitmapWidth * 3;
    bCopyWithoutAlpha = false;
  }

  RetainPtr<CFX_DIBitmap> pMaskBitmap;
  if (pBitmap->IsAlphaFormat()) {
    pMaskBitmap = pBitmap->CloneAlphaMask();
  }

  if (pMaskBitmap) {
    const int32_t mask_width = pMaskBitmap->GetWidth();
    const int32_t mask_height = pMaskBitmap->GetHeight();
    DataVector<uint8_t> mask_buf;
    RetainPtr<CPDF_Dictionary> pMaskDict =
        CreateXObjectImageDict(mask_width, mask_height);
    pMaskDict->SetNewFor<CPDF_Name>("ColorSpace", "DeviceGray");
    pMaskDict->SetNewFor<CPDF_Number>("BitsPerComponent", 8);
    if (pMaskBitmap->GetFormat() != FXDIB_Format::k1bppMask) {
      mask_buf.resize(Fx2DSizeOrDie(mask_width, mask_height));
      for (int32_t a = 0; a < mask_height; a++) {
        fxcrt::Copy(
            pMaskBitmap->GetScanline(a).first(static_cast<size_t>(mask_width)),
            pdfium::span(mask_buf).subspan(
                static_cast<size_t>(a * mask_width)));
      }
    }
    pMaskDict->SetNewFor<CPDF_Number>(
        "Length", pdfium::checked_cast<int>(mask_buf.size()));
    auto pNewStream = document_->NewIndirect<CPDF_Stream>(std::move(mask_buf),
                                                          std::move(pMaskDict));
    pDict->SetNewFor<CPDF_Reference>("SMask", document_,
                                     pNewStream->GetObjNum());
  }

  DataVector<uint8_t> dest_buf(Fx2DSizeOrDie(dest_pitch, BitmapHeight));
  pdfium::span<uint8_t> dest_span = pdfium::span(dest_buf);
  pdfium::span<const uint8_t> src_span = pBitmap->GetBuffer();
  const int32_t src_pitch = pBitmap->GetPitch();
  if (bCopyWithoutAlpha) {
    for (int32_t i = 0; i < BitmapHeight; i++) {
      dest_span = fxcrt::spancpy(dest_span, src_span.first(dest_pitch));
      src_span = src_span.subspan(static_cast<size_t>(src_pitch));
    }
  } else {
    const size_t src_step = bpp == 24 ? 3 : 4;
    for (int32_t row = 0; row < BitmapHeight; row++) {
      uint8_t* dest_ptr = dest_span.data();
      const uint8_t* src_ptr = src_span.data();
      for (int32_t column = 0; column < BitmapWidth; column++) {
        UNSAFE_TODO({
          dest_ptr[0] = src_ptr[2];
          dest_ptr[1] = src_ptr[1];
          dest_ptr[2] = src_ptr[0];
          dest_ptr += 3;
          src_ptr += src_step;
        });
      }
      dest_span = dest_span.subspan(dest_pitch);
      src_span = src_span.subspan(static_cast<size_t>(src_pitch));
    }
  }

  stream_ =
      pdfium::MakeRetain<CPDF_Stream>(std::move(dest_buf), std::move(pDict));
  is_mask_ = pBitmap->IsMaskFormat();
  width_ = BitmapWidth;
  height_ = BitmapHeight;
}

void CPDF_Image::ResetCache(CPDF_Page* pPage) {
  RetainPtr<CPDF_Image> pHolder(this);
  pPage->GetPageImageCache()->ResetBitmapForImage(std::move(pHolder));
}

void CPDF_Image::WillBeDestroyed() {
  will_be_destroyed_ = true;
}

RetainPtr<CPDF_DIB> CPDF_Image::CreateNewDIB() const {
  return pdfium::MakeRetain<CPDF_DIB>(GetDocument(), GetStream());
}

RetainPtr<CFX_DIBBase> CPDF_Image::LoadDIBBase() const {
  RetainPtr<CPDF_DIB> source = CreateNewDIB();
  if (!source->Load()) {
    return nullptr;
  }

  if (!source->IsJBigImage()) {
    return source;
  }

  CPDF_DIB::LoadState ret = CPDF_DIB::LoadState::kContinue;
  while (ret == CPDF_DIB::LoadState::kContinue) {
    ret = source->ContinueLoadDIBBase(nullptr);
  }
  return ret == CPDF_DIB::LoadState::kSuccess ? source : nullptr;
}

RetainPtr<CFX_DIBBase> CPDF_Image::DetachBitmap() {
  return std::move(dibbase_);
}

RetainPtr<CFX_DIBBase> CPDF_Image::DetachMask() {
  return std::move(mask_);
}

bool CPDF_Image::StartLoadDIBBase(const CPDF_Dictionary* pFormResource,
                                  const CPDF_Dictionary* pPageResource,
                                  bool bStdCS,
                                  CPDF_ColorSpace::Family GroupFamily,
                                  bool bLoadMask,
                                  const CFX_Size& max_size_required) {
  RetainPtr<CPDF_DIB> source = CreateNewDIB();
  CPDF_DIB::LoadState ret =
      source->StartLoadDIBBase(true, pFormResource, pPageResource, bStdCS,
                               GroupFamily, bLoadMask, max_size_required);
  if (ret == CPDF_DIB::LoadState::kFail) {
    dibbase_.Reset();
    return false;
  }
  dibbase_ = source;
  if (ret == CPDF_DIB::LoadState::kContinue) {
    return true;
  }

  mask_ = source->DetachMask();
  matte_color_ = source->GetMatteColor();
  return false;
}

bool CPDF_Image::Continue(PauseIndicatorIface* pPause) {
  RetainPtr<CPDF_DIB> pSource = dibbase_.As<CPDF_DIB>();
  CPDF_DIB::LoadState ret = pSource->ContinueLoadDIBBase(pPause);
  if (ret == CPDF_DIB::LoadState::kContinue) {
    return true;
  }

  if (ret == CPDF_DIB::LoadState::kSuccess) {
    mask_ = pSource->DetachMask();
    matte_color_ = pSource->GetMatteColor();
  } else {
    dibbase_.Reset();
  }
  return false;
}

RetainPtr<CPDF_Dictionary> CPDF_Image::CreateXObjectImageDict(int width,
                                                              int height) {
  auto dict = document_->New<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "XObject");
  dict->SetNewFor<CPDF_Name>("Subtype", "Image");
  dict->SetNewFor<CPDF_Number>("Width", width);
  dict->SetNewFor<CPDF_Number>("Height", height);
  return dict;
}
