// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPVT_TYPESET_H_
#define CORE_FPDFDOC_CPVT_TYPESET_H_

#include "core/fpdfdoc/cpvt_floatrect.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/unowned_ptr.h"

class CPDF_VariableText;
class CPVT_Section;

class CPVT_Typeset final {
 public:
  explicit CPVT_Typeset(CPVT_Section* pSection);
  ~CPVT_Typeset();

  CFX_SizeF GetEditSize(float fFontSize);
  CPVT_FloatRect Typeset();
  CPVT_FloatRect CharArray();

 private:
  void SplitLines(bool bTypeset, float fFontSize);
  void OutputLines();

  CPVT_FloatRect m_rcRet;
  UnownedPtr<CPDF_VariableText> const m_pVT;
  UnownedPtr<CPVT_Section> const m_pSection;
};

#endif  // CORE_FPDFDOC_CPVT_TYPESET_H_
