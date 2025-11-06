// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/edit/cpdf_creator.h"

#include <stdint.h>

#include <algorithm>
#include <array>
#include <set>
#include <utility>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_crypto_handler.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_encryptor.h"
#include "core/fpdfapi/parser/cpdf_flateencoder.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fpdfapi/parser/cpdf_security_handler.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "core/fpdfapi/parser/object_tree_traversal_util.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/containers/contains.h"
#include "core/fxcrt/fixed_size_data_vector.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_random.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/raw_span.h"
#include "core/fxcrt/span_util.h"
#include "core/fxcrt/stl_util.h"

namespace {

const size_t kArchiveBufferSize = 32768;

class CFX_FileBufferArchive final : public IFX_ArchiveStream {
 public:
  explicit CFX_FileBufferArchive(RetainPtr<IFX_RetainableWriteStream> file);
  ~CFX_FileBufferArchive() override;

  bool WriteBlock(pdfium::span<const uint8_t> buffer) override;
  FX_FILESIZE CurrentOffset() const override { return offset_; }

 private:
  bool Flush();

  FX_FILESIZE offset_ = 0;
  FixedSizeDataVector<uint8_t> buffer_;
  pdfium::raw_span<uint8_t> available_;
  RetainPtr<IFX_RetainableWriteStream> const backing_file_;
};

CFX_FileBufferArchive::CFX_FileBufferArchive(
    RetainPtr<IFX_RetainableWriteStream> file)
    : buffer_(FixedSizeDataVector<uint8_t>::Uninit(kArchiveBufferSize)),
      available_(buffer_.span()),
      backing_file_(std::move(file)) {
  DCHECK(backing_file_);
}

CFX_FileBufferArchive::~CFX_FileBufferArchive() {
  Flush();
}

bool CFX_FileBufferArchive::Flush() {
  size_t used = buffer_.size() - available_.size();
  available_ = buffer_.span();
  return used == 0 || backing_file_->WriteBlock(available_.first(used));
}

bool CFX_FileBufferArchive::WriteBlock(pdfium::span<const uint8_t> buffer) {
  if (buffer.empty()) {
    return true;
  }

  pdfium::span<const uint8_t> src_span = buffer;
  while (!src_span.empty()) {
    size_t copy_size = std::min(available_.size(), src_span.size());
    available_ = fxcrt::spancpy(available_, src_span.first(copy_size));
    src_span = src_span.subspan(copy_size);
    if (available_.empty() && !Flush()) {
      return false;
    }
  }

  FX_SAFE_FILESIZE safe_offset = offset_;
  safe_offset += buffer.size();
  if (!safe_offset.IsValid()) {
    return false;
  }

  offset_ = safe_offset.ValueOrDie();
  return true;
}

std::array<uint32_t, 4> GenerateFileID(uint32_t dwSeed1, uint32_t dwSeed2) {
  void* context1 = FX_Random_MT_Start(dwSeed1);
  void* context2 = FX_Random_MT_Start(dwSeed2);
  std::array<uint32_t, 4> buffer = {
      FX_Random_MT_Generate(context1), FX_Random_MT_Generate(context1),
      FX_Random_MT_Generate(context2), FX_Random_MT_Generate(context2)};
  FX_Random_MT_Close(context1);
  FX_Random_MT_Close(context2);
  return buffer;
}

bool OutputIndex(IFX_ArchiveStream* archive, FX_FILESIZE offset) {
  return archive->WriteByte(static_cast<uint8_t>(offset >> 24)) &&
         archive->WriteByte(static_cast<uint8_t>(offset >> 16)) &&
         archive->WriteByte(static_cast<uint8_t>(offset >> 8)) &&
         archive->WriteByte(static_cast<uint8_t>(offset)) &&
         archive->WriteByte(0);
}

}  // namespace

CPDF_Creator::CPDF_Creator(CPDF_Document* doc,
                           RetainPtr<IFX_RetainableWriteStream> archive)
    : document_(doc),
      parser_(doc->GetParser()),
      encrypt_dict_(parser_ ? parser_->GetEncryptDict() : nullptr),
      security_handler_(parser_ ? parser_->GetSecurityHandler() : nullptr),
      last_obj_num_(document_->GetLastObjNum()),
      archive_(std::make_unique<CFX_FileBufferArchive>(std::move(archive))) {}

CPDF_Creator::~CPDF_Creator() = default;

bool CPDF_Creator::WriteIndirectObj(uint32_t objnum, const CPDF_Object* pObj) {
  if (!archive_->WriteDWord(objnum) || !archive_->WriteString(" 0 obj\r\n")) {
    return false;
  }

  std::unique_ptr<CPDF_Encryptor> encryptor;
  if (GetCryptoHandler() && pObj != encrypt_dict_) {
    encryptor = std::make_unique<CPDF_Encryptor>(GetCryptoHandler(), objnum);
  }

  if (!pObj->WriteTo(archive_.get(), encryptor.get())) {
    return false;
  }

  return archive_->WriteString("\r\nendobj\r\n");
}

bool CPDF_Creator::WriteOldIndirectObject(uint32_t objnum) {
  if (parser_->IsObjectFree(objnum)) {
    return true;
  }

  object_offsets_[objnum] = archive_->CurrentOffset();

  bool bExistInMap = !!document_->GetIndirectObject(objnum);
  RetainPtr<CPDF_Object> pObj = document_->GetOrParseIndirectObject(objnum);
  if (!pObj) {
    object_offsets_.erase(objnum);
    return true;
  }
  if (!WriteIndirectObj(pObj->GetObjNum(), pObj.Get())) {
    return false;
  }
  if (!bExistInMap) {
    document_->DeleteIndirectObject(objnum);
  }
  return true;
}

bool CPDF_Creator::WriteOldObjs() {
  const uint32_t nLastObjNum = parser_->GetLastObjNum();
  if (!parser_->IsValidObjectNumber(nLastObjNum)) {
    return true;
  }
  if (cur_obj_num_ > nLastObjNum) {
    return true;
  }

  const std::set<uint32_t> objects_with_refs =
      GetObjectsWithReferences(document_);
  uint32_t last_object_number_written = 0;
  for (uint32_t objnum = cur_obj_num_; objnum <= nLastObjNum; ++objnum) {
    if (!pdfium::Contains(objects_with_refs, objnum)) {
      continue;
    }
    if (!WriteOldIndirectObject(objnum)) {
      return false;
    }
    last_object_number_written = objnum;
  }
  // If there are no new objects to write, then adjust `last_obj_num_` if
  // needed to reflect the actual last object number.
  if (new_obj_num_array_.empty()) {
    last_obj_num_ = last_object_number_written;
  }
  return true;
}

bool CPDF_Creator::WriteNewObjs() {
  for (size_t i = cur_obj_num_; i < new_obj_num_array_.size(); ++i) {
    uint32_t objnum = new_obj_num_array_[i];
    RetainPtr<const CPDF_Object> pObj = document_->GetIndirectObject(objnum);
    if (!pObj) {
      continue;
    }

    object_offsets_[objnum] = archive_->CurrentOffset();
    if (!WriteIndirectObj(pObj->GetObjNum(), pObj.Get())) {
      return false;
    }
  }
  return true;
}

void CPDF_Creator::InitNewObjNumOffsets() {
  for (const auto& pair : *document_) {
    const uint32_t objnum = pair.first;
    if (pair.second->GetObjNum() == CPDF_Object::kInvalidObjNum) {
      continue;
    }

    if (!is_incremental_ && parser_ && parser_->IsValidObjectNumber(objnum) &&
        !parser_->IsObjectFree(objnum)) {
      continue;
    }
    new_obj_num_array_.insert(
        std::ranges::lower_bound(new_obj_num_array_, objnum), objnum);
  }
}

CPDF_Creator::Stage CPDF_Creator::WriteDoc_Stage1() {
  DCHECK(stage_ > Stage::kInvalid || stage_ < Stage::kInitWriteObjs20);
  if (stage_ == Stage::kInit0) {
    if (!parser_ || (security_changed_ && is_original_)) {
      is_incremental_ = false;
    }

    stage_ = Stage::kWriteHeader10;
  }
  if (stage_ == Stage::kWriteHeader10) {
    if (!is_incremental_) {
      if (!archive_->WriteString("%PDF-1.")) {
        return Stage::kInvalid;
      }

      int32_t version = 7;
      if (file_version_) {
        version = file_version_;
      } else if (parser_) {
        version = parser_->GetFileVersion();
      }

      if (!archive_->WriteDWord(version % 10) ||
          !archive_->WriteString("\r\n%\xA1\xB3\xC5\xD7\r\n")) {
        return Stage::kInvalid;
      }
      stage_ = Stage::kInitWriteObjs20;
    } else {
      saved_offset_ = parser_->GetDocumentSize();
      stage_ = Stage::kWriteIncremental15;
    }
  }
  if (stage_ == Stage::kWriteIncremental15) {
    if (is_original_ && saved_offset_ > 0) {
      if (!parser_->WriteToArchive(archive_.get(), saved_offset_)) {
        return Stage::kInvalid;
      }
    }
    if (is_original_ && parser_->GetLastXRefOffset() == 0) {
      for (uint32_t num = 0; num <= parser_->GetLastObjNum(); ++num) {
        if (parser_->IsObjectFree(num)) {
          continue;
        }

        object_offsets_[num] = parser_->GetObjectPositionOrZero(num);
      }
    }
    stage_ = Stage::kInitWriteObjs20;
  }
  InitNewObjNumOffsets();
  return stage_;
}

CPDF_Creator::Stage CPDF_Creator::WriteDoc_Stage2() {
  DCHECK(stage_ >= Stage::kInitWriteObjs20 ||
         stage_ < Stage::kInitWriteXRefs80);
  if (stage_ == Stage::kInitWriteObjs20) {
    if (!is_incremental_ && parser_) {
      cur_obj_num_ = 0;
      stage_ = Stage::kWriteOldObjs21;
    } else {
      stage_ = Stage::kInitWriteNewObjs25;
    }
  }
  if (stage_ == Stage::kWriteOldObjs21) {
    if (!WriteOldObjs()) {
      return Stage::kInvalid;
    }

    stage_ = Stage::kInitWriteNewObjs25;
  }
  if (stage_ == Stage::kInitWriteNewObjs25) {
    cur_obj_num_ = 0;
    stage_ = Stage::kWriteNewObjs26;
  }
  if (stage_ == Stage::kWriteNewObjs26) {
    if (!WriteNewObjs()) {
      return Stage::kInvalid;
    }

    stage_ = Stage::kWriteEncryptDict27;
  }
  if (stage_ == Stage::kWriteEncryptDict27) {
    if (encrypt_dict_ && encrypt_dict_->IsInline()) {
      last_obj_num_ += 1;
      FX_FILESIZE saveOffset = archive_->CurrentOffset();
      if (!WriteIndirectObj(last_obj_num_, encrypt_dict_.Get())) {
        return Stage::kInvalid;
      }

      object_offsets_[last_obj_num_] = saveOffset;
      if (is_incremental_) {
        new_obj_num_array_.push_back(last_obj_num_);
      }
    }
    stage_ = Stage::kInitWriteXRefs80;
  }
  return stage_;
}

CPDF_Creator::Stage CPDF_Creator::WriteDoc_Stage3() {
  DCHECK(stage_ >= Stage::kInitWriteXRefs80 ||
         stage_ < Stage::kWriteTrailerAndFinish90);

  uint32_t dwLastObjNum = last_obj_num_;
  if (stage_ == Stage::kInitWriteXRefs80) {
    xref_start_ = archive_->CurrentOffset();
    if (!is_incremental_ || !parser_->IsXRefStream()) {
      if (!is_incremental_ || parser_->GetLastXRefOffset() == 0) {
        ByteString str;
        str = pdfium::Contains(object_offsets_, 1)
                  ? "xref\r\n"
                  : "xref\r\n0 1\r\n0000000000 65535 f\r\n";
        if (!archive_->WriteString(str.AsStringView())) {
          return Stage::kInvalid;
        }

        cur_obj_num_ = 1;
        stage_ = Stage::kWriteXrefsNotIncremental81;
      } else {
        if (!archive_->WriteString("xref\r\n")) {
          return Stage::kInvalid;
        }

        cur_obj_num_ = 0;
        stage_ = Stage::kWriteXrefsIncremental82;
      }
    } else {
      stage_ = Stage::kWriteTrailerAndFinish90;
    }
  }
  if (stage_ == Stage::kWriteXrefsNotIncremental81) {
    ByteString str;
    uint32_t i = cur_obj_num_;
    uint32_t j;
    while (i <= dwLastObjNum) {
      while (i <= dwLastObjNum && !pdfium::Contains(object_offsets_, i)) {
        i++;
      }

      if (i > dwLastObjNum) {
        break;
      }

      j = i;
      while (j <= dwLastObjNum && pdfium::Contains(object_offsets_, j)) {
        j++;
      }

      if (i == 1) {
        str = ByteString::Format("0 %d\r\n0000000000 65535 f\r\n", j);
      } else {
        str = ByteString::Format("%d %d\r\n", i, j - i);
      }

      if (!archive_->WriteString(str.AsStringView())) {
        return Stage::kInvalid;
      }

      while (i < j) {
        str = ByteString::Format("%010d 00000 n\r\n", object_offsets_[i++]);
        if (!archive_->WriteString(str.AsStringView())) {
          return Stage::kInvalid;
        }
      }
      if (i > dwLastObjNum) {
        break;
      }
    }
    stage_ = Stage::kWriteTrailerAndFinish90;
  }
  if (stage_ == Stage::kWriteXrefsIncremental82) {
    ByteString str;
    uint32_t iCount = fxcrt::CollectionSize<uint32_t>(new_obj_num_array_);
    uint32_t i = cur_obj_num_;
    while (i < iCount) {
      size_t j = i;
      uint32_t objnum = new_obj_num_array_[i];
      while (j < iCount) {
        if (++j == iCount) {
          break;
        }
        uint32_t dwCurrent = new_obj_num_array_[j];
        if (dwCurrent - objnum > 1) {
          break;
        }
        objnum = dwCurrent;
      }
      objnum = new_obj_num_array_[i];
      if (objnum == 1) {
        str = ByteString::Format("0 %d\r\n0000000000 65535 f\r\n", j - i + 1);
      } else {
        str = ByteString::Format("%d %d\r\n", objnum, j - i);
      }

      if (!archive_->WriteString(str.AsStringView())) {
        return Stage::kInvalid;
      }

      while (i < j) {
        objnum = new_obj_num_array_[i++];
        str = ByteString::Format("%010d 00000 n\r\n", object_offsets_[objnum]);
        if (!archive_->WriteString(str.AsStringView())) {
          return Stage::kInvalid;
        }
      }
    }
    stage_ = Stage::kWriteTrailerAndFinish90;
  }
  return stage_;
}

CPDF_Creator::Stage CPDF_Creator::WriteDoc_Stage4() {
  DCHECK(stage_ >= Stage::kWriteTrailerAndFinish90);

  bool bXRefStream = is_incremental_ && parser_->IsXRefStream();
  if (!bXRefStream) {
    if (!archive_->WriteString("trailer\r\n<<")) {
      return Stage::kInvalid;
    }
  } else {
    if (!archive_->WriteDWord(document_->GetLastObjNum() + 1) ||
        !archive_->WriteString(" 0 obj <<")) {
      return Stage::kInvalid;
    }
  }

  if (parser_) {
    CPDF_DictionaryLocker locker(parser_->GetCombinedTrailer());
    for (const auto& it : locker) {
      const ByteString& key = it.first;
      const RetainPtr<CPDF_Object>& pValue = it.second;
      if (key == "Encrypt" || key == "Size" || key == "Filter" ||
          key == "Index" || key == "Length" || key == "Prev" || key == "W" ||
          key == "XRefStm" || key == "ID" || key == "DecodeParms" ||
          key == "Type") {
        continue;
      }
      if (!archive_->WriteString(("/")) ||
          !archive_->WriteString(PDF_NameEncode(key).AsStringView())) {
        return Stage::kInvalid;
      }
      if (!pValue->WriteTo(archive_.get(), nullptr)) {
        return Stage::kInvalid;
      }
    }
  } else {
    if (!archive_->WriteString("\r\n/Root ") ||
        !archive_->WriteDWord(document_->GetRoot()->GetObjNum()) ||
        !archive_->WriteString(" 0 R\r\n")) {
      return Stage::kInvalid;
    }
    if (document_->GetInfo()) {
      if (!archive_->WriteString("/Info ") ||
          !archive_->WriteDWord(document_->GetInfo()->GetObjNum()) ||
          !archive_->WriteString(" 0 R\r\n")) {
        return Stage::kInvalid;
      }
    }
  }
  if (encrypt_dict_) {
    if (!archive_->WriteString("/Encrypt")) {
      return Stage::kInvalid;
    }

    uint32_t dwObjNum = encrypt_dict_->GetObjNum();
    if (dwObjNum == 0) {
      dwObjNum = document_->GetLastObjNum() + 1;
    }
    if (!archive_->WriteString(" ") || !archive_->WriteDWord(dwObjNum) ||
        !archive_->WriteString(" 0 R ")) {
      return Stage::kInvalid;
    }
  }

  if (!archive_->WriteString("/Size ") ||
      !archive_->WriteDWord(last_obj_num_ + (bXRefStream ? 2 : 1))) {
    return Stage::kInvalid;
  }
  if (is_incremental_) {
    FX_FILESIZE prev = parser_->GetLastXRefOffset();
    if (prev) {
      if (!archive_->WriteString("/Prev ") || !archive_->WriteFilesize(prev)) {
        return Stage::kInvalid;
      }
    }
  }
  if (id_array_) {
    if (!archive_->WriteString(("/ID")) ||
        !id_array_->WriteTo(archive_.get(), nullptr)) {
      return Stage::kInvalid;
    }
  }
  if (!bXRefStream) {
    if (!archive_->WriteString(">>")) {
      return Stage::kInvalid;
    }
  } else {
    if (!archive_->WriteString("/W[0 4 1]/Index[")) {
      return Stage::kInvalid;
    }
    if (is_incremental_ && parser_ && parser_->GetLastXRefOffset() == 0) {
      uint32_t i = 0;
      for (i = 0; i < last_obj_num_; i++) {
        if (!pdfium::Contains(object_offsets_, i)) {
          continue;
        }
        if (!archive_->WriteDWord(i) || !archive_->WriteString(" 1 ")) {
          return Stage::kInvalid;
        }
      }
      if (!archive_->WriteString("]/Length ") ||
          !archive_->WriteDWord(last_obj_num_ * 5) ||
          !archive_->WriteString(">>stream\r\n")) {
        return Stage::kInvalid;
      }
      for (i = 0; i < last_obj_num_; i++) {
        auto it = object_offsets_.find(i);
        if (it == object_offsets_.end()) {
          continue;
        }
        if (!OutputIndex(archive_.get(), it->second)) {
          return Stage::kInvalid;
        }
      }
    } else {
      int count = fxcrt::CollectionSize<int>(new_obj_num_array_);
      int i = 0;
      for (i = 0; i < count; i++) {
        if (!archive_->WriteDWord(new_obj_num_array_[i]) ||
            !archive_->WriteString(" 1 ")) {
          return Stage::kInvalid;
        }
      }
      if (!archive_->WriteString("]/Length ") ||
          !archive_->WriteDWord(count * 5) ||
          !archive_->WriteString(">>stream\r\n")) {
        return Stage::kInvalid;
      }
      for (i = 0; i < count; ++i) {
        if (!OutputIndex(archive_.get(),
                         object_offsets_[new_obj_num_array_[i]])) {
          return Stage::kInvalid;
        }
      }
    }
    if (!archive_->WriteString("\r\nendstream")) {
      return Stage::kInvalid;
    }
  }

  if (!archive_->WriteString("\r\nstartxref\r\n") ||
      !archive_->WriteFilesize(xref_start_) ||
      !archive_->WriteString("\r\n%%EOF\r\n")) {
    return Stage::kInvalid;
  }

  stage_ = Stage::kComplete100;
  return stage_;
}

bool CPDF_Creator::Create(uint32_t flags) {
  is_incremental_ = !!(flags & FPDFCREATE_INCREMENTAL);
  is_original_ = !(flags & FPDFCREATE_NO_ORIGINAL);

  stage_ = Stage::kInit0;
  last_obj_num_ = document_->GetLastObjNum();
  object_offsets_.clear();
  new_obj_num_array_.clear();

  InitID();
  return Continue();
}

void CPDF_Creator::InitID() {
  DCHECK(!id_array_);

  id_array_ = pdfium::MakeRetain<CPDF_Array>();
  RetainPtr<const CPDF_Array> pOldIDArray =
      parser_ ? parser_->GetIDArray() : nullptr;
  RetainPtr<const CPDF_Object> pID1 =
      pOldIDArray ? pOldIDArray->GetObjectAt(0) : nullptr;
  if (pID1) {
    id_array_->Append(pID1->Clone());
  } else {
    std::array<uint32_t, 4> file_id =
        GenerateFileID((uint32_t)(uintptr_t)this, last_obj_num_);
    id_array_->AppendNew<CPDF_String>(pdfium::as_byte_span(file_id),
                                      CPDF_String::DataType::kIsHex);
  }

  if (pOldIDArray) {
    RetainPtr<const CPDF_Object> pID2 = pOldIDArray->GetObjectAt(1);
    if (is_incremental_ && encrypt_dict_ && pID2) {
      id_array_->Append(pID2->Clone());
      return;
    }
    std::array<uint32_t, 4> file_id =
        GenerateFileID((uint32_t)(uintptr_t)this, last_obj_num_);
    id_array_->AppendNew<CPDF_String>(pdfium::as_byte_span(file_id),
                                      CPDF_String::DataType::kIsHex);
    return;
  }

  id_array_->Append(id_array_->GetObjectAt(0)->Clone());
  if (encrypt_dict_) {
    DCHECK(parser_);
    int revision = encrypt_dict_->GetIntegerFor("R");
    if ((revision == 2 || revision == 3) &&
        encrypt_dict_->GetByteStringFor("Filter") == "Standard") {
      new_encrypt_dict_ = ToDictionary(encrypt_dict_->Clone());
      encrypt_dict_ = new_encrypt_dict_;
      security_handler_ = pdfium::MakeRetain<CPDF_SecurityHandler>();
      security_handler_->OnCreate(new_encrypt_dict_.Get(), id_array_.Get(),
                                  parser_->GetEncodedPassword());
      security_changed_ = true;
    }
  }
}

bool CPDF_Creator::Continue() {
  if (stage_ < Stage::kInit0) {
    return false;
  }

  Stage iRet = Stage::kInit0;
  while (stage_ < Stage::kComplete100) {
    if (stage_ < Stage::kInitWriteObjs20) {
      iRet = WriteDoc_Stage1();
    } else if (stage_ < Stage::kInitWriteXRefs80) {
      iRet = WriteDoc_Stage2();
    } else if (stage_ < Stage::kWriteTrailerAndFinish90) {
      iRet = WriteDoc_Stage3();
    } else {
      iRet = WriteDoc_Stage4();
    }

    if (iRet < stage_) {
      break;
    }
  }

  if (iRet <= Stage::kInit0 || stage_ == Stage::kComplete100) {
    stage_ = Stage::kInvalid;
    return iRet > Stage::kInit0;
  }

  return stage_ > Stage::kInvalid;
}

bool CPDF_Creator::SetFileVersion(int32_t fileVersion) {
  if (fileVersion < 10 || fileVersion > 17) {
    return false;
  }
  file_version_ = fileVersion;
  return true;
}

void CPDF_Creator::RemoveSecurity() {
  security_handler_.Reset();
  security_changed_ = true;
  encrypt_dict_ = nullptr;
  new_encrypt_dict_.Reset();
}

CPDF_CryptoHandler* CPDF_Creator::GetCryptoHandler() {
  return security_handler_ ? security_handler_->GetCryptoHandler() : nullptr;
}
