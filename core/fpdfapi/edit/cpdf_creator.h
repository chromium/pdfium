// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_EDIT_CPDF_CREATOR_H_
#define CORE_FPDFAPI_EDIT_CPDF_CREATOR_H_

#include <map>
#include <memory>
#include <vector>

#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"

class CPDF_Array;
class CPDF_CryptoHandler;
class CPDF_SecurityHandler;
class CPDF_Dictionary;
class CPDF_Document;
class CPDF_Object;
class CPDF_Parser;

#define FPDFCREATE_INCREMENTAL 1
#define FPDFCREATE_NO_ORIGINAL 2

class CPDF_Creator {
 public:
  CPDF_Creator(CPDF_Document* doc,
               RetainPtr<IFX_RetainableWriteStream> archive);
  ~CPDF_Creator();

  void RemoveSecurity();
  bool Create(uint32_t flags);
  bool SetFileVersion(int32_t fileVersion);

 private:
  enum class Stage {
    kInvalid = -1,
    kInit0 = 0,
    kWriteHeader10 = 10,
    kWriteIncremental15 = 15,
    kInitWriteObjs20 = 20,
    kWriteOldObjs21 = 21,
    kInitWriteNewObjs25 = 25,
    kWriteNewObjs26 = 26,
    kWriteEncryptDict27 = 27,
    kInitWriteXRefs80 = 80,
    kWriteXrefsNotIncremental81 = 81,
    kWriteXrefsIncremental82 = 82,
    kWriteTrailerAndFinish90 = 90,
    kComplete100 = 100,
  };

  bool Continue();
  void Clear();

  void InitNewObjNumOffsets();
  void InitID();

  CPDF_Creator::Stage WriteDoc_Stage1();
  CPDF_Creator::Stage WriteDoc_Stage2();
  CPDF_Creator::Stage WriteDoc_Stage3();
  CPDF_Creator::Stage WriteDoc_Stage4();

  bool WriteOldIndirectObject(uint32_t objnum);
  bool WriteOldObjs();
  bool WriteNewObjs();
  bool WriteIndirectObj(uint32_t objnum, const CPDF_Object* pObj);

  CPDF_CryptoHandler* GetCryptoHandler();

  UnownedPtr<CPDF_Document> const document_;
  UnownedPtr<CPDF_Parser> const parser_;
  RetainPtr<const CPDF_Dictionary> encrypt_dict_;
  RetainPtr<CPDF_Dictionary> new_encrypt_dict_;
  RetainPtr<CPDF_SecurityHandler> security_handler_;
  uint32_t last_obj_num_;
  std::unique_ptr<IFX_ArchiveStream> archive_;
  FX_FILESIZE saved_offset_ = 0;
  Stage stage_ = Stage::kInvalid;
  uint32_t cur_obj_num_ = 0;
  FX_FILESIZE xref_start_ = 0;
  std::map<uint32_t, FX_FILESIZE> object_offsets_;
  std::vector<uint32_t> new_obj_num_array_;  // Sorted, ascending.
  RetainPtr<CPDF_Array> id_array_;
  int32_t file_version_ = 0;
  bool security_changed_ = false;
  bool is_incremental_ = false;
  bool is_original_ = false;
};

#endif  // CORE_FPDFAPI_EDIT_CPDF_CREATOR_H_
