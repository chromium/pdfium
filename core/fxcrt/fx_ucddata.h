// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FX_UCDDATA_H_
#define CORE_FXCRT_FX_UCDDATA_H_

#include "core/fxcrt/fx_system.h"

namespace fxcrt {

// Format of uint32_t values in kTextLayoutCodeProperties[].
constexpr uint32_t kBreakTypeBitPos = 0;
constexpr uint32_t kBreakTypeBitCount = 6;
constexpr uint32_t kBreakTypeBitMask =
    (((1u << kBreakTypeBitCount) - 1) << kBreakTypeBitPos);

constexpr uint32_t kBidiClassBitPos = 6;
constexpr uint32_t kBidiClassBitCount = 5;
constexpr uint32_t kBidiClassBitMask =
    (((1u << kBidiClassBitCount) - 1) << kBidiClassBitPos);

constexpr uint32_t kCharTypeBitPos = 11;
constexpr uint32_t kCharTypeBitCount = 4;
constexpr uint32_t kCharTypeBitMask =
    (((1u << kCharTypeBitCount) - 1) << kCharTypeBitPos);

// TODO(tsepez): Unknown, possibly unused field.
constexpr uint32_t kField2BitPos = 15;
constexpr uint32_t kField2BitCount = 8;
constexpr uint32_t kField2BitMask =
    (((1 << kField2BitCount) - 1) << kField2BitPos);

constexpr uint32_t kMirrorBitPos = 23;
constexpr uint32_t kMirrorBitCount = 9;
constexpr uint32_t kMirrorBitMask =
    (((1 << kMirrorBitCount) - 1) << kMirrorBitPos);

extern const uint32_t kTextLayoutCodeProperties[];
extern const size_t kTextLayoutCodePropertiesSize;

extern const uint16_t kFXTextLayoutBidiMirror[];
extern const size_t kFXTextLayoutBidiMirrorSize;

}  // namespace fxcrt

using fxcrt::kBidiClassBitCount;
using fxcrt::kBidiClassBitMask;
using fxcrt::kBidiClassBitPos;
using fxcrt::kBreakTypeBitCount;
using fxcrt::kBreakTypeBitMask;
using fxcrt::kBreakTypeBitPos;
using fxcrt::kCharTypeBitCount;
using fxcrt::kCharTypeBitMask;
using fxcrt::kCharTypeBitPos;
using fxcrt::kField2BitCount;
using fxcrt::kField2BitMask;
using fxcrt::kField2BitPos;
using fxcrt::kFXTextLayoutBidiMirror;
using fxcrt::kFXTextLayoutBidiMirrorSize;
using fxcrt::kMirrorBitCount;
using fxcrt::kMirrorBitMask;
using fxcrt::kMirrorBitPos;
using fxcrt::kTextLayoutCodeProperties;
using fxcrt::kTextLayoutCodePropertiesSize;

#endif  // CORE_FXCRT_FX_UCDDATA_H_
