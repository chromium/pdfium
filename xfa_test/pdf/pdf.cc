// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pdf/pdf.h"

#if defined(OS_WIN)
#include <windows.h>
#endif

#include "base/command_line.h"
#include "base/logging.h"
#include "pdf/instance.h"
#include "pdf/out_of_process_instance.h"
#include "ppapi/c/ppp.h"
#include "ppapi/cpp/private/pdf.h"

bool g_sdk_initialized_via_pepper = false;

// The Mac release builds discard CreateModule and the entire PDFModule
// definition because they are not referenced here. This causes the Pepper
// exports (PPP_GetInterface etc) to not be exported. So we force the linker
// to include this code by using __attribute__((used)).
#if __GNUC__ >= 4
#define PDF_USED __attribute__((used))
#else
#define PDF_USED
#endif

#if defined(OS_WIN)
HMODULE g_hmodule;

void HandleInvalidParameter(const wchar_t* expression,
                            const wchar_t* function,
                            const wchar_t* file,
                            unsigned int line,
                            uintptr_t reserved) {
  // Do the same as Chrome's CHECK(false) which is undefined.
  ::base::debug::BreakDebugger();
  return;
}

void HandlePureVirtualCall() {
  // Do the same as Chrome's CHECK(false) which is undefined.
  ::base::debug::BreakDebugger();
  return;
}


BOOL APIENTRY DllMain(HMODULE module, DWORD reason_for_call, LPVOID reserved) {
  g_hmodule = module;
  if (reason_for_call == DLL_PROCESS_ATTACH) {
    // On windows following handlers work only inside module. So breakpad in
    // chrome.dll does not catch that. To avoid linking related code or
    // duplication breakpad_win.cc::InitCrashReporter() just catch errors here
    // and crash in a way interceptable by breakpad of parent module.
    _set_invalid_parameter_handler(HandleInvalidParameter);
    _set_purecall_handler(HandlePureVirtualCall);
  }
  return TRUE;
}

#endif

namespace pp {

PDF_USED Module* CreateModule() {
  return new chrome_pdf::PDFModule();
}

}  // namespace pp

namespace chrome_pdf {

PDFModule::PDFModule() {
}

PDFModule::~PDFModule() {
  if (g_sdk_initialized_via_pepper) {
    chrome_pdf::ShutdownSDK();
    g_sdk_initialized_via_pepper = false;
  }
}

bool PDFModule::Init() {
  return true;
}

pp::Instance* PDFModule::CreateInstance(PP_Instance instance) {
  if (!g_sdk_initialized_via_pepper) {
    void* data = NULL;
#if defined(OS_WIN)
    data = g_hmodule;
#endif
    if (!chrome_pdf::InitializeSDK(data))
      return NULL;
    g_sdk_initialized_via_pepper = true;
  }

  if (pp::PDF::IsOutOfProcess(pp::InstanceHandle(instance)))
    return new OutOfProcessInstance(instance);
  return new Instance(instance);
}

}  // namespace chrome_pdf

extern "C" {

// TODO(sanjeevr): It might make sense to provide more stateful wrappers over
// the internal PDF SDK (such as LoadDocument, LoadPage etc). Determine if we
// need to provide this.
// Wrapper exports over the PDF engine that can be used by an external module
// such as Chrome (since Chrome cannot directly pull in PDFium sources).
#if defined(OS_WIN)
// |pdf_buffer| is the buffer that contains the entire PDF document to be
//     rendered.
// |buffer_size| is the size of |pdf_buffer| in bytes.
// |page_number| is the 0-based index of the page to be rendered.
// |dc| is the device context to render into.
// |dpi_x| and |dpi_y| are the x and y resolutions respectively. If either
//     value is -1, the dpi from the DC will be used.
// |bounds_origin_x|, |bounds_origin_y|, |bounds_width| and |bounds_height|
//     specify a bounds rectangle within the DC in which to render the PDF
//     page.
// |fit_to_bounds| specifies whether the output should be shrunk to fit the
//     supplied bounds if the page size is larger than the bounds in any
//     dimension. If this is false, parts of the PDF page that lie outside
//     the bounds will be clipped.
// |stretch_to_bounds| specifies whether the output should be stretched to fit
//     the supplied bounds if the page size is smaller than the bounds in any
//     dimension.
// If both |fit_to_bounds| and |stretch_to_bounds| are true, then
//     |fit_to_bounds| is honored first.
// |keep_aspect_ratio| If any scaling is to be done is true, this flag
//     specifies whether the original aspect ratio of the page should be
//     preserved while scaling.
// |center_in_bounds| specifies whether the final image (after any scaling is
//     done) should be centered within the given bounds.
// |autorotate| specifies whether the final image should be rotated to match
//     the output bound.
// Returns false if the document or the page number are not valid.
PP_EXPORT bool RenderPDFPageToDC(const void* pdf_buffer,
                                 int buffer_size,
                                 int page_number,
                                 HDC dc,
                                 int dpi_x,
                                 int dpi_y,
                                 int bounds_origin_x,
                                 int bounds_origin_y,
                                 int bounds_width,
                                 int bounds_height,
                                 bool fit_to_bounds,
                                 bool stretch_to_bounds,
                                 bool keep_aspect_ratio,
                                 bool center_in_bounds,
                                 bool autorotate) {
  if (!g_sdk_initialized_via_pepper) {
    if (!chrome_pdf::InitializeSDK(g_hmodule)) {
      return false;
    }
  }
  scoped_ptr<chrome_pdf::PDFEngineExports> engine_exports(
      chrome_pdf::PDFEngineExports::Create());
  chrome_pdf::PDFEngineExports::RenderingSettings settings(
      dpi_x, dpi_y, pp::Rect(bounds_origin_x, bounds_origin_y, bounds_width,
                             bounds_height),
      fit_to_bounds, stretch_to_bounds, keep_aspect_ratio, center_in_bounds,
      autorotate);
  bool ret = engine_exports->RenderPDFPageToDC(pdf_buffer, buffer_size,
                                               page_number, settings, dc);
  if (!g_sdk_initialized_via_pepper) {
    chrome_pdf::ShutdownSDK();
  }
  return ret;
}

#endif  // OS_WIN

// |page_count| and |max_page_width| are optional and can be NULL.
// Returns false if the document is not valid.
PDF_USED PP_EXPORT
bool GetPDFDocInfo(const void* pdf_buffer,
                   int buffer_size, int* page_count,
                   double* max_page_width) {
  if (!g_sdk_initialized_via_pepper) {
    void* data = NULL;
#if defined(OS_WIN)
    data = g_hmodule;
#endif
    if (!chrome_pdf::InitializeSDK(data))
      return false;
  }
  scoped_ptr<chrome_pdf::PDFEngineExports> engine_exports(
      chrome_pdf::PDFEngineExports::Create());
  bool ret = engine_exports->GetPDFDocInfo(
      pdf_buffer, buffer_size, page_count, max_page_width);
  if (!g_sdk_initialized_via_pepper) {
    chrome_pdf::ShutdownSDK();
  }
  return ret;
}

// Gets the dimensions of a specific page in a document.
// |pdf_buffer| is the buffer that contains the entire PDF document to be
//     rendered.
// |pdf_buffer_size| is the size of |pdf_buffer| in bytes.
// |page_number| is the page number that the function will get the dimensions
//     of.
// |width| is the output for the width of the page in points.
// |height| is the output for the height of the page in points.
// Returns false if the document or the page number are not valid.
PDF_USED PP_EXPORT
bool GetPDFPageSizeByIndex(const void* pdf_buffer,
                           int pdf_buffer_size, int page_number,
                           double* width, double* height) {
  if (!g_sdk_initialized_via_pepper) {
    void* data = NULL;
#if defined(OS_WIN)
    data = g_hmodule;
#endif
    if (!chrome_pdf::InitializeSDK(data))
      return false;
  }
  scoped_ptr<chrome_pdf::PDFEngineExports> engine_exports(
      chrome_pdf::PDFEngineExports::Create());
  bool ret = engine_exports->GetPDFPageSizeByIndex(
      pdf_buffer, pdf_buffer_size, page_number, width, height);
  if (!g_sdk_initialized_via_pepper)
    chrome_pdf::ShutdownSDK();
  return ret;
}

// Renders PDF page into 4-byte per pixel BGRA color bitmap.
// |pdf_buffer| is the buffer that contains the entire PDF document to be
//     rendered.
// |pdf_buffer_size| is the size of |pdf_buffer| in bytes.
// |page_number| is the 0-based index of the page to be rendered.
// |bitmap_buffer| is the output buffer for bitmap.
// |bitmap_width| is the width of the output bitmap.
// |bitmap_height| is the height of the output bitmap.
// |dpi| is the resolutions.
// |autorotate| specifies whether the final image should be rotated to match
//     the output bound.
// Returns false if the document or the page number are not valid.
PDF_USED PP_EXPORT
bool RenderPDFPageToBitmap(const void* pdf_buffer,
                           int pdf_buffer_size,
                           int page_number,
                           void* bitmap_buffer,
                           int bitmap_width,
                           int bitmap_height,
                           int dpi,
                           bool autorotate) {
  if (!g_sdk_initialized_via_pepper) {
    void* data = NULL;
#if defined(OS_WIN)
    data = g_hmodule;
#endif
    if (!chrome_pdf::InitializeSDK(data))
      return false;
  }
  scoped_ptr<chrome_pdf::PDFEngineExports> engine_exports(
      chrome_pdf::PDFEngineExports::Create());
  chrome_pdf::PDFEngineExports::RenderingSettings settings(
      dpi, dpi, pp::Rect(bitmap_width, bitmap_height), true, false, true, true,
      autorotate);
  bool ret = engine_exports->RenderPDFPageToBitmap(
      pdf_buffer, pdf_buffer_size, page_number, settings, bitmap_buffer);
  if (!g_sdk_initialized_via_pepper) {
    chrome_pdf::ShutdownSDK();
  }
  return ret;
}

}  // extern "C"
