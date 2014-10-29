// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pdf/instance.h"

#include <algorithm>  // for min()
#define _USE_MATH_DEFINES  // for M_PI
#include <cmath>      // for log() and pow()
#include <math.h>
#include <list>

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/values.h"
#include "chrome/browser/chrome_page_zoom_constants.h"
#include "chrome/common/content_restriction.h"
#include "content/public/common/page_zoom.h"
#include "net/base/escape.h"
#include "pdf/draw_utils.h"
#include "pdf/number_image_generator.h"
#include "pdf/pdf.h"
#include "pdf/resource_consts.h"
#include "ppapi/c/dev/ppb_cursor_control_dev.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/c/pp_rect.h"
#include "ppapi/c/private/ppp_pdf.h"
#include "ppapi/c/trusted/ppb_url_loader_trusted.h"
#include "ppapi/cpp/core.h"
#include "ppapi/cpp/dev/font_dev.h"
#include "ppapi/cpp/dev/memory_dev.h"
#include "ppapi/cpp/dev/text_input_dev.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/point.h"
#include "ppapi/cpp/private/pdf.h"
#include "ppapi/cpp/rect.h"
#include "ppapi/cpp/resource.h"
#include "ppapi/cpp/url_request_info.h"
#include "ui/events/keycodes/keyboard_codes.h"

#if defined(OS_MACOSX)
#include "base/mac/mac_util.h"
#endif

namespace chrome_pdf {

struct ToolbarButtonInfo {
  uint32 id;
  Button::ButtonStyle style;
  PP_ResourceImage normal;
  PP_ResourceImage highlighted;
  PP_ResourceImage pressed;
};

namespace {

// Uncomment following #define to enable thumbnails.
// #define ENABLE_THUMBNAILS

const uint32 kToolbarSplashTimeoutMs = 6000;
const uint32 kMessageTextColor = 0xFF575757;
const uint32 kMessageTextSize = 22;
const uint32 kProgressFadeTimeoutMs = 250;
const uint32 kProgressDelayTimeoutMs = 1000;
const uint32 kAutoScrollTimeoutMs = 50;
const double kAutoScrollFactor = 0.2;

// Javascript methods.
const char kJSAccessibility[] = "accessibility";
const char kJSDocumentLoadComplete[] = "documentLoadComplete";
const char kJSGetHeight[] = "getHeight";
const char kJSGetHorizontalScrollbarThickness[] =
    "getHorizontalScrollbarThickness";
const char kJSGetPageLocationNormalized[] = "getPageLocationNormalized";
const char kJSGetSelectedText[] = "getSelectedText";
const char kJSGetVerticalScrollbarThickness[] = "getVerticalScrollbarThickness";
const char kJSGetWidth[] = "getWidth";
const char kJSGetZoomLevel[] = "getZoomLevel";
const char kJSGoToPage[] = "goToPage";
const char kJSGrayscale[] = "grayscale";
const char kJSLoadPreviewPage[] = "loadPreviewPage";
const char kJSOnLoad[] = "onload";
const char kJSOnPluginSizeChanged[] = "onPluginSizeChanged";
const char kJSOnScroll[] = "onScroll";
const char kJSPageXOffset[] = "pageXOffset";
const char kJSPageYOffset[] = "pageYOffset";
const char kJSPrintPreviewPageCount[] = "printPreviewPageCount";
const char kJSReload[] = "reload";
const char kJSRemovePrintButton[] = "removePrintButton";
const char kJSResetPrintPreviewUrl[] = "resetPrintPreviewUrl";
const char kJSSendKeyEvent[] = "sendKeyEvent";
const char kJSSetPageNumbers[] = "setPageNumbers";
const char kJSSetPageXOffset[] = "setPageXOffset";
const char kJSSetPageYOffset[] = "setPageYOffset";
const char kJSSetZoomLevel[] = "setZoomLevel";
const char kJSZoomFitToHeight[] = "fitToHeight";
const char kJSZoomFitToWidth[] = "fitToWidth";
const char kJSZoomIn[] = "zoomIn";
const char kJSZoomOut[] = "zoomOut";

// URL reference parameters.
// For more possible parameters, see RFC 3778 and the "PDF Open Parameters"
// document from Adobe.
const char kDelimiters[] = "#&";
const char kNamedDest[] = "nameddest";
const char kPage[] = "page";

const char kChromePrint[] = "chrome://print/";

// Dictionary Value key names for the document accessibility info
const char kAccessibleNumberOfPages[] = "numberOfPages";
const char kAccessibleLoaded[] = "loaded";
const char kAccessibleCopyable[] = "copyable";

const ToolbarButtonInfo kPDFToolbarButtons[] = {
  { kFitToPageButtonId, Button::BUTTON_STATE,
      PP_RESOURCEIMAGE_PDF_BUTTON_FTP,
      PP_RESOURCEIMAGE_PDF_BUTTON_FTP_HOVER,
      PP_RESOURCEIMAGE_PDF_BUTTON_FTP_PRESSED },
  { kFitToWidthButtonId, Button::BUTTON_STATE,
      PP_RESOURCEIMAGE_PDF_BUTTON_FTW,
      PP_RESOURCEIMAGE_PDF_BUTTON_FTW_HOVER,
      PP_RESOURCEIMAGE_PDF_BUTTON_FTW_PRESSED },
  { kZoomOutButtonId, Button::BUTTON_CLICKABLE,
      PP_RESOURCEIMAGE_PDF_BUTTON_ZOOMOUT,
      PP_RESOURCEIMAGE_PDF_BUTTON_ZOOMOUT_HOVER,
      PP_RESOURCEIMAGE_PDF_BUTTON_ZOOMOUT_PRESSED },
  { kZoomInButtonId, Button::BUTTON_CLICKABLE,
      PP_RESOURCEIMAGE_PDF_BUTTON_ZOOMIN,
      PP_RESOURCEIMAGE_PDF_BUTTON_ZOOMIN_HOVER,
      PP_RESOURCEIMAGE_PDF_BUTTON_ZOOMIN_PRESSED },
  { kSaveButtonId, Button::BUTTON_CLICKABLE,
      PP_RESOURCEIMAGE_PDF_BUTTON_SAVE,
      PP_RESOURCEIMAGE_PDF_BUTTON_SAVE_HOVER,
      PP_RESOURCEIMAGE_PDF_BUTTON_SAVE_PRESSED },
  { kPrintButtonId, Button::BUTTON_CLICKABLE,
      PP_RESOURCEIMAGE_PDF_BUTTON_PRINT,
      PP_RESOURCEIMAGE_PDF_BUTTON_PRINT_HOVER,
      PP_RESOURCEIMAGE_PDF_BUTTON_PRINT_PRESSED },
};

const ToolbarButtonInfo kPDFNoPrintToolbarButtons[] = {
  { kFitToPageButtonId, Button::BUTTON_STATE,
      PP_RESOURCEIMAGE_PDF_BUTTON_FTP,
      PP_RESOURCEIMAGE_PDF_BUTTON_FTP_HOVER,
      PP_RESOURCEIMAGE_PDF_BUTTON_FTP_PRESSED },
  { kFitToWidthButtonId, Button::BUTTON_STATE,
      PP_RESOURCEIMAGE_PDF_BUTTON_FTW,
      PP_RESOURCEIMAGE_PDF_BUTTON_FTW_HOVER,
      PP_RESOURCEIMAGE_PDF_BUTTON_FTW_PRESSED },
  { kZoomOutButtonId, Button::BUTTON_CLICKABLE,
      PP_RESOURCEIMAGE_PDF_BUTTON_ZOOMOUT,
      PP_RESOURCEIMAGE_PDF_BUTTON_ZOOMOUT_HOVER,
      PP_RESOURCEIMAGE_PDF_BUTTON_ZOOMOUT_PRESSED },
  { kZoomInButtonId, Button::BUTTON_CLICKABLE,
      PP_RESOURCEIMAGE_PDF_BUTTON_ZOOMIN,
      PP_RESOURCEIMAGE_PDF_BUTTON_ZOOMIN_HOVER,
      PP_RESOURCEIMAGE_PDF_BUTTON_ZOOMIN_PRESSED },
  { kSaveButtonId, Button::BUTTON_CLICKABLE,
      PP_RESOURCEIMAGE_PDF_BUTTON_SAVE,
      PP_RESOURCEIMAGE_PDF_BUTTON_SAVE_HOVER,
      PP_RESOURCEIMAGE_PDF_BUTTON_SAVE_PRESSED },
  { kPrintButtonId, Button::BUTTON_CLICKABLE,
      PP_RESOURCEIMAGE_PDF_BUTTON_PRINT_DISABLED,
      PP_RESOURCEIMAGE_PDF_BUTTON_PRINT_DISABLED,
      PP_RESOURCEIMAGE_PDF_BUTTON_PRINT_DISABLED }
};

const ToolbarButtonInfo kPrintPreviewToolbarButtons[] = {
  { kFitToPageButtonId, Button::BUTTON_STATE,
      PP_RESOURCEIMAGE_PDF_BUTTON_FTP,
      PP_RESOURCEIMAGE_PDF_BUTTON_FTP_HOVER,
      PP_RESOURCEIMAGE_PDF_BUTTON_FTP_PRESSED },
  { kFitToWidthButtonId, Button::BUTTON_STATE,
      PP_RESOURCEIMAGE_PDF_BUTTON_FTW,
      PP_RESOURCEIMAGE_PDF_BUTTON_FTW_HOVER,
      PP_RESOURCEIMAGE_PDF_BUTTON_FTW_PRESSED },
  { kZoomOutButtonId, Button::BUTTON_CLICKABLE,
      PP_RESOURCEIMAGE_PDF_BUTTON_ZOOMOUT,
      PP_RESOURCEIMAGE_PDF_BUTTON_ZOOMOUT_HOVER,
      PP_RESOURCEIMAGE_PDF_BUTTON_ZOOMOUT_PRESSED },
  { kZoomInButtonId, Button::BUTTON_CLICKABLE,
      PP_RESOURCEIMAGE_PDF_BUTTON_ZOOMIN_END,
      PP_RESOURCEIMAGE_PDF_BUTTON_ZOOMIN_END_HOVER,
      PP_RESOURCEIMAGE_PDF_BUTTON_ZOOMIN_END_PRESSED },
};

static const char kPPPPdfInterface[] = PPP_PDF_INTERFACE_1;

PP_Var GetLinkAtPosition(PP_Instance instance, PP_Point point) {
  pp::Var var;
  void* object =
      pp::Instance::GetPerInstanceObject(instance, kPPPPdfInterface);
  if (object)
    var = static_cast<Instance*>(object)->GetLinkAtPosition(pp::Point(point));
  return var.Detach();
}

void Transform(PP_Instance instance, PP_PrivatePageTransformType type) {
  void* object =
      pp::Instance::GetPerInstanceObject(instance, kPPPPdfInterface);
  if (object) {
    Instance* obj_instance = static_cast<Instance*>(object);
    switch (type) {
      case PP_PRIVATEPAGETRANSFORMTYPE_ROTATE_90_CW:
        obj_instance->RotateClockwise();
        break;
      case PP_PRIVATEPAGETRANSFORMTYPE_ROTATE_90_CCW:
        obj_instance->RotateCounterclockwise();
        break;
    }
  }
}

const PPP_Pdf ppp_private = {
  &GetLinkAtPosition,
  &Transform
};

int ExtractPrintPreviewPageIndex(const std::string& src_url) {
  // Sample |src_url| format: chrome://print/id/page_index/print.pdf
  std::vector<std::string> url_substr;
  base::SplitString(src_url.substr(strlen(kChromePrint)), '/', &url_substr);
  if (url_substr.size() != 3)
    return -1;

  if (url_substr[2] != "print.pdf")
    return -1;

  int page_index = 0;
  if (!base::StringToInt(url_substr[1], &page_index))
    return -1;
  return page_index;
}

bool IsPrintPreviewUrl(const std::string& url) {
  return url.substr(0, strlen(kChromePrint)) == kChromePrint;
}

void ScalePoint(float scale, pp::Point* point) {
  point->set_x(static_cast<int>(point->x() * scale));
  point->set_y(static_cast<int>(point->y() * scale));
}

void ScaleRect(float scale, pp::Rect* rect) {
  int left = static_cast<int>(floorf(rect->x() * scale));
  int top = static_cast<int>(floorf(rect->y() * scale));
  int right = static_cast<int>(ceilf((rect->x() + rect->width()) * scale));
  int bottom = static_cast<int>(ceilf((rect->y() + rect->height()) * scale));
  rect->SetRect(left, top, right - left, bottom - top);
}

template<class T>
T ClipToRange(T value, T lower_boundary, T upper_boundary) {
  DCHECK(lower_boundary <= upper_boundary);
  return std::max<T>(lower_boundary, std::min<T>(value, upper_boundary));
}

}  // namespace

Instance::Instance(PP_Instance instance)
    : pp::InstancePrivate(instance),
      pp::Find_Private(this),
      pp::Printing_Dev(this),
      pp::Selection_Dev(this),
      pp::WidgetClient_Dev(this),
      pp::Zoom_Dev(this),
      cursor_(PP_CURSORTYPE_POINTER),
      timer_pending_(false),
      current_timer_id_(0),
      zoom_(1.0),
      device_scale_(1.0),
      printing_enabled_(true),
      hidpi_enabled_(false),
      full_(IsFullFrame()),
      zoom_mode_(full_ ? ZOOM_AUTO : ZOOM_SCALE),
      did_call_start_loading_(false),
      is_autoscroll_(false),
      scrollbar_thickness_(-1),
      scrollbar_reserved_thickness_(-1),
      current_tb_info_(NULL),
      current_tb_info_size_(0),
      paint_manager_(this, this, true),
      delayed_progress_timer_id_(0),
      first_paint_(true),
      painted_first_page_(false),
      show_page_indicator_(false),
      document_load_state_(LOAD_STATE_LOADING),
      preview_document_load_state_(LOAD_STATE_COMPLETE),
      told_browser_about_unsupported_feature_(false),
      print_preview_page_count_(0) {
  loader_factory_.Initialize(this);
  timer_factory_.Initialize(this);
  form_factory_.Initialize(this);
  callback_factory_.Initialize(this);
  engine_.reset(PDFEngine::Create(this));
  pp::Module::Get()->AddPluginInterface(kPPPPdfInterface, &ppp_private);
  AddPerInstanceObject(kPPPPdfInterface, this);

  RequestFilteringInputEvents(PP_INPUTEVENT_CLASS_MOUSE);
  RequestFilteringInputEvents(PP_INPUTEVENT_CLASS_WHEEL);
  RequestFilteringInputEvents(PP_INPUTEVENT_CLASS_KEYBOARD);
  RequestFilteringInputEvents(PP_INPUTEVENT_CLASS_TOUCH);
}

Instance::~Instance() {
  if (timer_pending_) {
    timer_factory_.CancelAll();
    timer_pending_ = false;
  }
  // The engine may try to access this instance during its destruction.
  // Make sure this happens early while the instance is still intact.
  engine_.reset();
  RemovePerInstanceObject(kPPPPdfInterface, this);
}

bool Instance::Init(uint32_t argc, const char* argn[], const char* argv[]) {
  // For now, we hide HiDPI support behind a flag.
  if (pp::PDF::IsFeatureEnabled(this, PP_PDFFEATURE_HIDPI))
    hidpi_enabled_ = true;

  printing_enabled_ = pp::PDF::IsFeatureEnabled(this, PP_PDFFEATURE_PRINTING);
  if (printing_enabled_) {
    CreateToolbar(kPDFToolbarButtons, arraysize(kPDFToolbarButtons));
  } else {
    CreateToolbar(kPDFNoPrintToolbarButtons,
                  arraysize(kPDFNoPrintToolbarButtons));
  }

  CreateProgressBar();

  // Load autoscroll anchor image.
  autoscroll_anchor_ =
      CreateResourceImage(PP_RESOURCEIMAGE_PDF_PAN_SCROLL_ICON);

#ifdef ENABLE_THUMBNAILS
  CreateThumbnails();
#endif
  const char* url = NULL;
  for (uint32_t i = 0; i < argc; ++i) {
    if (strcmp(argn[i], "src") == 0) {
      url = argv[i];
      break;
    }
  }

  if (!url)
    return false;

  CreatePageIndicator(IsPrintPreviewUrl(url));

  if (!full_) {
    // For PDFs embedded in a frame, we don't get the data automatically like we
    // do for full-frame loads.  Start loading the data manually.
    LoadUrl(url);
  } else {
    DCHECK(!did_call_start_loading_);
    pp::PDF::DidStartLoading(this);
    did_call_start_loading_ = true;
  }

  ZoomLimitsChanged(kMinZoom, kMaxZoom);

  text_input_.reset(new pp::TextInput_Dev(this));

  url_ = url;
  return engine_->New(url);
}

bool Instance::HandleDocumentLoad(const pp::URLLoader& loader) {
  delayed_progress_timer_id_ = ScheduleTimer(kProgressBarId,
                                             kProgressDelayTimeoutMs);
  return engine_->HandleDocumentLoad(loader);
}

bool Instance::HandleInputEvent(const pp::InputEvent& event) {
  // To simplify things, convert the event into device coordinates if it is
  // a mouse event.
  pp::InputEvent event_device_res(event);
  {
    pp::MouseInputEvent mouse_event(event);
    if (!mouse_event.is_null()) {
      pp::Point point = mouse_event.GetPosition();
      pp::Point movement = mouse_event.GetMovement();
      ScalePoint(device_scale_, &point);
      ScalePoint(device_scale_, &movement);
      mouse_event = pp::MouseInputEvent(
          this,
          event.GetType(),
          event.GetTimeStamp(),
          event.GetModifiers(),
          mouse_event.GetButton(),
          point,
          mouse_event.GetClickCount(),
          movement);
      event_device_res = mouse_event;
    }
  }

  // Check if we need to go to autoscroll mode.
  if (event.GetType() == PP_INPUTEVENT_TYPE_MOUSEMOVE &&
     (event.GetModifiers() & PP_INPUTEVENT_MODIFIER_MIDDLEBUTTONDOWN)) {
    pp::MouseInputEvent mouse_event(event_device_res);
    pp::Point pos = mouse_event.GetPosition();
    EnableAutoscroll(pos);
    UpdateCursor(CalculateAutoscroll(pos));
    return true;
  } else {
    // Quit autoscrolling on any other event.
    DisableAutoscroll();
  }

#ifdef ENABLE_THUMBNAILS
  if (event.GetType() == PP_INPUTEVENT_TYPE_MOUSELEAVE)
    thumbnails_.SlideOut();

  if (thumbnails_.HandleEvent(event_device_res))
    return true;
#endif

  if (toolbar_->HandleEvent(event_device_res))
    return true;

#ifdef ENABLE_THUMBNAILS
  if (v_scrollbar_.get() && event.GetType() == PP_INPUTEVENT_TYPE_MOUSEMOVE) {
    pp::MouseInputEvent mouse_event(event);
    pp::Point pt = mouse_event.GetPosition();
    pp::Rect v_scrollbar_rc;
    v_scrollbar_->GetLocation(&v_scrollbar_rc);
    // There is a bug (https://bugs.webkit.org/show_bug.cgi?id=45208)
    // in the webkit that makes event.u.mouse.button
    // equal to PP_INPUTEVENT_MOUSEBUTTON_LEFT, even when no button is pressed.
    // To work around this issue we use modifier for now, and will switch
    // to button once the bug is fixed and webkit got merged back to our tree.
    if (v_scrollbar_rc.Contains(pt) &&
        (event.GetModifiers() & PP_INPUTEVENT_MODIFIER_LEFTBUTTONDOWN)) {
      thumbnails_.SlideIn();
    }

    // When scrollbar is in the scrolling mode we should display thumbnails
    // even the mouse is outside the thumbnail and scrollbar areas.
    // If mouse is outside plugin area, we are still getting mouse move events
    // while scrolling. See bug description for details:
    // http://code.google.com/p/chromium/issues/detail?id=56444
    if (!v_scrollbar_rc.Contains(pt) && thumbnails_.visible() &&
        !(event.GetModifiers() & PP_INPUTEVENT_MODIFIER_LEFTBUTTONDOWN) &&
        !thumbnails_.rect().Contains(pt)) {
      thumbnails_.SlideOut();
    }
  }
#endif

  // Need to pass the event to the engine first, since if we're over an edit
  // control we want it to get keyboard events (like space) instead of the
  // scrollbar.
  // TODO: will have to offset the mouse coordinates once we support bidi and
  // there could be scrollbars on the left.
  pp::InputEvent offset_event(event_device_res);
  bool try_engine_first = true;
  switch (offset_event.GetType()) {
    case PP_INPUTEVENT_TYPE_MOUSEDOWN:
    case PP_INPUTEVENT_TYPE_MOUSEUP:
    case PP_INPUTEVENT_TYPE_MOUSEMOVE:
    case PP_INPUTEVENT_TYPE_MOUSEENTER:
    case PP_INPUTEVENT_TYPE_MOUSELEAVE: {
      pp::MouseInputEvent mouse_event(event_device_res);
      pp::MouseInputEvent mouse_event_dip(event);
      pp::Point point = mouse_event.GetPosition();
      point.set_x(point.x() - available_area_.x());
      offset_event = pp::MouseInputEvent(
          this,
          event.GetType(),
          event.GetTimeStamp(),
          event.GetModifiers(),
          mouse_event.GetButton(),
          point,
          mouse_event.GetClickCount(),
          mouse_event.GetMovement());
      if (!engine_->IsSelecting()) {
        if (!IsOverlayScrollbar() &&
            !available_area_.Contains(mouse_event.GetPosition())) {
          try_engine_first = false;
        } else if (IsOverlayScrollbar()) {
          pp::Rect temp;
          if ((v_scrollbar_.get() && v_scrollbar_->GetLocation(&temp) &&
              temp.Contains(mouse_event_dip.GetPosition())) ||
              (h_scrollbar_.get() && h_scrollbar_->GetLocation(&temp) &&
              temp.Contains(mouse_event_dip.GetPosition()))) {
            try_engine_first = false;
          }
        }
      }
      break;
    }
    default:
      break;
  }
  if (try_engine_first && engine_->HandleEvent(offset_event))
    return true;

  // Left/Right arrows should scroll to the beginning of the Prev/Next page if
  // there is no horizontal scroll bar.
  // If fit-to-height, PgDown/PgUp should scroll to the beginning of the
  // Prev/Next page. Spacebar / shift+spacebar should do the same.
  if (v_scrollbar_.get() && event.GetType() == PP_INPUTEVENT_TYPE_KEYDOWN) {
    pp::KeyboardInputEvent keyboard_event(event);
    bool no_h_scrollbar = !h_scrollbar_.get();
    uint32_t key_code = keyboard_event.GetKeyCode();
    bool page_down = no_h_scrollbar && key_code == ui::VKEY_RIGHT;
    bool page_up = no_h_scrollbar && key_code == ui::VKEY_LEFT;
    if (zoom_mode_ == ZOOM_FIT_TO_PAGE) {
      bool has_shift =
          keyboard_event.GetModifiers() & PP_INPUTEVENT_MODIFIER_SHIFTKEY;
      bool key_is_space = key_code == ui::VKEY_SPACE;
      page_down |= key_is_space || key_code == ui::VKEY_NEXT;
      page_up |= (key_is_space && has_shift) || (key_code == ui::VKEY_PRIOR);
    }
    if (page_down) {
      int page = engine_->GetFirstVisiblePage();
      if (page == -1)
        return true;
      // Engine calculates visible page including delimiter to the page size.
      // We need to check here if the page itself is completely out of view and
      // scroll to the next one in that case.
      if (engine_->GetPageRect(page).bottom() * zoom_ <=
          v_scrollbar_->GetValue())
        page++;
      ScrollToPage(page + 1);
      UpdateCursor(PP_CURSORTYPE_POINTER);
      return true;
    } else if (page_up) {
      int page = engine_->GetFirstVisiblePage();
      if (page == -1)
        return true;
      if (engine_->GetPageRect(page).y() * zoom_ >= v_scrollbar_->GetValue())
        page--;
      ScrollToPage(page);
      UpdateCursor(PP_CURSORTYPE_POINTER);
      return true;
    }
  }

  if (v_scrollbar_.get() && v_scrollbar_->HandleEvent(event)) {
    UpdateCursor(PP_CURSORTYPE_POINTER);
    return true;
  }

  if (h_scrollbar_.get() && h_scrollbar_->HandleEvent(event)) {
    UpdateCursor(PP_CURSORTYPE_POINTER);
    return true;
  }

  if (timer_pending_ &&
      (event.GetType() == PP_INPUTEVENT_TYPE_MOUSEUP ||
       event.GetType() == PP_INPUTEVENT_TYPE_MOUSEMOVE)) {
    timer_factory_.CancelAll();
    timer_pending_ = false;
  } else if (event.GetType() == PP_INPUTEVENT_TYPE_MOUSEMOVE &&
             engine_->IsSelecting()) {
    bool set_timer = false;
    pp::MouseInputEvent mouse_event(event);
    if (v_scrollbar_.get() &&
        (mouse_event.GetPosition().y() <= 0 ||
         mouse_event.GetPosition().y() >= (plugin_dip_size_.height() - 1))) {
      v_scrollbar_->ScrollBy(
          PP_SCROLLBY_LINE, mouse_event.GetPosition().y() >= 0 ? 1: -1);
      set_timer = true;
    }
    if (h_scrollbar_.get() &&
        (mouse_event.GetPosition().x() <= 0 ||
         mouse_event.GetPosition().x() >= (plugin_dip_size_.width() - 1))) {
      h_scrollbar_->ScrollBy(PP_SCROLLBY_LINE,
          mouse_event.GetPosition().x() >= 0 ? 1: -1);
      set_timer = true;
    }

    if (set_timer) {
      last_mouse_event_ = pp::MouseInputEvent(event);

      pp::CompletionCallback callback =
          timer_factory_.NewCallback(&Instance::OnTimerFired);
      pp::Module::Get()->core()->CallOnMainThread(kDragTimerMs, callback);
      timer_pending_ = true;
    }
  }

  if (event.GetType() == PP_INPUTEVENT_TYPE_KEYDOWN) {
    pp::KeyboardInputEvent keyboard_event(event);
    const uint32 modifier = event.GetModifiers();
    if (modifier & kDefaultKeyModifier) {
      switch (keyboard_event.GetKeyCode()) {
        case 'A':
          engine_->SelectAll();
          return true;
      }
    } else if (modifier & PP_INPUTEVENT_MODIFIER_CONTROLKEY) {
      switch (keyboard_event.GetKeyCode()) {
        case ui::VKEY_OEM_4:
          // Left bracket.
          engine_->RotateCounterclockwise();
          return true;
        case ui::VKEY_OEM_6:
          // Right bracket.
          engine_->RotateClockwise();
          return true;
      }
    }
  }

  // Return true for unhandled clicks so the plugin takes focus.
  return (event.GetType() == PP_INPUTEVENT_TYPE_MOUSEDOWN);
}

void Instance::DidChangeView(const pp::View& view) {
  pp::Rect view_rect(view.GetRect());
  float device_scale = 1.0f;
  float old_device_scale = device_scale_;
  if (hidpi_enabled_)
    device_scale = view.GetDeviceScale();
  pp::Size view_device_size(view_rect.width() * device_scale,
                            view_rect.height() * device_scale);
  if (view_device_size == plugin_size_ && device_scale == device_scale_)
    return;  // We don't care about the position, only the size.

  image_data_ = pp::ImageData();
  device_scale_ = device_scale;
  plugin_dip_size_ = view_rect.size();
  plugin_size_ = view_device_size;

  paint_manager_.SetSize(view_device_size, device_scale_);

  image_data_ = pp::ImageData(this,
                              PP_IMAGEDATAFORMAT_BGRA_PREMUL,
                              plugin_size_,
                              false);
  if (image_data_.is_null()) {
    DCHECK(plugin_size_.IsEmpty());
    return;
  }

  // View dimensions changed, disable autoscroll (if it was enabled).
  DisableAutoscroll();

  OnGeometryChanged(zoom_, old_device_scale);
}

pp::Var Instance::GetInstanceObject() {
  if (instance_object_.is_undefined()) {
    PDFScriptableObject* object = new PDFScriptableObject(this);
    // The pp::Var takes ownership of object here.
    instance_object_ = pp::VarPrivate(this, object);
  }

  return instance_object_;
}

pp::Var Instance::GetLinkAtPosition(const pp::Point& point) {
  pp::Point offset_point(point);
  ScalePoint(device_scale_, &offset_point);
  offset_point.set_x(offset_point.x() - available_area_.x());
  return engine_->GetLinkAtPosition(offset_point);
}

pp::Var Instance::GetSelectedText(bool html) {
  if (html || !engine_->HasPermission(PDFEngine::PERMISSION_COPY))
    return pp::Var();
  return engine_->GetSelectedText();
}

void Instance::InvalidateWidget(pp::Widget_Dev widget,
                                const pp::Rect& dirty_rect) {
  if (v_scrollbar_.get() && *v_scrollbar_ == widget) {
    if (!image_data_.is_null())
      v_scrollbar_->Paint(dirty_rect.pp_rect(), &image_data_);
  } else if (h_scrollbar_.get() && *h_scrollbar_ == widget) {
    if (!image_data_.is_null())
      h_scrollbar_->Paint(dirty_rect.pp_rect(), &image_data_);
  } else {
    // Possible to hit this condition since sometimes the scrollbar codes posts
    // a task to do something later, and we could have deleted our reference in
    // the meantime.
    return;
  }

  pp::Rect dirty_rect_scaled = dirty_rect;
  ScaleRect(device_scale_, &dirty_rect_scaled);
  paint_manager_.InvalidateRect(dirty_rect_scaled);
}

void Instance::ScrollbarValueChanged(pp::Scrollbar_Dev scrollbar,
                                     uint32_t value) {
  value = GetScaled(value);
  if (v_scrollbar_.get() && *v_scrollbar_ == scrollbar) {
    engine_->ScrolledToYPosition(value);
    pp::Rect rc;
    v_scrollbar_->GetLocation(&rc);
    int32 doc_height = GetDocumentPixelHeight();
    doc_height -= GetScaled(rc.height());
#ifdef ENABLE_THUMBNAILS
    if (thumbnails_.visible()) {
      thumbnails_.SetPosition(value, doc_height, true);
    }
#endif
    pp::Point origin(
        plugin_size_.width() - page_indicator_.rect().width() -
            GetScaled(GetScrollbarReservedThickness()),
        page_indicator_.GetYPosition(value, doc_height, plugin_size_.height()));
    page_indicator_.MoveTo(origin, page_indicator_.visible());
  } else if (h_scrollbar_.get() && *h_scrollbar_ == scrollbar) {
    engine_->ScrolledToXPosition(value);
  }
}

void Instance::ScrollbarOverlayChanged(pp::Scrollbar_Dev scrollbar,
                                       bool overlay) {
  scrollbar_reserved_thickness_ = overlay ? 0 : scrollbar_thickness_;
  OnGeometryChanged(zoom_, device_scale_);
}

uint32_t Instance::QuerySupportedPrintOutputFormats() {
  return engine_->QuerySupportedPrintOutputFormats();
}

int32_t Instance::PrintBegin(const PP_PrintSettings_Dev& print_settings) {
  // For us num_pages is always equal to the number of pages in the PDF
  // document irrespective of the printable area.
  int32_t ret = engine_->GetNumberOfPages();
  if (!ret)
    return 0;

  uint32_t supported_formats = engine_->QuerySupportedPrintOutputFormats();
  if ((print_settings.format & supported_formats) == 0)
    return 0;

  print_settings_.is_printing = true;
  print_settings_.pepper_print_settings = print_settings;
  engine_->PrintBegin();
  return ret;
}

pp::Resource Instance::PrintPages(
    const PP_PrintPageNumberRange_Dev* page_ranges,
    uint32_t page_range_count) {
  if (!print_settings_.is_printing)
    return pp::Resource();

  print_settings_.print_pages_called_ = true;
  return engine_->PrintPages(page_ranges, page_range_count,
                             print_settings_.pepper_print_settings);
}

void Instance::PrintEnd() {
  if (print_settings_.print_pages_called_)
    UserMetricsRecordAction("PDF.PrintPage");
  print_settings_.Clear();
  engine_->PrintEnd();
}

bool Instance::IsPrintScalingDisabled() {
  return !engine_->GetPrintScaling();
}

bool Instance::StartFind(const std::string& text, bool case_sensitive) {
  engine_->StartFind(text.c_str(), case_sensitive);
  return true;
}

void Instance::SelectFindResult(bool forward) {
  engine_->SelectFindResult(forward);
}

void Instance::StopFind() {
  engine_->StopFind();
}

void Instance::Zoom(double scale, bool text_only) {
  UserMetricsRecordAction("PDF.ZoomFromBrowser");

  // If the zoom level doesn't change it means that this zoom change might have
  // been initiated by the plugin. In that case, we don't want to change the
  // zoom mode to ZOOM_SCALE as it may have been intentionally set to
  // ZOOM_FIT_TO_PAGE or some other value when the zoom was last changed.
  if (scale == zoom_)
    return;

  SetZoom(ZOOM_SCALE, scale);
}

void Instance::ZoomChanged(double factor) {
  if (full_)
    Zoom_Dev::ZoomChanged(factor);
}

void Instance::OnPaint(const std::vector<pp::Rect>& paint_rects,
                       std::vector<PaintManager::ReadyRect>* ready,
                       std::vector<pp::Rect>* pending) {
  if (image_data_.is_null()) {
    DCHECK(plugin_size_.IsEmpty());
    return;
  }
  if (first_paint_) {
    first_paint_ = false;
    pp::Rect rect = pp::Rect(pp::Point(), plugin_size_);
    FillRect(rect, kBackgroundColor);
    ready->push_back(PaintManager::ReadyRect(rect, image_data_, true));
    *pending = paint_rects;
    return;
  }

  engine_->PrePaint();

  for (size_t i = 0; i < paint_rects.size(); i++) {
    // Intersect with plugin area since there could be pending invalidates from
    // when the plugin area was larger.
    pp::Rect rect =
        paint_rects[i].Intersect(pp::Rect(pp::Point(), plugin_size_));
    if (rect.IsEmpty())
      continue;

    pp::Rect pdf_rect = available_area_.Intersect(rect);
    if (!pdf_rect.IsEmpty()) {
      pdf_rect.Offset(available_area_.x() * -1, 0);

      std::vector<pp::Rect> pdf_ready;
      std::vector<pp::Rect> pdf_pending;
      engine_->Paint(pdf_rect, &image_data_, &pdf_ready, &pdf_pending);
      for (size_t j = 0; j < pdf_ready.size(); ++j) {
        pdf_ready[j].Offset(available_area_.point());
        ready->push_back(
            PaintManager::ReadyRect(pdf_ready[j], image_data_, false));
      }
      for (size_t j = 0; j < pdf_pending.size(); ++j) {
        pdf_pending[j].Offset(available_area_.point());
        pending->push_back(pdf_pending[j]);
      }
    }

    for (size_t j = 0; j < background_parts_.size(); ++j) {
      pp::Rect intersection = background_parts_[j].location.Intersect(rect);
      if (!intersection.IsEmpty()) {
        FillRect(intersection, background_parts_[j].color);
        ready->push_back(
            PaintManager::ReadyRect(intersection, image_data_, false));
      }
    }

    if (document_load_state_ == LOAD_STATE_FAILED) {
      pp::Point top_center;
      top_center.set_x(plugin_size_.width() / 2);
      top_center.set_y(plugin_size_.height() / 2);
      DrawText(top_center, PP_RESOURCESTRING_PDFLOAD_FAILED);
    }

#ifdef ENABLE_THUMBNAILS
    thumbnails_.Paint(&image_data_, rect);
#endif
  }

  engine_->PostPaint();

  // Must paint scrollbars after the background parts, in case we have an
  // overlay scrollbar that's over the background. We also do this in a separate
  // loop because the scrollbar painting logic uses the signal of whether there
  // are pending paints or not to figure out if it should draw right away or
  // not.
  for (size_t i = 0; i < paint_rects.size(); i++) {
    PaintIfWidgetIntersects(h_scrollbar_.get(), paint_rects[i], ready, pending);
    PaintIfWidgetIntersects(v_scrollbar_.get(), paint_rects[i], ready, pending);
  }

  if (progress_bar_.visible())
    PaintOverlayControl(&progress_bar_, &image_data_, ready);

  if (page_indicator_.visible())
    PaintOverlayControl(&page_indicator_, &image_data_, ready);

  if (toolbar_->current_transparency() != kTransparentAlpha)
    PaintOverlayControl(toolbar_.get(), &image_data_, ready);

  // Paint autoscroll anchor if needed.
  if (is_autoscroll_) {
    size_t limit = ready->size();
    for (size_t i = 0; i < limit; i++) {
      pp::Rect anchor_rect = autoscroll_rect_.Intersect((*ready)[i].rect);
      if (!anchor_rect.IsEmpty()) {
        pp::Rect draw_rc = pp::Rect(
            pp::Point(anchor_rect.x() - autoscroll_rect_.x(),
                      anchor_rect.y() - autoscroll_rect_.y()),
            anchor_rect.size());
        // Paint autoscroll anchor.
        AlphaBlend(autoscroll_anchor_, draw_rc,
                   &image_data_, anchor_rect.point(), kOpaqueAlpha);
      }
    }
  }
}

void Instance::PaintOverlayControl(
    Control* ctrl,
    pp::ImageData* image_data,
    std::vector<PaintManager::ReadyRect>* ready) {
  // Make sure that we only paint overlay controls over an area that's ready,
  // i.e. not pending.  Otherwise we'll mark the control rect as ready and
  // it'll overwrite the pdf region.
  std::list<pp::Rect> ctrl_rects;
  for (size_t i = 0; i < ready->size(); i++) {
    pp::Rect rc = ctrl->rect().Intersect((*ready)[i].rect);
    if (!rc.IsEmpty())
      ctrl_rects.push_back(rc);
  }

  if (!ctrl_rects.empty()) {
    ctrl->PaintMultipleRects(image_data, ctrl_rects);

    std::list<pp::Rect>::iterator iter;
    for (iter = ctrl_rects.begin(); iter != ctrl_rects.end(); ++iter) {
      ready->push_back(PaintManager::ReadyRect(*iter, *image_data, false));
    }
  }
}

void Instance::DidOpen(int32_t result) {
  if (result == PP_OK) {
    engine_->HandleDocumentLoad(embed_loader_);
  } else if (result != PP_ERROR_ABORTED) {  // Can happen in tests.
    NOTREACHED();
  }
}

void Instance::DidOpenPreview(int32_t result) {
  if (result == PP_OK) {
    preview_engine_.reset(PDFEngine::Create(new PreviewModeClient(this)));
    preview_engine_->HandleDocumentLoad(embed_preview_loader_);
  } else {
    NOTREACHED();
  }
}

void Instance::PaintIfWidgetIntersects(
    pp::Widget_Dev* widget,
    const pp::Rect& rect,
    std::vector<PaintManager::ReadyRect>* ready,
    std::vector<pp::Rect>* pending) {
  if (!widget)
    return;

  pp::Rect location;
  if (!widget->GetLocation(&location))
    return;

  ScaleRect(device_scale_, &location);
  location = location.Intersect(rect);
  if (location.IsEmpty())
    return;

  if (IsOverlayScrollbar()) {
    // If we're using overlay scrollbars, and there are pending paints under the
    // scrollbar, don't update the scrollbar instantly. While it would be nice,
    // we would need to double buffer the plugin area in order to make this
    // work. This is because we'd need to always have a copy of what the pdf
    // under the scrollbar looks like, and additionally we couldn't paint the
    // pdf under the scrollbar if it's ready until we got the preceding flush.
    // So in practice, it would make painting slower and introduce extra buffer
    // copies for the general case.
    for (size_t i = 0; i < pending->size(); ++i) {
      if ((*pending)[i].Intersects(location))
        return;
    }

    // Even if none of the pending paints are under the scrollbar, we never want
    // to paint it if it's over the pdf if there are other pending paints.
    // Otherwise different parts of the pdf plugin would display at different
    // scroll offsets.
    if (!pending->empty() && available_area_.Intersects(rect)) {
      pending->push_back(location);
      return;
    }
  }

  pp::Rect location_dip = location;
  ScaleRect(1.0f / device_scale_, &location_dip);

  DCHECK(!image_data_.is_null());
  widget->Paint(location_dip, &image_data_);

  ready->push_back(PaintManager::ReadyRect(location, image_data_, true));
}

void Instance::OnTimerFired(int32_t) {
  HandleInputEvent(last_mouse_event_);
}

void Instance::OnClientTimerFired(int32_t id) {
  engine_->OnCallback(id);
}

void Instance::OnControlTimerFired(int32_t,
                                   const uint32& control_id,
                                   const uint32& timer_id) {
  if (control_id == toolbar_->id()) {
    toolbar_->OnTimerFired(timer_id);
  } else if (control_id == progress_bar_.id()) {
    if (timer_id == delayed_progress_timer_id_) {
      if (document_load_state_ == LOAD_STATE_LOADING &&
          !progress_bar_.visible()) {
        progress_bar_.Fade(true, kProgressFadeTimeoutMs);
      }
      delayed_progress_timer_id_ = 0;
    } else {
      progress_bar_.OnTimerFired(timer_id);
    }
  } else if (control_id == kAutoScrollId) {
    if (is_autoscroll_) {
      if (autoscroll_x_ != 0 && h_scrollbar_.get()) {
        h_scrollbar_->ScrollBy(PP_SCROLLBY_PIXEL, autoscroll_x_);
      }
      if (autoscroll_y_ != 0 && v_scrollbar_.get()) {
        v_scrollbar_->ScrollBy(PP_SCROLLBY_PIXEL, autoscroll_y_);
      }

      // Reschedule timer.
      ScheduleTimer(kAutoScrollId, kAutoScrollTimeoutMs);
    }
  } else if (control_id == kPageIndicatorId) {
    page_indicator_.OnTimerFired(timer_id);
  }
#ifdef ENABLE_THUMBNAILS
  else if (control_id == thumbnails_.id()) {
    thumbnails_.OnTimerFired(timer_id);
  }
#endif
}

void Instance::CalculateBackgroundParts() {
  background_parts_.clear();
  int v_scrollbar_thickness =
      GetScaled(v_scrollbar_.get() ? GetScrollbarReservedThickness() : 0);
  int h_scrollbar_thickness =
      GetScaled(h_scrollbar_.get() ? GetScrollbarReservedThickness() : 0);
  int width_without_scrollbar = std::max(
      plugin_size_.width() - v_scrollbar_thickness, 0);
  int height_without_scrollbar = std::max(
      plugin_size_.height() - h_scrollbar_thickness, 0);
  int left_width = available_area_.x();
  int right_start = available_area_.right();
  int right_width = abs(width_without_scrollbar - available_area_.right());
  int bottom = std::min(available_area_.bottom(), height_without_scrollbar);

  // Add the left, right, and bottom rectangles.  Note: we assume only
  // horizontal centering.
  BackgroundPart part = {
    pp::Rect(0, 0, left_width, bottom),
    kBackgroundColor
  };
  if (!part.location.IsEmpty())
    background_parts_.push_back(part);
  part.location = pp::Rect(right_start, 0, right_width, bottom);
  if (!part.location.IsEmpty())
    background_parts_.push_back(part);
  part.location = pp::Rect(
      0, bottom, width_without_scrollbar, height_without_scrollbar - bottom);
  if (!part.location.IsEmpty())
    background_parts_.push_back(part);

  if (h_scrollbar_thickness
#if defined(OS_MACOSX)
      ||
#else
      &&
#endif
      v_scrollbar_thickness) {
    part.color = 0xFFFFFFFF;
    part.location = pp::Rect(plugin_size_.width() - v_scrollbar_thickness,
                             plugin_size_.height() - h_scrollbar_thickness,
                             h_scrollbar_thickness,
                             v_scrollbar_thickness);
    background_parts_.push_back(part);
  }
}

int Instance::GetDocumentPixelWidth() const {
  return static_cast<int>(ceil(document_size_.width() * zoom_ * device_scale_));
}

int Instance::GetDocumentPixelHeight() const {
  return static_cast<int>(ceil(document_size_.height() *
                               zoom_ *
                               device_scale_));
}

void Instance::FillRect(const pp::Rect& rect, uint32 color) {
  DCHECK(!image_data_.is_null() || rect.IsEmpty());
  uint32* buffer_start = static_cast<uint32*>(image_data_.data());
  int stride = image_data_.stride();
  uint32* ptr = buffer_start + rect.y() * stride / 4 + rect.x();
  int height = rect.height();
  int width = rect.width();
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x)
      *(ptr + x) = color;
    ptr += stride / 4;
  }
}

void Instance::DocumentSizeUpdated(const pp::Size& size) {
  document_size_ = size;

  OnGeometryChanged(zoom_, device_scale_);
}

void Instance::Invalidate(const pp::Rect& rect) {
  pp::Rect offset_rect(rect);
  offset_rect.Offset(available_area_.point());
  paint_manager_.InvalidateRect(offset_rect);
}

void Instance::Scroll(const pp::Point& point) {
  pp::Rect scroll_area = available_area_;
  if (IsOverlayScrollbar()) {
    pp::Rect rc;
    if (h_scrollbar_.get()) {
      h_scrollbar_->GetLocation(&rc);
      ScaleRect(device_scale_, &rc);
      if (scroll_area.bottom() > rc.y()) {
        scroll_area.set_height(rc.y() - scroll_area.y());
        paint_manager_.InvalidateRect(rc);
      }
    }
    if (v_scrollbar_.get()) {
      v_scrollbar_->GetLocation(&rc);
      ScaleRect(device_scale_, &rc);
      if (scroll_area.right() > rc.x()) {
        scroll_area.set_width(rc.x() - scroll_area.x());
        paint_manager_.InvalidateRect(rc);
      }
    }
  }
  paint_manager_.ScrollRect(scroll_area, point);

  if (toolbar_->current_transparency() != kTransparentAlpha)
    paint_manager_.InvalidateRect(toolbar_->GetControlsRect());

  if (progress_bar_.visible())
    paint_manager_.InvalidateRect(progress_bar_.rect());

  if (is_autoscroll_)
    paint_manager_.InvalidateRect(autoscroll_rect_);

  if (show_page_indicator_) {
    page_indicator_.set_current_page(GetPageNumberToDisplay());
    page_indicator_.Splash();
  }

  if (page_indicator_.visible())
    paint_manager_.InvalidateRect(page_indicator_.rect());

  // Run the scroll callback asynchronously. This function can be invoked by a
  // layout change which should not re-enter into JS synchronously.
  pp::CompletionCallback callback =
      callback_factory_.NewCallback(&Instance::RunCallback,
                                    on_scroll_callback_);
  pp::Module::Get()->core()->CallOnMainThread(0, callback);
}

void Instance::ScrollToX(int position) {
  if (!h_scrollbar_.get()) {
    NOTREACHED();
    return;
  }
  int position_dip = static_cast<int>(position / device_scale_);
  h_scrollbar_->SetValue(position_dip);
}

void Instance::ScrollToY(int position) {
  if (!v_scrollbar_.get()) {
    NOTREACHED();
    return;
  }
  int position_dip = static_cast<int>(position / device_scale_);
  v_scrollbar_->SetValue(ClipToRange(position_dip, 0, valid_v_range_));
}

void Instance::ScrollToPage(int page) {
  if (!v_scrollbar_.get())
    return;

  if (engine_->GetNumberOfPages() == 0)
    return;

  int index = ClipToRange(page, 0, engine_->GetNumberOfPages() - 1);
  pp::Rect rect = engine_->GetPageRect(index);
  // If we are trying to scroll pass the last page,
  // scroll to the end of the last page.
  int position = index < page ? rect.bottom() : rect.y();
  ScrollToY(position * zoom_ * device_scale_);
}

void Instance::NavigateTo(const std::string& url, bool open_in_new_tab) {
  std::string url_copy(url);

  // Empty |url_copy| is ok, and will effectively be a reload.
  // Skip the code below so an empty URL does not turn into "http://", which
  // will cause GURL to fail a DCHECK.
  if (!url_copy.empty()) {
    // If |url_copy| starts with '#', then it's for the same URL with a
    // different URL fragment.
    if (url_copy[0] == '#') {
      url_copy = url_ + url_copy;
      // Changing the href does not actually do anything when navigating in the
      // same tab, so do the actual page scroll here. Then fall through so the
      // href gets updated.
      if (!open_in_new_tab) {
        int page_number = GetInitialPage(url_copy);
        if (page_number >= 0)
          ScrollToPage(page_number);
      }
    }
    // If there's no scheme, add http.
    if (url_copy.find("://") == std::string::npos &&
        url_copy.find("mailto:") == std::string::npos) {
      url_copy = "http://" + url_copy;
    }
    // Make sure |url_copy| starts with a valid scheme.
    if (url_copy.find("http://") != 0 &&
        url_copy.find("https://") != 0 &&
        url_copy.find("ftp://") != 0 &&
        url_copy.find("file://") != 0 &&
        url_copy.find("mailto:") != 0) {
      return;
    }
    // Make sure |url_copy| is not only a scheme.
    if (url_copy == "http://" ||
        url_copy == "https://" ||
        url_copy == "ftp://" ||
        url_copy == "file://" ||
        url_copy == "mailto:") {
      return;
    }
  }
  if (open_in_new_tab) {
    GetWindowObject().Call("open", url_copy);
  } else {
    GetWindowObject().GetProperty("top").GetProperty("location").
        SetProperty("href", url_copy);
  }
}

void Instance::UpdateCursor(PP_CursorType_Dev cursor) {
  if (cursor == cursor_)
    return;
  cursor_ = cursor;

  const PPB_CursorControl_Dev* cursor_interface =
      reinterpret_cast<const PPB_CursorControl_Dev*>(
      pp::Module::Get()->GetBrowserInterface(PPB_CURSOR_CONTROL_DEV_INTERFACE));
  if (!cursor_interface) {
    NOTREACHED();
    return;
  }

  cursor_interface->SetCursor(
      pp_instance(), cursor_, pp::ImageData().pp_resource(), NULL);
}

void Instance::UpdateTickMarks(const std::vector<pp::Rect>& tickmarks) {
  if (!v_scrollbar_.get())
    return;

  float inverse_scale = 1.0f / device_scale_;
  std::vector<pp::Rect> scaled_tickmarks = tickmarks;
  for (size_t i = 0; i < scaled_tickmarks.size(); i++) {
    ScaleRect(inverse_scale, &scaled_tickmarks[i]);
  }

  v_scrollbar_->SetTickMarks(
      scaled_tickmarks.empty() ? NULL : &scaled_tickmarks[0], tickmarks.size());
}

void Instance::NotifyNumberOfFindResultsChanged(int total, bool final_result) {
  NumberOfFindResultsChanged(total, final_result);
}

void Instance::NotifySelectedFindResultChanged(int current_find_index) {
  SelectedFindResultChanged(current_find_index);
}

void Instance::OnEvent(uint32 control_id, uint32 event_id, void* data) {
  if (event_id == Button::EVENT_ID_BUTTON_CLICKED ||
      event_id == Button::EVENT_ID_BUTTON_STATE_CHANGED) {
    switch (control_id) {
      case kFitToPageButtonId:
        UserMetricsRecordAction("PDF.FitToPageButton");
        SetZoom(ZOOM_FIT_TO_PAGE, 0);
        ZoomChanged(zoom_);
        break;
      case kFitToWidthButtonId:
        UserMetricsRecordAction("PDF.FitToWidthButton");
        SetZoom(ZOOM_FIT_TO_WIDTH, 0);
        ZoomChanged(zoom_);
        break;
      case kZoomOutButtonId:
      case kZoomInButtonId:
        UserMetricsRecordAction(control_id == kZoomOutButtonId ?
            "PDF.ZoomOutButton" : "PDF.ZoomInButton");
        SetZoom(ZOOM_SCALE, CalculateZoom(control_id));
        ZoomChanged(zoom_);
        break;
      case kSaveButtonId:
        UserMetricsRecordAction("PDF.SaveButton");
        SaveAs();
        break;
      case kPrintButtonId:
        UserMetricsRecordAction("PDF.PrintButton");
        Print();
        break;
    }
  }
  if (control_id == kThumbnailsId &&
      event_id == ThumbnailControl::EVENT_ID_THUMBNAIL_SELECTED) {
    int page = *static_cast<int*>(data);
    pp::Rect page_rc(engine_->GetPageRect(page));
    ScrollToY(static_cast<int>(page_rc.y() * zoom_ * device_scale_));
  }
}

void Instance::Invalidate(uint32 control_id, const pp::Rect& rc) {
  paint_manager_.InvalidateRect(rc);
}

uint32 Instance::ScheduleTimer(uint32 control_id, uint32 timeout_ms) {
  current_timer_id_++;
  pp::CompletionCallback callback =
      timer_factory_.NewCallback(&Instance::OnControlTimerFired,
                                 control_id,
                                 current_timer_id_);
  pp::Module::Get()->core()->CallOnMainThread(timeout_ms, callback);
  return current_timer_id_;
}

void Instance::SetEventCapture(uint32 control_id, bool set_capture) {
  // TODO(gene): set event capture here.
}

void Instance::SetCursor(uint32 control_id, PP_CursorType_Dev cursor_type) {
  UpdateCursor(cursor_type);
}

pp::Instance* Instance::GetInstance() {
  return this;
}

void Instance::GetDocumentPassword(
    pp::CompletionCallbackWithOutput<pp::Var> callback) {
  std::string message(GetLocalizedString(PP_RESOURCESTRING_PDFGETPASSWORD));
  pp::Var result = pp::PDF::ModalPromptForPassword(this, message);
  *callback.output() = result.pp_var();
  callback.Run(PP_OK);
}

void Instance::Alert(const std::string& message) {
  GetWindowObject().Call("alert", message);
}

bool Instance::Confirm(const std::string& message) {
  pp::Var result = GetWindowObject().Call("confirm", message);
  return result.is_bool() ? result.AsBool() : false;
}

std::string Instance::Prompt(const std::string& question,
                             const std::string& default_answer) {
  pp::Var result = GetWindowObject().Call("prompt", question, default_answer);
  return result.is_string() ? result.AsString() : std::string();
}

std::string Instance::GetURL() {
  return url_;
}

void Instance::Email(const std::string& to,
                     const std::string& cc,
                     const std::string& bcc,
                     const std::string& subject,
                     const std::string& body) {
  std::string javascript =
      "var href = 'mailto:" + net::EscapeUrlEncodedData(to, false) +
      "?cc=" + net::EscapeUrlEncodedData(cc, false) +
      "&bcc=" + net::EscapeUrlEncodedData(bcc, false) +
      "&subject=" + net::EscapeUrlEncodedData(subject, false) +
      "&body=" + net::EscapeUrlEncodedData(body, false) +
      "';var temp = window.open(href, '_blank', " +
      "'width=1,height=1');if(temp) temp.close();";
  ExecuteScript(javascript);
}

void Instance::Print() {
  if (!printing_enabled_ ||
      (!engine_->HasPermission(PDFEngine::PERMISSION_PRINT_LOW_QUALITY) &&
       !engine_->HasPermission(PDFEngine::PERMISSION_PRINT_HIGH_QUALITY))) {
    return;
  }

  pp::CompletionCallback callback =
      callback_factory_.NewCallback(&Instance::OnPrint);
  pp::Module::Get()->core()->CallOnMainThread(0, callback);
}

void Instance::OnPrint(int32_t) {
  pp::PDF::Print(this);
}

void Instance::SaveAs() {
  pp::PDF::SaveAs(this);
}

void Instance::SubmitForm(const std::string& url,
                          const void* data,
                          int length) {
  pp::URLRequestInfo request(this);
  request.SetURL(url);
  request.SetMethod("POST");
  request.AppendDataToBody(reinterpret_cast<const char*>(data), length);

  pp::CompletionCallback callback =
      form_factory_.NewCallback(&Instance::FormDidOpen);
  form_loader_ = CreateURLLoaderInternal();
  int rv = form_loader_.Open(request, callback);
  if (rv != PP_OK_COMPLETIONPENDING)
    callback.Run(rv);
}

void Instance::FormDidOpen(int32_t result) {
  // TODO: inform the user of success/failure.
  if (result != PP_OK) {
    NOTREACHED();
  }
}

std::string Instance::ShowFileSelectionDialog() {
  // Seems like very low priority to implement, since the pdf has no way to get
  // the file data anyways.  Javascript doesn't let you do this synchronously.
  NOTREACHED();
  return std::string();
}

pp::URLLoader Instance::CreateURLLoader() {
  if (full_) {
    if (!did_call_start_loading_) {
      did_call_start_loading_ = true;
      pp::PDF::DidStartLoading(this);
    }

    // Disable save and print until the document is fully loaded, since they
    // would generate an incomplete document.  Need to do this each time we
    // call DidStartLoading since that resets the content restrictions.
    pp::PDF::SetContentRestriction(this, CONTENT_RESTRICTION_SAVE |
                                   CONTENT_RESTRICTION_PRINT);
  }

  return CreateURLLoaderInternal();
}

void Instance::ScheduleCallback(int id, int delay_in_ms) {
  pp::CompletionCallback callback =
      timer_factory_.NewCallback(&Instance::OnClientTimerFired);
  pp::Module::Get()->core()->CallOnMainThread(delay_in_ms, callback, id);
}

void Instance::SearchString(const base::char16* string,
                            const base::char16* term,
                            bool case_sensitive,
                            std::vector<SearchStringResult>* results) {
  if (!pp::PDF::IsAvailable()) {
    NOTREACHED();
    return;
  }

  PP_PrivateFindResult* pp_results;
  int count = 0;
  pp::PDF::SearchString(
      this,
      reinterpret_cast<const unsigned short*>(string),
      reinterpret_cast<const unsigned short*>(term),
      case_sensitive,
      &pp_results,
      &count);

  results->resize(count);
  for (int i = 0; i < count; ++i) {
    (*results)[i].start_index = pp_results[i].start_index;
    (*results)[i].length = pp_results[i].length;
  }

  pp::Memory_Dev memory;
  memory.MemFree(pp_results);
}

void Instance::DocumentPaintOccurred() {
  if (painted_first_page_)
    return;

  painted_first_page_ = true;
  UpdateToolbarPosition(false);
  toolbar_->Splash(kToolbarSplashTimeoutMs);

  if (engine_->GetNumberOfPages() > 1)
    show_page_indicator_ = true;
  else
    show_page_indicator_ = false;

  if (v_scrollbar_.get() && show_page_indicator_) {
    page_indicator_.set_current_page(GetPageNumberToDisplay());
    page_indicator_.Splash(kToolbarSplashTimeoutMs,
                           kPageIndicatorInitialFadeTimeoutMs);
  }
}

void Instance::DocumentLoadComplete(int page_count) {
  // Clear focus state for OSK.
  FormTextFieldFocusChange(false);

  // Update progress control.
  if (progress_bar_.visible())
    progress_bar_.Fade(false, kProgressFadeTimeoutMs);

  DCHECK(document_load_state_ == LOAD_STATE_LOADING);
  document_load_state_ = LOAD_STATE_COMPLETE;
  UserMetricsRecordAction("PDF.LoadSuccess");

  if (did_call_start_loading_) {
    pp::PDF::DidStopLoading(this);
    did_call_start_loading_ = false;
  }

  if (on_load_callback_.is_string())
    ExecuteScript(on_load_callback_);
  // Note: If we are in print preview mode on_load_callback_ might call
  // ScrollTo{X|Y}() and we don't want to scroll again and override it.
  // #page=N is not supported in Print Preview.
  if (!IsPrintPreview()) {
    int initial_page = GetInitialPage(url_);
    if (initial_page >= 0)
      ScrollToPage(initial_page);
  }

  if (!full_)
    return;
  if (!pp::PDF::IsAvailable())
    return;

  int content_restrictions =
      CONTENT_RESTRICTION_CUT | CONTENT_RESTRICTION_PASTE;
  if (!engine_->HasPermission(PDFEngine::PERMISSION_COPY))
    content_restrictions |= CONTENT_RESTRICTION_COPY;

  if (!engine_->HasPermission(PDFEngine::PERMISSION_PRINT_LOW_QUALITY) &&
      !engine_->HasPermission(PDFEngine::PERMISSION_PRINT_HIGH_QUALITY)) {
    printing_enabled_ = false;
    if (current_tb_info_ == kPDFToolbarButtons) {
      // Remove Print button.
      CreateToolbar(kPDFNoPrintToolbarButtons,
                    arraysize(kPDFNoPrintToolbarButtons));
      UpdateToolbarPosition(false);
      Invalidate(pp::Rect(plugin_size_));
    }
  }

  pp::PDF::SetContentRestriction(this, content_restrictions);

  pp::PDF::HistogramPDFPageCount(this, page_count);
}

void Instance::RotateClockwise() {
  engine_->RotateClockwise();
}

void Instance::RotateCounterclockwise() {
  engine_->RotateCounterclockwise();
}

void Instance::PreviewDocumentLoadComplete() {
  if (preview_document_load_state_ != LOAD_STATE_LOADING ||
      preview_pages_info_.empty()) {
    return;
  }

  preview_document_load_state_ = LOAD_STATE_COMPLETE;

  int dest_page_index = preview_pages_info_.front().second;
  int src_page_index =
      ExtractPrintPreviewPageIndex(preview_pages_info_.front().first);
  if (src_page_index > 0 &&  dest_page_index > -1 && preview_engine_.get())
    engine_->AppendPage(preview_engine_.get(), dest_page_index);

  preview_pages_info_.pop();
  // |print_preview_page_count_| is not updated yet. Do not load any
  // other preview pages till we get this information.
  if (print_preview_page_count_ == 0)
    return;

  if (preview_pages_info_.size())
    LoadAvailablePreviewPage();
}

void Instance::DocumentLoadFailed() {
  DCHECK(document_load_state_ == LOAD_STATE_LOADING);
  UserMetricsRecordAction("PDF.LoadFailure");

  // Hide progress control.
  progress_bar_.Fade(false, kProgressFadeTimeoutMs);

  if (did_call_start_loading_) {
    pp::PDF::DidStopLoading(this);
    did_call_start_loading_ = false;
  }

  document_load_state_ = LOAD_STATE_FAILED;
  paint_manager_.InvalidateRect(pp::Rect(pp::Point(), plugin_size_));
}

void Instance::PreviewDocumentLoadFailed() {
  UserMetricsRecordAction("PDF.PreviewDocumentLoadFailure");
  if (preview_document_load_state_ != LOAD_STATE_LOADING ||
      preview_pages_info_.empty()) {
    return;
  }

  preview_document_load_state_ = LOAD_STATE_FAILED;
  preview_pages_info_.pop();

  if (preview_pages_info_.size())
    LoadAvailablePreviewPage();
}

pp::Instance* Instance::GetPluginInstance() {
  return GetInstance();
}

void Instance::DocumentHasUnsupportedFeature(const std::string& feature) {
  std::string metric("PDF_Unsupported_");
  metric += feature;
  if (!unsupported_features_reported_.count(metric)) {
    unsupported_features_reported_.insert(metric);
    UserMetricsRecordAction(metric);
  }

  // Since we use an info bar, only do this for full frame plugins..
  if (!full_)
    return;

  if (told_browser_about_unsupported_feature_)
    return;
  told_browser_about_unsupported_feature_ = true;

  pp::PDF::HasUnsupportedFeature(this);
}

void Instance::DocumentLoadProgress(uint32 available, uint32 doc_size) {
  double progress = 0.0;
  if (doc_size == 0) {
    // Document size is unknown. Use heuristics.
    // We'll make progress logarithmic from 0 to 100M.
    static const double kFactor = log(100000000.0) / 100.0;
    if (available > 0) {
      progress = log(static_cast<double>(available)) / kFactor;
      if (progress > 100.0)
        progress = 100.0;
    }
  } else {
    progress = 100.0 * static_cast<double>(available) / doc_size;
  }
  progress_bar_.SetProgress(progress);
}

void Instance::FormTextFieldFocusChange(bool in_focus) {
  if (!text_input_.get())
    return;
  if (in_focus)
    text_input_->SetTextInputType(PP_TEXTINPUT_TYPE_DEV_TEXT);
  else
    text_input_->SetTextInputType(PP_TEXTINPUT_TYPE_DEV_NONE);
}

// Called by PDFScriptableObject.
bool Instance::HasScriptableMethod(const pp::Var& method, pp::Var* exception) {
  std::string method_str = method.AsString();
  return (method_str == kJSAccessibility ||
          method_str == kJSDocumentLoadComplete ||
          method_str == kJSGetHeight ||
          method_str == kJSGetHorizontalScrollbarThickness ||
          method_str == kJSGetPageLocationNormalized ||
          method_str == kJSGetSelectedText ||
          method_str == kJSGetVerticalScrollbarThickness ||
          method_str == kJSGetWidth ||
          method_str == kJSGetZoomLevel ||
          method_str == kJSGoToPage ||
          method_str == kJSGrayscale ||
          method_str == kJSLoadPreviewPage ||
          method_str == kJSOnLoad ||
          method_str == kJSOnPluginSizeChanged ||
          method_str == kJSOnScroll ||
          method_str == kJSPageXOffset ||
          method_str == kJSPageYOffset ||
          method_str == kJSPrintPreviewPageCount ||
          method_str == kJSReload ||
          method_str == kJSRemovePrintButton ||
          method_str == kJSResetPrintPreviewUrl ||
          method_str == kJSSendKeyEvent ||
          method_str == kJSSetPageNumbers ||
          method_str == kJSSetPageXOffset ||
          method_str == kJSSetPageYOffset ||
          method_str == kJSSetZoomLevel ||
          method_str == kJSZoomFitToHeight ||
          method_str == kJSZoomFitToWidth ||
          method_str == kJSZoomIn ||
          method_str == kJSZoomOut);
}

pp::Var Instance::CallScriptableMethod(const pp::Var& method,
                                       const std::vector<pp::Var>& args,
                                       pp::Var* exception) {
  std::string method_str = method.AsString();
  if (method_str == kJSGrayscale) {
    if (args.size() == 1 && args[0].is_bool()) {
      engine_->SetGrayscale(args[0].AsBool());
      // Redraw
      paint_manager_.InvalidateRect(pp::Rect(pp::Point(), plugin_size_));
#ifdef ENABLE_THUMBNAILS
      if (thumbnails_.visible())
        thumbnails_.Show(true, true);
#endif
      return pp::Var(true);
    }
    return pp::Var(false);
  }
  if (method_str == kJSOnLoad) {
    if (args.size() == 1 && args[0].is_string()) {
      on_load_callback_ = args[0];
      return pp::Var(true);
    }
    return pp::Var(false);
  }
  if (method_str == kJSOnScroll) {
    if (args.size() == 1 && args[0].is_string()) {
      on_scroll_callback_ = args[0];
      return pp::Var(true);
    }
    return pp::Var(false);
  }
  if (method_str == kJSOnPluginSizeChanged) {
    if (args.size() == 1 && args[0].is_string()) {
      on_plugin_size_changed_callback_ = args[0];
      return pp::Var(true);
    }
    return pp::Var(false);
  }
  if (method_str == kJSReload) {
    document_load_state_ = LOAD_STATE_LOADING;
    if (!full_)
      LoadUrl(url_);
    preview_engine_.reset();
    print_preview_page_count_ = 0;
    engine_.reset(PDFEngine::Create(this));
    engine_->New(url_.c_str());
#ifdef ENABLE_THUMBNAILS
    thumbnails_.ResetEngine(engine_.get());
#endif
    return pp::Var();
  }
  if (method_str == kJSResetPrintPreviewUrl) {
    if (args.size() == 1 && args[0].is_string()) {
      url_ = args[0].AsString();
      preview_pages_info_ = std::queue<PreviewPageInfo>();
      preview_document_load_state_ = LOAD_STATE_COMPLETE;
    }
    return pp::Var();
  }
  if (method_str == kJSZoomFitToHeight) {
    SetZoom(ZOOM_FIT_TO_PAGE, 0);
    return pp::Var();
  }
  if (method_str == kJSZoomFitToWidth) {
    SetZoom(ZOOM_FIT_TO_WIDTH, 0);
    return pp::Var();
  }
  if (method_str == kJSZoomIn) {
    SetZoom(ZOOM_SCALE, CalculateZoom(kZoomInButtonId));
    return pp::Var();
  }
  if (method_str == kJSZoomOut) {
    SetZoom(ZOOM_SCALE, CalculateZoom(kZoomOutButtonId));
    return pp::Var();
  }
  if (method_str == kJSSetZoomLevel) {
    if (args.size() == 1 && args[0].is_number())
      SetZoom(ZOOM_SCALE, args[0].AsDouble());
    return pp::Var();
  }
  if (method_str == kJSGetZoomLevel) {
    return pp::Var(zoom_);
  }
  if (method_str == kJSGetHeight) {
    return pp::Var(plugin_size_.height());
  }
  if (method_str == kJSGetWidth) {
    return pp::Var(plugin_size_.width());
  }
  if (method_str == kJSGetHorizontalScrollbarThickness) {
    return pp::Var(
          h_scrollbar_.get() ? GetScrollbarReservedThickness() : 0);
  }
  if (method_str == kJSGetVerticalScrollbarThickness) {
    return pp::Var(
          v_scrollbar_.get() ? GetScrollbarReservedThickness() : 0);
  }
  if (method_str == kJSGetSelectedText) {
    return GetSelectedText(false);
  }
  if (method_str == kJSDocumentLoadComplete) {
    return pp::Var((document_load_state_ != LOAD_STATE_LOADING));
  }
  if (method_str == kJSPageYOffset) {
    return pp::Var(static_cast<int32_t>(
        v_scrollbar_.get() ? v_scrollbar_->GetValue() : 0));
  }
  if (method_str == kJSSetPageYOffset) {
    if (args.size() == 1 && args[0].is_number() && v_scrollbar_.get())
      ScrollToY(GetScaled(args[0].AsInt()));
    return pp::Var();
  }
  if (method_str == kJSPageXOffset) {
    return pp::Var(static_cast<int32_t>(
        h_scrollbar_.get() ? h_scrollbar_->GetValue() : 0));
  }
  if (method_str == kJSSetPageXOffset) {
    if (args.size() == 1 && args[0].is_number() && h_scrollbar_.get())
      ScrollToX(GetScaled(args[0].AsInt()));
    return pp::Var();
  }
  if (method_str == kJSRemovePrintButton) {
    CreateToolbar(kPrintPreviewToolbarButtons,
                  arraysize(kPrintPreviewToolbarButtons));
    UpdateToolbarPosition(false);
    Invalidate(pp::Rect(plugin_size_));
    return pp::Var();
  }
  if (method_str == kJSGoToPage) {
    if (args.size() == 1 && args[0].is_string()) {
      ScrollToPage(atoi(args[0].AsString().c_str()));
    }
    return pp::Var();
  }
  if (method_str == kJSAccessibility) {
    if (args.size() == 0) {
      base::DictionaryValue node;
      node.SetInteger(kAccessibleNumberOfPages, engine_->GetNumberOfPages());
      node.SetBoolean(kAccessibleLoaded,
                      document_load_state_ != LOAD_STATE_LOADING);
      bool has_permissions =
          engine_->HasPermission(PDFEngine::PERMISSION_COPY) ||
          engine_->HasPermission(PDFEngine::PERMISSION_COPY_ACCESSIBLE);
      node.SetBoolean(kAccessibleCopyable, has_permissions);
      std::string json;
      base::JSONWriter::Write(&node, &json);
      return pp::Var(json);
    } else if (args[0].is_number()) {
      return pp::Var(engine_->GetPageAsJSON(args[0].AsInt()));
    }
  }
  if (method_str == kJSPrintPreviewPageCount) {
    if (args.size() == 1 && args[0].is_number())
      SetPrintPreviewMode(args[0].AsInt());
    return pp::Var();
  }
  if (method_str == kJSLoadPreviewPage) {
    if (args.size() == 2 && args[0].is_string() && args[1].is_number())
      ProcessPreviewPageInfo(args[0].AsString(), args[1].AsInt());
    return pp::Var();
  }
  if (method_str == kJSGetPageLocationNormalized) {
    const size_t kMaxLength = 30;
    char location_info[kMaxLength];
    int page_idx = engine_->GetMostVisiblePage();
    if (page_idx < 0)
      return pp::Var(std::string());
    pp::Rect rect = engine_->GetPageContentsRect(page_idx);
    int v_scrollbar_reserved_thickness =
        v_scrollbar_.get() ? GetScaled(GetScrollbarReservedThickness()) : 0;

    rect.set_x(rect.x() + ((plugin_size_.width() -
        v_scrollbar_reserved_thickness - available_area_.width()) / 2));
    base::snprintf(location_info,
                   kMaxLength,
                   "%0.4f;%0.4f;%0.4f;%0.4f;",
                   rect.x() / static_cast<float>(plugin_size_.width()),
                   rect.y() / static_cast<float>(plugin_size_.height()),
                   rect.width() / static_cast<float>(plugin_size_.width()),
                   rect.height()/ static_cast<float>(plugin_size_.height()));
    return pp::Var(std::string(location_info));
  }
  if (method_str == kJSSetPageNumbers) {
    if (args.size() != 1 || !args[0].is_string())
      return pp::Var();
    const int num_pages_signed = engine_->GetNumberOfPages();
    if (num_pages_signed <= 0)
      return pp::Var();
    scoped_ptr<base::ListValue> page_ranges(static_cast<base::ListValue*>(
        base::JSONReader::Read(args[0].AsString(), false)));
    const size_t num_pages = static_cast<size_t>(num_pages_signed);
    if (!page_ranges.get() || page_ranges->GetSize() != num_pages)
      return pp::Var();

    std::vector<int> print_preview_page_numbers;
    for (size_t index = 0; index < num_pages; ++index) {
      int page_number = 0;  // |page_number| is 1-based.
      if (!page_ranges->GetInteger(index, &page_number) || page_number < 1)
        return pp::Var();
      print_preview_page_numbers.push_back(page_number);
    }
    print_preview_page_numbers_ = print_preview_page_numbers;
    page_indicator_.set_current_page(GetPageNumberToDisplay());
    return pp::Var();
  }
  // This is here to work around https://bugs.webkit.org/show_bug.cgi?id=16735.
  // In JS, creating a synthetic keyboard event and dispatching it always
  // result in a keycode of 0.
  if (method_str == kJSSendKeyEvent) {
    if (args.size() == 1 && args[0].is_number()) {
      pp::KeyboardInputEvent event(
          this,                        // instance
          PP_INPUTEVENT_TYPE_KEYDOWN,  // HandleInputEvent only care about this.
          0,                           // timestamp, not used for kbd events.
          0,                           // no modifiers.
          args[0].AsInt(),             // keycode.
          pp::Var());                  // no char text needed.
      HandleInputEvent(event);
    }
  }
  return pp::Var();
}

void Instance::OnGeometryChanged(double old_zoom, float old_device_scale) {
  bool force_no_horizontal_scrollbar = false;
  int scrollbar_thickness = GetScrollbarThickness();

  if (old_device_scale != device_scale_) {
    // Change in device scale forces us to recreate resources
    ConfigureNumberImageGenerator();

    CreateToolbar(current_tb_info_, current_tb_info_size_);
    // Load autoscroll anchor image.
    autoscroll_anchor_ =
        CreateResourceImage(PP_RESOURCEIMAGE_PDF_PAN_SCROLL_ICON);

    ConfigurePageIndicator();
    ConfigureProgressBar();

    pp::Point scroll_position = engine_->GetScrollPosition();
    ScalePoint(device_scale_ / old_device_scale, &scroll_position);
    engine_->SetScrollPosition(scroll_position);
  }

  UpdateZoomScale();
  if (zoom_ != old_zoom || device_scale_ != old_device_scale)
    engine_->ZoomUpdated(zoom_ * device_scale_);
  if (zoom_ != old_zoom)
    ZoomChanged(zoom_);

  available_area_ = pp::Rect(plugin_size_);
  if (GetDocumentPixelHeight() > plugin_size_.height()) {
    CreateVerticalScrollbar();
  } else {
    DestroyVerticalScrollbar();
  }

  int v_scrollbar_reserved_thickness =
      v_scrollbar_.get() ? GetScaled(GetScrollbarReservedThickness()) : 0;

  if (!force_no_horizontal_scrollbar &&
      GetDocumentPixelWidth() >
      (plugin_size_.width() - v_scrollbar_reserved_thickness)) {
    CreateHorizontalScrollbar();

    // Adding the horizontal scrollbar now might cause us to need vertical
    // scrollbars.
    if (GetDocumentPixelHeight() >
        plugin_size_.height() - GetScaled(GetScrollbarReservedThickness())) {
      CreateVerticalScrollbar();
    }

  } else {
    DestroyHorizontalScrollbar();
  }

#ifdef ENABLE_THUMBNAILS
  int thumbnails_pos = 0, thumbnails_total = 0;
#endif
  if (v_scrollbar_.get()) {
    v_scrollbar_->SetScale(device_scale_);
    available_area_.set_width(
      std::max(0, plugin_size_.width() - v_scrollbar_reserved_thickness));

#ifdef ENABLE_THUMBNAILS
    int height = plugin_size_.height();
#endif
    int height_dip = plugin_dip_size_.height();

#if defined(OS_MACOSX)
    // Before Lion, Mac always had the resize at the bottom. After that, it
    // never did.
    if ((base::mac::IsOSSnowLeopard() && full_) ||
        (base::mac::IsOSLionOrLater() && h_scrollbar_.get())) {
#else
    if (h_scrollbar_.get()) {
#endif  // defined(OS_MACOSX)
#ifdef ENABLE_THUMBNAILS
      height -= GetScaled(GetScrollbarThickness());
#endif
      height_dip -= GetScrollbarThickness();
    }
#ifdef ENABLE_THUMBNAILS
    int32 doc_height = GetDocumentPixelHeight();
#endif
    int32 doc_height_dip =
        static_cast<int32>(GetDocumentPixelHeight() / device_scale_);
#if defined(OS_MACOSX)
    // On the Mac we always allow room for the resize button (whose width is
    // the same as that of the scrollbar) in full mode. However, if there is no
    // no horizontal scrollbar, the end of the scrollbar will scroll past the
    // end of the document. This is because the scrollbar assumes that its own
    // height (in the case of a vscroll bar) is the same as the height of the
    // viewport. Since the viewport is actually larger, we compensate by
    // adjusting the document height. Similar logic applies below for the
    // horizontal scrollbar.
    // For example, if the document size is 1000, and the viewport size is 200,
    // then the scrollbar position at the end will be 800. In this case the
    // viewport is actally 215 (assuming 15 as the scrollbar width) but the
    // scrollbar thinks it is 200. We want the scrollbar position at the end to
    // be 785. Making the document size 985 achieves this.
    if (full_ && !h_scrollbar_.get()) {
#ifdef ENABLE_THUMBNAILS
      doc_height -= GetScaled(GetScrollbarThickness());
#endif
      doc_height_dip -= GetScrollbarThickness();
    }
#endif  // defined(OS_MACOSX)

    int32 position;
    position = v_scrollbar_->GetValue();
    position = static_cast<int>(position * zoom_ / old_zoom);
    valid_v_range_ = doc_height_dip - height_dip;
    if (position > valid_v_range_)
      position = valid_v_range_;

    v_scrollbar_->SetValue(position);

    PP_Rect loc;
    loc.point.x = static_cast<int>(available_area_.right() / device_scale_);
    if (IsOverlayScrollbar())
      loc.point.x -= scrollbar_thickness;
    loc.point.y = 0;
    loc.size.width = scrollbar_thickness;
    loc.size.height = height_dip;
    v_scrollbar_->SetLocation(loc);
    v_scrollbar_->SetDocumentSize(doc_height_dip);

#ifdef ENABLE_THUMBNAILS
    thumbnails_pos = position;
    thumbnails_total = doc_height - height;
#endif
  }

  if (h_scrollbar_.get()) {
    h_scrollbar_->SetScale(device_scale_);
    available_area_.set_height(
        std::max(0, plugin_size_.height() -
                    GetScaled(GetScrollbarReservedThickness())));

    int width_dip = plugin_dip_size_.width();

    // See note above.
#if defined(OS_MACOSX)
    if ((base::mac::IsOSSnowLeopard() && full_) ||
        (base::mac::IsOSLionOrLater() && v_scrollbar_.get())) {
#else
    if (v_scrollbar_.get()) {
#endif
      width_dip -= GetScrollbarThickness();
    }
    int32 doc_width_dip =
        static_cast<int32>(GetDocumentPixelWidth() / device_scale_);
#if defined(OS_MACOSX)
    // See comment in the above if (v_scrollbar_.get()) block.
    if (full_ && !v_scrollbar_.get())
      doc_width_dip -= GetScrollbarThickness();
#endif  // defined(OS_MACOSX)

    int32 position;
    position = h_scrollbar_->GetValue();
    position = static_cast<int>(position * zoom_ / old_zoom);
    position = std::min(position, doc_width_dip - width_dip);

    h_scrollbar_->SetValue(position);

    PP_Rect loc;
    loc.point.x = 0;
    loc.point.y = static_cast<int>(available_area_.bottom() / device_scale_);
    if (IsOverlayScrollbar())
      loc.point.y -= scrollbar_thickness;
    loc.size.width = width_dip;
    loc.size.height = scrollbar_thickness;
    h_scrollbar_->SetLocation(loc);
    h_scrollbar_->SetDocumentSize(doc_width_dip);
  }

  int doc_width = GetDocumentPixelWidth();
  if (doc_width < available_area_.width()) {
    available_area_.Offset((available_area_.width() - doc_width) / 2, 0);
    available_area_.set_width(doc_width);
  }
  int doc_height = GetDocumentPixelHeight();
  if (doc_height < available_area_.height()) {
    available_area_.set_height(doc_height);
  }

  // We'll invalidate the entire plugin anyways.
  UpdateToolbarPosition(false);
  UpdateProgressBarPosition(false);
  UpdatePageIndicatorPosition(false);

#ifdef ENABLE_THUMBNAILS
  // Update thumbnail control position.
  thumbnails_.SetPosition(thumbnails_pos, thumbnails_total, false);
  pp::Rect thumbnails_rc(plugin_size_.width() - GetScaled(kThumbnailsWidth), 0,
    GetScaled(kThumbnailsWidth), plugin_size_.height());
  if (v_scrollbar_.get())
    thumbnails_rc.Offset(-v_scrollbar_reserved_thickness, 0);
  if (h_scrollbar_.get())
    thumbnails_rc.Inset(0, 0, 0, v_scrollbar_reserved_thickness);
  thumbnails_.SetRect(thumbnails_rc, false);
#endif

  CalculateBackgroundParts();
  engine_->PageOffsetUpdated(available_area_.point());
  engine_->PluginSizeUpdated(available_area_.size());

  if (!document_size_.GetArea())
    return;
  paint_manager_.InvalidateRect(pp::Rect(pp::Point(), plugin_size_));

  // Run the plugin size change callback asynchronously. This function can be
  // invoked by a layout change which should not re-enter into JS synchronously.
  pp::CompletionCallback callback =
      callback_factory_.NewCallback(&Instance::RunCallback,
                                    on_plugin_size_changed_callback_);
  pp::Module::Get()->core()->CallOnMainThread(0, callback);
}

void Instance::RunCallback(int32_t, pp::Var callback) {
  if (callback.is_string())
    ExecuteScript(callback);
}

void Instance::CreateHorizontalScrollbar() {
  if (h_scrollbar_.get())
    return;

  h_scrollbar_.reset(new pp::Scrollbar_Dev(this, false));
}

void Instance::CreateVerticalScrollbar() {
  if (v_scrollbar_.get())
    return;

  v_scrollbar_.reset(new pp::Scrollbar_Dev(this, true));
}

void Instance::DestroyHorizontalScrollbar() {
  if (!h_scrollbar_.get())
    return;
  if (h_scrollbar_->GetValue())
    engine_->ScrolledToXPosition(0);
  h_scrollbar_.reset();
}

void Instance::DestroyVerticalScrollbar() {
  if (!v_scrollbar_.get())
    return;
  if (v_scrollbar_->GetValue())
    engine_->ScrolledToYPosition(0);
  v_scrollbar_.reset();
  page_indicator_.Show(false, true);
}

int Instance::GetScrollbarThickness() {
  if (scrollbar_thickness_ == -1) {
    pp::Scrollbar_Dev temp_scrollbar(this, false);
    scrollbar_thickness_ = temp_scrollbar.GetThickness();
    scrollbar_reserved_thickness_ =
      temp_scrollbar.IsOverlay() ? 0 : scrollbar_thickness_;
  }

  return scrollbar_thickness_;
}

int Instance::GetScrollbarReservedThickness() {
  GetScrollbarThickness();
  return scrollbar_reserved_thickness_;
}

bool Instance::IsOverlayScrollbar() {
  return GetScrollbarReservedThickness() == 0;
}

void Instance::CreateToolbar(const ToolbarButtonInfo* tb_info, size_t size) {
  toolbar_.reset(new FadingControls());

  DCHECK(tb_info);
  DCHECK(size >= 0);

  // Remember the current toolbar information in case we need to recreate the
  // images later.
  current_tb_info_ = tb_info;
  current_tb_info_size_ = size;

  int max_height = 0;
  pp::Point origin(kToolbarFadingOffsetLeft, kToolbarFadingOffsetTop);
  ScalePoint(device_scale_, &origin);

  std::list<Button*> buttons;
  for (size_t i = 0; i < size; i++) {
    Button* btn = new Button;
    pp::ImageData normal_face =
        CreateResourceImage(tb_info[i].normal);
    btn->CreateButton(tb_info[i].id,
                      origin,
                      true,
                      toolbar_.get(),
                      tb_info[i].style,
                      normal_face,
                      CreateResourceImage(tb_info[i].highlighted),
                      CreateResourceImage(tb_info[i].pressed));
    buttons.push_back(btn);

    origin += pp::Point(normal_face.size().width(), 0);
    max_height = std::max(max_height, normal_face.size().height());
  }

  pp::Rect rc_toolbar(0, 0,
      origin.x() + GetToolbarRightOffset(),
      origin.y() + max_height + GetToolbarBottomOffset());
  toolbar_->CreateFadingControls(
      kToolbarId, rc_toolbar, false, this, kTransparentAlpha);

  std::list<Button*>::iterator iter;
  for (iter = buttons.begin(); iter != buttons.end(); ++iter) {
    toolbar_->AddControl(*iter);
  }
}

int Instance::GetToolbarRightOffset() {
  int scrollbar_thickness = GetScrollbarThickness();
  return GetScaled(kToolbarFadingOffsetRight) + 2 * scrollbar_thickness;
}

int Instance::GetToolbarBottomOffset() {
  int scrollbar_thickness = GetScrollbarThickness();
  return GetScaled(kToolbarFadingOffsetBottom) + scrollbar_thickness;
}

std::vector<pp::ImageData> Instance::GetThumbnailResources() {
  std::vector<pp::ImageData> num_images(10);
  num_images[0] = CreateResourceImage(PP_RESOURCEIMAGE_PDF_BUTTON_THUMBNAIL_0);
  num_images[1] = CreateResourceImage(PP_RESOURCEIMAGE_PDF_BUTTON_THUMBNAIL_1);
  num_images[2] = CreateResourceImage(PP_RESOURCEIMAGE_PDF_BUTTON_THUMBNAIL_2);
  num_images[3] = CreateResourceImage(PP_RESOURCEIMAGE_PDF_BUTTON_THUMBNAIL_3);
  num_images[4] = CreateResourceImage(PP_RESOURCEIMAGE_PDF_BUTTON_THUMBNAIL_4);
  num_images[5] = CreateResourceImage(PP_RESOURCEIMAGE_PDF_BUTTON_THUMBNAIL_5);
  num_images[6] = CreateResourceImage(PP_RESOURCEIMAGE_PDF_BUTTON_THUMBNAIL_6);
  num_images[7] = CreateResourceImage(PP_RESOURCEIMAGE_PDF_BUTTON_THUMBNAIL_7);
  num_images[8] = CreateResourceImage(PP_RESOURCEIMAGE_PDF_BUTTON_THUMBNAIL_8);
  num_images[9] = CreateResourceImage(PP_RESOURCEIMAGE_PDF_BUTTON_THUMBNAIL_9);
  return num_images;
}

std::vector<pp::ImageData> Instance::GetProgressBarResources(
    pp::ImageData* background) {
  std::vector<pp::ImageData> result(9);
  result[0] = CreateResourceImage(PP_RESOURCEIMAGE_PDF_PROGRESS_BAR_0);
  result[1] = CreateResourceImage(PP_RESOURCEIMAGE_PDF_PROGRESS_BAR_1);
  result[2] = CreateResourceImage(PP_RESOURCEIMAGE_PDF_PROGRESS_BAR_2);
  result[3] = CreateResourceImage(PP_RESOURCEIMAGE_PDF_PROGRESS_BAR_3);
  result[4] = CreateResourceImage(PP_RESOURCEIMAGE_PDF_PROGRESS_BAR_4);
  result[5] = CreateResourceImage(PP_RESOURCEIMAGE_PDF_PROGRESS_BAR_5);
  result[6] = CreateResourceImage(PP_RESOURCEIMAGE_PDF_PROGRESS_BAR_6);
  result[7] = CreateResourceImage(PP_RESOURCEIMAGE_PDF_PROGRESS_BAR_7);
  result[8] = CreateResourceImage(PP_RESOURCEIMAGE_PDF_PROGRESS_BAR_8);
  *background = CreateResourceImage(
      PP_RESOURCEIMAGE_PDF_PROGRESS_BAR_BACKGROUND);
  return result;
}

void Instance::CreatePageIndicator(bool always_visible) {
  page_indicator_.CreatePageIndicator(kPageIndicatorId, false, this,
      number_image_generator(), always_visible);
  ConfigurePageIndicator();
}

void Instance::ConfigurePageIndicator() {
  pp::ImageData background =
      CreateResourceImage(PP_RESOURCEIMAGE_PDF_PAGE_INDICATOR_BACKGROUND);
  page_indicator_.Configure(pp::Point(), background);
}

void Instance::CreateProgressBar() {
  pp::ImageData background;
  std::vector<pp::ImageData> images = GetProgressBarResources(&background);
  std::string text = GetLocalizedString(PP_RESOURCESTRING_PDFPROGRESSLOADING);
  progress_bar_.CreateProgressControl(kProgressBarId, false, this, 0.0,
      device_scale_, images, background, text);
}

void Instance::ConfigureProgressBar() {
  pp::ImageData background;
  std::vector<pp::ImageData> images = GetProgressBarResources(&background);
  progress_bar_.Reconfigure(background, images, device_scale_);
}

void Instance::CreateThumbnails() {
  thumbnails_.CreateThumbnailControl(
      kThumbnailsId, pp::Rect(), false, this, engine_.get(),
      number_image_generator());
}

void Instance::LoadUrl(const std::string& url) {
  LoadUrlInternal(url, &embed_loader_, &Instance::DidOpen);
}

void Instance::LoadPreviewUrl(const std::string& url) {
  LoadUrlInternal(url, &embed_preview_loader_, &Instance::DidOpenPreview);
}

void Instance::LoadUrlInternal(const std::string& url, pp::URLLoader* loader,
                               void (Instance::* method)(int32_t)) {
  pp::URLRequestInfo request(this);
  request.SetURL(url);
  request.SetMethod("GET");

  *loader = CreateURLLoaderInternal();
  pp::CompletionCallback callback = loader_factory_.NewCallback(method);
  int rv = loader->Open(request, callback);
  if (rv != PP_OK_COMPLETIONPENDING)
    callback.Run(rv);
}

pp::URLLoader Instance::CreateURLLoaderInternal() {
  pp::URLLoader loader(this);

  const PPB_URLLoaderTrusted* trusted_interface =
      reinterpret_cast<const PPB_URLLoaderTrusted*>(
          pp::Module::Get()->GetBrowserInterface(
              PPB_URLLOADERTRUSTED_INTERFACE));
  if (trusted_interface)
    trusted_interface->GrantUniversalAccess(loader.pp_resource());
  return loader;
}

int Instance::GetInitialPage(const std::string& url) {
  size_t found_idx = url.find('#');
  if (found_idx == std::string::npos)
    return -1;

  const std::string& ref = url.substr(found_idx + 1);
  std::vector<std::string> fragments;
  Tokenize(ref, kDelimiters, &fragments);

  // Page number to return, zero-based.
  int page = -1;

  // Handle the case of http://foo.com/bar#NAMEDDEST. This is not explicitly
  // mentioned except by example in the Adobe "PDF Open Parameters" document.
  if ((fragments.size() == 1) && (fragments[0].find('=') == std::string::npos))
    return engine_->GetNamedDestinationPage(fragments[0]);

  for (size_t i = 0; i < fragments.size(); ++i) {
    std::vector<std::string> key_value;
    base::SplitString(fragments[i], '=', &key_value);
    if (key_value.size() != 2)
      continue;
    const std::string& key = key_value[0];
    const std::string& value = key_value[1];

    if (base::strcasecmp(kPage, key.c_str()) == 0) {
      // |page_value| is 1-based.
      int page_value = -1;
      if (base::StringToInt(value, &page_value) && page_value > 0)
        page = page_value - 1;
      continue;
    }
    if (base::strcasecmp(kNamedDest, key.c_str()) == 0) {
      // |page_value| is 0-based.
      int page_value = engine_->GetNamedDestinationPage(value);
      if (page_value >= 0)
        page = page_value;
      continue;
    }
  }
  return page;
}

void Instance::UpdateToolbarPosition(bool invalidate) {
  pp::Rect ctrl_rc = toolbar_->GetControlsRect();
  int min_toolbar_width = ctrl_rc.width() + GetToolbarRightOffset() +
      GetScaled(kToolbarFadingOffsetLeft);
  int min_toolbar_height = ctrl_rc.width() + GetToolbarBottomOffset() +
      GetScaled(kToolbarFadingOffsetBottom);

  // Update toolbar position
  if (plugin_size_.width() < min_toolbar_width ||
      plugin_size_.height() < min_toolbar_height) {
    // Disable toolbar if it does not fit on the screen.
    toolbar_->Show(false, invalidate);
  } else {
    pp::Point offset(
        plugin_size_.width() - GetToolbarRightOffset() - ctrl_rc.right(),
        plugin_size_.height() - GetToolbarBottomOffset() - ctrl_rc.bottom());
    toolbar_->MoveBy(offset, invalidate);

    int toolbar_width = std::max(plugin_size_.width() / 2, min_toolbar_width);
    toolbar_->ExpandLeft(toolbar_width - toolbar_->rect().width());
    toolbar_->Show(painted_first_page_, invalidate);
  }
}

void Instance::UpdateProgressBarPosition(bool invalidate) {
  // TODO(gene): verify we don't overlap with toolbar.
  int scrollbar_thickness = GetScrollbarThickness();
  pp::Point progress_origin(
      scrollbar_thickness + GetScaled(kProgressOffsetLeft),
      plugin_size_.height() - progress_bar_.rect().height() -
          scrollbar_thickness - GetScaled(kProgressOffsetBottom));
  progress_bar_.MoveTo(progress_origin, invalidate);
}

void Instance::UpdatePageIndicatorPosition(bool invalidate) {
  int32 doc_height = static_cast<int>(document_size_.height() * zoom_);
  pp::Point origin(
      plugin_size_.width() - page_indicator_.rect().width() -
          GetScaled(GetScrollbarReservedThickness()),
      page_indicator_.GetYPosition(engine_->GetVerticalScrollbarYPosition(),
                                   doc_height, plugin_size_.height()));
  page_indicator_.MoveTo(origin, invalidate);
}

void Instance::SetZoom(ZoomMode zoom_mode, double scale) {
  double old_zoom = zoom_;

  zoom_mode_ = zoom_mode;
  if (zoom_mode_ == ZOOM_SCALE)
    zoom_ = scale;
  UpdateZoomScale();

  engine_->ZoomUpdated(zoom_ * device_scale_);
  OnGeometryChanged(old_zoom, device_scale_);

  // If fit-to-height, snap to the beginning of the most visible page.
  if (zoom_mode_ == ZOOM_FIT_TO_PAGE) {
    ScrollToPage(engine_->GetMostVisiblePage());
  }

  // Update sticky buttons to the current zoom style.
  Button* ftp_btn = static_cast<Button*>(
      toolbar_->GetControl(kFitToPageButtonId));
  Button* ftw_btn = static_cast<Button*>(
      toolbar_->GetControl(kFitToWidthButtonId));
  switch (zoom_mode_) {
    case ZOOM_FIT_TO_PAGE:
      ftp_btn->SetPressedState(true);
      ftw_btn->SetPressedState(false);
      break;
    case ZOOM_FIT_TO_WIDTH:
      ftw_btn->SetPressedState(true);
      ftp_btn->SetPressedState(false);
      break;
    default:
      ftw_btn->SetPressedState(false);
      ftp_btn->SetPressedState(false);
  }
}

void Instance::UpdateZoomScale() {
  switch (zoom_mode_) {
    case ZOOM_SCALE:
      break;  // Keep current scale.
    case ZOOM_FIT_TO_PAGE: {
      int page_num = engine_->GetFirstVisiblePage();
      if (page_num == -1)
        break;
      pp::Rect rc = engine_->GetPageRect(page_num);
      if (!rc.height())
        break;
      // Calculate fit to width zoom level.
      double ftw_zoom = static_cast<double>(plugin_dip_size_.width() -
          GetScrollbarReservedThickness()) / document_size_.width();
      // Calculate fit to height zoom level. If document will not fit
      // horizontally, adjust zoom level to allow space for horizontal
      // scrollbar.
      double fth_zoom =
          static_cast<double>(plugin_dip_size_.height()) / rc.height();
      if (fth_zoom * document_size_.width() >
          plugin_dip_size_.width() - GetScrollbarReservedThickness())
        fth_zoom = static_cast<double>(plugin_dip_size_.height()
            - GetScrollbarReservedThickness()) / rc.height();
      zoom_ = std::min(ftw_zoom, fth_zoom);
    } break;
    case ZOOM_FIT_TO_WIDTH:
    case ZOOM_AUTO:
      if (!document_size_.width())
        break;
      zoom_ = static_cast<double>(plugin_dip_size_.width() -
          GetScrollbarReservedThickness()) / document_size_.width();
      if (zoom_mode_ == ZOOM_AUTO && zoom_ > 1.0)
        zoom_ = 1.0;
      break;
  }
  zoom_ = ClipToRange(zoom_, kMinZoom, kMaxZoom);
}

double Instance::CalculateZoom(uint32 control_id) const {
  if (control_id == kZoomInButtonId) {
    for (size_t i = 0; i < chrome_page_zoom::kPresetZoomFactorsSize; ++i) {
      double current_zoom = chrome_page_zoom::kPresetZoomFactors[i];
      if (current_zoom - content::kEpsilon > zoom_)
        return current_zoom;
    }
  } else {
    for (size_t i = chrome_page_zoom::kPresetZoomFactorsSize; i > 0; --i) {
      double current_zoom = chrome_page_zoom::kPresetZoomFactors[i - 1];
      if (current_zoom + content::kEpsilon < zoom_)
        return current_zoom;
    }
  }
  return zoom_;
}

pp::ImageData Instance::CreateResourceImage(PP_ResourceImage image_id) {
  pp::ImageData resource_data;
  if (hidpi_enabled_) {
    resource_data =
        pp::PDF::GetResourceImageForScale(this, image_id, device_scale_);
  }

  return resource_data.data() ? resource_data
                              : pp::PDF::GetResourceImage(this, image_id);
}

std::string Instance::GetLocalizedString(PP_ResourceString id) {
  pp::Var rv(pp::PDF::GetLocalizedString(this, id));
  if (!rv.is_string())
    return std::string();

  return rv.AsString();
}

void Instance::DrawText(const pp::Point& top_center, PP_ResourceString id) {
  std::string str(GetLocalizedString(id));

  pp::FontDescription_Dev description;
  description.set_family(PP_FONTFAMILY_SANSSERIF);
  description.set_size(kMessageTextSize * device_scale_);
  pp::Font_Dev font(this, description);
  int length = font.MeasureSimpleText(str);
  pp::Point point(top_center);
  point.set_x(point.x() - length / 2);
  DCHECK(!image_data_.is_null());
  font.DrawSimpleText(&image_data_, str, point, kMessageTextColor);
}

void Instance::SetPrintPreviewMode(int page_count) {
  if (!IsPrintPreview() || page_count <= 0) {
    print_preview_page_count_ = 0;
    return;
  }

  print_preview_page_count_ = page_count;
  ScrollToPage(0);
  engine_->AppendBlankPages(print_preview_page_count_);
  if (preview_pages_info_.size() > 0)
    LoadAvailablePreviewPage();
}

bool Instance::IsPrintPreview() {
  return IsPrintPreviewUrl(url_);
}

int Instance::GetPageNumberToDisplay() {
  int page = engine_->GetMostVisiblePage();
  if (IsPrintPreview() && !print_preview_page_numbers_.empty()) {
    page = ClipToRange<int>(page, 0, print_preview_page_numbers_.size() - 1);
    return print_preview_page_numbers_[page];
  }
  return page + 1;
}

void Instance::ProcessPreviewPageInfo(const std::string& url,
                                      int dst_page_index) {
  if (!IsPrintPreview() || print_preview_page_count_ < 0)
    return;

  int src_page_index = ExtractPrintPreviewPageIndex(url);
  if (src_page_index < 1)
    return;

  preview_pages_info_.push(std::make_pair(url, dst_page_index));
  LoadAvailablePreviewPage();
}

void Instance::LoadAvailablePreviewPage() {
  if (preview_pages_info_.size() <= 0)
    return;

  std::string url = preview_pages_info_.front().first;
  int dst_page_index = preview_pages_info_.front().second;
  int src_page_index = ExtractPrintPreviewPageIndex(url);
  if (src_page_index < 1 ||
      dst_page_index >= print_preview_page_count_ ||
      preview_document_load_state_ == LOAD_STATE_LOADING) {
    return;
  }

  preview_document_load_state_ = LOAD_STATE_LOADING;
  LoadPreviewUrl(url);
}

void Instance::EnableAutoscroll(const pp::Point& origin) {
  if (is_autoscroll_)
    return;

  pp::Size client_size = plugin_size_;
  if (v_scrollbar_.get())
    client_size.Enlarge(-GetScrollbarThickness(), 0);
  if (h_scrollbar_.get())
    client_size.Enlarge(0, -GetScrollbarThickness());

  // Do not allow autoscroll if client area is too small.
  if (autoscroll_anchor_.size().width() > client_size.width() ||
      autoscroll_anchor_.size().height() > client_size.height())
    return;

  autoscroll_rect_ = pp::Rect(
      pp::Point(origin.x() - autoscroll_anchor_.size().width() / 2,
                origin.y() - autoscroll_anchor_.size().height() / 2),
      autoscroll_anchor_.size());

  // Make sure autoscroll anchor is in the client area.
  if (autoscroll_rect_.right() > client_size.width()) {
    autoscroll_rect_.set_x(
        client_size.width() - autoscroll_anchor_.size().width());
  }
  if (autoscroll_rect_.bottom() > client_size.height()) {
    autoscroll_rect_.set_y(
        client_size.height() - autoscroll_anchor_.size().height());
  }

  if (autoscroll_rect_.x() < 0)
    autoscroll_rect_.set_x(0);
  if (autoscroll_rect_.y() < 0)
    autoscroll_rect_.set_y(0);

  is_autoscroll_ = true;
  Invalidate(kAutoScrollId, autoscroll_rect_);

  ScheduleTimer(kAutoScrollId, kAutoScrollTimeoutMs);
}

void Instance::DisableAutoscroll() {
  if (is_autoscroll_) {
    is_autoscroll_ = false;
    Invalidate(kAutoScrollId, autoscroll_rect_);
  }
}

PP_CursorType_Dev Instance::CalculateAutoscroll(const pp::Point& mouse_pos) {
  // Scroll only if mouse pointer is outside of the anchor area.
  if (autoscroll_rect_.Contains(mouse_pos)) {
    autoscroll_x_ = 0;
    autoscroll_y_ = 0;
    return PP_CURSORTYPE_MIDDLEPANNING;
  }

  // Relative position to the center of anchor area.
  pp::Point rel_pos = mouse_pos - autoscroll_rect_.CenterPoint();

  // Calculate angle from the X axis. Angle is in range from -pi to pi.
  double angle = atan2(static_cast<double>(rel_pos.y()),
                       static_cast<double>(rel_pos.x()));

  autoscroll_x_ = rel_pos.x() * kAutoScrollFactor;
  autoscroll_y_ = rel_pos.y() * kAutoScrollFactor;

  // Angle is from -pi to pi. Screen Y is increasing toward bottom,
  // so negative angle represent north direction.
  if (angle < - (M_PI * 7.0 / 8.0)) {
    // going west
    return PP_CURSORTYPE_WESTPANNING;
  } else if (angle < - (M_PI * 5.0 / 8.0)) {
    // going north-west
    return PP_CURSORTYPE_NORTHWESTPANNING;
  } else if (angle < - (M_PI * 3.0 / 8.0)) {
    // going north.
    return PP_CURSORTYPE_NORTHPANNING;
  } else if (angle < - (M_PI * 1.0 / 8.0)) {
    // going north-east
    return PP_CURSORTYPE_NORTHEASTPANNING;
  } else if (angle < M_PI * 1.0 / 8.0) {
    // going east.
    return PP_CURSORTYPE_EASTPANNING;
  } else if (angle < M_PI * 3.0 / 8.0) {
    // going south-east
    return PP_CURSORTYPE_SOUTHEASTPANNING;
  } else if (angle < M_PI * 5.0 / 8.0) {
    // going south.
    return PP_CURSORTYPE_SOUTHPANNING;
  } else if (angle < M_PI * 7.0 / 8.0) {
    // going south-west
    return PP_CURSORTYPE_SOUTHWESTPANNING;
  }

  // went around the circle, going west again
  return PP_CURSORTYPE_WESTPANNING;
}

void Instance::ConfigureNumberImageGenerator() {
  std::vector<pp::ImageData> num_images = GetThumbnailResources();
  pp::ImageData number_background = CreateResourceImage(
      PP_RESOURCEIMAGE_PDF_BUTTON_THUMBNAIL_NUM_BACKGROUND);
  number_image_generator_->Configure(number_background,
                                     num_images,
                                     device_scale_);
}

NumberImageGenerator* Instance::number_image_generator() {
  if (!number_image_generator_.get()) {
    number_image_generator_.reset(new NumberImageGenerator(this));
    ConfigureNumberImageGenerator();
  }
  return number_image_generator_.get();
}

int Instance::GetScaled(int x) const {
  return static_cast<int>(x * device_scale_);
}

void Instance::UserMetricsRecordAction(const std::string& action) {
  pp::PDF::UserMetricsRecordAction(this, pp::Var(action));
}

PDFScriptableObject::PDFScriptableObject(Instance* instance)
    : instance_(instance) {
}

PDFScriptableObject::~PDFScriptableObject() {
}

bool PDFScriptableObject::HasMethod(const pp::Var& name, pp::Var* exception) {
  return instance_->HasScriptableMethod(name, exception);
}

pp::Var PDFScriptableObject::Call(const pp::Var& method,
                                  const std::vector<pp::Var>& args,
                                  pp::Var* exception) {
  return instance_->CallScriptableMethod(method, args, exception);
}

}  // namespace chrome_pdf
