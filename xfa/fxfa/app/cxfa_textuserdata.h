// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_CXFA_TEXTUSERDATA_H_
#define XFA_FXFA_APP_CXFA_TEXTUSERDATA_H_

#include "core/fxcrt/fx_basic.h"

class CXFA_LinkUserData;
class IFDE_CSSComputedStyle;

class CXFA_TextUserData : public IFX_Retainable {
 public:
  explicit CXFA_TextUserData(IFDE_CSSComputedStyle* pStyle);
  CXFA_TextUserData(IFDE_CSSComputedStyle* pStyle,
                    CXFA_LinkUserData* pLinkData);
  ~CXFA_TextUserData() override;

  // IFX_Retainable:
  uint32_t Retain() override;
  uint32_t Release() override;

  IFDE_CSSComputedStyle* m_pStyle;
  CXFA_LinkUserData* m_pLinkData;

 protected:
  uint32_t m_dwRefCount;
};

#endif  // XFA_FXFA_APP_CXFA_TEXTUSERDATA_H_
