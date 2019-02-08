// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LAYOUT_CFX_TEXTUSERDATA_H_
#define XFA_FGAS_LAYOUT_CFX_TEXTUSERDATA_H_

#include "core/fxcrt/retain_ptr.h"

class CFX_CSSComputedStyle;
class CFX_LinkUserData;

class CFX_TextUserData final : public Retainable {
 public:
  template <typename T, typename... Args>
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  RetainPtr<CFX_CSSComputedStyle> m_pStyle;
  RetainPtr<CFX_LinkUserData> m_pLinkData;

 private:
  explicit CFX_TextUserData(const RetainPtr<CFX_CSSComputedStyle>& pStyle);
  CFX_TextUserData(const RetainPtr<CFX_CSSComputedStyle>& pStyle,
                   const RetainPtr<CFX_LinkUserData>& pLinkData);
  ~CFX_TextUserData() override;
};

#endif  // XFA_FGAS_LAYOUT_CFX_TEXTUSERDATA_H_
