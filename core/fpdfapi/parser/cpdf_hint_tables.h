// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PARSER_CPDF_HINT_TABLES_H_
#define CORE_FPDFAPI_PARSER_CPDF_HINT_TABLES_H_

#include <memory>
#include <vector>

#include "core/fpdfapi/parser/cpdf_data_avail.h"
#include "core/fxcrt/unowned_ptr.h"

class CFX_BitStream;
class CPDF_LinearizedHeader;
class CPDF_ReadValidator;
class CPDF_Stream;
class CPDF_SyntaxParser;

class CPDF_HintTables {
 public:
  struct SharedObjGroupInfo {
    FX_FILESIZE offset_ = 0;
    uint32_t length_ = 0;
    uint32_t objects_count_ = 0;
    uint32_t start_obj_num_ = 0;
  };

  class PageInfo {
   public:
    PageInfo();
    ~PageInfo();

    void set_objects_count(uint32_t objects_count) {
      objects_count_ = objects_count;
    }
    uint32_t objects_count() const { return objects_count_; }

    void set_page_offset(FX_FILESIZE offset) { offset_ = offset; }
    FX_FILESIZE page_offset() const { return offset_; }

    void set_page_length(uint32_t length) { length_ = length; }
    uint32_t page_length() const { return length_; }

    void set_start_obj_num(uint32_t start_obj_num) {
      start_obj_num_ = start_obj_num;
    }
    uint32_t start_obj_num() const { return start_obj_num_; }

    void AddIdentifier(uint32_t Identifier) {
      identifier_array_.push_back(Identifier);
    }

    const std::vector<uint32_t>& Identifiers() const {
      return identifier_array_;
    }

   private:
    uint32_t objects_count_ = 0;
    FX_FILESIZE offset_ = 0;
    uint32_t length_ = 0;
    uint32_t start_obj_num_ = 0;
    std::vector<uint32_t> identifier_array_;

    PageInfo(const PageInfo& other) = delete;
    PageInfo& operator=(const PageInfo&) = delete;
  };

  static std::unique_ptr<CPDF_HintTables> Parse(
      CPDF_SyntaxParser* parser,
      const CPDF_LinearizedHeader* pLinearized);

  CPDF_HintTables(CPDF_ReadValidator* pValidator,
                  const CPDF_LinearizedHeader* pLinearized);
  virtual ~CPDF_HintTables();

  bool GetPagePos(uint32_t index,
                  FX_FILESIZE* szPageStartPos,
                  FX_FILESIZE* szPageLength,
                  uint32_t* dwObjNum) const;

  CPDF_DataAvail::DocAvailStatus CheckPage(uint32_t index);

  bool LoadHintStream(CPDF_Stream* pHintStream);

  const std::vector<PageInfo>& PageInfos() const { return page_infos_; }
  const std::vector<SharedObjGroupInfo>& SharedGroupInfos() const {
    return shared_obj_group_infos_;
  }

  FX_FILESIZE GetFirstPageObjOffset() const { return first_page_obj_offset_; }

 protected:
  bool ReadPageHintTable(CFX_BitStream* hStream);
  bool ReadSharedObjHintTable(CFX_BitStream* hStream, uint32_t offset);

 private:
  FX_FILESIZE HintsOffsetToFileOffset(uint32_t hints_offset) const;

  uint32_t first_page_shared_objs_ = 0;
  FX_FILESIZE first_page_obj_offset_ = 0;
  std::vector<PageInfo> page_infos_;
  std::vector<SharedObjGroupInfo> shared_obj_group_infos_;
  UnownedPtr<CPDF_ReadValidator> validator_;
  UnownedPtr<const CPDF_LinearizedHeader> const linearized_;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_HINT_TABLES_H_
