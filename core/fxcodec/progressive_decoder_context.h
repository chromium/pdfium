// Copyright 2025 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_PROGRESSIVE_DECODER_CONTEXT_H_
#define CORE_FXCODEC_PROGRESSIVE_DECODER_CONTEXT_H_

#ifndef PDF_ENABLE_XFA
#error "XFA Only"
#endif

namespace fxcodec {

class ProgressiveDecoderContext {
 public:
  virtual ~ProgressiveDecoderContext() = default;
};

}  // namespace fxcodec

using fxcodec::ProgressiveDecoderContext;

#endif  // CORE_FXCODEC_PROGRESSIVE_DECODER_CONTEXT_H_
