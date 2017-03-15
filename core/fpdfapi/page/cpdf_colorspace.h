// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_COLORSPACE_H_
#define CORE_FPDFAPI_PAGE_CPDF_COLORSPACE_H_

#include <memory>

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"

#define PDFCS_DEVICEGRAY 1
#define PDFCS_DEVICERGB 2
#define PDFCS_DEVICECMYK 3
#define PDFCS_CALGRAY 4
#define PDFCS_CALRGB 5
#define PDFCS_LAB 6
#define PDFCS_ICCBASED 7
#define PDFCS_SEPARATION 8
#define PDFCS_DEVICEN 9
#define PDFCS_INDEXED 10
#define PDFCS_PATTERN 11

class CPDF_Array;
class CPDF_Document;
class CPDF_Object;

class CPDF_ColorSpace {
 public:
  static CPDF_ColorSpace* GetStockCS(int Family);
  static CPDF_ColorSpace* ColorspaceFromName(const CFX_ByteString& name);
  static std::unique_ptr<CPDF_ColorSpace> Load(CPDF_Document* pDoc,
                                               CPDF_Object* pCSObj);

  void Release();

  int GetBufSize() const;
  float* CreateBuf();
  void GetDefaultColor(float* buf) const;
  uint32_t CountComponents() const;
  int GetFamily() const { return m_Family; }
  virtual void GetDefaultValue(int iComponent,
                               float* value,
                               float* min,
                               float* max) const;

  bool sRGB() const;
  virtual bool GetRGB(float* pBuf, float* R, float* G, float* B) const = 0;
  virtual bool SetRGB(float* pBuf, float R, float G, float B) const;

  bool GetCMYK(float* pBuf, float* c, float* m, float* y, float* k) const;
  bool SetCMYK(float* pBuf, float c, float m, float y, float k) const;

  virtual void TranslateImageLine(uint8_t* dest_buf,
                                  const uint8_t* src_buf,
                                  int pixels,
                                  int image_width,
                                  int image_height,
                                  bool bTransMask) const;

  CPDF_Array*& GetArray() { return m_pArray; }
  virtual CPDF_ColorSpace* GetBaseCS() const;

  virtual void EnableStdConversion(bool bEnabled);

  CPDF_Document* const m_pDocument;

 protected:
  CPDF_ColorSpace(CPDF_Document* pDoc, int family, uint32_t nComponents);
  virtual ~CPDF_ColorSpace();

  virtual bool v_Load(CPDF_Document* pDoc, CPDF_Array* pArray);
  virtual bool v_GetCMYK(float* pBuf,
                         float* c,
                         float* m,
                         float* y,
                         float* k) const;
  virtual bool v_SetCMYK(float* pBuf, float c, float m, float y, float k) const;

  int m_Family;
  uint32_t m_nComponents;
  CPDF_Array* m_pArray;
  uint32_t m_dwStdConversion;
};

namespace std {

// Make std::unique_ptr<CPDF_ColorSpace> call Release() rather than
// simply deleting the object.
template <>
struct default_delete<CPDF_ColorSpace> {
  void operator()(CPDF_ColorSpace* pColorSpace) const {
    if (pColorSpace)
      pColorSpace->Release();
  }
};

}  // namespace std

#endif  // CORE_FPDFAPI_PAGE_CPDF_COLORSPACE_H_
