// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_CXFA_LINKUSERDATA_H_
#define XFA_FXFA_APP_CXFA_LINKUSERDATA_H_

#include "core/fxcrt/fx_basic.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "xfa/fgas/crt/fgas_memory.h"

class IFX_MemoryAllocator;

class CXFA_LinkUserData : public IFX_Retainable, public CFX_Target {
 public:
  CXFA_LinkUserData(IFX_MemoryAllocator* pAllocator, FX_WCHAR* pszText);
  ~CXFA_LinkUserData() override;

  // IFX_Retainable:
  uint32_t Retain() override;
  uint32_t Release() override;

  const FX_WCHAR* GetLinkURL();

 protected:
  IFX_MemoryAllocator* m_pAllocator;
  uint32_t m_dwRefCount;
  CFX_WideString m_wsURLContent;
};

#endif  // XFA_FXFA_APP_CXFA_LINKUSERDATA_H_
