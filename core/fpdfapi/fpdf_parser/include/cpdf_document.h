// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PARSER_INCLUDE_CPDF_DOCUMENT_H_
#define CORE_FPDFAPI_FPDF_PARSER_INCLUDE_CPDF_DOCUMENT_H_

#include "core/fpdfapi/fpdf_parser/include/cpdf_indirect_object_holder.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_object.h"
#include "core/fxcrt/include/fx_basic.h"

class CFX_Font;
class CFX_Matrix;
class CPDF_ColorSpace;
class CPDF_DocPageData;
class CPDF_DocRenderData;
class CPDF_Font;
class CPDF_FontEncoding;
class CPDF_IccProfile;
class CPDF_Image;
class CPDF_Pattern;
class CPDF_StreamAcc;

#define FPDFPERM_PRINT 0x0004
#define FPDFPERM_MODIFY 0x0008
#define FPDFPERM_EXTRACT 0x0010
#define FPDFPERM_ANNOT_FORM 0x0020
#define FPDFPERM_FILL_FORM 0x0100
#define FPDFPERM_EXTRACT_ACCESS 0x0200
#define FPDFPERM_ASSEMBLE 0x0400
#define FPDFPERM_PRINT_HIGH 0x0800
#define FPDF_PAGE_MAX_NUM 0xFFFFF

class CPDF_Document : public CFX_PrivateData, public CPDF_IndirectObjectHolder {
 public:
  CPDF_Document();
  explicit CPDF_Document(CPDF_Parser* pParser);
  ~CPDF_Document();

  CPDF_Parser* GetParser() const { return m_pParser; }
  CPDF_Dictionary* GetRoot() const { return m_pRootDict; }
  CPDF_Dictionary* GetInfo() const { return m_pInfoDict; }

  void GetID(CFX_ByteString& id1, CFX_ByteString& id2) const {
    id1 = m_ID1;
    id2 = m_ID2;
  }

  int GetPageCount() const;
  CPDF_Dictionary* GetPage(int iPage);
  int GetPageIndex(uint32_t objnum);
  uint32_t GetUserPermissions(FX_BOOL bCheckRevision = FALSE) const;
  CPDF_DocPageData* GetPageData() { return GetValidatePageData(); }
  void ClearPageData();
  void RemoveColorSpaceFromPageData(CPDF_Object* pObject);

  CPDF_DocRenderData* GetRenderData() { return GetValidateRenderData(); }
  void ClearRenderData();
  void ClearRenderFont();

  FX_BOOL IsFormStream(uint32_t objnum, FX_BOOL& bForm) const;

  // |pFontDict| must not be null.
  CPDF_Font* LoadFont(CPDF_Dictionary* pFontDict);
  CPDF_ColorSpace* LoadColorSpace(CPDF_Object* pCSObj,
                                  CPDF_Dictionary* pResources = NULL);

  CPDF_Pattern* LoadPattern(CPDF_Object* pObj,
                            FX_BOOL bShading,
                            const CFX_Matrix* matrix = NULL);

  CPDF_Image* LoadImageF(CPDF_Object* pObj);
  CPDF_StreamAcc* LoadFontFile(CPDF_Stream* pStream);
  CPDF_IccProfile* LoadIccProfile(CPDF_Stream* pStream);

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_

  CPDF_Font* AddWindowsFont(LOGFONTA* pLogFont,
                            FX_BOOL bVert,
                            FX_BOOL bTranslateName = FALSE);
  CPDF_Font* AddWindowsFont(LOGFONTW* pLogFont,
                            FX_BOOL bVert,
                            FX_BOOL bTranslateName = FALSE);
#endif
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
  CPDF_Font* AddMacFont(CTFontRef pFont,
                        FX_BOOL bVert,
                        FX_BOOL bTranslateName = FALSE);
#endif

  CPDF_Font* AddStandardFont(const FX_CHAR* font, CPDF_FontEncoding* pEncoding);
  CPDF_Font* AddFont(CFX_Font* pFont, int charset, FX_BOOL bVert);
  void CreateNewDoc();

  CPDF_Dictionary* CreateNewPage(int iPage);
  void DeletePage(int iPage);

  void LoadDoc();
  void LoadAsynDoc(CPDF_Dictionary* pLinearized);
  void LoadPages();

 protected:
  // Retrieve page count information by getting count value from the tree nodes
  // or walking through the tree nodes to calculate it.
  int RetrievePageCount() const;
  CPDF_Dictionary* _FindPDFPage(CPDF_Dictionary* pPages,
                                int iPage,
                                int nPagesToGo,
                                int level);
  int _FindPageIndex(CPDF_Dictionary* pNode,
                     uint32_t& skip_count,
                     uint32_t objnum,
                     int& index,
                     int level = 0);
  FX_BOOL CheckOCGVisible(CPDF_Dictionary* pOCG, FX_BOOL bPrinting);
  CPDF_DocPageData* GetValidatePageData();
  CPDF_DocRenderData* GetValidateRenderData();
  friend class CPDF_Creator;
  friend class CPDF_Parser;
  friend class CPDF_DataAvail;
  friend class CPDF_OCContext;

  CPDF_Dictionary* m_pRootDict;
  CPDF_Dictionary* m_pInfoDict;
  CFX_ByteString m_ID1;
  CFX_ByteString m_ID2;
  FX_BOOL m_bLinearized;
  uint32_t m_dwFirstPageNo;
  uint32_t m_dwFirstPageObjNum;
  CFX_ArrayTemplate<uint32_t> m_PageList;
  CPDF_DocPageData* m_pDocPage;
  CPDF_DocRenderData* m_pDocRender;
};

#endif  // CORE_FPDFAPI_FPDF_PARSER_INCLUDE_CPDF_DOCUMENT_H_
