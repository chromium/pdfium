// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/graphics/cfgas_gecolor.h"

CFGAS_GEColor::CFGAS_GEColor(FX_ARGB argb) : type_(Solid), argb_(argb) {}

CFGAS_GEColor::CFGAS_GEColor(CFGAS_GEPattern* pattern, FX_ARGB argb)
    : type_(Pattern), argb_(argb), pattern_(pattern) {}

CFGAS_GEColor::CFGAS_GEColor(CFGAS_GEShading* shading)
    : type_(Shading), shading_(shading) {}

CFGAS_GEColor::CFGAS_GEColor(const CFGAS_GEColor& that) = default;

CFGAS_GEColor::~CFGAS_GEColor() = default;

CFGAS_GEColor& CFGAS_GEColor::operator=(const CFGAS_GEColor& that) = default;

// static
ByteString CFGAS_GEColor::ColorToString(FX_ARGB argb) {
  FX_BGR_STRUCT<uint8_t> bgr = ArgbToBGRStruct(argb);
  return ByteString::Format("%d,%d,%d", bgr.red, bgr.green, bgr.blue);
}
