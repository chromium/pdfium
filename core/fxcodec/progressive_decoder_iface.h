// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_PROGRESSIVE_DECODER_IFACE_H_
#define CORE_FXCODEC_PROGRESSIVE_DECODER_IFACE_H_

#ifndef PDF_ENABLE_XFA
#error "XFA Only"
#endif

namespace fxcodec {

class ProgressiveDecoderIface {
 public:
  // TODO(https://crbug.com/444045690): Hoist the `Context` class to the
  // top-level / delete empty `ProgressiveDecoderIface`.
  class Context {
   public:
    virtual ~Context() = default;
  };
};

}  // namespace fxcodec

using fxcodec::ProgressiveDecoderIface;

#endif  // CORE_FXCODEC_PROGRESSIVE_DECODER_IFACE_H_
