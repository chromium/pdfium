// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_COLOR_H_
#define CORE_FPDFAPI_PAGE_CPDF_COLOR_H_

#include <memory>
#include <vector>

#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/retain_ptr.h"

class CPDF_ColorSpace;
class CPDF_Pattern;
class PatternValue;

class CPDF_Color {
 public:
  CPDF_Color();
  CPDF_Color(const CPDF_Color& that);

  ~CPDF_Color();

  CPDF_Color& operator=(const CPDF_Color& that);

  bool IsNull() const { return m_Buffer.empty() && !m_pValue; }
  bool IsPattern() const;
  void SetColorSpace(const RetainPtr<CPDF_ColorSpace>& pCS);
  void SetValueForNonPattern(const std::vector<float>& values);
  void SetValueForPattern(const RetainPtr<CPDF_Pattern>& pPattern,
                          const std::vector<float>& values);
  uint32_t CountComponents() const;
  bool IsColorSpaceRGB() const;
  bool GetRGB(int* R, int* G, int* B) const;

  // Should only be called if IsPattern() returns true.
  CPDF_Pattern* GetPattern() const;

 protected:
  bool IsPatternInternal() const;

  std::vector<float> m_Buffer;             // Used for non-pattern colorspaces.
  std::unique_ptr<PatternValue> m_pValue;  // Used for pattern colorspaces.
  RetainPtr<CPDF_ColorSpace> m_pCS;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_COLOR_H_
