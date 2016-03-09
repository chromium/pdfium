# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Presubmit script for pdfium.

See http://dev.chromium.org/developers/how-tos/depottools/presubmit-scripts
for more details about the presubmit API built into depot_tools.
"""

LINT_FILTERS = [
  # Rvalue ref checks are unreliable.
  '-build/c++11',
  # Need to fix header names not matching cpp names.
  '-build/include',
  # Need to fix header names not matching cpp names.
  '-build/include_order',
  # Too many to fix at the moment.
  '-readability/casting',
  # Need to refactor large methods to fix.
  '-readability/fn_size',
  # Need to fix errors when making methods explicit.
  '-runtime/explicit',
  # Lots of usage to fix first.
  '-runtime/int',
  # Need to fix two snprintf TODOs
  '-runtime/printf',
  # Lots of non-const references need to be fixed
  '-runtime/references',
  # We are not thread safe, so this will never pass.
  '-runtime/threadsafe_fn',
  # Figure out how to deal with #defines that git cl format creates.
  '-whitespace/indent',
]

def CheckChangeOnUpload(input_api, output_api):
  results = []
  results += input_api.canned_checks.CheckPatchFormatted(input_api, output_api)
  results += input_api.canned_checks.CheckChangeLintsClean(
      input_api, output_api, None, LINT_FILTERS)
  return results
