// Copyright 2018 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "samples/pdfium_test_write_helper.h"

#include <limits.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "public/cpp/fpdf_scopers.h"
#include "public/fpdf_annot.h"
#include "public/fpdf_attachment.h"
#include "public/fpdf_edit.h"
#include "public/fpdf_thumbnail.h"
#include "testing/fx_string_testhelpers.h"
#include "testing/image_diff/image_diff_png.h"
#include "third_party/base/logging.h"

namespace {

bool CheckDimensions(int stride, int width, int height) {
  if (stride < 0 || width < 0 || height < 0)
    return false;
  if (height > 0 && stride > INT_MAX / height)
    return false;
  return true;
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

std::vector<uint8_t> EncodePng(pdfium::span<const uint8_t> input,
                               int width,
                               int height,
                               int stride,
                               int format) {
  std::vector<uint8_t> png;
  switch (format) {
    case FPDFBitmap_Unknown:
      break;
    case FPDFBitmap_Gray:
      png = image_diff_png::EncodeGrayPNG(input, width, height, stride);
      break;
    case FPDFBitmap_BGR:
      png = image_diff_png::EncodeBGRPNG(input, width, height, stride);
      break;
    case FPDFBitmap_BGRx:
      png = image_diff_png::EncodeBGRAPNG(input, width, height, stride,
                                          /*discard_transparency=*/true);
      break;
    case FPDFBitmap_BGRA:
      png = image_diff_png::EncodeBGRAPNG(input, width, height, stride,
                                          /*discard_transparency=*/false);
      break;
    default:
      NOTREACHED();
  }
  return png;
}

#ifdef _WIN32
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
#endif  // _WIN32

}  // namespace

std::string WritePpm(const char* pdf_name,
                     int num,
                     void* buffer_void,
                     int stride,
                     int width,
                     int height) {
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
  const uint8_t* buffer = reinterpret_cast<const uint8_t*>(buffer_void);
  std::vector<uint8_t> result(out_len);
  for (int h = 0; h < height; ++h) {
    const uint8_t* src_line = buffer + (stride * h);
    uint8_t* dest_line = result.data() + (width * h * 3);
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

  ScopedFPDFTextPage textpage(FPDFText_LoadPage(page));
  for (int i = 0; i < FPDFText_CountChars(textpage.get()); i++) {
    uint32_t c = FPDFText_GetUnicode(textpage.get(), i);
    if (fwrite(&c, sizeof(c), 1, fp) != 1) {
      fprintf(stderr, "Failed to write to %s\n", filename);
      break;
    }
  }
  (void)fclose(fp);
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
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, i));
    if (!annot) {
      fprintf(fp, "Failed to retrieve annotation!\n\n");
      continue;
    }

    FPDF_ANNOTATION_SUBTYPE subtype = FPDFAnnot_GetSubtype(annot.get());
    fprintf(fp, "Subtype: %s\n", AnnotSubtypeToCString(subtype));

    // Retrieve the annotation flags.
    fprintf(fp, "Flags set: %s\n",
            AnnotFlagsToString(FPDFAnnot_GetFlags(annot.get())).c_str());

    // Retrieve the annotation's object count and object types.
    const int obj_count = FPDFAnnot_GetObjectCount(annot.get());
    fprintf(fp, "Number of objects: %d\n", obj_count);
    if (obj_count > 0) {
      fprintf(fp, "Object types: ");
      for (int j = 0; j < obj_count; ++j) {
        const char* type = PageObjectTypeToCString(
            FPDFPageObj_GetType(FPDFAnnot_GetObject(annot.get(), j)));
        fprintf(fp, "%s  ", type);
      }
      fprintf(fp, "\n");
    }

    // Retrieve the annotation's color and interior color.
    unsigned int R;
    unsigned int G;
    unsigned int B;
    unsigned int A;
    if (FPDFAnnot_GetColor(annot.get(), FPDFANNOT_COLORTYPE_Color, &R, &G, &B,
                           &A)) {
      fprintf(fp, "Color in RGBA: %d %d %d %d\n", R, G, B, A);
    } else {
      fprintf(fp, "Failed to retrieve color.\n");
    }
    if (FPDFAnnot_GetColor(annot.get(), FPDFANNOT_COLORTYPE_InteriorColor, &R,
                           &G, &B, &A)) {
      fprintf(fp, "Interior color in RGBA: %d %d %d %d\n", R, G, B, A);
    } else {
      fprintf(fp, "Failed to retrieve interior color.\n");
    }

    // Retrieve the annotation's contents and author.
    static constexpr char kContentsKey[] = "Contents";
    static constexpr char kAuthorKey[] = "T";
    unsigned long length_bytes =
        FPDFAnnot_GetStringValue(annot.get(), kContentsKey, nullptr, 0);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    FPDFAnnot_GetStringValue(annot.get(), kContentsKey, buf.data(),
                             length_bytes);
    fprintf(fp, "Content: %ls\n", GetPlatformWString(buf.data()).c_str());
    length_bytes =
        FPDFAnnot_GetStringValue(annot.get(), kAuthorKey, nullptr, 0);
    buf = GetFPDFWideStringBuffer(length_bytes);
    FPDFAnnot_GetStringValue(annot.get(), kAuthorKey, buf.data(), length_bytes);
    fprintf(fp, "Author: %ls\n", GetPlatformWString(buf.data()).c_str());

    // Retrieve the annotation's quadpoints if it is a markup annotation.
    if (FPDFAnnot_HasAttachmentPoints(annot.get())) {
      size_t qp_count = FPDFAnnot_CountAttachmentPoints(annot.get());
      fprintf(fp, "Number of quadpoints sets: %zu\n", qp_count);

      // Iterate through all quadpoints of the current annotation
      for (size_t j = 0; j < qp_count; ++j) {
        FS_QUADPOINTSF quadpoints;
        if (FPDFAnnot_GetAttachmentPoints(annot.get(), j, &quadpoints)) {
          fprintf(fp,
                  "Quadpoints set #%zu: (%.3f, %.3f), (%.3f, %.3f), "
                  "(%.3f, %.3f), (%.3f, %.3f)\n",
                  j + 1, quadpoints.x1, quadpoints.y1, quadpoints.x2,
                  quadpoints.y2, quadpoints.x3, quadpoints.y3, quadpoints.x4,
                  quadpoints.y4);
        } else {
          fprintf(fp, "Failed to retrieve quadpoints set #%zu.\n", j + 1);
        }
      }
    }

    // Retrieve the annotation's rectangle coordinates.
    FS_RECTF rect;
    if (FPDFAnnot_GetRect(annot.get(), &rect)) {
      fprintf(fp, "Rectangle: l - %.3f, b - %.3f, r - %.3f, t - %.3f\n\n",
              rect.left, rect.bottom, rect.right, rect.top);
    } else {
      fprintf(fp, "Failed to retrieve annotation rectangle.\n");
    }
  }

  (void)fclose(fp);
}

std::string WritePng(const char* pdf_name,
                     int num,
                     void* buffer,
                     int stride,
                     int width,
                     int height) {
  if (!CheckDimensions(stride, width, height))
    return "";

  auto input =
      pdfium::make_span(static_cast<uint8_t*>(buffer), stride * height);
  std::vector<uint8_t> png_encoding =
      EncodePng(input, width, height, stride, FPDFBitmap_BGRA);
  if (png_encoding.empty()) {
    fprintf(stderr, "Failed to convert bitmap to PNG\n");
    return "";
  }

  char filename[256];
  int chars_formatted =
      snprintf(filename, sizeof(filename), "%s.%d.png", pdf_name, num);
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

  size_t bytes_written =
      fwrite(&png_encoding.front(), 1, png_encoding.size(), fp);
  if (bytes_written != png_encoding.size())
    fprintf(stderr, "Failed to write to %s\n", filename);

  (void)fclose(fp);
  return std::string(filename);
}

#ifdef _WIN32
std::string WriteBmp(const char* pdf_name,
                     int num,
                     void* buffer,
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

  int width = static_cast<int>(FPDF_GetPageWidthF(page));
  int height = static_cast<int>(FPDF_GetPageHeightF(page));
  HRGN rgn = CreateRectRgn(0, 0, width, height);
  SelectClipRgn(dc, rgn);
  DeleteObject(rgn);

  SelectObject(dc, GetStockObject(NULL_PEN));
  SelectObject(dc, GetStockObject(WHITE_BRUSH));
  // If a PS_NULL pen is used, the dimensions of the rectangle are 1 pixel less.
  Rectangle(dc, 0, 0, width + 1, height + 1);

  FPDF_RenderPage(dc, page, 0, 0, width, height, 0, FPDF_ANNOT | FPDF_PRINTING);

  DeleteEnhMetaFile(CloseEnhMetaFile(dc));
}

void WritePS(FPDF_PAGE page, const char* pdf_name, int num) {
  char filename[256];
  snprintf(filename, sizeof(filename), "%s.%d.ps", pdf_name, num);
  FILE* fp = fopen(filename, "wb");
  if (!fp)
    return;

  HDC dc = CreateEnhMetaFileA(nullptr, nullptr, nullptr, nullptr);

  int width = static_cast<int>(FPDF_GetPageWidthF(page));
  int height = static_cast<int>(FPDF_GetPageHeightF(page));
  FPDF_RenderPage(dc, page, 0, 0, width, height, 0, FPDF_ANNOT | FPDF_PRINTING);

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

enum class ThumbnailDecodeType { kBitmap, kRawStream, kDecodedStream };

bool GetThumbnailFilename(char* name_buf,
                          size_t name_buf_size,
                          const char* pdf_name,
                          int page_num,
                          ThumbnailDecodeType decode_type) {
  const char* format;
  switch (decode_type) {
    case ThumbnailDecodeType::kBitmap:
      format = "%s.thumbnail.%d.png";
      break;
    case ThumbnailDecodeType::kDecodedStream:
      format = "%s.thumbnail.decoded.%d.bin";
      break;
    case ThumbnailDecodeType::kRawStream:
      format = "%s.thumbnail.raw.%d.bin";
      break;
  }

  int chars_formatted =
      snprintf(name_buf, name_buf_size, format, pdf_name, page_num);
  if (chars_formatted < 0 ||
      static_cast<size_t>(chars_formatted) >= name_buf_size) {
    fprintf(stderr, "Filename %s for saving is too long.\n", name_buf);
    return false;
  }

  return true;
}

void WriteBufferToFile(const void* buf,
                       size_t buflen,
                       const char* filename,
                       const char* filetype) {
  FILE* fp = fopen(filename, "wb");
  if (!fp) {
    fprintf(stderr, "Failed to open %s for saving %s.", filename, filetype);
    return;
  }

  size_t bytes_written = fwrite(buf, 1, buflen, fp);
  if (bytes_written == buflen)
    fprintf(stderr, "Successfully wrote %s %s.\n", filetype, filename);
  else
    fprintf(stderr, "Failed to write to %s.\n", filename);
  fclose(fp);
}

std::vector<uint8_t> EncodeBitmapToPng(ScopedFPDFBitmap bitmap) {
  std::vector<uint8_t> png_encoding;
  int format = FPDFBitmap_GetFormat(bitmap.get());
  if (format == FPDFBitmap_Unknown)
    return png_encoding;

  int width = FPDFBitmap_GetWidth(bitmap.get());
  int height = FPDFBitmap_GetHeight(bitmap.get());
  int stride = FPDFBitmap_GetStride(bitmap.get());
  if (!CheckDimensions(stride, width, height))
    return png_encoding;

  auto input = pdfium::make_span(
      static_cast<const uint8_t*>(FPDFBitmap_GetBuffer(bitmap.get())),
      stride * height);

  png_encoding = EncodePng(input, width, height, stride, format);
  return png_encoding;
}

void WriteAttachments(FPDF_DOCUMENT doc, const std::string& name) {
  for (int i = 0; i < FPDFDoc_GetAttachmentCount(doc); ++i) {
    FPDF_ATTACHMENT attachment = FPDFDoc_GetAttachment(doc, i);

    // Retrieve the attachment file name.
    std::string attachment_name;
    unsigned long length_bytes = FPDFAttachment_GetName(attachment, nullptr, 0);
    if (length_bytes) {
      std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
      unsigned long actual_length_bytes =
          FPDFAttachment_GetName(attachment, buf.data(), length_bytes);
      if (actual_length_bytes == length_bytes)
        attachment_name = GetPlatformString(buf.data());
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
      fprintf(stderr, "Filename %s is too long.\n", save_name);
      continue;
    }

    // Retrieve the attachment.
    length_bytes = FPDFAttachment_GetFile(attachment, nullptr, 0);
    std::vector<char> data_buf(length_bytes);
    if (length_bytes) {
      unsigned long actual_length_bytes =
          FPDFAttachment_GetFile(attachment, data_buf.data(), length_bytes);
      if (actual_length_bytes != length_bytes)
        data_buf.clear();
    }
    if (data_buf.empty()) {
      fprintf(stderr, "Attachment \"%s\" is empty.\n", attachment_name.c_str());
      continue;
    }

    // Write the attachment file.
    WriteBufferToFile(data_buf.data(), length_bytes, save_name, "attachment");
  }
}

void WriteImages(FPDF_PAGE page, const char* pdf_name, int page_num) {
  for (int i = 0; i < FPDFPage_CountObjects(page); ++i) {
    FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, i);
    if (FPDFPageObj_GetType(obj) != FPDF_PAGEOBJ_IMAGE)
      continue;

    ScopedFPDFBitmap bitmap(FPDFImageObj_GetBitmap(obj));
    if (!bitmap) {
      fprintf(stderr, "Image object #%d on page #%d has an empty bitmap.\n",
              i + 1, page_num + 1);
      continue;
    }

    char filename[256];
    int chars_formatted = snprintf(filename, sizeof(filename), "%s.%d.%d.png",
                                   pdf_name, page_num, i);
    if (chars_formatted < 0 ||
        static_cast<size_t>(chars_formatted) >= sizeof(filename)) {
      fprintf(stderr, "Filename %s for saving image is too long.\n", filename);
      continue;
    }

    std::vector<uint8_t> png_encoding = EncodeBitmapToPng(std::move(bitmap));
    if (png_encoding.empty()) {
      fprintf(stderr,
              "Failed to convert image object #%d, on page #%d to png.\n",
              i + 1, page_num + 1);
      continue;
    }

    WriteBufferToFile(&png_encoding.front(), png_encoding.size(), filename,
                      "image");
  }
}

void WriteDecodedThumbnailStream(FPDF_PAGE page,
                                 const char* pdf_name,
                                 int page_num) {
  char filename[256];
  if (!GetThumbnailFilename(filename, sizeof(filename), pdf_name, page_num,
                            ThumbnailDecodeType::kDecodedStream)) {
    return;
  }

  unsigned long decoded_data_size =
      FPDFPage_GetDecodedThumbnailData(page, nullptr, 0u);

  // Only continue if there actually is a thumbnail for this page
  if (decoded_data_size == 0) {
    fprintf(stderr, "Failed to get decoded thumbnail for page #%d.\n",
            page_num + 1);
    return;
  }

  std::vector<uint8_t> thumb_buf(decoded_data_size);
  if (FPDFPage_GetDecodedThumbnailData(
          page, thumb_buf.data(), decoded_data_size) != decoded_data_size) {
    fprintf(stderr, "Failed to get decoded thumbnail data for %s.\n", filename);
    return;
  }

  WriteBufferToFile(thumb_buf.data(), decoded_data_size, filename,
                    "decoded thumbnail");
}

void WriteRawThumbnailStream(FPDF_PAGE page,
                             const char* pdf_name,
                             int page_num) {
  char filename[256];
  if (!GetThumbnailFilename(filename, sizeof(filename), pdf_name, page_num,
                            ThumbnailDecodeType::kRawStream)) {
    return;
  }

  unsigned long raw_data_size = FPDFPage_GetRawThumbnailData(page, nullptr, 0u);

  // Only continue if there actually is a thumbnail for this page
  if (raw_data_size == 0) {
    fprintf(stderr, "Failed to get raw thumbnail data for page #%d.\n",
            page_num + 1);
    return;
  }

  std::vector<uint8_t> thumb_buf(raw_data_size);
  if (FPDFPage_GetRawThumbnailData(page, thumb_buf.data(), raw_data_size) !=
      raw_data_size) {
    fprintf(stderr, "Failed to get raw thumbnail data for %s.\n", filename);
    return;
  }

  WriteBufferToFile(thumb_buf.data(), raw_data_size, filename, "raw thumbnail");
}

void WriteThumbnail(FPDF_PAGE page, const char* pdf_name, int page_num) {
  char filename[256];
  if (!GetThumbnailFilename(filename, sizeof(filename), pdf_name, page_num,
                            ThumbnailDecodeType::kBitmap)) {
    return;
  }

  ScopedFPDFBitmap thumb_bitmap(FPDFPage_GetThumbnailAsBitmap(page));
  if (!thumb_bitmap) {
    fprintf(stderr, "Thumbnail of page #%d has an empty bitmap.\n",
            page_num + 1);
    return;
  }

  std::vector<uint8_t> png_encoding =
      EncodeBitmapToPng(std::move(thumb_bitmap));
  if (png_encoding.empty()) {
    fprintf(stderr, "Failed to convert thumbnail of page #%d to png.\n",
            page_num + 1);
    return;
  }

  WriteBufferToFile(&png_encoding.front(), png_encoding.size(), filename,
                    "thumbnail");
}
