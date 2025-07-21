// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_dib.h"

#include <stdint.h>

#include <algorithm>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "core/fpdfapi/page/cpdf_colorspace.h"
#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/page/cpdf_image.h"
#include "core/fpdfapi/page/cpdf_imageobject.h"
#include "core/fpdfapi/page/cpdf_indexedcs.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "core/fxcodec/basic/basicmodule.h"
#include "core/fxcodec/icc/icc_transform.h"
#include "core/fxcodec/jbig2/jbig2_decoder.h"
#include "core/fxcodec/jpeg/jpegmodule.h"
#include "core/fxcodec/jpx/cjpx_decoder.h"
#include "core/fxcodec/scanlinedecoder.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_memcpy_wrappers.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/notreached.h"
#include "core/fxcrt/span_util.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxcrt/zip.h"
#include "core/fxge/calculate_pitch.h"
#include "core/fxge/dib/cfx_dibitmap.h"

namespace {

bool IsValidDimension(int value) {
  static constexpr int kMaxImageDimension = 0x01FFFF;
  return value > 0 && value <= kMaxImageDimension;
}

unsigned int GetBits8(pdfium::span<const uint8_t> pData,
                      uint64_t bitpos,
                      size_t nbits) {
  DCHECK(nbits == 1 || nbits == 2 || nbits == 4 || nbits == 8 || nbits == 16);
  DCHECK_EQ((bitpos & (nbits - 1)), 0u);
  unsigned int byte = pData[bitpos / 8];
  if (nbits == 8) {
    return byte;
  }
  if (nbits == 16) {
    return byte * 256 + pData[bitpos / 8 + 1];
  }
  return (byte >> (8 - nbits - (bitpos % 8))) & ((1 << nbits) - 1);
}

bool GetBitValue(pdfium::span<const uint8_t> pSrc, uint32_t pos) {
  return pSrc[pos / 8] & (1 << (7 - pos % 8));
}

// Just to sanity check and filter out obvious bad values.
bool IsMaybeValidBitsPerComponent(int bpc) {
  return bpc >= 0 && bpc <= 16;
}

bool IsAllowedBitsPerComponent(int bpc) {
  return bpc == 1 || bpc == 2 || bpc == 4 || bpc == 8 || bpc == 16;
}

bool IsColorIndexOutOfBounds(uint8_t index, const DIB_COMP_DATA& comp_datum) {
  return index < comp_datum.color_key_min_ || index > comp_datum.color_key_max_;
}

bool AreColorIndicesOutOfBounds(pdfium::span<const uint8_t> indices,
                                pdfium::span<const DIB_COMP_DATA> comp_data) {
  for (auto [idx, datum] : fxcrt::Zip(indices, comp_data)) {
    if (IsColorIndexOutOfBounds(idx, datum)) {
      return true;
    }
  }
  return false;
}

CJPX_Decoder::ColorSpaceOption ColorSpaceOptionFromColorSpace(
    CPDF_ColorSpace* pCS) {
  if (!pCS) {
    return CJPX_Decoder::ColorSpaceOption::kNone;
  }
  if (pCS->GetFamily() == CPDF_ColorSpace::Family::kIndexed) {
    return CJPX_Decoder::ColorSpaceOption::kIndexed;
  }
  return CJPX_Decoder::ColorSpaceOption::kNormal;
}

enum class JpxDecodeAction {
  kDoNothing,
  kUseGray,
  kUseIndexed,
  kUseRgb,
  kUseCmyk,
  kConvertArgbToRgb,
};

// ISO 32000-1:2008 section 7.4.9 says the PDF and JPX colorspaces should have
// the same number of color channels. This helper function checks the
// colorspaces match, but also tolerates unknowns.
bool IsJPXColorSpaceOrUnspecifiedOrUnknown(COLOR_SPACE actual,
                                           COLOR_SPACE expected) {
  return actual == expected || actual == OPJ_CLRSPC_UNSPECIFIED ||
         actual == OPJ_CLRSPC_UNKNOWN;
}

// Decides which JpxDecodeAction to use based on the colorspace information from
// the PDF and the JPX image. Called only when the PDF's image object contains a
// "/ColorSpace" entry.
std::optional<JpxDecodeAction> GetJpxDecodeActionFromColorSpaces(
    const CJPX_Decoder::JpxImageInfo& jpx_info,
    const CPDF_ColorSpace* pdf_colorspace) {
  if (pdf_colorspace ==
      CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceGray)) {
    if (!IsJPXColorSpaceOrUnspecifiedOrUnknown(/*actual=*/jpx_info.colorspace,
                                               /*expected=*/OPJ_CLRSPC_GRAY)) {
      return std::nullopt;
    }
    return JpxDecodeAction::kUseGray;
  }

  if (pdf_colorspace ==
      CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceRGB)) {
    if (!IsJPXColorSpaceOrUnspecifiedOrUnknown(/*actual=*/jpx_info.colorspace,
                                               /*expected=*/OPJ_CLRSPC_SRGB)) {
      return std::nullopt;
    }

    // The channel count of a JPX image can be different from the PDF color
    // space's component count.
    if (jpx_info.channels > 3) {
      return JpxDecodeAction::kConvertArgbToRgb;
    }
    return JpxDecodeAction::kUseRgb;
  }

  if (pdf_colorspace ==
      CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceCMYK)) {
    if (!IsJPXColorSpaceOrUnspecifiedOrUnknown(/*actual=*/jpx_info.colorspace,
                                               /*expected=*/OPJ_CLRSPC_CMYK)) {
      return std::nullopt;
    }
    return JpxDecodeAction::kUseCmyk;
  }

  // Many PDFs generated by iOS meets this condition. Handle the discrepancy.
  // See https://crbug.com/345431077 for example.
  if (pdf_colorspace->ComponentCount() == 3 && jpx_info.channels == 4 &&
      jpx_info.colorspace == OPJ_CLRSPC_SRGB) {
    return JpxDecodeAction::kConvertArgbToRgb;
  }

  if (pdf_colorspace->GetFamily() == CPDF_ColorSpace::Family::kIndexed &&
      pdf_colorspace->ComponentCount() == 1) {
    return JpxDecodeAction::kUseIndexed;
  }

  return JpxDecodeAction::kDoNothing;
}

JpxDecodeAction GetJpxDecodeActionFromImageColorSpace(
    const CJPX_Decoder::JpxImageInfo& jpx_info) {
  switch (jpx_info.colorspace) {
    case OPJ_CLRSPC_UNKNOWN:
    case OPJ_CLRSPC_UNSPECIFIED:
      return jpx_info.channels == 3 ? JpxDecodeAction::kUseRgb
                                    : JpxDecodeAction::kDoNothing;

    case OPJ_CLRSPC_SYCC:
    case OPJ_CLRSPC_EYCC:
      return JpxDecodeAction::kDoNothing;

    case OPJ_CLRSPC_SRGB:
      return jpx_info.channels > 3 ? JpxDecodeAction::kConvertArgbToRgb
                                   : JpxDecodeAction::kUseRgb;

    case OPJ_CLRSPC_GRAY:
      return JpxDecodeAction::kUseGray;

    case OPJ_CLRSPC_CMYK:
      return JpxDecodeAction::kUseCmyk;
  }
  NOTREACHED();
}

int GetComponentCountFromJpxImageInfo(
    const CJPX_Decoder::JpxImageInfo& jpx_info) {
  switch (jpx_info.colorspace) {
    case OPJ_CLRSPC_UNKNOWN:
    case OPJ_CLRSPC_UNSPECIFIED:
      return jpx_info.channels;

    case OPJ_CLRSPC_GRAY:
      return 1;

    case OPJ_CLRSPC_SRGB:
    case OPJ_CLRSPC_SYCC:
    case OPJ_CLRSPC_EYCC:
      return 3;

    case OPJ_CLRSPC_CMYK:
      return 4;
  }
  NOTREACHED();
}

class JpxDecodeConversion {
 public:
  static std::optional<JpxDecodeConversion> Create(
      const CJPX_Decoder::JpxImageInfo& jpx_info,
      const CPDF_ColorSpace* pdf_colorspace) {
    // When the PDF does not provide a color space, check the image color space.
    std::optional<JpxDecodeAction> maybe_action =
        pdf_colorspace
            ? GetJpxDecodeActionFromColorSpaces(jpx_info, pdf_colorspace)
            : GetJpxDecodeActionFromImageColorSpace(jpx_info);
    if (!maybe_action.has_value()) {
      return std::nullopt;
    }

    JpxDecodeConversion conversion;
    conversion.action_ = maybe_action.value();
    switch (conversion.action_) {
      case JpxDecodeAction::kDoNothing:
        break;

      case JpxDecodeAction::kUseGray:
        conversion.override_colorspace_ =
            CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceGray);
        break;

      case JpxDecodeAction::kUseIndexed:
        break;

      case JpxDecodeAction::kUseRgb:
        DCHECK_GE(jpx_info.channels, 3);
        conversion.override_colorspace_ = nullptr;
        break;

      case JpxDecodeAction::kUseCmyk:
        conversion.override_colorspace_ =
            CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceCMYK);
        break;

      case JpxDecodeAction::kConvertArgbToRgb:
        conversion.override_colorspace_ = nullptr;
        break;
    }

    // If there exists a PDF colorspace, then CPDF_DIB already has the
    // components count.
    if (!pdf_colorspace) {
      conversion.jpx_components_count_ =
          GetComponentCountFromJpxImageInfo(jpx_info);
    }
    return conversion;
  }

  JpxDecodeAction action() const { return action_; }

  const std::optional<RetainPtr<CPDF_ColorSpace>>& override_colorspace() const {
    return override_colorspace_;
  }

  const std::optional<int>& jpx_components_count() const {
    return jpx_components_count_;
  }

  bool swap_rgb() const {
    return action_ == JpxDecodeAction::kUseRgb ||
           action_ == JpxDecodeAction::kConvertArgbToRgb;
  }

 private:
  JpxDecodeAction action_;

  // The colorspace to override the existing colorspace.
  //
  // std::nullopt means no override colorspace.
  // nullptr means reset the colorspace.
  std::optional<RetainPtr<CPDF_ColorSpace>> override_colorspace_;

  // The components count from the JPEG2000 image.
  //
  // std::nullopt means no new components count.
  // Value <= 0 means failure.
  std::optional<int> jpx_components_count_;
};

}  // namespace

CPDF_DIB::CPDF_DIB(CPDF_Document* pDoc, RetainPtr<const CPDF_Stream> pStream)
    : document_(pDoc), stream_(std::move(pStream)) {}

CPDF_DIB::~CPDF_DIB() = default;

CPDF_DIB::JpxSMaskInlineData::JpxSMaskInlineData() = default;

CPDF_DIB::JpxSMaskInlineData::~JpxSMaskInlineData() = default;

bool CPDF_DIB::Load() {
  if (!LoadInternal(nullptr, nullptr)) {
    return false;
  }

  if (CreateDecoder(0) == LoadState::kFail) {
    return false;
  }

  return ContinueInternal();
}

bool CPDF_DIB::ContinueToLoadMask() {
  if (color_space_ && std_cs_) {
    color_space_->EnableStdConversion(true);
  }

  return ContinueInternal();
}

bool CPDF_DIB::ContinueInternal() {
  if (image_mask_) {
    SetMaskProperties();
  } else {
    const uint32_t bpp = bpc_ * components_;
    if (bpp == 0) {
      return false;
    }

    if (bpp == 1) {
      SetFormat(FXDIB_Format::k1bppRgb);
    } else if (bpp <= 8) {
      SetFormat(FXDIB_Format::k8bppRgb);
    } else {
      SetFormat(FXDIB_Format::kBgr);
    }
  }

  std::optional<uint32_t> pitch = fxge::CalculatePitch32(GetBPP(), GetWidth());
  if (!pitch.has_value()) {
    return false;
  }

  line_buf_ = DataVector<uint8_t>(pitch.value());
  LoadPalette();
  if (color_key_) {
    // TODO(crbug.com/355676038): Consider adding support for
    // `FXDIB_Format::kBgraPremul`
    SetFormat(FXDIB_Format::kBgra);
    pitch = fxge::CalculatePitch32(GetBPP(), GetWidth());
    if (!pitch.has_value()) {
      return false;
    }
    mask_buf_ = DataVector<uint8_t>(pitch.value());
  }
  SetPitch(pitch.value());
  return true;
}

CPDF_DIB::LoadState CPDF_DIB::StartLoadDIBBase(
    bool bHasMask,
    const CPDF_Dictionary* pFormResources,
    const CPDF_Dictionary* pPageResources,
    bool bStdCS,
    CPDF_ColorSpace::Family GroupFamily,
    bool bLoadMask,
    const CFX_Size& max_size_required) {
  std_cs_ = bStdCS;
  has_mask_ = bHasMask;
  group_family_ = GroupFamily;
  load_mask_ = bLoadMask;

  if (!stream_->IsInline()) {
    pFormResources = nullptr;
  }

  if (!LoadInternal(pFormResources, pPageResources)) {
    return LoadState::kFail;
  }

  uint8_t resolution_levels_to_skip = 0;
  if (max_size_required.width != 0 && max_size_required.height != 0) {
    resolution_levels_to_skip = static_cast<uint8_t>(std::log2(
        std::max(1, std::min(GetWidth() / max_size_required.width,
                             GetHeight() / max_size_required.height))));
  }

  LoadState iCreatedDecoder = CreateDecoder(resolution_levels_to_skip);
  if (iCreatedDecoder == LoadState::kFail) {
    return LoadState::kFail;
  }

  if (!ContinueToLoadMask()) {
    return LoadState::kFail;
  }

  LoadState iLoadedMask = has_mask_ ? StartLoadMask() : LoadState::kSuccess;
  if (iCreatedDecoder == LoadState::kContinue ||
      iLoadedMask == LoadState::kContinue) {
    return LoadState::kContinue;
  }

  DCHECK_EQ(iCreatedDecoder, LoadState::kSuccess);
  DCHECK_EQ(iLoadedMask, LoadState::kSuccess);
  if (color_space_ && std_cs_) {
    color_space_->EnableStdConversion(false);
  }
  return LoadState::kSuccess;
}

CPDF_DIB::LoadState CPDF_DIB::ContinueLoadDIBBase(PauseIndicatorIface* pPause) {
  if (status_ == LoadState::kContinue) {
    return ContinueLoadMaskDIB(pPause);
  }

  ByteString decoder = stream_acc_->GetImageDecoder();
  if (decoder == "JPXDecode") {
    return LoadState::kFail;
  }

  if (decoder != "JBIG2Decode") {
    return LoadState::kSuccess;
  }

  if (status_ == LoadState::kFail) {
    return LoadState::kFail;
  }

  FXCODEC_STATUS iDecodeStatus;
  if (!jbig_2context_) {
    jbig_2context_ = std::make_unique<Jbig2Context>();
    if (stream_acc_->GetImageParam()) {
      RetainPtr<const CPDF_Stream> pGlobals =
          stream_acc_->GetImageParam()->GetStreamFor("JBIG2Globals");
      if (pGlobals) {
        global_acc_ = pdfium::MakeRetain<CPDF_StreamAcc>(std::move(pGlobals));
        global_acc_->LoadAllDataFiltered();
      }
    }
    uint64_t nSrcKey = 0;
    pdfium::span<const uint8_t> pSrcSpan;
    if (stream_acc_) {
      pSrcSpan = stream_acc_->GetSpan();
      nSrcKey = stream_acc_->KeyForCache();
    }
    uint64_t nGlobalKey = 0;
    pdfium::span<const uint8_t> pGlobalSpan;
    if (global_acc_) {
      pGlobalSpan = global_acc_->GetSpan();
      nGlobalKey = global_acc_->KeyForCache();
    }
    iDecodeStatus = Jbig2Decoder::StartDecode(
        jbig_2context_.get(), document_->GetOrCreateCodecContext(), GetWidth(),
        GetHeight(), pSrcSpan, nSrcKey, pGlobalSpan, nGlobalKey,
        cached_bitmap_->GetWritableBuffer(), cached_bitmap_->GetPitch(),
        pPause);
  } else {
    iDecodeStatus = Jbig2Decoder::ContinueDecode(jbig_2context_.get(), pPause);
  }

  if (iDecodeStatus == FXCODEC_STATUS::kError) {
    jbig_2context_.reset();
    cached_bitmap_.Reset();
    global_acc_.Reset();
    return LoadState::kFail;
  }
  if (iDecodeStatus == FXCODEC_STATUS::kDecodeToBeContinued) {
    return LoadState::kContinue;
  }

  LoadState iContinueStatus = LoadState::kSuccess;
  if (has_mask_) {
    if (ContinueLoadMaskDIB(pPause) == LoadState::kContinue) {
      iContinueStatus = LoadState::kContinue;
      status_ = LoadState::kContinue;
    }
  }
  if (iContinueStatus == LoadState::kContinue) {
    return LoadState::kContinue;
  }

  if (color_space_ && std_cs_) {
    color_space_->EnableStdConversion(false);
  }
  return iContinueStatus;
}

bool CPDF_DIB::LoadColorInfo(const CPDF_Dictionary* pFormResources,
                             const CPDF_Dictionary* pPageResources) {
  std::optional<DecoderArray> decoder_array = GetDecoderArray(dict_);
  if (!decoder_array.has_value()) {
    return false;
  }

  bpc_orig_ = dict_->GetIntegerFor("BitsPerComponent");
  if (!IsMaybeValidBitsPerComponent(bpc_orig_)) {
    return false;
  }

  image_mask_ = dict_->GetBooleanFor("ImageMask", /*bDefault=*/false);

  if (image_mask_ || !dict_->KeyExist("ColorSpace")) {
    if (!image_mask_ && !decoder_array.value().empty()) {
      const ByteString& filter = decoder_array.value().back().first;
      if (filter == "JPXDecode") {
        do_bpc_check_ = false;
        return true;
      }
    }
    image_mask_ = true;
    bpc_ = components_ = 1;
    RetainPtr<const CPDF_Array> pDecode = dict_->GetArrayFor("Decode");
    default_decode_ = !pDecode || !pDecode->GetIntegerAt(0);
    return true;
  }

  RetainPtr<const CPDF_Object> pCSObj = dict_->GetDirectObjectFor("ColorSpace");
  if (!pCSObj) {
    return false;
  }

  auto* pDocPageData = CPDF_DocPageData::FromDocument(document_);
  if (pFormResources) {
    color_space_ = pDocPageData->GetColorSpace(pCSObj.Get(), pFormResources);
  }
  if (!color_space_) {
    color_space_ = pDocPageData->GetColorSpace(pCSObj.Get(), pPageResources);
  }
  if (!color_space_) {
    return false;
  }

  // If the checks above failed to find a colorspace, and the next line to set
  // |components_| does not get reached, then a decoder can try to set
  // |components_| based on the number of channels in the image being
  // decoded.
  components_ = color_space_->ComponentCount();
  family_ = color_space_->GetFamily();
  if (family_ == CPDF_ColorSpace::Family::kICCBased && pCSObj->IsName()) {
    ByteString cs = pCSObj->GetString();
    if (cs == "DeviceGray") {
      components_ = 1;
    } else if (cs == "DeviceRGB") {
      components_ = 3;
    } else if (cs == "DeviceCMYK") {
      components_ = 4;
    }
  }

  ByteString filter;
  if (!decoder_array.value().empty()) {
    filter = decoder_array.value().back().first;
  }

  if (!ValidateDictParam(filter)) {
    return false;
  }

  return GetDecodeAndMaskArray();
}

bool CPDF_DIB::GetDecodeAndMaskArray() {
  if (!color_space_) {
    return false;
  }

  comp_data_.resize(components_);
  int max_data = (1 << bpc_) - 1;
  RetainPtr<const CPDF_Array> pDecode = dict_->GetArrayFor("Decode");
  if (pDecode) {
    for (uint32_t i = 0; i < components_; i++) {
      comp_data_[i].decode_min_ = pDecode->GetFloatAt(i * 2);
      float max = pDecode->GetFloatAt(i * 2 + 1);
      comp_data_[i].decode_step_ = (max - comp_data_[i].decode_min_) / max_data;
      float def_value;
      float def_min;
      float def_max;
      color_space_->GetDefaultValue(i, &def_value, &def_min, &def_max);
      if (family_ == CPDF_ColorSpace::Family::kIndexed) {
        def_max = max_data;
      }
      if (def_min != comp_data_[i].decode_min_ || def_max != max) {
        default_decode_ = false;
      }
    }
  } else {
    for (uint32_t i = 0; i < components_; i++) {
      float def_value;
      color_space_->GetDefaultValue(i, &def_value, &comp_data_[i].decode_min_,
                                    &comp_data_[i].decode_step_);
      if (family_ == CPDF_ColorSpace::Family::kIndexed) {
        comp_data_[i].decode_step_ = max_data;
      }
      comp_data_[i].decode_step_ =
          (comp_data_[i].decode_step_ - comp_data_[i].decode_min_) / max_data;
    }
  }
  if (dict_->KeyExist("SMask")) {
    return true;
  }

  RetainPtr<const CPDF_Object> pMask = dict_->GetDirectObjectFor("Mask");
  if (!pMask) {
    return true;
  }

  if (const CPDF_Array* pArray = pMask->AsArray()) {
    if (pArray->size() >= components_ * 2) {
      for (uint32_t i = 0; i < components_; i++) {
        int min_num = pArray->GetIntegerAt(i * 2);
        int max_num = pArray->GetIntegerAt(i * 2 + 1);
        comp_data_[i].color_key_min_ = std::max(min_num, 0);
        comp_data_[i].color_key_max_ = std::min(max_num, max_data);
      }
    }
    color_key_ = true;
  }
  return true;
}

CPDF_DIB::LoadState CPDF_DIB::CreateDecoder(uint8_t resolution_levels_to_skip) {
  ByteString decoder = stream_acc_->GetImageDecoder();
  if (decoder.IsEmpty()) {
    return LoadState::kSuccess;
  }

  if (do_bpc_check_ && bpc_ == 0) {
    return LoadState::kFail;
  }

  if (decoder == "JPXDecode") {
    cached_bitmap_ = LoadJpxBitmap(resolution_levels_to_skip);
    return cached_bitmap_ ? LoadState::kSuccess : LoadState::kFail;
  }

  if (decoder == "JBIG2Decode") {
    cached_bitmap_ = pdfium::MakeRetain<CFX_DIBitmap>();
    if (!cached_bitmap_->Create(
            GetWidth(), GetHeight(),
            image_mask_ ? FXDIB_Format::k1bppMask : FXDIB_Format::k1bppRgb)) {
      cached_bitmap_.Reset();
      return LoadState::kFail;
    }
    status_ = LoadState::kSuccess;
    return LoadState::kContinue;
  }

  pdfium::span<const uint8_t> src_span = stream_acc_->GetSpan();
  RetainPtr<const CPDF_Dictionary> pParams = stream_acc_->GetImageParam();
  if (decoder == "CCITTFaxDecode") {
    decoder_ = CreateFaxDecoder(src_span, GetWidth(), GetHeight(), pParams);
  } else if (decoder == "FlateDecode") {
    decoder_ = CreateFlateDecoder(src_span, GetWidth(), GetHeight(),
                                  components_, bpc_, pParams);
  } else if (decoder == "RunLengthDecode") {
    decoder_ = BasicModule::CreateRunLengthDecoder(
        src_span, GetWidth(), GetHeight(), components_, bpc_);
  } else if (decoder == "DCTDecode") {
    if (!CreateDCTDecoder(src_span, pParams)) {
      return LoadState::kFail;
    }
  }
  if (!decoder_) {
    return LoadState::kFail;
  }

  const std::optional<uint32_t> requested_pitch =
      fxge::CalculatePitch8(bpc_, components_, GetWidth());
  if (!requested_pitch.has_value()) {
    return LoadState::kFail;
  }
  const std::optional<uint32_t> provided_pitch = fxge::CalculatePitch8(
      decoder_->GetBPC(), decoder_->CountComps(), decoder_->GetWidth());
  if (!provided_pitch.has_value()) {
    return LoadState::kFail;
  }
  if (provided_pitch.value() < requested_pitch.value()) {
    return LoadState::kFail;
  }
  return LoadState::kSuccess;
}

bool CPDF_DIB::CreateDCTDecoder(pdfium::span<const uint8_t> src_span,
                                const CPDF_Dictionary* pParams) {
  decoder_ = JpegModule::CreateDecoder(
      src_span, GetWidth(), GetHeight(), components_,
      !pParams || pParams->GetIntegerFor("ColorTransform", 1));
  if (decoder_) {
    return true;
  }

  std::optional<JpegModule::ImageInfo> info_opt =
      JpegModule::LoadInfo(src_span);
  if (!info_opt.has_value()) {
    return false;
  }

  const JpegModule::ImageInfo& info = info_opt.value();
  SetWidth(info.width);
  SetHeight(info.height);

  if (!CPDF_Image::IsValidJpegComponent(info.num_components) ||
      !CPDF_Image::IsValidJpegBitsPerComponent(info.bits_per_components)) {
    return false;
  }

  if (components_ == static_cast<uint32_t>(info.num_components)) {
    bpc_ = info.bits_per_components;
    decoder_ = JpegModule::CreateDecoder(src_span, GetWidth(), GetHeight(),
                                         components_, info.color_transform);
    return true;
  }

  components_ = static_cast<uint32_t>(info.num_components);
  comp_data_.clear();
  if (color_space_) {
    uint32_t colorspace_comps = color_space_->ComponentCount();
    switch (family_) {
      case CPDF_ColorSpace::Family::kDeviceGray:
      case CPDF_ColorSpace::Family::kDeviceRGB:
      case CPDF_ColorSpace::Family::kDeviceCMYK: {
        uint32_t dwMinComps = CPDF_ColorSpace::ComponentsForFamily(family_);
        if (colorspace_comps < dwMinComps || components_ < dwMinComps) {
          return false;
        }
        break;
      }
      case CPDF_ColorSpace::Family::kLab: {
        if (components_ != 3 || colorspace_comps < 3) {
          return false;
        }
        break;
      }
      case CPDF_ColorSpace::Family::kICCBased: {
        if (!fxcodec::IccTransform::IsValidIccComponents(colorspace_comps) ||
            !fxcodec::IccTransform::IsValidIccComponents(components_) ||
            colorspace_comps < components_) {
          return false;
        }
        break;
      }
      default: {
        if (colorspace_comps != components_) {
          return false;
        }
        break;
      }
    }
  } else {
    if (family_ == CPDF_ColorSpace::Family::kLab && components_ != 3) {
      return false;
    }
  }
  if (!GetDecodeAndMaskArray()) {
    return false;
  }

  bpc_ = info.bits_per_components;
  decoder_ = JpegModule::CreateDecoder(src_span, GetWidth(), GetHeight(),
                                       components_, info.color_transform);
  return true;
}

RetainPtr<CFX_DIBitmap> CPDF_DIB::LoadJpxBitmap(
    uint8_t resolution_levels_to_skip) {
  std::unique_ptr<CJPX_Decoder> decoder =
      CJPX_Decoder::Create(stream_acc_->GetSpan(),
                           ColorSpaceOptionFromColorSpace(color_space_.Get()),
                           resolution_levels_to_skip, /*strict_mode=*/true);
  if (!decoder) {
    return nullptr;
  }

  SetWidth(GetWidth() >> resolution_levels_to_skip);
  SetHeight(GetHeight() >> resolution_levels_to_skip);

  if (!decoder->StartDecode()) {
    return nullptr;
  }

  CJPX_Decoder::JpxImageInfo image_info = decoder->GetInfo();
  if (static_cast<int>(image_info.width) < GetWidth() ||
      static_cast<int>(image_info.height) < GetHeight()) {
    return nullptr;
  }

  auto maybe_conversion =
      JpxDecodeConversion::Create(image_info, color_space_.Get());
  if (!maybe_conversion.has_value()) {
    return nullptr;
  }

  const auto& conversion = maybe_conversion.value();
  if (conversion.override_colorspace().has_value()) {
    color_space_ = conversion.override_colorspace().value();
  }

  if (conversion.jpx_components_count().has_value()) {
    DCHECK_EQ(0u, components_);
    components_ = conversion.jpx_components_count().value();
    if (components_ <= 0) {
      return nullptr;
    }
  } else {
    // LoadColorInfo() already set `components_`.
    DCHECK_NE(0u, components_);
  }

  FXDIB_Format format;
  if (conversion.action() == JpxDecodeAction::kUseGray ||
      conversion.action() == JpxDecodeAction::kUseIndexed) {
    format = FXDIB_Format::k8bppRgb;
  } else if (conversion.action() == JpxDecodeAction::kUseRgb &&
             image_info.channels == 3) {
    format = FXDIB_Format::kBgr;
  } else if (conversion.action() == JpxDecodeAction::kUseRgb &&
             image_info.channels == 4) {
    format = FXDIB_Format::kBgrx;
  } else if (conversion.action() == JpxDecodeAction::kConvertArgbToRgb) {
    CHECK_GE(image_info.channels, 4);
    format = FXDIB_Format::kBgrx;
  } else {
    image_info.width = (image_info.width * image_info.channels + 2) / 3;
    format = FXDIB_Format::kBgr;
  }

  auto result_bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!result_bitmap->Create(image_info.width, image_info.height, format)) {
    return nullptr;
  }

  result_bitmap->Clear(0xFFFFFFFF);
  if (!decoder->Decode(result_bitmap->GetWritableBuffer(),
                       result_bitmap->GetPitch(), conversion.swap_rgb(),
                       components_)) {
    return nullptr;
  }

  if (conversion.action() == JpxDecodeAction::kConvertArgbToRgb) {
    result_bitmap = ConvertArgbJpxBitmapToRgb(result_bitmap, image_info.width,
                                              image_info.height);
    if (!result_bitmap) {
      return nullptr;
    }
  } else if (color_space_ &&
             color_space_->GetFamily() == CPDF_ColorSpace::Family::kIndexed &&
             bpc_ < 8) {
    int scale = 8 - bpc_;
    for (uint32_t row = 0; row < image_info.height; ++row) {
      pdfium::span<uint8_t> scanline =
          result_bitmap->GetWritableScanline(row).first(image_info.width);
      for (auto& pixel : scanline) {
        pixel >>= scale;
      }
    }
  }

  // TODO(crbug.com/pdfium/1747): Handle SMaskInData entries for different
  // color space types.

  bpc_ = 8;
  return result_bitmap;
}

RetainPtr<CFX_DIBitmap> CPDF_DIB::ConvertArgbJpxBitmapToRgb(
    RetainPtr<CFX_DIBitmap> argb_bitmap,
    uint32_t width,
    uint32_t height) {
  DCHECK_EQ(3u, components_);
  auto rgb_bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!rgb_bitmap->Create(width, height, FXDIB_Format::kBgr)) {
    return nullptr;
  }
  if (dict_->GetIntegerFor("SMaskInData") == 1) {
    // TODO(thestig): Acrobat does not support "/SMaskInData 1" combined with
    // filters. Check for that and fail early.
    DCHECK(jpx_inline_data_.data.empty());
    jpx_inline_data_.width = width;
    jpx_inline_data_.height = height;
    jpx_inline_data_.data.reserve(width * height);
    for (uint32_t row = 0; row < height; ++row) {
      auto src =
          argb_bitmap->GetScanlineAs<FX_BGRA_STRUCT<uint8_t>>(row).first(width);
      auto dest =
          rgb_bitmap->GetWritableScanlineAs<FX_BGR_STRUCT<uint8_t>>(row);
      for (auto [input, output] : fxcrt::Zip(src, dest)) {
        jpx_inline_data_.data.push_back(input.alpha);
        const uint8_t na = 255 - input.alpha;
        output.blue = (input.blue * input.alpha + 255 * na) / 255;
        output.green = (input.green * input.alpha + 255 * na) / 255;
        output.red = (input.red * input.alpha + 255 * na) / 255;
      }
    }
    return rgb_bitmap;
  }

  // TODO(thestig): Is there existing code that does this already?
  for (uint32_t row = 0; row < height; ++row) {
    auto src =
        argb_bitmap->GetScanlineAs<FX_BGRA_STRUCT<uint8_t>>(row).first(width);
    auto dest = rgb_bitmap->GetWritableScanlineAs<FX_BGR_STRUCT<uint8_t>>(row);
    for (auto [input, output] : fxcrt::Zip(src, dest)) {
      output.green = input.green;
      output.red = input.red;
      output.blue = input.blue;
    }
  }
  return rgb_bitmap;
}

bool CPDF_DIB::LoadInternal(const CPDF_Dictionary* pFormResources,
                            const CPDF_Dictionary* pPageResources) {
  if (!stream_) {
    return false;
  }

  dict_ = stream_->GetDict();
  SetWidth(dict_->GetIntegerFor("Width"));
  SetHeight(dict_->GetIntegerFor("Height"));
  if (!IsValidDimension(GetWidth()) || !IsValidDimension(GetHeight())) {
    return false;
  }

  if (!LoadColorInfo(pFormResources, pPageResources)) {
    return false;
  }

  if (do_bpc_check_ && (bpc_ == 0 || components_ == 0)) {
    return false;
  }

  const std::optional<uint32_t> maybe_size =
      fxge::CalculatePitch8(bpc_, components_, GetWidth());
  if (!maybe_size.has_value()) {
    return false;
  }

  FX_SAFE_UINT32 src_size = maybe_size.value();
  src_size *= GetHeight();
  if (!src_size.IsValid()) {
    return false;
  }

  stream_acc_ = pdfium::MakeRetain<CPDF_StreamAcc>(stream_);
  stream_acc_->LoadAllDataImageAcc(src_size.ValueOrDie());
  return !stream_acc_->GetSpan().empty();
}

CPDF_DIB::LoadState CPDF_DIB::StartLoadMask() {
  matte_color_ = 0XFFFFFFFF;

  if (!jpx_inline_data_.data.empty()) {
    auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
    dict->SetNewFor<CPDF_Name>("Type", "XObject");
    dict->SetNewFor<CPDF_Name>("Subtype", "Image");
    dict->SetNewFor<CPDF_Name>("ColorSpace", "DeviceGray");
    dict->SetNewFor<CPDF_Number>("Width", jpx_inline_data_.width);
    dict->SetNewFor<CPDF_Number>("Height", jpx_inline_data_.height);
    dict->SetNewFor<CPDF_Number>("BitsPerComponent", 8);

    return StartLoadMaskDIB(pdfium::MakeRetain<CPDF_Stream>(
        jpx_inline_data_.data, std::move(dict)));
  }

  RetainPtr<const CPDF_Stream> mask(dict_->GetStreamFor("SMask"));
  if (!mask) {
    mask = ToStream(dict_->GetDirectObjectFor("Mask"));
    return mask ? StartLoadMaskDIB(std::move(mask)) : LoadState::kSuccess;
  }

  RetainPtr<const CPDF_Array> pMatte = mask->GetDict()->GetArrayFor("Matte");
  if (pMatte && color_space_ && family_ != CPDF_ColorSpace::Family::kPattern &&
      pMatte->size() == components_ &&
      color_space_->ComponentCount() <= components_) {
    std::vector<float> colors =
        ReadArrayElementsToVector(pMatte.Get(), components_);

    auto rgb = color_space_->GetRGBOrZerosOnError(colors);
    matte_color_ =
        ArgbEncode(0, FXSYS_roundf(rgb.red * 255),
                   FXSYS_roundf(rgb.green * 255), FXSYS_roundf(rgb.blue * 255));
  }
  return StartLoadMaskDIB(std::move(mask));
}

CPDF_DIB::LoadState CPDF_DIB::ContinueLoadMaskDIB(PauseIndicatorIface* pPause) {
  if (!mask_) {
    return LoadState::kSuccess;
  }

  LoadState ret = mask_->ContinueLoadDIBBase(pPause);
  if (ret == LoadState::kContinue) {
    return LoadState::kContinue;
  }

  if (color_space_ && std_cs_) {
    color_space_->EnableStdConversion(false);
  }

  if (ret == LoadState::kFail) {
    mask_.Reset();
    return LoadState::kFail;
  }
  return LoadState::kSuccess;
}

RetainPtr<CPDF_DIB> CPDF_DIB::DetachMask() {
  return std::move(mask_);
}

bool CPDF_DIB::IsJBigImage() const {
  return stream_acc_->GetImageDecoder() == "JBIG2Decode";
}

CPDF_DIB::LoadState CPDF_DIB::StartLoadMaskDIB(
    RetainPtr<const CPDF_Stream> mask_stream) {
  mask_ = pdfium::MakeRetain<CPDF_DIB>(document_, std::move(mask_stream));
  LoadState ret =
      mask_->StartLoadDIBBase(false, nullptr, nullptr, true,
                              CPDF_ColorSpace::Family::kUnknown, false, {0, 0});
  if (ret == LoadState::kContinue) {
    if (status_ == LoadState::kFail) {
      status_ = LoadState::kContinue;
    }
    return LoadState::kContinue;
  }
  if (ret == LoadState::kFail) {
    mask_.Reset();
  }
  return LoadState::kSuccess;
}

void CPDF_DIB::LoadPalette() {
  if (!color_space_ || family_ == CPDF_ColorSpace::Family::kPattern) {
    return;
  }

  if (bpc_ == 0) {
    return;
  }

  // Use FX_SAFE_UINT32 just to be on the safe side, in case |bpc_| or
  // |components_| somehow gets a bad value.
  FX_SAFE_UINT32 safe_bits = bpc_;
  safe_bits *= components_;
  uint32_t bits = safe_bits.ValueOrDefault(255);
  if (bits > 8) {
    return;
  }

  if (bits == 1) {
    if (default_decode_ && (family_ == CPDF_ColorSpace::Family::kDeviceGray ||
                            family_ == CPDF_ColorSpace::Family::kDeviceRGB)) {
      return;
    }
    if (color_space_->ComponentCount() > 3) {
      return;
    }
    float color_values[3];
    std::fill(std::begin(color_values), std::end(color_values),
              comp_data_[0].decode_min_);

    auto rgb = color_space_->GetRGBOrZerosOnError(color_values);
    FX_ARGB argb0 =
        ArgbEncode(255, FXSYS_roundf(rgb.red * 255),
                   FXSYS_roundf(rgb.green * 255), FXSYS_roundf(rgb.blue * 255));
    FX_ARGB argb1;
    const CPDF_IndexedCS* indexed_cs = color_space_->AsIndexedCS();
    if (indexed_cs && indexed_cs->GetMaxIndex() == 0) {
      // If an indexed color space's hival value is 0, only 1 color is specified
      // in the lookup table. Another color should be set to 0xFF000000 by
      // default to set the range of the color space.
      argb1 = 0xFF000000;
    } else {
      color_values[0] += comp_data_[0].decode_step_;
      color_values[1] += comp_data_[0].decode_step_;
      color_values[2] += comp_data_[0].decode_step_;
      auto result = color_space_->GetRGBOrZerosOnError(color_values);
      argb1 = ArgbEncode(255, FXSYS_roundf(result.red * 255),
                         FXSYS_roundf(result.green * 255),
                         FXSYS_roundf(result.blue * 255));
    }

    if (argb0 != 0xFF000000 || argb1 != 0xFFFFFFFF) {
      SetPaletteArgb(0, argb0);
      SetPaletteArgb(1, argb1);
    }
    return;
  }
  if (bpc_ == 8 && default_decode_ &&
      color_space_ ==
          CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceGray)) {
    return;
  }

  int palette_count = 1 << bits;
  // Using at least 16 elements due to the call color_space_->GetRGB().
  std::vector<float> color_values(std::max(components_, 16u));
  for (int i = 0; i < palette_count; i++) {
    int color_data = i;
    for (uint32_t j = 0; j < components_; j++) {
      int encoded_component = color_data % (1 << bpc_);
      color_data /= 1 << bpc_;
      color_values[j] = comp_data_[j].decode_min_ +
                        comp_data_[j].decode_step_ * encoded_component;
    }
    FX_RGB_STRUCT<float> rgb;
    if (components_ == 1 && family_ == CPDF_ColorSpace::Family::kICCBased &&
        color_space_->ComponentCount() > 1) {
      const size_t nComponents = color_space_->ComponentCount();
      std::vector<float> temp_buf(nComponents, color_values[0]);
      rgb = color_space_->GetRGBOrZerosOnError(temp_buf);
    } else {
      rgb = color_space_->GetRGBOrZerosOnError(color_values);
    }
    SetPaletteArgb(i, ArgbEncode(255, FXSYS_roundf(rgb.red * 255),
                                 FXSYS_roundf(rgb.green * 255),
                                 FXSYS_roundf(rgb.blue * 255)));
  }
}

bool CPDF_DIB::ValidateDictParam(const ByteString& filter) {
  bpc_ = bpc_orig_;

  // Per spec, |bpc_| should always be 8 for RunLengthDecode, but too many
  // documents do not conform to it. So skip this check.

  if (filter == "JPXDecode") {
    do_bpc_check_ = false;
    return true;
  }

  if (filter == "CCITTFaxDecode" || filter == "JBIG2Decode") {
    bpc_ = 1;
    components_ = 1;
  } else if (filter == "DCTDecode") {
    bpc_ = 8;
  }

  if (!IsAllowedBitsPerComponent(bpc_)) {
    bpc_ = 0;
    return false;
  }
  return true;
}

void CPDF_DIB::TranslateScanline24bpp(
    pdfium::span<uint8_t> dest_scan,
    pdfium::span<const uint8_t> src_scan) const {
  if (bpc_ == 0) {
    return;
  }

  if (TranslateScanline24bppDefaultDecode(dest_scan, src_scan)) {
    return;
  }

  // Using at least 16 elements due to the call color_space_->GetRGB().
  std::vector<float> color_values(std::max(components_, 16u));
  FX_RGB_STRUCT<float> rgb = {};
  uint64_t src_bit_pos = 0;
  uint64_t src_byte_pos = 0;
  size_t dest_byte_pos = 0;
  const bool bpp8 = bpc_ == 8;
  for (int column = 0; column < GetWidth(); column++) {
    for (uint32_t color = 0; color < components_; color++) {
      if (bpp8) {
        uint8_t data = src_scan[src_byte_pos++];
        color_values[color] = comp_data_[color].decode_min_ +
                              comp_data_[color].decode_step_ * data;
      } else {
        unsigned int data = GetBits8(src_scan, src_bit_pos, bpc_);
        color_values[color] = comp_data_[color].decode_min_ +
                              comp_data_[color].decode_step_ * data;
        src_bit_pos += bpc_;
      }
    }
    if (TransMask()) {
      float k = 1.0f - color_values[3];
      rgb.red = (1.0f - color_values[0]) * k;
      rgb.green = (1.0f - color_values[1]) * k;
      rgb.blue = (1.0f - color_values[2]) * k;
    } else if (family_ != CPDF_ColorSpace::Family::kPattern) {
      rgb = color_space_->GetRGBOrZerosOnError(color_values);
    }
    const float R = std::clamp(rgb.red, 0.0f, 1.0f);
    const float G = std::clamp(rgb.green, 0.0f, 1.0f);
    const float B = std::clamp(rgb.blue, 0.0f, 1.0f);
    dest_scan[dest_byte_pos] = static_cast<uint8_t>(B * 255);
    dest_scan[dest_byte_pos + 1] = static_cast<uint8_t>(G * 255);
    dest_scan[dest_byte_pos + 2] = static_cast<uint8_t>(R * 255);
    dest_byte_pos += 3;
  }
}

bool CPDF_DIB::TranslateScanline24bppDefaultDecode(
    pdfium::span<uint8_t> dest_scan,
    pdfium::span<const uint8_t> src_scan) const {
  if (!default_decode_) {
    return false;
  }

  if (family_ != CPDF_ColorSpace::Family::kDeviceRGB &&
      family_ != CPDF_ColorSpace::Family::kCalRGB) {
    if (bpc_ != 8) {
      return false;
    }

    if (components_ == color_space_->ComponentCount()) {
      color_space_->TranslateImageLine(dest_scan, src_scan, GetWidth(),
                                       GetWidth(), GetHeight(), TransMask());
    }
    return true;
  }

  if (components_ != 3) {
    return true;
  }

  uint8_t* dest_pos = dest_scan.data();
  const uint8_t* src_pos = src_scan.data();
  switch (bpc_) {
    case 8:
      UNSAFE_TODO({
        for (int column = 0; column < GetWidth(); column++) {
          *dest_pos++ = src_pos[2];
          *dest_pos++ = src_pos[1];
          *dest_pos++ = *src_pos;
          src_pos += 3;
        }
      });
      break;
    case 16:
      UNSAFE_TODO({
        for (int col = 0; col < GetWidth(); col++) {
          *dest_pos++ = src_pos[4];
          *dest_pos++ = src_pos[2];
          *dest_pos++ = *src_pos;
          src_pos += 6;
        }
      });
      break;
    default:
      const unsigned int max_data = (1 << bpc_) - 1;
      uint64_t src_bit_pos = 0;
      size_t dest_byte_pos = 0;
      for (int column = 0; column < GetWidth(); column++) {
        unsigned int R = GetBits8(src_scan, src_bit_pos, bpc_);
        src_bit_pos += bpc_;
        unsigned int G = GetBits8(src_scan, src_bit_pos, bpc_);
        src_bit_pos += bpc_;
        unsigned int B = GetBits8(src_scan, src_bit_pos, bpc_);
        src_bit_pos += bpc_;
        R = std::min(R, max_data);
        G = std::min(G, max_data);
        B = std::min(B, max_data);
        UNSAFE_TODO({
          dest_pos[dest_byte_pos] = B * 255 / max_data;
          dest_pos[dest_byte_pos + 1] = G * 255 / max_data;
          dest_pos[dest_byte_pos + 2] = R * 255 / max_data;
          dest_byte_pos += 3;
        });
      }
      break;
  }
  return true;
}

pdfium::span<const uint8_t> CPDF_DIB::GetScanline(int line) const {
  if (bpc_ == 0) {
    return pdfium::span<const uint8_t>();
  }

  const std::optional<uint32_t> src_pitch =
      fxge::CalculatePitch8(bpc_, components_, GetWidth());
  if (!src_pitch.has_value()) {
    return pdfium::span<const uint8_t>();
  }

  uint32_t src_pitch_value = src_pitch.value();
  // This is used as the buffer of `pSrcLine` when the stream is truncated,
  // and the remaining bytes count is less than `src_pitch_value`
  DataVector<uint8_t> temp_buffer;
  pdfium::span<const uint8_t> pSrcLine;

  if (cached_bitmap_ && src_pitch_value <= cached_bitmap_->GetPitch()) {
    if (line >= cached_bitmap_->GetHeight()) {
      line = cached_bitmap_->GetHeight() - 1;
    }
    pSrcLine = cached_bitmap_->GetScanline(line);
  } else if (decoder_) {
    pSrcLine = decoder_->GetScanline(line);
  } else if (stream_acc_->GetSize() > line * src_pitch_value) {
    pdfium::span<const uint8_t> remaining_bytes =
        stream_acc_->GetSpan().subspan(line * src_pitch_value);
    if (remaining_bytes.size() >= src_pitch_value) {
      pSrcLine = remaining_bytes.first(src_pitch_value);
    } else {
      temp_buffer = DataVector<uint8_t>(src_pitch_value);
      fxcrt::Copy(remaining_bytes, temp_buffer);
      pSrcLine = temp_buffer;
    }
  }

  if (pSrcLine.empty()) {
    pdfium::span<uint8_t> result = !mask_buf_.empty() ? mask_buf_ : line_buf_;
    std::ranges::fill(result, 0);
    return result;
  }
  if (bpc_ * components_ == 1) {
    if (image_mask_ && default_decode_) {
      for (uint32_t i = 0; i < src_pitch_value; i++) {
        // TODO(tsepez): Bounds check if cost is acceptable.
        UNSAFE_TODO(line_buf_[i] = ~pSrcLine.data()[i]);
      }
      return pdfium::span(line_buf_).first(src_pitch_value);
    }
    if (!color_key_) {
      fxcrt::Copy(pSrcLine.first(src_pitch_value), line_buf_);
      return pdfium::span(line_buf_).first(src_pitch_value);
    }
    uint32_t reset_argb = Get1BitResetValue();
    uint32_t set_argb = Get1BitSetValue();
    auto mask32_span =
        fxcrt::reinterpret_span<uint32_t>(pdfium::span(mask_buf_));
    for (int col = 0; col < GetWidth(); col++) {
      mask32_span[col] = GetBitValue(pSrcLine, col) ? set_argb : reset_argb;
    }
    return fxcrt::reinterpret_span<uint8_t>(
        mask32_span.first(static_cast<size_t>(GetWidth())));
  }
  if (bpc_ * components_ <= 8) {
    pdfium::span<uint8_t> result = line_buf_;
    if (bpc_ == 8) {
      fxcrt::Copy(pSrcLine.first(src_pitch_value), result);
      result = result.first(src_pitch_value);
    } else {
      uint64_t src_bit_pos = 0;
      for (int col = 0; col < GetWidth(); col++) {
        unsigned int color_index = 0;
        for (uint32_t color = 0; color < components_; color++) {
          unsigned int data = GetBits8(pSrcLine, src_bit_pos, bpc_);
          color_index |= data << (color * bpc_);
          src_bit_pos += bpc_;
        }
        line_buf_[col] = color_index;
      }
      result = result.first(static_cast<size_t>(GetWidth()));
    }
    if (!color_key_) {
      return result;
    }

    uint8_t* pDestPixel = mask_buf_.data();
    const uint8_t* pSrcPixel = line_buf_.data();
    pdfium::span<const uint32_t> palette = GetPaletteSpan();
    UNSAFE_TODO({
      if (HasPalette()) {
        for (int col = 0; col < GetWidth(); col++) {
          uint8_t index = *pSrcPixel++;
          *pDestPixel++ = FXARGB_B(palette[index]);
          *pDestPixel++ = FXARGB_G(palette[index]);
          *pDestPixel++ = FXARGB_R(palette[index]);
          *pDestPixel++ =
              IsColorIndexOutOfBounds(index, comp_data_[0]) ? 0xFF : 0;
        }
      } else {
        for (int col = 0; col < GetWidth(); col++) {
          uint8_t index = *pSrcPixel++;
          *pDestPixel++ = index;
          *pDestPixel++ = index;
          *pDestPixel++ = index;
          *pDestPixel++ =
              IsColorIndexOutOfBounds(index, comp_data_[0]) ? 0xFF : 0;
        }
      }
    });
    return pdfium::span(mask_buf_).first(static_cast<size_t>(4 * GetWidth()));
  }
  if (color_key_) {
    if (components_ == 3 && bpc_ == 8) {
      UNSAFE_TODO({
        uint8_t* alpha_channel = mask_buf_.data() + 3;
        for (int col = 0; col < GetWidth(); col++) {
          const auto pPixel =
              pSrcLine.subspan(static_cast<size_t>(col * 3), 3u);
          alpha_channel[col * 4] =
              AreColorIndicesOutOfBounds(pPixel, comp_data_) ? 0xFF : 0;
        }
      });
    } else {
      std::ranges::fill(mask_buf_, 0xFF);
    }
  }
  if (color_space_) {
    TranslateScanline24bpp(line_buf_, pSrcLine);
    src_pitch_value = 3 * GetWidth();
    pSrcLine = pdfium::span(line_buf_).first(src_pitch_value);
  }
  if (!color_key_) {
    return pSrcLine;
  }

  // TODO(tsepez): Bounds check if cost is acceptable.
  const uint8_t* pSrcPixel = pSrcLine.data();
  uint8_t* pDestPixel = mask_buf_.data();
  UNSAFE_TODO({
    for (int col = 0; col < GetWidth(); col++) {
      *pDestPixel++ = *pSrcPixel++;
      *pDestPixel++ = *pSrcPixel++;
      *pDestPixel++ = *pSrcPixel++;
      pDestPixel++;
    }
  });
  return pdfium::span(mask_buf_).first(static_cast<size_t>(4 * GetWidth()));
}

bool CPDF_DIB::SkipToScanline(int line, PauseIndicatorIface* pPause) const {
  return decoder_ && decoder_->SkipToScanline(line, pPause);
}

size_t CPDF_DIB::GetEstimatedImageMemoryBurden() const {
  return cached_bitmap_ ? cached_bitmap_->GetEstimatedImageMemoryBurden() : 0;
}

bool CPDF_DIB::TransMask() const {
  return load_mask_ && group_family_ == CPDF_ColorSpace::Family::kDeviceCMYK &&
         family_ == CPDF_ColorSpace::Family::kDeviceCMYK;
}

void CPDF_DIB::SetMaskProperties() {
  bpc_ = 1;
  components_ = 1;
  SetFormat(FXDIB_Format::k1bppMask);
}

uint32_t CPDF_DIB::Get1BitSetValue() const {
  if (comp_data_[0].color_key_max_ == 1) {
    return 0x00000000;
  }
  return HasPalette() ? GetPaletteSpan()[1] : 0xFFFFFFFF;
}

uint32_t CPDF_DIB::Get1BitResetValue() const {
  if (comp_data_[0].color_key_min_ == 0) {
    return 0x00000000;
  }
  return HasPalette() ? GetPaletteSpan()[0] : 0xFF000000;
}
