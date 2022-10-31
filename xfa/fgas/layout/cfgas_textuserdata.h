// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LAYOUT_CFGAS_TEXTUSERDATA_H_
#define XFA_FGAS_LAYOUT_CFGAS_TEXTUSERDATA_H_

#include "core/fxcrt/retain_ptr.h"

class CFGAS_LinkUserData;
class CFX_CSSComputedStyle;

class CFGAS_TextUserData final : public Retainable {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;

  RetainPtr<CFX_CSSComputedStyle> m_pStyle;
  RetainPtr<CFGAS_LinkUserData> m_pLinkData;

 private:
  explicit CFGAS_TextUserData(const RetainPtr<CFX_CSSComputedStyle>& pStyle);
  CFGAS_TextUserData(const RetainPtr<CFX_CSSComputedStyle>& pStyle,
                     const RetainPtr<CFGAS_LinkUserData>& pLinkData);
  ~CFGAS_TextUserData() override;
};

#endif  // XFA_FGAS_LAYOUT_CFGAS_TEXTUSERDATA_H_
