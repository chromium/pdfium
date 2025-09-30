// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_JBIG2_JBIG2_SYMBOLDICT_H_
#define CORE_FXCODEC_JBIG2_JBIG2_SYMBOLDICT_H_

#include <memory>
#include <utility>
#include <vector>

#include "core/fxcodec/jbig2/JBig2_ArithDecoder.h"
#include "core/fxcodec/jbig2/JBig2_Image.h"

class CJBig2_SymbolDict {
 public:
  CJBig2_SymbolDict();
  ~CJBig2_SymbolDict();

  std::unique_ptr<CJBig2_SymbolDict> DeepCopy() const;

  void AddImage(std::unique_ptr<CJBig2_Image> image) {
    sdexsyms_.push_back(std::move(image));
  }

  size_t NumImages() const { return sdexsyms_.size(); }
  CJBig2_Image* GetImage(size_t index) const { return sdexsyms_[index].get(); }

  const std::vector<JBig2ArithCtx>& GbContexts() const { return gb_contexts_; }
  const std::vector<JBig2ArithCtx>& GrContexts() const { return gr_contexts_; }

  void SetGbContexts(std::vector<JBig2ArithCtx> gbContexts) {
    gb_contexts_ = std::move(gbContexts);
  }
  void SetGrContexts(std::vector<JBig2ArithCtx> grContexts) {
    gr_contexts_ = std::move(grContexts);
  }

 private:
  std::vector<JBig2ArithCtx> gb_contexts_;
  std::vector<JBig2ArithCtx> gr_contexts_;
  std::vector<std::unique_ptr<CJBig2_Image>> sdexsyms_;
};

#endif  // CORE_FXCODEC_JBIG2_JBIG2_SYMBOLDICT_H_
