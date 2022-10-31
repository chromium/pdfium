// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXGE_CALCULATE_PITCH_H_
#define CORE_FXGE_CALCULATE_PITCH_H_

#include <stdint.h>

#include "third_party/abseil-cpp/absl/types/optional.h"

namespace fxge {

uint32_t CalculatePitch8OrDie(uint32_t bpc, uint32_t components, int width);
uint32_t CalculatePitch32OrDie(int bpp, int width);
absl::optional<uint32_t> CalculatePitch8(uint32_t bpc,
                                         uint32_t components,
                                         int width);
absl::optional<uint32_t> CalculatePitch32(int bpp, int width);

}  // namespace fxge

#endif  // CORE_FXGE_CALCULATE_PITCH_H_
