// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bitset>
#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#if defined PDF_ENABLE_SKIA && !defined _SKIA_SUPPORT_
#define _SKIA_SUPPORT_
#endif

#include "public/cpp/fpdf_deleters.h"
#include "public/fpdf_annot.h"
#include "public/fpdf_attachment.h"
#include "public/fpdf_dataavail.h"
#include "public/fpdf_edit.h"
#include "public/fpdf_ext.h"
#include "public/fpdf_formfill.h"
#include "public/fpdf_progressive.h"
#include "public/fpdf_structtree.h"
#include "public/fpdf_text.h"
#include "public/fpdfview.h"
#include "testing/image_diff/image_diff_png.h"
#include "testing/test_support.h"
#include "third_party/base/logging.h"

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#ifdef ENABLE_CALLGRIND
#include <valgrind/callgrind.h>
#endif  // ENABLE_CALLGRIND

#ifdef PDF_ENABLE_V8
#include "v8/include/libplatform/libplatform.h"
#include "v8/include/v8.h"
#endif  // PDF_ENABLE_V8

#ifdef PDF_ENABLE_SKIA
#include "third_party/skia/include/core/SkPictureRecorder.h"
#include "third_party/skia/include/core/SkStream.h"
#endif

#ifdef _WIN32
#define access _access
#define snprintf _snprintf
#define R_OK 4
#endif

enum OutputFormat {
  OUTPUT_NONE,
  OUTPUT_STRUCTURE,
  OUTPUT_TEXT,
  OUTPUT_PPM,
  OUTPUT_PNG,
  OUTPUT_ANNOT,
#ifdef _WIN32
  OUTPUT_BMP,
  OUTPUT_EMF,
  OUTPUT_PS2,
  OUTPUT_PS3,
#endif
#ifdef PDF_ENABLE_SKIA
  OUTPUT_SKP,
#endif
};

namespace {

struct Options {
  Options()
      : show_config(false),
        show_metadata(false),
        send_events(false),
        render_oneshot(false),
        save_attachments(false),
        save_images(false),
#ifdef ENABLE_CALLGRIND
        callgrind_delimiters(false),
#endif  // ENABLE_CALLGRIND
        pages(false),
        md5(false),
        output_format(OUTPUT_NONE) {
  }

  bool show_config;
  bool show_metadata;
  bool send_events;
  bool render_oneshot;
  bool save_attachments;
  bool save_images;
#ifdef ENABLE_CALLGRIND
  bool callgrind_delimiters;
#endif  // ENABLE_CALLGRIND
  bool pages;
  bool md5;
  OutputFormat output_format;
  std::string scale_factor_as_string;
  std::string exe_path;
  std::string bin_directory;
  std::string font_directory;
  // 0-based page numbers to be rendered.
  int first_page;
  int last_page;
};

struct FPDF_FORMFILLINFO_PDFiumTest : public FPDF_FORMFILLINFO {
  // Hold a map of the currently loaded pages in order to avoid them
  // to get loaded twice.
  std::map<int, std::unique_ptr<void, FPDFPageDeleter>> loaded_pages;

  // Hold a pointer of FPDF_FORMHANDLE so that PDFium app hooks can
  // make use of it.
  FPDF_FORMHANDLE form_handle;
};

FPDF_FORMFILLINFO_PDFiumTest* ToPDFiumTestFormFillInfo(
    FPDF_FORMFILLINFO* form_fill_info) {
  return static_cast<FPDF_FORMFILLINFO_PDFiumTest*>(form_fill_info);
}

bool CheckDimensions(int stride, int width, int height) {
  if (stride < 0 || width < 0 || height < 0)
    return false;
  if (height > 0 && width > INT_MAX / height)
    return false;
  return true;
}

void OutputMD5Hash(const char* file_name, const char* buffer, int len) {
  // Get the MD5 hash and write it to stdout.
  std::string hash =
      GenerateMD5Base16(reinterpret_cast<const uint8_t*>(buffer), len);
  printf("MD5:%s:%s\n", file_name, hash.c_str());
}

std::string WritePpm(const char* pdf_name,
                     int num,
                     const void* buffer_void,
                     int stride,
                     int width,
                     int height) {
  const auto* buffer = reinterpret_cast<const char*>(buffer_void);

  if (!CheckDimensions(stride, width, height))
    return "";

  int out_len = width * height;
  if (out_len > INT_MAX / 3)
    return "";
  out_len *= 3;

  char filename[256];
  snprintf(filename, sizeof(filename), "%s.%d.ppm", pdf_name, num);
  FILE* fp = fopen(filename, "wb");
  if (!fp)
    return "";
  fprintf(fp, "P6\n# PDF test render\n%d %d\n255\n", width, height);
  // Source data is B, G, R, unused.
  // Dest data is R, G, B.
  std::vector<char> result(out_len);
  for (int h = 0; h < height; ++h) {
    const char* src_line = buffer + (stride * h);
    char* dest_line = result.data() + (width * h * 3);
    for (int w = 0; w < width; ++w) {
      // R
      dest_line[w * 3] = src_line[(w * 4) + 2];
      // G
      dest_line[(w * 3) + 1] = src_line[(w * 4) + 1];
      // B
      dest_line[(w * 3) + 2] = src_line[w * 4];
    }
  }
  if (fwrite(result.data(), out_len, 1, fp) != 1)
    fprintf(stderr, "Failed to write to %s\n", filename);
  fclose(fp);
  return std::string(filename);
}

void WriteText(FPDF_PAGE page, const char* pdf_name, int num) {
  char filename[256];
  int chars_formatted =
      snprintf(filename, sizeof(filename), "%s.%d.txt", pdf_name, num);
  if (chars_formatted < 0 ||
      static_cast<size_t>(chars_formatted) >= sizeof(filename)) {
    fprintf(stderr, "Filename %s is too long\n", filename);
    return;
  }

  FILE* fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Failed to open %s for output\n", filename);
    return;
  }

  // Output in UTF32-LE.
  uint32_t bom = 0x0000FEFF;
  if (fwrite(&bom, sizeof(bom), 1, fp) != 1) {
    fprintf(stderr, "Failed to write to %s\n", filename);
    (void)fclose(fp);
    return;
  }

  std::unique_ptr<void, FPDFTextPageDeleter> textpage(FPDFText_LoadPage(page));
  for (int i = 0; i < FPDFText_CountChars(textpage.get()); i++) {
    uint32_t c = FPDFText_GetUnicode(textpage.get(), i);
    if (fwrite(&c, sizeof(c), 1, fp) != 1) {
      fprintf(stderr, "Failed to write to %s\n", filename);
      break;
    }
  }
  (void)fclose(fp);
}

const char* AnnotSubtypeToCString(FPDF_ANNOTATION_SUBTYPE subtype) {
  if (subtype == FPDF_ANNOT_TEXT)
    return "Text";
  if (subtype == FPDF_ANNOT_LINK)
    return "Link";
  if (subtype == FPDF_ANNOT_FREETEXT)
    return "FreeText";
  if (subtype == FPDF_ANNOT_LINE)
    return "Line";
  if (subtype == FPDF_ANNOT_SQUARE)
    return "Square";
  if (subtype == FPDF_ANNOT_CIRCLE)
    return "Circle";
  if (subtype == FPDF_ANNOT_POLYGON)
    return "Polygon";
  if (subtype == FPDF_ANNOT_POLYLINE)
    return "PolyLine";
  if (subtype == FPDF_ANNOT_HIGHLIGHT)
    return "Highlight";
  if (subtype == FPDF_ANNOT_UNDERLINE)
    return "Underline";
  if (subtype == FPDF_ANNOT_SQUIGGLY)
    return "Squiggly";
  if (subtype == FPDF_ANNOT_STRIKEOUT)
    return "StrikeOut";
  if (subtype == FPDF_ANNOT_STAMP)
    return "Stamp";
  if (subtype == FPDF_ANNOT_CARET)
    return "Caret";
  if (subtype == FPDF_ANNOT_INK)
    return "Ink";
  if (subtype == FPDF_ANNOT_POPUP)
    return "Popup";
  if (subtype == FPDF_ANNOT_FILEATTACHMENT)
    return "FileAttachment";
  if (subtype == FPDF_ANNOT_SOUND)
    return "Sound";
  if (subtype == FPDF_ANNOT_MOVIE)
    return "Movie";
  if (subtype == FPDF_ANNOT_WIDGET)
    return "Widget";
  if (subtype == FPDF_ANNOT_SCREEN)
    return "Screen";
  if (subtype == FPDF_ANNOT_PRINTERMARK)
    return "PrinterMark";
  if (subtype == FPDF_ANNOT_TRAPNET)
    return "TrapNet";
  if (subtype == FPDF_ANNOT_WATERMARK)
    return "Watermark";
  if (subtype == FPDF_ANNOT_THREED)
    return "3D";
  if (subtype == FPDF_ANNOT_RICHMEDIA)
    return "RichMedia";
  if (subtype == FPDF_ANNOT_XFAWIDGET)
    return "XFAWidget";
  NOTREACHED();
  return "";
}

void AppendFlagString(const char* flag, std::string* output) {
  if (!output->empty())
    *output += ", ";
  *output += flag;
}

std::string AnnotFlagsToString(int flags) {
  std::string str;
  if (flags & FPDF_ANNOT_FLAG_INVISIBLE)
    AppendFlagString("Invisible", &str);
  if (flags & FPDF_ANNOT_FLAG_HIDDEN)
    AppendFlagString("Hidden", &str);
  if (flags & FPDF_ANNOT_FLAG_PRINT)
    AppendFlagString("Print", &str);
  if (flags & FPDF_ANNOT_FLAG_NOZOOM)
    AppendFlagString("NoZoom", &str);
  if (flags & FPDF_ANNOT_FLAG_NOROTATE)
    AppendFlagString("NoRotate", &str);
  if (flags & FPDF_ANNOT_FLAG_NOVIEW)
    AppendFlagString("NoView", &str);
  if (flags & FPDF_ANNOT_FLAG_READONLY)
    AppendFlagString("ReadOnly", &str);
  if (flags & FPDF_ANNOT_FLAG_LOCKED)
    AppendFlagString("Locked", &str);
  if (flags & FPDF_ANNOT_FLAG_TOGGLENOVIEW)
    AppendFlagString("ToggleNoView", &str);
  return str;
}

const char* PageObjectTypeToCString(int type) {
  if (type == FPDF_PAGEOBJ_TEXT)
    return "Text";
  if (type == FPDF_PAGEOBJ_PATH)
    return "Path";
  if (type == FPDF_PAGEOBJ_IMAGE)
    return "Image";
  if (type == FPDF_PAGEOBJ_SHADING)
    return "Shading";
  if (type == FPDF_PAGEOBJ_FORM)
    return "Form";
  NOTREACHED();
  return "";
}

void WriteAnnot(FPDF_PAGE page, const char* pdf_name, int num) {
  // Open the output text file.
  char filename[256];
  int chars_formatted =
      snprintf(filename, sizeof(filename), "%s.%d.annot.txt", pdf_name, num);
  if (chars_formatted < 0 ||
      static_cast<size_t>(chars_formatted) >= sizeof(filename)) {
    fprintf(stderr, "Filename %s is too long\n", filename);
    return;
  }
  FILE* fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Failed to open %s for output\n", filename);
    return;
  }

  int annot_count = FPDFPage_GetAnnotCount(page);
  fprintf(fp, "Number of annotations: %d\n\n", annot_count);

  // Iterate through all annotations on this page.
  for (int i = 0; i < annot_count; ++i) {
    // Retrieve the annotation object and its subtype.
    fprintf(fp, "Annotation #%d:\n", i + 1);
    FPDF_ANNOTATION annot = FPDFPage_GetAnnot(page, i);
    if (!annot) {
      fprintf(fp, "Failed to retrieve annotation!\n\n");
      continue;
    }
    FPDF_ANNOTATION_SUBTYPE subtype = FPDFAnnot_GetSubtype(annot);
    fprintf(fp, "Subtype: %s\n", AnnotSubtypeToCString(subtype));

    // Retrieve the annotation flags.
    fprintf(fp, "Flags set: %s\n",
            AnnotFlagsToString(FPDFAnnot_GetFlags(annot)).c_str());

    // Retrieve the annotation's object count and object types.
    const int obj_count = FPDFAnnot_GetObjectCount(annot);
    fprintf(fp, "Number of objects: %d\n", obj_count);
    if (obj_count > 0) {
      fprintf(fp, "Object types: ");
      for (int j = 0; j < obj_count; ++j) {
        const char* type = PageObjectTypeToCString(
            FPDFPageObj_GetType(FPDFAnnot_GetObject(annot, j)));
        fprintf(fp, "%s  ", type);
      }
      fprintf(fp, "\n");
    }

    // Retrieve the annotation's color and interior color.
    unsigned int R;
    unsigned int G;
    unsigned int B;
    unsigned int A;
    if (!FPDFAnnot_GetColor(annot, FPDFANNOT_COLORTYPE_Color, &R, &G, &B, &A)) {
      fprintf(fp, "Failed to retrieve color.\n");
    } else {
      fprintf(fp, "Color in RGBA: %d %d %d %d\n", R, G, B, A);
    }
    if (!FPDFAnnot_GetColor(annot, FPDFANNOT_COLORTYPE_InteriorColor, &R, &G,
                            &B, &A)) {
      fprintf(fp, "Failed to retrieve interior color.\n");
    } else {
      fprintf(fp, "Interior color in RGBA: %d %d %d %d\n", R, G, B, A);
    }

    // Retrieve the annotation's contents and author.
    static constexpr char kContentsKey[] = "Contents";
    static constexpr char kAuthorKey[] = "T";
    unsigned long len =
        FPDFAnnot_GetStringValue(annot, kContentsKey, nullptr, 0);
    std::vector<char> buf(len);
    FPDFAnnot_GetStringValue(annot, kContentsKey, buf.data(), len);
    fprintf(fp, "Content: %ls\n",
            GetPlatformWString(reinterpret_cast<unsigned short*>(buf.data()))
                .c_str());
    len = FPDFAnnot_GetStringValue(annot, kAuthorKey, nullptr, 0);
    buf.clear();
    buf.resize(len);
    FPDFAnnot_GetStringValue(annot, kAuthorKey, buf.data(), len);
    fprintf(fp, "Author: %ls\n",
            GetPlatformWString(reinterpret_cast<unsigned short*>(buf.data()))
                .c_str());

    // Retrieve the annotation's quadpoints if it is a markup annotation.
    if (FPDFAnnot_HasAttachmentPoints(annot)) {
      FS_QUADPOINTSF quadpoints;
      if (FPDFAnnot_GetAttachmentPoints(annot, &quadpoints)) {
        fprintf(fp,
                "Quadpoints: (%.3f, %.3f), (%.3f, %.3f), (%.3f, %.3f), (%.3f, "
                "%.3f)\n",
                quadpoints.x1, quadpoints.y1, quadpoints.x2, quadpoints.y2,
                quadpoints.x3, quadpoints.y3, quadpoints.x4, quadpoints.y4);
      } else {
        fprintf(fp, "Failed to retrieve quadpoints.\n");
      }
    }

    // Retrieve the annotation's rectangle coordinates.
    FS_RECTF rect;
    if (FPDFAnnot_GetRect(annot, &rect)) {
      fprintf(fp, "Rectangle: l - %.3f, b - %.3f, r - %.3f, t - %.3f\n\n",
              rect.left, rect.bottom, rect.right, rect.top);
    } else {
      fprintf(fp, "Failed to retrieve annotation rectangle.\n");
    }

    FPDFPage_CloseAnnot(annot);
  }

  (void)fclose(fp);
}

std::string WritePng(const char* pdf_name,
                     int num,
                     const void* buffer_void,
                     int stride,
                     int width,
                     int height) {
  if (!CheckDimensions(stride, width, height))
    return "";

  std::vector<unsigned char> png_encoding;
  const auto* buffer = static_cast<const unsigned char*>(buffer_void);
  if (!image_diff_png::EncodeBGRAPNG(
          buffer, width, height, stride, false, &png_encoding)) {
    fprintf(stderr, "Failed to convert bitmap to PNG\n");
    return "";
  }

  char filename[256];
  int chars_formatted = snprintf(
      filename, sizeof(filename), "%s.%d.png", pdf_name, num);
  if (chars_formatted < 0 ||
      static_cast<size_t>(chars_formatted) >= sizeof(filename)) {
    fprintf(stderr, "Filename %s is too long\n", filename);
    return "";
  }

  FILE* fp = fopen(filename, "wb");
  if (!fp) {
    fprintf(stderr, "Failed to open %s for output\n", filename);
    return "";
  }

  size_t bytes_written = fwrite(
      &png_encoding.front(), 1, png_encoding.size(), fp);
  if (bytes_written != png_encoding.size())
    fprintf(stderr, "Failed to write to %s\n", filename);

  (void)fclose(fp);
  return std::string(filename);
}

#ifdef _WIN32
std::string WriteBmp(const char* pdf_name,
                     int num,
                     const void* buffer,
                     int stride,
                     int width,
                     int height) {
  if (!CheckDimensions(stride, width, height))
    return "";

  int out_len = stride * height;
  if (out_len > INT_MAX / 3)
    return "";

  char filename[256];
  snprintf(filename, sizeof(filename), "%s.%d.bmp", pdf_name, num);
  FILE* fp = fopen(filename, "wb");
  if (!fp)
    return "";

  BITMAPINFO bmi = {};
  bmi.bmiHeader.biSize = sizeof(bmi) - sizeof(RGBQUAD);
  bmi.bmiHeader.biWidth = width;
  bmi.bmiHeader.biHeight = -height;  // top-down image
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;
  bmi.bmiHeader.biSizeImage = 0;

  BITMAPFILEHEADER file_header = {};
  file_header.bfType = 0x4d42;
  file_header.bfSize = sizeof(file_header) + bmi.bmiHeader.biSize + out_len;
  file_header.bfOffBits = file_header.bfSize - out_len;

  if (fwrite(&file_header, sizeof(file_header), 1, fp) != 1 ||
      fwrite(&bmi, bmi.bmiHeader.biSize, 1, fp) != 1 ||
      fwrite(buffer, out_len, 1, fp) != 1) {
    fprintf(stderr, "Failed to write to %s\n", filename);
  }
  fclose(fp);
  return std::string(filename);
}

void WriteEmf(FPDF_PAGE page, const char* pdf_name, int num) {
  char filename[256];
  snprintf(filename, sizeof(filename), "%s.%d.emf", pdf_name, num);

  HDC dc = CreateEnhMetaFileA(nullptr, filename, nullptr, nullptr);

  int width = static_cast<int>(FPDF_GetPageWidth(page));
  int height = static_cast<int>(FPDF_GetPageHeight(page));
  HRGN rgn = CreateRectRgn(0, 0, width, height);
  SelectClipRgn(dc, rgn);
  DeleteObject(rgn);

  SelectObject(dc, GetStockObject(NULL_PEN));
  SelectObject(dc, GetStockObject(WHITE_BRUSH));
  // If a PS_NULL pen is used, the dimensions of the rectangle are 1 pixel less.
  Rectangle(dc, 0, 0, width + 1, height + 1);

  FPDF_RenderPage(dc, page, 0, 0, width, height, 0,
                  FPDF_ANNOT | FPDF_PRINTING | FPDF_NO_CATCH);

  DeleteEnhMetaFile(CloseEnhMetaFile(dc));
}

int CALLBACK EnhMetaFileProc(HDC hdc,
                             HANDLETABLE* handle_table,
                             const ENHMETARECORD* record,
                             int objects_count,
                             LPARAM param) {
  std::vector<const ENHMETARECORD*>& items =
      *reinterpret_cast<std::vector<const ENHMETARECORD*>*>(param);
  items.push_back(record);
  return 1;
}

void WritePS(FPDF_PAGE page, const char* pdf_name, int num) {
  char filename[256];
  snprintf(filename, sizeof(filename), "%s.%d.ps", pdf_name, num);
  FILE* fp = fopen(filename, "wb");
  if (!fp)
    return;

  HDC dc = CreateEnhMetaFileA(nullptr, nullptr, nullptr, nullptr);

  int width = static_cast<int>(FPDF_GetPageWidth(page));
  int height = static_cast<int>(FPDF_GetPageHeight(page));
  FPDF_RenderPage(dc, page, 0, 0, width, height, 0,
                  FPDF_ANNOT | FPDF_PRINTING | FPDF_NO_CATCH);

  HENHMETAFILE emf = CloseEnhMetaFile(dc);
  std::vector<const ENHMETARECORD*> items;
  EnumEnhMetaFile(nullptr, emf, &EnhMetaFileProc, &items, nullptr);
  for (const ENHMETARECORD* record : items) {
    if (record->iType != EMR_GDICOMMENT)
      continue;

    const auto* comment = reinterpret_cast<const EMRGDICOMMENT*>(record);
    const char* data = reinterpret_cast<const char*>(comment->Data);
    uint16_t size = *reinterpret_cast<const uint16_t*>(data);
    if (fwrite(data + sizeof(uint16_t), size, 1, fp) != 1) {
      fprintf(stderr, "Failed to write to %s\n", filename);
      break;
    }
  }
  fclose(fp);
  DeleteEnhMetaFile(emf);
}
#endif  // _WIN32

#ifdef PDF_ENABLE_SKIA
std::string WriteSkp(const char* pdf_name,
                     int num,
                     SkPictureRecorder* recorder) {
  char filename[256];
  int chars_formatted =
      snprintf(filename, sizeof(filename), "%s.%d.skp", pdf_name, num);

  if (chars_formatted < 0 ||
      static_cast<size_t>(chars_formatted) >= sizeof(filename)) {
    fprintf(stderr, "Filename %s is too long\n", filename);
    return "";
  }

  sk_sp<SkPicture> picture(recorder->finishRecordingAsPicture());
  SkFILEWStream wStream(filename);
  picture->serialize(&wStream);
  return std::string(filename);
}
#endif

// These example JS platform callback handlers are entirely optional,
// and exist here to show the flow of information from a document back
// to the embedder.
int ExampleAppAlert(IPDF_JSPLATFORM*,
                    FPDF_WIDESTRING msg,
                    FPDF_WIDESTRING title,
                    int type,
                    int icon) {
  printf("%ls", GetPlatformWString(title).c_str());
  if (icon || type)
    printf("[icon=%d,type=%d]", icon, type);
  printf(": %ls\n", GetPlatformWString(msg).c_str());
  return 0;
}

int ExampleAppResponse(IPDF_JSPLATFORM*,
                       FPDF_WIDESTRING question,
                       FPDF_WIDESTRING title,
                       FPDF_WIDESTRING default_value,
                       FPDF_WIDESTRING label,
                       FPDF_BOOL is_password,
                       void* response,
                       int length) {
  printf("%ls: %ls, defaultValue=%ls, label=%ls, isPassword=%d, length=%d\n",
         GetPlatformWString(title).c_str(),
         GetPlatformWString(question).c_str(),
         GetPlatformWString(default_value).c_str(),
         GetPlatformWString(label).c_str(), is_password, length);

  // UTF-16, always LE regardless of platform.
  auto* ptr = static_cast<uint8_t*>(response);
  ptr[0] = 'N';
  ptr[1] = 0;
  ptr[2] = 'o';
  ptr[3] = 0;
  return 4;
}

void ExampleDocGotoPage(IPDF_JSPLATFORM*, int page_number) {
  printf("Goto Page: %d\n", page_number);
}

void ExampleDocMail(IPDF_JSPLATFORM*,
                    void* mailData,
                    int length,
                    FPDF_BOOL UI,
                    FPDF_WIDESTRING To,
                    FPDF_WIDESTRING Subject,
                    FPDF_WIDESTRING CC,
                    FPDF_WIDESTRING BCC,
                    FPDF_WIDESTRING Msg) {
  printf("Mail Msg: %d, to=%ls, cc=%ls, bcc=%ls, subject=%ls, body=%ls\n", UI,
         GetPlatformWString(To).c_str(), GetPlatformWString(CC).c_str(),
         GetPlatformWString(BCC).c_str(), GetPlatformWString(Subject).c_str(),
         GetPlatformWString(Msg).c_str());
}

void ExampleUnsupportedHandler(UNSUPPORT_INFO*, int type) {
  std::string feature = "Unknown";
  switch (type) {
    case FPDF_UNSP_DOC_XFAFORM:
      feature = "XFA";
      break;
    case FPDF_UNSP_DOC_PORTABLECOLLECTION:
      feature = "Portfolios_Packages";
      break;
    case FPDF_UNSP_DOC_ATTACHMENT:
    case FPDF_UNSP_ANNOT_ATTACHMENT:
      feature = "Attachment";
      break;
    case FPDF_UNSP_DOC_SECURITY:
      feature = "Rights_Management";
      break;
    case FPDF_UNSP_DOC_SHAREDREVIEW:
      feature = "Shared_Review";
      break;
    case FPDF_UNSP_DOC_SHAREDFORM_ACROBAT:
    case FPDF_UNSP_DOC_SHAREDFORM_FILESYSTEM:
    case FPDF_UNSP_DOC_SHAREDFORM_EMAIL:
      feature = "Shared_Form";
      break;
    case FPDF_UNSP_ANNOT_3DANNOT:
      feature = "3D";
      break;
    case FPDF_UNSP_ANNOT_MOVIE:
      feature = "Movie";
      break;
    case FPDF_UNSP_ANNOT_SOUND:
      feature = "Sound";
      break;
    case FPDF_UNSP_ANNOT_SCREEN_MEDIA:
    case FPDF_UNSP_ANNOT_SCREEN_RICHMEDIA:
      feature = "Screen";
      break;
    case FPDF_UNSP_ANNOT_SIG:
      feature = "Digital_Signature";
      break;
  }
  printf("Unsupported feature: %s.\n", feature.c_str());
}

bool ParseCommandLine(const std::vector<std::string>& args,
                      Options* options,
                      std::vector<std::string>* files) {
  if (args.empty())
    return false;

  options->exe_path = args[0];
  size_t cur_idx = 1;
  for (; cur_idx < args.size(); ++cur_idx) {
    const std::string& cur_arg = args[cur_idx];
    if (cur_arg == "--show-config") {
      options->show_config = true;
    } else if (cur_arg == "--show-metadata") {
      options->show_metadata = true;
    } else if (cur_arg == "--send-events") {
      options->send_events = true;
    } else if (cur_arg == "--render-oneshot") {
      options->render_oneshot = true;
    } else if (cur_arg == "--save-attachments") {
      options->save_attachments = true;
    } else if (cur_arg == "--save-images") {
      options->save_images = true;
#ifdef ENABLE_CALLGRIND
    } else if (cur_arg == "--callgrind-delim") {
      options->callgrind_delimiters = true;
#endif  // ENABLE_CALLGRIND
    } else if (cur_arg == "--ppm") {
      if (options->output_format != OUTPUT_NONE) {
        fprintf(stderr, "Duplicate or conflicting --ppm argument\n");
        return false;
      }
      options->output_format = OUTPUT_PPM;
    } else if (cur_arg == "--png") {
      if (options->output_format != OUTPUT_NONE) {
        fprintf(stderr, "Duplicate or conflicting --png argument\n");
        return false;
      }
      options->output_format = OUTPUT_PNG;
    } else if (cur_arg == "--txt") {
      if (options->output_format != OUTPUT_NONE) {
        fprintf(stderr, "Duplicate or conflicting --txt argument\n");
        return false;
      }
      options->output_format = OUTPUT_TEXT;
    } else if (cur_arg == "--annot") {
      if (options->output_format != OUTPUT_NONE) {
        fprintf(stderr, "Duplicate or conflicting --annot argument\n");
        return false;
      }
      options->output_format = OUTPUT_ANNOT;
#ifdef PDF_ENABLE_SKIA
    } else if (cur_arg == "--skp") {
      if (options->output_format != OUTPUT_NONE) {
        fprintf(stderr, "Duplicate or conflicting --skp argument\n");
        return false;
      }
      options->output_format = OUTPUT_SKP;
#endif
    } else if (cur_arg.size() > 11 &&
               cur_arg.compare(0, 11, "--font-dir=") == 0) {
      if (!options->font_directory.empty()) {
        fprintf(stderr, "Duplicate --font-dir argument\n");
        return false;
      }
      options->font_directory = cur_arg.substr(11);
#ifdef _WIN32
    } else if (cur_arg == "--emf") {
      if (options->output_format != OUTPUT_NONE) {
        fprintf(stderr, "Duplicate or conflicting --emf argument\n");
        return false;
      }
      options->output_format = OUTPUT_EMF;
    } else if (cur_arg == "--ps2") {
      if (options->output_format != OUTPUT_NONE) {
        fprintf(stderr, "Duplicate or conflicting --ps2 argument\n");
        return false;
      }
      options->output_format = OUTPUT_PS2;
    } else if (cur_arg == "--ps3") {
      if (options->output_format != OUTPUT_NONE) {
        fprintf(stderr, "Duplicate or conflicting --ps3 argument\n");
        return false;
      }
      options->output_format = OUTPUT_PS3;
    } else if (cur_arg == "--bmp") {
      if (options->output_format != OUTPUT_NONE) {
        fprintf(stderr, "Duplicate or conflicting --bmp argument\n");
        return false;
      }
      options->output_format = OUTPUT_BMP;
#endif  // _WIN32

#ifdef PDF_ENABLE_V8
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
    } else if (cur_arg.size() > 10 &&
               cur_arg.compare(0, 10, "--bin-dir=") == 0) {
      if (!options->bin_directory.empty()) {
        fprintf(stderr, "Duplicate --bin-dir argument\n");
        return false;
      }
      options->bin_directory = cur_arg.substr(10);
#endif  // V8_USE_EXTERNAL_STARTUP_DATA
#endif  // PDF_ENABLE_V8

    } else if (cur_arg.size() > 8 && cur_arg.compare(0, 8, "--scale=") == 0) {
      if (!options->scale_factor_as_string.empty()) {
        fprintf(stderr, "Duplicate --scale argument\n");
        return false;
      }
      options->scale_factor_as_string = cur_arg.substr(8);
    } else if (cur_arg == "--show-structure") {
      if (options->output_format != OUTPUT_NONE) {
        fprintf(stderr, "Duplicate or conflicting --show-structure argument\n");
        return false;
      }
      options->output_format = OUTPUT_STRUCTURE;
    } else if (cur_arg.size() > 8 && cur_arg.compare(0, 8, "--pages=") == 0) {
      if (options->pages) {
        fprintf(stderr, "Duplicate --pages argument\n");
        return false;
      }
      options->pages = true;
      const std::string pages_string = cur_arg.substr(8);
      size_t first_dash = pages_string.find("-");
      if (first_dash == std::string::npos) {
        std::stringstream(pages_string) >> options->first_page;
        options->last_page = options->first_page;
      } else {
        std::stringstream(pages_string.substr(0, first_dash)) >>
            options->first_page;
        std::stringstream(pages_string.substr(first_dash + 1)) >>
            options->last_page;
      }
    } else if (cur_arg == "--md5") {
      options->md5 = true;
    } else if (cur_arg.size() >= 2 && cur_arg[0] == '-' && cur_arg[1] == '-') {
      fprintf(stderr, "Unrecognized argument %s\n", cur_arg.c_str());
      return false;
    } else {
      break;
    }
  }
  for (size_t i = cur_idx; i < args.size(); i++)
    files->push_back(args[i]);

  return true;
}

void PrintLastError() {
  unsigned long err = FPDF_GetLastError();
  fprintf(stderr, "Load pdf docs unsuccessful: ");
  switch (err) {
    case FPDF_ERR_SUCCESS:
      fprintf(stderr, "Success");
      break;
    case FPDF_ERR_UNKNOWN:
      fprintf(stderr, "Unknown error");
      break;
    case FPDF_ERR_FILE:
      fprintf(stderr, "File not found or could not be opened");
      break;
    case FPDF_ERR_FORMAT:
      fprintf(stderr, "File not in PDF format or corrupted");
      break;
    case FPDF_ERR_PASSWORD:
      fprintf(stderr, "Password required or incorrect password");
      break;
    case FPDF_ERR_SECURITY:
      fprintf(stderr, "Unsupported security scheme");
      break;
    case FPDF_ERR_PAGE:
      fprintf(stderr, "Page not found or content error");
      break;
    default:
      fprintf(stderr, "Unknown error %ld", err);
  }
  fprintf(stderr, ".\n");
  return;
}

FPDF_BOOL Is_Data_Avail(FX_FILEAVAIL* avail, size_t offset, size_t size) {
  return true;
}

void Add_Segment(FX_DOWNLOADHINTS* hints, size_t offset, size_t size) {}

void SendPageEvents(FPDF_FORMHANDLE form,
                    FPDF_PAGE page,
                    const std::string& events) {
  auto lines = StringSplit(events, '\n');
  for (auto line : lines) {
    auto command = StringSplit(line, '#');
    if (command[0].empty())
      continue;
    auto tokens = StringSplit(command[0], ',');
    if (tokens[0] == "charcode") {
      if (tokens.size() == 2) {
        int keycode = atoi(tokens[1].c_str());
        FORM_OnChar(form, page, keycode, 0);
      } else {
        fprintf(stderr, "charcode: bad args\n");
      }
    } else if (tokens[0] == "keycode") {
      if (tokens.size() == 2) {
        int keycode = atoi(tokens[1].c_str());
        FORM_OnKeyDown(form, page, keycode, 0);
        FORM_OnKeyUp(form, page, keycode, 0);
      } else {
        fprintf(stderr, "keycode: bad args\n");
      }
    } else if (tokens[0] == "mousedown") {
      if (tokens.size() == 4) {
        int x = atoi(tokens[2].c_str());
        int y = atoi(tokens[3].c_str());
        if (tokens[1] == "left")
          FORM_OnLButtonDown(form, page, 0, x, y);
#ifdef PDF_ENABLE_XFA
        else if (tokens[1] == "right")
          FORM_OnRButtonDown(form, page, 0, x, y);
#endif
        else
          fprintf(stderr, "mousedown: bad button name\n");
      } else {
        fprintf(stderr, "mousedown: bad args\n");
      }
    } else if (tokens[0] == "mouseup") {
      if (tokens.size() == 4) {
        int x = atoi(tokens[2].c_str());
        int y = atoi(tokens[3].c_str());
        if (tokens[1] == "left")
          FORM_OnLButtonUp(form, page, 0, x, y);
#ifdef PDF_ENABLE_XFA
        else if (tokens[1] == "right")
          FORM_OnRButtonUp(form, page, 0, x, y);
#endif
        else
          fprintf(stderr, "mouseup: bad button name\n");
      } else {
        fprintf(stderr, "mouseup: bad args\n");
      }
    } else if (tokens[0] == "mousemove") {
      if (tokens.size() == 3) {
        int x = atoi(tokens[1].c_str());
        int y = atoi(tokens[2].c_str());
        FORM_OnMouseMove(form, page, 0, x, y);
      } else {
        fprintf(stderr, "mousemove: bad args\n");
      }
    } else if (tokens[0] == "focus") {
      if (tokens.size() == 3) {
        int x = atoi(tokens[1].c_str());
        int y = atoi(tokens[2].c_str());
        FORM_OnFocus(form, page, 0, x, y);
      } else {
        fprintf(stderr, "focus: bad args\n");
      }
    } else {
      fprintf(stderr, "Unrecognized event: %s\n", tokens[0].c_str());
    }
  }
}

FPDF_PAGE GetPageForIndex(FPDF_FORMFILLINFO* param,
                          FPDF_DOCUMENT doc,
                          int index) {
  FPDF_FORMFILLINFO_PDFiumTest* form_fill_info =
      ToPDFiumTestFormFillInfo(param);
  auto& loaded_pages = form_fill_info->loaded_pages;
  auto iter = loaded_pages.find(index);
  if (iter != loaded_pages.end())
    return iter->second.get();

  FPDF_PAGE page = FPDF_LoadPage(doc, index);
  if (!page)
    return nullptr;

  FPDF_FORMHANDLE& form_handle = form_fill_info->form_handle;
  FORM_OnAfterLoadPage(page, form_handle);
  FORM_DoPageAAction(page, form_handle, FPDFPAGE_AACTION_OPEN);
  loaded_pages[index].reset(page);
  return page;
}

std::wstring ConvertToWString(const unsigned short* buf,
                              unsigned long buf_size) {
  std::wstring result;
  result.reserve(buf_size);
  std::copy(buf, buf + buf_size, std::back_inserter(result));
  return result;
}

void DumpChildStructure(FPDF_STRUCTELEMENT child, int indent) {
  static const size_t kBufSize = 1024;
  unsigned short buf[kBufSize];
  unsigned long len = FPDF_StructElement_GetType(child, buf, kBufSize);
  printf("%*s%ls", indent * 2, "", ConvertToWString(buf, len).c_str());

  memset(buf, 0, sizeof(buf));
  len = FPDF_StructElement_GetTitle(child, buf, kBufSize);
  if (len > 0)
    printf(": '%ls'", ConvertToWString(buf, len).c_str());

  memset(buf, 0, sizeof(buf));
  len = FPDF_StructElement_GetAltText(child, buf, kBufSize);
  if (len > 0)
    printf(" (%ls)", ConvertToWString(buf, len).c_str());
  printf("\n");

  for (int i = 0; i < FPDF_StructElement_CountChildren(child); ++i) {
    FPDF_STRUCTELEMENT sub_child = FPDF_StructElement_GetChildAtIndex(child, i);
    // If the child is not an Element then this will return null. This can
    // happen if the element is things like an object reference or a stream.
    if (!sub_child)
      continue;

    DumpChildStructure(sub_child, indent + 1);
  }
}

void DumpPageStructure(FPDF_PAGE page, const int page_idx) {
  std::unique_ptr<void, FPDFStructTreeDeleter> tree(
      FPDF_StructTree_GetForPage(page));
  if (!tree) {
    fprintf(stderr, "Failed to load struct tree for page %d\n", page_idx);
    return;
  }

  printf("Structure Tree for Page %d\n", page_idx);
  for (int i = 0; i < FPDF_StructTree_CountChildren(tree.get()); ++i) {
    FPDF_STRUCTELEMENT child = FPDF_StructTree_GetChildAtIndex(tree.get(), i);
    if (!child) {
      fprintf(stderr, "Failed to load child %d for page %d\n", i, page_idx);
      continue;
    }
    DumpChildStructure(child, 0);
  }
  printf("\n\n");
}

void DumpMetaData(FPDF_DOCUMENT doc) {
  constexpr const char* meta_tags[] = {"Title",        "Author",  "Subject",
                                       "Keywords",     "Creator", "Producer",
                                       "CreationDate", "ModDate"};
  for (const char* meta_tag : meta_tags) {
    char meta_buffer[4096];
    unsigned long len =
        FPDF_GetMetaText(doc, meta_tag, meta_buffer, sizeof(meta_buffer));
    if (!len)
      continue;

    auto* meta_string = reinterpret_cast<unsigned short*>(meta_buffer);
    printf("%-12s = %ls (%lu bytes)\n", meta_tag,
           GetPlatformWString(meta_string).c_str(), len);
  }
}

void SaveAttachments(FPDF_DOCUMENT doc, const std::string& name) {
  for (int i = 0; i < FPDFDoc_GetAttachmentCount(doc); ++i) {
    FPDF_ATTACHMENT attachment = FPDFDoc_GetAttachment(doc, i);

    // Retrieve the attachment file name.
    std::string attachment_name;
    unsigned long len = FPDFAttachment_GetName(attachment, nullptr, 0);
    if (len) {
      std::vector<char> buf(len);
      unsigned long actual_len =
          FPDFAttachment_GetName(attachment, buf.data(), len);
      if (actual_len == len) {
        attachment_name =
            GetPlatformString(reinterpret_cast<unsigned short*>(buf.data()));
      }
    }
    if (attachment_name.empty()) {
      fprintf(stderr, "Attachment #%d has an empty file name.\n", i + 1);
      continue;
    }

    // Calculate the full attachment file name.
    char save_name[256];
    int chars_formatted =
        snprintf(save_name, sizeof(save_name), "%s.attachment.%s", name.c_str(),
                 attachment_name.c_str());
    if (chars_formatted < 0 ||
        static_cast<size_t>(chars_formatted) >= sizeof(save_name)) {
      fprintf(stderr, "Filename %s is too long\n", save_name);
      continue;
    }

    // Retrieve the attachment.
    len = FPDFAttachment_GetFile(attachment, nullptr, 0);
    std::vector<char> data_buf(len);
    if (len) {
      unsigned long actual_len =
          FPDFAttachment_GetFile(attachment, data_buf.data(), len);
      if (actual_len != len)
        data_buf.clear();
    }
    if (data_buf.empty()) {
      fprintf(stderr, "Attachment \"%s\" is empty.\n", attachment_name.c_str());
      continue;
    }

    // Write the attachment file.
    FILE* fp = fopen(save_name, "wb");
    if (!fp) {
      fprintf(stderr, "Failed to open %s for saving attachment.\n", save_name);
      continue;
    }

    size_t written_len = fwrite(data_buf.data(), 1, len, fp);
    if (written_len == len) {
      fprintf(stderr, "Saved attachment \"%s\" as: %s.\n",
              attachment_name.c_str(), save_name);
    } else {
      fprintf(stderr, "Failed to write to %s\n", save_name);
    }
    fclose(fp);
  }
}

void SaveImages(FPDF_PAGE page, const char* pdf_name, int page_num) {
  for (int i = 0; i < FPDFPage_CountObjects(page); ++i) {
    FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, i);
    if (FPDFPageObj_GetType(obj) != FPDF_PAGEOBJ_IMAGE)
      continue;

    std::unique_ptr<void, FPDFBitmapDeleter> bitmap(
        FPDFImageObj_GetBitmap(obj));
    if (!bitmap) {
      fprintf(stderr, "Image object #%d on page #%d has an empty bitmap.\n",
              i + 1, page_num + 1);
      continue;
    }

    int format = FPDFBitmap_GetFormat(bitmap.get());
    if (format == FPDFBitmap_Unknown) {
      fprintf(stderr,
              "Image object #%d on page #%d has a bitmap of unknown format.\n",
              i + 1, page_num + 1);
      continue;
    }

    std::vector<unsigned char> png_encoding;
    const unsigned char* buffer =
        static_cast<const unsigned char*>(FPDFBitmap_GetBuffer(bitmap.get()));
    int width = FPDFBitmap_GetWidth(bitmap.get());
    int height = FPDFBitmap_GetHeight(bitmap.get());
    int stride = FPDFBitmap_GetStride(bitmap.get());
    bool ret = false;
    switch (format) {
      case FPDFBitmap_Gray:
        ret = image_diff_png::EncodeGrayPNG(buffer, width, height, stride,
                                            &png_encoding);
        break;
      case FPDFBitmap_BGR:
        ret = image_diff_png::EncodeBGRPNG(buffer, width, height, stride,
                                           &png_encoding);
        break;
      case FPDFBitmap_BGRx:
        ret = image_diff_png::EncodeBGRAPNG(buffer, width, height, stride, true,
                                            &png_encoding);
        break;
      case FPDFBitmap_BGRA:
        ret = image_diff_png::EncodeBGRAPNG(buffer, width, height, stride,
                                            false, &png_encoding);
        break;
      default:
        NOTREACHED();
    }
    if (!ret) {
      fprintf(stderr,
              "Failed to convert image object #%d on page #%d to png.\n", i + 1,
              page_num + 1);
      continue;
    }

    char filename[256];
    int chars_formatted = snprintf(filename, sizeof(filename), "%s.%d.%d.png",
                                   pdf_name, page_num, i);
    if (chars_formatted < 0 ||
        static_cast<size_t>(chars_formatted) >= sizeof(filename)) {
      fprintf(stderr, "Filename %s for saving image is too long\n", filename);
      continue;
    }

    FILE* fp = fopen(filename, "wb");
    if (!fp) {
      fprintf(stderr, "Failed to open %s for saving image.\n", filename);
      continue;
    }

    size_t bytes_written =
        fwrite(&png_encoding.front(), 1, png_encoding.size(), fp);
    if (bytes_written != png_encoding.size())
      fprintf(stderr, "Failed to write to %s.\n", filename);
    else
      fprintf(stderr, "Successfully wrote embedded image %s.\n", filename);

    (void)fclose(fp);
  }
}

// Note, for a client using progressive rendering you'd want to determine if you
// need the rendering to pause instead of always saying |true|. This is for
// testing to force the renderer to break whenever possible.
FPDF_BOOL NeedToPauseNow(IFSDK_PAUSE* p) {
  return true;
}

bool RenderPage(const std::string& name,
                FPDF_DOCUMENT doc,
                FPDF_FORMHANDLE form,
                FPDF_FORMFILLINFO_PDFiumTest* form_fill_info,
                const int page_index,
                const Options& options,
                const std::string& events) {
  FPDF_PAGE page = GetPageForIndex(form_fill_info, doc, page_index);
  if (!page)
    return false;
  if (options.send_events)
    SendPageEvents(form, page, events);
  if (options.save_images)
    SaveImages(page, name.c_str(), page_index);
  if (options.output_format == OUTPUT_STRUCTURE) {
    DumpPageStructure(page, page_index);
    return true;
  }

  std::unique_ptr<void, FPDFTextPageDeleter> text_page(FPDFText_LoadPage(page));

  double scale = 1.0;
  if (!options.scale_factor_as_string.empty())
    std::stringstream(options.scale_factor_as_string) >> scale;

  auto width = static_cast<int>(FPDF_GetPageWidth(page) * scale);
  auto height = static_cast<int>(FPDF_GetPageHeight(page) * scale);
  int alpha = FPDFPage_HasTransparency(page) ? 1 : 0;
  std::unique_ptr<void, FPDFBitmapDeleter> bitmap(
      FPDFBitmap_Create(width, height, alpha));

  if (bitmap) {
    FPDF_DWORD fill_color = alpha ? 0x00000000 : 0xFFFFFFFF;
    FPDFBitmap_FillRect(bitmap.get(), 0, 0, width, height, fill_color);

    if (options.render_oneshot) {
      // Note, client programs probably want to use this method instead of the
      // progressive calls. The progressive calls are if you need to pause the
      // rendering to update the UI, the PDF renderer will break when possible.
      FPDF_RenderPageBitmap(bitmap.get(), page, 0, 0, width, height, 0,
                            FPDF_ANNOT);
    } else {
      IFSDK_PAUSE pause;
      pause.version = 1;
      pause.NeedToPauseNow = &NeedToPauseNow;

      int rv = FPDF_RenderPageBitmap_Start(bitmap.get(), page, 0, 0, width,
                                           height, 0, FPDF_ANNOT, &pause);
      while (rv == FPDF_RENDER_TOBECOUNTINUED)
        rv = FPDF_RenderPage_Continue(page, &pause);
    }

    FPDF_FFLDraw(form, bitmap.get(), page, 0, 0, width, height, 0, FPDF_ANNOT);

    if (!options.render_oneshot)
      FPDF_RenderPage_Close(page);

    int stride = FPDFBitmap_GetStride(bitmap.get());
    const char* buffer =
        reinterpret_cast<const char*>(FPDFBitmap_GetBuffer(bitmap.get()));

    std::string image_file_name;
    switch (options.output_format) {
#ifdef _WIN32
      case OUTPUT_BMP:
        image_file_name =
            WriteBmp(name.c_str(), page_index, buffer, stride, width, height);
        break;

      case OUTPUT_EMF:
        WriteEmf(page, name.c_str(), page_index);
        break;

      case OUTPUT_PS2:
      case OUTPUT_PS3:
        WritePS(page, name.c_str(), page_index);
        break;
#endif
      case OUTPUT_TEXT:
        WriteText(page, name.c_str(), page_index);
        break;

      case OUTPUT_ANNOT:
        WriteAnnot(page, name.c_str(), page_index);
        break;

      case OUTPUT_PNG:
        image_file_name =
            WritePng(name.c_str(), page_index, buffer, stride, width, height);
        break;

      case OUTPUT_PPM:
        image_file_name =
            WritePpm(name.c_str(), page_index, buffer, stride, width, height);
        break;

#ifdef PDF_ENABLE_SKIA
      case OUTPUT_SKP: {
        std::unique_ptr<SkPictureRecorder> recorder(
            reinterpret_cast<SkPictureRecorder*>(
                FPDF_RenderPageSkp(page, width, height)));
        FPDF_FFLRecord(form, recorder.get(), page, 0, 0, width, height, 0, 0);
        image_file_name = WriteSkp(name.c_str(), page_index, recorder.get());
      } break;
#endif
      default:
        break;
    }

    // Write the filename and the MD5 of the buffer to stdout if we wrote a
    // file.
    if (options.md5 && !image_file_name.empty())
      OutputMD5Hash(image_file_name.c_str(), buffer, stride * height);
  } else {
    fprintf(stderr, "Page was too large to be rendered.\n");
  }

  FORM_DoPageAAction(page, form, FPDFPAGE_AACTION_CLOSE);
  FORM_OnBeforeClosePage(page, form);
  return !!bitmap;
}

void RenderPdf(const std::string& name,
               const char* pBuf,
               size_t len,
               const Options& options,
               const std::string& events) {
  IPDF_JSPLATFORM platform_callbacks = {};
  platform_callbacks.version = 3;
  platform_callbacks.app_alert = ExampleAppAlert;
  platform_callbacks.app_response = ExampleAppResponse;
  platform_callbacks.Doc_gotoPage = ExampleDocGotoPage;
  platform_callbacks.Doc_mail = ExampleDocMail;

  // The pdf_avail must outlive doc.
  std::unique_ptr<void, FPDFAvailDeleter> pdf_avail;
  // The document must outlive |form_callbacks.loaded_pages|.
  std::unique_ptr<void, FPDFDocumentDeleter> doc;
  FPDF_FORMFILLINFO_PDFiumTest form_callbacks = {};
#ifdef PDF_ENABLE_XFA
  form_callbacks.version = 2;
#else   // PDF_ENABLE_XFA
  form_callbacks.version = 1;
#endif  // PDF_ENABLE_XFA
  form_callbacks.FFI_GetPage = GetPageForIndex;
  form_callbacks.m_pJsPlatform = &platform_callbacks;

  TestLoader loader(pBuf, len);
  FPDF_FILEACCESS file_access = {};
  file_access.m_FileLen = static_cast<unsigned long>(len);
  file_access.m_GetBlock = TestLoader::GetBlock;
  file_access.m_Param = &loader;

  FX_FILEAVAIL file_avail = {};
  file_avail.version = 1;
  file_avail.IsDataAvail = Is_Data_Avail;

  FX_DOWNLOADHINTS hints = {};
  hints.version = 1;
  hints.AddSegment = Add_Segment;

  int nRet = PDF_DATA_NOTAVAIL;
  bool bIsLinearized = false;
  pdf_avail.reset(FPDFAvail_Create(&file_avail, &file_access));

  if (FPDFAvail_IsLinearized(pdf_avail.get()) == PDF_LINEARIZED) {
    doc.reset(FPDFAvail_GetDocument(pdf_avail.get(), nullptr));
    if (doc) {
      while (nRet == PDF_DATA_NOTAVAIL)
        nRet = FPDFAvail_IsDocAvail(pdf_avail.get(), &hints);

      if (nRet == PDF_DATA_ERROR) {
        fprintf(stderr, "Unknown error in checking if doc was available.\n");
        return;
      }
      nRet = FPDFAvail_IsFormAvail(pdf_avail.get(), &hints);
      if (nRet == PDF_FORM_ERROR || nRet == PDF_FORM_NOTAVAIL) {
        fprintf(stderr,
                "Error %d was returned in checking if form was available.\n",
                nRet);
        return;
      }
      bIsLinearized = true;
    }
  } else {
    doc.reset(FPDF_LoadCustomDocument(&file_access, nullptr));
  }

  if (!doc) {
    PrintLastError();
    return;
  }

  (void)FPDF_GetDocPermissions(doc.get());

  if (options.show_metadata)
    DumpMetaData(doc.get());

  if (options.save_attachments)
    SaveAttachments(doc.get(), name);

  std::unique_ptr<void, FPDFFormHandleDeleter> form(
      FPDFDOC_InitFormFillEnvironment(doc.get(), &form_callbacks));
  form_callbacks.form_handle = form.get();

#ifdef PDF_ENABLE_XFA
  int doc_type = FPDF_GetFormType(doc.get());
  if (doc_type == FORMTYPE_XFA_FULL || doc_type == FORMTYPE_XFA_FOREGROUND) {
    if (!FPDF_LoadXFA(doc.get()))
      fprintf(stderr, "LoadXFA unsuccessful, continuing anyway.\n");
  }
#endif  // PDF_ENABLE_XFA

  FPDF_SetFormFieldHighlightColor(form.get(), FPDF_FORMFIELD_UNKNOWN, 0xFFE4DD);
  FPDF_SetFormFieldHighlightAlpha(form.get(), 100);
  FORM_DoDocumentJSAction(form.get());
  FORM_DoDocumentOpenAction(form.get());

#if _WIN32
  if (options.output_format == OUTPUT_PS2)
    FPDF_SetPrintMode(FPDF_PRINTMODE_POSTSCRIPT2);
  else if (options.output_format == OUTPUT_PS3)
    FPDF_SetPrintMode(FPDF_PRINTMODE_POSTSCRIPT3);
#endif

  int page_count = FPDF_GetPageCount(doc.get());
  int rendered_pages = 0;
  int bad_pages = 0;
  int first_page = options.pages ? options.first_page : 0;
  int last_page = options.pages ? options.last_page + 1 : page_count;
  for (int i = first_page; i < last_page; ++i) {
    if (bIsLinearized) {
      nRet = PDF_DATA_NOTAVAIL;
      while (nRet == PDF_DATA_NOTAVAIL)
        nRet = FPDFAvail_IsPageAvail(pdf_avail.get(), i, &hints);

      if (nRet == PDF_DATA_ERROR) {
        fprintf(stderr, "Unknown error in checking if page %d is available.\n",
                i);
        return;
      }
    }
    if (RenderPage(name, doc.get(), form.get(), &form_callbacks, i, options,
                   events)) {
      ++rendered_pages;
    } else {
      ++bad_pages;
    }
  }

  FORM_DoDocumentAAction(form.get(), FPDFDOC_AACTION_WC);
  fprintf(stderr, "Rendered %d pages.\n", rendered_pages);
  if (bad_pages)
    fprintf(stderr, "Skipped %d bad pages.\n", bad_pages);
}

void ShowConfig() {
  std::string config;
  std::string maybe_comma;
#if PDF_ENABLE_V8
  config.append(maybe_comma);
  config.append("V8");
  maybe_comma = ",";
#endif  // PDF_ENABLE_V8
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
  config.append(maybe_comma);
  config.append("V8_EXTERNAL");
  maybe_comma = ",";
#endif  // V8_USE_EXTERNAL_STARTUP_DATA
#ifdef PDF_ENABLE_XFA
  config.append(maybe_comma);
  config.append("XFA");
  maybe_comma = ",";
#endif  // PDF_ENABLE_XFA
#ifdef PDF_ENABLE_ASAN
  config.append(maybe_comma);
  config.append("ASAN");
  maybe_comma = ",";
#endif  // PDF_ENABLE_ASAN
  printf("%s\n", config.c_str());
}

constexpr char kUsageString[] =
    "Usage: pdfium_test [OPTION] [FILE]...\n"
    "  --show-config       - print build options and exit\n"
    "  --show-metadata     - print the file metadata\n"
    "  --show-structure    - print the structure elements from the document\n"
    "  --send-events       - send input described by .evt file\n"
    "  --render-oneshot    - render image without using progressive renderer\n"
    "  --save-attachments  - write embedded attachments "
    "<pdf-name>.attachment.<attachment-name>\n"
    "  --save-images       - write embedded images "
    "<pdf-name>.<page-number>.<object-number>.png\n"
#ifdef ENABLE_CALLGRIND
    "  --callgrind-delim   - delimit interesting section when using callgrind\n"
#endif  // ENABLE_CALLGRIND
    "  --bin-dir=<path>    - override path to v8 external data\n"
    "  --font-dir=<path>   - override path to external fonts\n"
    "  --scale=<number>    - scale output size by number (e.g. 0.5)\n"
    "  --pages=<number>(-<number>) - only render the given 0-based page(s)\n"
#ifdef _WIN32
    "  --bmp   - write page images <pdf-name>.<page-number>.bmp\n"
    "  --emf   - write page meta files <pdf-name>.<page-number>.emf\n"
    "  --ps2   - write page raw PostScript (Lvl 2) "
    "<pdf-name>.<page-number>.ps\n"
    "  --ps3   - write page raw PostScript (Lvl 3) "
    "<pdf-name>.<page-number>.ps\n"
#endif  // _WIN32
    "  --txt   - write page text in UTF32-LE <pdf-name>.<page-number>.txt\n"
    "  --png   - write page images <pdf-name>.<page-number>.png\n"
    "  --ppm   - write page images <pdf-name>.<page-number>.ppm\n"
    "  --annot - write annotation info <pdf-name>.<page-number>.annot.txt\n"
#ifdef PDF_ENABLE_SKIA
    "  --skp   - write page images <pdf-name>.<page-number>.skp\n"
#endif
    "  --md5   - write output image paths and their md5 hashes to stdout.\n"
    "";

}  // namespace

int main(int argc, const char* argv[]) {
  std::vector<std::string> args(argv, argv + argc);
  Options options;
  std::vector<std::string> files;
  if (!ParseCommandLine(args, &options, &files)) {
    fprintf(stderr, "%s", kUsageString);
    return 1;
  }

  if (options.show_config) {
    ShowConfig();
    return 0;
  }

  if (files.empty()) {
    fprintf(stderr, "No input files.\n");
    return 1;
  }

#ifdef PDF_ENABLE_V8
  v8::Platform* platform;
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
  v8::StartupData natives;
  v8::StartupData snapshot;
  InitializeV8ForPDFium(options.exe_path, options.bin_directory, &natives,
                        &snapshot, &platform);
#else   // V8_USE_EXTERNAL_STARTUP_DATA
  InitializeV8ForPDFium(options.exe_path, &platform);
#endif  // V8_USE_EXTERNAL_STARTUP_DATA
#endif  // PDF_ENABLE_V8

  FPDF_LIBRARY_CONFIG config;
  config.version = 2;
  config.m_pUserFontPaths = nullptr;
  config.m_pIsolate = nullptr;
  config.m_v8EmbedderSlot = 0;

  const char* path_array[2];
  if (!options.font_directory.empty()) {
    path_array[0] = options.font_directory.c_str();
    path_array[1] = nullptr;
    config.m_pUserFontPaths = path_array;
  }
  FPDF_InitLibraryWithConfig(&config);

  UNSUPPORT_INFO unsupported_info = {};
  unsupported_info.version = 1;
  unsupported_info.FSDK_UnSupport_Handler = ExampleUnsupportedHandler;

  FSDK_SetUnSpObjProcessHandler(&unsupported_info);

  for (const std::string& filename : files) {
    size_t file_length = 0;
    std::unique_ptr<char, pdfium::FreeDeleter> file_contents =
        GetFileContents(filename.c_str(), &file_length);
    if (!file_contents)
      continue;
    fprintf(stderr, "Rendering PDF file %s.\n", filename.c_str());

#ifdef ENABLE_CALLGRIND
    if (options.callgrind_delimiters)
      CALLGRIND_START_INSTRUMENTATION;
#endif  // ENABLE_CALLGRIND

    std::string events;
    if (options.send_events) {
      std::string event_filename = filename;
      size_t event_length = 0;
      size_t extension_pos = event_filename.find(".pdf");
      if (extension_pos != std::string::npos) {
        event_filename.replace(extension_pos, 4, ".evt");
        if (access(event_filename.c_str(), R_OK) == 0) {
          fprintf(stderr, "Using event file %s.\n", event_filename.c_str());
          std::unique_ptr<char, pdfium::FreeDeleter> event_contents =
              GetFileContents(event_filename.c_str(), &event_length);
          if (event_contents) {
            fprintf(stderr, "Sending events from: %s\n",
                    event_filename.c_str());
            events = std::string(event_contents.get(), event_length);
          }
        }
      }
    }
    RenderPdf(filename, file_contents.get(), file_length, options, events);

#ifdef ENABLE_CALLGRIND
    if (options.callgrind_delimiters)
      CALLGRIND_STOP_INSTRUMENTATION;
#endif  // ENABLE_CALLGRIND
  }

  FPDF_DestroyLibrary();
#ifdef PDF_ENABLE_V8
  v8::V8::ShutdownPlatform();
  delete platform;

#ifdef V8_USE_EXTERNAL_STARTUP_DATA
  free(const_cast<char*>(natives.data));
  free(const_cast<char*>(snapshot.data));
#endif  // V8_USE_EXTERNAL_STARTUP_DATA
#endif  // PDF_ENABLE_V8

  return 0;
}
