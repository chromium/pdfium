// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDF_PDFIUM_PDFIUM_ENGINE_H_
#define PDF_PDFIUM_PDFIUM_ENGINE_H_

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "base/memory/scoped_ptr.h"
#include "base/time/time.h"
#include "pdf/document_loader.h"
#include "pdf/pdf_engine.h"
#include "pdf/pdfium/pdfium_page.h"
#include "pdf/pdfium/pdfium_range.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/dev/buffer_dev.h"
#include "ppapi/cpp/image_data.h"
#include "ppapi/cpp/point.h"
#include "third_party/pdfium/fpdfsdk/include/fpdf_dataavail.h"
#include "third_party/pdfium/fpdfsdk/include/fpdf_progressive.h"
#include "third_party/pdfium/fpdfsdk/include/fpdfformfill.h"
#include "third_party/pdfium/fpdfsdk/include/fpdfview.h"

#define _TEST_XFA
#ifdef _TEST_XFA
# if defined(WIN32)
#	define FXQA_TESTFILE(filename) "E:/"#filename
# else
#	define FXQA_TESTFILE(filename) "/home/"#filename
# endif
#endif

namespace pp {
class KeyboardInputEvent;
class MouseInputEvent;
}

namespace chrome_pdf {

class ShadowMatrix;

class PDFiumEngine : public PDFEngine,
                     public DocumentLoader::Client,
                     public FPDF_FORMFILLINFO,
                     public IPDF_JSPLATFORM,
                     public IFSDK_PAUSE {
 public:
  explicit PDFiumEngine(PDFEngine::Client* client);
  virtual ~PDFiumEngine();

  // PDFEngine implementation.
  virtual bool New(const char* url);
  virtual bool New(const char* url,
                   const char* headers);
  virtual void PageOffsetUpdated(const pp::Point& page_offset);
  virtual void PluginSizeUpdated(const pp::Size& size);
  virtual void ScrolledToXPosition(int position);
  virtual void ScrolledToYPosition(int position);
  virtual void PrePaint();
  virtual void Paint(const pp::Rect& rect,
                     pp::ImageData* image_data,
                     std::vector<pp::Rect>* ready,
                     std::vector<pp::Rect>* pending);
  virtual void PostPaint();
  virtual bool HandleDocumentLoad(const pp::URLLoader& loader);
  virtual bool HandleEvent(const pp::InputEvent& event);
  virtual uint32_t QuerySupportedPrintOutputFormats();
  virtual void PrintBegin();
  virtual pp::Resource PrintPages(
      const PP_PrintPageNumberRange_Dev* page_ranges,
      uint32_t page_range_count,
      const PP_PrintSettings_Dev& print_settings);
  virtual void PrintEnd();
  virtual void StartFind(const char* text, bool case_sensitive);
  virtual bool SelectFindResult(bool forward);
  virtual void StopFind();
  virtual void ZoomUpdated(double new_zoom_level);
  virtual void RotateClockwise();
  virtual void RotateCounterclockwise();
  virtual std::string GetSelectedText();
  virtual std::string GetLinkAtPosition(const pp::Point& point);
  virtual bool IsSelecting();
  virtual bool HasPermission(DocumentPermission permission) const;
  virtual void SelectAll();
  virtual int GetNumberOfPages();
  virtual int GetNamedDestinationPage(const std::string& destination);
  virtual int GetFirstVisiblePage();
  virtual int GetMostVisiblePage();
  virtual pp::Rect GetPageRect(int index);
  virtual pp::Rect GetPageContentsRect(int index);
  virtual int GetVerticalScrollbarYPosition() { return position_.y(); }
  virtual void PaintThumbnail(pp::ImageData* image_data, int index);
  virtual void SetGrayscale(bool grayscale);
  virtual void OnCallback(int id);
  virtual std::string GetPageAsJSON(int index);
  virtual bool GetPrintScaling();
  virtual void AppendBlankPages(int num_pages);
  virtual void AppendPage(PDFEngine* engine, int index);
  virtual pp::Point GetScrollPosition();
  virtual void SetScrollPosition(const pp::Point& position);
  virtual bool IsProgressiveLoad();

  // DocumentLoader::Client implementation.
  virtual pp::Instance* GetPluginInstance();
  virtual pp::URLLoader CreateURLLoader();
  virtual void OnPartialDocumentLoaded();
  virtual void OnPendingRequestComplete();
  virtual void OnNewDataAvailable();
  virtual void OnDocumentComplete();

  void UnsupportedFeature(int type);

  std::string current_find_text() const { return current_find_text_; }

  FPDF_DOCUMENT doc() { return doc_; }
  FPDF_FORMHANDLE form() { return form_; }

 private:
  // This helper class is used to detect the difference in selection between
  // construction and destruction.  At destruction, it invalidates all the
  // parts that are newly selected, along with all the parts that used to be
  // selected but are not anymore.
  class SelectionChangeInvalidator {
   public:
    explicit SelectionChangeInvalidator(PDFiumEngine* engine);
    ~SelectionChangeInvalidator();
   private:
    // Sets the given container to the all the currently visible selection
    // rectangles, in screen coordinates.
    void GetVisibleSelectionsScreenRects(std::vector<pp::Rect>* rects);

    PDFiumEngine* engine_;
    // Screen rectangles that were selected on construction.
    std::vector<pp::Rect> old_selections_;
    // The origin at the time this object was constructed.
    pp::Point previous_origin_;
  };

  // Used to store mouse down state to handle it in other mouse event handlers.
  class MouseDownState {
   public:
    MouseDownState(const PDFiumPage::Area& area,
                   const PDFiumPage::LinkTarget& target);
    ~MouseDownState();

    void Set(const PDFiumPage::Area& area,
             const PDFiumPage::LinkTarget& target);
    void Reset();
    bool Matches(const PDFiumPage::Area& area,
                 const PDFiumPage::LinkTarget& target) const;

   private:
    PDFiumPage::Area area_;
    PDFiumPage::LinkTarget target_;

    DISALLOW_COPY_AND_ASSIGN(MouseDownState);
  };

  friend class SelectionChangeInvalidator;

  struct FileAvail : public FX_FILEAVAIL {
    DocumentLoader* loader;
  };

  struct DownloadHints : public FX_DOWNLOADHINTS {
    DocumentLoader* loader;
  };

  // PDFium interface to get block of data.
  static int GetBlock(void* param, unsigned long position,
                      unsigned char* buffer, unsigned long size);

  // PDFium interface to check is block of data is available.
  static bool IsDataAvail(FX_FILEAVAIL* param,
                          size_t offset, size_t size);

  // PDFium interface to request download of the block of data.
  static void AddSegment(FX_DOWNLOADHINTS* param,
                         size_t offset, size_t size);

  // We finished getting the pdf file, so load it. This will complete
  // asynchronously (due to password fetching) and may be run multiple times.
  void LoadDocument();

  // Try loading the document. Returns true if the document is successfully
  // loaded or is already loaded otherwise it will return false. If
  // |with_password| is set to true, the document will be loaded with
  // |password|. If the document could not be loaded and needs a password,
  // |needs_password| will be set to true.
  bool TryLoadingDoc(bool with_password,
                     const std::string& password,
                     bool* needs_password);

  // Ask the user for the document password and then continue loading the
  // document.
  void GetPasswordAndLoad();

  // Called when the password has been retrieved.
  void OnGetPasswordComplete(int32_t result,
                             const pp::Var& password);

  // Continues loading the document when the password has been retrieved, or if
  // there is no password.
  void ContinueLoadingDocument(bool has_password,
                               const std::string& password);

  // Finish loading the document and notify the client that the document has
  // been loaded. This should only be run after |doc_| has been loaded and the
  // document is fully downloaded. If this has been run once, it will result in
  // a no-op.
  void FinishLoadingDocument();

  // Loads information about the pages in the document and calculate the
  // document size.
  void LoadPageInfo(bool reload);

  // Calculate which pages should be displayed right now.
  void CalculateVisiblePages();

  // Returns true iff the given page index is visible.  CalculateVisiblePages
  // must have been called first.
  bool IsPageVisible(int index) const;

  // Checks if a page is now available, and if so marks it as such and returns
  // true.  Otherwise, it will return false and will add the index to the given
  // array if it's not already there.
  bool CheckPageAvailable(int index, std::vector<int>* pending);

  // Helper function to get a given page's size in pixels.  This is not part of
  // PDFiumPage because we might not have that structure when we need this.
  pp::Size GetPageSize(int index);

  void UpdateTickMarks();

  // Called to continue searching so we don't block the main thread.
  void ContinueFind(int32_t result);

  // Inserts a find result into find_results_, which is sorted.
  void AddFindResult(const PDFiumRange& result);

  // Search a page using PDFium's methods.  Doesn't work with unicode.  This
  // function is just kept arount in case PDFium code is fixed.
  void SearchUsingPDFium(const base::string16& term,
                         bool case_sensitive,
                         bool first_search,
                         int character_to_start_searching_from,
                         int current_page);

  // Search a page ourself using ICU.
  void SearchUsingICU(const base::string16& term,
                      bool case_sensitive,
                      bool first_search,
                      int character_to_start_searching_from,
                      int current_page);

  // Input event handlers.
  bool OnMouseDown(const pp::MouseInputEvent& event);
  bool OnMouseUp(const pp::MouseInputEvent& event);
  bool OnMouseMove(const pp::MouseInputEvent& event);
  bool OnKeyDown(const pp::KeyboardInputEvent& event);
  bool OnKeyUp(const pp::KeyboardInputEvent& event);
  bool OnChar(const pp::KeyboardInputEvent& event);

  FPDF_DOCUMENT CreateSinglePageRasterPdf(
      double source_page_width,
      double source_page_height,
      const PP_PrintSettings_Dev& print_settings,
      PDFiumPage* page_to_print);

  pp::Buffer_Dev PrintPagesAsRasterPDF(
      const PP_PrintPageNumberRange_Dev* page_ranges,
      uint32_t page_range_count,
      const PP_PrintSettings_Dev& print_settings);

  pp::Buffer_Dev PrintPagesAsPDF(const PP_PrintPageNumberRange_Dev* page_ranges,
                                 uint32_t page_range_count,
                                 const PP_PrintSettings_Dev& print_settings);

  pp::Buffer_Dev GetFlattenedPrintData(const FPDF_DOCUMENT& doc);
  void FitContentsToPrintableAreaIfRequired(
      const FPDF_DOCUMENT& doc,
      const PP_PrintSettings_Dev& print_settings);
  void SaveSelectedFormForPrint();

  // Given a mouse event, returns which page and character location it's closest
  // to.
  PDFiumPage::Area GetCharIndex(const pp::MouseInputEvent& event,
                                int* page_index,
                                int* char_index,
                                PDFiumPage::LinkTarget* target);
  PDFiumPage::Area GetCharIndex(const pp::Point& point,
                                int* page_index,
                                int* char_index,
                                PDFiumPage::LinkTarget* target);

  void OnSingleClick(int page_index, int char_index);
  void OnMultipleClick(int click_count, int page_index, int char_index);

  // Starts a progressive paint operation given a rectangle in screen
  // coordinates. Returns the index in progressive_rects_.
  int StartPaint(int page_index, const pp::Rect& dirty);

  // Continues a paint operation that was started earlier.  Returns true if the
  // paint is done, or false if it needs to be continued.
  bool ContinuePaint(int progressive_index, pp::ImageData* image_data);

  // Called once PDFium is finished rendering a page so that we draw our
  // borders, highlighting etc.
  void FinishPaint(int progressive_index, pp::ImageData* image_data);

  // Stops any paints that are in progress.
  void CancelPaints();

  // Invalidates all pages. Use this when some global parameter, such as page
  // orientation, has changed.
  void InvalidateAllPages();

  // If the page is narrower than the document size, paint the extra space
  // with the page background.
  void FillPageSides(int progressive_index);

  void PaintPageShadow(int progressive_index, pp::ImageData* image_data);

  // Highlight visible find results and selections.
  void DrawSelections(int progressive_index, pp::ImageData* image_data);

  // Paints an page that hasn't finished downloading.
  void PaintUnavailablePage(int page_index,
                            const pp::Rect& dirty,
                            pp::ImageData* image_data);

  // Given a page index, returns the corresponding index in progressive_rects_,
  // or -1 if it doesn't exist.
  int GetProgressiveIndex(int page_index) const;

  // Creates a FPDF_BITMAP from a rectangle in screen coordinates.
  FPDF_BITMAP CreateBitmap(const pp::Rect& rect,
                           pp::ImageData* image_data) const;

  // Given a rectangle in screen coordinates, returns the coordinates in the
  // units that PDFium rendering functions expect.
  void GetPDFiumRect(int page_index, const pp::Rect& rect, int* start_x,
                     int* start_y, int* size_x, int* size_y) const;

  // Returns the rendering flags to pass to PDFium.
  int GetRenderingFlags() const;

  // Returns the currently visible rectangle in document coordinates.
  pp::Rect GetVisibleRect() const;

  // Returns a page's rect in screen coordinates, as well as its surrounding
  // border areas and bottom separator.
  pp::Rect GetPageScreenRect(int page_index) const;

  // Given a rectangle in document coordinates, returns the rectange into screen
  // coordinates (i.e. 0,0 is top left corner of plugin area).  If it's not
  // visible, an empty rectangle is returned.
  pp::Rect GetScreenRect(const pp::Rect& rect) const;

  // Highlights the given rectangle.
  void Highlight(void* buffer,
                 int stride,
                 const pp::Rect& rect,
                 std::vector<pp::Rect>* highlighted_rects);

  // Helper function to convert a device to page coordinates.  If the page is
  // not yet loaded, page_x and page_y will be set to 0.
  void DeviceToPage(int page_index,
                    float device_x,
                    float device_y,
                    double* page_x,
                    double* page_y);

  // Helper function to get the index of a given FPDF_PAGE.  Returns -1 if not
  // found.
  int GetVisiblePageIndex(FPDF_PAGE page);

  // Helper function to change the current page, running page open/close
  // triggers as necessary.
  void SetCurrentPage(int index);

  // Transform |page| contents to fit in the selected printer paper size.
  void TransformPDFPageForPrinting(FPDF_PAGE page,
                                   const PP_PrintSettings_Dev& print_settings);

  void DrawPageShadow(const pp::Rect& page_rect,
                      const pp::Rect& shadow_rect,
                      const pp::Rect& clip_rect,
                      pp::ImageData* image_data);

  void GetRegion(const pp::Point& location,
                 pp::ImageData* image_data,
                 void** region,
                 int* stride) const;

  // Called when the selection changes.
  void OnSelectionChanged();

  // FPDF_FORMFILLINFO callbacks.
  static void Form_Invalidate(FPDF_FORMFILLINFO* param,
                              FPDF_PAGE page,
                              double left,
                              double top,
                              double right,
                              double bottom);
  static void Form_OutputSelectedRect(FPDF_FORMFILLINFO* param,
                                      FPDF_PAGE page,
                                      double left,
                                      double top,
                                      double right,
                                      double bottom);
  static void Form_SetCursor(FPDF_FORMFILLINFO* param, int cursor_type);
  static int Form_SetTimer(FPDF_FORMFILLINFO* param,
                           int elapse,
                           TimerCallback timer_func);
  static void Form_KillTimer(FPDF_FORMFILLINFO* param, int timer_id);
  static FPDF_SYSTEMTIME Form_GetLocalTime(FPDF_FORMFILLINFO* param);
  static void Form_OnChange(FPDF_FORMFILLINFO* param);
  static FPDF_PAGE Form_GetPage(FPDF_FORMFILLINFO* param,
                                FPDF_DOCUMENT document,
                                int page_index);
  static FPDF_PAGE Form_GetCurrentPage(FPDF_FORMFILLINFO* param,
                                       FPDF_DOCUMENT document);
  static int Form_GetRotation(FPDF_FORMFILLINFO* param, FPDF_PAGE page);
  static void Form_ExecuteNamedAction(FPDF_FORMFILLINFO* param,
                                      FPDF_BYTESTRING named_action);
  static void Form_SetTextFieldFocus(FPDF_FORMFILLINFO* param,
                                     FPDF_WIDESTRING value,
                                     FPDF_DWORD valueLen,
                                     FPDF_BOOL is_focus);
  static void Form_DoURIAction(FPDF_FORMFILLINFO* param, FPDF_BYTESTRING uri);
  static void Form_DoGoToAction(FPDF_FORMFILLINFO* param,
                                int page_index,
                                int zoom_mode,
                                float* position_array,
                                int size_of_array);

  // IPDF_JSPLATFORM callbacks.
  static int Form_Alert(IPDF_JSPLATFORM* param,
                        FPDF_WIDESTRING message,
                        FPDF_WIDESTRING title,
                        int type,
                        int icon);
  static void Form_Beep(IPDF_JSPLATFORM* param, int type);
  static int Form_Response(IPDF_JSPLATFORM* param,
                           FPDF_WIDESTRING question,
                           FPDF_WIDESTRING title,
                           FPDF_WIDESTRING default_response,
                           FPDF_WIDESTRING label,
                           FPDF_BOOL password,
                           void* response,
                           int length);
  static int Form_GetFilePath(IPDF_JSPLATFORM* param,
                              void* file_path,
                              int length);
  static void Form_Mail(IPDF_JSPLATFORM* param,
                        void* mail_data,
                        int length,
                        FPDF_BOOL ui,
                        FPDF_WIDESTRING to,
                        FPDF_WIDESTRING subject,
                        FPDF_WIDESTRING cc,
                        FPDF_WIDESTRING bcc,
                        FPDF_WIDESTRING message);
  static void Form_Print(IPDF_JSPLATFORM* param,
                         FPDF_BOOL ui,
                         int start,
                         int end,
                         FPDF_BOOL silent,
                         FPDF_BOOL shrink_to_fit,
                         FPDF_BOOL print_as_image,
                         FPDF_BOOL reverse,
                         FPDF_BOOL annotations);
  static void Form_SubmitForm(IPDF_JSPLATFORM* param,
                              void* form_data,
                              int length,
                              FPDF_WIDESTRING url);
  static void Form_GotoPage(IPDF_JSPLATFORM* param, int page_number);
  static int Form_Browse(IPDF_JSPLATFORM* param, void* file_path, int length);
#ifdef _TEST_XFA
  static void Form_EmailTo(FPDF_FORMFILLINFO* pThis, FPDF_FILEHANDLER* fileHandler, 
	  FPDF_WIDESTRING to, FPDF_WIDESTRING subject, FPDF_WIDESTRING cc, FPDF_WIDESTRING bcc, FPDF_WIDESTRING message);
  static void Form_DisplayCaret(FPDF_FORMFILLINFO* pThis, FPDF_PAGE page, FPDF_BOOL bVisible, double left, double top, double right, double bottom);
  //static int Form_GetCurDocumentIndex(FPDF_FORMFILLINFO* pThis);
  //static int Form_GetDocumentCount(FPDF_FORMFILLINFO* pThis);
  static void Form_SetCurrentPage(FPDF_FORMFILLINFO* pThis, FPDF_DOCUMENT document, int iCurPage);
  static int Form_GetCurrentPageIndex(FPDF_FORMFILLINFO* pThis, FPDF_DOCUMENT document);
  static void Form_GetPageViewRect(FPDF_FORMFILLINFO* pThis, FPDF_PAGE page, double* left, double* top, double* right, double* bottom);
  static int Form_GetPlatform(FPDF_FORMFILLINFO* pThis, void* platform, int length);
  static FPDF_BOOL Form_PopupMenu(FPDF_FORMFILLINFO* pThis, FPDF_PAGE page, FPDF_WIDGET hWidget, int menuFlag, float x, float y);
  static FPDF_BOOL Form_PostRequestURL(FPDF_FORMFILLINFO* pThis, FPDF_WIDESTRING wsURL, FPDF_WIDESTRING wsData, FPDF_WIDESTRING wsContentType, FPDF_WIDESTRING wsEncode, FPDF_WIDESTRING wsHeader, FPDF_BSTR* respone);
  static FPDF_BOOL Form_PutRequestURL(FPDF_FORMFILLINFO* pThis, FPDF_WIDESTRING wsURL, FPDF_WIDESTRING wsData, FPDF_WIDESTRING wsEncode);
  //static FPDF_BOOL Form_ShowFileDialog(FPDF_FORMFILLINFO* pThis, FPDF_WIDESTRING wsTitle, FPDF_WIDESTRING wsFilter, FPDF_BOOL isOpen, FPDF_STRINGHANDLE pathArr);
  static void Form_UploadTo(FPDF_FORMFILLINFO* pThis, FPDF_FILEHANDLER* fileHandler, int fileFlag, FPDF_WIDESTRING uploadTo);
  static FPDF_LPFILEHANDLER Form_DownloadFromURL(FPDF_FORMFILLINFO* pThis, FPDF_WIDESTRING URL);
  //static FPDF_BOOL MyForm_GetFilePath(FPDF_FORMFILLINFO* pThis, FPDF_FILEHANDLER* pFileHandler, void* filePath, int length);
  static FPDF_FILEHANDLER* Form_OpenFile(FPDF_FORMFILLINFO* pThis, int fileFlag, FPDF_WIDESTRING wsURL, const char* mode);
  static void Form_GotoURL(FPDF_FORMFILLINFO* pThis, FPDF_DOCUMENT document, FPDF_WIDESTRING wsURL);
  static int Form_GetLanguage(FPDF_FORMFILLINFO* pThis, void* language, int length);
#endif // _TEST_XFA
  // IFSDK_PAUSE callbacks
  static FPDF_BOOL Pause_NeedToPauseNow(IFSDK_PAUSE* param);

  PDFEngine::Client* client_;
  pp::Size document_size_;  // Size of document in pixels.

  // The scroll position in screen coordinates.
  pp::Point position_;
  // The offset of the page into the viewport.
  pp::Point page_offset_;
  // The plugin size in screen coordinates.
  pp::Size plugin_size_;
  double current_zoom_;
  unsigned int current_rotation_;

  DocumentLoader doc_loader_;  // Main document's loader.
  std::string url_;
  std::string headers_;
  pp::CompletionCallbackFactory<PDFiumEngine> find_factory_;

  pp::CompletionCallbackFactory<PDFiumEngine> password_factory_;
  int32_t password_tries_remaining_;

  // The current text used for searching.
  std::string current_find_text_;

  // The PDFium wrapper object for the document.
  FPDF_DOCUMENT doc_;

  // The PDFium wrapper for form data.  Used even if there are no form controls
  // on the page.
  FPDF_FORMHANDLE form_;

  // The page(s) of the document. Store a vector of pointers so that when the
  // vector is resized we don't close the pages that are used in pending
  // paints.
  std::vector<PDFiumPage*> pages_;

  // The indexes of the pages currently visible.
  std::vector<int> visible_pages_;

  // The indexes of the pages pending download.
  std::vector<int> pending_pages_;

  // During handling of input events we don't want to unload any pages in
  // callbacks to us from PDFium, since the current page can change while PDFium
  // code still has a pointer to it.
  bool defer_page_unload_;
  std::vector<int> deferred_page_unloads_;

  // Used for selection.  There could be more than one range if selection spans
  // more than one page.
  std::vector<PDFiumRange> selection_;
  // True if we're in the middle of selection.
  bool selecting_;

  MouseDownState mouse_down_state_;

  // Used for searching.
  typedef std::vector<PDFiumRange> FindResults;
  FindResults find_results_;
  // Which page to search next.
  int next_page_to_search_;
  // Where to stop searching.
  int last_page_to_search_;
  int last_character_index_to_search_;  // -1 if search until end of page.
  // Which result the user has currently selected.
  int current_find_index_;
  // Where to resume searching.
  int resume_find_index_;

  // Permissions bitfield.
  unsigned long permissions_;

  // Interface structure to provide access to document stream.
  FPDF_FILEACCESS file_access_;
  // Interface structure to check data availability in the document stream.
  FileAvail file_availability_;
  // Interface structure to request data chunks from the document stream.
  DownloadHints download_hints_;
  // Pointer to the document availability interface.
  FPDF_AVAIL fpdf_availability_;

  pp::Size default_page_size_;

  // Used to manage timers that form fill API needs.  The pair holds the timer
  // period, in ms, and the callback function.
  std::map<int, std::pair<int, TimerCallback> > timers_;
  int next_timer_id_;

  // Holds the page index of the last page that the mouse clicked on.
  int last_page_mouse_down_;

  // Holds the page index of the first visible page; refreshed by calling
  // CalculateVisiblePages()
  int first_visible_page_;

  // Holds the page index of the most visible page; refreshed by calling
  // CalculateVisiblePages()
  int most_visible_page_;

  // Set to true after FORM_DoDocumentJSAction/FORM_DoDocumentOpenAction have
  // been called. Only after that can we call FORM_DoPageAAction.
  bool called_do_document_action_;

  // Records parts of form fields that need to be highlighted at next paint, in
  // screen coordinates.
  std::vector<pp::Rect> form_highlights_;

  // Whether to render in grayscale or in color.
  bool render_grayscale_;

  // The link currently under the cursor.
  std::string link_under_cursor_;

  // Pending progressive paints.
  struct ProgressivePaint {
    pp::Rect rect;  // In screen coordinates.
    FPDF_BITMAP bitmap;
    int page_index;
    // Temporary used to figure out if in a series of Paint() calls whether this
    // pending paint was updated or not.
    int painted_;
  };
  std::vector<ProgressivePaint> progressive_paints_;

  // Keeps track of when we started the last progressive paint, so that in our
  // callback we can determine if we need to pause.
  base::Time last_progressive_start_time_;

  // The timeout to use for the current progressive paint.
  int progressive_paint_timeout_;

  // Shadow matrix for generating the page shadow bitmap.
  scoped_ptr<ShadowMatrix> page_shadow_;

  // Set to true if the user is being prompted for their password. Will be set
  // to false after the user finishes getting their password.
  bool getting_password_;
};

// Create a local variable of this when calling PDFium functions which can call
// our global callback when an unsupported feature is reached.
class ScopedUnsupportedFeature {
 public:
  explicit ScopedUnsupportedFeature(PDFiumEngine* engine);
  ~ScopedUnsupportedFeature();
 private:
  PDFiumEngine* engine_;
  PDFiumEngine* old_engine_;
};

class PDFiumEngineExports : public PDFEngineExports {
 public:
  PDFiumEngineExports() {}
#if defined(OS_WIN)
  // See the definition of RenderPDFPageToDC in pdf.cc for details.
  virtual bool RenderPDFPageToDC(const void* pdf_buffer,
                                 int buffer_size,
                                 int page_number,
                                 const RenderingSettings& settings,
                                 HDC dc);
#endif  // OS_WIN
  virtual bool RenderPDFPageToBitmap(const void* pdf_buffer,
                                     int pdf_buffer_size,
                                     int page_number,
                                     const RenderingSettings& settings,
                                     void* bitmap_buffer);

  virtual bool GetPDFDocInfo(const void* pdf_buffer,
                             int buffer_size,
                             int* page_count,
                             double* max_page_width);

  // See the definition of GetPDFPageSizeByIndex in pdf.cc for details.
  virtual bool GetPDFPageSizeByIndex(const void* pdf_buffer,
                                     int pdf_buffer_size, int page_number,
                                     double* width, double* height);
};

}  // namespace chrome_pdf

#endif  // PDF_PDFIUM_PDFIUM_ENGINE_H_
