// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDF_RESOURCE_RESOURCE_CONSTS_H_
#define PDF_RESOURCE_RESOURCE_CONSTS_H_

#include "base/basictypes.h"

namespace chrome_pdf {

const int kDragTimerMs = 50;
const double kMinZoom = 0.1;
const double kMaxZoom = 10.0;
const double kZoomStep = 1.2;

const uint32 kFadingTimeoutMs = 50;

const uint32 kToolbarId = 10;
const uint32 kThumbnailsId = 11;
const uint32 kProgressBarId = 12;
const uint32 kPageIndicatorId = 13;
const uint32 kFitToPageButtonId = 100;
const uint32 kFitToWidthButtonId = 101;
const uint32 kZoomOutButtonId = 102;
const uint32 kZoomInButtonId = 103;
const uint32 kSaveButtonId = 104;
const uint32 kPrintButtonId = 105;

const uint32 kAutoScrollId = 200;

// fading_rect.left = button_rect.left - kToolbarFadingOffsetLeft
const int32 kToolbarFadingOffsetLeft = 40;
// fading_rect.top = button_rect.top - kToolbarFadingOffsetTop
const int32 kToolbarFadingOffsetTop = 40;
// fading_rect.right = button_rect.right + kToolbarFadingOffsetRight
const int32 kToolbarFadingOffsetRight = 10;
// fading_rect.bottom = button_rect.bottom + kToolbarFadingOffsetBottom
const int32 kToolbarFadingOffsetBottom = 8;

const int32 kProgressOffsetLeft = 8;
const int32 kProgressOffsetBottom = 8;

// Width of the thumbnails control.
const int32 kThumbnailsWidth = 196;

}  // namespace chrome_pdf

#endif  // PDF_RESOURCE_RESOURCE_CONSTS_H_
