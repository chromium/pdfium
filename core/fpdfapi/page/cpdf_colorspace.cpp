// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_colorspace.h"

#include <math.h>
#include <stdint.h>

#include <algorithm>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "core/fpdfapi/page/cpdf_devicecs.h"
#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/page/cpdf_function.h"
#include "core/fpdfapi/page/cpdf_iccprofile.h"
#include "core/fpdfapi/page/cpdf_indexedcs.h"
#include "core/fpdfapi/page/cpdf_pattern.h"
#include "core/fpdfapi/page/cpdf_patterncs.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "core/fxcodec/fx_codec.h"
#include "core/fxcodec/icc/icc_transform.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_2d_size.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/maybe_owned.h"
#include "core/fxcrt/scoped_set_insertion.h"
#include "core/fxcrt/span_util.h"
#include "core/fxcrt/stl_util.h"
#include "third_party/base/check.h"
#include "third_party/base/check_op.h"
#include "third_party/base/containers/contains.h"
#include "third_party/base/notreached.h"

namespace {

constexpr uint8_t kSRGBSamples1[] = {
    0,   3,   6,   10,  13,  15,  18,  20,  22,  23,  25,  27,  28,  30,  31,
    32,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
    48,  49,  49,  50,  51,  52,  53,  53,  54,  55,  56,  56,  57,  58,  58,
    59,  60,  61,  61,  62,  62,  63,  64,  64,  65,  66,  66,  67,  67,  68,
    68,  69,  70,  70,  71,  71,  72,  72,  73,  73,  74,  74,  75,  76,  76,
    77,  77,  78,  78,  79,  79,  79,  80,  80,  81,  81,  82,  82,  83,  83,
    84,  84,  85,  85,  85,  86,  86,  87,  87,  88,  88,  88,  89,  89,  90,
    90,  91,  91,  91,  92,  92,  93,  93,  93,  94,  94,  95,  95,  95,  96,
    96,  97,  97,  97,  98,  98,  98,  99,  99,  99,  100, 100, 101, 101, 101,
    102, 102, 102, 103, 103, 103, 104, 104, 104, 105, 105, 106, 106, 106, 107,
    107, 107, 108, 108, 108, 109, 109, 109, 110, 110, 110, 110, 111, 111, 111,
    112, 112, 112, 113, 113, 113, 114, 114, 114, 115, 115, 115, 115, 116, 116,
    116, 117, 117, 117, 118, 118, 118, 118, 119, 119, 119, 120,
};

constexpr uint8_t kSRGBSamples2[] = {
    120, 121, 122, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135,
    136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 148, 149,
    150, 151, 152, 153, 154, 155, 155, 156, 157, 158, 159, 159, 160, 161, 162,
    163, 163, 164, 165, 166, 167, 167, 168, 169, 170, 170, 171, 172, 173, 173,
    174, 175, 175, 176, 177, 178, 178, 179, 180, 180, 181, 182, 182, 183, 184,
    185, 185, 186, 187, 187, 188, 189, 189, 190, 190, 191, 192, 192, 193, 194,
    194, 195, 196, 196, 197, 197, 198, 199, 199, 200, 200, 201, 202, 202, 203,
    203, 204, 205, 205, 206, 206, 207, 208, 208, 209, 209, 210, 210, 211, 212,
    212, 213, 213, 214, 214, 215, 215, 216, 216, 217, 218, 218, 219, 219, 220,
    220, 221, 221, 222, 222, 223, 223, 224, 224, 225, 226, 226, 227, 227, 228,
    228, 229, 229, 230, 230, 231, 231, 232, 232, 233, 233, 234, 234, 235, 235,
    236, 236, 237, 237, 238, 238, 238, 239, 239, 240, 240, 241, 241, 242, 242,
    243, 243, 244, 244, 245, 245, 246, 246, 246, 247, 247, 248, 248, 249, 249,
    250, 250, 251, 251, 251, 252, 252, 253, 253, 254, 254, 255, 255,
};

constexpr size_t kBlackWhitePointCount = 3;

void GetDefaultBlackPoint(float* pPoints) {
  static constexpr float kDefaultValue = 0.0f;
  for (size_t i = 0; i < kBlackWhitePointCount; ++i)
    pPoints[i] = kDefaultValue;
}

void GetBlackPoint(const CPDF_Dictionary* pDict, float* pPoints) {
  RetainPtr<const CPDF_Array> pParam = pDict->GetArrayFor("BlackPoint");
  if (!pParam || pParam->size() != kBlackWhitePointCount) {
    GetDefaultBlackPoint(pPoints);
    return;
  }

  // Check to make sure all values are non-negative.
  for (size_t i = 0; i < kBlackWhitePointCount; ++i) {
    pPoints[i] = pParam->GetFloatAt(i);
    if (pPoints[i] < 0) {
      GetDefaultBlackPoint(pPoints);
      return;
    }
  }
}

bool GetWhitePoint(const CPDF_Dictionary* pDict, float* pPoints) {
  RetainPtr<const CPDF_Array> pParam = pDict->GetArrayFor("WhitePoint");
  if (!pParam || pParam->size() != kBlackWhitePointCount)
    return false;

  for (size_t i = 0; i < kBlackWhitePointCount; ++i)
    pPoints[i] = pParam->GetFloatAt(i);
  return pPoints[0] > 0.0f && pPoints[1] == 1.0f && pPoints[2] > 0.0f;
}

class CPDF_CalGray final : public CPDF_ColorSpace {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;
  ~CPDF_CalGray() override;

  // CPDF_ColorSpace:
  bool GetRGB(pdfium::span<const float> pBuf,
              float* R,
              float* G,
              float* B) const override;
  uint32_t v_Load(CPDF_Document* pDoc,
                  const CPDF_Array* pArray,
                  std::set<const CPDF_Object*>* pVisited) override;
  void TranslateImageLine(pdfium::span<uint8_t> dest_span,
                          pdfium::span<const uint8_t> src_span,
                          int pixels,
                          int image_width,
                          int image_height,
                          bool bTransMask) const override;

 private:
  static constexpr float kDefaultGamma = 1.0f;

  CPDF_CalGray();

  float m_Gamma = kDefaultGamma;
  float m_WhitePoint[kBlackWhitePointCount] = {1.0f, 1.0f, 1.0f};
  float m_BlackPoint[kBlackWhitePointCount] = {0.0f, 0.0f, 0.0f};
};

class CPDF_CalRGB final : public CPDF_ColorSpace {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;
  ~CPDF_CalRGB() override;

  // CPDF_ColorSpace:
  bool GetRGB(pdfium::span<const float> pBuf,
              float* R,
              float* G,
              float* B) const override;
  void TranslateImageLine(pdfium::span<uint8_t> dest_span,
                          pdfium::span<const uint8_t> src_span,
                          int pixels,
                          int image_width,
                          int image_height,
                          bool bTransMask) const override;
  uint32_t v_Load(CPDF_Document* pDoc,
                  const CPDF_Array* pArray,
                  std::set<const CPDF_Object*>* pVisited) override;

 private:
  static constexpr size_t kGammaCount = 3;
  static constexpr size_t kMatrixCount = 9;

  CPDF_CalRGB();

  float m_WhitePoint[kBlackWhitePointCount] = {1.0f, 1.0f, 1.0f};
  float m_BlackPoint[kBlackWhitePointCount] = {0.0f, 0.0f, 0.0f};
  float m_Gamma[kGammaCount] = {};
  float m_Matrix[kMatrixCount] = {};
  bool m_bHasGamma = false;
  bool m_bHasMatrix = false;
};

class CPDF_LabCS final : public CPDF_ColorSpace {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;
  ~CPDF_LabCS() override;

  // CPDF_ColorSpace:
  bool GetRGB(pdfium::span<const float> pBuf,
              float* R,
              float* G,
              float* B) const override;
  void GetDefaultValue(int iComponent,
                       float* value,
                       float* min,
                       float* max) const override;
  void TranslateImageLine(pdfium::span<uint8_t> dest_span,
                          pdfium::span<const uint8_t> src_span,
                          int pixels,
                          int image_width,
                          int image_height,
                          bool bTransMask) const override;
  uint32_t v_Load(CPDF_Document* pDoc,
                  const CPDF_Array* pArray,
                  std::set<const CPDF_Object*>* pVisited) override;

 private:
  static constexpr size_t kRangesCount = 4;

  CPDF_LabCS();

  float m_WhitePoint[kBlackWhitePointCount] = {1.0f, 1.0f, 1.0f};
  float m_BlackPoint[kBlackWhitePointCount] = {0.0f, 0.0f, 0.0f};
  float m_Ranges[kRangesCount] = {};
};

class CPDF_ICCBasedCS final : public CPDF_BasedCS {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;
  ~CPDF_ICCBasedCS() override;

  // CPDF_ColorSpace:
  bool GetRGB(pdfium::span<const float> pBuf,
              float* R,
              float* G,
              float* B) const override;
  void TranslateImageLine(pdfium::span<uint8_t> dest_span,
                          pdfium::span<const uint8_t> src_span,
                          int pixels,
                          int image_width,
                          int image_height,
                          bool bTransMask) const override;
  bool IsNormal() const override;
  uint32_t v_Load(CPDF_Document* pDoc,
                  const CPDF_Array* pArray,
                  std::set<const CPDF_Object*>* pVisited) override;

 private:
  CPDF_ICCBasedCS();

  // If no valid ICC profile or using sRGB, try looking for an alternate.
  bool FindAlternateProfile(CPDF_Document* pDoc,
                            const CPDF_Dictionary* pDict,
                            std::set<const CPDF_Object*>* pVisited,
                            uint32_t nExpectedComponents);
  static RetainPtr<CPDF_ColorSpace> GetStockAlternateProfile(
      uint32_t nComponents);
  static std::vector<float> GetRanges(const CPDF_Dictionary* pDict,
                                      uint32_t nComponents);

  RetainPtr<CPDF_IccProfile> m_pProfile;
  mutable DataVector<uint8_t> m_pCache;
  std::vector<float> m_pRanges;
};

class CPDF_SeparationCS final : public CPDF_BasedCS {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;
  ~CPDF_SeparationCS() override;

  // CPDF_ColorSpace:
  bool GetRGB(pdfium::span<const float> pBuf,
              float* R,
              float* G,
              float* B) const override;
  void GetDefaultValue(int iComponent,
                       float* value,
                       float* min,
                       float* max) const override;
  uint32_t v_Load(CPDF_Document* pDoc,
                  const CPDF_Array* pArray,
                  std::set<const CPDF_Object*>* pVisited) override;

 private:
  CPDF_SeparationCS();

  bool m_IsNoneType = false;
  std::unique_ptr<const CPDF_Function> m_pFunc;
};

class CPDF_DeviceNCS final : public CPDF_BasedCS {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;
  ~CPDF_DeviceNCS() override;

  // CPDF_ColorSpace:
  bool GetRGB(pdfium::span<const float> pBuf,
              float* R,
              float* G,
              float* B) const override;
  void GetDefaultValue(int iComponent,
                       float* value,
                       float* min,
                       float* max) const override;
  uint32_t v_Load(CPDF_Document* pDoc,
                  const CPDF_Array* pArray,
                  std::set<const CPDF_Object*>* pVisited) override;

 private:
  CPDF_DeviceNCS();

  std::unique_ptr<const CPDF_Function> m_pFunc;
};

class Vector_3by1 {
 public:
  Vector_3by1() : a(0.0f), b(0.0f), c(0.0f) {}

  Vector_3by1(float a1, float b1, float c1) : a(a1), b(b1), c(c1) {}

  float a;
  float b;
  float c;
};

class Matrix_3by3 {
 public:
  Matrix_3by3()
      : a(0.0f),
        b(0.0f),
        c(0.0f),
        d(0.0f),
        e(0.0f),
        f(0.0f),
        g(0.0f),
        h(0.0f),
        i(0.0f) {}

  Matrix_3by3(float a1,
              float b1,
              float c1,
              float d1,
              float e1,
              float f1,
              float g1,
              float h1,
              float i1)
      : a(a1), b(b1), c(c1), d(d1), e(e1), f(f1), g(g1), h(h1), i(i1) {}

  Matrix_3by3 Inverse() {
    float det = a * (e * i - f * h) - b * (i * d - f * g) + c * (d * h - e * g);
    if (fabs(det) < std::numeric_limits<float>::epsilon())
      return Matrix_3by3();

    return Matrix_3by3(
        (e * i - f * h) / det, -(b * i - c * h) / det, (b * f - c * e) / det,
        -(d * i - f * g) / det, (a * i - c * g) / det, -(a * f - c * d) / det,
        (d * h - e * g) / det, -(a * h - b * g) / det, (a * e - b * d) / det);
  }

  Matrix_3by3 Multiply(const Matrix_3by3& m) {
    return Matrix_3by3(a * m.a + b * m.d + c * m.g, a * m.b + b * m.e + c * m.h,
                       a * m.c + b * m.f + c * m.i, d * m.a + e * m.d + f * m.g,
                       d * m.b + e * m.e + f * m.h, d * m.c + e * m.f + f * m.i,
                       g * m.a + h * m.d + i * m.g, g * m.b + h * m.e + i * m.h,
                       g * m.c + h * m.f + i * m.i);
  }

  Vector_3by1 TransformVector(const Vector_3by1& v) {
    return Vector_3by1(a * v.a + b * v.b + c * v.c, d * v.a + e * v.b + f * v.c,
                       g * v.a + h * v.b + i * v.c);
  }

  float a;
  float b;
  float c;
  float d;
  float e;
  float f;
  float g;
  float h;
  float i;
};

float RGB_Conversion(float colorComponent) {
  colorComponent = std::clamp(colorComponent, 0.0f, 1.0f);
  int scale = std::max(static_cast<int>(colorComponent * 1023), 0);
  if (scale < 192)
    return kSRGBSamples1[scale] / 255.0f;
  return kSRGBSamples2[scale / 4 - 48] / 255.0f;
}

void XYZ_to_sRGB(float X, float Y, float Z, float* R, float* G, float* B) {
  float R1 = 3.2410f * X - 1.5374f * Y - 0.4986f * Z;
  float G1 = -0.9692f * X + 1.8760f * Y + 0.0416f * Z;
  float B1 = 0.0556f * X - 0.2040f * Y + 1.0570f * Z;

  *R = RGB_Conversion(R1);
  *G = RGB_Conversion(G1);
  *B = RGB_Conversion(B1);
}

void XYZ_to_sRGB_WhitePoint(float X,
                            float Y,
                            float Z,
                            float Xw,
                            float Yw,
                            float Zw,
                            float* R,
                            float* G,
                            float* B) {
  // The following RGB_xyz is based on
  // sRGB value {Rx,Ry}={0.64, 0.33}, {Gx,Gy}={0.30, 0.60}, {Bx,By}={0.15, 0.06}

  constexpr float Rx = 0.64f;
  constexpr float Ry = 0.33f;
  constexpr float Gx = 0.30f;
  constexpr float Gy = 0.60f;
  constexpr float Bx = 0.15f;
  constexpr float By = 0.06f;
  Matrix_3by3 RGB_xyz(Rx, Gx, Bx, Ry, Gy, By, 1 - Rx - Ry, 1 - Gx - Gy,
                      1 - Bx - By);
  Vector_3by1 whitePoint(Xw, Yw, Zw);
  Vector_3by1 XYZ(X, Y, Z);

  Vector_3by1 RGB_Sum_XYZ = RGB_xyz.Inverse().TransformVector(whitePoint);
  Matrix_3by3 RGB_SUM_XYZ_DIAG(RGB_Sum_XYZ.a, 0, 0, 0, RGB_Sum_XYZ.b, 0, 0, 0,
                               RGB_Sum_XYZ.c);
  Matrix_3by3 M = RGB_xyz.Multiply(RGB_SUM_XYZ_DIAG);
  Vector_3by1 RGB = M.Inverse().TransformVector(XYZ);

  *R = RGB_Conversion(RGB.a);
  *G = RGB_Conversion(RGB.b);
  *B = RGB_Conversion(RGB.c);
}

class StockColorSpaces {
 public:
  StockColorSpaces()
      : gray_(pdfium::MakeRetain<CPDF_DeviceCS>(
            CPDF_ColorSpace::Family::kDeviceGray)),
        rgb_(pdfium::MakeRetain<CPDF_DeviceCS>(
            CPDF_ColorSpace::Family::kDeviceRGB)),
        cmyk_(pdfium::MakeRetain<CPDF_DeviceCS>(
            CPDF_ColorSpace::Family::kDeviceCMYK)),
        pattern_(pdfium::MakeRetain<CPDF_PatternCS>()) {
    pattern_->InitializeStockPattern();
  }
  StockColorSpaces(const StockColorSpaces&) = delete;
  StockColorSpaces& operator=(const StockColorSpaces&) = delete;
  ~StockColorSpaces() = default;

  RetainPtr<CPDF_ColorSpace> GetStockCS(CPDF_ColorSpace::Family family) {
    if (family == CPDF_ColorSpace::Family::kDeviceGray) {
      return gray_;
    }
    if (family == CPDF_ColorSpace::Family::kDeviceRGB) {
      return rgb_;
    }
    if (family == CPDF_ColorSpace::Family::kDeviceCMYK) {
      return cmyk_;
    }
    if (family == CPDF_ColorSpace::Family::kPattern) {
      return pattern_;
    }
    NOTREACHED_NORETURN();
  }

 private:
  RetainPtr<CPDF_DeviceCS> gray_;
  RetainPtr<CPDF_DeviceCS> rgb_;
  RetainPtr<CPDF_DeviceCS> cmyk_;
  RetainPtr<CPDF_PatternCS> pattern_;
};

StockColorSpaces* g_stock_colorspaces = nullptr;

}  // namespace

PatternValue::PatternValue() = default;

PatternValue::PatternValue(const PatternValue& that) = default;

PatternValue::~PatternValue() = default;

void PatternValue::SetComps(pdfium::span<const float> comps) {
  fxcrt::spancpy(pdfium::make_span(m_Comps), comps);
}

// static
void CPDF_ColorSpace::InitializeGlobals() {
  CHECK(!g_stock_colorspaces);
  g_stock_colorspaces = new StockColorSpaces();
}

// static
void CPDF_ColorSpace::DestroyGlobals() {
  delete g_stock_colorspaces;
  g_stock_colorspaces = nullptr;
}

// static
RetainPtr<CPDF_ColorSpace> CPDF_ColorSpace::GetStockCS(Family family) {
  return g_stock_colorspaces->GetStockCS(family);
}

// static
RetainPtr<CPDF_ColorSpace> CPDF_ColorSpace::GetStockCSForName(
    const ByteString& name) {
  if (name == "DeviceRGB" || name == "RGB")
    return GetStockCS(Family::kDeviceRGB);
  if (name == "DeviceGray" || name == "G")
    return GetStockCS(Family::kDeviceGray);
  if (name == "DeviceCMYK" || name == "CMYK")
    return GetStockCS(Family::kDeviceCMYK);
  if (name == "Pattern")
    return GetStockCS(Family::kPattern);
  return nullptr;
}

// static
RetainPtr<CPDF_ColorSpace> CPDF_ColorSpace::Load(
    CPDF_Document* pDoc,
    const CPDF_Object* pObj,
    std::set<const CPDF_Object*>* pVisited) {
  if (!pObj)
    return nullptr;

  if (pdfium::Contains(*pVisited, pObj))
    return nullptr;

  ScopedSetInsertion<const CPDF_Object*> insertion(pVisited, pObj);

  if (pObj->IsName())
    return GetStockCSForName(pObj->GetString());

  if (const CPDF_Stream* pStream = pObj->AsStream()) {
    RetainPtr<const CPDF_Dictionary> pDict = pStream->GetDict();
    if (!pDict)
      return nullptr;

    CPDF_DictionaryLocker locker(std::move(pDict));
    for (const auto& it : locker) {
      RetainPtr<const CPDF_Name> pValue = ToName(it.second);
      if (pValue) {
        RetainPtr<CPDF_ColorSpace> pRet =
            GetStockCSForName(pValue->GetString());
        if (pRet)
          return pRet;
      }
    }
    return nullptr;
  }

  const CPDF_Array* pArray = pObj->AsArray();
  if (!pArray || pArray->IsEmpty())
    return nullptr;

  RetainPtr<const CPDF_Object> pFamilyObj = pArray->GetDirectObjectAt(0);
  if (!pFamilyObj)
    return nullptr;

  ByteString familyname = pFamilyObj->GetString();
  if (pArray->size() == 1)
    return GetStockCSForName(familyname);

  RetainPtr<CPDF_ColorSpace> pCS =
      CPDF_ColorSpace::AllocateColorSpace(familyname.AsStringView());
  if (!pCS)
    return nullptr;

  pCS->m_pArray.Reset(pArray);
  pCS->m_nComponents = pCS->v_Load(pDoc, pArray, pVisited);
  if (pCS->m_nComponents == 0)
    return nullptr;

  return pCS;
}

// static
RetainPtr<CPDF_ColorSpace> CPDF_ColorSpace::AllocateColorSpace(
    ByteStringView bsFamilyName) {
  switch (bsFamilyName.GetID()) {
    case FXBSTR_ID('C', 'a', 'l', 'G'):
      return pdfium::MakeRetain<CPDF_CalGray>();
    case FXBSTR_ID('C', 'a', 'l', 'R'):
      return pdfium::MakeRetain<CPDF_CalRGB>();
    case FXBSTR_ID('L', 'a', 'b', 0):
      return pdfium::MakeRetain<CPDF_LabCS>();
    case FXBSTR_ID('I', 'C', 'C', 'B'):
      return pdfium::MakeRetain<CPDF_ICCBasedCS>();
    case FXBSTR_ID('I', 'n', 'd', 'e'):
    case FXBSTR_ID('I', 0, 0, 0):
      return pdfium::MakeRetain<CPDF_IndexedCS>();
    case FXBSTR_ID('S', 'e', 'p', 'a'):
      return pdfium::MakeRetain<CPDF_SeparationCS>();
    case FXBSTR_ID('D', 'e', 'v', 'i'):
      return pdfium::MakeRetain<CPDF_DeviceNCS>();
    case FXBSTR_ID('P', 'a', 't', 't'):
      return pdfium::MakeRetain<CPDF_PatternCS>();
    default:
      return nullptr;
  }
}

// static
uint32_t CPDF_ColorSpace::ComponentsForFamily(Family family) {
  switch (family) {
    case Family::kDeviceGray:
      return 1;
    case Family::kDeviceRGB:
      return 3;
    case Family::kDeviceCMYK:
      return 4;
    default:
      NOTREACHED_NORETURN();
  }
}

std::vector<float> CPDF_ColorSpace::CreateBufAndSetDefaultColor() const {
  DCHECK(m_Family != Family::kPattern);

  float min;
  float max;
  std::vector<float> buf(m_nComponents);
  for (uint32_t i = 0; i < m_nComponents; i++)
    GetDefaultValue(i, &buf[i], &min, &max);

  return buf;
}

uint32_t CPDF_ColorSpace::CountComponents() const {
  return m_nComponents;
}

void CPDF_ColorSpace::GetDefaultValue(int iComponent,
                                      float* value,
                                      float* min,
                                      float* max) const {
  *value = 0.0f;
  *min = 0.0f;
  *max = 1.0f;
}

void CPDF_ColorSpace::TranslateImageLine(pdfium::span<uint8_t> dest_span,
                                         pdfium::span<const uint8_t> src_span,
                                         int pixels,
                                         int image_width,
                                         int image_height,
                                         bool bTransMask) const {
  uint8_t* dest_buf = dest_span.data();
  const uint8_t* src_buf = src_span.data();
  std::vector<float> src(m_nComponents);
  float R;
  float G;
  float B;
  const int divisor = m_Family != Family::kIndexed ? 255 : 1;
  for (int i = 0; i < pixels; i++) {
    for (uint32_t j = 0; j < m_nComponents; j++)
      src[j] = static_cast<float>(*src_buf++) / divisor;
    GetRGB(src, &R, &G, &B);
    *dest_buf++ = static_cast<int32_t>(B * 255);
    *dest_buf++ = static_cast<int32_t>(G * 255);
    *dest_buf++ = static_cast<int32_t>(R * 255);
  }
}

void CPDF_ColorSpace::EnableStdConversion(bool bEnabled) {
  if (bEnabled)
    m_dwStdConversion++;
  else if (m_dwStdConversion)
    m_dwStdConversion--;
}

bool CPDF_ColorSpace::IsNormal() const {
  return GetFamily() == Family::kDeviceGray ||
         GetFamily() == Family::kDeviceRGB ||
         GetFamily() == Family::kDeviceCMYK ||
         GetFamily() == Family::kCalGray || GetFamily() == Family::kCalRGB;
}

const CPDF_PatternCS* CPDF_ColorSpace::AsPatternCS() const {
  return nullptr;
}

const CPDF_IndexedCS* CPDF_ColorSpace::AsIndexedCS() const {
  return nullptr;
}

CPDF_ColorSpace::CPDF_ColorSpace(Family family) : m_Family(family) {}

CPDF_ColorSpace::~CPDF_ColorSpace() = default;

void CPDF_ColorSpace::SetComponentsForStockCS(uint32_t nComponents) {
  m_nComponents = nComponents;
}

CPDF_CalGray::CPDF_CalGray() : CPDF_ColorSpace(Family::kCalGray) {}

CPDF_CalGray::~CPDF_CalGray() = default;

uint32_t CPDF_CalGray::v_Load(CPDF_Document* pDoc,
                              const CPDF_Array* pArray,
                              std::set<const CPDF_Object*>* pVisited) {
  RetainPtr<const CPDF_Dictionary> pDict = pArray->GetDictAt(1);
  if (!pDict)
    return 0;

  if (!GetWhitePoint(pDict.Get(), m_WhitePoint))
    return 0;

  GetBlackPoint(pDict.Get(), m_BlackPoint);

  m_Gamma = pDict->GetFloatFor("Gamma");
  if (m_Gamma == 0)
    m_Gamma = kDefaultGamma;
  return 1;
}

bool CPDF_CalGray::GetRGB(pdfium::span<const float> pBuf,
                          float* R,
                          float* G,
                          float* B) const {
  *R = pBuf[0];
  *G = pBuf[0];
  *B = pBuf[0];
  return true;
}

void CPDF_CalGray::TranslateImageLine(pdfium::span<uint8_t> dest_span,
                                      pdfium::span<const uint8_t> src_span,
                                      int pixels,
                                      int image_width,
                                      int image_height,
                                      bool bTransMask) const {
  uint8_t* pDestBuf = dest_span.data();
  const uint8_t* pSrcBuf = src_span.data();
  for (int i = 0; i < pixels; i++) {
    // Compiler can not conclude that src/dest don't overlap.
    const uint8_t pix = pSrcBuf[i];
    *pDestBuf++ = pix;
    *pDestBuf++ = pix;
    *pDestBuf++ = pix;
  }
}

CPDF_CalRGB::CPDF_CalRGB() : CPDF_ColorSpace(Family::kCalRGB) {}

CPDF_CalRGB::~CPDF_CalRGB() = default;

uint32_t CPDF_CalRGB::v_Load(CPDF_Document* pDoc,
                             const CPDF_Array* pArray,
                             std::set<const CPDF_Object*>* pVisited) {
  RetainPtr<const CPDF_Dictionary> pDict = pArray->GetDictAt(1);
  if (!pDict)
    return 0;

  if (!GetWhitePoint(pDict.Get(), m_WhitePoint))
    return 0;

  GetBlackPoint(pDict.Get(), m_BlackPoint);

  RetainPtr<const CPDF_Array> pGamma = pDict->GetArrayFor("Gamma");
  if (pGamma) {
    m_bHasGamma = true;
    for (size_t i = 0; i < std::size(m_Gamma); ++i)
      m_Gamma[i] = pGamma->GetFloatAt(i);
  }

  RetainPtr<const CPDF_Array> pMatrix = pDict->GetArrayFor("Matrix");
  if (pMatrix) {
    m_bHasMatrix = true;
    for (size_t i = 0; i < std::size(m_Matrix); ++i)
      m_Matrix[i] = pMatrix->GetFloatAt(i);
  }
  return 3;
}

bool CPDF_CalRGB::GetRGB(pdfium::span<const float> pBuf,
                         float* R,
                         float* G,
                         float* B) const {
  float A_ = pBuf[0];
  float B_ = pBuf[1];
  float C_ = pBuf[2];
  if (m_bHasGamma) {
    A_ = powf(A_, m_Gamma[0]);
    B_ = powf(B_, m_Gamma[1]);
    C_ = powf(C_, m_Gamma[2]);
  }

  float X;
  float Y;
  float Z;
  if (m_bHasMatrix) {
    X = m_Matrix[0] * A_ + m_Matrix[3] * B_ + m_Matrix[6] * C_;
    Y = m_Matrix[1] * A_ + m_Matrix[4] * B_ + m_Matrix[7] * C_;
    Z = m_Matrix[2] * A_ + m_Matrix[5] * B_ + m_Matrix[8] * C_;
  } else {
    X = A_;
    Y = B_;
    Z = C_;
  }
  XYZ_to_sRGB_WhitePoint(X, Y, Z, m_WhitePoint[0], m_WhitePoint[1],
                         m_WhitePoint[2], R, G, B);
  return true;
}

void CPDF_CalRGB::TranslateImageLine(pdfium::span<uint8_t> dest_span,
                                     pdfium::span<const uint8_t> src_span,
                                     int pixels,
                                     int image_width,
                                     int image_height,
                                     bool bTransMask) const {
  uint8_t* pDestBuf = dest_span.data();
  const uint8_t* pSrcBuf = src_span.data();
  if (!bTransMask) {
    fxcodec::ReverseRGB(pDestBuf, pSrcBuf, pixels);
    return;
  }

  float Cal[3];
  float R;
  float G;
  float B;
  for (int i = 0; i < pixels; i++) {
    Cal[0] = static_cast<float>(pSrcBuf[2]) / 255;
    Cal[1] = static_cast<float>(pSrcBuf[1]) / 255;
    Cal[2] = static_cast<float>(pSrcBuf[0]) / 255;
    GetRGB(Cal, &R, &G, &B);
    pDestBuf[0] = FXSYS_roundf(B * 255);
    pDestBuf[1] = FXSYS_roundf(G * 255);
    pDestBuf[2] = FXSYS_roundf(R * 255);
    pSrcBuf += 3;
    pDestBuf += 3;
  }
}

CPDF_LabCS::CPDF_LabCS() : CPDF_ColorSpace(Family::kLab) {}

CPDF_LabCS::~CPDF_LabCS() = default;

void CPDF_LabCS::GetDefaultValue(int iComponent,
                                 float* value,
                                 float* min,
                                 float* max) const {
  DCHECK_LT(iComponent, 3);

  if (iComponent > 0) {
    float range_min = m_Ranges[iComponent * 2 - 2];
    float range_max = m_Ranges[iComponent * 2 - 1];
    if (range_min <= range_max) {
      *min = range_min;
      *max = range_max;
      *value = std::clamp(0.0f, *min, *max);
      return;
    }
  }

  *min = 0.0f;
  *max = 100.0f;
  *value = 0.0f;
}

uint32_t CPDF_LabCS::v_Load(CPDF_Document* pDoc,
                            const CPDF_Array* pArray,
                            std::set<const CPDF_Object*>* pVisited) {
  RetainPtr<const CPDF_Dictionary> pDict = pArray->GetDictAt(1);
  if (!pDict)
    return 0;

  if (!GetWhitePoint(pDict.Get(), m_WhitePoint))
    return 0;

  GetBlackPoint(pDict.Get(), m_BlackPoint);

  RetainPtr<const CPDF_Array> pParam = pDict->GetArrayFor("Range");
  static constexpr float kDefaultRanges[kRangesCount] = {-100.0f, 100.0f,
                                                         -100.0f, 100.0f};
  static_assert(std::size(kDefaultRanges) == std::extent<decltype(m_Ranges)>(),
                "Range size mismatch");
  for (size_t i = 0; i < std::size(kDefaultRanges); ++i)
    m_Ranges[i] = pParam ? pParam->GetFloatAt(i) : kDefaultRanges[i];
  return 3;
}

bool CPDF_LabCS::GetRGB(pdfium::span<const float> pBuf,
                        float* R,
                        float* G,
                        float* B) const {
  float Lstar = pBuf[0];
  float astar = pBuf[1];
  float bstar = pBuf[2];
  float M = (Lstar + 16.0f) / 116.0f;
  float L = M + astar / 500.0f;
  float N = M - bstar / 200.0f;
  float X;
  float Y;
  float Z;
  if (L < 0.2069f)
    X = 0.957f * 0.12842f * (L - 0.1379f);
  else
    X = 0.957f * L * L * L;

  if (M < 0.2069f)
    Y = 0.12842f * (M - 0.1379f);
  else
    Y = M * M * M;

  if (N < 0.2069f)
    Z = 1.0889f * 0.12842f * (N - 0.1379f);
  else
    Z = 1.0889f * N * N * N;

  XYZ_to_sRGB(X, Y, Z, R, G, B);
  return true;
}

void CPDF_LabCS::TranslateImageLine(pdfium::span<uint8_t> dest_span,
                                    pdfium::span<const uint8_t> src_span,
                                    int pixels,
                                    int image_width,
                                    int image_height,
                                    bool bTransMask) const {
  uint8_t* pDestBuf = dest_span.data();
  const uint8_t* pSrcBuf = src_span.data();
  for (int i = 0; i < pixels; i++) {
    float lab[3];
    lab[0] = pSrcBuf[0] * 100 / 255.0f;
    lab[1] = pSrcBuf[1] - 128;
    lab[2] = pSrcBuf[2] - 128;

    float R;
    float G;
    float B;
    GetRGB(lab, &R, &G, &B);
    pDestBuf[0] = static_cast<int32_t>(B * 255);
    pDestBuf[1] = static_cast<int32_t>(G * 255);
    pDestBuf[2] = static_cast<int32_t>(R * 255);
    pDestBuf += 3;
    pSrcBuf += 3;
  }
}

CPDF_ICCBasedCS::CPDF_ICCBasedCS() : CPDF_BasedCS(Family::kICCBased) {}

CPDF_ICCBasedCS::~CPDF_ICCBasedCS() = default;

uint32_t CPDF_ICCBasedCS::v_Load(CPDF_Document* pDoc,
                                 const CPDF_Array* pArray,
                                 std::set<const CPDF_Object*>* pVisited) {
  RetainPtr<const CPDF_Stream> pStream = pArray->GetStreamAt(1);
  if (!pStream)
    return 0;

  // The PDF 1.7 spec says the number of components must be valid. While some
  // PDF viewers tolerate invalid values, Acrobat does not, so be consistent
  // with Acrobat and reject bad values.
  RetainPtr<const CPDF_Dictionary> pDict = pStream->GetDict();
  const int32_t nDictComponents = pDict ? pDict->GetIntegerFor("N") : 0;
  if (!fxcodec::IccTransform::IsValidIccComponents(nDictComponents)) {
    return 0;
  }

  // Safe to cast, as the value just got validated.
  const uint32_t nComponents = static_cast<uint32_t>(nDictComponents);
  m_pProfile = CPDF_DocPageData::FromDocument(pDoc)->GetIccProfile(pStream);
  if (!m_pProfile)
    return 0;

  // If PDFium does not understand the ICC profile format at all, or if it's
  // SRGB, a profile PDFium recognizes but does not support well, then try the
  // alternate profile.
  if (!m_pProfile->IsSupported() &&
      !FindAlternateProfile(pDoc, pDict.Get(), pVisited, nComponents)) {
    // If there is no alternate profile, use a stock profile as mentioned in
    // the PDF 1.7 spec in table 4.16 in the "Alternate" key description.
    DCHECK(!m_pBaseCS);
    m_pBaseCS = GetStockAlternateProfile(nComponents);
  }

  m_pRanges = GetRanges(pDict.Get(), nComponents);
  return nComponents;
}

bool CPDF_ICCBasedCS::GetRGB(pdfium::span<const float> pBuf,
                             float* R,
                             float* G,
                             float* B) const {
  DCHECK(m_pProfile);
  if (m_pProfile->IsSRGB()) {
    *R = pBuf[0];
    *G = pBuf[1];
    *B = pBuf[2];
    return true;
  }
  if (m_pProfile->IsSupported()) {
    float rgb[3];
    m_pProfile->Translate(pBuf.first(CountComponents()), rgb);
    *R = rgb[0];
    *G = rgb[1];
    *B = rgb[2];
    return true;
  }
  if (m_pBaseCS)
    return m_pBaseCS->GetRGB(pBuf, R, G, B);

  *R = 0.0f;
  *G = 0.0f;
  *B = 0.0f;
  return true;
}

void CPDF_ICCBasedCS::TranslateImageLine(pdfium::span<uint8_t> dest_span,
                                         pdfium::span<const uint8_t> src_span,
                                         int pixels,
                                         int image_width,
                                         int image_height,
                                         bool bTransMask) const {
  if (m_pProfile->IsSRGB()) {
    fxcodec::ReverseRGB(dest_span.data(), src_span.data(), pixels);
    return;
  }
  if (!m_pProfile->IsSupported()) {
    if (m_pBaseCS) {
      m_pBaseCS->TranslateImageLine(dest_span, src_span, pixels, image_width,
                                    image_height, false);
    }
    return;
  }

  // |nMaxColors| will not overflow since |nComponents| is limited in size.
  const uint32_t nComponents = CountComponents();
  DCHECK(fxcodec::IccTransform::IsValidIccComponents(nComponents));
  int nMaxColors = 1;
  for (uint32_t i = 0; i < nComponents; i++)
    nMaxColors *= 52;

  bool bTranslate = nComponents > 3;
  if (!bTranslate) {
    FX_SAFE_INT32 nPixelCount = image_width;
    nPixelCount *= image_height;
    if (nPixelCount.IsValid())
      bTranslate = nPixelCount.ValueOrDie() < nMaxColors * 3 / 2;
  }
  if (bTranslate && m_pProfile->IsSupported()) {
    m_pProfile->TranslateScanline(dest_span, src_span, pixels);
    return;
  }
  if (m_pCache.empty()) {
    m_pCache.resize(Fx2DSizeOrDie(nMaxColors, 3));
    DataVector<uint8_t> temp_src(Fx2DSizeOrDie(nMaxColors, nComponents));
    size_t src_index = 0;
    for (int i = 0; i < nMaxColors; i++) {
      uint32_t color = i;
      uint32_t order = nMaxColors / 52;
      for (uint32_t c = 0; c < nComponents; c++) {
        temp_src[src_index++] = static_cast<uint8_t>(color / order * 5);
        color %= order;
        order /= 52;
      }
    }
    if (m_pProfile->IsSupported()) {
      m_pProfile->TranslateScanline(m_pCache, temp_src, nMaxColors);
    }
  }
  uint8_t* pDestBuf = dest_span.data();
  const uint8_t* pSrcBuf = src_span.data();
  for (int i = 0; i < pixels; i++) {
    int index = 0;
    for (uint32_t c = 0; c < nComponents; c++) {
      index = index * 52 + (*pSrcBuf) / 5;
      pSrcBuf++;
    }
    index *= 3;
    *pDestBuf++ = m_pCache[index];
    *pDestBuf++ = m_pCache[index + 1];
    *pDestBuf++ = m_pCache[index + 2];
  }
}

bool CPDF_ICCBasedCS::IsNormal() const {
  if (m_pProfile->IsSRGB())
    return true;
  if (m_pProfile->IsSupported())
    return m_pProfile->IsNormal();
  if (m_pBaseCS)
    return m_pBaseCS->IsNormal();
  return false;
}

bool CPDF_ICCBasedCS::FindAlternateProfile(
    CPDF_Document* pDoc,
    const CPDF_Dictionary* pDict,
    std::set<const CPDF_Object*>* pVisited,
    uint32_t nExpectedComponents) {
  RetainPtr<const CPDF_Object> pAlterCSObj =
      pDict->GetDirectObjectFor("Alternate");
  if (!pAlterCSObj)
    return false;

  auto pAlterCS = CPDF_ColorSpace::Load(pDoc, pAlterCSObj.Get(), pVisited);
  if (!pAlterCS)
    return false;

  if (pAlterCS->GetFamily() == Family::kPattern)
    return false;

  if (pAlterCS->CountComponents() != nExpectedComponents)
    return false;

  m_pBaseCS = std::move(pAlterCS);
  return true;
}

// static
RetainPtr<CPDF_ColorSpace> CPDF_ICCBasedCS::GetStockAlternateProfile(
    uint32_t nComponents) {
  if (nComponents == 1)
    return GetStockCS(Family::kDeviceGray);
  if (nComponents == 3)
    return GetStockCS(Family::kDeviceRGB);
  if (nComponents == 4)
    return GetStockCS(Family::kDeviceCMYK);
  NOTREACHED_NORETURN();
}

// static
std::vector<float> CPDF_ICCBasedCS::GetRanges(const CPDF_Dictionary* pDict,
                                              uint32_t nComponents) {
  DCHECK(fxcodec::IccTransform::IsValidIccComponents(nComponents));
  RetainPtr<const CPDF_Array> pRanges = pDict->GetArrayFor("Range");
  if (pRanges && pRanges->size() >= nComponents * 2)
    return ReadArrayElementsToVector(pRanges.Get(), nComponents * 2);

  std::vector<float> ranges;
  ranges.reserve(nComponents * 2);
  for (uint32_t i = 0; i < nComponents; i++) {
    ranges.push_back(0.0f);
    ranges.push_back(1.0f);
  }
  return ranges;
}

CPDF_SeparationCS::CPDF_SeparationCS() : CPDF_BasedCS(Family::kSeparation) {}

CPDF_SeparationCS::~CPDF_SeparationCS() = default;

void CPDF_SeparationCS::GetDefaultValue(int iComponent,
                                        float* value,
                                        float* min,
                                        float* max) const {
  *value = 1.0f;
  *min = 0;
  *max = 1.0f;
}

uint32_t CPDF_SeparationCS::v_Load(CPDF_Document* pDoc,
                                   const CPDF_Array* pArray,
                                   std::set<const CPDF_Object*>* pVisited) {
  m_IsNoneType = pArray->GetByteStringAt(1) == "None";
  if (m_IsNoneType)
    return 1;

  RetainPtr<const CPDF_Object> pAltArray = pArray->GetDirectObjectAt(2);
  if (HasSameArray(pAltArray.Get()))
    return 0;

  m_pBaseCS = Load(pDoc, pAltArray.Get(), pVisited);
  if (!m_pBaseCS)
    return 0;

  if (m_pBaseCS->IsSpecial())
    return 0;

  RetainPtr<const CPDF_Object> pFuncObj = pArray->GetDirectObjectAt(3);
  if (pFuncObj && !pFuncObj->IsName()) {
    auto pFunc = CPDF_Function::Load(std::move(pFuncObj));
    if (pFunc && pFunc->CountOutputs() >= m_pBaseCS->CountComponents())
      m_pFunc = std::move(pFunc);
  }
  return 1;
}

bool CPDF_SeparationCS::GetRGB(pdfium::span<const float> pBuf,
                               float* R,
                               float* G,
                               float* B) const {
  if (m_IsNoneType)
    return false;

  if (!m_pFunc) {
    if (!m_pBaseCS)
      return false;

    int nComps = m_pBaseCS->CountComponents();
    std::vector<float> results(nComps);
    for (int i = 0; i < nComps; i++)
      results[i] = pBuf[0];
    return m_pBaseCS->GetRGB(results, R, G, B);
  }

  // Using at least 16 elements due to the call m_pAltCS->GetRGB() below.
  std::vector<float> results(std::max(m_pFunc->CountOutputs(), 16u));
  uint32_t nresults = m_pFunc->Call(pBuf.first(1), results).value_or(0);
  if (nresults == 0)
    return false;

  if (m_pBaseCS)
    return m_pBaseCS->GetRGB(results, R, G, B);

  *R = 0.0f;
  *G = 0.0f;
  *B = 0.0f;
  return false;
}

CPDF_DeviceNCS::CPDF_DeviceNCS() : CPDF_BasedCS(Family::kDeviceN) {}

CPDF_DeviceNCS::~CPDF_DeviceNCS() = default;

void CPDF_DeviceNCS::GetDefaultValue(int iComponent,
                                     float* value,
                                     float* min,
                                     float* max) const {
  *value = 1.0f;
  *min = 0;
  *max = 1.0f;
}

uint32_t CPDF_DeviceNCS::v_Load(CPDF_Document* pDoc,
                                const CPDF_Array* pArray,
                                std::set<const CPDF_Object*>* pVisited) {
  RetainPtr<const CPDF_Array> pObj = ToArray(pArray->GetDirectObjectAt(1));
  if (!pObj)
    return 0;

  RetainPtr<const CPDF_Object> pAltCS = pArray->GetDirectObjectAt(2);
  if (!pAltCS || HasSameArray(pAltCS.Get()))
    return 0;

  m_pBaseCS = Load(pDoc, pAltCS.Get(), pVisited);
  m_pFunc = CPDF_Function::Load(pArray->GetDirectObjectAt(3));
  if (!m_pBaseCS || !m_pFunc)
    return 0;

  if (m_pBaseCS->IsSpecial())
    return 0;

  if (m_pFunc->CountOutputs() < m_pBaseCS->CountComponents())
    return 0;

  return fxcrt::CollectionSize<uint32_t>(*pObj);
}

bool CPDF_DeviceNCS::GetRGB(pdfium::span<const float> pBuf,
                            float* R,
                            float* G,
                            float* B) const {
  if (!m_pFunc)
    return false;

  // Using at least 16 elements due to the call m_pAltCS->GetRGB() below.
  std::vector<float> results(std::max(m_pFunc->CountOutputs(), 16u));
  uint32_t nresults =
      m_pFunc->Call(pBuf.first(CountComponents()), pdfium::make_span(results))
          .value_or(0);

  if (nresults == 0)
    return false;

  return m_pBaseCS->GetRGB(results, R, G, B);
}
