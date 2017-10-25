// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XFA_FXFA_FM2JS_CXFA_FMTOJAVASCRIPTDEPTH_H_
#define XFA_FXFA_FM2JS_CXFA_FMTOJAVASCRIPTDEPTH_H_

class CXFA_FMToJavaScriptDepth {
 public:
  CXFA_FMToJavaScriptDepth() { depth_++; }
  ~CXFA_FMToJavaScriptDepth() { depth_--; }

  bool IsWithinMaxDepth() const { return depth_ <= max_depth_; }

  static void Reset();

 private:
  static unsigned long depth_;
  static unsigned long max_depth_;
};

#endif  // XFA_FXFA_FM2JS_CXFA_FMTOJAVASCRIPTDEPTH_H_
