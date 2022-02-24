// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/range_set.h"

#include <algorithm>

#include "core/fxcrt/fx_system.h"
#include "third_party/base/check.h"

RangeSet::RangeSet() = default;

RangeSet::~RangeSet() = default;

bool RangeSet::Contains(const Range& range) const {
  if (IsEmptyRange(range))
    return false;

  const Range fixed_range = FixDirection(range);
  auto it = ranges().upper_bound(fixed_range);

  if (it == ranges().begin())
    return false;  // No ranges includes range.first.

  --it;  // Now it starts equal or before range.first.
  return it->second >= fixed_range.second;
}

void RangeSet::Union(const Range& range) {
  if (IsEmptyRange(range))
    return;

  Range fixed_range = FixDirection(range);
  if (IsEmpty()) {
    ranges_.insert(fixed_range);
    return;
  }

  auto start = ranges_.upper_bound(fixed_range);
  if (start != ranges_.begin())
    --start;  // start now points to the key equal or lower than offset.

  if (start->second < fixed_range.first)
    ++start;  // start element is entirely before current range, skip it.

  auto end = ranges_.upper_bound(Range(fixed_range.second, fixed_range.second));

  if (start == end) {  // No ranges to merge.
    ranges_.insert(fixed_range);
    return;
  }

  --end;

  const size_t new_start = std::min(start->first, fixed_range.first);
  const size_t new_end = std::max(end->second, fixed_range.second);
  ranges_.erase(start, ++end);
  ranges_.insert(Range(new_start, new_end));
}

void RangeSet::Union(const RangeSet& range_set) {
  DCHECK(&range_set != this);
  for (const auto& it : range_set.ranges())
    Union(it);
}

RangeSet::Range RangeSet::FixDirection(const Range& range) const {
  return range.first <= range.second ? range
                                     : Range(range.second + 1, range.first + 1);
}

bool RangeSet::IsEmptyRange(const Range& range) const {
  return range.first == range.second;
}
