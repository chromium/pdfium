// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_attachment.h"

#include "core/fpdfapi/page/cpdf_streamparser.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fpdfdoc/cpdf_filespec.h"
#include "core/fpdfdoc/cpdf_nametree.h"
#include "fpdfsdk/fsdk_define.h"

DLLEXPORT int STDCALL FPDFDoc_GetAttachmentCount(FPDF_DOCUMENT document) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return 0;

  return CPDF_NameTree(pDoc, "EmbeddedFiles").GetCount();
}

DLLEXPORT FPDF_ATTACHMENT STDCALL FPDFDoc_GetAttachment(FPDF_DOCUMENT document,
                                                        int index) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc || index < 0)
    return nullptr;

  CPDF_NameTree nameTree(pDoc, "EmbeddedFiles");
  if (static_cast<size_t>(index) >= nameTree.GetCount())
    return nullptr;

  CFX_WideString csName;
  return nameTree.LookupValueAndName(index, &csName);
}

DLLEXPORT unsigned long STDCALL
FPDFAttachment_GetName(FPDF_ATTACHMENT attachment,
                       void* buffer,
                       unsigned long buflen) {
  CPDF_Object* pFile = CPDFObjectFromFPDFAttachment(attachment);
  if (!pFile)
    return 0;

  return Utf16EncodeMaybeCopyAndReturnLength(CPDF_FileSpec(pFile).GetFileName(),
                                             buffer, buflen);
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAttachment_HasKey(FPDF_ATTACHMENT attachment,
                                                  FPDF_WIDESTRING key) {
  CPDF_Object* pFile = CPDFObjectFromFPDFAttachment(attachment);
  if (!pFile)
    return 0;

  CPDF_Dictionary* pParamsDict = CPDF_FileSpec(pFile).GetParamsDict();
  if (!pParamsDict)
    return 0;

  return pParamsDict->KeyExist(CFXByteStringFromFPDFWideString(key));
}

DLLEXPORT FPDF_OBJECT_TYPE STDCALL
FPDFAttachment_GetValueType(FPDF_ATTACHMENT attachment, FPDF_WIDESTRING key) {
  if (!FPDFAttachment_HasKey(attachment, key))
    return FPDF_OBJECT_UNKNOWN;

  CPDF_Object* pObj = CPDF_FileSpec(CPDFObjectFromFPDFAttachment(attachment))
                          .GetParamsDict()
                          ->GetObjectFor(CFXByteStringFromFPDFWideString(key));
  if (!pObj)
    return FPDF_OBJECT_UNKNOWN;

  return pObj->GetType();
}

DLLEXPORT unsigned long STDCALL
FPDFAttachment_GetStringValue(FPDF_ATTACHMENT attachment,
                              FPDF_WIDESTRING key,
                              void* buffer,
                              unsigned long buflen) {
  CPDF_Object* pFile = CPDFObjectFromFPDFAttachment(attachment);
  if (!pFile)
    return 0;

  CPDF_Dictionary* pParamsDict = CPDF_FileSpec(pFile).GetParamsDict();
  if (!pParamsDict)
    return 0;

  CFX_ByteString bsKey = CFXByteStringFromFPDFWideString(key);
  CFX_WideString value = pParamsDict->GetUnicodeTextFor(bsKey);
  if (bsKey == "CheckSum") {
    CPDF_String* stringValue = pParamsDict->GetObjectFor(bsKey)->AsString();
    if (stringValue->IsHex()) {
      value =
          CPDF_String(nullptr, PDF_EncodeString(stringValue->GetString(), true),
                      false)
              .GetUnicodeText();
    }
  }

  return Utf16EncodeMaybeCopyAndReturnLength(value, buffer, buflen);
}

DLLEXPORT unsigned long STDCALL
FPDFAttachment_GetFile(FPDF_ATTACHMENT attachment,
                       void* buffer,
                       unsigned long buflen) {
  CPDF_Object* pFile = CPDFObjectFromFPDFAttachment(attachment);
  if (!pFile)
    return 0;

  CPDF_Stream* pFileStream = CPDF_FileSpec(pFile).GetFileStream();
  if (!pFileStream)
    return 0;

  uint8_t* data = pFileStream->GetRawData();
  uint32_t len = pFileStream->GetRawSize();
  CPDF_Dictionary* pFileDict = pFileStream->GetDict();
  if (!pFileDict || pFileDict->GetStringFor("Filter").IsEmpty()) {
    if (buffer && buflen >= len)
      memcpy(buffer, data, len);

    return len;
  }

  // Decode the stream if a stream filter is specified.
  uint8_t* decodedData = nullptr;
  uint32_t decodedLen = 0;
  CPDF_StreamParser::DecodeInlineStream(
      data, len, pFileDict->GetIntegerFor("Width"),
      pFileDict->GetIntegerFor("Height"), pFileDict->GetStringFor("Filter"),
      pFileDict->GetDictFor("DecodeParms"), &decodedData, &decodedLen);
  if (buffer && buflen >= decodedLen)
    memcpy(buffer, decodedData, decodedLen);

  FX_Free(decodedData);
  return decodedLen;
}
