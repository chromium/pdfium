// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_CRT_LOCALE_MGR_IFACE_H_
#define XFA_FGAS_CRT_LOCALE_MGR_IFACE_H_

#include "core/fxcrt/fx_string.h"

class LocaleIface;

class LocaleMgrIface {
 public:
  virtual ~LocaleMgrIface() = default;

  virtual LocaleIface* GetDefLocale() = 0;
  virtual LocaleIface* GetLocaleByName(const WideString& wsLCID) = 0;
};

#endif  // XFA_FGAS_CRT_LOCALE_MGR_IFACE_H_
