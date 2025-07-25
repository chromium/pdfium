// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_PAGEOBJECTHOLDER_H_
#define CORE_FPDFAPI_PAGE_CPDF_PAGEOBJECTHOLDER_H_

#include <stddef.h>
#include <stdint.h>

#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <utility>
#include <vector>

#include "core/fpdfapi/page/cpdf_transparency.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/dib/fx_dib.h"

class CPDF_ContentParser;
class CPDF_Document;
class CPDF_PageObject;
class PauseIndicatorIface;

// These structs are used to keep track of resources that have already been
// generated in the page object holder.
struct GraphicsData {
  float fillAlpha;
  float strokeAlpha;
  BlendMode blendType;

  bool operator<(const GraphicsData& other) const;
};

struct FontData {
  ByteString baseFont;
  ByteString type;

  bool operator<(const FontData& other) const;
};

class CPDF_PageObjectHolder {
 public:
  enum class ParseState : uint8_t { kNotParsed, kParsing, kParsed };

  // Key: The stream index.
  // Value: The current transformation matrix at the end of the stream.
  using CTMMap = std::map<int32_t, CFX_Matrix>;

  // Key: The dictionary key for a given removed resource.
  // Value: The removed object.
  using RemovedResourceMap = std::map<ByteString, RetainPtr<CPDF_Object>>;

  // Key: The resource dictionary name.
  // Value: The entries removed from that dictionary.
  using AllRemovedResourcesMap =
      std::map<ByteString, RemovedResourceMap, std::less<>>;

  using iterator = std::deque<std::unique_ptr<CPDF_PageObject>>::iterator;
  using const_iterator =
      std::deque<std::unique_ptr<CPDF_PageObject>>::const_iterator;

  CPDF_PageObjectHolder(CPDF_Document* doc,
                        RetainPtr<CPDF_Dictionary> dict,
                        RetainPtr<CPDF_Dictionary> pPageResources,
                        RetainPtr<CPDF_Dictionary> pResources);
  virtual ~CPDF_PageObjectHolder();

  virtual bool IsPage() const;

  void StartParse(std::unique_ptr<CPDF_ContentParser> pParser);
  void ContinueParse(PauseIndicatorIface* pPause);
  ParseState GetParseState() const { return parse_state_; }

  CPDF_Document* GetDocument() const { return document_; }
  RetainPtr<const CPDF_Dictionary> GetDict() const { return dict_; }
  RetainPtr<CPDF_Dictionary> GetMutableDict() { return dict_; }
  RetainPtr<const CPDF_Dictionary> GetResources() const { return resources_; }
  RetainPtr<CPDF_Dictionary> GetMutableResources() { return resources_; }
  void SetResources(RetainPtr<CPDF_Dictionary> dict) {
    resources_ = std::move(dict);
  }
  RetainPtr<const CPDF_Dictionary> GetPageResources() const {
    return page_resources_;
  }
  RetainPtr<CPDF_Dictionary> GetMutablePageResources() {
    return page_resources_;
  }
  size_t GetPageObjectCount() const { return page_object_list_.size(); }
  size_t GetActivePageObjectCount() const;
  CPDF_PageObject* GetPageObjectByIndex(size_t index) const;
  void AppendPageObject(std::unique_ptr<CPDF_PageObject> pPageObj);

  bool InsertPageObjectAtIndex(size_t index,
                               std::unique_ptr<CPDF_PageObject> page_obj);

  // Remove `pPageObj` if present, and transfer ownership to the caller.
  std::unique_ptr<CPDF_PageObject> RemovePageObject(CPDF_PageObject* pPageObj);
  bool ErasePageObjectAtIndex(size_t index);

  iterator begin() { return page_object_list_.begin(); }
  const_iterator begin() const { return page_object_list_.begin(); }

  iterator end() { return page_object_list_.end(); }
  const_iterator end() const { return page_object_list_.end(); }

  const CFX_FloatRect& GetBBox() const { return bbox_; }

  const CPDF_Transparency& GetTransparency() const { return transparency_; }
  bool BackgroundAlphaNeeded() const { return background_alpha_needed_; }
  void SetBackgroundAlphaNeeded(bool needed) {
    background_alpha_needed_ = needed;
  }

  bool HasImageMask() const { return !mask_bounding_boxes_.empty(); }
  const std::vector<CFX_FloatRect>& GetMaskBoundingBoxes() const {
    return mask_bounding_boxes_;
  }
  void AddImageMaskBoundingBox(const CFX_FloatRect& box);
  bool HasDirtyStreams() const { return !dirty_streams_.empty(); }
  std::set<int32_t> TakeDirtyStreams();

  std::optional<ByteString> GraphicsMapSearch(const GraphicsData& gd);
  void GraphicsMapInsert(const GraphicsData& gd, const ByteString& str);

  std::optional<ByteString> FontsMapSearch(const FontData& fd);
  void FontsMapInsert(const FontData& fd, const ByteString& str);

  // `stream` must be non-negative or `CPDF_PageObject::kNoContentStream`.
  CFX_Matrix GetCTMAtBeginningOfStream(int32_t stream);

  // `stream` must be non-negative.
  CFX_Matrix GetCTMAtEndOfStream(int32_t stream);

  AllRemovedResourcesMap& all_removed_resources_map() {
    return all_removed_resources_map_;
  }

 protected:
  void LoadTransparencyInfo();

  RetainPtr<CPDF_Dictionary> page_resources_;
  RetainPtr<CPDF_Dictionary> resources_;
  std::map<GraphicsData, ByteString> graphics_map_;
  std::map<FontData, ByteString> fonts_map_;
  CFX_FloatRect bbox_;
  CPDF_Transparency transparency_;

 private:
  bool background_alpha_needed_ = false;
  ParseState parse_state_ = ParseState::kNotParsed;
  RetainPtr<CPDF_Dictionary> const dict_;
  UnownedPtr<CPDF_Document> document_;
  std::vector<CFX_FloatRect> mask_bounding_boxes_;
  std::unique_ptr<CPDF_ContentParser> parser_;
  std::deque<std::unique_ptr<CPDF_PageObject>> page_object_list_;

  CTMMap all_ctms_;

  // The indexes of Content streams that are dirty and need to be regenerated.
  std::set<int32_t> dirty_streams_;

  // All the resources from `resources_` that are unused. Hold on to them here
  // in case they need to be restored.
  AllRemovedResourcesMap all_removed_resources_map_;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_PAGEOBJECTHOLDER_H_
