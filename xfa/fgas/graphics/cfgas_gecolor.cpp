// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/graphics/cfgas_gecolor.h"

CFGAS_GEColor::CFGAS_GEColor(const FX_ARGB argb)
    : m_type(Solid), m_argb(argb) {}

CFGAS_GEColor::CFGAS_GEColor(CFGAS_GEPattern* pattern, const FX_ARGB argb)
    : m_type(Pattern), m_argb(argb), m_pPattern(pattern) {}

CFGAS_GEColor::CFGAS_GEColor(CFGAS_GEShading* shading)
    : m_type(Shading), m_pShading(shading) {}

CFGAS_GEColor::CFGAS_GEColor(const CFGAS_GEColor& that) = default;

CFGAS_GEColor::~CFGAS_GEColor() = default;

CFGAS_GEColor& CFGAS_GEColor::operator=(const CFGAS_GEColor& that) = default;
