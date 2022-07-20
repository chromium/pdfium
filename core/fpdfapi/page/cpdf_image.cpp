// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_image.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "constants/stream_dict_common.h"
#include "core/fpdfapi/page/cpdf_dib.h"
#include "core/fpdfapi/page/cpdf_page.h"
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
#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/span_util.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/fx_dib.h"
#include "third_party/base/check.h"
#include "third_party/base/numerics/safe_conversions.h"

// static
bool CPDF_Image::IsValidJpegComponent(int32_t comps) {
  return comps == 1 || comps == 3 || comps == 4;
}

// static
bool CPDF_Image::IsValidJpegBitsPerComponent(int32_t bpc) {
  return bpc == 1 || bpc == 2 || bpc == 4 || bpc == 8 || bpc == 16;
}

CPDF_Image::CPDF_Image(CPDF_Document* pDoc) : m_pDocument(pDoc) {
  DCHECK(m_pDocument);
}

CPDF_Image::CPDF_Image(CPDF_Document* pDoc, RetainPtr<CPDF_Stream> pStream)
    : m_bIsInline(true), m_pDocument(pDoc), m_pStream(std::move(pStream)) {
  DCHECK(m_pDocument);
  FinishInitialization();
}

CPDF_Image::CPDF_Image(CPDF_Document* pDoc, uint32_t dwStreamObjNum)
    : m_pDocument(pDoc),
      m_pStream(ToStream(pDoc->GetIndirectObject(dwStreamObjNum))) {
  DCHECK(m_pDocument);
  FinishInitialization();
}

CPDF_Image::~CPDF_Image() = default;

void CPDF_Image::FinishInitialization() {
  RetainPtr<CPDF_Dictionary> pStreamDict = m_pStream->GetMutableDict();
  m_pOC = pStreamDict->GetMutableDictFor("OC");
  m_bIsMask = !pStreamDict->KeyExist("ColorSpace") ||
              pStreamDict->GetBooleanFor("ImageMask", /*bDefault=*/false);
  m_bInterpolate = !!pStreamDict->GetIntegerFor("Interpolate");
  m_Height = pStreamDict->GetIntegerFor("Height");
  m_Width = pStreamDict->GetIntegerFor("Width");
}

void CPDF_Image::ConvertStreamToIndirectObject() {
  if (!m_pStream->IsInline())
    return;

  m_pDocument->AddIndirectObject(m_pStream);
}

const CPDF_Dictionary* CPDF_Image::GetDict() const {
  return m_pStream ? m_pStream->GetDict() : nullptr;
}

RetainPtr<CPDF_Dictionary> CPDF_Image::InitJPEG(
    pdfium::span<uint8_t> src_span) {
  absl::optional<JpegModule::ImageInfo> info_opt =
      JpegModule::LoadInfo(src_span);
  if (!info_opt.has_value())
    return nullptr;

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
    CPDF_Array* pDecode = pDict->SetNewFor<CPDF_Array>("Decode");
    for (int n = 0; n < 4; n++) {
      pDecode->AppendNew<CPDF_Number>(1);
      pDecode->AppendNew<CPDF_Number>(0);
    }
  }
  pDict->SetNewFor<CPDF_Name>("ColorSpace", csname);
  pDict->SetNewFor<CPDF_Number>("BitsPerComponent", info.bits_per_components);
  pDict->SetNewFor<CPDF_Name>("Filter", "DCTDecode");
  if (!info.color_transform) {
    CPDF_Dictionary* pParms =
        pDict->SetNewFor<CPDF_Dictionary>(pdfium::stream::kDecodeParms);
    pParms->SetNewFor<CPDF_Number>("ColorTransform", 0);
  }
  m_bIsMask = false;
  m_Width = info.width;
  m_Height = info.height;
  if (!m_pStream)
    m_pStream = pdfium::MakeRetain<CPDF_Stream>();
  return pDict;
}

void CPDF_Image::SetJpegImage(const RetainPtr<IFX_SeekableReadStream>& pFile) {
  uint32_t size = pdfium::base::checked_cast<uint32_t>(pFile->GetSize());
  if (!size)
    return;

  uint32_t dwEstimateSize = std::min(size, 8192U);
  std::vector<uint8_t, FxAllocAllocator<uint8_t>> data(dwEstimateSize);
  if (!pFile->ReadBlockAtOffset(data.data(), 0, dwEstimateSize))
    return;

  RetainPtr<CPDF_Dictionary> pDict = InitJPEG(data);
  if (!pDict && size > dwEstimateSize) {
    data.resize(size);
    if (pFile->ReadBlockAtOffset(data.data(), 0, size))
      pDict = InitJPEG(data);
  }
  if (!pDict)
    return;

  m_pStream->InitStreamFromFile(pFile, std::move(pDict));
}

void CPDF_Image::SetJpegImageInline(
    const RetainPtr<IFX_SeekableReadStream>& pFile) {
  uint32_t size = pdfium::base::checked_cast<uint32_t>(pFile->GetSize());
  if (!size)
    return;

  std::vector<uint8_t, FxAllocAllocator<uint8_t>> data(size);
  if (!pFile->ReadBlockAtOffset(data.data(), 0, size))
    return;

  RetainPtr<CPDF_Dictionary> pDict = InitJPEG(data);
  if (!pDict)
    return;

  m_pStream = pdfium::MakeRetain<CPDF_Stream>(data, std::move(pDict));
}

void CPDF_Image::SetImage(const RetainPtr<CFX_DIBitmap>& pBitmap) {
  int32_t BitmapWidth = pBitmap->GetWidth();
  int32_t BitmapHeight = pBitmap->GetHeight();
  if (BitmapWidth < 1 || BitmapHeight < 1)
    return;

  RetainPtr<CPDF_Dictionary> pDict =
      CreateXObjectImageDict(BitmapWidth, BitmapHeight);
  const int32_t bpp = pBitmap->GetBPP();
  size_t dest_pitch = 0;
  bool bCopyWithoutAlpha = true;
  if (bpp == 1) {
    int32_t reset_a = 0;
    int32_t reset_r = 0;
    int32_t reset_g = 0;
    int32_t reset_b = 0;
    int32_t set_a = 0;
    int32_t set_r = 0;
    int32_t set_g = 0;
    int32_t set_b = 0;
    if (!pBitmap->IsMaskFormat()) {
      std::tie(reset_a, reset_r, reset_g, reset_b) =
          ArgbDecode(pBitmap->GetPaletteArgb(0));
      std::tie(set_a, set_r, set_g, set_b) =
          ArgbDecode(pBitmap->GetPaletteArgb(1));
    }
    if (set_a == 0 || reset_a == 0) {
      pDict->SetNewFor<CPDF_Boolean>("ImageMask", true);
      if (reset_a == 0) {
        CPDF_Array* pArray = pDict->SetNewFor<CPDF_Array>("Decode");
        pArray->AppendNew<CPDF_Number>(1);
        pArray->AppendNew<CPDF_Number>(0);
      }
    } else {
      CPDF_Array* pCS = pDict->SetNewFor<CPDF_Array>("ColorSpace");
      pCS->AppendNew<CPDF_Name>("Indexed");
      pCS->AppendNew<CPDF_Name>("DeviceRGB");
      pCS->AppendNew<CPDF_Number>(1);
      ByteString ct;
      {
        // Span's lifetime must end before ReleaseBuffer() below.
        pdfium::span<char> pBuf = ct.GetBuffer(6);
        pBuf[0] = static_cast<char>(reset_r);
        pBuf[1] = static_cast<char>(reset_g);
        pBuf[2] = static_cast<char>(reset_b);
        pBuf[3] = static_cast<char>(set_r);
        pBuf[4] = static_cast<char>(set_g);
        pBuf[5] = static_cast<char>(set_b);
      }
      ct.ReleaseBuffer(6);
      pCS->AppendNew<CPDF_String>(ct, true);
    }
    pDict->SetNewFor<CPDF_Number>("BitsPerComponent", 1);
    dest_pitch = (BitmapWidth + 7) / 8;
  } else if (bpp == 8) {
    size_t palette_size = pBitmap->GetRequiredPaletteSize();
    if (palette_size > 0) {
      DCHECK(palette_size <= 256);
      CPDF_Array* pCS = m_pDocument->NewIndirect<CPDF_Array>();
      pCS->AppendNew<CPDF_Name>("Indexed");
      pCS->AppendNew<CPDF_Name>("DeviceRGB");
      pCS->AppendNew<CPDF_Number>(static_cast<int>(palette_size - 1));
      std::unique_ptr<uint8_t, FxFreeDeleter> pColorTable(
          FX_Alloc2D(uint8_t, palette_size, 3));
      uint8_t* ptr = pColorTable.get();
      for (size_t i = 0; i < palette_size; i++) {
        uint32_t argb = pBitmap->GetPaletteArgb(i);
        ptr[0] = FXARGB_R(argb);
        ptr[1] = FXARGB_G(argb);
        ptr[2] = FXARGB_B(argb);
        ptr += 3;
      }
      auto pNewDict = m_pDocument->New<CPDF_Dictionary>();
      CPDF_Stream* pCTS = m_pDocument->NewIndirect<CPDF_Stream>(
          std::move(pColorTable), palette_size * 3, std::move(pNewDict));
      pCS->AppendNew<CPDF_Reference>(m_pDocument.Get(), pCTS->GetObjNum());
      pDict->SetNewFor<CPDF_Reference>("ColorSpace", m_pDocument.Get(),
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
  if (pBitmap->IsAlphaFormat())
    pMaskBitmap = pBitmap->CloneAlphaMask();

  if (pMaskBitmap) {
    int32_t maskWidth = pMaskBitmap->GetWidth();
    int32_t maskHeight = pMaskBitmap->GetHeight();
    std::unique_ptr<uint8_t, FxFreeDeleter> mask_buf;
    int32_t mask_size = 0;
    RetainPtr<CPDF_Dictionary> pMaskDict =
        CreateXObjectImageDict(maskWidth, maskHeight);
    pMaskDict->SetNewFor<CPDF_Name>("ColorSpace", "DeviceGray");
    pMaskDict->SetNewFor<CPDF_Number>("BitsPerComponent", 8);
    if (pMaskBitmap->GetFormat() != FXDIB_Format::k1bppMask) {
      mask_buf.reset(FX_AllocUninit2D(uint8_t, maskHeight, maskWidth));
      mask_size = maskHeight * maskWidth;  // Safe since checked alloc returned.
      for (int32_t a = 0; a < maskHeight; a++) {
        memcpy(mask_buf.get() + a * maskWidth,
               pMaskBitmap->GetScanline(a).data(), maskWidth);
      }
    }
    pMaskDict->SetNewFor<CPDF_Number>("Length", mask_size);
    CPDF_Stream* pNewStream = m_pDocument->NewIndirect<CPDF_Stream>(
        std::move(mask_buf), mask_size, std::move(pMaskDict));
    pDict->SetNewFor<CPDF_Reference>("SMask", m_pDocument.Get(),
                                     pNewStream->GetObjNum());
  }

  uint8_t* src_buf = pBitmap->GetBuffer();
  int32_t src_pitch = pBitmap->GetPitch();
  std::unique_ptr<uint8_t, FxFreeDeleter> dest_buf(
      FX_Alloc2D(uint8_t, dest_pitch, BitmapHeight));
  // Safe as checked alloc returned.
  size_t dest_size = dest_pitch * BitmapHeight;
  auto dest_span = pdfium::make_span(dest_buf.get(), dest_size);
  size_t dest_span_offset = 0;
  if (bCopyWithoutAlpha) {
    for (int32_t i = 0; i < BitmapHeight; i++) {
      fxcrt::spancpy(dest_span.subspan(dest_span_offset),
                     pdfium::make_span(src_buf, dest_pitch));
      dest_span_offset += dest_pitch;
      src_buf += src_pitch;
    }
  } else {
    int32_t src_offset = 0;
    for (int32_t row = 0; row < BitmapHeight; row++) {
      size_t dest_span_row_offset = dest_span_offset;
      src_offset = row * src_pitch;
      for (int32_t column = 0; column < BitmapWidth; column++) {
        float alpha = 1;
        dest_span[dest_span_row_offset] =
            static_cast<uint8_t>(src_buf[src_offset + 2] * alpha);
        dest_span[dest_span_row_offset + 1] =
            static_cast<uint8_t>(src_buf[src_offset + 1] * alpha);
        dest_span[dest_span_row_offset + 2] =
            static_cast<uint8_t>(src_buf[src_offset] * alpha);
        dest_span_row_offset += 3;
        src_offset += bpp == 24 ? 3 : 4;
      }

      dest_span_offset += dest_pitch;
    }
  }

  m_pStream = pdfium::MakeRetain<CPDF_Stream>(dest_span, std::move(pDict));
  m_bIsMask = pBitmap->IsMaskFormat();
  m_Width = BitmapWidth;
  m_Height = BitmapHeight;
}

void CPDF_Image::ResetCache(CPDF_Page* pPage) {
  RetainPtr<CPDF_Image> pHolder(this);
  pPage->GetRenderCache()->ResetBitmapForImage(pHolder);
}

RetainPtr<CFX_DIBBase> CPDF_Image::LoadDIBBase() const {
  auto source =
      pdfium::MakeRetain<CPDF_DIB>(m_pDocument.Get(), m_pStream.Get());
  if (!source->Load())
    return nullptr;

  if (!source->IsJBigImage())
    return source;

  CPDF_DIB::LoadState ret = CPDF_DIB::LoadState::kContinue;
  while (ret == CPDF_DIB::LoadState::kContinue)
    ret = source->ContinueLoadDIBBase(nullptr);
  return ret == CPDF_DIB::LoadState::kSuccess ? source : nullptr;
}

RetainPtr<CFX_DIBBase> CPDF_Image::DetachBitmap() {
  return std::move(m_pDIBBase);
}

RetainPtr<CFX_DIBBase> CPDF_Image::DetachMask() {
  return std::move(m_pMask);
}

bool CPDF_Image::StartLoadDIBBase(const CPDF_Dictionary* pFormResource,
                                  const CPDF_Dictionary* pPageResource,
                                  bool bStdCS,
                                  CPDF_ColorSpace::Family GroupFamily,
                                  bool bLoadMask) {
  auto source =
      pdfium::MakeRetain<CPDF_DIB>(m_pDocument.Get(), m_pStream.Get());
  CPDF_DIB::LoadState ret = source->StartLoadDIBBase(
      true, pFormResource, pPageResource, bStdCS, GroupFamily, bLoadMask);
  if (ret == CPDF_DIB::LoadState::kFail) {
    m_pDIBBase.Reset();
    return false;
  }
  m_pDIBBase = source;
  if (ret == CPDF_DIB::LoadState::kContinue)
    return true;

  m_pMask = source->DetachMask();
  m_MatteColor = source->GetMatteColor();
  return false;
}

bool CPDF_Image::Continue(PauseIndicatorIface* pPause) {
  RetainPtr<CPDF_DIB> pSource = m_pDIBBase.As<CPDF_DIB>();
  CPDF_DIB::LoadState ret = pSource->ContinueLoadDIBBase(pPause);
  if (ret == CPDF_DIB::LoadState::kContinue)
    return true;

  if (ret == CPDF_DIB::LoadState::kSuccess) {
    m_pMask = pSource->DetachMask();
    m_MatteColor = pSource->GetMatteColor();
  } else {
    m_pDIBBase.Reset();
  }
  return false;
}

RetainPtr<CPDF_Dictionary> CPDF_Image::CreateXObjectImageDict(int width,
                                                              int height) {
  auto dict = m_pDocument->New<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "XObject");
  dict->SetNewFor<CPDF_Name>("Subtype", "Image");
  dict->SetNewFor<CPDF_Number>("Width", width);
  dict->SetNewFor<CPDF_Number>("Height", height);
  return dict;
}
