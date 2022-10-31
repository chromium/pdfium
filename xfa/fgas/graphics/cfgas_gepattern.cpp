// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/graphics/cfgas_gepattern.h"

CFGAS_GEPattern::CFGAS_GEPattern(HatchStyle hatchStyle,
                                 FX_ARGB foreArgb,
                                 FX_ARGB backArgb)
    : m_hatchStyle(hatchStyle), m_foreArgb(foreArgb), m_backArgb(backArgb) {}

CFGAS_GEPattern::~CFGAS_GEPattern() = default;
