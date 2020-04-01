// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#if defined PDF_ENABLE_SKIA && !defined _SKIA_SUPPORT_
#define _SKIA_SUPPORT_
#endif

#if defined PDF_ENABLE_SKIA_PATHS && !defined _SKIA_SUPPORT_PATHS_
#define _SKIA_SUPPORT_PATHS_
#endif

#include "public/cpp/fpdf_scopers.h"
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
#include "samples/pdfium_test_dump_helper.h"
#include "samples/pdfium_test_event_helper.h"
#include "samples/pdfium_test_write_helper.h"
#include "testing/fx_string_testhelpers.h"
#include "testing/test_loader.h"
#include "testing/utils/file_util.h"
#include "testing/utils/hash.h"
#include "testing/utils/path_service.h"
#include "third_party/base/logging.h"
#include "third_party/base/optional.h"

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#ifdef ENABLE_CALLGRIND
#include <valgrind/callgrind.h>
#endif  // ENABLE_CALLGRIND

#ifdef PDF_ENABLE_V8
#include "testing/v8_initializer.h"
#include "v8/include/libplatform/libplatform.h"
#include "v8/include/v8.h"
#endif  // PDF_ENABLE_V8

#ifdef _WIN32
#define access _access
#define snprintf _snprintf
#define R_OK 4
#endif

// wordexp is a POSIX function that is only available on OSX and non-Android
// Linux platforms.
#if defined(__APPLE__) || (defined(__linux__) && !defined(__ANDROID__))
#define WORDEXP_AVAILABLE
#endif

#ifdef WORDEXP_AVAILABLE
#include <wordexp.h>
#endif  // WORDEXP_AVAILABLE

enum OutputFormat {
  OUTPUT_NONE,
  OUTPUT_PAGEINFO,
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
  Options() = default;

  bool show_config = false;
  bool show_metadata = false;
  bool send_events = false;
  bool use_load_mem_document = false;
  bool render_oneshot = false;
  bool lcd_text = false;
  bool no_nativetext = false;
  bool grayscale = false;
  bool limit_cache = false;
  bool force_halftone = false;
  bool printing = false;
  bool no_smoothtext = false;
  bool no_smoothimage = false;
  bool no_smoothpath = false;
  bool reverse_byte_order = false;
  bool save_attachments = false;
  bool save_images = false;
  bool save_thumbnails = false;
  bool save_thumbnails_decoded = false;
  bool save_thumbnails_raw = false;
#ifdef PDF_ENABLE_V8
  bool disable_javascript = false;
#ifdef PDF_ENABLE_XFA
  bool disable_xfa = false;
#endif  // PDF_ENABLE_XFA
#endif  // PDF_ENABLE_V8
  bool pages = false;
  bool md5 = false;
#ifdef ENABLE_CALLGRIND
  bool callgrind_delimiters = false;
#endif
#if defined(__APPLE__) || (defined(__linux__) && !defined(__ANDROID__))
  bool linux_no_system_fonts = false;
#endif
  OutputFormat output_format = OUTPUT_NONE;
  std::string password;
  std::string scale_factor_as_string;
  std::string exe_path;
  std::string bin_directory;
  std::string font_directory;
  int first_page = 0;  // First 0-based page number to renderer.
  int last_page = 0;   // Last 0-based page number to renderer.
  time_t time = -1;
};

int PageRenderFlagsFromOptions(const Options& options) {
  int flags = FPDF_ANNOT;
  if (options.lcd_text)
    flags |= FPDF_LCD_TEXT;
  if (options.no_nativetext)
    flags |= FPDF_NO_NATIVETEXT;
  if (options.grayscale)
    flags |= FPDF_GRAYSCALE;
  if (options.limit_cache)
    flags |= FPDF_RENDER_LIMITEDIMAGECACHE;
  if (options.force_halftone)
    flags |= FPDF_RENDER_FORCEHALFTONE;
  if (options.printing)
    flags |= FPDF_PRINTING;
  if (options.no_smoothtext)
    flags |= FPDF_RENDER_NO_SMOOTHTEXT;
  if (options.no_smoothimage)
    flags |= FPDF_RENDER_NO_SMOOTHIMAGE;
  if (options.no_smoothpath)
    flags |= FPDF_RENDER_NO_SMOOTHPATH;
  if (options.reverse_byte_order)
    flags |= FPDF_REVERSE_BYTE_ORDER;
  return flags;
}

Optional<std::string> ExpandDirectoryPath(const std::string& path) {
#if defined(WORDEXP_AVAILABLE)
  wordexp_t expansion;
  if (wordexp(path.c_str(), &expansion, 0) != 0 || expansion.we_wordc < 1) {
    wordfree(&expansion);
    return {};
  }
  // Need to contruct the return value before hand, since wordfree will
  // deallocate |expansion|.
  Optional<std::string> ret_val = {expansion.we_wordv[0]};
  wordfree(&expansion);
  return ret_val;
#else
  return {path};
#endif  // WORDEXP_AVAILABLE
}

Optional<const char*> GetCustomFontPath(const Options& options) {
#if defined(__APPLE__) || (defined(__linux__) && !defined(__ANDROID__))
  // Set custom font path to an empty path. This avoids the fallback to default
  // font paths.
  if (options.linux_no_system_fonts)
    return nullptr;
#endif

  // No custom font path. Use default.
  if (options.font_directory.empty())
    return pdfium::nullopt;

  // Set custom font path to |options.font_directory|.
  return options.font_directory.c_str();
}

struct FPDF_FORMFILLINFO_PDFiumTest final : public FPDF_FORMFILLINFO {
  // Hold a map of the currently loaded pages in order to avoid them
  // to get loaded twice.
  std::map<int, ScopedFPDFPage> loaded_pages;

  // Hold a pointer of FPDF_FORMHANDLE so that PDFium app hooks can
  // make use of it.
  FPDF_FORMHANDLE form_handle;
};

FPDF_FORMFILLINFO_PDFiumTest* ToPDFiumTestFormFillInfo(
    FPDF_FORMFILLINFO* form_fill_info) {
  return static_cast<FPDF_FORMFILLINFO_PDFiumTest*>(form_fill_info);
}

void OutputMD5Hash(const char* file_name, const uint8_t* buffer, int len) {
  // Get the MD5 hash and write it to stdout.
  std::string hash = GenerateMD5Base16(buffer, len);
  printf("MD5:%s:%s\n", file_name, hash.c_str());
}

#ifdef PDF_ENABLE_V8
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

void ExampleAppBeep(IPDF_JSPLATFORM*, int type) {
  printf("BEEP!!! %d\n", type);
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

int ExampleDocGetFilePath(IPDF_JSPLATFORM*, void* file_path, int length) {
  static const char kPath[] = "myfile.pdf";
  constexpr int kRequired = static_cast<int>(sizeof(kPath));
  if (file_path && length >= kRequired)
    memcpy(file_path, kPath, kRequired);
  return kRequired;
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

void ExampleDocPrint(IPDF_JSPLATFORM*,
                     FPDF_BOOL bUI,
                     int nStart,
                     int nEnd,
                     FPDF_BOOL bSilent,
                     FPDF_BOOL bShrinkToFit,
                     FPDF_BOOL bPrintAsImage,
                     FPDF_BOOL bReverse,
                     FPDF_BOOL bAnnotations) {
  printf("Doc Print: %d, %d, %d, %d, %d, %d, %d, %d\n", bUI, nStart, nEnd,
         bSilent, bShrinkToFit, bPrintAsImage, bReverse, bAnnotations);
}

void ExampleDocSubmitForm(IPDF_JSPLATFORM*,
                          void* formData,
                          int length,
                          FPDF_WIDESTRING url) {
  printf("Doc Submit Form: url=%ls + %d data bytes:\n",
         GetPlatformWString(url).c_str(), length);
  uint8_t* ptr = reinterpret_cast<uint8_t*>(formData);
  for (int i = 0; i < length; ++i)
    printf(" %02x", ptr[i]);
  printf("\n");
}

void ExampleDocGotoPage(IPDF_JSPLATFORM*, int page_number) {
  printf("Goto Page: %d\n", page_number);
}

int ExampleFieldBrowse(IPDF_JSPLATFORM*, void* file_path, int length) {
  static const char kPath[] = "selected.txt";
  constexpr int kRequired = static_cast<int>(sizeof(kPath));
  if (file_path && length >= kRequired)
    memcpy(file_path, kPath, kRequired);
  return kRequired;
}
#endif  // PDF_ENABLE_V8

#ifdef PDF_ENABLE_XFA
FPDF_BOOL ExamplePopupMenu(FPDF_FORMFILLINFO* pInfo,
                           FPDF_PAGE page,
                           FPDF_WIDGET always_null,
                           int flags,
                           float x,
                           float y) {
  printf("Popup: x=%2.1f, y=%2.1f, flags=0x%x\n", x, y, flags);
  return true;
}
#endif  // PDF_ENABLE_XFA

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

// |arg| is expected to be "--key=value", and |key| is "--key=".
bool ParseSwitchKeyValue(const std::string& arg,
                         const std::string& key,
                         std::string* value) {
  if (arg.size() <= key.size() || arg.compare(0, key.size(), key) != 0)
    return false;

  *value = arg.substr(key.size());
  return true;
}

bool ParseCommandLine(const std::vector<std::string>& args,
                      Options* options,
                      std::vector<std::string>* files) {
  if (args.empty())
    return false;

  options->exe_path = args[0];
  size_t cur_idx = 1;
  std::string value;
  for (; cur_idx < args.size(); ++cur_idx) {
    const std::string& cur_arg = args[cur_idx];
    if (cur_arg == "--show-config") {
      options->show_config = true;
    } else if (cur_arg == "--show-metadata") {
      options->show_metadata = true;
    } else if (cur_arg == "--send-events") {
      options->send_events = true;
    } else if (cur_arg == "--mem-document") {
      options->use_load_mem_document = true;
    } else if (cur_arg == "--render-oneshot") {
      options->render_oneshot = true;
    } else if (cur_arg == "--lcd-text") {
      options->lcd_text = true;
    } else if (cur_arg == "--no-nativetext") {
      options->no_nativetext = true;
    } else if (cur_arg == "--grayscale") {
      options->grayscale = true;
    } else if (cur_arg == "--limit-cache") {
      options->limit_cache = true;
    } else if (cur_arg == "--force-halftone") {
      options->force_halftone = true;
    } else if (cur_arg == "--printing") {
      options->printing = true;
    } else if (cur_arg == "--no-smoothtext") {
      options->no_smoothtext = true;
    } else if (cur_arg == "--no-smoothimage") {
      options->no_smoothimage = true;
    } else if (cur_arg == "--no-smoothpath") {
      options->no_smoothpath = true;
    } else if (cur_arg == "--reverse-byte-order") {
      options->reverse_byte_order = true;
    } else if (cur_arg == "--save-attachments") {
      options->save_attachments = true;
    } else if (cur_arg == "--save-images") {
      options->save_images = true;
    } else if (cur_arg == "--save-thumbs") {
      options->save_thumbnails = true;
    } else if (cur_arg == "--save-thumbs-dec") {
      options->save_thumbnails_decoded = true;
    } else if (cur_arg == "--save-thumbs-raw") {
      options->save_thumbnails_raw = true;
#ifdef PDF_ENABLE_V8
    } else if (cur_arg == "--disable-javascript") {
      options->disable_javascript = true;
#ifdef PDF_ENABLE_XFA
    } else if (cur_arg == "--disable-xfa") {
      options->disable_xfa = true;
#endif  // PDF_ENABLE_XFA
#endif  // PDF_ENABLE_V8
#ifdef ENABLE_CALLGRIND
    } else if (cur_arg == "--callgrind-delim") {
      options->callgrind_delimiters = true;
#endif
#if defined(__APPLE__) || (defined(__linux__) && !defined(__ANDROID__))
    } else if (cur_arg == "--no-system-fonts") {
      options->linux_no_system_fonts = true;
#endif
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
#endif  // PDF_ENABLE_SKIA
    } else if (ParseSwitchKeyValue(cur_arg, "--font-dir=", &value)) {
      if (!options->font_directory.empty()) {
        fprintf(stderr, "Duplicate --font-dir argument\n");
        return false;
      }
      std::string path = value;
      auto expanded_path = ExpandDirectoryPath(path);
      if (!expanded_path) {
        fprintf(stderr, "Failed to expand --font-dir, %s\n", path.c_str());
        return false;
      }

      if (!PathService::DirectoryExists(expanded_path.value())) {
        fprintf(stderr, "--font-dir, %s, appears to not be a directory\n",
                path.c_str());
        return false;
      }

      options->font_directory = expanded_path.value();

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
    } else if (ParseSwitchKeyValue(cur_arg, "--bin-dir=", &value)) {
      if (!options->bin_directory.empty()) {
        fprintf(stderr, "Duplicate --bin-dir argument\n");
        return false;
      }
      std::string path = value;
      auto expanded_path = ExpandDirectoryPath(path);
      if (!expanded_path) {
        fprintf(stderr, "Failed to expand --bin-dir, %s\n", path.c_str());
        return false;
      }
      options->bin_directory = expanded_path.value();
#endif  // V8_USE_EXTERNAL_STARTUP_DATA
#endif  // PDF_ENABLE_V8

    } else if (ParseSwitchKeyValue(cur_arg, "--password=", &value)) {
      if (!options->password.empty()) {
        fprintf(stderr, "Duplicate --password argument\n");
        return false;
      }
      options->password = value;
    } else if (ParseSwitchKeyValue(cur_arg, "--scale=", &value)) {
      if (!options->scale_factor_as_string.empty()) {
        fprintf(stderr, "Duplicate --scale argument\n");
        return false;
      }
      options->scale_factor_as_string = value;
    } else if (cur_arg == "--show-pageinfo") {
      if (options->output_format != OUTPUT_NONE) {
        fprintf(stderr, "Duplicate or conflicting --show-pageinfo argument\n");
        return false;
      }
      options->output_format = OUTPUT_PAGEINFO;
    } else if (cur_arg == "--show-structure") {
      if (options->output_format != OUTPUT_NONE) {
        fprintf(stderr, "Duplicate or conflicting --show-structure argument\n");
        return false;
      }
      options->output_format = OUTPUT_STRUCTURE;
    } else if (ParseSwitchKeyValue(cur_arg, "--pages=", &value)) {
      if (options->pages) {
        fprintf(stderr, "Duplicate --pages argument\n");
        return false;
      }
      options->pages = true;
      const std::string pages_string = value;
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
    } else if (ParseSwitchKeyValue(cur_arg, "--time=", &value)) {
      if (options->time > -1) {
        fprintf(stderr, "Duplicate --time argument\n");
        return false;
      }
      const std::string time_string = value;
      std::stringstream(time_string) >> options->time;
      if (options->time < 0) {
        fprintf(stderr, "Invalid --time argument, must be non-negative\n");
        return false;
      }
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
}

FPDF_BOOL Is_Data_Avail(FX_FILEAVAIL* avail, size_t offset, size_t size) {
  return true;
}

void Add_Segment(FX_DOWNLOADHINTS* hints, size_t offset, size_t size) {}

FPDF_PAGE GetPageForIndex(FPDF_FORMFILLINFO* param,
                          FPDF_DOCUMENT doc,
                          int index) {
  FPDF_FORMFILLINFO_PDFiumTest* form_fill_info =
      ToPDFiumTestFormFillInfo(param);
  auto& loaded_pages = form_fill_info->loaded_pages;
  auto iter = loaded_pages.find(index);
  if (iter != loaded_pages.end())
    return iter->second.get();

  ScopedFPDFPage page(FPDF_LoadPage(doc, index));
  if (!page)
    return nullptr;

  // Mark the page as loaded first to prevent infinite recursion.
  FPDF_PAGE page_ptr = page.get();
  loaded_pages[index] = std::move(page);

  FPDF_FORMHANDLE& form_handle = form_fill_info->form_handle;
  FORM_OnAfterLoadPage(page_ptr, form_handle);
  FORM_DoPageAAction(page_ptr, form_handle, FPDFPAGE_AACTION_OPEN);
  return page_ptr;
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
    WriteImages(page, name.c_str(), page_index);
  if (options.save_thumbnails)
    WriteThumbnail(page, name.c_str(), page_index);
  if (options.save_thumbnails_decoded)
    WriteDecodedThumbnailStream(page, name.c_str(), page_index);
  if (options.save_thumbnails_raw)
    WriteRawThumbnailStream(page, name.c_str(), page_index);
  if (options.output_format == OUTPUT_PAGEINFO) {
    DumpPageInfo(page, page_index);
    return true;
  }
  if (options.output_format == OUTPUT_STRUCTURE) {
    DumpPageStructure(page, page_index);
    return true;
  }

  ScopedFPDFTextPage text_page(FPDFText_LoadPage(page));
  double scale = 1.0;
  if (!options.scale_factor_as_string.empty())
    std::stringstream(options.scale_factor_as_string) >> scale;

  auto width = static_cast<int>(FPDF_GetPageWidthF(page) * scale);
  auto height = static_cast<int>(FPDF_GetPageHeightF(page) * scale);
  int alpha = FPDFPage_HasTransparency(page) ? 1 : 0;
  ScopedFPDFBitmap bitmap(FPDFBitmap_Create(width, height, alpha));

  if (bitmap) {
    FPDF_DWORD fill_color = alpha ? 0x00000000 : 0xFFFFFFFF;
    FPDFBitmap_FillRect(bitmap.get(), 0, 0, width, height, fill_color);

    int flags = PageRenderFlagsFromOptions(options);
    if (options.render_oneshot) {
      // Note, client programs probably want to use this method instead of the
      // progressive calls. The progressive calls are if you need to pause the
      // rendering to update the UI, the PDF renderer will break when possible.
      FPDF_RenderPageBitmap(bitmap.get(), page, 0, 0, width, height, 0, flags);
    } else {
      IFSDK_PAUSE pause;
      pause.version = 1;
      pause.NeedToPauseNow = &NeedToPauseNow;

      int rv = FPDF_RenderPageBitmap_Start(bitmap.get(), page, 0, 0, width,
                                           height, 0, flags, &pause);
      while (rv == FPDF_RENDER_TOBECONTINUED)
        rv = FPDF_RenderPage_Continue(page, &pause);
    }

    FPDF_FFLDraw(form, bitmap.get(), page, 0, 0, width, height, 0, flags);

    if (!options.render_oneshot)
      FPDF_RenderPage_Close(page);

    int stride = FPDFBitmap_GetStride(bitmap.get());
    void* buffer = FPDFBitmap_GetBuffer(bitmap.get());

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
    if (options.md5 && !image_file_name.empty()) {
      OutputMD5Hash(image_file_name.c_str(),
                    static_cast<const uint8_t*>(buffer), stride * height);
    }
  } else {
    fprintf(stderr, "Page was too large to be rendered.\n");
  }

  FORM_DoPageAAction(page, form, FPDFPAGE_AACTION_CLOSE);
  FORM_OnBeforeClosePage(page, form);
  return !!bitmap;
}

void RenderPdf(const std::string& name,
               const char* buf,
               size_t len,
               const Options& options,
               const std::string& events) {
  TestLoader loader({buf, len});

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

  // |pdf_avail| must outlive |doc|.
  ScopedFPDFAvail pdf_avail(FPDFAvail_Create(&file_avail, &file_access));

  // |doc| must outlive |form_callbacks.loaded_pages|.
  ScopedFPDFDocument doc;

  const char* password =
      options.password.empty() ? nullptr : options.password.c_str();
  bool is_linearized = false;
  if (options.use_load_mem_document) {
    doc.reset(FPDF_LoadMemDocument(buf, len, password));
  } else {
    if (FPDFAvail_IsLinearized(pdf_avail.get()) == PDF_LINEARIZED) {
      int avail_status = PDF_DATA_NOTAVAIL;
      doc.reset(FPDFAvail_GetDocument(pdf_avail.get(), password));
      if (doc) {
        while (avail_status == PDF_DATA_NOTAVAIL)
          avail_status = FPDFAvail_IsDocAvail(pdf_avail.get(), &hints);

        if (avail_status == PDF_DATA_ERROR) {
          fprintf(stderr, "Unknown error in checking if doc was available.\n");
          return;
        }
        avail_status = FPDFAvail_IsFormAvail(pdf_avail.get(), &hints);
        if (avail_status == PDF_FORM_ERROR ||
            avail_status == PDF_FORM_NOTAVAIL) {
          fprintf(stderr,
                  "Error %d was returned in checking if form was available.\n",
                  avail_status);
          return;
        }
        is_linearized = true;
      }
    } else {
      doc.reset(FPDF_LoadCustomDocument(&file_access, password));
    }
  }

  if (!doc) {
    PrintLastError();
    return;
  }

  if (!FPDF_DocumentHasValidCrossReferenceTable(doc.get()))
    fprintf(stderr, "Document has invalid cross reference table\n");

  (void)FPDF_GetDocPermissions(doc.get());

  if (options.show_metadata)
    DumpMetaData(doc.get());

  if (options.save_attachments)
    WriteAttachments(doc.get(), name);

#ifdef PDF_ENABLE_V8
  IPDF_JSPLATFORM platform_callbacks = {};
  platform_callbacks.version = 3;
  platform_callbacks.app_alert = ExampleAppAlert;
  platform_callbacks.app_beep = ExampleAppBeep;
  platform_callbacks.app_response = ExampleAppResponse;
  platform_callbacks.Doc_getFilePath = ExampleDocGetFilePath;
  platform_callbacks.Doc_mail = ExampleDocMail;
  platform_callbacks.Doc_print = ExampleDocPrint;
  platform_callbacks.Doc_submitForm = ExampleDocSubmitForm;
  platform_callbacks.Doc_gotoPage = ExampleDocGotoPage;
  platform_callbacks.Field_browse = ExampleFieldBrowse;
#endif  // PDF_ENABLE_V8

  FPDF_FORMFILLINFO_PDFiumTest form_callbacks = {};
#ifdef PDF_ENABLE_XFA
  form_callbacks.version = 2;
  form_callbacks.xfa_disabled =
      options.disable_xfa || options.disable_javascript;
  form_callbacks.FFI_PopupMenu = ExamplePopupMenu;
#else   // PDF_ENABLE_XFA
  form_callbacks.version = 1;
#endif  // PDF_ENABLE_XFA
  form_callbacks.FFI_GetPage = GetPageForIndex;

#ifdef PDF_ENABLE_V8
  if (!options.disable_javascript)
    form_callbacks.m_pJsPlatform = &platform_callbacks;
#endif  // PDF_ENABLE_V8

  ScopedFPDFFormHandle form(
      FPDFDOC_InitFormFillEnvironment(doc.get(), &form_callbacks));
  form_callbacks.form_handle = form.get();

#ifdef PDF_ENABLE_XFA
  if (!options.disable_xfa && !options.disable_javascript) {
    int doc_type = FPDF_GetFormType(doc.get());
    if (doc_type == FORMTYPE_XFA_FULL || doc_type == FORMTYPE_XFA_FOREGROUND) {
      if (!FPDF_LoadXFA(doc.get()))
        fprintf(stderr, "LoadXFA unsuccessful, continuing anyway.\n");
    }
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
    if (is_linearized) {
      int avail_status = PDF_DATA_NOTAVAIL;
      while (avail_status == PDF_DATA_NOTAVAIL)
        avail_status = FPDFAvail_IsPageAvail(pdf_avail.get(), i, &hints);

      if (avail_status == PDF_DATA_ERROR) {
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
#ifdef PDF_ENABLE_V8
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
#if defined(PDF_ENABLE_SKIA)
  config.append(maybe_comma);
  config.append("SKIA");
  maybe_comma = ",";
#elif defined(PDF_ENABLE_SKIA_PATHS)
  config.append(maybe_comma);
  config.append("SKIAPATHS");
  maybe_comma = ",";
#endif
  printf("%s\n", config.c_str());
}

constexpr char kUsageString[] =
    "Usage: pdfium_test [OPTION] [FILE]...\n"
    "  --show-config        - print build options and exit\n"
    "  --show-metadata      - print the file metadata\n"
    "  --show-pageinfo      - print information about pages\n"
    "  --show-structure     - print the structure elements from the document\n"
    "  --send-events        - send input described by .evt file\n"
    "  --mem-document       - load document with FPDF_LoadMemDocument()\n"
    "  --render-oneshot     - render image without using progressive renderer\n"
    "  --lcd-text           - render text optimized for LCD displays\n"
    "  --no-nativetext      - render without using the native text output\n"
    "  --grayscale          - render grayscale output\n"
    "  --limit-cache        - render limiting image cache size\n"
    "  --force-halftone     - render forcing halftone\n"
    "  --printing           - render as if for printing\n"
    "  --no-smoothtext      - render disabling text anti-aliasing\n"
    "  --no-smoothimage     - render disabling image anti-alisasing\n"
    "  --no-smoothpath      - render disabling path anti-aliasing\n"
    "  --reverse-byte-order - render to BGRA, if supported by the output "
    "format\n"
    "  --save-attachments   - write embedded attachments "
    "<pdf-name>.attachment.<attachment-name>\n"
    "  --save-images        - write embedded images "
    "<pdf-name>.<page-number>.<object-number>.png\n"
    "  --save-thumbs        - write page thumbnails "
    "<pdf-name>.thumbnail.<page-number>.png\n"
    "  --save-thumbs-dec    - write page thumbnails' decoded stream data"
    "<pdf-name>.thumbnail.decoded.<page-number>.png\n"
    "  --save-thumbs-raw    - write page thumbnails' raw stream data"
    "<pdf-name>.thumbnail.raw.<page-number>.png\n"
#ifdef PDF_ENABLE_V8
    "  --disable-javascript - do not execute JS in PDF files\n"
#ifdef PDF_ENABLE_XFA
    "  --disable-xfa        - do not process XFA forms\n"
#endif  // PDF_ENABLE_XFA
#endif  // PDF_ENABLE_V8
#ifdef ENABLE_CALLGRIND
    "  --callgrind-delim    - delimit interesting section when using "
    "callgrind\n"
#endif
#if defined(__APPLE__) || (defined(__linux__) && !defined(__ANDROID__))
    "  --no-system-fonts    - do not use system fonts, overrides --font-dir\n"
#endif
    "  --bin-dir=<path>     - override path to v8 external data\n"
    "  --font-dir=<path>    - override path to external fonts\n"
    "  --scale=<number>     - scale output size by number (e.g. 0.5)\n"
    "  --pages=<number>(-<number>) - only render the given 0-based page(s)\n"
#ifdef _WIN32
    "  --bmp   - write page images <pdf-name>.<page-number>.bmp\n"
    "  --emf   - write page meta files <pdf-name>.<page-number>.emf\n"
    "  --ps2   - write page raw PostScript (Lvl 2) "
    "<pdf-name>.<page-number>.ps\n"
    "  --ps3   - write page raw PostScript (Lvl 3) "
    "<pdf-name>.<page-number>.ps\n"
#endif
    "  --txt   - write page text in UTF32-LE <pdf-name>.<page-number>.txt\n"
    "  --png   - write page images <pdf-name>.<page-number>.png\n"
    "  --ppm   - write page images <pdf-name>.<page-number>.ppm\n"
    "  --annot - write annotation info <pdf-name>.<page-number>.annot.txt\n"
#ifdef PDF_ENABLE_SKIA
    "  --skp   - write page images <pdf-name>.<page-number>.skp\n"
#endif
    "  --md5   - write output image paths and their md5 hashes to stdout.\n"
    "  --time=<number> - Seconds since the epoch to set system time.\n"
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
  std::unique_ptr<v8::Platform> platform;
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
  v8::StartupData snapshot;
  if (!options.disable_javascript) {
    platform = InitializeV8ForPDFiumWithStartupData(
        options.exe_path, options.bin_directory, &snapshot);
  }
#else   // V8_USE_EXTERNAL_STARTUP_DATA
  if (!options.disable_javascript)
    platform = InitializeV8ForPDFium(options.exe_path);
#endif  // V8_USE_EXTERNAL_STARTUP_DATA
#endif  // PDF_ENABLE_V8

  FPDF_LIBRARY_CONFIG config;
  config.version = 2;
  config.m_pUserFontPaths = nullptr;
  config.m_pIsolate = nullptr;
  config.m_v8EmbedderSlot = 0;

  const char* path_array[2] = {nullptr, nullptr};
  Optional<const char*> custom_font_path = GetCustomFontPath(options);
  if (custom_font_path.has_value()) {
    path_array[0] = custom_font_path.value();
    config.m_pUserFontPaths = path_array;
  }

  FPDF_InitLibraryWithConfig(&config);

  UNSUPPORT_INFO unsupported_info = {};
  unsupported_info.version = 1;
  unsupported_info.FSDK_UnSupport_Handler = ExampleUnsupportedHandler;

  FSDK_SetUnSpObjProcessHandler(&unsupported_info);

  if (options.time > -1) {
    // This must be a static var to avoid explicit capture, so the lambda can be
    // converted to a function ptr.
    static time_t time_ret = options.time;
    FSDK_SetTimeFunction([]() { return time_ret; });
    FSDK_SetLocaltimeFunction([](const time_t* tp) { return gmtime(tp); });
  }

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
  if (!options.disable_javascript) {
    v8::V8::ShutdownPlatform();
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
    free(const_cast<char*>(snapshot.data));
#endif  // V8_USE_EXTERNAL_STARTUP_DATA
  }
#endif  // PDF_ENABLE_V8

  return 0;
}
