// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/calculate_pitch.h"

#include "core/fxcrt/fx_safe_types.h"
#include "core/fxge/dib/fx_dib.h"

namespace fxge {
namespace {

FX_SAFE_UINT32 CalculatePitch8Safely(uint32_t bpc,
                                     uint32_t components,
                                     int width) {
  FX_SAFE_UINT32 pitch = bpc;
  pitch *= components;
  pitch *= width;
  pitch += 7;
  pitch /= 8;
  return pitch;
}

FX_SAFE_UINT32 CalculatePitch32Safely(int bpp, int width) {
  FX_SAFE_UINT32 pitch = bpp;
  pitch *= width;
  pitch += 31;
  pitch /= 32;  // quantized to number of 32-bit words.
  pitch *= 4;   // and then back to bytes, (not just /8 in one step).
  return pitch;
}

}  // namespace

uint32_t CalculatePitch8OrDie(uint32_t bpc, uint32_t components, int width) {
  return CalculatePitch8Safely(bpc, components, width).ValueOrDie();
}

uint32_t CalculatePitch32OrDie(int bpp, int width) {
  return CalculatePitch32Safely(bpp, width).ValueOrDie();
}

absl::optional<uint32_t> CalculatePitch8(uint32_t bpc,
                                         uint32_t components,
                                         int width) {
  FX_SAFE_UINT32 pitch = CalculatePitch8Safely(bpc, components, width);
  if (!pitch.IsValid())
    return absl::nullopt;
  return pitch.ValueOrDie();
}

absl::optional<uint32_t> CalculatePitch32(int bpp, int width) {
  FX_SAFE_UINT32 pitch = CalculatePitch32Safely(bpp, width);
  if (!pitch.IsValid())
    return absl::nullopt;
  return pitch.ValueOrDie();
}

}  // namespace fxge
