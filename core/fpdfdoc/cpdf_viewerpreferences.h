// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPDF_VIEWERPREFERENCES_H_
#define CORE_FPDFDOC_CPDF_VIEWERPREFERENCES_H_

#include <stdint.h>

#include <optional>

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"

class CPDF_Array;
class CPDF_Dictionary;
class CPDF_Document;

class CPDF_ViewerPreferences {
 public:
  explicit CPDF_ViewerPreferences(const CPDF_Document* doc);
  ~CPDF_ViewerPreferences();

  bool IsDirectionR2L() const;
  bool PrintScaling() const;
  int32_t NumCopies() const;
  RetainPtr<const CPDF_Array> PrintPageRange() const;
  ByteString Duplex() const;

  // Gets the entry for `key`.
  std::optional<ByteString> GenericName(ByteStringView key) const;

 private:
  RetainPtr<const CPDF_Dictionary> GetViewerPreferences() const;

  UnownedPtr<const CPDF_Document> const doc_;
};

#endif  // CORE_FPDFDOC_CPDF_VIEWERPREFERENCES_H_
