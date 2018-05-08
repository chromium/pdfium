// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <memory>
#include <vector>

#include "core/fxcodec/codec/ccodec_iccmodule.h"
#include "core/fxcodec/codec/codec_int.h"

namespace {

bool Check3Components(cmsColorSpaceSignature cs, bool bDst) {
  switch (cs) {
    case cmsSigGrayData:
      return false;
    case cmsSigCmykData:
      if (bDst)
        return false;
      break;
    case cmsSigLabData:
    case cmsSigRgbData:
    default:
      break;
  }
  return true;
}

}  // namespace

CLcmsCmm::CLcmsCmm(int srcComponents, cmsHTRANSFORM hTransform, bool isLab)
    : m_hTransform(hTransform),
      m_nSrcComponents(srcComponents),
      m_bLab(isLab) {}

CLcmsCmm::~CLcmsCmm() {
  cmsDeleteTransform(m_hTransform);
}

CCodec_IccModule::CCodec_IccModule() : m_nComponents(0) {}

CCodec_IccModule::~CCodec_IccModule() {}

std::unique_ptr<CLcmsCmm> CCodec_IccModule::CreateTransform_sRGB(
    const unsigned char* pSrcProfileData,
    uint32_t dwSrcProfileSize,
    uint32_t* nSrcComponents) {
  *nSrcComponents = 0;
  cmsHPROFILE srcProfile =
      cmsOpenProfileFromMem(pSrcProfileData, dwSrcProfileSize);
  if (!srcProfile)
    return nullptr;

  cmsHPROFILE dstProfile;
  dstProfile = cmsCreate_sRGBProfile();
  if (!dstProfile) {
    cmsCloseProfile(srcProfile);
    return nullptr;
  }
  int srcFormat;
  bool bLab = false;
  cmsColorSpaceSignature srcCS = cmsGetColorSpace(srcProfile);

  *nSrcComponents = cmsChannelsOf(srcCS);
  // According to PDF spec, number of components must be 1, 3, or 4.
  if (*nSrcComponents != 1 && *nSrcComponents != 3 && *nSrcComponents != 4) {
    cmsCloseProfile(srcProfile);
    cmsCloseProfile(dstProfile);
    return nullptr;
  }

  if (srcCS == cmsSigLabData) {
    srcFormat =
        COLORSPACE_SH(PT_Lab) | CHANNELS_SH(*nSrcComponents) | BYTES_SH(0);
    bLab = true;
  } else {
    srcFormat =
        COLORSPACE_SH(PT_ANY) | CHANNELS_SH(*nSrcComponents) | BYTES_SH(1);
  }
  cmsColorSpaceSignature dstCS = cmsGetColorSpace(dstProfile);
  if (!Check3Components(dstCS, true)) {
    cmsCloseProfile(srcProfile);
    cmsCloseProfile(dstProfile);
    return nullptr;
  }

  cmsHTRANSFORM hTransform = nullptr;
  int intent = 0;
  switch (dstCS) {
    case cmsSigGrayData:
      hTransform = cmsCreateTransform(srcProfile, srcFormat, dstProfile,
                                      TYPE_GRAY_8, intent, 0);
      break;
    case cmsSigRgbData:
      hTransform = cmsCreateTransform(srcProfile, srcFormat, dstProfile,
                                      TYPE_BGR_8, intent, 0);
      break;
    case cmsSigCmykData:
      hTransform = cmsCreateTransform(srcProfile, srcFormat, dstProfile,
                                      TYPE_CMYK_8, intent, 0);
      break;
    default:
      break;
  }
  if (!hTransform) {
    cmsCloseProfile(srcProfile);
    cmsCloseProfile(dstProfile);
    return nullptr;
  }
  auto pCmm = pdfium::MakeUnique<CLcmsCmm>(*nSrcComponents, hTransform, bLab);
  cmsCloseProfile(srcProfile);
  cmsCloseProfile(dstProfile);
  return pCmm;
}

void CCodec_IccModule::Translate(CLcmsCmm* pTransform,
                                 const float* pSrcValues,
                                 float* pDestValues) {
  if (!pTransform)
    return;

  uint32_t nSrcComponents = m_nComponents;
  uint8_t output[4];
  if (pTransform->m_bLab) {
    std::vector<double> input(pSrcValues, pSrcValues + nSrcComponents);
    cmsDoTransform(pTransform->m_hTransform, input.data(), output, 1);
  } else {
    std::vector<uint8_t> input;
    input.reserve(nSrcComponents);
    for (uint32_t i = 0; i < nSrcComponents; ++i) {
      input.push_back(
          pdfium::clamp(static_cast<int>(pSrcValues[i] * 255.0f), 0, 255));
    }
    cmsDoTransform(pTransform->m_hTransform, input.data(), output, 1);
  }
  pDestValues[0] = output[2] / 255.0f;
  pDestValues[1] = output[1] / 255.0f;
  pDestValues[2] = output[0] / 255.0f;
}

void CCodec_IccModule::TranslateScanline(CLcmsCmm* pTransform,
                                         unsigned char* pDest,
                                         const unsigned char* pSrc,
                                         int32_t pixels) {
  if (pTransform)
    cmsDoTransform(pTransform->m_hTransform, pSrc, pDest, pixels);
}
