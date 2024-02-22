// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/gif/gif_progressive_decoder.h"

#include "core/fxcodec/cfx_codec_memory.h"
#include "core/fxcodec/gif/gif_decoder.h"
#include "core/fxcrt/check.h"

namespace fxcodec {

namespace {

GifProgressiveDecoder* g_gif_decoder = nullptr;

}  // namespace

// static
void GifProgressiveDecoder::InitializeGlobals() {
  CHECK(!g_gif_decoder);
  g_gif_decoder = new GifProgressiveDecoder();
}

// static
void GifProgressiveDecoder::DestroyGlobals() {
  delete g_gif_decoder;
  g_gif_decoder = nullptr;
}

// static
GifProgressiveDecoder* GifProgressiveDecoder::GetInstance() {
  return g_gif_decoder;
}

GifProgressiveDecoder::GifProgressiveDecoder() = default;

GifProgressiveDecoder::~GifProgressiveDecoder() = default;

FX_FILESIZE GifProgressiveDecoder::GetAvailInput(Context* context) const {
  return GifDecoder::GetAvailInput(context);
}

bool GifProgressiveDecoder::Input(Context* context,
                                  RetainPtr<CFX_CodecMemory> codec_memory) {
  return GifDecoder::Input(context, codec_memory);
}

}  // namespace fxcodec
