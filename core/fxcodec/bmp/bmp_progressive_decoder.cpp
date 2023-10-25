// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/bmp/bmp_progressive_decoder.h"

#include "core/fxcodec/bmp/bmp_decoder.h"
#include "core/fxcodec/cfx_codec_memory.h"
#include "third_party/base/check.h"

namespace fxcodec {

namespace {

BmpProgressiveDecoder* g_bmp_decoder = nullptr;

}  // namespace

// static
void BmpProgressiveDecoder::InitializeGlobals() {
  CHECK(!g_bmp_decoder);
  g_bmp_decoder = new BmpProgressiveDecoder();
}

// static
void BmpProgressiveDecoder::DestroyGlobals() {
  delete g_bmp_decoder;
  g_bmp_decoder = nullptr;
}

// static
BmpProgressiveDecoder* BmpProgressiveDecoder::GetInstance() {
  return g_bmp_decoder;
}

BmpProgressiveDecoder::BmpProgressiveDecoder() = default;

BmpProgressiveDecoder::~BmpProgressiveDecoder() = default;

FX_FILESIZE BmpProgressiveDecoder::GetAvailInput(Context* context) const {
  return BmpDecoder::GetAvailInput(context);
}

bool BmpProgressiveDecoder::Input(Context* context,
                                  RetainPtr<CFX_CodecMemory> codec_memory) {
  return BmpDecoder::Input(context, codec_memory);
}

}  // namespace fxcodec
