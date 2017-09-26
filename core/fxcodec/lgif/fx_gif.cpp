// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/lgif/fx_gif.h"

static_assert(sizeof(GifImageInfo) == 9,
              "GifImageInfo should have a size of 9");
static_assert(sizeof(GifPalette) == 3, "GifPalette should have a size of 3");
static_assert(sizeof(GifPlainTextExtension) == 13,
              "GifPlainTextExtension should have a size of 13");
static_assert(sizeof(GifGraphicControlExtension) == 5,
              "GifGraphicControlExtension should have a size of 5");
static_assert(sizeof(GifHeader) == 6, "GifHeader should have a size of 6");
static_assert(sizeof(GifLocalScreenDescriptor) == 7,
              "GifLocalScreenDescriptor should have a size of 7");
