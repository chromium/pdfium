// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <list>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "../public/fpdf_dataavail.h"
#include "../public/fpdf_ext.h"
#include "../public/fpdf_formfill.h"
#include "../public/fpdf_text.h"
#include "../public/fpdfview.h"
#include "image_diff_png.h"
#include "v8/include/libplatform/libplatform.h"
#include "v8/include/v8.h"

#ifdef _WIN32
#define snprintf _snprintf
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

enum OutputFormat {
  OUTPUT_NONE,
  OUTPUT_PPM,
  OUTPUT_PNG,
#ifdef _WIN32
  OUTPUT_BMP,
  OUTPUT_EMF,
#endif
};

struct Options {
  Options() : output_format(OUTPUT_NONE) { }

  OutputFormat output_format;
  std::string scale_factor_as_string;
  std::string exe_path;
  std::string bin_directory;
};

// Reads the entire contents of a file into a newly malloc'd buffer.
static char* GetFileContents(const char* filename, size_t* retlen) {
  FILE* file = fopen(filename, "rb");
  if (!file) {
    fprintf(stderr, "Failed to open: %s\n", filename);
    return NULL;
  }
  (void) fseek(file, 0, SEEK_END);
  size_t file_length = ftell(file);
  if (!file_length) {
    return NULL;
  }
  (void) fseek(file, 0, SEEK_SET);
  char* buffer = (char*) malloc(file_length);
  if (!buffer) {
    return NULL;
  }
  size_t bytes_read = fread(buffer, 1, file_length, file);
  (void) fclose(file);
  if (bytes_read != file_length) {
    fprintf(stderr, "Failed to read: %s\n", filename);
    free(buffer);
    return NULL;
  }
  *retlen = bytes_read;
  return buffer;
}

#ifdef V8_USE_EXTERNAL_STARTUP_DATA
// Returns the full path for an external V8 data file based on either
// the currect exectuable path or an explicit override.
static std::string GetFullPathForSnapshotFile(const Options& options,
                                              const std::string& filename) {
  std::string result;
  if (!options.bin_directory.empty()) {
    result = options.bin_directory;
    if (*options.bin_directory.rbegin() != PATH_SEPARATOR) {
      result += PATH_SEPARATOR;
    }
  } else if (!options.exe_path.empty()) {
    size_t last_separator = options.exe_path.rfind(PATH_SEPARATOR);
    if (last_separator != std::string::npos)  {
      result = options.exe_path.substr(0, last_separator + 1);
    }
  }
  result += filename;
  return result;
}

// Reads an extenal V8 data file from the |options|-indicated location,
// returing true on success and false on error.
static bool GetExternalData(const Options& options,
                            const std::string& bin_filename,
                            v8::StartupData* result_data) {
  std::string full_path = GetFullPathForSnapshotFile(options, bin_filename);
  size_t data_length = 0;
  char* data_buffer = GetFileContents(full_path.c_str(), &data_length);
  if (!data_buffer) {
    return false;
  }
  result_data->data = const_cast<const char*>(data_buffer);
  result_data->raw_size = static_cast<int>(data_length);
  return true;
}
#endif  // V8_USE_EXTERNAL_STARTUP_DATA

static bool CheckDimensions(int stride, int width, int height) {
  if (stride < 0 || width < 0 || height < 0)
    return false;
  if (height > 0 && width > INT_MAX / height)
    return false;
  return true;
}

static void WritePpm(const char* pdf_name, int num, const void* buffer_void,
                     int stride, int width, int height) {
  const char* buffer = reinterpret_cast<const char*>(buffer_void);

  if (!CheckDimensions(stride, width, height))
    return;

  int out_len = width * height;
  if (out_len > INT_MAX / 3)
    return;
  out_len *= 3;

  char filename[256];
  snprintf(filename, sizeof(filename), "%s.%d.ppm", pdf_name, num);
  FILE* fp = fopen(filename, "wb");
  if (!fp)
    return;
  fprintf(fp, "P6\n# PDF test render\n%d %d\n255\n", width, height);
  // Source data is B, G, R, unused.
  // Dest data is R, G, B.
  char* result = new char[out_len];
  if (result) {
    for (int h = 0; h < height; ++h) {
      const char* src_line = buffer + (stride * h);
      char* dest_line = result + (width * h * 3);
      for (int w = 0; w < width; ++w) {
        // R
        dest_line[w * 3] = src_line[(w * 4) + 2];
        // G
        dest_line[(w * 3) + 1] = src_line[(w * 4) + 1];
        // B
        dest_line[(w * 3) + 2] = src_line[w * 4];
      }
    }
    fwrite(result, out_len, 1, fp);
    delete [] result;
  }
  fclose(fp);
}

static void WritePng(const char* pdf_name, int num, const void* buffer_void,
                     int stride, int width, int height) {
  if (!CheckDimensions(stride, width, height))
    return;

  std::vector<unsigned char> png_encoding;
  const unsigned char* buffer = static_cast<const unsigned char*>(buffer_void);
  if (!image_diff_png::EncodeBGRAPNG(
          buffer, width, height, stride, false, &png_encoding)) {
    fprintf(stderr, "Failed to convert bitmap to PNG\n");
    return;
  }

  char filename[256];
  int chars_formatted = snprintf(
      filename, sizeof(filename), "%s.%d.png", pdf_name, num);
  if (chars_formatted < 0 ||
      static_cast<size_t>(chars_formatted) >= sizeof(filename)) {
    fprintf(stderr, "Filname %s is too long\n", filename);
    return;
  }

  FILE* fp = fopen(filename, "wb");
  if (!fp) {
    fprintf(stderr, "Failed to open %s for output\n", filename);
    return;
  }

  size_t bytes_written = fwrite(
      &png_encoding.front(), 1, png_encoding.size(), fp);
  if (bytes_written != png_encoding.size())
    fprintf(stderr, "Failed to write to  %s\n", filename);

  (void) fclose(fp);
}

#ifdef _WIN32
static void WriteBmp(const char* pdf_name, int num, const void* buffer,
                     int stride, int width, int height) {
  if (stride < 0 || width < 0 || height < 0)
    return;
  if (height > 0 && width > INT_MAX / height)
    return;
  int out_len = stride * height;
  if (out_len > INT_MAX / 3)
    return;

  char filename[256];
  snprintf(filename, sizeof(filename), "%s.%d.bmp", pdf_name, num);
  FILE* fp = fopen(filename, "wb");
  if (!fp)
    return;

  BITMAPINFO bmi = {0};
  bmi.bmiHeader.biSize = sizeof(bmi) - sizeof(RGBQUAD);
  bmi.bmiHeader.biWidth = width;
  bmi.bmiHeader.biHeight = -height;  // top-down image
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;
  bmi.bmiHeader.biSizeImage = 0;

  BITMAPFILEHEADER file_header = {0};
  file_header.bfType = 0x4d42;
  file_header.bfSize = sizeof(file_header) + bmi.bmiHeader.biSize + out_len;
  file_header.bfOffBits = file_header.bfSize - out_len;

  fwrite(&file_header, sizeof(file_header), 1, fp);
  fwrite(&bmi, bmi.bmiHeader.biSize, 1, fp);
  fwrite(buffer, out_len, 1, fp);
  fclose(fp);
}

void WriteEmf(FPDF_PAGE page, const char* pdf_name, int num) {
  int width = static_cast<int>(FPDF_GetPageWidth(page));
  int height = static_cast<int>(FPDF_GetPageHeight(page));

  char filename[256];
  snprintf(filename, sizeof(filename), "%s.%d.emf", pdf_name, num);

  HDC dc = CreateEnhMetaFileA(NULL, filename, NULL, NULL);

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
#endif

int ExampleAppAlert(IPDF_JSPLATFORM*, FPDF_WIDESTRING msg, FPDF_WIDESTRING,
                    int, int) {
  // Deal with differences between UTF16LE and wchar_t on this platform.
  size_t characters = 0;
  while (msg[characters]) {
    ++characters;
  }
  wchar_t* platform_string =
      (wchar_t*)malloc((characters + 1) * sizeof(wchar_t));
  for (size_t i = 0; i < characters + 1; ++i) {
    unsigned char* ptr = (unsigned char*)&msg[i];
    platform_string[i] = ptr[0] + 256 * ptr[1];
  }
  printf("Alert: %ls\n", platform_string);
  free(platform_string);
  return 0;
}

void ExampleDocGotoPage(IPDF_JSPLATFORM*, int pageNumber) {
  printf("Goto Page: %d\n", pageNumber);
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
                      Options* options, std::list<std::string>* files) {
  if (args.empty()) {
    return false;
  }
  options->exe_path = args[0];
  size_t cur_idx = 1;
  for (; cur_idx < args.size(); ++cur_idx) {
    const std::string& cur_arg = args[cur_idx];
    if (cur_arg == "--ppm") {
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
    }
#ifdef _WIN32
    else if (cur_arg == "--emf") {
      if (options->output_format != OUTPUT_NONE) {
        fprintf(stderr, "Duplicate or conflicting --emf argument\n");
        return false;
      }
      options->output_format = OUTPUT_EMF;
    }
    else if (cur_arg == "--bmp") {
      if (options->output_format != OUTPUT_NONE) {
        fprintf(stderr, "Duplicate or conflicting --bmp argument\n");
        return false;
      }
      options->output_format = OUTPUT_BMP;
    }
#endif  // _WIN32
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
    else if (cur_arg.size() > 10 && cur_arg.compare(0, 10, "--bin-dir=") == 0) {
      if (!options->bin_directory.empty()) {
        fprintf(stderr, "Duplicate --bin-dir argument\n");
        return false;
      }
      options->bin_directory = cur_arg.substr(10);
    }
#endif  // V8_USE_EXTERNAL_STARTUP_DATA
    else if (cur_arg.size() > 8 && cur_arg.compare(0, 8, "--scale=") == 0) {
      if (!options->scale_factor_as_string.empty()) {
        fprintf(stderr, "Duplicate --scale argument\n");
        return false;
      }
      options->scale_factor_as_string = cur_arg.substr(8);
    }
    else
      break;
  }
  if (cur_idx >= args.size()) {
    fprintf(stderr, "No input files.\n");
    return false;
  }
  for (size_t i = cur_idx; i < args.size(); i++) {
    files->push_back(args[i]);
  }
  return true;
}

class TestLoader {
 public:
  TestLoader(const char* pBuf, size_t len);

  const char* m_pBuf;
  size_t m_Len;
};

TestLoader::TestLoader(const char* pBuf, size_t len)
    : m_pBuf(pBuf), m_Len(len) {
}

int Get_Block(void* param, unsigned long pos, unsigned char* pBuf,
              unsigned long size) {
  TestLoader* pLoader = (TestLoader*) param;
  if (pos + size < pos || pos + size > pLoader->m_Len) return 0;
  memcpy(pBuf, pLoader->m_pBuf + pos, size);
  return 1;
}

FPDF_BOOL Is_Data_Avail(FX_FILEAVAIL* pThis, size_t offset, size_t size) {
  return true;
}

void Add_Segment(FX_DOWNLOADHINTS* pThis, size_t offset, size_t size) {
}

void RenderPdf(const std::string& name, const char* pBuf, size_t len,
               const Options& options) {
  fprintf(stderr, "Rendering PDF file %s.\n", name.c_str());

  IPDF_JSPLATFORM platform_callbacks;
  memset(&platform_callbacks, '\0', sizeof(platform_callbacks));
  platform_callbacks.version = 1;
  platform_callbacks.app_alert = ExampleAppAlert;
  platform_callbacks.Doc_gotoPage = ExampleDocGotoPage;

  FPDF_FORMFILLINFO form_callbacks;
  memset(&form_callbacks, '\0', sizeof(form_callbacks));
  form_callbacks.version = 1;
  form_callbacks.m_pJsPlatform = &platform_callbacks;

  TestLoader loader(pBuf, len);

  FPDF_FILEACCESS file_access;
  memset(&file_access, '\0', sizeof(file_access));
  file_access.m_FileLen = static_cast<unsigned long>(len);
  file_access.m_GetBlock = Get_Block;
  file_access.m_Param = &loader;

  FX_FILEAVAIL file_avail;
  memset(&file_avail, '\0', sizeof(file_avail));
  file_avail.version = 1;
  file_avail.IsDataAvail = Is_Data_Avail;

  FX_DOWNLOADHINTS hints;
  memset(&hints, '\0', sizeof(hints));
  hints.version = 1;
  hints.AddSegment = Add_Segment;

  FPDF_DOCUMENT doc;
  FPDF_AVAIL pdf_avail = FPDFAvail_Create(&file_avail, &file_access);

  (void) FPDFAvail_IsDocAvail(pdf_avail, &hints);

  if (!FPDFAvail_IsLinearized(pdf_avail)) {
    fprintf(stderr, "Non-linearized path...\n");
    doc = FPDF_LoadCustomDocument(&file_access, NULL);
  } else {
    fprintf(stderr, "Linearized path...\n");
    doc = FPDFAvail_GetDocument(pdf_avail, NULL);
  }

  (void) FPDF_GetDocPermissions(doc);
  (void) FPDFAvail_IsFormAvail(pdf_avail, &hints);

  FPDF_FORMHANDLE form = FPDFDOC_InitFormFillEnvironment(doc, &form_callbacks);
  FPDF_SetFormFieldHighlightColor(form, 0, 0xFFE4DD);
  FPDF_SetFormFieldHighlightAlpha(form, 100);

  int first_page = FPDFAvail_GetFirstPageNum(doc);
  (void) FPDFAvail_IsPageAvail(pdf_avail, first_page, &hints);

  int page_count = FPDF_GetPageCount(doc);
  for (int i = 0; i < page_count; ++i) {
    (void) FPDFAvail_IsPageAvail(pdf_avail, i, &hints);
  }

  FORM_DoDocumentJSAction(form);
  FORM_DoDocumentOpenAction(form);

  int rendered_pages = 0;
  int bad_pages = 0;
  for (int i = 0; i < page_count; ++i) {
    FPDF_PAGE page = FPDF_LoadPage(doc, i);
    if (!page) {
        bad_pages ++;
        continue;
    }
    FPDF_TEXTPAGE text_page = FPDFText_LoadPage(page);
    FORM_OnAfterLoadPage(page, form);
    FORM_DoPageAAction(page, form, FPDFPAGE_AACTION_OPEN);

    double scale = 1.0;
    if (!options.scale_factor_as_string.empty()) {
      std::stringstream(options.scale_factor_as_string) >> scale;
    }
    int width = static_cast<int>(FPDF_GetPageWidth(page) * scale);
    int height = static_cast<int>(FPDF_GetPageHeight(page) * scale);

    FPDF_BITMAP bitmap = FPDFBitmap_Create(width, height, 0);
    if (!bitmap) {
      fprintf(stderr, "Page was too large to be rendered.\n");
      bad_pages++;
      continue;
    }

    FPDFBitmap_FillRect(bitmap, 0, 0, width, height, 0xFFFFFFFF);
    FPDF_RenderPageBitmap(bitmap, page, 0, 0, width, height, 0, 0);
    rendered_pages ++;

    FPDF_FFLDraw(form, bitmap, page, 0, 0, width, height, 0, 0);
    int stride = FPDFBitmap_GetStride(bitmap);
    const char* buffer =
        reinterpret_cast<const char*>(FPDFBitmap_GetBuffer(bitmap));

    switch (options.output_format) {
#ifdef _WIN32
      case OUTPUT_BMP:
        WriteBmp(name.c_str(), i, buffer, stride, width, height);
        break;

      case OUTPUT_EMF:
        WriteEmf(page, name.c_str(), i);
        break;
#endif
      case OUTPUT_PNG:
        WritePng(name.c_str(), i, buffer, stride, width, height);
        break;

      case OUTPUT_PPM:
        WritePpm(name.c_str(), i, buffer, stride, width, height);
        break;

      default:
        break;
    }

    FPDFBitmap_Destroy(bitmap);

    FORM_DoPageAAction(page, form, FPDFPAGE_AACTION_CLOSE);
    FORM_OnBeforeClosePage(page, form);
    FPDFText_ClosePage(text_page);
    FPDF_ClosePage(page);
  }

  FORM_DoDocumentAAction(form, FPDFDOC_AACTION_WC);
  FPDF_CloseDocument(doc);
  FPDFDOC_ExitFormFillEnvironment(form);
  FPDFAvail_Destroy(pdf_avail);

  fprintf(stderr, "Rendered %d pages.\n", rendered_pages);
  fprintf(stderr, "Skipped %d bad pages.\n", bad_pages);
}

static const char usage_string[] =
    "Usage: pdfium_test [OPTION] [FILE]...\n"
    "  --bin-dir=<path> - override path to v8 external data\n"
    "  --scale=<number> - scale output size by number (e.g. 0.5)\n"
#ifdef _WIN32
    "  --bmp - write page images <pdf-name>.<page-number>.bmp\n"
    "  --emf - write page meta files <pdf-name>.<page-number>.emf\n"
#endif
    "  --png - write page images <pdf-name>.<page-number>.png\n"
    "  --ppm - write page images <pdf-name>.<page-number>.ppm\n";

int main(int argc, const char* argv[]) {
  std::vector<std::string> args(argv, argv + argc);
  Options options;
  std::list<std::string> files;
  if (!ParseCommandLine(args, &options, &files)) {
    fprintf(stderr, "%s", usage_string);
    return 1;
  }

  v8::V8::InitializeICU();
  v8::Platform* platform = v8::platform::CreateDefaultPlatform();
  v8::V8::InitializePlatform(platform);
  v8::V8::Initialize();

  // By enabling predictable mode, V8 won't post any background tasks.
  static const char predictable_flag[] = "--predictable";
  v8::V8::SetFlagsFromString(predictable_flag,
                             static_cast<int>(strlen(predictable_flag)));

#ifdef V8_USE_EXTERNAL_STARTUP_DATA
  v8::StartupData natives;
  v8::StartupData snapshot;
  if (!GetExternalData(options, "natives_blob.bin", &natives) ||
      !GetExternalData(options, "snapshot_blob.bin", &snapshot)) {
    return 1;
  }
  v8::V8::SetNativesDataBlob(&natives);
  v8::V8::SetSnapshotDataBlob(&snapshot);
#endif  // V8_USE_EXTERNAL_STARTUP_DATA

  FPDF_InitLibrary();

  UNSUPPORT_INFO unsuppored_info;
  memset(&unsuppored_info, '\0', sizeof(unsuppored_info));
  unsuppored_info.version = 1;
  unsuppored_info.FSDK_UnSupport_Handler = ExampleUnsupportedHandler;

  FSDK_SetUnSpObjProcessHandler(&unsuppored_info);

  while (!files.empty()) {
    std::string filename = files.front();
    files.pop_front();
    size_t file_length = 0;
    char* file_contents = GetFileContents(filename.c_str(), &file_length);
    if (!file_contents)
      continue;
    RenderPdf(filename, file_contents, file_length, options);
    free(file_contents);
  }

  FPDF_DestroyLibrary();
  v8::V8::ShutdownPlatform();
  delete platform;

  return 0;
}
