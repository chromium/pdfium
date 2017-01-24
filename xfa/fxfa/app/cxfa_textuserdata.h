// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_CXFA_TEXTUSERDATA_H_
#define XFA_FXFA_APP_CXFA_TEXTUSERDATA_H_

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_basic.h"

class CFDE_CSSComputedStyle;
class CXFA_LinkUserData;

class CXFA_TextUserData : public CFX_Retainable {
 public:
  template <typename T, typename... Args>
  friend CFX_RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  CFX_RetainPtr<CFDE_CSSComputedStyle> m_pStyle;
  CFX_RetainPtr<CXFA_LinkUserData> m_pLinkData;

 protected:
  explicit CXFA_TextUserData(
      const CFX_RetainPtr<CFDE_CSSComputedStyle>& pStyle);
  CXFA_TextUserData(const CFX_RetainPtr<CFDE_CSSComputedStyle>& pStyle,
                    const CFX_RetainPtr<CXFA_LinkUserData>& pLinkData);
  ~CXFA_TextUserData() override;
};

#endif  // XFA_FXFA_APP_CXFA_TEXTUSERDATA_H_
