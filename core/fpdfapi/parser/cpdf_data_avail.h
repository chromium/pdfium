// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PARSER_CPDF_DATA_AVAIL_H_
#define CORE_FPDFAPI_PARSER_CPDF_DATA_AVAIL_H_

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"

class CPDF_CrossRefAvail;
class CPDF_Dictionary;
class CPDF_HintTables;
class CPDF_IndirectObjectHolder;
class CPDF_LinearizedHeader;
class CPDF_PageObjectAvail;
class CPDF_ReadValidator;
class CPDF_SyntaxParser;

class CPDF_DataAvail final : public Observable::ObserverIface {
 public:
  // Must match PDF_DATA_* definitions in public/fpdf_dataavail.h, but cannot
  // #include that header. fpdfsdk/fpdf_dataavail.cpp has static_asserts
  // to make sure the two sets of values match.
  enum DocAvailStatus {
    kDataError = -1,        // PDF_DATA_ERROR
    kDataNotAvailable = 0,  // PDF_DATA_NOTAVAIL
    kDataAvailable = 1,     // PDF_DATA_AVAIL
  };

  // Must match PDF_*LINEAR* definitions in public/fpdf_dataavail.h, but cannot
  // #include that header. fpdfsdk/fpdf_dataavail.cpp has static_asserts
  // to make sure the two sets of values match.
  enum DocLinearizationStatus {
    kLinearizationUnknown = -1,  // PDF_LINEARIZATION_UNKNOWN
    kNotLinearized = 0,          // PDF_NOT_LINEARIZED
    kLinearized = 1,             // PDF_LINEARIZED
  };

  // Must match PDF_FORM_* definitions in public/fpdf_dataavail.h, but cannot
  // #include that header. fpdfsdk/fpdf_dataavail.cpp has static_asserts
  // to make sure the two sets of values match.
  enum DocFormStatus {
    kFormError = -1,        // PDF_FORM_ERROR
    kFormNotAvailable = 0,  // PDF_FORM_NOTAVAIL
    kFormAvailable = 1,     // PDF_FORM_AVAIL
    kFormNotExist = 2,      // PDF_FORM_NOTEXIST
  };

  class FileAvail {
   public:
    virtual ~FileAvail();
    virtual bool IsDataAvail(FX_FILESIZE offset, size_t size) = 0;
  };

  class DownloadHints {
   public:
    virtual ~DownloadHints();
    virtual void AddSegment(FX_FILESIZE offset, size_t size) = 0;
  };

  CPDF_DataAvail(FileAvail* pFileAvail,
                 RetainPtr<IFX_SeekableReadStream> pFileRead);
  ~CPDF_DataAvail() override;

  // Observable::ObserverIface:
  void OnObservableDestroyed() override;

  DocAvailStatus IsDocAvail(DownloadHints* pHints);
  DocAvailStatus IsPageAvail(uint32_t dwPage, DownloadHints* pHints);
  DocFormStatus IsFormAvail(DownloadHints* pHints);
  DocLinearizationStatus IsLinearizedPDF();
  int GetPageCount() const;
  RetainPtr<const CPDF_Dictionary> GetPageDictionary(int index) const;
  RetainPtr<CPDF_ReadValidator> GetValidator() const;

  std::pair<CPDF_Parser::Error, std::unique_ptr<CPDF_Document>> ParseDocument(
      std::unique_ptr<CPDF_Document::RenderDataIface> pRenderData,
      std::unique_ptr<CPDF_Document::PageDataIface> pPageData,
      const ByteString& password);

  const CPDF_HintTables* GetHintTablesForTest() const {
    return hint_tables_.get();
  }

 private:
  enum class InternalStatus : uint8_t {
    kHeader = 0,
    kFirstPage,
    kHintTable,
    kLoadAllCrossRef,
    kRoot,
    kInfo,
    kPageTree,
    kPage,
    kPageLaterLoad,
    kResources,
    kDone,
    kError,
    kLoadAllFile,
  };

  class PageNode {
   public:
    enum class Type { kUnknown = 0, kPage, kPages, kArray };

    PageNode();
    ~PageNode();

    Type type_ = Type::kUnknown;
    uint32_t page_no_ = 0;
    std::vector<std::unique_ptr<PageNode>> child_nodes_;
  };

  static constexpr int kMaxPageRecursionDepth = 1024;

  bool CheckDocStatus();
  bool CheckHeader();
  bool CheckFirstPage();
  bool CheckHintTables();
  bool CheckRoot();
  bool CheckInfo();
  bool CheckPages();
  bool CheckPage();
  DocAvailStatus CheckResources(RetainPtr<CPDF_Dictionary> page);
  DocFormStatus CheckAcroForm();
  bool CheckPageStatus();

  DocAvailStatus CheckHeaderAndLinearized();
  RetainPtr<CPDF_Object> ParseIndirectObjectAt(
      FX_FILESIZE pos,
      uint32_t objnum,
      CPDF_IndirectObjectHolder* pObjList) const;
  RetainPtr<CPDF_Object> GetObject(uint32_t objnum, bool* pExistInFile);
  bool GetPageKids(CPDF_Object* pPages);
  bool PreparePageItem();
  bool LoadPages();
  bool CheckAndLoadAllXref();
  bool LoadAllFile();
  DocAvailStatus CheckLinearizedData();

  bool CheckPage(uint32_t dwPage);
  bool LoadDocPages();
  bool LoadDocPage(uint32_t dwPage);
  bool CheckPageNode(const PageNode& pageNode,
                     int32_t iPage,
                     int32_t& iCount,
                     int level);
  bool CheckUnknownPageNode(uint32_t dwPageNo, PageNode* pPageNode);
  bool CheckArrayPageNode(uint32_t dwPageNo, PageNode* pPageNode);
  bool CheckPageCount();
  bool IsFirstCheck(uint32_t dwPage);
  void ResetFirstCheck(uint32_t dwPage);
  bool ValidatePage(uint32_t dwPage) const;
  CPDF_SyntaxParser* GetSyntaxParser() const;

  RetainPtr<CPDF_ReadValidator> file_read_;
  CPDF_Parser parser_;
  RetainPtr<CPDF_Dictionary> root_;
  std::unique_ptr<CPDF_LinearizedHeader> linearized_;
  bool doc_avail_ = false;
  InternalStatus internal_status_ = InternalStatus::kHeader;
  std::unique_ptr<CPDF_CrossRefAvail> cross_ref_avail_;
  const FX_FILESIZE file_len_;
  UnownedPtr<CPDF_Document> document_;
  std::vector<uint32_t> page_obj_list_;
  std::set<uint32_t> seen_page_obj_list_;
  uint32_t pages_obj_num_ = 0;
  bool lineared_data_ok_ = false;
  bool main_xref_load_tried_ = false;
  bool main_xref_loaded_ok_ = false;
  bool pages_tree_load_ = false;
  bool pages_load_ = false;
  std::unique_ptr<CPDF_PageObjectAvail> form_avail_;
  std::vector<RetainPtr<CPDF_Object>> pages_array_;
  bool total_load_page_tree_ = false;
  bool cur_page_dict_load_ok_ = false;
  bool header_avail_ = false;
  PageNode page_node_;
  std::set<uint32_t> page_map_check_state_;
  std::set<uint32_t> pages_load_state_;
  std::unique_ptr<CPDF_HintTables> hint_tables_;
  std::map<uint32_t, std::unique_ptr<CPDF_PageObjectAvail>> pages_obj_avail_;
  std::map<RetainPtr<const CPDF_Object>,
           std::unique_ptr<CPDF_PageObjectAvail>,
           std::less<>>
      pages_resources_avail_;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_DATA_AVAIL_H_
