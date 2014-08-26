// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <list>
#include <string>
#include <utility>

#include "../fpdfsdk/include/fpdf_dataavail.h"
#include "../fpdfsdk/include/fpdf_ext.h"
#include "../fpdfsdk/include/fpdfformfill.h"
#include "../fpdfsdk/include/fpdftext.h"
#include "../fpdfsdk/include/fpdfview.h"
#include "v8/include/v8.h"

#ifdef _WIN32
  #define snprintf _snprintf
#endif

enum OutputFormat {
  OUTPUT_NONE,
  OUTPUT_PPM,
#ifdef _WIN32
  OUTPUT_BMP,
  OUTPUT_EMF,
#endif
};

static void WritePpm(const char* pdf_name, int num, const void* buffer_void,
                     int stride, int width, int height) {
  const char* buffer = reinterpret_cast<const char*>(buffer_void);

  if (stride < 0 || width < 0 || height < 0)
    return;
  if (height > 0 && width > INT_MAX / height)
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

int Form_Alert(IPDF_JSPLATFORM*, FPDF_WIDESTRING, FPDF_WIDESTRING, int, int) {
  printf("Form_Alert called.\n");
  return 0;
}

void Unsupported_Handler(UNSUPPORT_INFO*, int type) {
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

bool ParseCommandLine(int argc, const char* argv[], OutputFormat* output_format,
                      std::list<const char*>* files) {
  *output_format = OUTPUT_NONE;
  files->clear();

  int cur_arg = 1;
  for (; cur_arg < argc; ++cur_arg) {
    if (strcmp(argv[cur_arg], "--ppm") == 0)
      *output_format = OUTPUT_PPM;
#ifdef _WIN32
    else if (strcmp(argv[cur_arg], "--emf") == 0)
      *output_format = OUTPUT_EMF;
    else if (strcmp(argv[cur_arg], "--bmp") == 0)
      *output_format = OUTPUT_BMP;
#endif
    else
      break;
  }

  if (cur_arg > 2)  // Multiple options.
    return false;

  if (cur_arg >= argc)  // No input files.
    return false;

  for (int i = cur_arg; i < argc; i++)
    files->push_back(argv[i]);

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

bool Is_Data_Avail(FX_FILEAVAIL* pThis, size_t offset, size_t size) {
  return true;
}

void Add_Segment(FX_DOWNLOADHINTS* pThis, size_t offset, size_t size) {
}

void RenderPdf(const char* name, const char* pBuf, size_t len,
               OutputFormat format) {
  printf("Rendering PDF file %s.\n", name);

  IPDF_JSPLATFORM platform_callbacks;
  memset(&platform_callbacks, '\0', sizeof(platform_callbacks));
  platform_callbacks.version = 1;
  platform_callbacks.app_alert = Form_Alert;

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
    printf("Non-linearized path...\n");
    doc = FPDF_LoadCustomDocument(&file_access, NULL);
  } else {
    printf("Linearized path...\n");
    doc = FPDFAvail_GetDocument(pdf_avail, NULL);
  }

  (void) FPDF_GetDocPermissions(doc);
  (void) FPDFAvail_IsFormAvail(pdf_avail, &hints);

  FPDF_FORMHANDLE form = FPDFDOC_InitFormFillEnviroument(doc, &form_callbacks);
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

  size_t rendered_pages = 0;
  size_t bad_pages = 0;
  for (int i = 0; i < page_count; ++i) {
    FPDF_PAGE page = FPDF_LoadPage(doc, i);
    if (!page) {
        bad_pages ++;
        continue;
    }
    FPDF_TEXTPAGE text_page = FPDFText_LoadPage(page);
    FORM_OnAfterLoadPage(page, form);
    FORM_DoPageAAction(page, form, FPDFPAGE_AACTION_OPEN);

    int width = static_cast<int>(FPDF_GetPageWidth(page));
    int height = static_cast<int>(FPDF_GetPageHeight(page));
    FPDF_BITMAP bitmap = FPDFBitmap_Create(width, height, 0);
    FPDFBitmap_FillRect(bitmap, 0, 0, width, height, 0xFFFFFFFF);

    FPDF_RenderPageBitmap(bitmap, page, 0, 0, width, height, 0, 0);
    rendered_pages ++;

    FPDF_FFLDraw(form, bitmap, page, 0, 0, width, height, 0, 0);
    int stride = FPDFBitmap_GetStride(bitmap);
    const char* buffer =
        reinterpret_cast<const char*>(FPDFBitmap_GetBuffer(bitmap));

    switch (format) {
#ifdef _WIN32
      case OUTPUT_BMP:
        WriteBmp(name, i, buffer, stride, width, height);
        break;

      case OUTPUT_EMF:
        WriteEmf(page, name, i);
        break;
#endif
      case OUTPUT_PPM:
        WritePpm(name, i, buffer, stride, width, height);
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
  FPDFDOC_ExitFormFillEnviroument(form);
  FPDF_CloseDocument(doc);
  FPDFAvail_Destroy(pdf_avail);

  printf("Loaded, parsed and rendered %zu pages.\n", rendered_pages);
  printf("Skipped %zu bad pages.\n", bad_pages);
}

int main(int argc, const char* argv[]) {
  v8::V8::InitializeICU();
  OutputFormat format = OUTPUT_NONE;
  std::list<const char*> files;
  if (!ParseCommandLine(argc, argv, &format, &files)) {
    printf("Usage: pdfium_test [OPTION] [FILE]...\n");
    printf("--ppm    write page images <pdf-name>.<page-number>.ppm\n");
#ifdef _WIN32
    printf("--bmp    write page images <pdf-name>.<page-number>.bmp\n");
    printf("--emf    write page meta files <pdf-name>.<page-number>.emf\n");
#endif
    return 1;
  }

  FPDF_InitLibrary(NULL);

  UNSUPPORT_INFO unsuppored_info;
  memset(&unsuppored_info, '\0', sizeof(unsuppored_info));
  unsuppored_info.version = 1;
  unsuppored_info.FSDK_UnSupport_Handler = Unsupported_Handler;

  FSDK_SetUnSpObjProcessHandler(&unsuppored_info);

  while (!files.empty()) {
    const char* filename = files.front();
    files.pop_front();
    FILE* file = fopen(filename, "rb");
    if (!file) {
      fprintf(stderr, "Failed to open: %s\n", filename);
      continue;
    }
    (void) fseek(file, 0, SEEK_END);
    size_t len = ftell(file);
    (void) fseek(file, 0, SEEK_SET);
    char* pBuf = (char*) malloc(len);
    size_t ret = fread(pBuf, 1, len, file);
    (void) fclose(file);
    if (ret != len) {
      fprintf(stderr, "Failed to read: %s\n", filename);
    } else {
      RenderPdf(filename, pBuf, len, format);
    }
    free(pBuf);
  }

  FPDF_DestroyLibrary();

  return 0;
}
