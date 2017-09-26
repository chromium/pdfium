// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/lgif/fx_gif.h"

#include <algorithm>
#include <utility>

#include "core/fxcodec/lbmp/fx_bmp.h"
#include "core/fxcodec/lgif/cgifcontext.h"
#include "third_party/base/logging.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

static_assert(sizeof(GifImageInfo) == 9,
              "GifImageInfo should have a size of 9");
static_assert(sizeof(GifPalette) == 3, "GifPalette should have a size of 3");
static_assert(sizeof(GifPTE) == 13, "GifPTE should have a size of 13");
static_assert(sizeof(GifGCE) == 5, "GifGCE should have a size of 5");
static_assert(sizeof(GifHeader) == 6, "GifHeader should have a size of 6");
static_assert(sizeof(GifLSD) == 7, "GifLSD should have a size of 7");

GifImage::GifImage() {}

GifImage::~GifImage() {}
