// Copyright 2015 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <set>
#include <vector>

#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fxcrt/bytestring.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "public/cpp/fpdf_scopers.h"
#include "public/fpdf_doc.h"
#include "public/fpdf_edit.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/fx_string_testhelpers.h"
#include "testing/gtest/include/gtest/gtest.h"

class FPDFDocEmbedderTest : public EmbedderTest {};

TEST_F(FPDFDocEmbedderTest, MultipleSamePage) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document());

  std::set<FPDF_PAGE> unique_pages;
  std::vector<ScopedFPDFPage> owned_pages(4);
  for (auto& ref : owned_pages) {
    ref.reset(FPDF_LoadPage(document(), 0));
    unique_pages.insert(ref.get());
  }
#ifdef PDF_ENABLE_XFA
  EXPECT_EQ(1u, unique_pages.size());
  EXPECT_EQ(1u, pDoc->GetParsedPageCountForTesting());
#else   // PDF_ENABLE_XFA
  EXPECT_EQ(4u, unique_pages.size());
  EXPECT_EQ(4u, pDoc->GetParsedPageCountForTesting());
#endif  // PDF_ENABLE_XFA
}

TEST_F(FPDFDocEmbedderTest, DestGetPageIndex) {
  ASSERT_TRUE(OpenDocument("named_dests.pdf"));

  // NULL argument cases.
  EXPECT_EQ(-1, FPDFDest_GetDestPageIndex(nullptr, nullptr));
  EXPECT_EQ(-1, FPDFDest_GetDestPageIndex(document(), nullptr));

  // Page number directly in item from Dests NameTree.
  FPDF_DEST dest = FPDF_GetNamedDestByName(document(), "First");
  EXPECT_TRUE(dest);
  EXPECT_EQ(1, FPDFDest_GetDestPageIndex(document(), dest));

  // Page number via object reference in item from Dests NameTree.
  dest = FPDF_GetNamedDestByName(document(), "Next");
  EXPECT_TRUE(dest);
  EXPECT_EQ(1, FPDFDest_GetDestPageIndex(document(), dest));

  // Page number directly in item from Dests dictionary.
  dest = FPDF_GetNamedDestByName(document(), "FirstAlternate");
  EXPECT_TRUE(dest);
  EXPECT_EQ(11, FPDFDest_GetDestPageIndex(document(), dest));

  // Invalid object reference in item from Dests NameTree.
  dest = FPDF_GetNamedDestByName(document(), "LastAlternate");
  EXPECT_TRUE(dest);
  EXPECT_EQ(-1, FPDFDest_GetDestPageIndex(document(), dest));
}

TEST_F(FPDFDocEmbedderTest, DestGetView) {
  ASSERT_TRUE(OpenDocument("named_dests.pdf"));

  unsigned long numParams;
  FS_FLOAT params[4];

  numParams = 42;
  std::fill_n(params, 4, 42.4242f);
  EXPECT_EQ(static_cast<unsigned long>(PDFDEST_VIEW_UNKNOWN_MODE),
            FPDFDest_GetView(nullptr, &numParams, params));
  EXPECT_EQ(0U, numParams);
  EXPECT_FLOAT_EQ(42.4242f, params[0]);

  numParams = 42;
  std::fill_n(params, 4, 42.4242f);
  FPDF_DEST dest = FPDF_GetNamedDestByName(document(), "First");
  EXPECT_TRUE(dest);
  EXPECT_EQ(static_cast<unsigned long>(PDFDEST_VIEW_XYZ),
            FPDFDest_GetView(dest, &numParams, params));
  EXPECT_EQ(3U, numParams);
  EXPECT_FLOAT_EQ(0, params[0]);
  EXPECT_FLOAT_EQ(0, params[1]);
  EXPECT_FLOAT_EQ(1, params[2]);
  EXPECT_FLOAT_EQ(42.4242f, params[3]);

  numParams = 42;
  std::fill_n(params, 4, 42.4242f);
  dest = FPDF_GetNamedDestByName(document(), "Next");
  EXPECT_TRUE(dest);
  EXPECT_EQ(static_cast<unsigned long>(PDFDEST_VIEW_FIT),
            FPDFDest_GetView(dest, &numParams, params));
  EXPECT_EQ(0U, numParams);
  EXPECT_FLOAT_EQ(42.4242f, params[0]);

  numParams = 42;
  std::fill_n(params, 4, 42.4242f);
  dest = FPDF_GetNamedDestByName(document(), "FirstAlternate");
  EXPECT_TRUE(dest);
  EXPECT_EQ(static_cast<unsigned long>(PDFDEST_VIEW_XYZ),
            FPDFDest_GetView(dest, &numParams, params));
  EXPECT_EQ(3U, numParams);
  EXPECT_FLOAT_EQ(200, params[0]);
  EXPECT_FLOAT_EQ(400, params[1]);
  EXPECT_FLOAT_EQ(800, params[2]);
  EXPECT_FLOAT_EQ(42.4242f, params[3]);

  numParams = 42;
  std::fill_n(params, 4, 42.4242f);
  dest = FPDF_GetNamedDestByName(document(), "LastAlternate");
  EXPECT_TRUE(dest);
  EXPECT_EQ(static_cast<unsigned long>(PDFDEST_VIEW_XYZ),
            FPDFDest_GetView(dest, &numParams, params));
  EXPECT_EQ(3U, numParams);
  EXPECT_FLOAT_EQ(0, params[0]);
  EXPECT_FLOAT_EQ(0, params[1]);
  EXPECT_FLOAT_EQ(-200, params[2]);
  EXPECT_FLOAT_EQ(42.4242f, params[3]);
}

TEST_F(FPDFDocEmbedderTest, DestGetLocationInPage) {
  ASSERT_TRUE(OpenDocument("named_dests.pdf"));

  FPDF_DEST dest = FPDF_GetNamedDestByName(document(), "First");
  EXPECT_TRUE(dest);

  FPDF_BOOL hasX = 0;
  FPDF_BOOL hasY = 0;
  FPDF_BOOL hasZoom = 0;
  FS_FLOAT x = -1.0f;
  FS_FLOAT y = -1.0f;
  FS_FLOAT zoom = -1.0f;

  // NULL argument case
  EXPECT_FALSE(FPDFDest_GetLocationInPage(nullptr, &hasX, &hasY, &hasZoom, &x,
                                          &y, &zoom));

  // Actual argument case.
  EXPECT_TRUE(
      FPDFDest_GetLocationInPage(dest, &hasX, &hasY, &hasZoom, &x, &y, &zoom));
  EXPECT_TRUE(hasX);
  EXPECT_TRUE(hasY);
  EXPECT_TRUE(hasZoom);
  EXPECT_EQ(0, x);
  EXPECT_EQ(0, y);
  EXPECT_EQ(1, zoom);
}

TEST_F(FPDFDocEmbedderTest, BUG_1506_1) {
  ASSERT_TRUE(OpenDocument("bug_1506.pdf"));

  FPDF_DEST dest = FPDF_GetNamedDestByName(document(), "First");
  ASSERT_TRUE(dest);
  EXPECT_EQ(3, FPDFDest_GetDestPageIndex(document(), dest));
}

TEST_F(FPDFDocEmbedderTest, BUG_1506_2) {
  ASSERT_TRUE(OpenDocument("bug_1506.pdf"));

  std::vector<FPDF_PAGE> pages;
  for (int i : {0, 2})
    pages.push_back(LoadPage(i));

  FPDF_DEST dest = FPDF_GetNamedDestByName(document(), "First");
  ASSERT_TRUE(dest);
  EXPECT_EQ(3, FPDFDest_GetDestPageIndex(document(), dest));

  for (FPDF_PAGE page : pages)
    UnloadPage(page);
}

TEST_F(FPDFDocEmbedderTest, BUG_1506_3) {
  ASSERT_TRUE(OpenDocument("bug_1506.pdf"));

  std::vector<FPDF_PAGE> pages;
  for (int i : {0, 1, 3})
    pages.push_back(LoadPage(i));

  FPDF_DEST dest = FPDF_GetNamedDestByName(document(), "First");
  ASSERT_TRUE(dest);
  EXPECT_EQ(3, FPDFDest_GetDestPageIndex(document(), dest));

  for (FPDF_PAGE page : pages)
    UnloadPage(page);
}

TEST_F(FPDFDocEmbedderTest, BUG_680376) {
  ASSERT_TRUE(OpenDocument("bug_680376.pdf"));

  // Page number directly in item from Dests NameTree.
  FPDF_DEST dest = FPDF_GetNamedDestByName(document(), "First");
  EXPECT_TRUE(dest);
  EXPECT_EQ(-1, FPDFDest_GetDestPageIndex(document(), dest));
}

TEST_F(FPDFDocEmbedderTest, BUG_821454) {
  ASSERT_TRUE(OpenDocument("bug_821454.pdf"));

  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Cover some invalid argument cases while we're at it.
  EXPECT_FALSE(FPDFLink_GetLinkAtPoint(nullptr, 150, 360));
  EXPECT_EQ(-1, FPDFLink_GetLinkZOrderAtPoint(nullptr, 150, 360));

  FPDF_LINK link1 = FPDFLink_GetLinkAtPoint(page, 150, 360);
  ASSERT_TRUE(link1);
  FPDF_LINK link2 = FPDFLink_GetLinkAtPoint(page, 150, 420);
  ASSERT_TRUE(link2);

  EXPECT_EQ(0, FPDFLink_GetLinkZOrderAtPoint(page, 150, 360));
  EXPECT_EQ(1, FPDFLink_GetLinkZOrderAtPoint(page, 150, 420));

  FPDF_DEST dest1 = FPDFLink_GetDest(document(), link1);
  ASSERT_TRUE(dest1);
  FPDF_DEST dest2 = FPDFLink_GetDest(document(), link2);
  ASSERT_TRUE(dest2);

  // Cover more invalid argument cases while we're at it.
  EXPECT_FALSE(FPDFLink_GetDest(nullptr, nullptr));
  EXPECT_FALSE(FPDFLink_GetDest(nullptr, link1));
  EXPECT_FALSE(FPDFLink_GetDest(document(), nullptr));

  EXPECT_EQ(0, FPDFDest_GetDestPageIndex(document(), dest1));
  EXPECT_EQ(0, FPDFDest_GetDestPageIndex(document(), dest2));

  {
    FPDF_BOOL has_x_coord;
    FPDF_BOOL has_y_coord;
    FPDF_BOOL has_zoom;
    FS_FLOAT x;
    FS_FLOAT y;
    FS_FLOAT zoom;
    FPDF_BOOL success = FPDFDest_GetLocationInPage(
        dest1, &has_x_coord, &has_y_coord, &has_zoom, &x, &y, &zoom);
    ASSERT_TRUE(success);
    EXPECT_TRUE(has_x_coord);
    EXPECT_TRUE(has_y_coord);
    EXPECT_FALSE(has_zoom);
    EXPECT_FLOAT_EQ(100.0f, x);
    EXPECT_FLOAT_EQ(200.0f, y);
  }
  {
    FPDF_BOOL has_x_coord;
    FPDF_BOOL has_y_coord;
    FPDF_BOOL has_zoom;
    FS_FLOAT x;
    FS_FLOAT y;
    FS_FLOAT zoom;
    FPDF_BOOL success = FPDFDest_GetLocationInPage(
        dest2, &has_x_coord, &has_y_coord, &has_zoom, &x, &y, &zoom);
    ASSERT_TRUE(success);
    EXPECT_TRUE(has_x_coord);
    EXPECT_TRUE(has_y_coord);
    EXPECT_FALSE(has_zoom);
    EXPECT_FLOAT_EQ(150.0f, x);
    EXPECT_FLOAT_EQ(250.0f, y);
  }

  UnloadPage(page);
}

TEST_F(FPDFDocEmbedderTest, ActionBadArguments) {
  ASSERT_TRUE(OpenDocument("launch_action.pdf"));
  EXPECT_EQ(static_cast<unsigned long>(PDFACTION_UNSUPPORTED),
            FPDFAction_GetType(nullptr));

  EXPECT_FALSE(FPDFAction_GetDest(nullptr, nullptr));
  EXPECT_FALSE(FPDFAction_GetDest(document(), nullptr));
  EXPECT_EQ(0u, FPDFAction_GetFilePath(nullptr, nullptr, 0));
  EXPECT_EQ(0u, FPDFAction_GetURIPath(nullptr, nullptr, nullptr, 0));
  EXPECT_EQ(0u, FPDFAction_GetURIPath(document(), nullptr, nullptr, 0));
}

TEST_F(FPDFDocEmbedderTest, ActionLaunch) {
  ASSERT_TRUE(OpenDocument("launch_action.pdf"));

  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // The target action is nearly the size of the whole page.
  FPDF_LINK link = FPDFLink_GetLinkAtPoint(page, 100, 100);
  ASSERT_TRUE(link);

  FPDF_ACTION action = FPDFLink_GetAction(link);
  ASSERT_TRUE(action);
  EXPECT_EQ(static_cast<unsigned long>(PDFACTION_LAUNCH),
            FPDFAction_GetType(action));

  const char kExpectedResult[] = "test.pdf";
  const unsigned long kExpectedLength = sizeof(kExpectedResult);
  unsigned long bufsize = FPDFAction_GetFilePath(action, nullptr, 0);
  EXPECT_EQ(kExpectedLength, bufsize);

  char buf[1024];
  EXPECT_EQ(bufsize, FPDFAction_GetFilePath(action, buf, bufsize));
  EXPECT_STREQ(kExpectedResult, buf);

  // Other public methods are not appropriate for launch actions.
  EXPECT_FALSE(FPDFAction_GetDest(document(), action));
  EXPECT_EQ(0u, FPDFAction_GetURIPath(document(), action, buf, sizeof(buf)));

  UnloadPage(page);
}

TEST_F(FPDFDocEmbedderTest, ActionUri) {
  ASSERT_TRUE(OpenDocument("uri_action.pdf"));

  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // The target action is nearly the size of the whole page.
  FPDF_LINK link = FPDFLink_GetLinkAtPoint(page, 100, 100);
  ASSERT_TRUE(link);

  FPDF_ACTION action = FPDFLink_GetAction(link);
  ASSERT_TRUE(action);
  EXPECT_EQ(static_cast<unsigned long>(PDFACTION_URI),
            FPDFAction_GetType(action));

  const char kExpectedResult[] = "https://example.com/page.html";
  const unsigned long kExpectedLength = sizeof(kExpectedResult);
  unsigned long bufsize = FPDFAction_GetURIPath(document(), action, nullptr, 0);
  ASSERT_EQ(kExpectedLength, bufsize);

  char buf[1024];
  EXPECT_EQ(bufsize, FPDFAction_GetURIPath(document(), action, buf, bufsize));
  EXPECT_STREQ(kExpectedResult, buf);

  // Other public methods are not appropriate for URI actions
  EXPECT_FALSE(FPDFAction_GetDest(document(), action));
  EXPECT_EQ(0u, FPDFAction_GetFilePath(action, buf, sizeof(buf)));

  UnloadPage(page);
}

TEST_F(FPDFDocEmbedderTest, ActionUriNonAscii) {
  ASSERT_TRUE(OpenDocument("uri_action_nonascii.pdf"));

  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // The target action is nearly the size of the whole page.
  FPDF_LINK link = FPDFLink_GetLinkAtPoint(page, 100, 100);
  ASSERT_TRUE(link);

  FPDF_ACTION action = FPDFLink_GetAction(link);
  ASSERT_TRUE(action);
  EXPECT_EQ(static_cast<unsigned long>(PDFACTION_URI),
            FPDFAction_GetType(action));

  // FPDFAction_GetURIPath() may return data in any encoding, or even with bad
  // encoding.
  const char kExpectedResult[] =
      "https://example.com/\xA5octal\xC7"
      "chars";
  const unsigned long kExpectedLength = sizeof(kExpectedResult);
  unsigned long bufsize = FPDFAction_GetURIPath(document(), action, nullptr, 0);
  ASSERT_EQ(kExpectedLength, bufsize);

  char buf[1024];
  EXPECT_EQ(bufsize, FPDFAction_GetURIPath(document(), action, buf, bufsize));
  EXPECT_STREQ(kExpectedResult, buf);

  UnloadPage(page);
}

TEST_F(FPDFDocEmbedderTest, LinkToAnnotConversion) {
  ASSERT_TRUE(OpenDocument("annots.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  {
    FPDF_LINK first_link = FPDFLink_GetLinkAtPoint(page, 69.00, 653.00);
    ScopedFPDFAnnotation first_annot(FPDFLink_GetAnnot(page, first_link));
    EXPECT_EQ(0, FPDFPage_GetAnnotIndex(page, first_annot.get()));

    FPDF_LINK second_link = FPDFLink_GetLinkAtPoint(page, 80.00, 633.00);
    ScopedFPDFAnnotation second_annot(FPDFLink_GetAnnot(page, second_link));
    EXPECT_EQ(1, FPDFPage_GetAnnotIndex(page, second_annot.get()));

    // Also test invalid arguments.
    EXPECT_FALSE(FPDFLink_GetAnnot(nullptr, nullptr));
    EXPECT_FALSE(FPDFLink_GetAnnot(page, nullptr));
    EXPECT_FALSE(FPDFLink_GetAnnot(nullptr, second_link));
  }

  UnloadPage(page);
}

TEST_F(FPDFDocEmbedderTest, ActionGoto) {
  ASSERT_TRUE(OpenDocument("goto_action.pdf"));

  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // The target action is nearly the size of the whole page.
  FPDF_LINK link = FPDFLink_GetLinkAtPoint(page, 100, 100);
  ASSERT_TRUE(link);

  FPDF_ACTION action = FPDFLink_GetAction(link);
  ASSERT_TRUE(action);
  EXPECT_EQ(static_cast<unsigned long>(PDFACTION_GOTO),
            FPDFAction_GetType(action));

  EXPECT_TRUE(FPDFAction_GetDest(document(), action));

  // Other public methods are not appropriate for GoTo actions.
  char buf[1024];
  EXPECT_EQ(0u, FPDFAction_GetFilePath(action, buf, sizeof(buf)));
  EXPECT_EQ(0u, FPDFAction_GetURIPath(document(), action, buf, sizeof(buf)));

  UnloadPage(page);
}

TEST_F(FPDFDocEmbedderTest, ActionEmbeddedGoto) {
  ASSERT_TRUE(OpenDocument("gotoe_action.pdf"));

  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // The target action is nearly the size of the whole page.
  FPDF_LINK link = FPDFLink_GetLinkAtPoint(page, 100, 100);
  ASSERT_TRUE(link);

  FPDF_ACTION action = FPDFLink_GetAction(link);
  ASSERT_TRUE(action);
  EXPECT_EQ(static_cast<unsigned long>(PDFACTION_EMBEDDEDGOTO),
            FPDFAction_GetType(action));

  FPDF_DEST dest = FPDFAction_GetDest(document(), action);
  EXPECT_TRUE(dest);

  unsigned long num_params = 42;
  FS_FLOAT params[4];
  std::fill_n(params, 4, 42.4242f);
  EXPECT_EQ(static_cast<unsigned long>(PDFDEST_VIEW_FIT),
            FPDFDest_GetView(dest, &num_params, params));
  EXPECT_EQ(0u, num_params);
  EXPECT_FLOAT_EQ(42.4242f, params[0]);

  const char kExpectedResult[] = "ExampleFile.pdf";
  const unsigned long kExpectedLength = sizeof(kExpectedResult);
  char buf[1024];
  unsigned long bufsize = FPDFAction_GetFilePath(action, nullptr, 0);
  EXPECT_EQ(kExpectedLength, bufsize);
  EXPECT_EQ(kExpectedLength, FPDFAction_GetFilePath(action, buf, bufsize));
  EXPECT_STREQ(kExpectedResult, buf);

  UnloadPage(page);
}

TEST_F(FPDFDocEmbedderTest, ActionNonesuch) {
  ASSERT_TRUE(OpenDocument("nonesuch_action.pdf"));

  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // The target action is nearly the size of the whole page.
  FPDF_LINK link = FPDFLink_GetLinkAtPoint(page, 100, 100);
  ASSERT_TRUE(link);

  FPDF_ACTION action = FPDFLink_GetAction(link);
  ASSERT_TRUE(action);
  EXPECT_EQ(static_cast<unsigned long>(PDFACTION_UNSUPPORTED),
            FPDFAction_GetType(action));

  // No public methods are appropriate for unsupported actions.
  char buf[1024];
  EXPECT_FALSE(FPDFAction_GetDest(document(), action));
  EXPECT_EQ(0u, FPDFAction_GetFilePath(action, buf, sizeof(buf)));
  EXPECT_EQ(0u, FPDFAction_GetURIPath(document(), action, buf, sizeof(buf)));

  UnloadPage(page);
}

TEST_F(FPDFDocEmbedderTest, NoBookmarks) {
  unsigned short buf[128];

  // Open a file with no bookmarks.
  ASSERT_TRUE(OpenDocument("named_dests.pdf"));

  // NULL argument cases.
  EXPECT_EQ(0u, FPDFBookmark_GetTitle(nullptr, buf, sizeof(buf)));
  EXPECT_FALSE(FPDFBookmark_GetFirstChild(nullptr, nullptr));
  EXPECT_FALSE(FPDFBookmark_GetFirstChild(document(), nullptr));
  EXPECT_FALSE(FPDFBookmark_GetNextSibling(nullptr, nullptr));
  EXPECT_FALSE(FPDFBookmark_GetNextSibling(document(), nullptr));
  EXPECT_FALSE(FPDFBookmark_Find(nullptr, nullptr));
  EXPECT_FALSE(FPDFBookmark_Find(document(), nullptr));
  EXPECT_FALSE(FPDFBookmark_GetDest(nullptr, nullptr));
  EXPECT_FALSE(FPDFBookmark_GetDest(document(), nullptr));
  EXPECT_FALSE(FPDFBookmark_GetAction(nullptr));
}

TEST_F(FPDFDocEmbedderTest, Bookmarks) {
  unsigned short buf[128];

  // Open a file with many bookmarks.
  ASSERT_TRUE(OpenDocument("bookmarks.pdf"));

  FPDF_BOOKMARK child = FPDFBookmark_GetFirstChild(document(), nullptr);
  EXPECT_TRUE(child);
  EXPECT_EQ(34u, FPDFBookmark_GetTitle(child, buf, sizeof(buf)));
  EXPECT_EQ(L"A Good Beginning", GetPlatformWString(buf));
  EXPECT_EQ(0, FPDFBookmark_GetCount(child));
  EXPECT_EQ(0, FPDFBookmark_GetCount(nullptr));

  EXPECT_FALSE(FPDFBookmark_GetDest(document(), child));
  EXPECT_FALSE(FPDFBookmark_GetAction(child));

  FPDF_BOOKMARK grand_child = FPDFBookmark_GetFirstChild(document(), child);
  EXPECT_FALSE(grand_child);

  FPDF_BOOKMARK sibling = FPDFBookmark_GetNextSibling(document(), child);
  EXPECT_TRUE(sibling);
  EXPECT_EQ(24u, FPDFBookmark_GetTitle(sibling, buf, sizeof(buf)));
  EXPECT_EQ(L"Open Middle", GetPlatformWString(buf));
  EXPECT_TRUE(FPDFBookmark_GetAction(sibling));
  EXPECT_EQ(1, FPDFBookmark_GetCount(sibling));

  FPDF_BOOKMARK sibling2 = FPDFBookmark_GetNextSibling(document(), sibling);
  EXPECT_TRUE(sibling2);
  EXPECT_EQ(42u, FPDFBookmark_GetTitle(sibling2, buf, sizeof(buf)));
  EXPECT_EQ(L"A Good Closed Ending", GetPlatformWString(buf));
  EXPECT_EQ(-2, FPDFBookmark_GetCount(sibling2));

  EXPECT_FALSE(FPDFBookmark_GetNextSibling(document(), sibling2));

  grand_child = FPDFBookmark_GetFirstChild(document(), sibling);
  EXPECT_TRUE(grand_child);
  EXPECT_EQ(46u, FPDFBookmark_GetTitle(grand_child, buf, sizeof(buf)));
  EXPECT_EQ(L"Open Middle Descendant", GetPlatformWString(buf));
  EXPECT_EQ(0, FPDFBookmark_GetCount(grand_child));
  EXPECT_TRUE(FPDFBookmark_GetDest(document(), grand_child));

  EXPECT_FALSE(FPDFBookmark_GetNextSibling(document(), grand_child));
}

TEST_F(FPDFDocEmbedderTest, FindBookmarks) {
  unsigned short buf[128];

  // Open a file with many bookmarks.
  ASSERT_TRUE(OpenDocument("bookmarks.pdf"));

  // Find the first one, based on its known title.
  ScopedFPDFWideString title = GetFPDFWideString(L"A Good Beginning");
  FPDF_BOOKMARK child = FPDFBookmark_Find(document(), title.get());
  EXPECT_TRUE(child);

  // Check that the string matches.
  EXPECT_EQ(34u, FPDFBookmark_GetTitle(child, buf, sizeof(buf)));
  EXPECT_EQ(L"A Good Beginning", GetPlatformWString(buf));

  // Check that it is them same as the one returned by GetFirstChild.
  EXPECT_EQ(child, FPDFBookmark_GetFirstChild(document(), nullptr));

  // Try to find one using a non-existent title.
  ScopedFPDFWideString bad_title = GetFPDFWideString(L"A BAD Beginning");
  EXPECT_FALSE(FPDFBookmark_Find(document(), bad_title.get()));
}

// Check circular bookmarks will not cause infinite loop.
TEST_F(FPDFDocEmbedderTest, FindBookmarks_bug420) {
  // Open a file with circular bookmarks.
  ASSERT_TRUE(OpenDocument("bookmarks_circular.pdf"));

  // Try to find a title.
  ScopedFPDFWideString title = GetFPDFWideString(L"anything");
  EXPECT_FALSE(FPDFBookmark_Find(document(), title.get()));
}

TEST_F(FPDFDocEmbedderTest, DeletePage) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_EQ(1, FPDF_GetPageCount(document()));

  FPDFPage_Delete(nullptr, 0);
  EXPECT_EQ(1, FPDF_GetPageCount(document()));

  FPDFPage_Delete(document(), -1);
  EXPECT_EQ(1, FPDF_GetPageCount(document()));
  FPDFPage_Delete(document(), 1);
  EXPECT_EQ(1, FPDF_GetPageCount(document()));

  FPDFPage_Delete(document(), 0);
  EXPECT_EQ(0, FPDF_GetPageCount(document()));
}

TEST_F(FPDFDocEmbedderTest, GetFileIdentifier) {
  ASSERT_TRUE(OpenDocument("split_streams.pdf"));
  constexpr size_t kMd5Length = 17;
  char buf[kMd5Length];
  EXPECT_EQ(0u,
            FPDF_GetFileIdentifier(document(), static_cast<FPDF_FILEIDTYPE>(-1),
                                   buf, sizeof(buf)));
  EXPECT_EQ(0u,
            FPDF_GetFileIdentifier(document(), static_cast<FPDF_FILEIDTYPE>(2),
                                   buf, sizeof(buf)));
  EXPECT_EQ(0u, FPDF_GetFileIdentifier(nullptr, FILEIDTYPE_PERMANENT, buf,
                                       sizeof(buf)));
  EXPECT_EQ(kMd5Length, FPDF_GetFileIdentifier(document(), FILEIDTYPE_PERMANENT,
                                               nullptr, 0));

  constexpr char kExpectedPermanent[] =
      "\xF3\x41\xAE\x65\x4A\x77\xAC\xD5\x06\x5A\x76\x45\xE5\x96\xE6\xE6";
  ASSERT_EQ(kMd5Length, FPDF_GetFileIdentifier(document(), FILEIDTYPE_PERMANENT,
                                               buf, sizeof(buf)));
  EXPECT_EQ(kExpectedPermanent, ByteString(buf));

  constexpr char kExpectedChanging[] =
      "\xBC\x37\x29\x8A\x3F\x87\xF4\x79\x22\x9B\xCE\x99\x7C\xA7\x91\xF7";
  ASSERT_EQ(kMd5Length, FPDF_GetFileIdentifier(document(), FILEIDTYPE_CHANGING,
                                               buf, sizeof(buf)));
  EXPECT_EQ(kExpectedChanging, ByteString(buf));
}

TEST_F(FPDFDocEmbedderTest, GetNonHexFileIdentifier) {
  ASSERT_TRUE(OpenDocument("non_hex_file_id.pdf"));
  char buf[18];

  constexpr char kPermanentNonHex[] = "permanent non-hex";
  ASSERT_EQ(18u, FPDF_GetFileIdentifier(document(), FILEIDTYPE_PERMANENT, buf,
                                        sizeof(buf)));
  EXPECT_EQ(kPermanentNonHex, ByteString(buf));

  constexpr char kChangingNonHex[] = "changing non-hex";
  ASSERT_EQ(17u, FPDF_GetFileIdentifier(document(), FILEIDTYPE_CHANGING, buf,
                                        sizeof(buf)));
  EXPECT_EQ(kChangingNonHex, ByteString(buf));
}

TEST_F(FPDFDocEmbedderTest, GetNonexistentFileIdentifier) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_EQ(
      0u, FPDF_GetFileIdentifier(document(), FILEIDTYPE_PERMANENT, nullptr, 0));
  EXPECT_EQ(
      0u, FPDF_GetFileIdentifier(document(), FILEIDTYPE_CHANGING, nullptr, 0));
}

TEST_F(FPDFDocEmbedderTest, GetMetaText) {
  ASSERT_TRUE(OpenDocument("bug_601362.pdf"));

  // Invalid document / tag results in 0.
  unsigned short buf[128];
  EXPECT_EQ(0u, FPDF_GetMetaText(document(), nullptr, buf, sizeof(buf)));
  EXPECT_EQ(0u, FPDF_GetMetaText(nullptr, "", buf, sizeof(buf)));

  // Tags that do not eixst results in an empty wide string.
  EXPECT_EQ(2u, FPDF_GetMetaText(document(), "", buf, sizeof(buf)));
  EXPECT_EQ(2u, FPDF_GetMetaText(document(), "foo", buf, sizeof(buf)));
  ASSERT_EQ(2u, FPDF_GetMetaText(document(), "Title", buf, sizeof(buf)));
  ASSERT_EQ(2u, FPDF_GetMetaText(document(), "Author", buf, sizeof(buf)));
  ASSERT_EQ(2u, FPDF_GetMetaText(document(), "Subject", buf, sizeof(buf)));
  ASSERT_EQ(2u, FPDF_GetMetaText(document(), "Keywords", buf, sizeof(buf)));
  ASSERT_EQ(2u, FPDF_GetMetaText(document(), "Producer", buf, sizeof(buf)));

  ASSERT_EQ(30u, FPDF_GetMetaText(document(), "Creator", buf, sizeof(buf)));
  EXPECT_EQ(L"Microsoft Word", GetPlatformWString(buf));

  ASSERT_EQ(48u,
            FPDF_GetMetaText(document(), "CreationDate", buf, sizeof(buf)));
  EXPECT_EQ(L"D:20160411190039+00'00'", GetPlatformWString(buf));

  ASSERT_EQ(48u, FPDF_GetMetaText(document(), "ModDate", buf, sizeof(buf)));
  EXPECT_EQ(L"D:20160411190039+00'00'", GetPlatformWString(buf));
}

TEST_F(FPDFDocEmbedderTest, Bug_182) {
  ASSERT_TRUE(OpenDocument("bug_182.pdf"));

  unsigned short buf[128];

  ASSERT_EQ(48u, FPDF_GetMetaText(document(), "Title", buf, sizeof(buf)));
  EXPECT_EQ(L"Super Visual Formade 印刷", GetPlatformWString(buf));
}

TEST_F(FPDFDocEmbedderTest, GetMetaTextSameObjectNumber) {
  ASSERT_TRUE(OpenDocument("annotation_highlight_square_with_ap.pdf"));

  // The PDF has been edited. It has two %%EOF markers, and 2 objects numbered
  // (1 0). Both objects are /Info dictionaries, but contain different data.
  // Make sure ModDate is the date of the last modification.
  unsigned short buf[128];
  ASSERT_EQ(48u, FPDF_GetMetaText(document(), "ModDate", buf, sizeof(buf)));
  EXPECT_EQ(L"D:20170612232940-04'00'", GetPlatformWString(buf));
}

TEST_F(FPDFDocEmbedderTest, GetMetaTextInAttachmentFile) {
  ASSERT_TRUE(OpenDocument("embedded_attachments.pdf"));

  // Make sure this is the date from the PDF itself and not the attached PDF.
  unsigned short buf[128];
  ASSERT_EQ(48u, FPDF_GetMetaText(document(), "ModDate", buf, sizeof(buf)));
  EXPECT_EQ(L"D:20170712214448-07'00'", GetPlatformWString(buf));
}

TEST_F(FPDFDocEmbedderTest, GetMetaTextFromNewDocument) {
  FPDF_DOCUMENT empty_doc = FPDF_CreateNewDocument();
  unsigned short buf[128];
  EXPECT_EQ(2u, FPDF_GetMetaText(empty_doc, "Title", buf, sizeof(buf)));
  FPDF_CloseDocument(empty_doc);
}

TEST_F(FPDFDocEmbedderTest, GetPageAAction) {
  ASSERT_TRUE(OpenDocument("get_page_aaction.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);

  EXPECT_FALSE(FPDF_GetPageAAction(nullptr, FPDFPAGE_AACTION_OPEN));
  EXPECT_FALSE(FPDF_GetPageAAction(page, FPDFPAGE_AACTION_CLOSE));
  EXPECT_FALSE(FPDF_GetPageAAction(page, -1));
  EXPECT_FALSE(FPDF_GetPageAAction(page, 999));

  FPDF_ACTION action = FPDF_GetPageAAction(page, FPDFPAGE_AACTION_OPEN);
  EXPECT_EQ(static_cast<unsigned long>(PDFACTION_EMBEDDEDGOTO),
            FPDFAction_GetType(action));

  const char kExpectedResult[] = "\\\\127.0.0.1\\c$\\Program Files\\test.exe";
  const unsigned long kExpectedLength = sizeof(kExpectedResult);
  char buf[1024];

  unsigned long bufsize = FPDFAction_GetFilePath(action, nullptr, 0);
  EXPECT_EQ(kExpectedLength, bufsize);
  EXPECT_EQ(kExpectedLength, FPDFAction_GetFilePath(action, buf, bufsize));
  EXPECT_STREQ(kExpectedResult, buf);

  UnloadPage(page);

  page = LoadPage(1);
  EXPECT_TRUE(page);
  EXPECT_FALSE(FPDF_GetPageAAction(page, -1));

  UnloadPage(page);
}

TEST_F(FPDFDocEmbedderTest, NoPageLabels) {
  ASSERT_TRUE(OpenDocument("about_blank.pdf"));
  EXPECT_EQ(1, FPDF_GetPageCount(document()));

  ASSERT_EQ(0u, FPDF_GetPageLabel(document(), 0, nullptr, 0));
}

TEST_F(FPDFDocEmbedderTest, GetPageLabels) {
  ASSERT_TRUE(OpenDocument("page_labels.pdf"));
  EXPECT_EQ(7, FPDF_GetPageCount(document()));

  // We do not request labels, when use FPDFAvail_IsXXXAvail.
  // Flush all data, to allow read labels.
  SetWholeFileAvailable();

  unsigned short buf[128];
  EXPECT_EQ(0u, FPDF_GetPageLabel(document(), -2, buf, sizeof(buf)));
  EXPECT_EQ(0u, FPDF_GetPageLabel(document(), -1, buf, sizeof(buf)));

  ASSERT_EQ(4u, FPDF_GetPageLabel(document(), 0, buf, sizeof(buf)));
  EXPECT_EQ(L"i", GetPlatformWString(buf));

  ASSERT_EQ(6u, FPDF_GetPageLabel(document(), 1, buf, sizeof(buf)));
  EXPECT_EQ(L"ii", GetPlatformWString(buf));

  ASSERT_EQ(4u, FPDF_GetPageLabel(document(), 2, buf, sizeof(buf)));
  EXPECT_EQ(L"1", GetPlatformWString(buf));

  ASSERT_EQ(4u, FPDF_GetPageLabel(document(), 3, buf, sizeof(buf)));
  EXPECT_EQ(L"2", GetPlatformWString(buf));

  ASSERT_EQ(8u, FPDF_GetPageLabel(document(), 4, buf, sizeof(buf)));
  EXPECT_EQ(L"zzA", GetPlatformWString(buf));

  ASSERT_EQ(8u, FPDF_GetPageLabel(document(), 5, buf, sizeof(buf)));
  EXPECT_EQ(L"zzB", GetPlatformWString(buf));

  ASSERT_EQ(2u, FPDF_GetPageLabel(document(), 6, buf, sizeof(buf)));
  EXPECT_EQ(L"", GetPlatformWString(buf));

  ASSERT_EQ(0u, FPDF_GetPageLabel(document(), 7, buf, sizeof(buf)));
  ASSERT_EQ(0u, FPDF_GetPageLabel(document(), 8, buf, sizeof(buf)));
}

#ifdef PDF_ENABLE_XFA
TEST_F(FPDFDocEmbedderTest, GetXFALinks) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ScopedFPDFPage page(FPDF_LoadPage(document(), 0));
  ASSERT_TRUE(page);

  FPDFLink_GetLinkAtPoint(page.get(), 150, 360);
  FPDFLink_GetLinkAtPoint(page.get(), 150, 420);

  // Test passes if it doesn't crash. See https://crbug.com/840922
}
#endif  // PDF_ENABLE_XFA
