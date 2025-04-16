// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_ANNOTCONTEXT_H_
#define CORE_FPDFAPI_PAGE_CPDF_ANNOTCONTEXT_H_

#include <memory>

#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"

class CPDF_Dictionary;
class CPDF_Form;
class CPDF_Stream;
class IPDF_Page;

class CPDF_AnnotContext {
 public:
  CPDF_AnnotContext(RetainPtr<CPDF_Dictionary> pAnnotDict, IPDF_Page* pPage);
  ~CPDF_AnnotContext();

  void SetForm(RetainPtr<CPDF_Stream> pStream);
  bool HasForm() const { return !!annot_form_; }
  CPDF_Form* GetForm() const { return annot_form_.get(); }

  // Never nullptr.
  RetainPtr<CPDF_Dictionary> GetMutableAnnotDict() { return annot_dict_; }
  const CPDF_Dictionary* GetAnnotDict() const { return annot_dict_.Get(); }

  // Never nullptr.
  IPDF_Page* GetPage() const { return page_; }

 private:
  std::unique_ptr<CPDF_Form> annot_form_;
  RetainPtr<CPDF_Dictionary> const annot_dict_;
  UnownedPtr<IPDF_Page> const page_;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_ANNOTCONTEXT_H_
