// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPDF_STRUCTTREE_H_
#define CORE_FPDFDOC_CPDF_STRUCTTREE_H_

#include <functional>
#include <map>
#include <memory>
#include <vector>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/retain_ptr.h"

class CPDF_Document;
class CPDF_StructElement;

class CPDF_StructTree {
 public:
  static std::unique_ptr<CPDF_StructTree> LoadPage(
      const CPDF_Document* doc,
      RetainPtr<const CPDF_Dictionary> pPageDict);

  explicit CPDF_StructTree(const CPDF_Document* doc);
  ~CPDF_StructTree();

  size_t CountTopElements() const { return kids_.size(); }
  CPDF_StructElement* GetTopElement(size_t i) const { return kids_[i].Get(); }
  uint32_t GetPageObjNum() const { return page_->GetObjNum(); }
  ByteString GetRoleMapNameFor(ByteStringView type) const;

 private:
  using StructElementMap = std::map<RetainPtr<const CPDF_Dictionary>,
                                    RetainPtr<CPDF_StructElement>,
                                    std::less<>>;

  void LoadPageTree(RetainPtr<const CPDF_Dictionary> pPageDict);
  RetainPtr<CPDF_StructElement> AddPageNode(
      RetainPtr<const CPDF_Dictionary> dict,
      StructElementMap* map,
      int nLevel);
  bool AddTopLevelNode(const CPDF_Dictionary* dict,
                       const RetainPtr<CPDF_StructElement>& pElement);

  RetainPtr<const CPDF_Dictionary> const tree_root_;
  RetainPtr<const CPDF_Dictionary> const role_map_;
  RetainPtr<const CPDF_Dictionary> page_;
  std::vector<RetainPtr<CPDF_StructElement>> kids_;
};

#endif  // CORE_FPDFDOC_CPDF_STRUCTTREE_H_
