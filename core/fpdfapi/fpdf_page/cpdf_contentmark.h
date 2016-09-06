// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PAGE_CPDF_CONTENTMARK_H_
#define CORE_FPDFAPI_FPDF_PAGE_CPDF_CONTENTMARK_H_

#include <vector>

#include "core/fpdfapi/fpdf_page/cpdf_contentmarkitem.h"
#include "core/fxcrt/include/cfx_count_ref.h"
#include "core/fxcrt/include/fx_basic.h"
#include "core/fxcrt/include/fx_system.h"

class CPDF_Dictionary;

class CPDF_ContentMark {
 public:
  CPDF_ContentMark();
  CPDF_ContentMark(const CPDF_ContentMark& that);
  ~CPDF_ContentMark();

  void SetNull();

  int GetMCID() const;
  int CountItems() const;
  const CPDF_ContentMarkItem& GetItem(int i) const;

  bool HasMark(const CFX_ByteStringC& mark) const;
  bool LookupMark(const CFX_ByteStringC& mark, CPDF_Dictionary*& pDict) const;
  void AddMark(const CFX_ByteString& name,
               CPDF_Dictionary* pDict,
               FX_BOOL bDirect);
  void DeleteLastMark();

  explicit operator bool() const { return !!m_Ref; }

 private:
  class MarkData {
   public:
    MarkData();
    MarkData(const MarkData& src);
    ~MarkData();

    int CountItems() const;
    CPDF_ContentMarkItem& GetItem(int index);
    const CPDF_ContentMarkItem& GetItem(int index) const;

    int GetMCID() const;
    void AddMark(const CFX_ByteString& name,
                 CPDF_Dictionary* pDict,
                 FX_BOOL bDictNeedClone);
    void DeleteLastMark();

   private:
    std::vector<CPDF_ContentMarkItem> m_Marks;
  };

  CFX_CountRef<MarkData> m_Ref;
};

#endif  // CORE_FPDFAPI_FPDF_PAGE_CPDF_CONTENTMARK_H_
