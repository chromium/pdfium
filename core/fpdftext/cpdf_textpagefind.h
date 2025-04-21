// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFTEXT_CPDF_TEXTPAGEFIND_H_
#define CORE_FPDFTEXT_CPDF_TEXTPAGEFIND_H_

#include <stddef.h>

#include <memory>
#include <optional>
#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxcrt/widestring.h"

class CPDF_TextPage;

class CPDF_TextPageFind {
 public:
  struct Options {
    bool bMatchCase = false;
    bool bMatchWholeWord = false;
    bool bConsecutive = false;
  };

  static std::unique_ptr<CPDF_TextPageFind> Create(
      const CPDF_TextPage* pTextPage,
      const WideString& findwhat,
      const Options& options,
      std::optional<size_t> startPos);

  ~CPDF_TextPageFind();

  bool FindNext();
  bool FindPrev();
  int GetCurOrder() const;
  int GetMatchedCount() const;

 private:
  CPDF_TextPageFind(const CPDF_TextPage* pTextPage,
                    const std::vector<WideString>& findwhat_array,
                    const Options& options,
                    std::optional<size_t> startPos);

  // Should be called immediately after construction.
  bool FindFirst();

  int GetCharIndex(int index) const;

  UnownedPtr<const CPDF_TextPage> const text_page_;
  const WideString str_text_;
  const std::vector<WideString> find_what_array_;
  std::optional<size_t> find_next_start_;
  std::optional<size_t> find_pre_start_;
  int res_start_ = 0;
  int res_end_ = -1;
  const Options options_;
};

#endif  // CORE_FPDFTEXT_CPDF_TEXTPAGEFIND_H_
