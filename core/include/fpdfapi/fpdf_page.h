// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FPDFAPI_FPDF_PAGE_H_
#define CORE_INCLUDE_FPDFAPI_FPDF_PAGE_H_

#include <deque>
#include <memory>

#include "core/include/fpdfapi/fpdf_parser.h"
#include "core/include/fpdfapi/fpdf_resource.h"
#include "core/include/fxge/fx_dib.h"

class CPDF_Page;
class CPDF_Form;
class CPDF_ParseOptions;
class CPDF_PageObject;
class CPDF_PageRenderCache;
class CPDF_StreamFilter;
class CPDF_AllStates;
class CPDF_ContentParser;
class CPDF_StreamContentParser;

#define PDFTRANS_GROUP 0x0100
#define PDFTRANS_ISOLATED 0x0200
#define PDFTRANS_KNOCKOUT 0x0400

class CPDF_PageObjectList
    : public std::deque<std::unique_ptr<CPDF_PageObject>> {
 public:
  CPDF_PageObject* GetPageObjectByIndex(int index);
};

class CPDF_PageObjectHolder {
 public:
  CPDF_PageObjectHolder();

  void ContinueParse(IFX_Pause* pPause);
  FX_BOOL IsParsed() const { return m_ParseState == CONTENT_PARSED; }

  CPDF_PageObjectList* GetPageObjectList() { return &m_PageObjectList; }
  const CPDF_PageObjectList* GetPageObjectList() const {
    return &m_PageObjectList;
  }

  FX_BOOL BackgroundAlphaNeeded() const { return m_bBackgroundAlphaNeeded; }
  void SetBackgroundAlphaNeeded(FX_BOOL needed) {
    m_bBackgroundAlphaNeeded = needed;
  }

  FX_BOOL HasImageMask() const { return m_bHasImageMask; }
  void SetHasImageMask(FX_BOOL value) { m_bHasImageMask = value; }

  void Transform(const CFX_Matrix& matrix);
  CFX_FloatRect CalcBoundingBox() const;

  CPDF_Dictionary* m_pFormDict;
  CPDF_Stream* m_pFormStream;
  CPDF_Document* m_pDocument;
  CPDF_Dictionary* m_pPageResources;
  CPDF_Dictionary* m_pResources;
  CFX_FloatRect m_BBox;
  int m_Transparency;

 protected:
  enum ParseState { CONTENT_NOT_PARSED, CONTENT_PARSING, CONTENT_PARSED };

  void LoadTransInfo();

  FX_BOOL m_bBackgroundAlphaNeeded;
  FX_BOOL m_bHasImageMask;
  ParseState m_ParseState;
  std::unique_ptr<CPDF_ContentParser> m_pParser;
  CPDF_PageObjectList m_PageObjectList;
};

class CPDF_Page : public CPDF_PageObjectHolder, public CFX_PrivateData {
 public:
  CPDF_Page();
  ~CPDF_Page();

  void Load(CPDF_Document* pDocument,
            CPDF_Dictionary* pPageDict,
            FX_BOOL bPageCache = TRUE);

  void ParseContent(CPDF_ParseOptions* pOptions);

  void GetDisplayMatrix(CFX_Matrix& matrix,
                        int xPos,
                        int yPos,
                        int xSize,
                        int ySize,
                        int iRotate) const;

  FX_FLOAT GetPageWidth() const { return m_PageWidth; }
  FX_FLOAT GetPageHeight() const { return m_PageHeight; }
  CFX_FloatRect GetPageBBox() const { return m_BBox; }
  const CFX_Matrix& GetPageMatrix() const { return m_PageMatrix; }
  CPDF_Object* GetPageAttr(const CFX_ByteStringC& name) const;
  CPDF_PageRenderCache* GetRenderCache() const { return m_pPageRender; }

 protected:
  friend class CPDF_ContentParser;
  void StartParse(CPDF_ParseOptions* pOptions);

  FX_FLOAT m_PageWidth;
  FX_FLOAT m_PageHeight;
  CFX_Matrix m_PageMatrix;
  CPDF_PageRenderCache* m_pPageRender;
};
class CPDF_ParseOptions {
 public:
  CPDF_ParseOptions();

  FX_BOOL m_bTextOnly;

  FX_BOOL m_bMarkedContent;

  FX_BOOL m_bSeparateForm;

  FX_BOOL m_bDecodeInlineImage;
};
class CPDF_Form : public CPDF_PageObjectHolder {
 public:
  CPDF_Form(CPDF_Document* pDocument,
            CPDF_Dictionary* pPageResources,
            CPDF_Stream* pFormStream,
            CPDF_Dictionary* pParentResources = NULL);

  ~CPDF_Form();

  void StartParse(CPDF_AllStates* pGraphicStates,
                  CFX_Matrix* pParentMatrix,
                  CPDF_Type3Char* pType3Char,
                  CPDF_ParseOptions* pOptions,
                  int level = 0);

  void ParseContent(CPDF_AllStates* pGraphicStates,
                    CFX_Matrix* pParentMatrix,
                    CPDF_Type3Char* pType3Char,
                    CPDF_ParseOptions* pOptions,
                    int level = 0);

  CPDF_Form* Clone() const;
};
class CPDF_PageContentGenerator {
 public:
  explicit CPDF_PageContentGenerator(CPDF_Page* pPage);

  FX_BOOL InsertPageObject(CPDF_PageObject* pPageObject);
  void GenerateContent();
  void TransformContent(CFX_Matrix& matrix);

 private:
  void ProcessImage(CFX_ByteTextBuf& buf, CPDF_ImageObject* pImageObj);
  void ProcessForm(CFX_ByteTextBuf& buf,
                   const uint8_t* data,
                   FX_DWORD size,
                   CFX_Matrix& matrix);
  CFX_ByteString RealizeResource(CPDF_Object* pResourceObj,
                                 const FX_CHAR* szType);

  CPDF_Page* m_pPage;
  CPDF_Document* m_pDocument;
  CFX_ArrayTemplate<CPDF_PageObject*> m_pageObjects;
};

#endif  // CORE_INCLUDE_FPDFAPI_FPDF_PAGE_H_
