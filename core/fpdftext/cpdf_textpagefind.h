// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFTEXT_CPDF_TEXTPAGEFIND_H_
#define CORE_FPDFTEXT_CPDF_TEXTPAGEFIND_H_

#include <memory>
#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/unowned_ptr.h"
#include "third_party/base/optional.h"

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
      Optional<size_t> startPos);

  ~CPDF_TextPageFind();

  bool FindNext();
  bool FindPrev();
  int GetCurOrder() const;
  int GetMatchedCount() const;

 private:
  CPDF_TextPageFind(const CPDF_TextPage* pTextPage,
                    const std::vector<WideString>& findwhat_array,
                    const Options& options,
                    Optional<size_t> startPos);

  // Should be called immediately after construction.
  bool FindFirst();

  int GetCharIndex(int index) const;

  UnownedPtr<const CPDF_TextPage> const m_pTextPage;
  const WideString m_strText;
  const std::vector<WideString> m_csFindWhatArray;
  Optional<size_t> m_findNextStart;
  Optional<size_t> m_findPreStart;
  int m_resStart = 0;
  int m_resEnd = -1;
  const Options m_options;
};

#endif  // CORE_FPDFTEXT_CPDF_TEXTPAGEFIND_H_
