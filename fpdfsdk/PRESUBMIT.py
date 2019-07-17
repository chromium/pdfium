# Copyright 2019 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Presubmit script for pdfium.

See http://dev.chromium.org/developers/how-tos/depottools/presubmit-scripts
for more details about the presubmit API built into depot_tools.
"""

def _IsApiTestFile(affected_file):
  return affected_file.LocalPath() == 'fpdfsdk/fpdf_view_c_api_test.c'


def _CheckApiTestFile(input_api, output_api):
  """Checks that the public headers match the API tests."""
  if all([not _IsApiTestFile(f) for f in input_api.AffectedSourceFiles([])]):
    return []

  src_path = input_api.os_path.dirname(input_api.PresubmitLocalPath())
  check_script = input_api.os_path.join(
      src_path, 'testing' , 'tools' , 'api_check.py')
  cmd = [input_api.python_executable, check_script]
  try:
    input_api.subprocess.check_output(cmd)
    return []
  except input_api.subprocess.CalledProcessError as error:
    return [output_api.PresubmitError('api_check.py failed:',
                                      long_text=error.output)]


def CheckChangeOnUpload(input_api, output_api):
  results = []
  results += _CheckApiTestFile(input_api, output_api)
  return results


def CheckChangeOnCommit(input_api, output_api):
  results = []
  results += _CheckApiTestFile(input_api, output_api)
  return results
