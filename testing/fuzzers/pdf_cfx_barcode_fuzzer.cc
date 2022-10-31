// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fx_string.h"
#include "fxbarcode/cfx_barcode.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size < 2 * sizeof(uint16_t))
    return 0;

  BC_TYPE type =
      static_cast<BC_TYPE>(data[0] % (static_cast<int>(BC_TYPE::kLast) + 1));

  // Only used one byte, but align with uint16_t for string below.
  data += sizeof(uint16_t);
  size -= sizeof(uint16_t);

  auto barcode = CFX_Barcode::Create(type);

  // TODO(tsepez): Setup more options from |data|.
  barcode->SetModuleHeight(300);
  barcode->SetModuleWidth(420);
  barcode->SetHeight(298);
  barcode->SetWidth(418);

  WideString content = WideString::FromUTF16LE(
      reinterpret_cast<const uint16_t*>(data), size / sizeof(uint16_t));

  if (!barcode->Encode(content.AsStringView()))
    return 0;

  // TODO(tsepez): Output to device.
  return 0;
}
