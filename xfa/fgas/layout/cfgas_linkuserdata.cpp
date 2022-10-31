// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/layout/cfgas_linkuserdata.h"

CFGAS_LinkUserData::CFGAS_LinkUserData(const WideString& wsText)
    : m_wsURLContent(wsText) {}

CFGAS_LinkUserData::~CFGAS_LinkUserData() = default;
