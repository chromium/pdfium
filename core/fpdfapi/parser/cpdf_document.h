// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PARSER_CPDF_DOCUMENT_H_
#define CORE_FPDFAPI_PARSER_CPDF_DOCUMENT_H_

#include <functional>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "core/fpdfapi/page/cpdf_image.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fpdfdoc/cpdf_linklist.h"
#include "core/fxcrt/observable.h"
#include "core/fxcrt/retain_ptr.h"

class CFX_Font;
class CFX_Matrix;
class CPDF_ColorSpace;
class CPDF_DocPageData;
class CPDF_DocRenderData;
class CPDF_Font;
class CPDF_FontEncoding;
class CPDF_IccProfile;
class CPDF_LinearizedHeader;
class CPDF_Pattern;
class CPDF_ReadValidator;
class CPDF_StreamAcc;
class IFX_SeekableReadStream;
class JBig2_DocumentContext;

#define FPDFPERM_MODIFY 0x0008
#define FPDFPERM_ANNOT_FORM 0x0020
#define FPDFPERM_FILL_FORM 0x0100
#define FPDFPERM_EXTRACT_ACCESS 0x0200

class CPDF_Document : public Observable<CPDF_Document>,
                      public CPDF_Parser::ParsedObjectsHolder {
 public:
  // Type from which the XFA extension can subclass itself.
  class Extension {
   public:
    virtual ~Extension() {}
    virtual CPDF_Document* GetPDFDoc() const = 0;
    virtual int GetPageCount() const = 0;
    virtual void DeletePage(int page_index) = 0;
    virtual uint32_t GetUserPermissions() const = 0;
  };

  static const int kPageMaxNum = 0xFFFFF;

  CPDF_Document();
  ~CPDF_Document() override;

  Extension* GetExtension() const { return m_pExtension.get(); }
  void SetExtension(std::unique_ptr<Extension> pExt) {
    m_pExtension = std::move(pExt);
  }

  CPDF_Parser* GetParser() const { return m_pParser.get(); }
  CPDF_Dictionary* GetRoot() const { return m_pRootDict.Get(); }
  CPDF_Dictionary* GetInfo();

  void DeletePage(int iPage);
  int GetPageCount() const;
  bool IsPageLoaded(int iPage) const;
  CPDF_Dictionary* GetPageDictionary(int iPage);
  int GetPageIndex(uint32_t objnum);
  uint32_t GetUserPermissions() const;

  // Returns a valid pointer, unless it is called during destruction.
  CPDF_DocPageData* GetPageData() const { return m_pDocPage.get(); }

  void SetPageObjNum(int iPage, uint32_t objNum);

  std::unique_ptr<JBig2_DocumentContext>* CodecContext() {
    return &m_pCodecContext;
  }
  std::unique_ptr<CPDF_LinkList>* LinksContext() { return &m_pLinksContext; }

  CPDF_DocRenderData* GetRenderData() const { return m_pDocRender.get(); }

  // |pFontDict| must not be null.
  CPDF_Font* LoadFont(CPDF_Dictionary* pFontDict);
  CPDF_ColorSpace* LoadColorSpace(const CPDF_Object* pCSObj,
                                  const CPDF_Dictionary* pResources = nullptr);

  CPDF_Pattern* LoadPattern(CPDF_Object* pObj,
                            bool bShading,
                            const CFX_Matrix& matrix);

  RetainPtr<CPDF_Image> LoadImageFromPageData(uint32_t dwStreamObjNum);
  RetainPtr<CPDF_StreamAcc> LoadFontFile(const CPDF_Stream* pStream);
  RetainPtr<CPDF_IccProfile> LoadIccProfile(const CPDF_Stream* pStream);

  //  CPDF_Parser::ParsedObjectsHolder overrides:
  bool TryInit() override;

  CPDF_Parser::Error LoadDoc(
      const RetainPtr<IFX_SeekableReadStream>& pFileAccess,
      const char* password);
  CPDF_Parser::Error LoadLinearizedDoc(
      const RetainPtr<CPDF_ReadValidator>& validator,
      const char* password);

  void LoadPages();

  void CreateNewDoc();
  CPDF_Dictionary* CreateNewPage(int iPage);

  void IncrementParsedPageCount() { ++m_ParsedPageCount; }
  uint32_t GetParsedPageCountForTesting() { return m_ParsedPageCount; }

  CPDF_Font* AddStandardFont(const char* font, CPDF_FontEncoding* pEncoding);
  CPDF_Font* AddFont(CFX_Font* pFont, int charset, bool bVert);
#if _FX_PLATFORM_ == _FX_PLATFORM_WINDOWS_
  CPDF_Font* AddWindowsFont(LOGFONTA* pLogFont,
                            bool bVert,
                            bool bTranslateName = false);
  CPDF_Font* AddWindowsFont(LOGFONTW* pLogFont,
                            bool bVert,
                            bool bTranslateName = false);
#endif

 protected:
  // Retrieve page count information by getting count value from the tree nodes
  int RetrievePageCount();
  // When this method is called, m_pTreeTraversal[level] exists.
  CPDF_Dictionary* TraversePDFPages(int iPage, int* nPagesToGo, size_t level);
  int FindPageIndex(const CPDF_Dictionary* pNode,
                    uint32_t* skip_count,
                    uint32_t objnum,
                    int* index,
                    int level = 0) const;
  std::unique_ptr<CPDF_Object> ParseIndirectObject(uint32_t objnum) override;
  void LoadDocInternal();
  size_t CalculateEncodingDict(int charset, CPDF_Dictionary* pBaseDict);
  const CPDF_Dictionary* GetPagesDict() const;
  CPDF_Dictionary* GetPagesDict();
  CPDF_Dictionary* ProcessbCJK(
      CPDF_Dictionary* pBaseDict,
      int charset,
      bool bVert,
      ByteString basefont,
      std::function<void(wchar_t, wchar_t, CPDF_Array*)> Insert);
  bool InsertDeletePDFPage(CPDF_Dictionary* pPages,
                           int nPagesToGo,
                           CPDF_Dictionary* pPageDict,
                           bool bInsert,
                           std::set<CPDF_Dictionary*>* pVisited);
  bool InsertNewPage(int iPage, CPDF_Dictionary* pPageDict);
  void ResetTraversal();
  void SetParser(std::unique_ptr<CPDF_Parser> pParser);

  std::unique_ptr<CPDF_Parser> m_pParser;
  UnownedPtr<CPDF_Dictionary> m_pRootDict;
  UnownedPtr<CPDF_Dictionary> m_pInfoDict;

  // Vector of pairs to know current position in the page tree. The index in the
  // vector corresponds to the level being described. The pair contains a
  // pointer to the dictionary being processed at the level, and an index of the
  // of the child being processed within the dictionary's /Kids array.
  std::vector<std::pair<CPDF_Dictionary*, size_t>> m_pTreeTraversal;

  // Index of the next page that will be traversed from the page tree.
  int m_iNextPageToTraverse = 0;
  bool m_bReachedMaxPageLevel = false;
  uint32_t m_ParsedPageCount = 0;
  std::unique_ptr<CPDF_DocPageData> m_pDocPage;
  std::unique_ptr<CPDF_DocRenderData> m_pDocRender;
  std::unique_ptr<JBig2_DocumentContext> m_pCodecContext;
  std::unique_ptr<CPDF_LinkList> m_pLinksContext;
  std::vector<uint32_t> m_PageList;  // Page number to page's dict objnum.
  std::unique_ptr<Extension> m_pExtension;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_DOCUMENT_H_
