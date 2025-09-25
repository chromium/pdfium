// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PARSER_CPDF_PARSER_H_
#define CORE_FPDFAPI_PARSER_CPDF_PARSER_H_

#include <stddef.h>
#include <stdint.h>

#include <limits>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "core/fpdfapi/parser/cpdf_cross_ref_table.h"
#include "core/fpdfapi/parser/cpdf_indirect_object_holder.h"
#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/fx_types.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"

class CPDF_Array;
class CPDF_Dictionary;
class CPDF_LinearizedHeader;
class CPDF_Object;
class CPDF_ObjectStream;
class CPDF_ReadValidator;
class CPDF_SecurityHandler;
class CPDF_SyntaxParser;
class IFX_ArchiveStream;
class IFX_SeekableReadStream;

class CPDF_Parser {
 public:
  class ParsedObjectsHolder : public CPDF_IndirectObjectHolder {
   public:
    virtual bool TryInit() = 0;
  };

  enum Error {
    SUCCESS = 0,
    FILE_ERROR,
    FORMAT_ERROR,
    PASSWORD_ERROR,
    HANDLER_ERROR
  };

  // A limit on the maximum object number in the xref table, so objects need to
  // be in the range [0, kMaxObjectNumber].
  //
  // Theoretical limits are higher, but this may be large enough in practice.
  //
  // Note: This was 1M, but https://crbug.com/910009 encountered a PDF with
  // object numbers in the 1.7M range. The PDF only has 10K objects, but they
  // are non-consecutive.
  //
  // This increased to 4M, but some PDFs found in the wild have larger object
  // numbers than that. e.g. One PDF had a 38 MB object stream with 6 bytes
  // per entry. Another sample had a 41 MB object stream with 7 bytes per entry.
  static constexpr uint32_t kMaxObjectNumber = 24 * 1024 * 1024;

  static constexpr size_t kInvalidPos = std::numeric_limits<size_t>::max();

  explicit CPDF_Parser(ParsedObjectsHolder* holder);
  CPDF_Parser();
  ~CPDF_Parser();

  Error StartParse(RetainPtr<IFX_SeekableReadStream> pFile,
                   const ByteString& password);
  Error StartLinearizedParse(RetainPtr<CPDF_ReadValidator> validator,
                             const ByteString& password);

  ByteString GetPassword() const { return password_; }

  // Take the GetPassword() value and encode it, if necessary, based on the
  // password encoding conversion.
  ByteString GetEncodedPassword() const;

  const CPDF_Dictionary* GetTrailer() const;
  uint32_t GetTrailerObjectNumber() const;

  // Returns a new trailer which combines the last read trailer with the /Root
  // and /Info from previous ones.
  RetainPtr<CPDF_Dictionary> GetCombinedTrailer() const;

  FX_FILESIZE GetLastXRefOffset() const { return last_xref_offset_; }

  uint32_t GetPermissions(bool get_owner_perms) const;
  uint32_t GetRootObjNum() const;
  uint32_t GetInfoObjNum() const;
  RetainPtr<const CPDF_Array> GetIDArray() const;
  RetainPtr<const CPDF_Dictionary> GetEncryptDict() const;

  RetainPtr<CPDF_Object> ParseIndirectObject(uint32_t objnum);

  uint32_t GetLastObjNum() const;
  bool IsValidObjectNumber(uint32_t objnum) const;
  FX_FILESIZE GetObjectPositionOrZero(uint32_t objnum) const;
  const RetainPtr<CPDF_SecurityHandler>& GetSecurityHandler() const {
    return security_handler_;
  }
  bool IsObjectFree(uint32_t objnum) const;

  int GetFileVersion() const { return file_version_; }
  bool IsXRefStream() const { return xref_stream_; }

  FX_FILESIZE GetDocumentSize() const;
  uint32_t GetFirstPageNo() const;
  const CPDF_LinearizedHeader* GetLinearizedHeader() const {
    return linearized_.get();
  }

  bool xref_table_rebuilt() const { return xref_table_rebuilt_; }

  std::vector<unsigned int> GetTrailerEnds();
  bool WriteToArchive(IFX_ArchiveStream* archive, FX_FILESIZE src_size);

  const CPDF_CrossRefTable* GetCrossRefTableForTesting() const {
    return cross_ref_table_.get();
  }

  CPDF_Dictionary* GetMutableTrailerForTesting();

  RetainPtr<CPDF_Object> ParseIndirectObjectAtForTesting(FX_FILESIZE pos) {
    return ParseIndirectObjectAt(pos, 0);
  }

  void SetLinearizedHeaderForTesting(
      std::unique_ptr<CPDF_LinearizedHeader> pLinearized);

 protected:
  bool LoadCrossRefTable(FX_FILESIZE pos, bool skip);
  bool RebuildCrossRef();
  Error StartParseInternal();
  FX_FILESIZE ParseStartXRef();
  std::unique_ptr<CPDF_LinearizedHeader> ParseLinearizedHeader();

  void SetSyntaxParserForTesting(std::unique_ptr<CPDF_SyntaxParser> parser);

 private:
  friend class CPDF_DataAvail;

  struct CrossRefObjData {
    uint32_t obj_num = 0;
    CPDF_CrossRefTable::ObjectInfo info;
  };

  bool LoadAllCrossRefTablesAndStreams(FX_FILESIZE xref_offset);
  bool FindAllCrossReferenceTablesAndStream(
      FX_FILESIZE main_xref_offset,
      std::vector<FX_FILESIZE>& xref_list,
      std::vector<FX_FILESIZE>& xref_stream_list);
  bool LoadCrossRefStream(FX_FILESIZE* pos, bool is_main_xref);
  void ProcessCrossRefStreamEntry(pdfium::span<const uint8_t> entry_span,
                                  pdfium::span<const uint32_t> field_widths,
                                  uint32_t obj_num);
  RetainPtr<CPDF_Dictionary> LoadTrailer();
  Error SetEncryptHandler();
  void ReleaseEncryptHandler();
  bool LoadLinearizedAllCrossRefTable(FX_FILESIZE main_xref_offset);
  bool LoadLinearizedAllCrossRefStream(FX_FILESIZE main_xref_offset);
  Error LoadLinearizedMainXRefTable();

  const CPDF_ObjectStream* GetObjectStream(uint32_t object_number);
  RetainPtr<const CPDF_Dictionary> GetRoot() const;

  // A simple check whether the cross reference table matches with
  // the objects.
  bool VerifyCrossRefTable();

  RetainPtr<CPDF_Object> ParseIndirectObjectAt(FX_FILESIZE pos,
                                               uint32_t objnum);

  // If out_objects is null, the parser position will be moved to end subsection
  // without additional validation.
  bool ParseAndAppendCrossRefSubsectionData(
      uint32_t start_objnum,
      uint32_t count,
      std::vector<CrossRefObjData>* out_objects);
  bool ParseCrossRefTable(std::vector<CrossRefObjData>* out_objects);
  void MergeCrossRefObjectsData(const std::vector<CrossRefObjData>& objects);

  bool InitSyntaxParser(RetainPtr<CPDF_ReadValidator> validator);
  bool ParseFileVersion();
  void SetPassword(const ByteString& password) { password_ = password; }

  std::unique_ptr<CPDF_SyntaxParser> syntax_;
  std::unique_ptr<ParsedObjectsHolder> owned_objects_holder_;
  UnownedPtr<ParsedObjectsHolder> objects_holder_;

  bool has_parsed_ = false;
  bool xref_stream_ = false;
  bool xref_table_rebuilt_ = false;
  int file_version_ = 0;
  uint32_t metadata_objnum_ = 0;
  // cross_ref_table_ must be destroyed after security_handler_ due to the
  // ownership of the ID array data.
  std::unique_ptr<CPDF_CrossRefTable> cross_ref_table_;
  FX_FILESIZE last_xref_offset_ = 0;
  ByteString password_;
  std::unique_ptr<CPDF_LinearizedHeader> linearized_;

  // A map of object numbers to indirect streams.
  std::map<uint32_t, std::unique_ptr<CPDF_ObjectStream>> object_stream_map_;

  // All indirect object numbers that are being parsed.
  std::set<uint32_t> parsing_obj_nums_;

  RetainPtr<CPDF_SecurityHandler> security_handler_;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_PARSER_H_
