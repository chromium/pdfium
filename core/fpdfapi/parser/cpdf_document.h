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

#include "build/build_config.h"
#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fxcrt/observed_ptr.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"

class CFX_Font;
class CFX_Matrix;
class CPDF_LinearizedHeader;
class CPDF_Object;
class CPDF_ReadValidator;
class CPDF_StreamAcc;
class IFX_SeekableReadStream;
class JBig2_DocumentContext;

#define FPDFPERM_MODIFY 0x0008
#define FPDFPERM_ANNOT_FORM 0x0020
#define FPDFPERM_FILL_FORM 0x0100
#define FPDFPERM_EXTRACT_ACCESS 0x0200

class CPDF_Document : public Observable,
                      public CPDF_Parser::ParsedObjectsHolder {
 public:
  // Type from which the XFA extension can subclass itself.
  class Extension {
   public:
    virtual ~Extension() = default;
    virtual CPDF_Document* GetPDFDoc() const = 0;
    virtual int GetPageCount() const = 0;
    virtual void DeletePage(int page_index) = 0;
    virtual uint32_t GetUserPermissions() const = 0;
    virtual bool ContainsExtensionForm() const = 0;
  };

  class LinkListIface {
   public:
    // CPDF_Document merely helps manage the lifetime.
    virtual ~LinkListIface() = default;
  };

  class PageDataIface {
   public:
    PageDataIface();
    virtual ~PageDataIface();

    virtual void ClearStockFont() = 0;
    virtual RetainPtr<CPDF_StreamAcc> GetFontFileStreamAcc(
        const CPDF_Stream* pFontStream) = 0;
    virtual void MaybePurgeFontFileStreamAcc(
        const CPDF_Stream* pFontStream) = 0;

    void SetDocument(CPDF_Document* pDoc) { m_pDoc = pDoc; }
    CPDF_Document* GetDocument() const { return m_pDoc.Get(); }

   private:
    UnownedPtr<CPDF_Document> m_pDoc;
  };

  class RenderDataIface {
   public:
    RenderDataIface();
    virtual ~RenderDataIface();

    void SetDocument(CPDF_Document* pDoc) { m_pDoc = pDoc; }
    CPDF_Document* GetDocument() const { return m_pDoc.Get(); }

   private:
    UnownedPtr<CPDF_Document> m_pDoc;
  };

  static const int kPageMaxNum = 0xFFFFF;

  CPDF_Document(std::unique_ptr<RenderDataIface> pRenderData,
                std::unique_ptr<PageDataIface> pPageData);
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
  PageDataIface* GetPageData() const { return m_pDocPage.get(); }
  RenderDataIface* GetRenderData() const { return m_pDocRender.get(); }

  void SetPageObjNum(int iPage, uint32_t objNum);

  std::unique_ptr<JBig2_DocumentContext>* CodecContext() {
    return &m_pCodecContext;
  }
  LinkListIface* GetLinksContext() const { return m_pLinksContext.get(); }
  void SetLinksContext(std::unique_ptr<LinkListIface> pContext) {
    m_pLinksContext = std::move(pContext);
  }

  //  CPDF_Parser::ParsedObjectsHolder overrides:
  bool TryInit() override;

  CPDF_Parser::Error LoadDoc(
      const RetainPtr<IFX_SeekableReadStream>& pFileAccess,
      const char* password);
  CPDF_Parser::Error LoadLinearizedDoc(
      const RetainPtr<CPDF_ReadValidator>& validator,
      const char* password);
  bool has_valid_cross_reference_table() const {
    return m_bHasValidCrossReferenceTable;
  }

  void LoadPages();
  void CreateNewDoc();
  CPDF_Dictionary* CreateNewPage(int iPage);

  void IncrementParsedPageCount() { ++m_ParsedPageCount; }
  uint32_t GetParsedPageCountForTesting() { return m_ParsedPageCount; }

 protected:
  class StockFontClearer {
   public:
    explicit StockFontClearer(CPDF_Document::PageDataIface* pPageData);
    ~StockFontClearer();

   private:
    UnownedPtr<CPDF_Document::PageDataIface> const m_pPageData;
  };

  // Retrieve page count information by getting count value from the tree nodes
  int RetrievePageCount();
  // When this method is called, m_pTreeTraversal[level] exists.
  CPDF_Dictionary* TraversePDFPages(int iPage, int* nPagesToGo, size_t level);
  int FindPageIndex(const CPDF_Dictionary* pNode,
                    uint32_t* skip_count,
                    uint32_t objnum,
                    int* index,
                    int level) const;
  RetainPtr<CPDF_Object> ParseIndirectObject(uint32_t objnum) override;
  const CPDF_Dictionary* GetPagesDict() const;
  CPDF_Dictionary* GetPagesDict();
  bool InsertDeletePDFPage(CPDF_Dictionary* pPages,
                           int nPagesToGo,
                           CPDF_Dictionary* pPageDict,
                           bool bInsert,
                           std::set<CPDF_Dictionary*>* pVisited);
  bool InsertNewPage(int iPage, CPDF_Dictionary* pPageDict);
  void ResetTraversal();
  void SetParser(std::unique_ptr<CPDF_Parser> pParser);
  CPDF_Parser::Error HandleLoadResult(CPDF_Parser::Error error);

  std::unique_ptr<CPDF_Parser> m_pParser;
  RetainPtr<CPDF_Dictionary> m_pRootDict;
  RetainPtr<CPDF_Dictionary> m_pInfoDict;

  // Vector of pairs to know current position in the page tree. The index in the
  // vector corresponds to the level being described. The pair contains a
  // pointer to the dictionary being processed at the level, and an index of the
  // of the child being processed within the dictionary's /Kids array.
  std::vector<std::pair<CPDF_Dictionary*, size_t>> m_pTreeTraversal;

  // True if the CPDF_Parser succeeded without having to rebuild the cross
  // reference table.
  bool m_bHasValidCrossReferenceTable = false;

  // Index of the next page that will be traversed from the page tree.
  bool m_bReachedMaxPageLevel = false;
  int m_iNextPageToTraverse = 0;
  uint32_t m_ParsedPageCount = 0;

  std::unique_ptr<RenderDataIface> m_pDocRender;
  std::unique_ptr<PageDataIface> m_pDocPage;  // Must be after |m_pDocRender|.
  std::unique_ptr<JBig2_DocumentContext> m_pCodecContext;
  std::unique_ptr<LinkListIface> m_pLinksContext;
  std::vector<uint32_t> m_PageList;  // Page number to page's dict objnum.

  // Must be second to last.
  StockFontClearer m_StockFontClearer;

  // Must be last. Destroy the extension before any non-extension teardown.
  std::unique_ptr<Extension> m_pExtension;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_DOCUMENT_H_
