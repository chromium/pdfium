// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/helpers/win32/com_factory.h"

#include <combaseapi.h>
#include <objbase.h>
#include <winerror.h>
#include <xpsobjectmodel.h>

#include "core/fxcrt/check_op.h"

ComFactory::ComFactory() = default;

ComFactory::~ComFactory() {
  if (xps_om_object_factory_) {
    xps_om_object_factory_->Release();
  }

  if (initialized_) {
    CoUninitialize();
  }
}

bool ComFactory::Initialize() {
  if (!initialized_) {
    HRESULT result =
        CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
    DCHECK_NE(RPC_E_CHANGED_MODE, result);
    initialized_ = SUCCEEDED(result);
  }

  return initialized_;
}

IXpsOMObjectFactory* ComFactory::GetXpsOMObjectFactory() {
  if (!xps_om_object_factory_ && Initialize()) {
    HRESULT result =
        CoCreateInstance(__uuidof(XpsOMObjectFactory), /*pUnkOuter=*/nullptr,
                         CLSCTX_INPROC_SERVER, __uuidof(IXpsOMObjectFactory),
                         reinterpret_cast<LPVOID*>(&xps_om_object_factory_));
    if (SUCCEEDED(result)) {
      DCHECK(xps_om_object_factory_);
    } else {
      DCHECK(!xps_om_object_factory_);
    }
  }

  return xps_om_object_factory_;
}
