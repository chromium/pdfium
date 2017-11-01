// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjx_thisproxy.h"

#include "xfa/fxfa/parser/cxfa_thisproxy.h"

CJX_ThisProxy::CJX_ThisProxy(CXFA_ThisProxy* proxy) : CJX_Object(proxy) {}

CJX_ThisProxy::~CJX_ThisProxy() {}
