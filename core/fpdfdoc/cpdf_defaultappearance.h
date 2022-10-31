// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPDF_DEFAULTAPPEARANCE_H_
#define CORE_FPDFDOC_CPDF_DEFAULTAPPEARANCE_H_

#include "core/fxcrt/bytestring.h"
#include "core/fxge/cfx_color.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

class CPDF_SimpleParser;

class CPDF_DefaultAppearance {
 public:
  CPDF_DefaultAppearance();
  explicit CPDF_DefaultAppearance(const ByteString& csDA);
  CPDF_DefaultAppearance(const CPDF_DefaultAppearance& cDA);
  ~CPDF_DefaultAppearance();

  absl::optional<ByteString> GetFont(float* fFontSize) const;

  absl::optional<CFX_Color> GetColor() const;
  absl::optional<CFX_Color::TypeAndARGB> GetColorARGB() const;

  static bool FindTagParamFromStartForTesting(CPDF_SimpleParser* parser,
                                              ByteStringView token,
                                              int nParams);

 private:
  const ByteString m_csDA;
};

#endif  // CORE_FPDFDOC_CPDF_DEFAULTAPPEARANCE_H_
