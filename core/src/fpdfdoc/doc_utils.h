// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FPDFDOC_DOC_UTILS_H_
#define CORE_SRC_FPDFDOC_DOC_UTILS_H_

CFX_WideString	GetFullName(CPDF_Dictionary* pFieldDict);
void			InitInterFormDict(CPDF_Dictionary*& pFormDict, CPDF_Document* pDocument);
FX_DWORD		CountInterFormFonts(CPDF_Dictionary* pFormDict);
CPDF_Font*		GetInterFormFont(CPDF_Dictionary* pFormDict, CPDF_Document* pDocument, FX_DWORD index, CFX_ByteString& csNameTag);
CPDF_Font*		GetInterFormFont(CPDF_Dictionary* pFormDict, CPDF_Document* pDocument, CFX_ByteString csNameTag);
CPDF_Font*		GetInterFormFont(CPDF_Dictionary* pFormDict, CPDF_Document* pDocument, CFX_ByteString csFontName, CFX_ByteString& csNameTag);
CPDF_Font*		GetNativeInterFormFont(CPDF_Dictionary* pFormDict, CPDF_Document* pDocument, FX_BYTE charSet, CFX_ByteString& csNameTag);
CPDF_Font*		GetNativeInterFormFont(CPDF_Dictionary* pFormDict, CPDF_Document* pDocument, CFX_ByteString& csNameTag);
FX_BOOL			FindInterFormFont(CPDF_Dictionary* pFormDict, const CPDF_Font* pFont, CFX_ByteString& csNameTag);
FX_BOOL			FindInterFormFont(CPDF_Dictionary* pFormDict, CPDF_Document* pDocument, CFX_ByteString csFontName, CPDF_Font*& pFont, CFX_ByteString& csNameTag);
void			AddInterFormFont(CPDF_Dictionary*& pFormDict, CPDF_Document* pDocument, const CPDF_Font* pFont, CFX_ByteString& csNameTag);
CPDF_Font*		AddNativeInterFormFont(CPDF_Dictionary*& pFormDict, CPDF_Document* pDocument, FX_BYTE charSet, CFX_ByteString& csNameTag);
CPDF_Font*		AddNativeInterFormFont(CPDF_Dictionary*& pFormDict, CPDF_Document* pDocument, CFX_ByteString& csNameTag);
void			RemoveInterFormFont(CPDF_Dictionary* pFormDict, const CPDF_Font* pFont);
void			RemoveInterFormFont(CPDF_Dictionary* pFormDict, CFX_ByteString csNameTag);
CPDF_Font*		GetDefaultInterFormFont(CPDF_Dictionary* pFormDict, CPDF_Document* pDocument);
void			SetDefaultInterFormFont(CPDF_Dictionary*& pFormDict, CPDF_Document* pDocument, const CPDF_Font* pFont);
void			SaveCheckedFieldStatus(CPDF_FormField* pField, CFX_ByteArray& statusArray);
FX_BOOL			NeedPDFEncodeForFieldFullName(const CFX_WideString& csFieldName);
FX_BOOL			NeedPDFEncodeForFieldTree(CPDF_Dictionary* pFieldDict, int nLevel = 0);
void			EncodeFieldName(const CFX_WideString& csName, CFX_ByteString& csT);
void			UpdateEncodeFieldName(CPDF_Dictionary* pFieldDict, int nLevel = 0);

#endif  // CORE_SRC_FPDFDOC_DOC_UTILS_H_
