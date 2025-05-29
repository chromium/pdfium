// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/parser/cpdf_data_avail.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_cross_ref_avail.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_hint_tables.h"
#include "core/fpdfapi/parser/cpdf_linearized_header.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_page_object_avail.h"
#include "core/fpdfapi/parser/cpdf_read_validator.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_syntax_parser.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "core/fxcrt/autorestorer.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/containers/contains.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/notreached.h"
#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxcrt/stl_util.h"

namespace {

RetainPtr<CPDF_Object> GetResourceObject(RetainPtr<CPDF_Dictionary> dict) {
  static constexpr size_t kMaxHierarchyDepth = 64;
  size_t depth = 0;

  while (dict) {
    RetainPtr<CPDF_Object> result = dict->GetMutableObjectFor("Resources");
    if (result) {
      return result;
    }
    if (++depth > kMaxHierarchyDepth) {
      // We have cycle in parents hierarchy.
      return nullptr;
    }
    RetainPtr<CPDF_Object> parent = dict->GetMutableObjectFor("Parent");
    dict = parent ? parent->GetMutableDict() : nullptr;
  }
  return nullptr;
}

class HintsScope {
 public:
  HintsScope(RetainPtr<CPDF_ReadValidator> validator,
             CPDF_DataAvail::DownloadHints* hints)
      : validator_(std::move(validator)) {
    DCHECK(validator_);
    validator_->SetDownloadHints(hints);
  }

  ~HintsScope() { validator_->SetDownloadHints(nullptr); }

 private:
  RetainPtr<CPDF_ReadValidator> validator_;
};

}  // namespace

CPDF_DataAvail::FileAvail::~FileAvail() = default;

CPDF_DataAvail::DownloadHints::~DownloadHints() = default;

CPDF_DataAvail::CPDF_DataAvail(FileAvail* pFileAvail,
                               RetainPtr<IFX_SeekableReadStream> pFileRead)
    : file_read_(pdfium::MakeRetain<CPDF_ReadValidator>(std::move(pFileRead),
                                                        pFileAvail)),
      file_len_(file_read_->GetSize()) {}

CPDF_DataAvail::~CPDF_DataAvail() {
  hint_tables_.reset();
  if (document_) {
    document_->RemoveObserver(this);
  }
}

void CPDF_DataAvail::OnObservableDestroyed() {
  document_ = nullptr;
  form_avail_.reset();
  pages_array_.clear();
  pages_obj_avail_.clear();
  pages_resources_avail_.clear();
}

CPDF_DataAvail::DocAvailStatus CPDF_DataAvail::IsDocAvail(
    DownloadHints* pHints) {
  if (!file_len_) {
    return kDataError;
  }

  DCHECK(seen_page_obj_list_.empty());
  AutoRestorer<std::set<uint32_t>> seen_objects_restorer(&seen_page_obj_list_);
  const HintsScope hints_scope(GetValidator(), pHints);
  while (!doc_avail_) {
    if (!CheckDocStatus()) {
      return kDataNotAvailable;
    }
  }

  return kDataAvailable;
}

bool CPDF_DataAvail::CheckDocStatus() {
  switch (internal_status_) {
    case InternalStatus::kHeader:
      return CheckHeader();
    case InternalStatus::kFirstPage:
      return CheckFirstPage();
    case InternalStatus::kHintTable:
      return CheckHintTables();
    case InternalStatus::kLoadAllCrossRef:
      return CheckAndLoadAllXref();
    case InternalStatus::kLoadAllFile:
      return LoadAllFile();
    case InternalStatus::kRoot:
      return CheckRoot();
    case InternalStatus::kInfo:
      return CheckInfo();
    case InternalStatus::kPageTree:
      if (total_load_page_tree_) {
        return CheckPages();
      }
      return LoadDocPages();
    case InternalStatus::kPage:
      if (total_load_page_tree_) {
        return CheckPage();
      }
      internal_status_ = InternalStatus::kPageLaterLoad;
      return true;
    case InternalStatus::kError:
      return LoadAllFile();
    case InternalStatus::kPageLaterLoad:
      internal_status_ = InternalStatus::kPage;
      [[fallthrough]];
    default:
      doc_avail_ = true;
      return true;
  }
}

bool CPDF_DataAvail::CheckPageStatus() {
  switch (internal_status_) {
    case InternalStatus::kPageTree:
      return CheckPages();
    case InternalStatus::kPage:
      return CheckPage();
    case InternalStatus::kError:
      return LoadAllFile();
    default:
      pages_tree_load_ = true;
      pages_load_ = true;
      return true;
  }
}

bool CPDF_DataAvail::LoadAllFile() {
  if (GetValidator()->CheckWholeFileAndRequestIfUnavailable()) {
    internal_status_ = InternalStatus::kDone;
    return true;
  }
  return false;
}

bool CPDF_DataAvail::CheckAndLoadAllXref() {
  if (!cross_ref_avail_) {
    CPDF_ReadValidator::ScopedSession read_session(GetValidator());
    const FX_FILESIZE last_xref_offset = parser_.ParseStartXRef();
    if (GetValidator()->has_read_problems()) {
      return false;
    }

    if (last_xref_offset <= 0) {
      internal_status_ = InternalStatus::kError;
      return false;
    }

    cross_ref_avail_ = std::make_unique<CPDF_CrossRefAvail>(GetSyntaxParser(),
                                                            last_xref_offset);
  }

  switch (cross_ref_avail_->CheckAvail()) {
    case kDataAvailable:
      break;
    case kDataNotAvailable:
      return false;
    case kDataError:
      internal_status_ = InternalStatus::kError;
      return false;
  }

  if (!parser_.LoadAllCrossRefTablesAndStreams(
          cross_ref_avail_->last_crossref_offset())) {
    internal_status_ = InternalStatus::kLoadAllFile;
    return false;
  }

  internal_status_ = InternalStatus::kRoot;
  return true;
}

RetainPtr<CPDF_Object> CPDF_DataAvail::GetObject(uint32_t objnum,
                                                 bool* pExistInFile) {
  *pExistInFile = false;
  CPDF_Parser* pParser = document_ ? document_->GetParser() : &parser_;
  if (!pParser) {
    return nullptr;
  }

  CPDF_ReadValidator::ScopedSession read_session(GetValidator());
  RetainPtr<CPDF_Object> pRet = pParser->ParseIndirectObject(objnum);
  if (!pRet) {
    return nullptr;
  }

  *pExistInFile = true;
  if (GetValidator()->has_read_problems()) {
    return nullptr;
  }

  return pRet;
}

bool CPDF_DataAvail::CheckInfo() {
  const uint32_t dwInfoObjNum = parser_.GetInfoObjNum();
  if (dwInfoObjNum == CPDF_Object::kInvalidObjNum) {
    internal_status_ = InternalStatus::kPageTree;
    return true;
  }

  CPDF_ReadValidator::ScopedSession read_session(GetValidator());
  parser_.ParseIndirectObject(dwInfoObjNum);
  if (GetValidator()->has_read_problems()) {
    return false;
  }

  internal_status_ = InternalStatus::kPageTree;
  return true;
}

bool CPDF_DataAvail::CheckRoot() {
  const uint32_t dwRootObjNum = parser_.GetRootObjNum();
  if (dwRootObjNum == CPDF_Object::kInvalidObjNum) {
    internal_status_ = InternalStatus::kError;
    return true;
  }

  CPDF_ReadValidator::ScopedSession read_session(GetValidator());
  root_ = ToDictionary(parser_.ParseIndirectObject(dwRootObjNum));
  if (GetValidator()->has_read_problems()) {
    return false;
  }

  if (!root_) {
    internal_status_ = InternalStatus::kError;
    return false;
  }

  RetainPtr<const CPDF_Reference> pRef =
      ToReference(root_->GetObjectFor("Pages"));
  if (!pRef) {
    internal_status_ = InternalStatus::kError;
    return false;
  }

  pages_obj_num_ = pRef->GetRefObjNum();
  internal_status_ = InternalStatus::kInfo;
  return true;
}

bool CPDF_DataAvail::PreparePageItem() {
  const CPDF_Dictionary* pRoot = document_->GetRoot();
  if (!pRoot) {
    internal_status_ = InternalStatus::kError;
    return false;
  }

  RetainPtr<const CPDF_Reference> pRef =
      ToReference(pRoot->GetObjectFor("Pages"));
  if (!pRef) {
    internal_status_ = InternalStatus::kError;
    return false;
  }

  pages_obj_num_ = pRef->GetRefObjNum();
  internal_status_ = InternalStatus::kPageTree;
  return true;
}

bool CPDF_DataAvail::IsFirstCheck(uint32_t dwPage) {
  return page_map_check_state_.insert(dwPage).second;
}

void CPDF_DataAvail::ResetFirstCheck(uint32_t dwPage) {
  page_map_check_state_.erase(dwPage);
}

bool CPDF_DataAvail::CheckPage() {
  std::vector<uint32_t> UnavailObjList;
  for (uint32_t dwPageObjNum : page_obj_list_) {
    bool bExists = false;
    RetainPtr<CPDF_Object> pObj = GetObject(dwPageObjNum, &bExists);
    if (!pObj) {
      if (bExists) {
        UnavailObjList.push_back(dwPageObjNum);
      }
      continue;
    }

    switch (pObj->GetType()) {
      case CPDF_Object::kArray: {
        CPDF_ArrayLocker locker(pObj->AsArray());
        for (const auto& pArrayObj : locker) {
          const CPDF_Reference* pRef = ToReference(pArrayObj.Get());
          if (pRef) {
            UnavailObjList.push_back(pRef->GetRefObjNum());
          }
        }
        break;
      }
      case CPDF_Object::kDictionary:
        if (pObj->GetDict()->GetNameFor("Type") == "Pages") {
          pages_array_.push_back(std::move(pObj));
        }
        break;
      default:
        break;
    }
  }
  page_obj_list_.clear();
  if (!UnavailObjList.empty()) {
    page_obj_list_ = std::move(UnavailObjList);
    return false;
  }
  size_t iPages = pages_array_.size();
  for (size_t i = 0; i < iPages; ++i) {
    RetainPtr<CPDF_Object> pPages = std::move(pages_array_[i]);
    if (pPages && !GetPageKids(pPages.Get())) {
      pages_array_.clear();
      internal_status_ = InternalStatus::kError;
      return false;
    }
  }
  pages_array_.clear();
  if (page_obj_list_.empty()) {
    internal_status_ = InternalStatus::kDone;
  }

  return true;
}

bool CPDF_DataAvail::GetPageKids(CPDF_Object* pPages) {
  RetainPtr<const CPDF_Dictionary> dict = pPages->GetDict();
  if (!dict) {
    return true;
  }

  RetainPtr<const CPDF_Object> pKids = dict->GetObjectFor("Kids");
  if (!pKids) {
    return true;
  }

  std::vector<uint32_t> object_numbers;
  switch (pKids->GetType()) {
    case CPDF_Object::kReference:
      object_numbers.push_back(pKids->AsReference()->GetRefObjNum());
      break;
    case CPDF_Object::kArray: {
      CPDF_ArrayLocker locker(pKids->AsArray());
      for (const auto& pArrayObj : locker) {
        const CPDF_Reference* pRef = ToReference(pArrayObj.Get());
        if (pRef) {
          object_numbers.push_back(pRef->GetRefObjNum());
        }
      }
      break;
    }
    default:
      internal_status_ = InternalStatus::kError;
      return false;
  }

  for (uint32_t num : object_numbers) {
    bool inserted = seen_page_obj_list_.insert(num).second;
    if (inserted) {
      page_obj_list_.push_back(num);
    }
  }
  return true;
}

bool CPDF_DataAvail::CheckPages() {
  bool bExists = false;
  RetainPtr<CPDF_Object> pPages = GetObject(pages_obj_num_, &bExists);
  if (!bExists) {
    internal_status_ = InternalStatus::kLoadAllFile;
    return true;
  }

  if (!pPages) {
    if (internal_status_ == InternalStatus::kError) {
      internal_status_ = InternalStatus::kLoadAllFile;
      return true;
    }
    return false;
  }

  if (!GetPageKids(pPages.Get())) {
    internal_status_ = InternalStatus::kError;
    return false;
  }

  internal_status_ = InternalStatus::kPage;
  return true;
}

bool CPDF_DataAvail::CheckHeader() {
  switch (CheckHeaderAndLinearized()) {
    case kDataAvailable:
      internal_status_ = linearized_ ? InternalStatus::kFirstPage
                                     : InternalStatus::kLoadAllCrossRef;
      return true;
    case kDataNotAvailable:
      return false;
    case kDataError:
      internal_status_ = InternalStatus::kError;
      return true;
  }
}

bool CPDF_DataAvail::CheckFirstPage() {
  if (!linearized_->GetFirstPageEndOffset() || !linearized_->GetFileSize() ||
      !linearized_->GetMainXRefTableFirstEntryOffset()) {
    internal_status_ = InternalStatus::kError;
    return false;
  }

  uint32_t dwEnd = linearized_->GetFirstPageEndOffset();
  dwEnd += 512;
  if ((FX_FILESIZE)dwEnd > file_len_) {
    dwEnd = (uint32_t)file_len_;
  }

  const FX_FILESIZE start_pos = file_len_ > 1024 ? 1024 : file_len_;
  const size_t data_size = dwEnd > 1024 ? static_cast<size_t>(dwEnd - 1024) : 0;
  if (!GetValidator()->CheckDataRangeAndRequestIfUnavailable(start_pos,
                                                             data_size)) {
    return false;
  }

  internal_status_ = InternalStatus::kHintTable;
  return true;
}

bool CPDF_DataAvail::CheckHintTables() {
  CPDF_ReadValidator::ScopedSession read_session(GetValidator());
  hint_tables_ = CPDF_HintTables::Parse(GetSyntaxParser(), linearized_.get());

  if (GetValidator()->read_error()) {
    internal_status_ = InternalStatus::kError;
    return true;
  }
  if (GetValidator()->has_unavailable_data()) {
    return false;
  }

  internal_status_ = InternalStatus::kDone;
  return true;
}

RetainPtr<CPDF_Object> CPDF_DataAvail::ParseIndirectObjectAt(
    FX_FILESIZE pos,
    uint32_t objnum,
    CPDF_IndirectObjectHolder* pObjList) const {
  const FX_FILESIZE SavedPos = GetSyntaxParser()->GetPos();
  GetSyntaxParser()->SetPos(pos);
  RetainPtr<CPDF_Object> result = GetSyntaxParser()->GetIndirectObject(
      pObjList, CPDF_SyntaxParser::ParseType::kLoose);
  GetSyntaxParser()->SetPos(SavedPos);
  return (result && (!objnum || result->GetObjNum() == objnum))
             ? std::move(result)
             : nullptr;
}

CPDF_DataAvail::DocLinearizationStatus CPDF_DataAvail::IsLinearizedPDF() {
  switch (CheckHeaderAndLinearized()) {
    case kDataAvailable:
      return linearized_ ? kLinearized : kNotLinearized;
    case kDataNotAvailable:
      return kLinearizationUnknown;
    case kDataError:
      return kNotLinearized;
  }
}

CPDF_DataAvail::DocAvailStatus CPDF_DataAvail::CheckHeaderAndLinearized() {
  if (header_avail_) {
    return kDataAvailable;
  }

  CPDF_ReadValidator::ScopedSession read_session(GetValidator());
  const std::optional<FX_FILESIZE> header_offset =
      GetHeaderOffset(GetValidator());
  if (GetValidator()->has_read_problems()) {
    return kDataNotAvailable;
  }

  if (!header_offset.has_value()) {
    return kDataError;
  }

  parser_.syntax_ = std::make_unique<CPDF_SyntaxParser>(GetValidator(),
                                                        header_offset.value());
  linearized_ = parser_.ParseLinearizedHeader();
  if (GetValidator()->has_read_problems()) {
    return kDataNotAvailable;
  }

  header_avail_ = true;
  return kDataAvailable;
}

bool CPDF_DataAvail::CheckPage(uint32_t dwPage) {
  while (true) {
    switch (internal_status_) {
      case InternalStatus::kPageTree:
        if (!LoadDocPages()) {
          return false;
        }
        break;
      case InternalStatus::kPage:
        if (!LoadDocPage(dwPage)) {
          return false;
        }
        break;
      case InternalStatus::kError:
        return LoadAllFile();
      default:
        pages_tree_load_ = true;
        pages_load_ = true;
        cur_page_dict_load_ok_ = true;
        internal_status_ = InternalStatus::kPage;
        return true;
    }
  }
}

bool CPDF_DataAvail::CheckArrayPageNode(uint32_t dwPageNo,
                                        PageNode* pPageNode) {
  bool bExists = false;
  RetainPtr<CPDF_Object> pPages = GetObject(dwPageNo, &bExists);
  if (!bExists) {
    internal_status_ = InternalStatus::kError;
    return false;
  }

  if (!pPages) {
    return false;
  }

  const CPDF_Array* pArray = pPages->AsArray();
  if (!pArray) {
    internal_status_ = InternalStatus::kError;
    return false;
  }

  pPageNode->type_ = PageNode::Type::kPages;
  for (size_t i = 0; i < pArray->size(); ++i) {
    RetainPtr<const CPDF_Reference> pKid = ToReference(pArray->GetObjectAt(i));
    if (!pKid) {
      continue;
    }

    auto pNode = std::make_unique<PageNode>();
    pNode->page_no_ = pKid->GetRefObjNum();
    pPageNode->child_nodes_.push_back(std::move(pNode));
  }
  return true;
}

bool CPDF_DataAvail::CheckUnknownPageNode(uint32_t dwPageNo,
                                          PageNode* pPageNode) {
  bool bExists = false;
  RetainPtr<CPDF_Object> pPage = GetObject(dwPageNo, &bExists);
  if (!bExists) {
    internal_status_ = InternalStatus::kError;
    return false;
  }

  if (!pPage) {
    return false;
  }

  if (pPage->IsArray()) {
    pPageNode->page_no_ = dwPageNo;
    pPageNode->type_ = PageNode::Type::kArray;
    return true;
  }

  if (!pPage->IsDictionary()) {
    internal_status_ = InternalStatus::kError;
    return false;
  }

  pPageNode->page_no_ = dwPageNo;
  RetainPtr<CPDF_Dictionary> dict = pPage->GetMutableDict();
  const ByteString type = dict->GetNameFor("Type");
  if (type == "Page") {
    pPageNode->type_ = PageNode::Type::kPage;
    return true;
  }

  if (type != "Pages") {
    internal_status_ = InternalStatus::kError;
    return false;
  }

  pPageNode->type_ = PageNode::Type::kPages;
  RetainPtr<CPDF_Object> pKids = dict->GetMutableObjectFor("Kids");
  if (!pKids) {
    internal_status_ = InternalStatus::kPage;
    return true;
  }

  switch (pKids->GetType()) {
    case CPDF_Object::kReference: {
      const CPDF_Reference* pKid = pKids->AsReference();
      auto pNode = std::make_unique<PageNode>();
      pNode->page_no_ = pKid->GetRefObjNum();
      pPageNode->child_nodes_.push_back(std::move(pNode));
      break;
    }
    case CPDF_Object::kArray: {
      const CPDF_Array* pKidsArray = pKids->AsArray();
      for (size_t i = 0; i < pKidsArray->size(); ++i) {
        RetainPtr<const CPDF_Reference> pKid =
            ToReference(pKidsArray->GetObjectAt(i));
        if (!pKid) {
          continue;
        }

        auto pNode = std::make_unique<PageNode>();
        pNode->page_no_ = pKid->GetRefObjNum();
        pPageNode->child_nodes_.push_back(std::move(pNode));
      }
      break;
    }
    default:
      break;
  }
  return true;
}

bool CPDF_DataAvail::CheckPageNode(const CPDF_DataAvail::PageNode& pageNode,
                                   int32_t iPage,
                                   int32_t& iCount,
                                   int level) {
  if (level >= kMaxPageRecursionDepth) {
    return false;
  }

  int32_t iSize = fxcrt::CollectionSize<int32_t>(pageNode.child_nodes_);
  if (iSize <= 0 || iPage >= iSize) {
    internal_status_ = InternalStatus::kError;
    return false;
  }
  for (int32_t i = 0; i < iSize; ++i) {
    PageNode* pNode = pageNode.child_nodes_[i].get();
    if (!pNode) {
      continue;
    }

    if (pNode->type_ == PageNode::Type::kUnknown) {
      // Updates the type for the unknown page node.
      if (!CheckUnknownPageNode(pNode->page_no_, pNode)) {
        return false;
      }
    }
    if (pNode->type_ == PageNode::Type::kArray) {
      // Updates a more specific type for the array page node.
      if (!CheckArrayPageNode(pNode->page_no_, pNode)) {
        return false;
      }
    }
    switch (pNode->type_) {
      case PageNode::Type::kPage:
        iCount++;
        if (iPage == iCount && document_) {
          document_->SetPageObjNum(iPage, pNode->page_no_);
        }
        break;
      case PageNode::Type::kPages:
        if (!CheckPageNode(*pNode, iPage, iCount, level + 1)) {
          return false;
        }
        break;
      case PageNode::Type::kUnknown:
      case PageNode::Type::kArray:
        // Already converted above, error if we get here.
        return false;
    }
    if (iPage == iCount) {
      internal_status_ = InternalStatus::kDone;
      return true;
    }
  }
  return true;
}

bool CPDF_DataAvail::LoadDocPage(uint32_t dwPage) {
  int iPage = pdfium::checked_cast<int>(dwPage);
  if (document_->GetPageCount() <= iPage || document_->IsPageLoaded(iPage)) {
    internal_status_ = InternalStatus::kDone;
    return true;
  }
  if (page_node_.type_ == PageNode::Type::kPage) {
    internal_status_ =
        iPage == 0 ? InternalStatus::kDone : InternalStatus::kError;
    return true;
  }
  int32_t iCount = -1;
  return CheckPageNode(page_node_, iPage, iCount, 0);
}

bool CPDF_DataAvail::CheckPageCount() {
  bool bExists = false;
  RetainPtr<CPDF_Object> pPages = GetObject(pages_obj_num_, &bExists);
  if (!bExists) {
    internal_status_ = InternalStatus::kError;
    return false;
  }
  if (!pPages) {
    return false;
  }

  RetainPtr<const CPDF_Dictionary> pPagesDict = pPages->GetDict();
  if (!pPagesDict) {
    internal_status_ = InternalStatus::kError;
    return false;
  }
  if (!pPagesDict->KeyExist("Kids")) {
    return true;
  }

  return pPagesDict->GetIntegerFor("Count") > 0;
}

bool CPDF_DataAvail::LoadDocPages() {
  if (!CheckUnknownPageNode(pages_obj_num_, &page_node_)) {
    return false;
  }

  if (CheckPageCount()) {
    internal_status_ = InternalStatus::kPage;
    return true;
  }

  total_load_page_tree_ = true;
  return false;
}

bool CPDF_DataAvail::LoadPages() {
  while (!pages_tree_load_) {
    if (!CheckPageStatus()) {
      return false;
    }
  }

  if (pages_load_) {
    return true;
  }

  document_->LoadPages();
  return false;
}

CPDF_DataAvail::DocAvailStatus CPDF_DataAvail::CheckLinearizedData() {
  if (lineared_data_ok_) {
    return kDataAvailable;
  }
  DCHECK(linearized_);
  if (!linearized_->GetMainXRefTableFirstEntryOffset() || !document_ ||
      !document_->GetParser() || !document_->GetParser()->GetTrailer()) {
    return kDataError;
  }

  if (!main_xref_load_tried_) {
    const FX_SAFE_FILESIZE prev =
        document_->GetParser()->GetTrailer()->GetIntegerFor("Prev");
    const FX_FILESIZE main_xref_offset = prev.ValueOrDefault(-1);
    if (main_xref_offset < 0) {
      return kDataError;
    }

    if (main_xref_offset == 0) {
      return kDataAvailable;
    }

    FX_SAFE_SIZE_T data_size = file_len_;
    data_size -= main_xref_offset;
    if (!data_size.IsValid()) {
      return kDataError;
    }

    if (!GetValidator()->CheckDataRangeAndRequestIfUnavailable(
            main_xref_offset, data_size.ValueOrDie())) {
      return kDataNotAvailable;
    }

    CPDF_Parser::Error eRet =
        document_->GetParser()->LoadLinearizedMainXRefTable();
    main_xref_load_tried_ = true;
    if (eRet != CPDF_Parser::SUCCESS) {
      return kDataError;
    }

    if (!PreparePageItem()) {
      return kDataNotAvailable;
    }

    main_xref_loaded_ok_ = true;
    lineared_data_ok_ = true;
  }

  return lineared_data_ok_ ? kDataAvailable : kDataNotAvailable;
}

CPDF_DataAvail::DocAvailStatus CPDF_DataAvail::IsPageAvail(
    uint32_t dwPage,
    DownloadHints* pHints) {
  if (!document_) {
    return kDataError;
  }

  const int iPage = pdfium::checked_cast<int>(dwPage);
  if (iPage >= document_->GetPageCount()) {
    // This is XFA page.
    return kDataAvailable;
  }

  if (IsFirstCheck(dwPage)) {
    cur_page_dict_load_ok_ = false;
  }

  if (pdfium::Contains(pages_load_state_, dwPage)) {
    return kDataAvailable;
  }

  const HintsScope hints_scope(GetValidator(), pHints);
  if (linearized_) {
    if (dwPage == linearized_->GetFirstPageNo()) {
      RetainPtr<const CPDF_Dictionary> pPageDict =
          document_->GetPageDictionary(iPage);
      if (!pPageDict) {
        return kDataError;
      }

      auto page_num_obj =
          std::make_pair(dwPage, std::make_unique<CPDF_PageObjectAvail>(
                                     GetValidator(), document_, pPageDict));

      CPDF_PageObjectAvail* page_obj_avail =
          pages_obj_avail_.insert(std::move(page_num_obj)).first->second.get();
      // TODO(art-snake): Check resources.
      return page_obj_avail->CheckAvail();
    }

    DocAvailStatus nResult = CheckLinearizedData();
    if (nResult != kDataAvailable) {
      return nResult;
    }

    if (hint_tables_) {
      nResult = hint_tables_->CheckPage(dwPage);
      if (nResult != kDataAvailable) {
        return nResult;
      }
      if (GetPageDictionary(dwPage)) {
        pages_load_state_.insert(dwPage);
        return kDataAvailable;
      }
    }

    if (!main_xref_loaded_ok_) {
      if (!LoadAllFile()) {
        return kDataNotAvailable;
      }
      document_->GetParser()->RebuildCrossRef();
      ResetFirstCheck(dwPage);
      return kDataAvailable;
    }
    if (total_load_page_tree_) {
      if (!LoadPages()) {
        return kDataNotAvailable;
      }
    } else {
      if (!cur_page_dict_load_ok_ && !CheckPage(dwPage)) {
        return kDataNotAvailable;
      }
    }
  } else {
    if (!total_load_page_tree_ && !cur_page_dict_load_ok_ &&
        !CheckPage(dwPage)) {
      return kDataNotAvailable;
    }
  }

  if (CheckAcroForm() == kFormNotAvailable) {
    return kDataNotAvailable;
  }

  RetainPtr<CPDF_Dictionary> pPageDict =
      document_->GetMutablePageDictionary(iPage);
  if (!pPageDict) {
    return kDataError;
  }

  {
    auto page_num_obj =
        std::make_pair(dwPage, std::make_unique<CPDF_PageObjectAvail>(
                                   GetValidator(), document_, pPageDict));
    CPDF_PageObjectAvail* page_obj_avail =
        pages_obj_avail_.insert(std::move(page_num_obj)).first->second.get();
    const DocAvailStatus status = page_obj_avail->CheckAvail();
    if (status != kDataAvailable) {
      return status;
    }
  }

  const DocAvailStatus resources_status = CheckResources(std::move(pPageDict));
  if (resources_status != kDataAvailable) {
    return resources_status;
  }

  cur_page_dict_load_ok_ = false;
  ResetFirstCheck(dwPage);
  pages_load_state_.insert(dwPage);
  return kDataAvailable;
}

CPDF_DataAvail::DocAvailStatus CPDF_DataAvail::CheckResources(
    RetainPtr<CPDF_Dictionary> page) {
  DCHECK(page);
  CPDF_ReadValidator::ScopedSession read_session(GetValidator());
  RetainPtr<CPDF_Object> resources = GetResourceObject(std::move(page));
  if (GetValidator()->has_read_problems()) {
    return kDataNotAvailable;
  }

  if (!resources) {
    return kDataAvailable;
  }

  CPDF_PageObjectAvail* resource_avail =
      pages_resources_avail_
          .insert(std::make_pair(resources,
                                 std::make_unique<CPDF_PageObjectAvail>(
                                     GetValidator(), document_, resources)))
          .first->second.get();
  return resource_avail->CheckAvail();
}

RetainPtr<CPDF_ReadValidator> CPDF_DataAvail::GetValidator() const {
  return file_read_;
}

CPDF_SyntaxParser* CPDF_DataAvail::GetSyntaxParser() const {
  return document_ ? document_->GetParser()->syntax_.get()
                   : parser_.syntax_.get();
}

int CPDF_DataAvail::GetPageCount() const {
  if (linearized_) {
    return linearized_->GetPageCount();
  }
  return document_ ? document_->GetPageCount() : 0;
}

RetainPtr<const CPDF_Dictionary> CPDF_DataAvail::GetPageDictionary(
    int index) const {
  if (!document_ || index < 0 || index >= GetPageCount()) {
    return nullptr;
  }
  RetainPtr<const CPDF_Dictionary> page = document_->GetPageDictionary(index);
  if (page) {
    return page;
  }
  if (!linearized_ || !hint_tables_) {
    return nullptr;
  }

  if (index == static_cast<int>(linearized_->GetFirstPageNo())) {
    return nullptr;
  }
  FX_FILESIZE szPageStartPos = 0;
  FX_FILESIZE szPageLength = 0;
  uint32_t dwObjNum = 0;
  const bool bPagePosGot = hint_tables_->GetPagePos(index, &szPageStartPos,
                                                    &szPageLength, &dwObjNum);
  if (!bPagePosGot || !dwObjNum) {
    return nullptr;
  }
  // We should say to the document, which object is the page.
  document_->SetPageObjNum(index, dwObjNum);
  // Page object already can be parsed in document.
  if (!document_->GetIndirectObject(dwObjNum)) {
    document_->ReplaceIndirectObjectIfHigherGeneration(
        dwObjNum, ParseIndirectObjectAt(szPageStartPos, dwObjNum, document_));
  }
  if (!ValidatePage(index)) {
    return nullptr;
  }
  return document_->GetPageDictionary(index);
}

CPDF_DataAvail::DocFormStatus CPDF_DataAvail::IsFormAvail(
    DownloadHints* pHints) {
  const HintsScope hints_scope(GetValidator(), pHints);
  return CheckAcroForm();
}

CPDF_DataAvail::DocFormStatus CPDF_DataAvail::CheckAcroForm() {
  if (!document_) {
    return kFormAvailable;
  }

  if (linearized_) {
    DocAvailStatus nDocStatus = CheckLinearizedData();
    if (nDocStatus == kDataError) {
      return kFormError;
    }
    if (nDocStatus == kDataNotAvailable) {
      return kFormNotAvailable;
    }
  }

  if (!form_avail_) {
    const CPDF_Dictionary* pRoot = document_->GetRoot();
    if (!pRoot) {
      return kFormAvailable;
    }

    RetainPtr<const CPDF_Object> pAcroForm = pRoot->GetObjectFor("AcroForm");
    if (!pAcroForm) {
      return kFormNotExist;
    }

    form_avail_ = std::make_unique<CPDF_PageObjectAvail>(
        GetValidator(), document_, std::move(pAcroForm));
  }
  switch (form_avail_->CheckAvail()) {
    case kDataError:
      return kFormError;
    case kDataNotAvailable:
      return kFormNotAvailable;
    case kDataAvailable:
      return kFormAvailable;
  }
}

bool CPDF_DataAvail::ValidatePage(uint32_t dwPage) const {
  int iPage = pdfium::checked_cast<int>(dwPage);
  RetainPtr<const CPDF_Dictionary> pPageDict =
      document_->GetPageDictionary(iPage);
  if (!pPageDict) {
    return false;
  }

  CPDF_PageObjectAvail obj_avail(GetValidator(), document_,
                                 std::move(pPageDict));
  return obj_avail.CheckAvail() == kDataAvailable;
}

std::pair<CPDF_Parser::Error, std::unique_ptr<CPDF_Document>>
CPDF_DataAvail::ParseDocument(
    std::unique_ptr<CPDF_Document::RenderDataIface> pRenderData,
    std::unique_ptr<CPDF_Document::PageDataIface> pPageData,
    const ByteString& password) {
  if (document_) {
    // We already returned parsed document.
    return std::make_pair(CPDF_Parser::HANDLER_ERROR, nullptr);
  }
  auto document = std::make_unique<CPDF_Document>(std::move(pRenderData),
                                                  std::move(pPageData));
  document->AddObserver(this);

  CPDF_ReadValidator::ScopedSession read_session(GetValidator());
  CPDF_Parser::Error error =
      document->LoadLinearizedDoc(GetValidator(), password);

  // Additional check, that all ok.
  if (GetValidator()->has_read_problems()) {
    // TODO(crbug.com/42271016): Figure out if this should be a CHECK() or the
    // DCHECK() removed.
    DCHECK(false);
    return std::make_pair(CPDF_Parser::HANDLER_ERROR, nullptr);
  }

  if (error != CPDF_Parser::SUCCESS) {
    return std::make_pair(error, nullptr);
  }

  document_ = document.get();
  return std::make_pair(CPDF_Parser::SUCCESS, std::move(document));
}

CPDF_DataAvail::PageNode::PageNode() = default;

CPDF_DataAvail::PageNode::~PageNode() = default;
