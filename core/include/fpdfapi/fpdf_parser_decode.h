// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FPDFAPI_FPDF_PARSER_DECODE_H_
#define CORE_INCLUDE_FPDFAPI_FPDF_PARSER_DECODE_H_

#include "core/include/fxcrt/fx_basic.h"

class CPDF_Dictionary;

// Indexed by 8-bit char code, contains unicode code points.
extern const FX_WORD PDFDocEncoding[256];

CFX_ByteString PDF_NameDecode(const CFX_ByteStringC& orig);
CFX_ByteString PDF_NameDecode(const CFX_ByteString& orig);
CFX_ByteString PDF_NameEncode(const CFX_ByteString& orig);
CFX_ByteString PDF_EncodeString(const CFX_ByteString& src,
                                FX_BOOL bHex = FALSE);
CFX_WideString PDF_DecodeText(const uint8_t* pData, FX_DWORD size);
CFX_WideString PDF_DecodeText(const CFX_ByteString& bstr);
CFX_ByteString PDF_EncodeText(const FX_WCHAR* pString, int len = -1);
CFX_ByteString PDF_EncodeText(const CFX_WideString& str);

void FlateEncode(const uint8_t* src_buf,
                 FX_DWORD src_size,
                 uint8_t*& dest_buf,
                 FX_DWORD& dest_size);
void FlateEncode(const uint8_t* src_buf,
                 FX_DWORD src_size,
                 int predictor,
                 int Colors,
                 int BitsPerComponent,
                 int Columns,
                 uint8_t*& dest_buf,
                 FX_DWORD& dest_size);
FX_DWORD FlateDecode(const uint8_t* src_buf,
                     FX_DWORD src_size,
                     uint8_t*& dest_buf,
                     FX_DWORD& dest_size);
FX_DWORD RunLengthDecode(const uint8_t* src_buf,
                         FX_DWORD src_size,
                         uint8_t*& dest_buf,
                         FX_DWORD& dest_size);

// Public for testing.
FX_DWORD A85Decode(const uint8_t* src_buf,
                   FX_DWORD src_size,
                   uint8_t*& dest_buf,
                   FX_DWORD& dest_size);
// Public for testing.
FX_DWORD HexDecode(const uint8_t* src_buf,
                   FX_DWORD src_size,
                   uint8_t*& dest_buf,
                   FX_DWORD& dest_size);
// Public for testing.
FX_DWORD FPDFAPI_FlateOrLZWDecode(FX_BOOL bLZW,
                                  const uint8_t* src_buf,
                                  FX_DWORD src_size,
                                  CPDF_Dictionary* pParams,
                                  FX_DWORD estimated_size,
                                  uint8_t*& dest_buf,
                                  FX_DWORD& dest_size);
FX_BOOL PDF_DataDecode(const uint8_t* src_buf,
                       FX_DWORD src_size,
                       const CPDF_Dictionary* pDict,
                       uint8_t*& dest_buf,
                       FX_DWORD& dest_size,
                       CFX_ByteString& ImageEncoding,
                       CPDF_Dictionary*& pImageParms,
                       FX_DWORD estimated_size,
                       FX_BOOL bImageAcc);

#endif  // CORE_INCLUDE_FPDFAPI_FPDF_PARSER_DECODE_H_
