// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_attachment.h"

#include <limits.h>

#include <memory>
#include <utility>

#include "constants/stream_dict_common.h"
#include "core/fdrm/fx_crypt.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fpdfdoc/cpdf_filespec.h"
#include "core/fpdfdoc/cpdf_nametree.h"
#include "core/fxcrt/cfx_datetime.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "third_party/base/numerics/safe_conversions.h"

namespace {

constexpr char kChecksumKey[] = "CheckSum";

ByteString CFXByteStringHexDecode(const ByteString& bsHex) {
  std::unique_ptr<uint8_t, FxFreeDeleter> result;
  uint32_t size = 0;
  HexDecode(bsHex.raw_span(), &result, &size);
  return ByteString(result.get(), size);
}

ByteString GenerateMD5Base16(const void* contents, const unsigned long len) {
  uint8_t digest[16];
  CRYPT_MD5Generate({static_cast<const uint8_t*>(contents), len}, digest);
  char buf[32];
  for (int i = 0; i < 16; ++i)
    FXSYS_IntToTwoHexChars(digest[i], &buf[i * 2]);

  return ByteString(buf, 32);
}

}  // namespace

FPDF_EXPORT int FPDF_CALLCONV
FPDFDoc_GetAttachmentCount(FPDF_DOCUMENT document) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return 0;

  auto name_tree = CPDF_NameTree::Create(pDoc, "EmbeddedFiles");
  return name_tree ? pdfium::base::checked_cast<int>(name_tree->GetCount()) : 0;
}

FPDF_EXPORT FPDF_ATTACHMENT FPDF_CALLCONV
FPDFDoc_AddAttachment(FPDF_DOCUMENT document, FPDF_WIDESTRING name) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;

  WideString wsName = WideStringFromFPDFWideString(name);
  if (wsName.IsEmpty())
    return nullptr;

  auto name_tree =
      CPDF_NameTree::CreateWithRootNameArray(pDoc, "EmbeddedFiles");
  if (!name_tree)
    return nullptr;

  // Set up the basic entries in the filespec dictionary.
  auto pFile = pDoc->NewIndirect<CPDF_Dictionary>();
  pFile->SetNewFor<CPDF_Name>("Type", "Filespec");
  pFile->SetNewFor<CPDF_String>("UF", wsName.AsStringView());
  pFile->SetNewFor<CPDF_String>(pdfium::stream::kF, wsName.AsStringView());

  // Add the new attachment name and filespec into the document's EmbeddedFiles.
  if (!name_tree->AddValueAndName(pFile->MakeReference(pDoc), wsName))
    return nullptr;

  // Unretained reference in public API. NOLINTNEXTLINE
  return FPDFAttachmentFromCPDFObject(pFile);
}

FPDF_EXPORT FPDF_ATTACHMENT FPDF_CALLCONV
FPDFDoc_GetAttachment(FPDF_DOCUMENT document, int index) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc || index < 0)
    return nullptr;

  auto name_tree = CPDF_NameTree::Create(pDoc, "EmbeddedFiles");
  if (!name_tree || static_cast<size_t>(index) >= name_tree->GetCount())
    return nullptr;

  WideString csName;

  // Unretained reference in public API. NOLINTNEXTLINE
  return FPDFAttachmentFromCPDFObject(
      name_tree->LookupValueAndName(index, &csName));
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFDoc_DeleteAttachment(FPDF_DOCUMENT document, int index) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc || index < 0)
    return false;

  auto name_tree = CPDF_NameTree::Create(pDoc, "EmbeddedFiles");
  if (!name_tree || static_cast<size_t>(index) >= name_tree->GetCount())
    return false;

  return name_tree->DeleteValueAndName(index);
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFAttachment_GetName(FPDF_ATTACHMENT attachment,
                       FPDF_WCHAR* buffer,
                       unsigned long buflen) {
  CPDF_Object* pFile = CPDFObjectFromFPDFAttachment(attachment);
  if (!pFile)
    return 0;

  CPDF_FileSpec spec(pdfium::WrapRetain(pFile));
  return Utf16EncodeMaybeCopyAndReturnLength(spec.GetFileName(), buffer,
                                             buflen);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFAttachment_HasKey(FPDF_ATTACHMENT attachment, FPDF_BYTESTRING key) {
  CPDF_Object* pFile = CPDFObjectFromFPDFAttachment(attachment);
  if (!pFile)
    return 0;

  CPDF_FileSpec spec(pdfium::WrapRetain(pFile));
  RetainPtr<const CPDF_Dictionary> pParamsDict = spec.GetParamsDict();
  return pParamsDict ? pParamsDict->KeyExist(key) : 0;
}

FPDF_EXPORT FPDF_OBJECT_TYPE FPDF_CALLCONV
FPDFAttachment_GetValueType(FPDF_ATTACHMENT attachment, FPDF_BYTESTRING key) {
  if (!FPDFAttachment_HasKey(attachment, key))
    return FPDF_OBJECT_UNKNOWN;

  CPDF_FileSpec spec(
      pdfium::WrapRetain(CPDFObjectFromFPDFAttachment(attachment)));
  RetainPtr<const CPDF_Object> pObj = spec.GetParamsDict()->GetObjectFor(key);
  return pObj ? pObj->GetType() : FPDF_OBJECT_UNKNOWN;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFAttachment_SetStringValue(FPDF_ATTACHMENT attachment,
                              FPDF_BYTESTRING key,
                              FPDF_WIDESTRING value) {
  CPDF_Object* pFile = CPDFObjectFromFPDFAttachment(attachment);
  if (!pFile)
    return false;

  CPDF_FileSpec spec(pdfium::WrapRetain(pFile));
  RetainPtr<CPDF_Dictionary> pParamsDict = spec.GetMutableParamsDict();
  if (!pParamsDict)
    return false;

  ByteString bsKey = key;
  ByteString bsValue = ByteStringFromFPDFWideString(value);
  bool bEncodedAsHex = bsKey == kChecksumKey;
  if (bEncodedAsHex)
    bsValue = CFXByteStringHexDecode(bsValue);

  pParamsDict->SetNewFor<CPDF_String>(bsKey, bsValue, bEncodedAsHex);
  return true;
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFAttachment_GetStringValue(FPDF_ATTACHMENT attachment,
                              FPDF_BYTESTRING key,
                              FPDF_WCHAR* buffer,
                              unsigned long buflen) {
  CPDF_Object* pFile = CPDFObjectFromFPDFAttachment(attachment);
  if (!pFile)
    return 0;

  CPDF_FileSpec spec(pdfium::WrapRetain(pFile));
  RetainPtr<const CPDF_Dictionary> pParamsDict = spec.GetParamsDict();
  if (!pParamsDict)
    return 0;

  ByteString bsKey = key;
  WideString value = pParamsDict->GetUnicodeTextFor(bsKey);
  if (bsKey == kChecksumKey && !value.IsEmpty()) {
    const CPDF_String* stringValue =
        pParamsDict->GetObjectFor(bsKey)->AsString();
    if (stringValue->IsHex()) {
      ByteString encoded =
          PDF_HexEncodeString(stringValue->GetString().AsStringView());
      value = pdfium::MakeRetain<CPDF_String>(nullptr, encoded, false)
                  ->GetUnicodeText();
    }
  }

  return Utf16EncodeMaybeCopyAndReturnLength(value, buffer, buflen);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFAttachment_SetFile(FPDF_ATTACHMENT attachment,
                       FPDF_DOCUMENT document,
                       const void* contents,
                       unsigned long len) {
  CPDF_Object* pFile = CPDFObjectFromFPDFAttachment(attachment);
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pFile || !pFile->IsDictionary() || !pDoc || len > INT_MAX)
    return false;

  // An empty content must have a zero length.
  if (!contents && len != 0)
    return false;

  // Create a dictionary for the new embedded file stream.
  auto pFileStreamDict = pdfium::MakeRetain<CPDF_Dictionary>();
  auto pParamsDict = pFileStreamDict->SetNewFor<CPDF_Dictionary>("Params");

  // Set the size of the new file in the dictionary.
  pFileStreamDict->SetNewFor<CPDF_Number>(pdfium::stream::kDL,
                                          static_cast<int>(len));
  pParamsDict->SetNewFor<CPDF_Number>("Size", static_cast<int>(len));

  // Set the creation date of the new attachment in the dictionary.
  CFX_DateTime dateTime = CFX_DateTime::Now();
  pParamsDict->SetNewFor<CPDF_String>(
      "CreationDate",
      ByteString::Format("D:%d%02d%02d%02d%02d%02d", dateTime.GetYear(),
                         dateTime.GetMonth(), dateTime.GetDay(),
                         dateTime.GetHour(), dateTime.GetMinute(),
                         dateTime.GetSecond()),
      false);

  // Set the checksum of the new attachment in the dictionary.
  pParamsDict->SetNewFor<CPDF_String>(
      kChecksumKey, CFXByteStringHexDecode(GenerateMD5Base16(contents, len)),
      true);

  // Create the file stream and have the filespec dictionary link to it.
  const uint8_t* contents_as_bytes = static_cast<const uint8_t*>(contents);
  auto pFileStream = pDoc->NewIndirect<CPDF_Stream>(
      DataVector<uint8_t>(contents_as_bytes, contents_as_bytes + len),
      std::move(pFileStreamDict));
  auto pEFDict = pFile->AsMutableDictionary()->SetNewFor<CPDF_Dictionary>("EF");
  pEFDict->SetNewFor<CPDF_Reference>("F", pDoc, pFileStream->GetObjNum());
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFAttachment_GetFile(FPDF_ATTACHMENT attachment,
                       void* buffer,
                       unsigned long buflen,
                       unsigned long* out_buflen) {
  if (!out_buflen)
    return false;

  CPDF_Object* pFile = CPDFObjectFromFPDFAttachment(attachment);
  if (!pFile)
    return false;

  CPDF_FileSpec spec(pdfium::WrapRetain(pFile));
  RetainPtr<const CPDF_Stream> pFileStream = spec.GetFileStream();
  if (!pFileStream)
    return false;

  *out_buflen = DecodeStreamMaybeCopyAndReturnLength(
      std::move(pFileStream),
      {static_cast<uint8_t*>(buffer), static_cast<size_t>(buflen)});
  return true;
}
