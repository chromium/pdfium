// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_INCLUDE_CPDF_INTERFORM_H_
#define CORE_FPDFDOC_INCLUDE_CPDF_INTERFORM_H_

#include <map>
#include <memory>
#include <vector>

#include "core/fpdfapi/fpdf_parser/include/fpdf_parser_decode.h"
#include "core/fpdfdoc/include/cpdf_defaultappearance.h"
#include "core/fxcrt/include/fx_string.h"
#include "core/fxcrt/include/fx_system.h"

class CFieldTree;
class CFDF_Document;
class CPDF_Document;
class CPDF_Dictionary;
class CPDF_Font;
class CPDF_FormControl;
class CPDF_FormField;
class CPDF_Object;
class CPDF_Page;
class IPDF_FormNotify;

CPDF_Font* AddNativeInterFormFont(CPDF_Dictionary*& pFormDict,
                                  CPDF_Document* pDocument,
                                  CFX_ByteString& csNameTag);

class CPDF_InterForm {
 public:
  explicit CPDF_InterForm(CPDF_Document* pDocument);
  ~CPDF_InterForm();

  static void SetUpdateAP(FX_BOOL bUpdateAP);
  static FX_BOOL IsUpdateAPEnabled();
  static CFX_ByteString GenerateNewResourceName(const CPDF_Dictionary* pResDict,
                                                const FX_CHAR* csType,
                                                int iMinLen = 2,
                                                const FX_CHAR* csPrefix = "");
  static CPDF_Font* AddStandardFont(CPDF_Document* pDocument,
                                    CFX_ByteString csFontName);
  static CFX_ByteString GetNativeFont(uint8_t iCharSet,
                                      void* pLogFont = nullptr);
  static CFX_ByteString GetNativeFont(void* pLogFont = nullptr);
  static uint8_t GetNativeCharSet();
  static CPDF_Font* AddNativeFont(uint8_t iCharSet, CPDF_Document* pDocument);
  static CPDF_Font* AddNativeFont(CPDF_Document* pDocument);

  FX_BOOL ValidateFieldName(CFX_WideString& csNewFieldName, int iType);
  FX_BOOL ValidateFieldName(const CPDF_FormField* pField,
                            CFX_WideString& csNewFieldName);
  FX_BOOL ValidateFieldName(const CPDF_FormControl* pControl,
                            CFX_WideString& csNewFieldName);

  uint32_t CountFields(const CFX_WideString& csFieldName = L"");
  CPDF_FormField* GetField(uint32_t index,
                           const CFX_WideString& csFieldName = L"");
  CPDF_FormField* GetFieldByDict(CPDF_Dictionary* pFieldDict) const;

  CPDF_FormControl* GetControlAtPoint(CPDF_Page* pPage,
                                      FX_FLOAT pdf_x,
                                      FX_FLOAT pdf_y,
                                      int* z_order) const;
  CPDF_FormControl* GetControlByDict(const CPDF_Dictionary* pWidgetDict) const;

  CPDF_Document* GetDocument() const { return m_pDocument; }
  CPDF_Dictionary* GetFormDict() const { return m_pFormDict; }
  FX_BOOL NeedConstructAP() const;
  int CountFieldsInCalculationOrder();
  CPDF_FormField* GetFieldInCalculationOrder(int index);
  int FindFieldInCalculationOrder(const CPDF_FormField* pField);

  uint32_t CountFormFonts();
  CPDF_Font* GetFormFont(uint32_t index, CFX_ByteString& csNameTag);
  CPDF_Font* GetFormFont(CFX_ByteString csNameTag);
  CPDF_Font* GetFormFont(CFX_ByteString csFontName, CFX_ByteString& csNameTag);
  CPDF_Font* GetNativeFormFont(uint8_t iCharSet, CFX_ByteString& csNameTag);
  CPDF_Font* GetNativeFormFont(CFX_ByteString& csNameTag);
  FX_BOOL FindFormFont(const CPDF_Font* pFont, CFX_ByteString& csNameTag);
  FX_BOOL FindFormFont(CFX_ByteString csFontName,
                       CPDF_Font*& pFont,
                       CFX_ByteString& csNameTag);

  FX_BOOL FindFormFont(CFX_WideString csFontName,
                       CPDF_Font*& pFont,
                       CFX_ByteString& csNameTag) {
    return FindFormFont(PDF_EncodeText(csFontName), pFont, csNameTag);
  }

  void AddFormFont(const CPDF_Font* pFont, CFX_ByteString& csNameTag);
  CPDF_Font* AddNativeFormFont(uint8_t iCharSet, CFX_ByteString& csNameTag);
  CPDF_Font* AddNativeFormFont(CFX_ByteString& csNameTag);

  void RemoveFormFont(const CPDF_Font* pFont);
  void RemoveFormFont(CFX_ByteString csNameTag);

  CPDF_DefaultAppearance GetDefaultAppearance();
  CPDF_Font* GetDefaultFormFont();
  int GetFormAlignment();

  CPDF_FormField* CheckRequiredFields(
      const std::vector<CPDF_FormField*>* fields,
      bool bIncludeOrExclude) const;

  CFDF_Document* ExportToFDF(const CFX_WideStringC& pdf_path,
                             bool bSimpleFileSpec = false) const;
  CFDF_Document* ExportToFDF(const CFX_WideStringC& pdf_path,
                             const std::vector<CPDF_FormField*>& fields,
                             bool bIncludeOrExclude = true,
                             bool bSimpleFileSpec = false) const;
  FX_BOOL ImportFromFDF(const CFDF_Document* pFDFDoc, FX_BOOL bNotify = FALSE);

  bool ResetForm(const std::vector<CPDF_FormField*>& fields,
                 bool bIncludeOrExclude = true,
                 bool bNotify = false);
  bool ResetForm(bool bNotify = false);

  void SetFormNotify(IPDF_FormNotify* pNotify);
  FX_BOOL HasXFAForm() const;
  void FixPageFields(const CPDF_Page* pPage);

 private:
  friend class CPDF_FormControl;
  friend class CPDF_FormField;

  void LoadField(CPDF_Dictionary* pFieldDict, int nLevel = 0);
  CPDF_Object* GetFieldAttr(CPDF_Dictionary* pFieldDict, const FX_CHAR* name);
  CPDF_FormField* AddTerminalField(CPDF_Dictionary* pFieldDict);
  CPDF_FormControl* AddControl(CPDF_FormField* pField,
                               CPDF_Dictionary* pWidgetDict);
  void FDF_ImportField(CPDF_Dictionary* pField,
                       const CFX_WideString& parent_name,
                       FX_BOOL bNotify = FALSE,
                       int nLevel = 0);
  FX_BOOL ValidateFieldName(CFX_WideString& csNewFieldName,
                            int iType,
                            const CPDF_FormField* pExcludedField,
                            const CPDF_FormControl* pExcludedControl);
  int CompareFieldName(const CFX_WideString& name1,
                       const CFX_WideString& name2);
  int CompareFieldName(const CFX_ByteString& name1,
                       const CFX_ByteString& name2);

  static FX_BOOL s_bUpdateAP;

  CPDF_Document* const m_pDocument;
  CPDF_Dictionary* m_pFormDict;
  std::map<const CPDF_Dictionary*, CPDF_FormControl*> m_ControlMap;
  std::unique_ptr<CFieldTree> m_pFieldTree;
  CFX_ByteString m_bsEncoding;
  IPDF_FormNotify* m_pFormNotify;
};

#endif  // CORE_FPDFDOC_INCLUDE_CPDF_INTERFORM_H_
