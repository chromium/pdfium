// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDF_PDFIUM_PDFIUM_PAGE_H_
#define PDF_PDFIUM_PDFIUM_PAGE_H_

#include <string>
#include <vector>

#include "base/strings/string16.h"
#include "ppapi/cpp/rect.h"
#include "third_party/pdfium/fpdfsdk/include/fpdfdoc.h"
#include "third_party/pdfium/fpdfsdk/include/fpdfformfill.h"
#include "third_party/pdfium/fpdfsdk/include/fpdftext.h"

namespace base {
class Value;
}

namespace chrome_pdf {

class PDFiumEngine;

// Wrapper around a page from the document.
class PDFiumPage {
 public:
  PDFiumPage(PDFiumEngine* engine,
             int i,
             const pp::Rect& r,
             bool available);
  ~PDFiumPage();
  // Unloads the PDFium data for this page from memory.
  void Unload();
  // Gets the FPDF_PAGE for this page, loading and parsing it if necessary.
  FPDF_PAGE GetPage();
  //Get the FPDF_PAGE for printing.
  FPDF_PAGE GetPrintPage();
  //Close the printing page.
  void ClosePrintPage();

  // Returns FPDF_TEXTPAGE for the page, loading and parsing it if necessary.
  FPDF_TEXTPAGE GetTextPage();

  // Returns a DictionaryValue version of the page.
  base::Value* GetAccessibleContentAsValue(int rotation);

  enum Area {
    NONSELECTABLE_AREA,
    TEXT_AREA,
    WEBLINK_AREA,  // Area is a hyperlink.
    DOCLINK_AREA,  // Area is a link to a different part of the same document.
  };

  struct LinkTarget {
    // We are using std::string here which have a copy contructor.
    // That prevents us from using union here.
    std::string url;  // Valid for WEBLINK_AREA only.
    int page;         // Valid for DOCLINK_AREA only.
  };

  // Given a point in the document that's in this page, returns its character
  // index if it's near a character, and also the type of text.
  // Target is optional. It will be filled in for WEBLINK_AREA or
  // DOCLINK_AREA only.
  Area GetCharIndex(const pp::Point& point, int rotation, int* char_index,
                    LinkTarget* target);

  // Gets the character at the given index.
  base::char16 GetCharAtIndex(int index);

  // Gets the number of characters in the page.
  int GetCharCount();

  // Converts from page coordinates to screen coordinates.
  pp::Rect PageToScreen(const pp::Point& offset,
                        double zoom,
                        double left,
                        double top,
                        double right,
                        double bottom,
                        int rotation);

  int index() const { return index_; }
  pp::Rect rect() const { return rect_; }
  void set_rect(const pp::Rect& r) { rect_ = r; }
  bool available() const { return available_; }
  void set_available(bool available) { available_ = available; }
  void set_calculated_links(bool calculated_links) {
     calculated_links_ = calculated_links;
  }

 private:
  // Returns a link index if the given character index is over a link, or -1
  // otherwise.
  int GetLink(int char_index, LinkTarget* target);
  // Returns the link indices if the given rect intersects a link rect, or an
  // empty vector otherwise.
  std::vector<int> GetLinks(pp::Rect text_area,
                            std::vector<LinkTarget>* targets);
  // Calculate the locations of any links on the page.
  void CalculateLinks();
  // Returns link type and target associated with a link. Returns
  // NONSELECTABLE_AREA if link detection failed.
  Area GetLinkTarget(FPDF_LINK link, LinkTarget* target);
  // Returns target associated with a destination.
  Area GetDestinationTarget(FPDF_DEST destination, LinkTarget* target);
  // Returns the text in the supplied box as a Value Node
  base::Value* GetTextBoxAsValue(double page_height, double left, double top,
                                 double right, double bottom, int rotation);
  // Helper functions for JSON generation
  base::Value* CreateTextNode(std::string text);
  base::Value* CreateURLNode(std::string text, std::string url);

  struct Link {
    Link();
    ~Link();

    std::string url;
    // Bounding rectangles of characters.
    std::vector<pp::Rect> rects;
  };

  PDFiumEngine* engine_;
  FPDF_PAGE page_;
  FPDF_TEXTPAGE text_page_;
  int index_;
  pp::Rect rect_;
  bool calculated_links_;
  std::vector<Link> links_;
  bool available_;
};

}  // namespace chrome_pdf

#endif  // PDF_PDFIUM_PDFIUM_PAGE_H_
