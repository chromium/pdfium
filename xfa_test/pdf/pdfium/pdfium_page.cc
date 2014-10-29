// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pdf/pdfium/pdfium_page.h"

#include <math.h>

#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "pdf/pdfium/pdfium_engine.h"

// Used when doing hit detection.
#define kTolerance 20.0

// Dictionary Value key names for returning the accessible page content as JSON.
const char kPageWidth[] = "width";
const char kPageHeight[] = "height";
const char kPageTextBox[] = "textBox";
const char kTextBoxLeft[] = "left";
const char kTextBoxTop[]  = "top";
const char kTextBoxWidth[] = "width";
const char kTextBoxHeight[]  = "height";
const char kTextBoxFontSize[] = "fontSize";
const char kTextBoxNodes[] = "textNodes";
const char kTextNodeType[] = "type";
const char kTextNodeText[] = "text";
const char kTextNodeURL[] = "url";
const char kTextNodeTypeText[] = "text";
const char kTextNodeTypeURL[] = "url";
const char kDocLinkURLPrefix[] = "#page";

namespace chrome_pdf {

PDFiumPage::PDFiumPage(PDFiumEngine* engine,
                       int i,
                       const pp::Rect& r,
                       bool available)
    : engine_(engine),
      page_(NULL),
      text_page_(NULL),
      index_(i),
      rect_(r),
      calculated_links_(false),
      available_(available) {
}

PDFiumPage::~PDFiumPage() {
}

void PDFiumPage::Unload() {
  if (text_page_) {
    FPDFText_ClosePage(text_page_);
    text_page_ = NULL;
  }

  if (page_) {
    if (engine_->form()) {
      FORM_OnBeforeClosePage(page_, engine_->form());
    }
    FPDF_ClosePage(page_);
    page_ = NULL;
  }
}

FPDF_PAGE PDFiumPage::GetPage() {
  ScopedUnsupportedFeature scoped_unsupported_feature(engine_);
  if (!available_)
    return NULL;
  if (!page_) {
    page_ = FPDF_LoadPage(engine_->doc(), index_);
    if (page_ && engine_->form()) {
      FORM_OnAfterLoadPage(page_, engine_->form());
    }
  }
  return page_;
}

FPDF_PAGE PDFiumPage::GetPrintPage() {
  ScopedUnsupportedFeature scoped_unsupported_feature(engine_);
  if (!available_)
    return NULL;
  if (!page_)
    page_ = FPDF_LoadPage(engine_->doc(), index_);
  return page_;
}

void PDFiumPage::ClosePrintPage() {
  if (page_) {
    FPDF_ClosePage(page_);
    page_ = NULL;
  }
}

FPDF_TEXTPAGE PDFiumPage::GetTextPage() {
  if (!available_)
    return NULL;
  if (!text_page_)
    text_page_ = FPDFText_LoadPage(GetPage());
  return text_page_;
}

base::Value* PDFiumPage::GetAccessibleContentAsValue(int rotation) {
  base::DictionaryValue* node = new base::DictionaryValue();

  if (!available_)
    return node;

  double width = FPDF_GetPageWidth(GetPage());
  double height = FPDF_GetPageHeight(GetPage());

  base::ListValue* text = new base::ListValue();
  int box_count = FPDFText_CountRects(GetTextPage(), 0, GetCharCount());
  for (int i = 0; i < box_count; i++) {
    double left, top, right, bottom;
    FPDFText_GetRect(GetTextPage(), i, &left, &top, &right, &bottom);
    text->Append(
        GetTextBoxAsValue(height, left, top, right, bottom, rotation));
  }

  node->SetDouble(kPageWidth, width);
  node->SetDouble(kPageHeight, height);
  node->Set(kPageTextBox, text);  // Takes ownership of |text|

  return node;
}

base::Value* PDFiumPage::GetTextBoxAsValue(double page_height,
                                           double left, double top,
                                           double right, double bottom,
                                           int rotation) {
  base::string16 text_utf16;
  int char_count =
    FPDFText_GetBoundedText(GetTextPage(), left, top, right, bottom, NULL, 0);
  if (char_count > 0) {
    unsigned short* data = reinterpret_cast<unsigned short*>(
        WriteInto(&text_utf16, char_count + 1));
    FPDFText_GetBoundedText(GetTextPage(),
                            left, top, right, bottom,
                            data, char_count);
  }
  std::string text_utf8 = base::UTF16ToUTF8(text_utf16);

  FPDF_LINK link = FPDFLink_GetLinkAtPoint(GetPage(), left, top);
  Area area;
  std::vector<LinkTarget> targets;
  if (link) {
    targets.push_back(LinkTarget());
    area = GetLinkTarget(link, &targets[0]);
  } else {
    pp::Rect rect(
        PageToScreen(pp::Point(), 1.0, left, top, right, bottom, rotation));
    GetLinks(rect, &targets);
    area = targets.size() == 0 ? TEXT_AREA : WEBLINK_AREA;
  }

  int char_index = FPDFText_GetCharIndexAtPos(GetTextPage(), left, top,
                                              kTolerance, kTolerance);
  double font_size = FPDFText_GetFontSize(GetTextPage(), char_index);

  base::DictionaryValue* node = new base::DictionaryValue();
  node->SetDouble(kTextBoxLeft, left);
  node->SetDouble(kTextBoxTop, page_height - top);
  node->SetDouble(kTextBoxWidth, right - left);
  node->SetDouble(kTextBoxHeight, top - bottom);
  node->SetDouble(kTextBoxFontSize, font_size);

  base::ListValue* text_nodes = new base::ListValue();

  if (area == DOCLINK_AREA) {
    std::string url = kDocLinkURLPrefix + base::IntToString(targets[0].page);
    text_nodes->Append(CreateURLNode(text_utf8, url));
  } else if (area == WEBLINK_AREA && link) {
    text_nodes->Append(CreateURLNode(text_utf8, targets[0].url));
  } else if (area == WEBLINK_AREA && !link) {
    size_t start = 0;
    for (size_t i = 0; i < targets.size(); ++i) {
      // Remove the extra NULL character at end.
      // Otherwise, find() will not return any matches.
      if (targets[i].url.size() > 0 &&
          targets[i].url[targets[i].url.size() - 1] == '\0') {
        targets[i].url.resize(targets[i].url.size() - 1);
      }
      // There should only ever be one NULL character
      DCHECK(targets[i].url[targets[i].url.size() - 1] != '\0');

      // PDFium may change the case of generated links.
      std::string lowerCaseURL = base::StringToLowerASCII(targets[i].url);
      std::string lowerCaseText = base::StringToLowerASCII(text_utf8);
      size_t pos = lowerCaseText.find(lowerCaseURL, start);
      size_t length = targets[i].url.size();
      if (pos == std::string::npos) {
        // Check if the link is a "mailto:" URL
        if (lowerCaseURL.compare(0, 7, "mailto:") == 0) {
          pos = lowerCaseText.find(lowerCaseURL.substr(7), start);
          length -= 7;
        }

        if (pos == std::string::npos) {
          // No match has been found.  This should never happen.
          continue;
        }
      }

      std::string before_text = text_utf8.substr(start, pos - start);
      if (before_text.size() > 0)
        text_nodes->Append(CreateTextNode(before_text));
      std::string link_text = text_utf8.substr(pos, length);
      text_nodes->Append(CreateURLNode(link_text, targets[i].url));

      start = pos + length;
    }
    std::string before_text = text_utf8.substr(start);
    if (before_text.size() > 0)
      text_nodes->Append(CreateTextNode(before_text));
  } else {
    text_nodes->Append(CreateTextNode(text_utf8));
  }

  node->Set(kTextBoxNodes, text_nodes);  // Takes ownership of |text_nodes|.
  return node;
}

base::Value* PDFiumPage::CreateTextNode(std::string text) {
  base::DictionaryValue* node = new base::DictionaryValue();
  node->SetString(kTextNodeType, kTextNodeTypeText);
  node->SetString(kTextNodeText, text);
  return node;
}

base::Value* PDFiumPage::CreateURLNode(std::string text, std::string url) {
  base::DictionaryValue* node = new base::DictionaryValue();
  node->SetString(kTextNodeType, kTextNodeTypeURL);
  node->SetString(kTextNodeText, text);
  node->SetString(kTextNodeURL, url);
  return node;
}

PDFiumPage::Area PDFiumPage::GetCharIndex(const pp::Point& point,
                                          int rotation,
                                          int* char_index,
                                          LinkTarget* target) {
  if (!available_)
    return NONSELECTABLE_AREA;
  pp::Point point2 = point - rect_.point();
  double new_x, new_y;
  FPDF_DeviceToPage(GetPage(), 0, 0, rect_.width(), rect_.height(),
        rotation, point2.x(), point2.y(), &new_x, &new_y);

  int rv = FPDFText_GetCharIndexAtPos(
      GetTextPage(), new_x, new_y, kTolerance, kTolerance);
  *char_index = rv;

  FPDF_LINK link = FPDFLink_GetLinkAtPoint(GetPage(), new_x, new_y);
  if (link) {
    // We don't handle all possible link types of the PDF. For example,
    // launch actions, cross-document links, etc.
    // In that case, GetLinkTarget() will return NONSELECTABLE_AREA
    // and we should proceed with area detection.
    PDFiumPage::Area area = GetLinkTarget(link, target);
    if (area != PDFiumPage::NONSELECTABLE_AREA)
      return area;
  }

  if (rv < 0)
    return NONSELECTABLE_AREA;

  return GetLink(*char_index, target) != -1 ? WEBLINK_AREA : TEXT_AREA;
}

base::char16 PDFiumPage::GetCharAtIndex(int index) {
  if (!available_)
    return L'\0';
  return static_cast<base::char16>(FPDFText_GetUnicode(GetTextPage(), index));
}

int PDFiumPage::GetCharCount() {
  if (!available_)
    return 0;
  return FPDFText_CountChars(GetTextPage());
}

PDFiumPage::Area PDFiumPage::GetLinkTarget(
    FPDF_LINK link, PDFiumPage::LinkTarget* target) {
  FPDF_DEST dest = FPDFLink_GetDest(engine_->doc(), link);
  if (dest != NULL)
    return GetDestinationTarget(dest, target);

  FPDF_ACTION action = FPDFLink_GetAction(link);
  if (action) {
    switch (FPDFAction_GetType(action)) {
      case PDFACTION_GOTO: {
          FPDF_DEST dest = FPDFAction_GetDest(engine_->doc(), action);
          if (dest)
            return GetDestinationTarget(dest, target);
          // TODO(gene): We don't fully support all types of the in-document
          // links. Need to implement that. There is a bug to track that:
          // http://code.google.com/p/chromium/issues/detail?id=55776
        } break;
      case PDFACTION_URI: {
          if (target) {
            size_t buffer_size =
                FPDFAction_GetURIPath(engine_->doc(), action, NULL, 0);
            if (buffer_size > 1) {
              void* data = WriteInto(&target->url, buffer_size + 1);
              FPDFAction_GetURIPath(engine_->doc(), action, data, buffer_size);
            }
          }
          return WEBLINK_AREA;
        } break;
      // TODO(gene): We don't support PDFACTION_REMOTEGOTO and PDFACTION_LAUNCH
      // at the moment.
    }
  }

  return NONSELECTABLE_AREA;
}

PDFiumPage::Area PDFiumPage::GetDestinationTarget(
    FPDF_DEST destination, PDFiumPage::LinkTarget* target) {
  int page_index = FPDFDest_GetPageIndex(engine_->doc(), destination);
  if (target) {
    target->page = page_index;
  }
  return DOCLINK_AREA;
}

int PDFiumPage::GetLink(int char_index, PDFiumPage::LinkTarget* target) {
  if (!available_)
    return -1;

  CalculateLinks();

  // Get the bounding box of the rect again, since it might have moved because
  // of the tolerance above.
  double left, right, bottom, top;
  FPDFText_GetCharBox(GetTextPage(), char_index, &left, &right, &bottom, &top);

  pp::Point origin(
      PageToScreen(pp::Point(), 1.0, left, top, right, bottom, 0).point());
  for (size_t i = 0; i < links_.size(); ++i) {
    for (size_t j = 0; j < links_[i].rects.size(); ++j) {
      if (links_[i].rects[j].Contains(origin)) {
        if (target)
          target->url = links_[i].url;
        return i;
      }
    }
  }
  return -1;
}

std::vector<int> PDFiumPage::GetLinks(pp::Rect text_area,
                                      std::vector<LinkTarget>* targets) {
  if (!available_)
    return std::vector<int>();

  CalculateLinks();

  std::vector<int> links;

  for (size_t i = 0; i < links_.size(); ++i) {
    for (size_t j = 0; j < links_[i].rects.size(); ++j) {
      if (links_[i].rects[j].Intersects(text_area)) {
        if (targets) {
          LinkTarget target;
          target.url = links_[i].url;
          targets->push_back(target);
        }
        links.push_back(i);
      }
    }
  }
  return links;
}

void PDFiumPage::CalculateLinks() {
  if (calculated_links_)
    return;

  calculated_links_ = true;
  FPDF_PAGELINK links = FPDFLink_LoadWebLinks(GetTextPage());
  int count = FPDFLink_CountWebLinks(links);
  for (int i = 0; i < count; ++i) {
    base::string16 url;
    int url_length = FPDFLink_GetURL(links, i, NULL, 0);
    if (url_length > 1) {  // WriteInto needs at least 2 characters.
      unsigned short* data =
          reinterpret_cast<unsigned short*>(WriteInto(&url, url_length + 1));
      FPDFLink_GetURL(links, i, data, url_length);
    }
    Link link;
    link.url = base::UTF16ToUTF8(url);

    // If the link cannot be converted to a pp::Var, then it is not possible to
    // pass it to JS. In this case, ignore the link like other PDF viewers.
    // See http://crbug.com/312882 for an example.
    pp::Var link_var(link.url);
    if (!link_var.is_string())
      continue;

    // Make sure all the characters in the URL are valid per RFC 1738.
    // http://crbug.com/340326 has a sample bad PDF.
    // GURL does not work correctly, e.g. it just strips \t \r \n.
    bool is_invalid_url = false;
    for (size_t j = 0; j < link.url.length(); ++j) {
      // Control characters are not allowed.
      // 0x7F is also a control character.
      // 0x80 and above are not in US-ASCII.
      if (link.url[j] < ' ' || link.url[j] >= '\x7F') {
        is_invalid_url = true;
        break;
      }
    }
    if (is_invalid_url)
      continue;

    int rect_count = FPDFLink_CountRects(links, i);
    for (int j = 0; j < rect_count; ++j) {
      double left, top, right, bottom;
      FPDFLink_GetRect(links, i, j, &left, &top, &right, &bottom);
      link.rects.push_back(
          PageToScreen(pp::Point(), 1.0, left, top, right, bottom, 0));
    }
    links_.push_back(link);
  }
  FPDFLink_CloseWebLinks(links);
}

pp::Rect PDFiumPage::PageToScreen(const pp::Point& offset,
                                  double zoom,
                                  double left,
                                  double top,
                                  double right,
                                  double bottom,
                                  int rotation) {
  if (!available_)
    return pp::Rect();

  int new_left, new_top, new_right, new_bottom;
  FPDF_PageToDevice(
      page_,
      static_cast<int>((rect_.x() - offset.x()) * zoom),
      static_cast<int>((rect_.y() - offset.y()) * zoom),
      static_cast<int>(ceil(rect_.width() * zoom)),
      static_cast<int>(ceil(rect_.height() * zoom)),
      rotation, left, top, &new_left, &new_top);
  FPDF_PageToDevice(
      page_,
      static_cast<int>((rect_.x() - offset.x()) * zoom),
      static_cast<int>((rect_.y() - offset.y()) * zoom),
      static_cast<int>(ceil(rect_.width() * zoom)),
      static_cast<int>(ceil(rect_.height() * zoom)),
      rotation, right, bottom, &new_right, &new_bottom);

  // If the PDF is rotated, the horizontal/vertical coordinates could be
  // flipped.  See
  // http://www.netl.doe.gov/publications/proceedings/03/ubc/presentations/Goeckner-pres.pdf
  if (new_right < new_left)
    std::swap(new_right, new_left);
  if (new_bottom < new_top)
    std::swap(new_bottom, new_top);

  return pp::Rect(
      new_left, new_top, new_right - new_left + 1, new_bottom - new_top + 1);
}

PDFiumPage::Link::Link() {
}

PDFiumPage::Link::~Link() {
}

}  // namespace chrome_pdf
