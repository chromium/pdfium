#!/usr/bin/env python
# Copyright 2017 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Verifies exported functions in public/*.h are in fpdf_view_c_api_test.c.

This script gathers a list of functions from public/*.h that contain
FPDF_EXPORT. It then gathers a list of functions from
fpdfsdk/fpdf_view_c_api_test.c. It then verifies both lists do not contain
duplicates, and they match each other. It also checks that the order in
fpdf_view_c_api_test.c is alphabetical within each section.

"""

import os
import re
import sys


def _IsValidFunctionName(function, filename):
  if function.startswith('FPDF'):
    return True
  if function.startswith('FSDK_') and filename == 'fpdf_ext.h':
    return True
  if function.startswith('FORM_') and filename == 'fpdf_formfill.h':
    return True
  return False


def _FindFunction(function_snippet, filename):
  function_split = function_snippet.split('(')
  assert len(function_split) == 2
  function = function_split[0]
  assert _IsValidFunctionName(function, filename)
  return function


def _GetExportsFromHeader(dirname, filename):
  with open(os.path.join(dirname, filename)) as f:
    contents = f.readlines()
    look_for_function_name = False
    functions = []
    for line in contents:
      if look_for_function_name:
        look_for_function_name = False
        split_line = line.rstrip().split(' ')
        functions.append(_FindFunction(split_line[0], filename))
        continue

      if not line.startswith('FPDF_EXPORT '):
        continue

      # Format should be: FPDF_EXPORT return_type FPDF_CALLCONV
      split_line = line.rstrip().split(' ')
      callconv_index = split_line.index('FPDF_CALLCONV')
      assert callconv_index >= 2
      if callconv_index + 1 == len(split_line):
        look_for_function_name = True
        continue

      functions.append(_FindFunction(split_line[callconv_index + 1], filename))
    return functions


def _GetFunctionsFromPublicHeaders(src_path):
  public_path = os.path.join(src_path, 'public')
  functions = []
  for filename in os.listdir(public_path):
    if filename.endswith('.h'):
      functions.extend(_GetExportsFromHeader(public_path, filename))
  return functions


def _CheckSorted(functions):
  unsorted_functions = set()
  for i in range(len(functions) - 1):
    if functions[i] > functions[i + 1]:
      unsorted_functions.add(functions[i])
      unsorted_functions.add(functions[i + 1])
  return unsorted_functions


def _GetFunctionsFromTest(api_test_path):
  chk_regex = re.compile('^    CHK\((.*)\);\n$')
  file_regex = re.compile('^    //.*\.h\n$')
  with open(api_test_path) as f:
    contents = f.readlines()
    functions = []
    functions_in_file = []
    for line in contents:
      if (file_regex.match(line)):
        functions.append(functions_in_file)
        functions_in_file = []
      match = chk_regex.match(line)
      if match:
        functions_in_file.append(match.groups()[0])
    functions.append(functions_in_file)
    return functions


def _FindDuplicates(functions):
  return set([f for f in functions if functions.count(f) > 1])


def _CheckAndPrintFailures(failure_list, failure_message):
  if not failure_list:
    return True

  print '%s:' % failure_message
  for f in sorted(failure_list):
    print f
  return False


def main():
  script_abspath = os.path.abspath(__file__)
  src_path = os.path.dirname(os.path.dirname(os.path.dirname(script_abspath)))
  public_functions = _GetFunctionsFromPublicHeaders(src_path)

  api_test_relative_path = os.path.join('fpdfsdk', 'fpdf_view_c_api_test.c')
  api_test_path = os.path.join(src_path, api_test_relative_path)
  test_functions_per_section = _GetFunctionsFromTest(api_test_path)
  result = True
  unsorted_functions = set()
  for functions in test_functions_per_section:
    unsorted_functions |= _CheckSorted(functions)
  check = _CheckAndPrintFailures(
      unsorted_functions,
      'Found CHKs that are not in alphabetical order within each section in %s'
      % api_test_path)
  result = result and check

  duplicate_public_functions = _FindDuplicates(public_functions)
  check = _CheckAndPrintFailures(duplicate_public_functions,
                                 'Found duplicate functions in public headers')
  result = result and check

  test_functions = [
      function for functions in test_functions_per_section
      for function in functions
  ]
  duplicate_test_functions = _FindDuplicates(test_functions)
  check = _CheckAndPrintFailures(duplicate_test_functions,
                                 'Found duplicate functions in API test')
  result = result and check

  public_functions_set = set(public_functions)
  test_functions_set = set(test_functions)
  not_tested = public_functions_set.difference(test_functions_set)
  check = _CheckAndPrintFailures(not_tested, 'Functions not tested')
  result = result and check
  non_existent = test_functions_set.difference(public_functions_set)
  check = _CheckAndPrintFailures(non_existent, 'Tested functions do not exist')
  result = result and check

  if not result:
    print('Some checks failed. Make sure %s is in sync with the public API '
          'headers.' % api_test_relative_path)
    return 1

  return 0


if __name__ == '__main__':
  sys.exit(main())
