// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_nametree.h"

#include <utility>
#include <vector>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fxcrt/stl_util.h"
#include "third_party/base/check.h"
#include "third_party/base/ptr_util.h"

namespace {

constexpr int kNameTreeMaxRecursion = 32;

std::pair<WideString, WideString> GetNodeLimitsAndSanitize(
    CPDF_Array* pLimits) {
  DCHECK(pLimits);
  WideString csLeft = pLimits->GetUnicodeTextAt(0);
  WideString csRight = pLimits->GetUnicodeTextAt(1);
  // If the lower limit is greater than the upper limit, swap them.
  if (csLeft.Compare(csRight) > 0) {
    pLimits->SetNewAt<CPDF_String>(0, csRight);
    pLimits->SetNewAt<CPDF_String>(1, csLeft);
    csLeft = pLimits->GetUnicodeTextAt(0);
    csRight = pLimits->GetUnicodeTextAt(1);
  }
  while (pLimits->size() > 2)
    pLimits->RemoveAt(pLimits->size() - 1);
  return {csLeft, csRight};
}

// Get the limit arrays that leaf array |pFind| is under in the tree with root
// |pNode|. |pLimits| will hold all the limit arrays from the leaf up to before
// the root. Return true if successful.
bool GetNodeAncestorsLimitsInternal(CPDF_Dictionary* pNode,
                                    const CPDF_Array* pFind,
                                    int nLevel,
                                    std::vector<CPDF_Array*>* pLimits) {
  if (nLevel > kNameTreeMaxRecursion)
    return false;

  if (pNode->GetArrayFor("Names") == pFind) {
    pLimits->push_back(pNode->GetArrayFor("Limits"));
    return true;
  }

  CPDF_Array* pKids = pNode->GetArrayFor("Kids");
  if (!pKids)
    return false;

  for (size_t i = 0; i < pKids->size(); ++i) {
    CPDF_Dictionary* pKid = pKids->GetDictAt(i);
    if (!pKid)
      continue;

    if (GetNodeAncestorsLimitsInternal(pKid, pFind, nLevel + 1, pLimits)) {
      pLimits->push_back(pNode->GetArrayFor("Limits"));
      return true;
    }
  }
  return false;
}

// Wrapper for GetNodeAncestorsLimitsInternal() so callers do not need to know
// about the details.
std::vector<CPDF_Array*> GetNodeAncestorsLimits(CPDF_Dictionary* pNode,
                                                const CPDF_Array* pFind) {
  std::vector<CPDF_Array*> results;
  GetNodeAncestorsLimitsInternal(pNode, pFind, 0, &results);
  return results;
}

// Upon the deletion of |csName| from leaf array |pFind|, update the ancestors
// of |pFind|. Specifically, the limits of |pFind|'s ancestors will be updated
// if needed, and any ancestors that are now empty will be removed.
bool UpdateNodesAndLimitsUponDeletion(CPDF_Dictionary* pNode,
                                      const CPDF_Array* pFind,
                                      const WideString& csName,
                                      int nLevel) {
  if (nLevel > kNameTreeMaxRecursion)
    return false;

  CPDF_Array* pLimits = pNode->GetArrayFor("Limits");
  WideString csLeft;
  WideString csRight;
  if (pLimits)
    std::tie(csLeft, csRight) = GetNodeLimitsAndSanitize(pLimits);

  CPDF_Array* pNames = pNode->GetArrayFor("Names");
  if (pNames) {
    if (pNames != pFind)
      return false;
    if (pNames->IsEmpty() || !pLimits)
      return true;
    if (csLeft != csName && csRight != csName)
      return true;

    // Since |csName| defines |pNode|'s limits, we need to loop through the
    // names to find the new lower and upper limits.
    WideString csNewLeft = csRight;
    WideString csNewRight = csLeft;
    for (size_t i = 0; i < pNames->size() / 2; ++i) {
      WideString wsName = pNames->GetUnicodeTextAt(i * 2);
      if (wsName.Compare(csNewLeft) < 0)
        csNewLeft = wsName;
      if (wsName.Compare(csNewRight) > 0)
        csNewRight = wsName;
    }
    pLimits->SetNewAt<CPDF_String>(0, csNewLeft);
    pLimits->SetNewAt<CPDF_String>(1, csNewRight);
    return true;
  }

  CPDF_Array* pKids = pNode->GetArrayFor("Kids");
  if (!pKids)
    return false;

  // Loop through the kids to find the leaf array |pFind|.
  for (size_t i = 0; i < pKids->size(); ++i) {
    CPDF_Dictionary* pKid = pKids->GetDictAt(i);
    if (!pKid)
      continue;
    if (!UpdateNodesAndLimitsUponDeletion(pKid, pFind, csName, nLevel + 1))
      continue;

    // Remove this child node if it's empty.
    if ((pKid->KeyExist("Names") && pKid->GetArrayFor("Names")->IsEmpty()) ||
        (pKid->KeyExist("Kids") && pKid->GetArrayFor("Kids")->IsEmpty())) {
      pKids->RemoveAt(i);
    }
    if (pKids->IsEmpty() || !pLimits)
      return true;
    if (csLeft != csName && csRight != csName)
      return true;

    // Since |csName| defines |pNode|'s limits, we need to loop through the
    // kids to find the new lower and upper limits.
    WideString csNewLeft = csRight;
    WideString csNewRight = csLeft;
    for (size_t j = 0; j < pKids->size(); ++j) {
      CPDF_Array* pKidLimits = pKids->GetDictAt(j)->GetArrayFor("Limits");
      DCHECK(pKidLimits);
      if (pKidLimits->GetUnicodeTextAt(0).Compare(csNewLeft) < 0)
        csNewLeft = pKidLimits->GetUnicodeTextAt(0);
      if (pKidLimits->GetUnicodeTextAt(1).Compare(csNewRight) > 0)
        csNewRight = pKidLimits->GetUnicodeTextAt(1);
    }
    pLimits->SetNewAt<CPDF_String>(0, csNewLeft);
    pLimits->SetNewAt<CPDF_String>(1, csNewRight);
    return true;
  }
  return false;
}

// Search for |csName| in the tree with root |pNode|. If successful, return the
// value that |csName| points to; |nIndex| will be the index of |csName|,
// |ppFind| will be the leaf array that |csName| is found in, and |pFindIndex|
// will be the index of |csName| in |ppFind|. If |csName| is not found, |ppFind|
// will be the leaf array that |csName| should be added to, and |pFindIndex|
// will be the index that it should be added at.
CPDF_Object* SearchNameNodeByNameInternal(CPDF_Dictionary* pNode,
                                          const WideString& csName,
                                          int nLevel,
                                          size_t* nIndex,
                                          CPDF_Array** ppFind,
                                          int* pFindIndex) {
  if (nLevel > kNameTreeMaxRecursion)
    return nullptr;

  CPDF_Array* pLimits = pNode->GetArrayFor("Limits");
  CPDF_Array* pNames = pNode->GetArrayFor("Names");
  if (pLimits) {
    WideString csLeft;
    WideString csRight;
    std::tie(csLeft, csRight) = GetNodeLimitsAndSanitize(pLimits);
    // Skip this node if the name to look for is smaller than its lower limit.
    if (csName.Compare(csLeft) < 0)
      return nullptr;

    // Skip this node if the name to look for is greater than its higher limit,
    // and the node itself is a leaf node.
    if (csName.Compare(csRight) > 0 && pNames) {
      if (ppFind)
        *ppFind = pNames;
      if (pFindIndex)
        *pFindIndex = fxcrt::CollectionSize<int32_t>(*pNames) / 2 - 1;
      return nullptr;
    }
  }

  // If the node is a leaf node, look for the name in its names array.
  if (pNames) {
    size_t dwCount = pNames->size() / 2;
    for (size_t i = 0; i < dwCount; i++) {
      WideString csValue = pNames->GetUnicodeTextAt(i * 2);
      int32_t iCompare = csValue.Compare(csName);
      if (iCompare > 0)
        break;
      if (ppFind)
        *ppFind = pNames;
      if (pFindIndex)
        *pFindIndex = pdfium::base::checked_cast<int32_t>(i);
      if (iCompare < 0)
        continue;

      *nIndex += i;
      return pNames->GetDirectObjectAt(i * 2 + 1);
    }
    *nIndex += dwCount;
    return nullptr;
  }

  // Search through the node's children.
  CPDF_Array* pKids = pNode->GetArrayFor("Kids");
  if (!pKids)
    return nullptr;

  for (size_t i = 0; i < pKids->size(); i++) {
    CPDF_Dictionary* pKid = pKids->GetDictAt(i);
    if (!pKid)
      continue;

    CPDF_Object* pFound = SearchNameNodeByNameInternal(
        pKid, csName, nLevel + 1, nIndex, ppFind, pFindIndex);
    if (pFound)
      return pFound;
  }
  return nullptr;
}

// Wrapper for SearchNameNodeByNameInternal() so callers do not need to know
// about the details.
CPDF_Object* SearchNameNodeByName(CPDF_Dictionary* pNode,
                                  const WideString& csName,
                                  CPDF_Array** ppFind,
                                  int* pFindIndex) {
  size_t nIndex = 0;
  return SearchNameNodeByNameInternal(pNode, csName, 0, &nIndex, ppFind,
                                      pFindIndex);
}

struct IndexSearchResult {
  // For the n-th object in a tree, the key and value.
  WideString key;
  CPDF_Object* value;
  // The leaf node that holds `key` and `value`.
  CPDF_Array* container;
  // The index for `key` in `container`. Must be even.
  size_t index;
};

// Find the `nTargetPairIndex` node in the tree with root `pNode`. `nLevel`
// tracks the recursion level and `nCurPairIndex` tracks the progress towards
// `nTargetPairIndex`.
absl::optional<IndexSearchResult> SearchNameNodeByIndexInternal(
    CPDF_Dictionary* pNode,
    size_t nTargetPairIndex,
    int nLevel,
    size_t* nCurPairIndex) {
  if (nLevel > kNameTreeMaxRecursion)
    return absl::nullopt;

  CPDF_Array* pNames = pNode->GetArrayFor("Names");
  if (pNames) {
    size_t nCount = pNames->size() / 2;
    if (nTargetPairIndex >= *nCurPairIndex + nCount) {
      *nCurPairIndex += nCount;
      return absl::nullopt;
    }

    size_t index = 2 * (nTargetPairIndex - *nCurPairIndex);
    CPDF_Object* value = pNames->GetDirectObjectAt(index + 1);
    if (!value)
      return absl::nullopt;

    IndexSearchResult result;
    result.key = pNames->GetUnicodeTextAt(index);
    result.value = value;
    result.container = pNames;
    result.index = index;
    return result;
  }

  CPDF_Array* pKids = pNode->GetArrayFor("Kids");
  if (!pKids)
    return absl::nullopt;

  for (size_t i = 0; i < pKids->size(); i++) {
    CPDF_Dictionary* pKid = pKids->GetDictAt(i);
    if (!pKid)
      continue;
    absl::optional<IndexSearchResult> result = SearchNameNodeByIndexInternal(
        pKid, nTargetPairIndex, nLevel + 1, nCurPairIndex);
    if (result.has_value())
      return result;
  }
  return absl::nullopt;
}

// Wrapper for SearchNameNodeByIndexInternal() so callers do not need to know
// about the details.
absl::optional<IndexSearchResult> SearchNameNodeByIndex(
    CPDF_Dictionary* pNode,
    size_t nTargetPairIndex) {
  size_t nCurPairIndex = 0;
  return SearchNameNodeByIndexInternal(pNode, nTargetPairIndex, 0,
                                       &nCurPairIndex);
}

// Get the total number of key-value pairs in the tree with root |pNode|.
size_t CountNamesInternal(CPDF_Dictionary* pNode, int nLevel) {
  if (nLevel > kNameTreeMaxRecursion)
    return 0;

  CPDF_Array* pNames = pNode->GetArrayFor("Names");
  if (pNames)
    return pNames->size() / 2;

  CPDF_Array* pKids = pNode->GetArrayFor("Kids");
  if (!pKids)
    return 0;

  size_t nCount = 0;
  for (size_t i = 0; i < pKids->size(); i++) {
    CPDF_Dictionary* pKid = pKids->GetDictAt(i);
    if (!pKid)
      continue;

    nCount += CountNamesInternal(pKid, nLevel + 1);
  }
  return nCount;
}

CPDF_Array* GetNamedDestFromObject(CPDF_Object* obj) {
  if (!obj)
    return nullptr;
  CPDF_Array* array = obj->AsArray();
  if (array)
    return array;
  CPDF_Dictionary* dict = obj->AsDictionary();
  if (dict)
    return dict->GetArrayFor("D");
  return nullptr;
}

CPDF_Array* LookupOldStyleNamedDest(CPDF_Document* pDoc,
                                    const ByteString& name) {
  CPDF_Dictionary* pDests = pDoc->GetRoot()->GetDictFor("Dests");
  if (!pDests)
    return nullptr;
  return GetNamedDestFromObject(pDests->GetDirectObjectFor(name));
}

}  // namespace

CPDF_NameTree::CPDF_NameTree(CPDF_Dictionary* pRoot) : m_pRoot(pRoot) {
  DCHECK(m_pRoot);
}

CPDF_NameTree::~CPDF_NameTree() = default;

// static
std::unique_ptr<CPDF_NameTree> CPDF_NameTree::Create(
    CPDF_Document* pDoc,
    const ByteString& category) {
  CPDF_Dictionary* pRoot = pDoc->GetRoot();
  if (!pRoot)
    return nullptr;

  CPDF_Dictionary* pNames = pRoot->GetDictFor("Names");
  if (!pNames)
    return nullptr;

  CPDF_Dictionary* pCategory = pNames->GetDictFor(category);
  if (!pCategory)
    return nullptr;

  return pdfium::WrapUnique(new CPDF_NameTree(pCategory));  // Private ctor.
}

// static
std::unique_ptr<CPDF_NameTree> CPDF_NameTree::CreateWithRootNameArray(
    CPDF_Document* pDoc,
    const ByteString& category) {
  CPDF_Dictionary* pRoot = pDoc->GetRoot();
  if (!pRoot)
    return nullptr;

  // Retrieve the document's Names dictionary; create it if missing.
  CPDF_Dictionary* pNames = pRoot->GetDictFor("Names");
  if (!pNames) {
    pNames = pDoc->NewIndirect<CPDF_Dictionary>();
    pRoot->SetNewFor<CPDF_Reference>("Names", pDoc, pNames->GetObjNum());
  }

  // Create the |category| dictionary if missing.
  CPDF_Dictionary* pCategory = pNames->GetDictFor(category);
  if (!pCategory) {
    pCategory = pDoc->NewIndirect<CPDF_Dictionary>();
    pCategory->SetNewFor<CPDF_Array>("Names");
    pNames->SetNewFor<CPDF_Reference>(category, pDoc, pCategory->GetObjNum());
  }

  return pdfium::WrapUnique(new CPDF_NameTree(pCategory));  // Private ctor.
}

// static
std::unique_ptr<CPDF_NameTree> CPDF_NameTree::CreateForTesting(
    CPDF_Dictionary* pRoot) {
  return pdfium::WrapUnique(new CPDF_NameTree(pRoot));  // Private ctor.
}

// static
CPDF_Array* CPDF_NameTree::LookupNamedDest(CPDF_Document* pDoc,
                                           const ByteString& name) {
  CPDF_Array* dest_array = nullptr;
  std::unique_ptr<CPDF_NameTree> name_tree = Create(pDoc, "Dests");
  if (name_tree)
    dest_array = name_tree->LookupNewStyleNamedDest(name);
  if (!dest_array)
    dest_array = LookupOldStyleNamedDest(pDoc, name);
  return dest_array;
}

size_t CPDF_NameTree::GetCount() const {
  return CountNamesInternal(m_pRoot.Get(), 0);
}

bool CPDF_NameTree::AddValueAndName(RetainPtr<CPDF_Object> pObj,
                                    const WideString& name) {
  CPDF_Array* pFind = nullptr;
  int nFindIndex = -1;
  // Handle the corner case where the root node is empty. i.e. No kids and no
  // names. In which case, just insert into it and skip all the searches.
  CPDF_Array* pNames = m_pRoot->GetArrayFor("Names");
  if (pNames && pNames->IsEmpty() && !m_pRoot->GetArrayFor("Kids"))
    pFind = pNames;

  if (!pFind) {
    // Fail if the tree already contains this name or if the tree is too deep.
    if (SearchNameNodeByName(m_pRoot.Get(), name, &pFind, &nFindIndex))
      return false;
  }

  // If the returned |pFind| is a nullptr, then |name| is smaller than all
  // existing entries in the tree, and we did not find a leaf array to place
  // |name| into. We instead will find the leftmost leaf array in which to place
  // |name| and |pObj|.
  if (!pFind) {
    absl::optional<IndexSearchResult> result =
        SearchNameNodeByIndex(m_pRoot.Get(), 0);
    if (!result.has_value()) {
      // Give up if that fails too.
      return false;
    }

    pFind = result.value().container;
    DCHECK(pFind);
  }

  // Insert the name and the object into the leaf array found. Note that the
  // insertion position is right after the key-value pair returned by |index|.
  size_t nNameIndex = (nFindIndex + 1) * 2;
  size_t nValueIndex = nNameIndex + 1;
  pFind->InsertNewAt<CPDF_String>(nNameIndex, name);
  pFind->InsertAt(nValueIndex, std::move(pObj));

  // Expand the limits that the newly added name is under, if the name falls
  // outside of the limits of its leaf array or any arrays above it.
  std::vector<CPDF_Array*> all_limits =
      GetNodeAncestorsLimits(m_pRoot.Get(), pFind);
  for (auto* pLimits : all_limits) {
    if (!pLimits)
      continue;

    if (name.Compare(pLimits->GetUnicodeTextAt(0)) < 0)
      pLimits->SetNewAt<CPDF_String>(0, name);

    if (name.Compare(pLimits->GetUnicodeTextAt(1)) > 0)
      pLimits->SetNewAt<CPDF_String>(1, name);
  }
  return true;
}

bool CPDF_NameTree::DeleteValueAndName(size_t nIndex) {
  absl::optional<IndexSearchResult> result =
      SearchNameNodeByIndex(m_pRoot.Get(), nIndex);
  if (!result) {
    // Fail if the tree does not contain |nIndex|.
    return false;
  }

  // Remove the name and the object from the leaf array |pFind|.
  CPDF_Array* pFind = result.value().container;
  pFind->RemoveAt(result.value().index + 1);
  pFind->RemoveAt(result.value().index);

  // Delete empty nodes and update the limits of |pFind|'s ancestors as needed.
  UpdateNodesAndLimitsUponDeletion(m_pRoot.Get(), pFind, result.value().key, 0);
  return true;
}

CPDF_Object* CPDF_NameTree::LookupValueAndName(size_t nIndex,
                                               WideString* csName) const {
  absl::optional<IndexSearchResult> result =
      SearchNameNodeByIndex(m_pRoot.Get(), nIndex);
  if (!result) {
    csName->clear();
    return nullptr;
  }

  *csName = std::move(result.value().key);
  return result.value().value;
}

CPDF_Object* CPDF_NameTree::LookupValue(const WideString& csName) const {
  return SearchNameNodeByName(m_pRoot.Get(), csName, nullptr, nullptr);
}

CPDF_Array* CPDF_NameTree::LookupNewStyleNamedDest(const ByteString& sName) {
  return GetNamedDestFromObject(LookupValue(PDF_DecodeText(sName.raw_span())));
}
