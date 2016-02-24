# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Presubmit script for pdfium.

See http://dev.chromium.org/developers/how-tos/depottools/presubmit-scripts
for more details about the presubmit API built into depot_tools.
"""

LINT_FILTERS = [
  '-build/c++11',
  '-build/include',
  '-build/include_order',
  '-build/include_what_you_use',
  '-build/namespaces',
  '-build/storage_class',
  '-readability/braces',
  '-readability/casting',
  '-readability/fn_size',
  '-readability/function',
  '-readability/inheritance',
  '-readability/multiline_comment',
  '-readability/multiline_string',
  '-readability/namespace',
  '-readability/todo',
  '-readability/utf8',
  '-runtime/arrays',
  '-runtime/casting',
  '-runtime/explicit',
  '-runtime/int',
  '-runtime/printf',
  '-runtime/references',
  '-runtime/threadsafe_fn',
  '-whitespace/indent',
  '-whitespace/line_length',
]

def CheckChangeOnUpload(input_api, output_api):
  results = []
  results += input_api.canned_checks.CheckPatchFormatted(input_api, output_api)
  results += input_api.canned_checks.CheckChangeLintsClean(
      input_api, output_api, None, LINT_FILTERS)
  return results
