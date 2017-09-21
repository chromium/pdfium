// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_CONTENTMARK_H_
#define CORE_FPDFAPI_PAGE_CPDF_CONTENTMARK_H_

#include <vector>

#include "core/fpdfapi/page/cpdf_contentmarkitem.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/shared_copy_on_write.h"

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

  bool HasMark(const ByteStringView& mark) const;
  bool LookupMark(const ByteStringView& mark, CPDF_Dictionary*& pDict) const;
  void AddMark(const ByteString& name, CPDF_Dictionary* pDict, bool bDirect);
  void DeleteLastMark();

  bool HasRef() const { return !!m_Ref; }

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
    void AddMark(const ByteString& name,
                 CPDF_Dictionary* pDict,
                 bool bDictNeedClone);
    void DeleteLastMark();

   private:
    std::vector<CPDF_ContentMarkItem> m_Marks;
  };

  SharedCopyOnWrite<MarkData> m_Ref;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_CONTENTMARK_H_
