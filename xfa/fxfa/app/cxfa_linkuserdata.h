// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_CXFA_LINKUSERDATA_H_
#define XFA_FXFA_APP_CXFA_LINKUSERDATA_H_

#include "core/fxcrt/fx_basic.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"

class CXFA_LinkUserData : public IFX_Retainable {
 public:
  explicit CXFA_LinkUserData(FX_WCHAR* pszText);
  ~CXFA_LinkUserData() override;

  // IFX_Retainable:
  uint32_t Retain() override;
  uint32_t Release() override;

  const FX_WCHAR* GetLinkURL();

 protected:
  uint32_t m_dwRefCount;
  CFX_WideString m_wsURLContent;
};

#endif  // XFA_FXFA_APP_CXFA_LINKUSERDATA_H_
