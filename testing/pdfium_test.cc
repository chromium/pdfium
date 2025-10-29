// Copyright 2010 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <locale.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#if defined(PDF_ENABLE_SKIA) && !defined(PDF_USE_SKIA)
#define PDF_USE_SKIA
#endif

#include "core/fxcrt/check_op.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/span.h"
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
#include "testing/command_line_helpers.h"
#include "testing/font_renamer.h"
#include "testing/fx_string_testhelpers.h"
#include "testing/helpers/dump.h"
#include "testing/helpers/event.h"
#include "testing/helpers/page_renderer.h"
#include "testing/helpers/write.h"
#include "testing/test_loader.h"
#include "testing/utils/file_util.h"
#include "testing/utils/hash.h"
#include "testing/utils/path_service.h"

#ifdef _WIN32
#include <crtdbg.h>
#include <errhandlingapi.h>
#include <io.h>
#include <wingdi.h>

#include "testing/helpers/win32/com_factory.h"
#else
#include <unistd.h>
#endif  // _WIN32

#ifdef ENABLE_CALLGRIND
#include <valgrind/callgrind.h>
#endif  // ENABLE_CALLGRIND

#if defined(PDF_USE_PARTITION_ALLOC)
#include "testing/allocator_shim_config.h"
#endif

#ifdef PDF_ENABLE_SKIA
#include "third_party/skia/include/core/SkCanvas.h"           // nogncheck
#include "third_party/skia/include/core/SkColor.h"            // nogncheck
#include "third_party/skia/include/core/SkDocument.h"         // nogncheck
#include "third_party/skia/include/core/SkPicture.h"          // nogncheck
#include "third_party/skia/include/core/SkPictureRecorder.h"  // nogncheck
#include "third_party/skia/include/core/SkPixmap.h"           // nogncheck
#include "third_party/skia/include/core/SkRefCnt.h"           // nogncheck
#include "third_party/skia/include/core/SkStream.h"           // nogncheck
#include "third_party/skia/include/core/SkSurface.h"          // nogncheck

#ifdef _WIN32
#include "third_party/skia/include/docs/SkXPSDocument.h"  // nogncheck
#ifdef PDF_ENABLE_RUST_PNG
#include "third_party/skia/include/encode/SkPngRustEncoder.h"  // nogncheck
#else
#include "third_party/skia/include/encode/SkPngEncoder.h"  // nogncheck
#endif  // PDF_ENABLE_RUST_PNG
#endif  // _WIN32

#ifdef BUILD_WITH_CHROMIUM
#include "testing/chromium_support/discardable_memory_allocator.h"  // nogncheck
#endif
#endif  // PDF_ENABLE_SKIA

#ifdef PDF_ENABLE_V8
#include "testing/v8_initializer.h"
#include "v8/include/libplatform/libplatform.h"
#include "v8/include/v8-array-buffer.h"
#include "v8/include/v8-isolate.h"
#include "v8/include/v8-platform.h"
#include "v8/include/v8-snapshot.h"
#endif  // PDF_ENABLE_V8

#ifdef _WIN32
#define access _access
#define snprintf _snprintf
#define R_OK 4
#endif

// wordexp is a POSIX function that is only available on macOS and non-Android
// Linux platforms.
#if defined(__APPLE__) || (defined(__linux__) && !defined(__ANDROID__))
#define WORDEXP_AVAILABLE
#endif

#ifdef WORDEXP_AVAILABLE
#include <wordexp.h>
#endif  // WORDEXP_AVAILABLE

namespace {

enum class RendererType {
  kDefault,
  kAgg,
#ifdef _WIN32
  kGdi,
#endif  // _WIN32
#if defined(PDF_ENABLE_SKIA)
  kSkia,
#endif  // defined(PDF_ENABLE_SKIA)
};

enum class OutputFormat {
  kNone,
  kPageInfo,
  kStructure,
  kText,
  kPpm,
  kPng,
  kAnnot,
#ifdef _WIN32
  kBmp,
  kEmf,
  kPs2,
  kPs3,
  kPs3Type42,
#endif
#ifdef PDF_ENABLE_SKIA
  kSkp,
#ifdef _WIN32
  kXps,
#endif  // _WIN32
#endif  // PDF_ENABLE_SKIA
};

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
  bool forced_color = false;
  bool fill_to_stroke = false;
  bool limit_cache = false;
  bool force_halftone = false;
  bool printing = false;
  bool no_smoothtext = false;
  bool no_smoothimage = false;
  bool no_smoothpath = false;
  bool reverse_byte_order = false;
  bool save_attachments = false;
  bool save_images = false;
  bool save_rendered_images = false;
  bool save_thumbnails = false;
  bool save_thumbnails_decoded = false;
  bool save_thumbnails_raw = false;
  RendererType use_renderer_type = RendererType::kDefault;
#ifdef PDF_ENABLE_V8
  bool disable_javascript = false;
  std::string js_flags;  // Extra flags to pass to v8 init.
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
  bool croscore_font_names = false;
  OutputFormat output_format = OutputFormat::kNone;
  std::string password;
  std::string render_repeats_as_string;
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
  if (options.lcd_text) {
    flags |= FPDF_LCD_TEXT;
  }
  if (options.no_nativetext) {
    flags |= FPDF_NO_NATIVETEXT;
  }
  if (options.grayscale) {
    flags |= FPDF_GRAYSCALE;
  }
  if (options.fill_to_stroke) {
    flags |= FPDF_CONVERT_FILL_TO_STROKE;
  }
  if (options.limit_cache) {
    flags |= FPDF_RENDER_LIMITEDIMAGECACHE;
  }
  if (options.force_halftone) {
    flags |= FPDF_RENDER_FORCEHALFTONE;
  }
  if (options.printing) {
    flags |= FPDF_PRINTING;
  }
  if (options.no_smoothtext) {
    flags |= FPDF_RENDER_NO_SMOOTHTEXT;
  }
  if (options.no_smoothimage) {
    flags |= FPDF_RENDER_NO_SMOOTHIMAGE;
  }
  if (options.no_smoothpath) {
    flags |= FPDF_RENDER_NO_SMOOTHPATH;
  }
  if (options.reverse_byte_order) {
    flags |= FPDF_REVERSE_BYTE_ORDER;
  }
  return flags;
}

std::optional<std::string> ExpandDirectoryPath(const std::string& path) {
#if defined(WORDEXP_AVAILABLE)
  wordexp_t expansion;
  if (wordexp(path.c_str(), &expansion, 0) != 0 || expansion.we_wordc < 1) {
    wordfree(&expansion);
    return {};
  }
  // Need to contruct the return value before hand, since wordfree will
  // deallocate |expansion|.
  std::optional<std::string> ret_val = {expansion.we_wordv[0]};
  wordfree(&expansion);
  return ret_val;
#else
  return {path};
#endif  // WORDEXP_AVAILABLE
}

std::optional<const char*> GetCustomFontPath(const Options& options) {
#if defined(__APPLE__) || (defined(__linux__) && !defined(__ANDROID__))
  // Set custom font path to an empty path. This avoids the fallback to default
  // font paths.
  if (options.linux_no_system_fonts) {
    return nullptr;
  }
#endif

  // No custom font path. Use default.
  if (options.font_directory.empty()) {
    return std::nullopt;
  }

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

void OutputMD5Hash(const char* file_name, pdfium::span<const uint8_t> output) {
  // Get the MD5 hash and write it to stdout.
  std::string hash = GenerateMD5Base16(output);
  printf("MD5:%s:%s\n", file_name, hash.c_str());
}

#ifdef PDF_ENABLE_V8

struct V8IsolateDeleter {
  inline void operator()(v8::Isolate* ptr) { ptr->Dispose(); }
};

// These example JS platform callback handlers are entirely optional,
// and exist here to show the flow of information from a document back
// to the embedder.
int ExampleAppAlert(IPDF_JSPLATFORM*,
                    FPDF_WIDESTRING msg,
                    FPDF_WIDESTRING title,
                    int type,
                    int icon) {
  printf("%ls", GetPlatformWString(title).c_str());
  if (icon || type) {
    printf("[icon=%d,type=%d]", icon, type);
  }
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
  static constexpr int kRequired = static_cast<int>(sizeof(kPath));
  if (file_path && length >= kRequired) {
    memcpy(file_path, kPath, kRequired);
  }
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
  for (int i = 0; i < length; ++i) {
    printf(" %02x", ptr[i]);
  }
  printf("\n");
}

void ExampleDocGotoPage(IPDF_JSPLATFORM*, int page_number) {
  printf("Goto Page: %d\n", page_number);
}

int ExampleFieldBrowse(IPDF_JSPLATFORM*, void* file_path, int length) {
  static const char kPath[] = "selected.txt";
  static constexpr int kRequired = static_cast<int>(sizeof(kPath));
  if (file_path && length >= kRequired) {
    memcpy(file_path, kPath, kRequired);
  }
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

void ExampleNamedAction(FPDF_FORMFILLINFO* pInfo, FPDF_BYTESTRING name) {
  printf("Execute named action: %s\n", name);
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
  if (args.empty()) {
    return false;
  }

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
    } else if (cur_arg == "--forced-color") {
      options->forced_color = true;
    } else if (cur_arg == "--fill-to-stroke") {
      options->fill_to_stroke = true;
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
      if (options->save_rendered_images) {
        fprintf(stderr,
                "--save-rendered-images conflicts with --save-images\n");
        return false;
      }
      options->save_images = true;
    } else if (cur_arg == "--save-rendered-images") {
      if (options->save_images) {
        fprintf(stderr,
                "--save-images conflicts with --save-rendered-images\n");
        return false;
      }
      options->save_rendered_images = true;
    } else if (cur_arg == "--save-thumbs") {
      options->save_thumbnails = true;
    } else if (cur_arg == "--save-thumbs-dec") {
      options->save_thumbnails_decoded = true;
    } else if (cur_arg == "--save-thumbs-raw") {
      options->save_thumbnails_raw = true;
    } else if (ParseSwitchKeyValue(cur_arg, "--use-renderer=", &value)) {
      if (options->use_renderer_type != RendererType::kDefault) {
        fprintf(stderr, "Duplicate --use-renderer argument\n");
        return false;
      }
      if (value == "agg") {
        options->use_renderer_type = RendererType::kAgg;
#ifdef _WIN32
      } else if (value == "gdi") {
        options->use_renderer_type = RendererType::kGdi;
#endif  // _WIN32
#if defined(PDF_ENABLE_SKIA)
      } else if (value == "skia") {
        options->use_renderer_type = RendererType::kSkia;
#endif  // defined(PDF_ENABLE_SKIA)
      } else {
        fprintf(stderr, "Invalid --use-renderer argument\n");
        return false;
      }
#ifdef PDF_ENABLE_V8
    } else if (cur_arg == "--disable-javascript") {
      options->disable_javascript = true;
    } else if (ParseSwitchKeyValue(cur_arg, "--js-flags=", &value)) {
      if (!options->js_flags.empty()) {
        fprintf(stderr, "Duplicate --js-flags argument\n");
        return false;
      }
      options->js_flags = value;
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
    } else if (cur_arg == "--croscore-font-names") {
      options->croscore_font_names = true;
    } else if (cur_arg == "--ppm") {
      if (options->output_format != OutputFormat::kNone) {
        fprintf(stderr, "Duplicate or conflicting --ppm argument\n");
        return false;
      }
      options->output_format = OutputFormat::kPpm;
    } else if (cur_arg == "--png") {
      if (options->output_format != OutputFormat::kNone) {
        fprintf(stderr, "Duplicate or conflicting --png argument\n");
        return false;
      }
      options->output_format = OutputFormat::kPng;
    } else if (cur_arg == "--txt") {
      if (options->output_format != OutputFormat::kNone) {
        fprintf(stderr, "Duplicate or conflicting --txt argument\n");
        return false;
      }
      options->output_format = OutputFormat::kText;
    } else if (cur_arg == "--annot") {
      if (options->output_format != OutputFormat::kNone) {
        fprintf(stderr, "Duplicate or conflicting --annot argument\n");
        return false;
      }
      options->output_format = OutputFormat::kAnnot;
#ifdef PDF_ENABLE_SKIA
    } else if (cur_arg == "--skp") {
      if (options->output_format != OutputFormat::kNone) {
        fprintf(stderr, "Duplicate or conflicting --skp argument\n");
        return false;
      }
      options->output_format = OutputFormat::kSkp;
#ifdef _WIN32
    } else if (cur_arg == "--xps") {
      if (options->output_format != OutputFormat::kNone) {
        fprintf(stderr, "Duplicate or conflicting --xps argument\n");
        return false;
      }
      options->output_format = OutputFormat::kXps;
#endif  // _WIN32
#endif  // PDF_ENABLE_SKIA
    } else if (ParseSwitchKeyValue(cur_arg, "--font-dir=", &value)) {
      if (!options->font_directory.empty()) {
        fprintf(stderr, "Duplicate --font-dir argument\n");
        return false;
      }
      std::string path = value;
      std::optional<std::string> expanded_path = ExpandDirectoryPath(path);
      if (!expanded_path.has_value()) {
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
      if (options->output_format != OutputFormat::kNone) {
        fprintf(stderr, "Duplicate or conflicting --emf argument\n");
        return false;
      }
      options->output_format = OutputFormat::kEmf;
    } else if (cur_arg == "--ps2") {
      if (options->output_format != OutputFormat::kNone) {
        fprintf(stderr, "Duplicate or conflicting --ps2 argument\n");
        return false;
      }
      options->output_format = OutputFormat::kPs2;
    } else if (cur_arg == "--ps3") {
      if (options->output_format != OutputFormat::kNone) {
        fprintf(stderr, "Duplicate or conflicting --ps3 argument\n");
        return false;
      }
      options->output_format = OutputFormat::kPs3;
    } else if (cur_arg == "--ps3-type42") {
      if (options->output_format != OutputFormat::kNone) {
        fprintf(stderr, "Duplicate or conflicting --ps3-type42 argument\n");
        return false;
      }
      options->output_format = OutputFormat::kPs3Type42;
    } else if (cur_arg == "--bmp") {
      if (options->output_format != OutputFormat::kNone) {
        fprintf(stderr, "Duplicate or conflicting --bmp argument\n");
        return false;
      }
      options->output_format = OutputFormat::kBmp;
#endif  // _WIN32

#ifdef PDF_ENABLE_V8
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
    } else if (ParseSwitchKeyValue(cur_arg, "--bin-dir=", &value)) {
      if (!options->bin_directory.empty()) {
        fprintf(stderr, "Duplicate --bin-dir argument\n");
        return false;
      }
      std::string path = value;
      std::optional<std::string> expanded_path = ExpandDirectoryPath(path);
      if (!expanded_path.has_value()) {
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
    } else if (ParseSwitchKeyValue(cur_arg, "--render-repeats=", &value)) {
      if (!options->render_repeats_as_string.empty()) {
        fprintf(stderr, "Duplicate --render-repeats argument\n");
        return false;
      }
      options->render_repeats_as_string = value;
    } else if (ParseSwitchKeyValue(cur_arg, "--scale=", &value)) {
      if (!options->scale_factor_as_string.empty()) {
        fprintf(stderr, "Duplicate --scale argument\n");
        return false;
      }
      options->scale_factor_as_string = value;
    } else if (cur_arg == "--show-pageinfo") {
      if (options->output_format != OutputFormat::kNone) {
        fprintf(stderr, "Duplicate or conflicting --show-pageinfo argument\n");
        return false;
      }
      options->output_format = OutputFormat::kPageInfo;
    } else if (cur_arg == "--show-structure") {
      if (options->output_format != OutputFormat::kNone) {
        fprintf(stderr, "Duplicate or conflicting --show-structure argument\n");
        return false;
      }
      options->output_format = OutputFormat::kStructure;
    } else if (ParseSwitchKeyValue(cur_arg, "--pages=", &value)) {
      if (options->pages) {
        fprintf(stderr, "Duplicate --pages argument\n");
        return false;
      }
      options->pages = true;
      const std::string pages_string = value;
      size_t first_dash = pages_string.find('-');
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
      files->push_back(cur_arg);
    }
  }

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
  if (iter != loaded_pages.end()) {
    return iter->second.get();
  }

  ScopedFPDFPage page(FPDF_LoadPage(doc, index));
  if (!page) {
    return nullptr;
  }

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

class Processor final {
 public:
  Processor(const Options* options, const std::function<void()>* idler)
      : options_(options), idler_(idler) {
    DCHECK(options_);
    DCHECK(idler_);
  }

  const Options& options() const { return *options_; }
  const std::function<void()>& idler() const { return *idler_; }

#ifdef _WIN32
  ComFactory& com_factory() { return com_factory_; }
#endif  // _WIN32

  // Invokes `idler()`.
  void Idle() const { idler()(); }

  void ProcessPdf(const std::string& name,
                  pdfium::span<const uint8_t> data,
                  const std::string& events);

 private:
  const Options* options_;
  const std::function<void()>* idler_;

#ifdef _WIN32
  ComFactory com_factory_;
#endif  // _WIN32
};

class PdfProcessor final {
 public:
  PdfProcessor(Processor* processor,
               const std::string* name,
               const std::string* events,
               FPDF_DOCUMENT doc,
               FPDF_FORMHANDLE form,
               FPDF_FORMFILLINFO_PDFiumTest* form_fill_info)
      : processor_(processor),
        name_(name),
        events_(events),
        doc_(doc),
        form_(form),
        form_fill_info_(form_fill_info) {
    DCHECK(processor_);
    DCHECK(name_);
    DCHECK(events_);
    DCHECK(doc_);
    DCHECK(form_);
    DCHECK(form_fill_info_);
  }

  bool ProcessPage(int page_index);

 private:
  // Per processor state.
  const Options& options() const { return processor_->options(); }
  const std::function<void()>& idler() const { return processor_->idler(); }

#ifdef _WIN32
  ComFactory& com_factory() { return processor_->com_factory(); }
#endif  // _WIN32

  // Per PDF state.
  const std::string& name() const { return *name_; }
  const std::string& events() const { return *events_; }
  FPDF_DOCUMENT doc() const { return doc_; }
  FPDF_FORMHANDLE form() const { return form_; }

  // Invokes `idler()`.
  void Idle() const { idler()(); }

  FPDF_PAGE GetPage(int page_index) const {
    return GetPageForIndex(form_fill_info_, doc_, page_index);
  }

  Processor* processor_;
  const std::string* name_;
  const std::string* events_;
  FPDF_DOCUMENT doc_;
  FPDF_FORMHANDLE form_;
  FPDF_FORMFILLINFO_PDFiumTest* form_fill_info_;
};

// Page renderer with bitmap output.
class BitmapPageRenderer : public PageRenderer {
 public:
  // Function type that writes rendered output to a file, returning `false` on
  // failure.
  //
  // Intended to wrap functions from `pdfium_test_write_helper.h`.
  using PageWriter = std::function<bool(BitmapPageRenderer& renderer,
                                        const std::string& name,
                                        int page_index,
                                        bool md5)>;

  // Wraps a `PageWriter` around a function pointer that writes the text page.
  static PageWriter WrapPageWriter(
      void (*text_page_writer)(FPDF_TEXTPAGE text_page,
                               const char* pdf_name,
                               int num)) {
    return [text_page_writer](BitmapPageRenderer& renderer,
                              const std::string& name, int page_index,
                              bool /*md5*/) {
      ScopedFPDFTextPage text_page(FPDFText_LoadPage(renderer.page()));
      if (!text_page) {
        return false;
      }

      text_page_writer(text_page.get(), name.c_str(), page_index);
      return true;
    };
  }

  // Wraps a `PageWriter` around a function pointer that writes the page.
  static PageWriter WrapPageWriter(void (*page_writer)(FPDF_PAGE page,
                                                       const char* pdf_name,
                                                       int num)) {
    return [page_writer](BitmapPageRenderer& renderer, const std::string& name,
                         int page_index, bool /*md5*/) {
      page_writer(renderer.page(), name.c_str(), page_index);
      return true;
    };
  }

  // Wraps a `PageWriter` around a function pointer that writes the rasterized
  // bitmap to an image file.
  static PageWriter WrapPageWriter(
      std::string (*bitmap_writer)(const char* pdf_name,
                                   int num,
                                   void* buffer,
                                   int stride,
                                   int width,
                                   int height)) {
    return [bitmap_writer](BitmapPageRenderer& renderer,
                           const std::string& name, int page_index, bool md5) {
      int stride = FPDFBitmap_GetStride(renderer.bitmap());
      void* buffer = FPDFBitmap_GetBuffer(renderer.bitmap());
      std::string image_file_name = bitmap_writer(
          name.c_str(), page_index, buffer, /*stride=*/stride,
          /*width=*/renderer.width(), /*height=*/renderer.height());
      if (image_file_name.empty()) {
        return false;
      }

      if (md5) {
        // Write the filename and the MD5 of the buffer to stdout.
        OutputMD5Hash(image_file_name.c_str(),
                      UNSAFE_TODO(pdfium::span(
                          static_cast<const uint8_t*>(buffer),
                          static_cast<size_t>(stride) * renderer.height())));
      }
      return true;
    };
  }

  bool HasOutput() const override { return !!bitmap_; }

  void Finish(FPDF_FORMHANDLE form) override {
    FPDF_FFLDraw(form, bitmap(), page(), /*start_x=*/0, /*start_y=*/0,
                 /*size_x=*/width(), /*size_y=*/height(), /*rotate=*/0,
                 /*flags=*/flags());
    Idle();
  }

  bool Write(const std::string& name, int page_index, bool md5) override {
    return writer_ && writer_(*this, name, page_index, md5);
  }

 protected:
  BitmapPageRenderer(FPDF_PAGE page,
                     int width,
                     int height,
                     int flags,
                     const std::function<void()>& idler,
                     PageWriter writer)
      : PageRenderer(page, /*width=*/width, /*height=*/height, /*flags=*/flags),
        idler_(idler),
        writer_(std::move(writer)) {}

  bool InitializeBitmap(void* first_scan) {
    bool alpha = FPDFPage_HasTransparency(page());
    bitmap_.reset(FPDFBitmap_CreateEx(
        /*width=*/width(), /*height=*/height(),
        /*format=*/alpha ? FPDFBitmap_BGRA : FPDFBitmap_BGRx, first_scan,
        /*stride=*/width() * sizeof(uint32_t)));
    if (!bitmap()) {
      return false;
    }

    FPDF_DWORD fill_color = alpha ? 0x00000000 : 0xFFFFFFFF;
    return FPDFBitmap_FillRect(bitmap(), /*left=*/0, /*top=*/0,
                               /*width=*/width(),
                               /*height=*/height(), /*color=*/fill_color);
  }

  void ResetBitmap() { bitmap_.reset(); }

  void Idle() const { idler_(); }
  FPDF_BITMAP bitmap() { return bitmap_.get(); }

 private:
  const std::function<void()>& idler_;
  PageWriter writer_;
  ScopedFPDFBitmap bitmap_;
};

// Bitmap page renderer completing in a single operation.
class OneShotBitmapPageRenderer : public BitmapPageRenderer {
 public:
  OneShotBitmapPageRenderer(FPDF_PAGE page,
                            int width,
                            int height,
                            int flags,
                            const std::function<void()>& idler,
                            PageWriter writer)
      : BitmapPageRenderer(page,
                           /*width=*/width,
                           /*height=*/height,
                           /*flags=*/flags,
                           idler,
                           std::move(writer)) {}

  bool Start() override {
    if (!InitializeBitmap(/*first_scan=*/nullptr)) {
      return false;
    }

    // Note, client programs probably want to use this method instead of the
    // progressive calls. The progressive calls are if you need to pause the
    // rendering to update the UI, the PDF renderer will break when possible.
    FPDF_RenderPageBitmap(bitmap(), page(), /*start_x=*/0, /*start_y=*/0,
                          /*size_x=*/width(), /*size_y=*/height(), /*rotate=*/0,
                          /*flags=*/flags());
    return true;
  }
};

// Bitmap page renderer completing over multiple operations.
class ProgressiveBitmapPageRenderer : public BitmapPageRenderer {
 public:
  ProgressiveBitmapPageRenderer(FPDF_PAGE page,
                                int width,
                                int height,
                                int flags,
                                const std::function<void()>& idler,
                                PageWriter writer,
                                const FPDF_COLORSCHEME* color_scheme)
      : BitmapPageRenderer(page,
                           /*width=*/width,
                           /*height=*/height,
                           /*flags=*/flags,
                           idler,
                           std::move(writer)),
        color_scheme_(color_scheme) {
    pause_.version = 1;
    pause_.NeedToPauseNow = &NeedToPauseNow;
  }

  bool Start() override {
    if (!InitializeBitmap(/*first_scan=*/nullptr)) {
      return false;
    }

    if (FPDF_RenderPageBitmapWithColorScheme_Start(
            bitmap(), page(), /*start_x=*/0, /*start_y=*/0, /*size_x=*/width(),
            /*size_y=*/height(), /*rotate=*/0, /*flags=*/flags(), color_scheme_,
            &pause_) == FPDF_RENDER_TOBECONTINUED) {
      to_be_continued_ = true;
    }
    return true;
  }

  bool Continue() override {
    if (to_be_continued_) {
      to_be_continued_ = (FPDF_RenderPage_Continue(page(), &pause_) ==
                          FPDF_RENDER_TOBECONTINUED);
    }
    return to_be_continued_;
  }

  void Finish(FPDF_FORMHANDLE form) override {
    BitmapPageRenderer::Finish(form);
    FPDF_RenderPage_Close(page());
    Idle();
  }

 private:
  const FPDF_COLORSCHEME* color_scheme_;
  IFSDK_PAUSE pause_;
  bool to_be_continued_ = false;
};

#ifdef _WIN32
class ScopedGdiDc final {
 public:
  ~ScopedGdiDc() { Reset(nullptr); }

  void Reset(HDC dc) {
    if (dc_) {
      [[maybe_unused]] BOOL success = DeleteDC(dc_);
      DCHECK(success);
    }
    dc_ = dc;
  }

  HDC Get() const { return dc_; }

 private:
  HDC dc_ = nullptr;
};

class ScopedGdiObject final {
 public:
  ~ScopedGdiObject() { Reset(nullptr); }

  void Reset(HGDIOBJ object) {
    if (object_) {
      [[maybe_unused]] BOOL success = DeleteObject(object_);
      DCHECK(success);
    }
    object_ = object;
  }

  HGDIOBJ Get() const { return object_; }

 private:
  HGDIOBJ object_ = nullptr;
};

class GdiDisplayPageRenderer : public BitmapPageRenderer {
 public:
  GdiDisplayPageRenderer(FPDF_PAGE page,
                         int width,
                         int height,
                         int flags,
                         const std::function<void()>& idler,
                         PageWriter writer)
      : BitmapPageRenderer(page,
                           /*width=*/width,
                           /*height=*/height,
                           /*flags=*/flags,
                           idler,
                           std::move(writer)) {}

  ~GdiDisplayPageRenderer() override {
    // Need to free `bitmap()` first, in case it points at `dib_` memory.
    ResetBitmap();
  }

  bool Start() override {
    // Create an in-memory DC compatible with the display.
    dc_.Reset(CreateCompatibleDC(/*hdc=*/nullptr));
    if (!dc_.Get()) {
      return false;
    }

    // Create a BGRA DIB and select it into the in-memory DC.
    BITMAPINFO dib_info;
    memset(&dib_info, 0, sizeof(BITMAPINFO));
    dib_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    dib_info.bmiHeader.biWidth = width();
    dib_info.bmiHeader.biHeight = -height();  // top-down
    dib_info.bmiHeader.biPlanes = 1;
    dib_info.bmiHeader.biBitCount = 32;
    dib_info.bmiHeader.biCompression = BI_RGB;

    VOID* dib_pixels;
    dib_.Reset(CreateDIBSection(dc_.Get(), &dib_info, DIB_RGB_COLORS,
                                &dib_pixels, /*hSection=*/nullptr,
                                /*offset=*/0));
    if (!dib_.Get() || !InitializeBitmap(dib_pixels)) {
      return false;
    }

    HGDIOBJ old_obj = SelectObject(dc_.Get(), dib_.Get());
    CHECK(old_obj);
    CHECK_NE(old_obj, HGDI_ERROR);

    // Render into the in-memory DC.
    FPDF_BOOL render_result =
        FPDF_RenderPage(dc_.Get(), page(), /*start_x=*/0, /*start_y=*/0,
                        /*size_x=*/width(), /*size_y=*/height(), /*rotate=*/0,
                        /*flags=*/flags());
    if (!render_result) {
      return false;
    }

    bool result = !!GdiFlush();
    HGDIOBJ dib_obj = SelectObject(dc_.Get(), old_obj);
    CHECK((GetObjectType(old_obj) != OBJ_REGION && dib_obj) ||
          (GetObjectType(old_obj) == OBJ_REGION && dib_obj != HGDI_ERROR));
    return result;
  }

  void Finish(FPDF_FORMHANDLE /*form*/) override {
    // Note that `fpdf_formfill.h` does not support GDI.

    // The GDI backend doesn't support alpha and clears the alpha component to
    // transparent, so clear the alpha component back to opaque.
    const int stride = FPDFBitmap_GetStride(bitmap());
    DCHECK_EQ(width() * sizeof(uint32_t), static_cast<size_t>(stride));
    const int pixel_stride = stride / sizeof(uint32_t);

    uint32_t* scanline =
        reinterpret_cast<uint32_t*>(FPDFBitmap_GetBuffer(bitmap()));
    for (int row = 0; row < height(); ++row) {
      for (int column = 0; column < width(); ++column) {
        scanline[column] |= 0xFF000000;
      }
      scanline += pixel_stride;
    }
  }

 private:
  ScopedGdiDc dc_;
  ScopedGdiObject dib_;
};
#endif  // _WIN32

#ifdef PDF_ENABLE_SKIA
class SkCanvasPageRenderer : public PageRenderer {
 public:
  bool Start() override {
    FPDF_RenderPageSkia(reinterpret_cast<FPDF_SKIA_CANVAS>(canvas()), page(),
                        width(), height());
    return true;
  }

  void Finish(FPDF_FORMHANDLE form) override {
    FPDF_FFLDrawSkia(form, reinterpret_cast<FPDF_SKIA_CANVAS>(canvas()), page(),
                     /*start_x=*/0, /*start_y=*/0, width(), height(),
                     /*rotate=*/0, flags());
  }

 protected:
  SkCanvasPageRenderer(FPDF_PAGE page, int width, int height, int flags)
      : PageRenderer(page, width, height, flags) {}

  virtual SkCanvas* canvas() = 0;
};

class SkPicturePageRenderer final : public SkCanvasPageRenderer {
 public:
  SkPicturePageRenderer(FPDF_PAGE page, int width, int height, int flags)
      : SkCanvasPageRenderer(page, width, height, flags) {}

  bool HasOutput() const override { return !!picture_; }

  bool Start() override {
    recorder_ = std::make_unique<SkPictureRecorder>();
    recorder_->beginRecording(width(), height());
    return SkCanvasPageRenderer::Start();
  }

  void Finish(FPDF_FORMHANDLE form) override {
    SkCanvasPageRenderer::Finish(form);
    picture_ = recorder_->finishRecordingAsPicture();
    recorder_.reset();
  }

  bool Write(const std::string& name, int page_index, bool md5) override {
    std::string image_file_name = WriteSkp(name.c_str(), page_index, *picture_);
    if (image_file_name.empty()) {
      return false;
    }

    if (md5) {
      // Play back the `SkPicture` so we can take a hash of the result.
      sk_sp<SkSurface> surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(
          /*width=*/width(), /*height=*/height()));
      if (!surface) {
        return false;
      }

      // Must clear to white before replay to match initial `CFX_DIBitmap`.
      surface->getCanvas()->clear(SK_ColorWHITE);
      surface->getCanvas()->drawPicture(picture_);

      // Write the filename and the MD5 of the buffer to stdout.
      SkPixmap pixmap;
      if (!surface->peekPixels(&pixmap)) {
        return false;
      }

      OutputMD5Hash(
          image_file_name.c_str(),
          UNSAFE_TODO(pdfium::span(static_cast<const uint8_t*>(pixmap.addr()),
                                   pixmap.computeByteSize())));
    }
    return true;
  }

 protected:
  SkCanvas* canvas() override { return recorder_->getRecordingCanvas(); }

 private:
  std::unique_ptr<SkPictureRecorder> recorder_;
  sk_sp<SkPicture> picture_;
};

class SkDocumentPageRenderer final : public SkCanvasPageRenderer {
 public:
  SkDocumentPageRenderer(std::unique_ptr<SkWStream> stream,
                         sk_sp<SkDocument> document,
                         FPDF_PAGE page,
                         int width,
                         int height,
                         int flags)
      : SkCanvasPageRenderer(page, width, height, flags),
        stream_(std::move(stream)),
        document_(std::move(document)) {
    DCHECK(stream_);
    DCHECK(document_);
  }

  bool HasOutput() const override { return has_output_; }

  bool Start() override {
    if (!document_) {
      return false;
    }

    DCHECK(!canvas_);
    canvas_ = document_->beginPage(width(), height());
    if (!canvas_) {
      return false;
    }

    return SkCanvasPageRenderer::Start();
  }

  void Finish(FPDF_FORMHANDLE form) override {
    SkCanvasPageRenderer::Finish(form);

    DCHECK(canvas_);
    canvas_ = nullptr;
    document_->endPage();

    has_output_ = true;
  }

  bool Write(const std::string& /*name*/,
             int /*page_index*/,
             bool /*md5*/) override {
    bool success = HasOutput();
    if (success) {
      document_->close();
    } else {
      document_->abort();
    }

    document_.reset();
    stream_.reset();
    return success;
  }

 protected:
  SkCanvas* canvas() override { return canvas_; }

 private:
  std::unique_ptr<SkWStream> stream_;
  sk_sp<SkDocument> document_;

  SkCanvas* canvas_ = nullptr;
  bool has_output_ = false;
};
#endif  // PDF_ENABLE_SKIA

bool PdfProcessor::ProcessPage(const int page_index) {
  FPDF_PAGE page = GetPage(page_index);
  if (!page) {
    return false;
  }

  if (options().send_events) {
    SendPageEvents(form(), page, events(), idler());
  }
  if (options().save_images) {
    WriteImages(page, name().c_str(), page_index);
  }
  if (options().save_rendered_images) {
    WriteRenderedImages(doc(), page, name().c_str(), page_index);
  }
  if (options().save_thumbnails) {
    WriteThumbnail(page, name().c_str(), page_index);
  }
  if (options().save_thumbnails_decoded) {
    WriteDecodedThumbnailStream(page, name().c_str(), page_index);
  }
  if (options().save_thumbnails_raw) {
    WriteRawThumbnailStream(page, name().c_str(), page_index);
  }
  if (options().output_format == OutputFormat::kPageInfo) {
    DumpPageInfo(page, page_index);
    return true;
  }
  if (options().output_format == OutputFormat::kStructure) {
    DumpPageStructure(page, page_index);
    return true;
  }

  ScopedFPDFTextPage text_page(FPDFText_LoadPage(page));
  double scale = 1.0;
  if (!options().scale_factor_as_string.empty()) {
    std::stringstream(options().scale_factor_as_string) >> scale;
  }

  int width = static_cast<int>(FPDF_GetPageWidthF(page) * scale);
  int height = static_cast<int>(FPDF_GetPageHeightF(page) * scale);
  int flags = PageRenderFlagsFromOptions(options());

  std::unique_ptr<PageRenderer> renderer;
  BitmapPageRenderer::PageWriter writer;
  switch (options().output_format) {
    case OutputFormat::kText:
      writer = BitmapPageRenderer::WrapPageWriter(WriteText);
      break;

    case OutputFormat::kAnnot:
      writer = BitmapPageRenderer::WrapPageWriter(WriteAnnot);
      break;

    case OutputFormat::kPpm:
      writer = BitmapPageRenderer::WrapPageWriter(WritePpm);
      break;

    case OutputFormat::kPng:
      writer = BitmapPageRenderer::WrapPageWriter(WritePng);
      break;

#ifdef _WIN32
    case OutputFormat::kBmp:
      writer = BitmapPageRenderer::WrapPageWriter(WriteBmp);
      break;

    case OutputFormat::kEmf:
      // TODO(crbug.com/pdfium/2054): Render directly to DC.
      writer = BitmapPageRenderer::WrapPageWriter(WriteEmf);
      break;

    case OutputFormat::kPs2:
    case OutputFormat::kPs3:
      // TODO(crbug.com/pdfium/2054): Render directly to DC.
      writer = BitmapPageRenderer::WrapPageWriter(WritePS);
      break;
#endif  // _WIN32

#ifdef PDF_ENABLE_SKIA
    case OutputFormat::kSkp:
      renderer = std::make_unique<SkPicturePageRenderer>(
          page, /*width=*/width, /*height=*/height, /*flags=*/flags);
      break;

#ifdef _WIN32
    case OutputFormat::kXps: {
      IXpsOMObjectFactory* xps_factory = com_factory().GetXpsOMObjectFactory();
      if (!xps_factory) {
        break;
      }

      std::unique_ptr<SkWStream> stream =
          WriteToSkWStream(name(), page_index, "xps");
      if (!stream) {
        break;
      }

      SkXPS::Options xps_options;
      xps_options.pngEncoder = [](SkWStream* dst, const SkPixmap& src) {
#ifdef PDF_ENABLE_RUST_PNG
        return SkPngRustEncoder::Encode(dst, src, {});
#else
        return SkPngEncoder::Encode(dst, src, {});
#endif
      };
      sk_sp<SkDocument> document =
          SkXPS::MakeDocument(stream.get(), xps_factory, xps_options);
      if (!document) {
        break;
      }

      renderer = std::make_unique<SkDocumentPageRenderer>(
          std::move(stream), std::move(document), page, width, height, flags);
      break;
    }
#endif  // _WIN32
#endif  // PDF_ENABLE_SKIA

    default:
      // Other formats won't write the output to a file, but still rasterize.
      break;
  }

#ifdef _WIN32
  if (!renderer && options().use_renderer_type == RendererType::kGdi) {
    renderer = std::make_unique<GdiDisplayPageRenderer>(
        page, /*width=*/width, /*height=*/height, /*flags=*/flags, idler(),
        std::move(writer));
  }
#endif  // _WIN32

  // Client programs will be setting these values when rendering.
  // This is a sample color scheme with distinct colors.
  // Used only when `options().forced_color` is true.
  static constexpr FPDF_COLORSCHEME kColorScheme = {
      .path_fill_color = 0xFFFF0000,
      .path_stroke_color = 0xFF00FF00,
      .text_fill_color = 0xFF0000FF,
      .text_stroke_color = 0xFF00FFFF};

  if (!renderer) {
    // Use a rasterizing page renderer by default.
    if (options().render_oneshot) {
      renderer = std::make_unique<OneShotBitmapPageRenderer>(
          page, /*width=*/width, /*height=*/height, /*flags=*/flags, idler(),
          std::move(writer));
    } else {
      renderer = std::make_unique<ProgressiveBitmapPageRenderer>(
          page, /*width=*/width, /*height=*/height, /*flags=*/flags, idler(),
          std::move(writer), options().forced_color ? &kColorScheme : nullptr);
    }
  }

  if (renderer->Start()) {
    while (renderer->Continue()) {
      continue;
    }
    renderer->Finish(form());
    renderer->Write(name(), page_index, /*md5=*/options().md5);
  } else {
    fprintf(stderr, "Page was too large to be rendered.\n");
  }

  FORM_DoPageAAction(page, form(), FPDFPAGE_AACTION_CLOSE);
  Idle();

  FORM_OnBeforeClosePage(page, form());
  Idle();

  return renderer->HasOutput();
}

void Processor::ProcessPdf(const std::string& name,
                           pdfium::span<const uint8_t> data,
                           const std::string& events) {
  TestLoader loader(data);

  FPDF_FILEACCESS file_access = {};
  file_access.m_FileLen = static_cast<unsigned long>(data.size());
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
      options().password.empty() ? nullptr : options().password.c_str();
  bool is_linearized = false;
  if (options().use_load_mem_document) {
    doc.reset(FPDF_LoadMemDocument(data.data(), data.size(), password));
  } else {
    if (FPDFAvail_IsLinearized(pdf_avail.get()) == PDF_LINEARIZED) {
      int avail_status = PDF_DATA_NOTAVAIL;
      doc.reset(FPDFAvail_GetDocument(pdf_avail.get(), password));
      if (doc) {
        while (avail_status == PDF_DATA_NOTAVAIL) {
          avail_status = FPDFAvail_IsDocAvail(pdf_avail.get(), &hints);
        }

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

  if (!FPDF_DocumentHasValidCrossReferenceTable(doc.get())) {
    fprintf(stderr, "Document has invalid cross reference table\n");
  }

  if (options().show_metadata) {
    DumpMetaData(doc.get());
  }

  if (options().save_attachments) {
    WriteAttachments(doc.get(), name);
  }

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
      options().disable_xfa || options().disable_javascript;
  form_callbacks.FFI_PopupMenu = ExamplePopupMenu;
#else   // PDF_ENABLE_XFA
  form_callbacks.version = 1;
#endif  // PDF_ENABLE_XFA
  form_callbacks.FFI_ExecuteNamedAction = ExampleNamedAction;
  form_callbacks.FFI_GetPage = GetPageForIndex;

#ifdef PDF_ENABLE_V8
  if (!options().disable_javascript) {
    form_callbacks.m_pJsPlatform = &platform_callbacks;
  }
#endif  // PDF_ENABLE_V8

  ScopedFPDFFormHandle form(
      FPDFDOC_InitFormFillEnvironment(doc.get(), &form_callbacks));
  form_callbacks.form_handle = form.get();

#ifdef PDF_ENABLE_XFA
  if (!options().disable_xfa && !options().disable_javascript) {
    int doc_type = FPDF_GetFormType(doc.get());
    if (doc_type == FORMTYPE_XFA_FULL || doc_type == FORMTYPE_XFA_FOREGROUND) {
      if (!FPDF_LoadXFA(doc.get())) {
        fprintf(stderr, "LoadXFA unsuccessful, continuing anyway.\n");
      }
    }
  }
#endif  // PDF_ENABLE_XFA

  FPDF_SetFormFieldHighlightColor(form.get(), FPDF_FORMFIELD_UNKNOWN, 0xFFE4DD);
  FPDF_SetFormFieldHighlightAlpha(form.get(), 100);
  FORM_DoDocumentJSAction(form.get());
  FORM_DoDocumentOpenAction(form.get());

#if _WIN32
  if (options().output_format == OutputFormat::kPs2) {
    FPDF_SetPrintMode(FPDF_PRINTMODE_POSTSCRIPT2);
  } else if (options().output_format == OutputFormat::kPs3) {
    FPDF_SetPrintMode(FPDF_PRINTMODE_POSTSCRIPT3);
  } else if (options().output_format == OutputFormat::kPs3Type42) {
    FPDF_SetPrintMode(FPDF_PRINTMODE_POSTSCRIPT3_TYPE42);
  }
#endif

  int render_repeats = 1;
  if (!options().render_repeats_as_string.empty()) {
    std::stringstream(options().render_repeats_as_string) >> render_repeats;
  }

  int page_count = FPDF_GetPageCount(doc.get());
  int processed_pages = 0;
  int bad_pages = 0;
  int first_page = options().pages ? options().first_page : 0;
  int last_page = options().pages ? options().last_page + 1 : page_count;
  PdfProcessor pdf_processor(this, &name, &events, doc.get(), form.get(),
                             &form_callbacks);

  for (int repetition = 0; repetition < render_repeats; ++repetition) {
    for (int i = first_page; i < last_page; ++i) {
      if (is_linearized) {
        int avail_status = PDF_DATA_NOTAVAIL;
        while (avail_status == PDF_DATA_NOTAVAIL) {
          avail_status = FPDFAvail_IsPageAvail(pdf_avail.get(), i, &hints);
        }

        if (avail_status == PDF_DATA_ERROR) {
          fprintf(stderr,
                  "Unknown error in checking if page %d is available.\n", i);
          return;
        }
      }
      if (pdf_processor.ProcessPage(i)) {
        ++processed_pages;
      } else {
        ++bad_pages;
      }
      Idle();
    }
  }

  FORM_DoDocumentAAction(form.get(), FPDFDOC_AACTION_WC);
  Idle();

  fprintf(stderr, "Processed %d pages.\n", processed_pages);
  if (bad_pages) {
    fprintf(stderr, "Skipped %d bad pages.\n", bad_pages);
  }
}

void ShowConfig() {
  std::string config;
  [[maybe_unused]] auto append_config = [&config](const char* name) {
    if (!config.empty()) {
      config += ',';
    }
    config += name;
  };

#ifdef PDF_ENABLE_V8
  append_config("V8");
#endif
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
  append_config("V8_EXTERNAL");
#endif
#ifdef PDF_ENABLE_XFA
  append_config("XFA");
#endif
#ifdef PDF_ENABLE_ASAN
  append_config("ASAN");
#endif
#ifdef PDF_ENABLE_SKIA
  append_config("SKIA");
#endif
#ifdef _WIN32
  append_config("GDI");
#endif
  printf("%s\n", config.c_str());
}

constexpr char kUsageString[] =
    "Usage: pdfium_test [OPTION] [FILE]...\n"
    "  --show-config          - print build options and exit\n"
    "  --show-metadata        - print the file metadata\n"
    "  --show-pageinfo        - print information about pages\n"
    "  --show-structure       - print the structure elements from the "
    "document\n"
    "  --send-events          - send input described by .evt file\n"
    "  --mem-document         - load document with FPDF_LoadMemDocument()\n"
    "  --render-oneshot       - render image without using progressive "
    "renderer\n"
    "  --render-repeats=<n>   - render PDF n times; useful for benchmarking\n"
    "  --lcd-text             - render text optimized for LCD displays\n"
    "  --no-nativetext        - render without using the native text output\n"
    "  --grayscale            - render grayscale output\n"
    "  --forced-color         - render in forced color mode\n"
    "  --fill-to-stroke       - render fill as stroke in forced color mode\n"
    "  --limit-cache          - render limiting image cache size\n"
    "  --force-halftone       - render forcing halftone\n"
    "  --printing             - render as if for printing\n"
    "  --no-smoothtext        - render disabling text anti-aliasing\n"
    "  --no-smoothimage       - render disabling image anti-alisasing\n"
    "  --no-smoothpath        - render disabling path anti-aliasing\n"
    "  --reverse-byte-order   - render to BGRA, if supported by the output "
    "format\n"
    "  --save-attachments     - write embedded attachments "
    "<pdf-name>.attachment.<attachment-name>\n"
    "  --save-images          - write raw embedded images "
    "<pdf-name>.<page-number>.<object-number>.png\n"
    "  --save-rendered-images - write embedded images as rendered on the page "
    "<pdf-name>.<page-number>.<object-number>.png\n"
    "  --save-thumbs          - write page thumbnails "
    "<pdf-name>.thumbnail.<page-number>.png\n"
    "  --save-thumbs-dec      - write page thumbnails' decoded stream data"
    "<pdf-name>.thumbnail.decoded.<page-number>.png\n"
    "  --save-thumbs-raw      - write page thumbnails' raw stream data"
    "<pdf-name>.thumbnail.raw.<page-number>.png\n"

#if defined(PDF_ENABLE_SKIA)
#ifdef _WIN32
    "  --use-renderer         - renderer to use, one of [agg | gdi | skia]\n"
#else
    "  --use-renderer         - renderer to use, one of [agg | skia]\n"
#endif  // _WIN32
#else
#ifdef _WIN32
    "  --use-renderer         - renderer to use, one of [agg | gdi]\n"
#else
    "  --use-renderer         - renderer to use, one of [agg]\n"
#endif  // _WIN32
#endif  // defined(PDF_ENABLE_SKIA)

#ifdef PDF_ENABLE_V8
    "  --disable-javascript   - do not execute JS in PDF files\n"
    "  --js-flags=<flags>     - additional flags to pass to V8\n"
#ifdef PDF_ENABLE_XFA
    "  --disable-xfa          - do not process XFA forms\n"
#endif  // PDF_ENABLE_XFA
#endif  // PDF_ENABLE_V8
#ifdef ENABLE_CALLGRIND
    "  --callgrind-delim      - delimit interesting section when using "
    "callgrind\n"
#endif
#if defined(__APPLE__) || (defined(__linux__) && !defined(__ANDROID__))
    "  --no-system-fonts      - do not use system fonts, overrides --font-dir\n"
#endif
    "  --croscore-font-names  - use Croscore font names\n"
    "  --bin-dir=<path>       - override path to v8 external data\n"
    "  --font-dir=<path>      - override path to external fonts\n"
    "  --scale=<number>       - scale output size by number (e.g. 0.5)\n"
    "  --password=<secret>    - password to decrypt the PDF with\n"
    "  --pages=<number>(-<number>) - only render the given 0-based page(s)\n"
#ifdef _WIN32
    "  --bmp   - write page images <pdf-name>.<page-number>.bmp\n"
    "  --emf   - write page meta files <pdf-name>.<page-number>.emf\n"
    "  --ps2   - write page raw PostScript (Lvl 2) "
    "<pdf-name>.<page-number>.ps\n"
    "  --ps3   - write page raw PostScript (Lvl 3) "
    "<pdf-name>.<page-number>.ps\n"
    "  --ps3-type42 - write page raw PostScript (Lvl 3 with Type 42 fonts) "
    "<pdf-name>.<page-number>.ps\n"
#endif
    "  --txt   - write page text in UTF32-LE <pdf-name>.<page-number>.txt\n"
    "  --png   - write page images <pdf-name>.<page-number>.png\n"
    "  --ppm   - write page images <pdf-name>.<page-number>.ppm\n"
    "  --annot - write annotation info <pdf-name>.<page-number>.annot.txt\n"
#ifdef PDF_ENABLE_SKIA
    "  --skp   - write page images <pdf-name>.<page-number>.skp\n"
#ifdef _WIN32
    "  --xps   - write page images <pdf-name>.<page-number>.xps\n"
#endif  // _WIN32
#endif  // PDF_ENABLE_SKIA
    "  --md5   - write output image paths and their md5 hashes to stdout.\n"
    "  --time=<number> - Seconds since the epoch to set system time.\n"
    "";

void SetUpErrorHandling() {
#ifdef _WIN32
  // Suppress various Windows error reporting mechanisms that can pop up dialog
  // boxes and cause the program to hang.
  SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOALIGNMENTFAULTEXCEPT |
               SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
  _set_error_mode(_OUT_TO_STDERR);
  _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
  _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
  _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif  // _WIN32
}

}  // namespace

int main(int argc, const char* argv[]) {
#if defined(PDF_USE_PARTITION_ALLOC)
  pdfium::ConfigurePartitionAllocShimPartitionForTest();
#endif

  SetUpErrorHandling();
  setlocale(LC_CTYPE, "en_US.UTF-8");  // For printf() of high-characters.

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

  FPDF_LIBRARY_CONFIG config;
  config.version = 4;
  config.m_pUserFontPaths = nullptr;
  config.m_pIsolate = nullptr;
  config.m_v8EmbedderSlot = 0;
  config.m_pPlatform = nullptr;

  switch (options.use_renderer_type) {
    case RendererType::kDefault:
      config.m_RendererType = GetDefaultRendererType();
      break;

    case RendererType::kAgg:
      config.m_RendererType = FPDF_RENDERERTYPE_AGG;
      break;

#ifdef _WIN32
    case RendererType::kGdi:
      // GDI renderer uses `FPDF_RenderPage()`, rather than a renderer type.
      config.m_RendererType = GetDefaultRendererType();
      break;
#endif  // _WIN32

#if defined(PDF_ENABLE_SKIA)
    case RendererType::kSkia:
      config.m_RendererType = FPDF_RENDERERTYPE_SKIA;
      break;
#endif  // defined(PDF_ENABLE_SKIA)
  }

#if defined(PDF_ENABLE_SKIA) && defined(BUILD_WITH_CHROMIUM)
  // Needed to support Chromium's copy of Skia, which uses a
  // `DiscardableMemoryAllocator`.
  if (config.m_RendererType == FPDF_RENDERERTYPE_SKIA) {
    chromium_support::InitializeDiscardableMemoryAllocator();
  }
#endif

  std::function<void()> idler = []() {};
#ifdef PDF_ENABLE_V8
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
  v8::StartupData snapshot;
#endif  // V8_USE_EXTERNAL_STARTUP_DATA
  std::unique_ptr<v8::Platform> platform;
  std::unique_ptr<v8::Isolate, V8IsolateDeleter> isolate;
  if (!options.disable_javascript) {
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
    platform = InitializeV8ForPDFiumWithStartupData(
        options.exe_path, options.js_flags, options.bin_directory, &snapshot);
#else   // V8_USE_EXTERNAL_STARTUP_DATA
    platform = InitializeV8ForPDFium(options.exe_path, options.js_flags);
#endif  // V8_USE_EXTERNAL_STARTUP_DATA
    if (!platform) {
      fprintf(stderr, "V8 initialization failed.\n");
      return 1;
    }
    config.m_pPlatform = platform.get();

    v8::Isolate::CreateParams params;
    params.array_buffer_allocator = static_cast<v8::ArrayBuffer::Allocator*>(
        FPDF_GetArrayBufferAllocatorSharedInstance());
    isolate.reset(v8::Isolate::New(params));
    config.m_pIsolate = isolate.get();

    idler = [&platform, &isolate]() {
      v8::Isolate::Scope isolate_scope(isolate.get());
      int task_count = 0;
      while (v8::platform::PumpMessageLoop(platform.get(), isolate.get())) {
        ++task_count;
      }
      if (task_count) {
        fprintf(stderr, "Pumped %d tasks\n", task_count);
      }
    };
  }
#endif  // PDF_ENABLE_V8

  const char* path_array[2] = {nullptr, nullptr};
  std::optional<const char*> custom_font_path = GetCustomFontPath(options);
  if (custom_font_path.has_value()) {
    path_array[0] = custom_font_path.value();
    config.m_pUserFontPaths = path_array;
  }

  FPDF_InitLibraryWithConfig(&config);

  {
    std::unique_ptr<FontRenamer> font_renamer;
    if (options.croscore_font_names) {
      // Must be destroyed before FPDF_DestroyLibrary().
      font_renamer = std::make_unique<FontRenamer>();
    }

    UNSUPPORT_INFO unsupported_info = {};
    unsupported_info.version = 1;
    unsupported_info.FSDK_UnSupport_Handler = ExampleUnsupportedHandler;

    FSDK_SetUnSpObjProcessHandler(&unsupported_info);

    if (options.time > -1) {
      // This must be a static var to avoid explicit capture, so the lambda can
      // be converted to a function ptr.
      static time_t time_ret = options.time;
      FSDK_SetTimeFunction([]() { return time_ret; });
      FSDK_SetLocaltimeFunction([](const time_t* tp) { return gmtime(tp); });
    }

    Processor processor(&options, &idler);
    for (const std::string& filename : files) {
      std::vector<uint8_t> file_contents = GetFileContents(filename.c_str());
      if (file_contents.empty()) {
        continue;
      }
      fprintf(stderr, "Processing PDF file %s.\n", filename.c_str());

#ifdef ENABLE_CALLGRIND
      if (options.callgrind_delimiters) {
        CALLGRIND_START_INSTRUMENTATION;
      }
#endif  // ENABLE_CALLGRIND

      std::string events;
      if (options.send_events) {
        std::string event_filename = filename;
        size_t extension_pos = event_filename.find(".pdf");
        if (extension_pos != std::string::npos) {
          event_filename.replace(extension_pos, 4, ".evt");
          if (access(event_filename.c_str(), R_OK) == 0) {
            fprintf(stderr, "Using event file %s.\n", event_filename.c_str());
            std::vector<uint8_t> event_contents =
                GetFileContents(event_filename.c_str());
            if (!event_contents.empty()) {
              fprintf(stderr, "Sending events from: %s\n",
                      event_filename.c_str());
              std::copy(event_contents.begin(), event_contents.end(),
                        std::back_inserter(events));
            }
          }
        }
      }

      processor.ProcessPdf(filename, file_contents, events);

#ifdef ENABLE_CALLGRIND
      if (options.callgrind_delimiters) {
        CALLGRIND_STOP_INSTRUMENTATION;
      }
#endif  // ENABLE_CALLGRIND
    }
  }

  FPDF_DestroyLibrary();

#ifdef PDF_ENABLE_V8
  if (!options.disable_javascript) {
    isolate.reset();
    ShutdownV8ForPDFium();
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
    free(const_cast<char*>(snapshot.data));
#endif  // V8_USE_EXTERNAL_STARTUP_DATA
  }
#endif  // PDF_ENABLE_V8

  return 0;
}
