// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDF_PREVIEW_MODE_CLIENT_H_
#define PDF_PREVIEW_MODE_CLIENT_H_

#include <string>
#include <vector>

#include "pdf/pdf_engine.h"

namespace chrome_pdf {

// The interface that's provided to the print preview rendering engine.
class PreviewModeClient : public PDFEngine::Client {
 public:
  class Client {
   public:
    virtual void PreviewDocumentLoadFailed() = 0;
    virtual void PreviewDocumentLoadComplete() = 0;
  };
  explicit PreviewModeClient(Client* client);
  virtual ~PreviewModeClient() {}

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
  virtual void NotifyNumberOfFindResultsChanged(int total,
                                                bool final_result);
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

 private:
  Client* client_;
};

}  // namespace chrome_pdf

#endif  // PDF_PREVIEW_MODE_CLIENT_H_
