// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pdf/document_loader.h"

#include "base/logging.h"
#include "base/strings/string_util.h"
#include "net/http/http_util.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/cpp/url_loader.h"
#include "ppapi/cpp/url_request_info.h"
#include "ppapi/cpp/url_response_info.h"

namespace chrome_pdf {

// Document below size will be downloaded in one chunk.
const uint32 kMinFileSize = 64*1024;

DocumentLoader::DocumentLoader(Client* client)
    : client_(client), partial_document_(false), request_pending_(false),
      current_pos_(0), current_chunk_size_(0), current_chunk_read_(0),
      document_size_(0), header_request_(true), is_multipart_(false) {
  loader_factory_.Initialize(this);
}

DocumentLoader::~DocumentLoader() {
}

bool DocumentLoader::Init(const pp::URLLoader& loader,
                          const std::string& url,
                          const std::string& headers) {
  DCHECK(url_.empty());
  url_ = url;
  loader_ = loader;

  std::string response_headers;
  if (!headers.empty()) {
    response_headers = headers;
  } else {
    pp::URLResponseInfo response = loader_.GetResponseInfo();
    pp::Var headers_var = response.GetHeaders();

    if (headers_var.is_string()) {
      response_headers = headers_var.AsString();
    }
  }

  bool accept_ranges_bytes = false;
  bool content_encoded = false;
  uint32 content_length = 0;
  std::string type;
  std::string disposition;
  if (!response_headers.empty()) {
    net::HttpUtil::HeadersIterator it(response_headers.begin(),
                                      response_headers.end(), "\n");
    while (it.GetNext()) {
      if (LowerCaseEqualsASCII(it.name(), "content-length")) {
        content_length = atoi(it.values().c_str());
      } else if (LowerCaseEqualsASCII(it.name(), "accept-ranges")) {
        accept_ranges_bytes = LowerCaseEqualsASCII(it.values(), "bytes");
      } else if (LowerCaseEqualsASCII(it.name(), "content-encoding")) {
        content_encoded = true;
      } else if (LowerCaseEqualsASCII(it.name(), "content-type")) {
        type = it.values();
        size_t semi_colon_pos = type.find(';');
        if (semi_colon_pos != std::string::npos) {
          type = type.substr(0, semi_colon_pos);
        }
        TrimWhitespace(type, base::TRIM_ALL, &type);
      } else if (LowerCaseEqualsASCII(it.name(), "content-disposition")) {
        disposition = it.values();
      }
    }
  }
  if (!type.empty() &&
      !EndsWith(type, "/pdf", false) &&
      !EndsWith(type, ".pdf", false) &&
      !EndsWith(type, "/x-pdf", false) &&
      !EndsWith(type, "/*", false) &&
      !EndsWith(type, "/acrobat", false) &&
      !EndsWith(type, "/unknown", false)) {
    return false;
  }
  if (StartsWithASCII(disposition, "attachment", false)) {
    return false;
  }

  if (content_length > 0)
    chunk_stream_.Preallocate(content_length);

  document_size_ = content_length;
  requests_count_ = 0;

  // Enable partial loading only if file size is above the threshold.
  // It will allow avoiding latency for multiple requests.
  if (content_length > kMinFileSize &&
      accept_ranges_bytes &&
      !content_encoded) {
    LoadPartialDocument();
  } else {
    LoadFullDocument();
  }
  return true;
}

void DocumentLoader::LoadPartialDocument() {
  partial_document_ = true;
  // Force the main request to be cancelled, since if we're a full-frame plugin
  // there could be other references to the loader.
  loader_.Close();
  loader_ = pp::URLLoader();
  // Download file header.
  header_request_ = true;
  RequestData(0, std::min(GetRequestSize(), document_size_));
}

void DocumentLoader::LoadFullDocument() {
  partial_document_ = false;
  chunk_buffer_.clear();
  ReadMore();
}

bool DocumentLoader::IsDocumentComplete() const {
  if (document_size_ == 0)  // Document size unknown.
    return false;
  return IsDataAvailable(0, document_size_);
}

uint32 DocumentLoader::GetAvailableData() const {
  if (document_size_ == 0) {  // If document size is unknown.
    return current_pos_;
  }

  std::vector<std::pair<size_t, size_t> > ranges;
  chunk_stream_.GetMissedRanges(0, document_size_, &ranges);
  uint32 available = document_size_;
  std::vector<std::pair<size_t, size_t> >::iterator it;
  for (it = ranges.begin(); it != ranges.end(); ++it) {
    available -= it->second;
  }
  return available;
}

void DocumentLoader::ClearPendingRequests() {
  // The first item in the queue is pending (need to keep it in the queue).
  if (pending_requests_.size() > 1) {
    // Remove all elements except the first one.
    pending_requests_.erase(++pending_requests_.begin(),
                            pending_requests_.end());
  }
}

bool DocumentLoader::GetBlock(uint32 position, uint32 size, void* buf) const {
  return chunk_stream_.ReadData(position, size, buf);
}

bool DocumentLoader::IsDataAvailable(uint32 position, uint32 size) const {
  return chunk_stream_.IsRangeAvailable(position, size);
}

void DocumentLoader::RequestData(uint32 position, uint32 size) {
  DCHECK(partial_document_);

  // We have some artefact request from
  // PDFiumEngine::OnDocumentComplete() -> FPDFAvail_IsPageAvail after
  // document is complete.
  // We need this fix in PDFIum. Adding this as a work around.
  // Bug: http://code.google.com/p/chromium/issues/detail?id=79996
  // Test url:
  // http://www.icann.org/en/correspondence/holtzman-to-jeffrey-02mar11-en.pdf
  if (IsDocumentComplete())
    return;

  pending_requests_.push_back(std::pair<size_t, size_t>(position, size));
  DownloadPendingRequests();
}

void DocumentLoader::DownloadPendingRequests() {
  if (request_pending_ || pending_requests_.empty())
    return;

  // Remove already completed requests.
  // By design DownloadPendingRequests() should have at least 1 request in the
  // queue. ReadComplete() will remove the last pending comment from the queue.
  while (pending_requests_.size() > 1) {
    if (IsDataAvailable(pending_requests_.front().first,
                        pending_requests_.front().second)) {
      pending_requests_.pop_front();
    } else {
      break;
    }
  }

  uint32 pos = pending_requests_.front().first;
  uint32 size = pending_requests_.front().second;
  if (IsDataAvailable(pos, size)) {
    ReadComplete();
    return;
  }

  // If current request has been partially downloaded already, split it into
  // a few smaller requests.
  std::vector<std::pair<size_t, size_t> > ranges;
  chunk_stream_.GetMissedRanges(pos, size, &ranges);
  if (ranges.size() > 0) {
    pending_requests_.pop_front();
    pending_requests_.insert(pending_requests_.begin(),
                             ranges.begin(), ranges.end());
    pos = pending_requests_.front().first;
    size = pending_requests_.front().second;
  }

  uint32 cur_request_size = GetRequestSize();
  // If size is less than default request, try to expand download range for
  // more optimal download.
  if (size < cur_request_size && partial_document_) {
    // First, try to expand block towards the end of the file.
    uint32 new_pos = pos;
    uint32 new_size = cur_request_size;
    if (pos + new_size > document_size_)
      new_size = document_size_ - pos;

    std::vector<std::pair<size_t, size_t> > ranges;
    if (chunk_stream_.GetMissedRanges(new_pos, new_size, &ranges)) {
      new_pos = ranges[0].first;
      new_size = ranges[0].second;
    }

    // Second, try to expand block towards the beginning of the file.
    if (new_size < cur_request_size) {
      uint32 block_end = new_pos + new_size;
      if (block_end > cur_request_size) {
        new_pos = block_end - cur_request_size;
      } else {
        new_pos = 0;
      }
      new_size = block_end - new_pos;

      if (chunk_stream_.GetMissedRanges(new_pos, new_size, &ranges)) {
        new_pos = ranges.back().first;
        new_size = ranges.back().second;
      }
    }
    pos = new_pos;
    size = new_size;
  }

  size_t last_byte_before = chunk_stream_.GetLastByteBefore(pos);
  size_t first_byte_after = chunk_stream_.GetFirstByteAfter(pos + size - 1);
  if (pos - last_byte_before < cur_request_size) {
    size = pos + size - last_byte_before;
    pos = last_byte_before;
  }

  if ((pos + size < first_byte_after) &&
      (pos + size + cur_request_size >= first_byte_after))
    size = first_byte_after - pos;

  request_pending_ = true;

  // Start downloading first pending request.
  loader_.Close();
  loader_ = client_->CreateURLLoader();
  pp::CompletionCallback callback =
      loader_factory_.NewCallback(&DocumentLoader::DidOpen);
  pp::URLRequestInfo request = GetRequest(pos, size);
  requests_count_++;
  int rv = loader_.Open(request, callback);
  if (rv != PP_OK_COMPLETIONPENDING)
    callback.Run(rv);
}

pp::URLRequestInfo DocumentLoader::GetRequest(uint32 position,
                                              uint32 size) const {
  pp::URLRequestInfo request(client_->GetPluginInstance());
  request.SetURL(url_.c_str());
  request.SetMethod("GET");
  request.SetFollowRedirects(true);

  const size_t kBufSize = 100;
  char buf[kBufSize];
  // According to rfc2616, byte range specifies position of the first and last
  // bytes in the requested range inclusively. Therefore we should subtract 1
  // from the position + size, to get index of the last byte that needs to be
  // downloaded.
  base::snprintf(buf, kBufSize, "Range: bytes=%d-%d", position,
                 position + size - 1);
  pp::Var header(buf);
  request.SetHeaders(header);

  return request;
}

void DocumentLoader::DidOpen(int32_t result) {
  if (result != PP_OK) {
    NOTREACHED();
    return;
  }

  int32_t http_code = loader_.GetResponseInfo().GetStatusCode();
  if (http_code >= 400 && http_code < 500) {
    // Error accessing resource. 4xx error indicate subsequent requests
    // will fail too.
    // E.g. resource has been removed from the server while loading it.
    // https://code.google.com/p/chromium/issues/detail?id=414827
    return;
  }

  is_multipart_ = false;
  current_chunk_size_ = 0;
  current_chunk_read_ = 0;

  pp::Var headers_var = loader_.GetResponseInfo().GetHeaders();
  std::string headers;
  if (headers_var.is_string())
    headers = headers_var.AsString();

  std::string boundary = GetMultiPartBoundary(headers);
  if (boundary.size()) {
    // Leave position untouched for now, when we read the data we'll get it.
    is_multipart_ = true;
    multipart_boundary_ = boundary;
  } else {
    // Need to make sure that the server returned a byte-range, since it's
    // possible for a server to just ignore our bye-range request and just
    // return the entire document even if it supports byte-range requests.
    // i.e. sniff response to
    // http://www.act.org/compass/sample/pdf/geometry.pdf
    current_pos_ = 0;
    uint32 start_pos, end_pos;
    if (GetByteRange(headers, &start_pos, &end_pos)) {
      current_pos_ = start_pos;
      if (end_pos && end_pos > start_pos)
        current_chunk_size_ = end_pos - start_pos + 1;
    }
  }

  ReadMore();
}

bool DocumentLoader::GetByteRange(const std::string& headers, uint32* start,
                                  uint32* end) {
  net::HttpUtil::HeadersIterator it(headers.begin(), headers.end(), "\n");
  while (it.GetNext()) {
    if (LowerCaseEqualsASCII(it.name(), "content-range")) {
      std::string range = it.values().c_str();
      if (StartsWithASCII(range, "bytes", false)) {
        range = range.substr(strlen("bytes"));
        std::string::size_type pos = range.find('-');
        std::string range_end;
        if (pos != std::string::npos)
          range_end = range.substr(pos + 1);
        TrimWhitespaceASCII(range, base::TRIM_LEADING, &range);
        TrimWhitespaceASCII(range_end, base::TRIM_LEADING, &range_end);
        *start = atoi(range.c_str());
        *end = atoi(range_end.c_str());
        return true;
      }
    }
  }
  return false;
}

std::string DocumentLoader::GetMultiPartBoundary(const std::string& headers) {
  net::HttpUtil::HeadersIterator it(headers.begin(), headers.end(), "\n");
  while (it.GetNext()) {
    if (LowerCaseEqualsASCII(it.name(), "content-type")) {
      std::string type = base::StringToLowerASCII(it.values());
      if (StartsWithASCII(type, "multipart/", true)) {
        const char* boundary = strstr(type.c_str(), "boundary=");
        if (!boundary) {
          NOTREACHED();
          break;
        }

        return std::string(boundary + 9);
      }
    }
  }
  return std::string();
}

void DocumentLoader::ReadMore() {
  pp::CompletionCallback callback =
        loader_factory_.NewCallback(&DocumentLoader::DidRead);
  int rv = loader_.ReadResponseBody(buffer_, sizeof(buffer_), callback);
  if (rv != PP_OK_COMPLETIONPENDING)
    callback.Run(rv);
}

void DocumentLoader::DidRead(int32_t result) {
  if (result > 0) {
    char* start = buffer_;
    size_t length = result;
    if (is_multipart_ && result > 2) {
      for (int i = 2; i < result; ++i) {
        if ((buffer_[i - 1] == '\n' && buffer_[i - 2] == '\n') ||
            (i >= 4 &&
             buffer_[i - 1] == '\n' && buffer_[i - 2] == '\r' &&
             buffer_[i - 3] == '\n' && buffer_[i - 4] == '\r')) {
          uint32 start_pos, end_pos;
          if (GetByteRange(std::string(buffer_, i), &start_pos, &end_pos)) {
            current_pos_ = start_pos;
            start += i;
            length -= i;
            if (end_pos && end_pos > start_pos)
              current_chunk_size_ = end_pos - start_pos + 1;
          }
          break;
        }
      }

      // Reset this flag so we don't look inside the buffer in future calls of
      // DidRead for this response.  Note that this code DOES NOT handle multi-
      // part responses with more than one part (we don't issue them at the
      // moment, so they shouldn't arrive).
      is_multipart_ = false;
    }

    if (current_chunk_size_ &&
        current_chunk_read_ + length > current_chunk_size_)
      length = current_chunk_size_ - current_chunk_read_;

    if (length) {
      if (document_size_ > 0) {
        chunk_stream_.WriteData(current_pos_, start, length);
      } else {
        // If we did not get content-length in the response, we can't
        // preallocate buffer for the entire document. Resizing array causing
        // memory fragmentation issues on the large files and OOM exceptions.
        // To fix this, we collect all chunks of the file to the list and
        // concatenate them together after request is complete.
        chunk_buffer_.push_back(std::vector<unsigned char>());
        chunk_buffer_.back().resize(length);
        memcpy(&(chunk_buffer_.back()[0]), start, length);
      }
      current_pos_ += length;
      current_chunk_read_ += length;
      client_->OnNewDataAvailable();
    }
    ReadMore();
  } else if (result == PP_OK) {
    ReadComplete();
  } else {
    NOTREACHED();
  }
}

void DocumentLoader::ReadComplete() {
  if (!partial_document_) {
    if (document_size_ == 0) {
      // For the document with no 'content-length" specified we've collected all
      // the chunks already. Let's allocate final document buffer and copy them
      // over.
      chunk_stream_.Preallocate(current_pos_);
      uint32 pos = 0;
      std::list<std::vector<unsigned char> >::iterator it;
      for (it = chunk_buffer_.begin(); it != chunk_buffer_.end(); ++it) {
        chunk_stream_.WriteData(pos, &((*it)[0]), it->size());
        pos += it->size();
      }
      chunk_buffer_.clear();
    }
    document_size_ = current_pos_;
    client_->OnDocumentComplete();
    return;
  }

  request_pending_ = false;
  pending_requests_.pop_front();

  // If there are more pending request - continue downloading.
  if (!pending_requests_.empty()) {
    DownloadPendingRequests();
    return;
  }

  if (IsDocumentComplete()) {
    client_->OnDocumentComplete();
    return;
  }

  if (header_request_)
    client_->OnPartialDocumentLoaded();
  else
    client_->OnPendingRequestComplete();
  header_request_ = false;

  // The OnPendingRequestComplete could have added more requests.
  if (!pending_requests_.empty()) {
    DownloadPendingRequests();
  } else {
    // Document is not complete and we have no outstanding requests.
    // Let's keep downloading PDF file in small chunks.
    uint32 pos = chunk_stream_.GetFirstMissingByte();
    std::vector<std::pair<size_t, size_t> > ranges;
    chunk_stream_.GetMissedRanges(pos, GetRequestSize(), &ranges);
    DCHECK(ranges.size() > 0);
    RequestData(ranges[0].first, ranges[0].second);
  }
}

uint32 DocumentLoader::GetRequestSize() const {
  // Document loading strategy:
  // For first 10 requests, we use 32k chunk sizes, for the next 10 requests we
  // double the size (64k), and so on, until we cap max request size at 2M for
  // 71 or more requests.
  uint32 limited_count = std::min(std::max(requests_count_, 10u), 70u);
  return 32*1024 * (1 << ((limited_count - 1) / 10u));
}

}  // namespace chrome_pdf
