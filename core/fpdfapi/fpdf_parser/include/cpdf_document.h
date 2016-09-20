// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PARSER_INCLUDE_CPDF_DOCUMENT_H_
#define CORE_FPDFAPI_FPDF_PARSER_INCLUDE_CPDF_DOCUMENT_H_

#include <functional>
#include <memory>

#include "core/fpdfapi/fpdf_parser/include/cpdf_indirect_object_holder.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_object.h"
#include "core/fpdfdoc/include/cpdf_linklist.h"
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
class CPDF_Parser;
class CPDF_Pattern;
class CPDF_StreamAcc;
class JBig2_DocumentContext;

#define FPDFPERM_PRINT 0x0004
#define FPDFPERM_MODIFY 0x0008
#define FPDFPERM_EXTRACT 0x0010
#define FPDFPERM_ANNOT_FORM 0x0020
#define FPDFPERM_FILL_FORM 0x0100
#define FPDFPERM_EXTRACT_ACCESS 0x0200
#define FPDFPERM_ASSEMBLE 0x0400
#define FPDFPERM_PRINT_HIGH 0x0800
#define FPDF_PAGE_MAX_NUM 0xFFFFF

class CPDF_Document : public CPDF_IndirectObjectHolder {
 public:
  explicit CPDF_Document(std::unique_ptr<CPDF_Parser> pParser);
  ~CPDF_Document() override;

  CPDF_Parser* GetParser() const { return m_pParser.get(); }
  CPDF_Dictionary* GetRoot() const { return m_pRootDict; }
  CPDF_Dictionary* GetInfo() const { return m_pInfoDict; }

  void DeletePage(int iPage);
  int GetPageCount() const;

  bool IsPageLoaded(int iPage) const;
  CPDF_Dictionary* GetPage(int iPage);
  int GetPageIndex(uint32_t objnum);
  uint32_t GetUserPermissions() const;
  CPDF_DocPageData* GetPageData() const { return m_pDocPage; }

  void SetPageObjNum(int iPage, uint32_t objNum);

  std::unique_ptr<JBig2_DocumentContext>* CodecContext() {
    return &m_pCodecContext;
  }
  std::unique_ptr<CPDF_LinkList>* LinksContext() { return &m_pLinksContext; }

  CPDF_DocRenderData* GetRenderData() const { return m_pDocRender.get(); }

  // |pFontDict| must not be null.
  CPDF_Font* LoadFont(CPDF_Dictionary* pFontDict);
  CPDF_ColorSpace* LoadColorSpace(CPDF_Object* pCSObj,
                                  CPDF_Dictionary* pResources = nullptr);

  CPDF_Pattern* LoadPattern(CPDF_Object* pObj,
                            FX_BOOL bShading,
                            const CFX_Matrix& matrix);

  CPDF_Image* LoadImageF(CPDF_Object* pObj);
  CPDF_StreamAcc* LoadFontFile(CPDF_Stream* pStream);
  CPDF_IccProfile* LoadIccProfile(CPDF_Stream* pStream);

  void LoadDoc();
  void LoadLinearizedDoc(CPDF_Dictionary* pLinearizationParams);
  void LoadPages();

  void CreateNewDoc();
  CPDF_Dictionary* CreateNewPage(int iPage);

  CPDF_Font* AddStandardFont(const FX_CHAR* font, CPDF_FontEncoding* pEncoding);
  CPDF_Font* AddFont(CFX_Font* pFont, int charset, FX_BOOL bVert);
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  CPDF_Font* AddWindowsFont(LOGFONTA* pLogFont,
                            FX_BOOL bVert,
                            FX_BOOL bTranslateName = FALSE);
  CPDF_Font* AddWindowsFont(LOGFONTW* pLogFont,
                            FX_BOOL bVert,
                            FX_BOOL bTranslateName = FALSE);
#endif

 private:
  friend class CPDF_TestDocument;

  // Retrieve page count information by getting count value from the tree nodes
  int RetrievePageCount() const;
  CPDF_Dictionary* FindPDFPage(CPDF_Dictionary* pPages,
                               int iPage,
                               int nPagesToGo,
                               int level);
  int FindPageIndex(CPDF_Dictionary* pNode,
                    uint32_t& skip_count,
                    uint32_t objnum,
                    int& index,
                    int level = 0);
  CPDF_Object* ParseIndirectObject(uint32_t objnum) override;
  void LoadDocInternal();
  size_t CalculateEncodingDict(int charset, CPDF_Dictionary* pBaseDict);
  CPDF_Dictionary* GetPagesDict() const;
  CPDF_Dictionary* ProcessbCJK(
      CPDF_Dictionary* pBaseDict,
      int charset,
      FX_BOOL bVert,
      CFX_ByteString basefont,
      std::function<void(FX_WCHAR, FX_WCHAR, CPDF_Array*)> Insert);

  std::unique_ptr<CPDF_Parser> m_pParser;
  CPDF_Dictionary* m_pRootDict;
  CPDF_Dictionary* m_pInfoDict;
  bool m_bLinearized;
  int m_iFirstPageNo;
  uint32_t m_dwFirstPageObjNum;
  // TODO(thestig): Figure out why this cannot be a std::unique_ptr.
  CPDF_DocPageData* m_pDocPage;
  std::unique_ptr<CPDF_DocRenderData> m_pDocRender;
  std::unique_ptr<JBig2_DocumentContext> m_pCodecContext;
  std::unique_ptr<CPDF_LinkList> m_pLinksContext;
  CFX_ArrayTemplate<uint32_t> m_PageList;
};

#endif  // CORE_FPDFAPI_FPDF_PARSER_INCLUDE_CPDF_DOCUMENT_H_
