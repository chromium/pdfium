#!/usr/bin/env python
# Copyright 2014 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Expands a hand-written PDF testcase (template) into a valid PDF file.

There are several places in a PDF file where byte-offsets are required. This
script replaces {{name}}-style variables in the input with calculated results

  {{header}}     - expands to the header comment required for PDF files.
  {{xref}}       - expands to a generated xref table, noting the offset.
  {{startxref}   - expands to a startxref directive followed by correct offset.
  {{object x y}} - expands to |x y obj| declaration, noting the offset."""

import optparse
import os
import re
import sys

class TemplateProcessor:
  HEADER_TOKEN =  '{{header}}'
  HEADER_REPLACEMENT = '%PDF-1.7\n%\xa0\xf2\xa4\xf4'

  XREF_TOKEN = '{{xref}}'
  XREF_REPLACEMENT = 'xref\n%d %d\n'
  XREF_REPLACEMENT_N = '%010d %05d n\n'
  XREF_REPLACEMENT_F = '0000000000 65536 f\n'

  STARTXREF_TOKEN= '{{startxref}}'
  STARTXREF_REPLACEMENT = 'startxref\n%d'

  OBJECT_PATTERN = r'\{\{object\s+(\d+)\s+(\d+)\}\}'
  OBJECT_REPLACEMENT = r'\1 \2 obj'

  def __init__(self):
    self.offset = 0
    self.xref_offset = 0
    self.max_object_number = 0
    self.objects = { }

  def insert_xref_entry(self, object_number, generation_number):
    self.objects[object_number] = (self.offset, generation_number)
    self.max_object_number = max(self.max_object_number, object_number)

  def generate_xref_table(self):
    result = self.XREF_REPLACEMENT % (0, self.max_object_number + 1)
    for i in range(0, self.max_object_number + 1):
      if i in self.objects:
        result += self.XREF_REPLACEMENT_N % self.objects[i]
      else:
        result += self.XREF_REPLACEMENT_F
    return result

  def process_line(self, line):
    if self.HEADER_TOKEN in line:
      line = line.replace(self.HEADER_TOKEN, self.HEADER_REPLACEMENT)
    if self.XREF_TOKEN in line:
      self.xref_offset = self.offset
      line = self.generate_xref_table()
    if self.STARTXREF_TOKEN in line:
      replacement = self.STARTXREF_REPLACEMENT % self.xref_offset
      line = line.replace(self.STARTXREF_TOKEN, replacement)
    match = re.match(self.OBJECT_PATTERN, line)
    if match:
      self.insert_xref_entry(int(match.group(1)), int(match.group(2)))
      line = re.sub(self.OBJECT_PATTERN, self.OBJECT_REPLACEMENT, line)
    self.offset += len(line)
    return line

def expand_file(input_filename):
  (input_root, extension) = os.path.splitext(input_filename)
  output_filename = input_root + '.pdf'
  processor = TemplateProcessor()
  try:
    with open(input_filename, 'r') as infile:
      with open(output_filename, 'w') as outfile:
        for line in infile:
           outfile.write(processor.process_line(line))
  except IOError:
    print >> sys.stderr, 'failed to process %s' % input_filename

def main():
  for arg in sys.argv[1:]:
    expand_file(arg)
  return 0

if __name__ == '__main__':
  sys.exit(main())
