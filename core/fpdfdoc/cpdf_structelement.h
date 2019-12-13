// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPDF_STRUCTELEMENT_H_
#define CORE_FPDFDOC_CPDF_STRUCTELEMENT_H_

#include <vector>

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"

class CPDF_Dictionary;
class CPDF_Object;
class CPDF_StructElement;
class CPDF_StructTree;

class CPDF_StructKid {
 public:
  enum Type { kInvalid, kElement, kPageContent, kStreamContent, kObject };

  CPDF_StructKid();
  CPDF_StructKid(const CPDF_StructKid& that);
  ~CPDF_StructKid();

  Type m_Type = kInvalid;
  uint32_t m_PageObjNum = 0;  // For {PageContent, StreamContent, Object} types.
  uint32_t m_RefObjNum = 0;   // For {StreamContent, Object} types.
  uint32_t m_ContentId = 0;   // For {PageContent, StreamContent} types.
  RetainPtr<CPDF_StructElement> m_pElement;  // For Element.
  RetainPtr<const CPDF_Dictionary> m_pDict;  // For Element.
};

class CPDF_StructElement final : public Retainable {
 public:
  template <typename T, typename... Args>
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  ByteString GetType() const { return m_Type; }
  WideString GetAltText() const;
  WideString GetTitle() const;

  // Never returns nullptr.
  const CPDF_Dictionary* GetDict() const { return m_pDict.Get(); }

  size_t CountKids() const;
  CPDF_StructElement* GetKidIfElement(size_t index) const;
  std::vector<CPDF_StructKid>* GetKids() { return &m_Kids; }

 private:
  CPDF_StructElement(CPDF_StructTree* pTree,
                     CPDF_StructElement* pParent,
                     const CPDF_Dictionary* pDict);
  ~CPDF_StructElement() override;

  void LoadKids(const CPDF_Dictionary* pDict);
  void LoadKid(uint32_t PageObjNum,
               const CPDF_Object* pKidObj,
               CPDF_StructKid* pKid);

  UnownedPtr<CPDF_StructTree> const m_pTree;
  UnownedPtr<CPDF_StructElement> const m_pParent;
  RetainPtr<const CPDF_Dictionary> const m_pDict;
  const ByteString m_Type;
  std::vector<CPDF_StructKid> m_Kids;
};

#endif  // CORE_FPDFDOC_CPDF_STRUCTELEMENT_H_
