// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDF_INSTANCE_H_
#define PDF_INSTANCE_H_

#include <queue>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "base/memory/scoped_ptr.h"
#include "pdf/button.h"
#include "pdf/fading_controls.h"
#include "pdf/page_indicator.h"
#include "pdf/paint_manager.h"
#include "pdf/pdf_engine.h"
#include "pdf/preview_mode_client.h"
#include "pdf/progress_control.h"
#include "pdf/thumbnail_control.h"

#include "ppapi/c/private/ppb_pdf.h"
#include "ppapi/cpp/dev/printing_dev.h"
#include "ppapi/cpp/dev/scriptable_object_deprecated.h"
#include "ppapi/cpp/dev/scrollbar_dev.h"
#include "ppapi/cpp/dev/selection_dev.h"
#include "ppapi/cpp/dev/widget_client_dev.h"
#include "ppapi/cpp/dev/zoom_dev.h"
#include "ppapi/cpp/graphics_2d.h"
#include "ppapi/cpp/image_data.h"
#include "ppapi/cpp/input_event.h"
#include "ppapi/cpp/private/find_private.h"
#include "ppapi/cpp/private/instance_private.h"
#include "ppapi/cpp/private/var_private.h"
#include "ppapi/cpp/url_loader.h"
#include "ppapi/utility/completion_callback_factory.h"

namespace pp {
class TextInput_Dev;
}

namespace chrome_pdf {

struct ToolbarButtonInfo;

class Instance : public pp::InstancePrivate,
                 public pp::Find_Private,
                 public pp::Printing_Dev,
                 public pp::Selection_Dev,
                 public pp::WidgetClient_Dev,
                 public pp::Zoom_Dev,
                 public PaintManager::Client,
                 public PDFEngine::Client,
                 public PreviewModeClient::Client,
                 public ControlOwner {
 public:
  explicit Instance(PP_Instance instance);
  virtual ~Instance();

  // pp::Instance implementation.
  virtual bool Init(uint32_t argc,
                    const char* argn[],
                    const char* argv[]) override;
  virtual bool HandleDocumentLoad(const pp::URLLoader& loader) override;
  virtual bool HandleInputEvent(const pp::InputEvent& event) override;
  virtual void DidChangeView(const pp::View& view) override;
  virtual pp::Var GetInstanceObject() override;

  // pp::Find_Private implementation.
  virtual bool StartFind(const std::string& text, bool case_sensitive) override;
  virtual void SelectFindResult(bool forward) override;
  virtual void StopFind() override;

  // pp::PaintManager::Client implementation.
  virtual void OnPaint(const std::vector<pp::Rect>& paint_rects,
                       std::vector<PaintManager::ReadyRect>* ready,
                       std::vector<pp::Rect>* pending) override;

  // pp::Printing_Dev implementation.
  virtual uint32_t QuerySupportedPrintOutputFormats() override;
  virtual int32_t PrintBegin(
      const PP_PrintSettings_Dev& print_settings) override;
  virtual pp::Resource PrintPages(
      const PP_PrintPageNumberRange_Dev* page_ranges,
      uint32_t page_range_count) override;
  virtual void PrintEnd() override;
  virtual bool IsPrintScalingDisabled() override;

  // pp::Private implementation.
  virtual pp::Var GetLinkAtPosition(const pp::Point& point);

  // PPP_Selection_Dev implementation.
  virtual pp::Var GetSelectedText(bool html) override;

  // WidgetClient_Dev implementation.
  virtual void InvalidateWidget(pp::Widget_Dev widget,
                                const pp::Rect& dirty_rect) override;
  virtual void ScrollbarValueChanged(pp::Scrollbar_Dev scrollbar,
                                     uint32_t value) override;
  virtual void ScrollbarOverlayChanged(pp::Scrollbar_Dev scrollbar,
                                       bool overlay) override;

  // pp::Zoom_Dev implementation.
  virtual void Zoom(double scale, bool text_only) override;
  void ZoomChanged(double factor);  // Override.

  void FlushCallback(int32_t result);
  void DidOpen(int32_t result);
  void DidOpenPreview(int32_t result);
  // If the given widget intersects the rectangle, paints it and adds the
  // rect to ready.
  void PaintIfWidgetIntersects(pp::Widget_Dev* widget,
                               const pp::Rect& rect,
                               std::vector<PaintManager::ReadyRect>* ready,
                               std::vector<pp::Rect>* pending);

  // Called when the timer is fired.
  void OnTimerFired(int32_t);
  void OnClientTimerFired(int32_t id);

  // Called when the control timer is fired.
  void OnControlTimerFired(int32_t,
                           const uint32& control_id,
                           const uint32& timer_id);

  // Called to print without re-entrancy issues.
  void OnPrint(int32_t);

  // PDFEngine::Client implementation.
  virtual void DocumentSizeUpdated(const pp::Size& size);
  virtual void Invalidate(const pp::Rect& rect);
  virtual void Scroll(const pp::Point& point);
  virtual void ScrollToX(int position);
  virtual void ScrollToY(int position);
  virtual void ScrollToPage(int page);
  virtual void NavigateTo(const std::string& url, bool open_in_new_tab);
  virtual void UpdateCursor(PP_CursorType_Dev cursor);
  virtual void UpdateTickMarks(const std::vector<pp::Rect>& tickmarks);
  virtual void NotifyNumberOfFindResultsChanged(int total, bool final_result);
  virtual void NotifySelectedFindResultChanged(int current_find_index);
  virtual void GetDocumentPassword(
      pp::CompletionCallbackWithOutput<pp::Var> callback);
  virtual void Alert(const std::string& message);
  virtual bool Confirm(const std::string& message);
  virtual std::string Prompt(const std::string& question,
                             const std::string& default_answer);
  virtual std::string GetURL();
  virtual void Email(const std::string& to,
                     const std::string& cc,
                     const std::string& bcc,
                     const std::string& subject,
                     const std::string& body);
  virtual void Print();
  virtual void SubmitForm(const std::string& url,
                          const void* data,
                          int length);
  virtual std::string ShowFileSelectionDialog();
  virtual pp::URLLoader CreateURLLoader();
  virtual void ScheduleCallback(int id, int delay_in_ms);
  virtual void SearchString(const base::char16* string,
                            const base::char16* term,
                            bool case_sensitive,
                            std::vector<SearchStringResult>* results);
  virtual void DocumentPaintOccurred();
  virtual void DocumentLoadComplete(int page_count);
  virtual void DocumentLoadFailed();
  virtual pp::Instance* GetPluginInstance();
  virtual void DocumentHasUnsupportedFeature(const std::string& feature);
  virtual void DocumentLoadProgress(uint32 available, uint32 doc_size);
  virtual void FormTextFieldFocusChange(bool in_focus);
  virtual bool IsPrintPreview();

  // ControlOwner implementation.
  virtual void OnEvent(uint32 control_id, uint32 event_id, void* data);
  virtual void Invalidate(uint32 control_id, const pp::Rect& rc);
  virtual uint32 ScheduleTimer(uint32 control_id, uint32 timeout_ms);
  virtual void SetEventCapture(uint32 control_id, bool set_capture);
  virtual void SetCursor(uint32 control_id, PP_CursorType_Dev cursor_type);
  virtual pp::Instance* GetInstance();

  bool dont_paint() const { return dont_paint_; }
  void set_dont_paint(bool dont_paint) { dont_paint_ = dont_paint; }

  // Called by PDFScriptableObject.
  bool HasScriptableMethod(const pp::Var& method, pp::Var* exception);
  pp::Var CallScriptableMethod(const pp::Var& method,
                               const std::vector<pp::Var>& args,
                               pp::Var* exception);

  // PreviewModeClient::Client implementation.
  virtual void PreviewDocumentLoadComplete() override;
  virtual void PreviewDocumentLoadFailed() override;

  // Helper functions for implementing PPP_PDF.
  void RotateClockwise();
  void RotateCounterclockwise();

 private:
  // Called whenever the plugin geometry changes to update the location of the
  // scrollbars, background parts, and notifies the pdf engine.
  void OnGeometryChanged(double old_zoom, float old_device_scale);

  // Runs the given JS callback given in |callback|.
  void RunCallback(int32_t, pp::Var callback);

  void CreateHorizontalScrollbar();
  void CreateVerticalScrollbar();
  void DestroyHorizontalScrollbar();
  void DestroyVerticalScrollbar();

  // Returns the thickness of a scrollbar. This returns the thickness when it's
  // shown, so for overlay scrollbars it'll still be non-zero.
  int GetScrollbarThickness();

  // Returns the space we need to reserve for the scrollbar in the plugin area.
  // If overlay scrollbars are used, this will be 0.
  int GetScrollbarReservedThickness();

  // Returns true if overlay scrollbars are in use.
  bool IsOverlayScrollbar();

  // Figures out the location of any background rectangles (i.e. those that
  // aren't painted by the PDF engine).
  void CalculateBackgroundParts();

  // Computes document width/height in device pixels, based on current zoom and
  // device scale
  int GetDocumentPixelWidth() const;
  int GetDocumentPixelHeight() const;

  // Draws a rectangle with the specified dimensions and color in our buffer.
  void FillRect(const pp::Rect& rect, uint32 color);

  std::vector<pp::ImageData> GetThumbnailResources();
  std::vector<pp::ImageData> GetProgressBarResources(pp::ImageData* background);

  void CreateToolbar(const ToolbarButtonInfo* tb_info, size_t size);
  int GetToolbarRightOffset();
  int GetToolbarBottomOffset();
  void CreateProgressBar();
  void ConfigureProgressBar();
  void CreateThumbnails();
  void CreatePageIndicator(bool always_visible);
  void ConfigurePageIndicator();

  void PaintOverlayControl(Control* ctrl,
                           pp::ImageData* image_data,
                           std::vector<PaintManager::ReadyRect>* ready);

  void LoadUrl(const std::string& url);
  void LoadPreviewUrl(const std::string& url);
  void LoadUrlInternal(const std::string& url, pp::URLLoader* loader,
                       void (Instance::* method)(int32_t));

  // Creates a URL loader and allows it to access all urls, i.e. not just the
  // frame's origin.
  pp::URLLoader CreateURLLoaderInternal();

  // Figure out the initial page to display based on #page=N and #nameddest=foo
  // in the |url_|.
  // Returns -1 if there is no valid fragment. The returned value is 0-based,
  // whereas page=N is 1-based.
  int GetInitialPage(const std::string& url);

  void UpdateToolbarPosition(bool invalidate);
  void UpdateProgressBarPosition(bool invalidate);
  void UpdatePageIndicatorPosition(bool invalidate);

  void FormDidOpen(int32_t result);

  std::string GetLocalizedString(PP_ResourceString id);

  void UserMetricsRecordAction(const std::string& action);

  void SaveAs();

  enum ZoomMode {
    ZOOM_SCALE,  // Standard zooming mode, resize will not affect it.
    ZOOM_FIT_TO_WIDTH,  // Maintain fit to width on resize.
    ZOOM_FIT_TO_PAGE,  // Maintain fit to page on resize.
    ZOOM_AUTO  // Maintain the default auto fitting mode on resize.
  };

  enum DocumentLoadState {
    LOAD_STATE_LOADING,
    LOAD_STATE_COMPLETE,
    LOAD_STATE_FAILED,
  };

  // Set new zoom mode and scale. Scale will be ignored if mode != ZOOM_SCALE.
  void SetZoom(ZoomMode zoom_mode, double scale);

  // Updates internal zoom scale based on the plugin/document geometry and
  // current mode.
  void UpdateZoomScale();

  // Simulates how Chrome "snaps" zooming up/down to the next nearest zoom level
  // when the previous zoom level wasn't an integer.  We do this so that
  // pressing the zoom buttons has the same effect as the menu buttons, even if
  // we start from a non-standard zoom level because of fit-width or fit-height.
  double CalculateZoom(uint32 control_id) const;

  pp::ImageData CreateResourceImage(PP_ResourceImage image_id);

  void DrawText(const pp::Point& top_center, PP_ResourceString id);

  // Set print preview mode, where the current PDF document is reduced to
  // only one page, and then extended to |page_count| pages with
  // |page_count| - 1 blank pages.
  void SetPrintPreviewMode(int page_count);

  // Returns the page number to be displayed in the page indicator. If the
  // plugin is running within print preview, the displayed number might be
  // different from the index of the displayed page.
  int GetPageNumberToDisplay();

  // Process the preview page data information. |src_url| specifies the preview
  // page data location. The |src_url| is in the format:
  // chrome://print/id/page_number/print.pdf
  // |dst_page_index| specifies the blank page index that needs to be replaced
  // with the new page data.
  void ProcessPreviewPageInfo(const std::string& src_url, int dst_page_index);
  // Load the next available preview page into the blank page.
  void LoadAvailablePreviewPage();

  // Enables autoscroll using origin as a neutral (center) point.
  void EnableAutoscroll(const pp::Point& origin);
  // Disables autoscroll and returns to normal functionality.
  void DisableAutoscroll();
  // Calculate autoscroll info and return proper mouse pointer and scroll
  // andjustments.
  PP_CursorType_Dev CalculateAutoscroll(const pp::Point& mouse_pos);

  void ConfigureNumberImageGenerator();

  NumberImageGenerator* number_image_generator();

  int GetScaled(int x) const;

  pp::ImageData image_data_;
  // Used when the plugin is embedded in a page and we have to create the loader
  // ourself.
  pp::CompletionCallbackFactory<Instance> loader_factory_;
  pp::URLLoader embed_loader_;
  pp::URLLoader embed_preview_loader_;

  scoped_ptr<pp::Scrollbar_Dev> h_scrollbar_;
  scoped_ptr<pp::Scrollbar_Dev> v_scrollbar_;
  int32 valid_v_range_;

  PP_CursorType_Dev cursor_;  // The current cursor.

  // Used when selecting and dragging beyond the visible portion, in which case
  // we want to scroll the document.
  bool timer_pending_;
  pp::MouseInputEvent last_mouse_event_;
  pp::CompletionCallbackFactory<Instance> timer_factory_;
  uint32 current_timer_id_;

  // Size, in pixels, of plugin rectangle.
  pp::Size plugin_size_;
  // Size, in DIPs, of plugin rectangle.
  pp::Size plugin_dip_size_;
  // Remaining area, in pixels, to render the pdf in after accounting for
  // scrollbars/toolbars and horizontal centering.
  pp::Rect available_area_;
  // Size of entire document in pixels (i.e. if each page is 800 pixels high and
  // there are 10 pages, the height will be 8000).
  pp::Size document_size_;

  double zoom_;  // Current zoom factor.

  float device_scale_;  // Current device scale factor.
  bool printing_enabled_;
  bool hidpi_enabled_;
  // True if the plugin is full-page.
  bool full_;
  // Zooming mode (none, fit to width, fit to height)
  ZoomMode zoom_mode_;

  // If true, this means we told the RenderView that we're starting a network
  // request so that it can start the throbber. We will tell it again once the
  // document finishes loading.
  bool did_call_start_loading_;

  // Hold off on painting invalidated requests while this flag is true.
  bool dont_paint_;

  // Indicates if plugin is in autoscroll mode.
  bool is_autoscroll_;
  // Rect for autoscroll anchor.
  pp::Rect autoscroll_rect_;
  // Image of the autoscroll anchor and its background.
  pp::ImageData autoscroll_anchor_;
  // Autoscrolling deltas in pixels.
  int autoscroll_x_;
  int autoscroll_y_;

  // Thickness of a scrollbar.
  int scrollbar_thickness_;

  // Reserved thickness of a scrollbar. This is how much space the scrollbar
  // takes from the available area. 0 for overlay.
  int scrollbar_reserved_thickness_;

  // Used to remember which toolbar is in use
  const ToolbarButtonInfo* current_tb_info_;
  size_t current_tb_info_size_;

  PaintManager paint_manager_;

  struct BackgroundPart {
    pp::Rect location;
    uint32 color;
  };
  std::vector<BackgroundPart> background_parts_;

  struct PrintSettings {
    PrintSettings() {
      Clear();
    }
    void Clear() {
      is_printing = false;
      print_pages_called_ = false;
      memset(&pepper_print_settings, 0, sizeof(pepper_print_settings));
    }
    // This is set to true when PrintBegin is called and false when PrintEnd is
    // called.
    bool is_printing;
    // To know whether this was an actual print operation, so we don't double
    // count UMA logging.
    bool print_pages_called_;
    PP_PrintSettings_Dev pepper_print_settings;
  };

  PrintSettings print_settings_;

  scoped_ptr<PDFEngine> engine_;

  // This engine is used to render the individual preview page data. This is
  // used only in print preview mode. This will use |PreviewModeClient|
  // interface which has very limited access to the pp::Instance.
  scoped_ptr<PDFEngine> preview_engine_;

  std::string url_;

  scoped_ptr<FadingControls> toolbar_;
  ThumbnailControl thumbnails_;
  ProgressControl progress_bar_;
  uint32 delayed_progress_timer_id_;
  PageIndicator page_indicator_;

  // Used for creating images from numbers.
  scoped_ptr<NumberImageGenerator> number_image_generator_;

  // Used for submitting forms.
  pp::CompletionCallbackFactory<Instance> form_factory_;
  pp::URLLoader form_loader_;

  // Used for generating callbacks.
  // TODO(raymes): We don't really need other callback factories we can just
  // fold them into this one.
  pp::CompletionCallbackFactory<Instance> callback_factory_;

  // True if we haven't painted the plugin viewport yet.
  bool first_paint_;

  // True when we've painted at least one page from the document.
  bool painted_first_page_;

  // True if we should display page indicator, false otherwise
  bool show_page_indicator_;

  // Callback when the document load completes.
  pp::Var on_load_callback_;
  pp::Var on_scroll_callback_;
  pp::Var on_plugin_size_changed_callback_;

  DocumentLoadState document_load_state_;
  DocumentLoadState preview_document_load_state_;

  // JavaScript interface to control this instance.
  // This wraps a PDFScriptableObject in a pp::Var.
  pp::VarPrivate instance_object_;

  // Used so that we only tell the browser once about an unsupported feature, to
  // avoid the infobar going up more than once.
  bool told_browser_about_unsupported_feature_;

  // Keeps track of which unsupported features we reported, so we avoid spamming
  // the stats if a feature shows up many times per document.
  std::set<std::string> unsupported_features_reported_;

  // Number of pages in print preview mode, 0 if not in print preview mode.
  int print_preview_page_count_;
  std::vector<int> print_preview_page_numbers_;

  // Used to manage loaded print preview page information. A |PreviewPageInfo|
  // consists of data source url string and the page index in the destination
  // document.
  typedef std::pair<std::string, int> PreviewPageInfo;
  std::queue<PreviewPageInfo> preview_pages_info_;

  // Used to signal the browser about focus changes to trigger the OSK.
  // TODO(abodenha@chromium.org) Implement full IME support in the plugin.
  // http://crbug.com/132565
  scoped_ptr<pp::TextInput_Dev> text_input_;
};

// This implements the JavaScript class entrypoint for the plugin instance.
// This class is just a thin wrapper. It delegates relevant methods to Instance.
class PDFScriptableObject : public pp::deprecated::ScriptableObject {
 public:
  explicit PDFScriptableObject(Instance* instance);
  virtual ~PDFScriptableObject();

  // pp::deprecated::ScriptableObject implementation.
  virtual bool HasMethod(const pp::Var& method, pp::Var* exception);
  virtual pp::Var Call(const pp::Var& method,
                       const std::vector<pp::Var>& args,
                       pp::Var* exception);

 private:
  Instance* instance_;
};

}  // namespace chrome_pdf

#endif  // PDF_INSTANCE_H_
