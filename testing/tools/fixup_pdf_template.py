#!/usr/bin/env python
# Copyright 2014 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Expands a hand-written PDF testcase (template) into a valid PDF file.

There are several places in a PDF file where byte-offsets are required. This
script replaces {{name}}-style variables in the input with calculated results

  {{include path/to/file}} - inserts file's contents into stream.
  {{header}} - expands to the header comment required for PDF files.
  {{xref}} - expands to a generated xref table, noting the offset.
  {{trailer}} - expands to a standard trailer with "1 0 R" as the /Root.
  {{trailersize}} - expands to |/Size n|, to be used in non-standard trailers.
  {{startxref} - expands to a startxref directive followed by correct offset.
  {{object x y}} - expands to |x y obj| declaration, noting the offset.
  {{streamlen}} - expands to |/Length n|.
"""

import cStringIO
import optparse
import os
import re
import sys


class StreamLenState:
  START = 1
  FIND_STREAM = 2
  FIND_ENDSTREAM = 3


class TemplateProcessor:
  HEADER_TOKEN = '{{header}}'
  HEADER_REPLACEMENT = '%PDF-1.7\n%\xa0\xf2\xa4\xf4'

  XREF_TOKEN = '{{xref}}'
  XREF_REPLACEMENT = 'xref\n%d %d\n'

  XREF_REPLACEMENT_N = '%010d %05d n \n'
  XREF_REPLACEMENT_F = '0000000000 65535 f \n'
  # XREF rows must be exactly 20 bytes - space required.
  assert len(XREF_REPLACEMENT_F) == 20

  TRAILER_TOKEN = '{{trailer}}'
  TRAILER_REPLACEMENT = 'trailer <<\n  /Root 1 0 R\n  /Size %d\n>>'

  TRAILERSIZE_TOKEN = '{{trailersize}}'
  TRAILERSIZE_REPLACEMENT = '/Size %d'

  STARTXREF_TOKEN = '{{startxref}}'
  STARTXREF_REPLACEMENT = 'startxref\n%d'

  OBJECT_PATTERN = r'\{\{object\s+(\d+)\s+(\d+)\}\}'
  OBJECT_REPLACEMENT = r'\1 \2 obj'

  STREAMLEN_TOKEN = '{{streamlen}}'
  STREAMLEN_REPLACEMENT = '/Length %d'

  def __init__(self):
    self.streamlen_state = StreamLenState.START
    self.streamlens = []
    self.offset = 0
    self.xref_offset = 0
    self.max_object_number = 0
    self.objects = {}

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

  def preprocess_line(self, line):
    if self.STREAMLEN_TOKEN in line:
      assert self.streamlen_state == StreamLenState.START
      self.streamlen_state = StreamLenState.FIND_STREAM
      self.streamlens.append(0)
      return

    if (self.streamlen_state == StreamLenState.FIND_STREAM and
        line.rstrip() == 'stream'):
      self.streamlen_state = StreamLenState.FIND_ENDSTREAM
      return

    if self.streamlen_state == StreamLenState.FIND_ENDSTREAM:
      if line.rstrip() == 'endstream':
        self.streamlen_state = StreamLenState.START
      else:
        self.streamlens[-1] += len(line)

  def process_line(self, line):
    if self.HEADER_TOKEN in line:
      line = line.replace(self.HEADER_TOKEN, self.HEADER_REPLACEMENT)
    if self.STREAMLEN_TOKEN in line:
      sub = self.STREAMLEN_REPLACEMENT % self.streamlens.pop(0)
      line = re.sub(self.STREAMLEN_TOKEN, sub, line)
    if self.XREF_TOKEN in line:
      self.xref_offset = self.offset
      line = self.generate_xref_table()
    if self.TRAILER_TOKEN in line:
      replacement = self.TRAILER_REPLACEMENT % (self.max_object_number + 1)
      line = line.replace(self.TRAILER_TOKEN, replacement)
    if self.TRAILERSIZE_TOKEN in line:
      replacement = self.TRAILERSIZE_REPLACEMENT % (self.max_object_number + 1)
      line = line.replace(self.TRAILERSIZE_TOKEN, replacement)
    if self.STARTXREF_TOKEN in line:
      replacement = self.STARTXREF_REPLACEMENT % self.xref_offset
      line = line.replace(self.STARTXREF_TOKEN, replacement)
    match = re.match(self.OBJECT_PATTERN, line)
    if match:
      self.insert_xref_entry(int(match.group(1)), int(match.group(2)))
      line = re.sub(self.OBJECT_PATTERN, self.OBJECT_REPLACEMENT, line)
    self.offset += len(line)
    return line


def expand_file(infile, output_path):
  processor = TemplateProcessor()
  try:
    with open(output_path, 'wb') as outfile:
      preprocessed = cStringIO.StringIO()
      for line in infile:
        preprocessed.write(line)
        processor.preprocess_line(line)
      preprocessed.seek(0)
      for line in preprocessed:
        outfile.write(processor.process_line(line))
  except IOError:
    print >> sys.stderr, 'failed to process %s' % input_path


def insert_includes(input_path, output_file, visited_set):
  input_path = os.path.normpath(input_path)
  if input_path in visited_set:
    print >> sys.stderr, 'Circular inclusion %s, ignoring' % input_path
    return
  visited_set.add(input_path)
  try:
    with open(input_path, 'rb') as infile:
      for line in infile:
        match = re.match(r'\s*\{\{include\s+(.+)\}\}', line)
        if match:
          insert_includes(
              os.path.join(os.path.dirname(input_path), match.group(1)),
              output_file, visited_set)
        else:
          output_file.write(line)
  except IOError:
    print >> sys.stderr, 'failed to include %s' % input_path
    raise
  visited_set.discard(input_path)


def main():
  parser = optparse.OptionParser()
  parser.add_option('--output-dir', default='')
  options, args = parser.parse_args()
  for testcase_path in args:
    testcase_filename = os.path.basename(testcase_path)
    testcase_root, _ = os.path.splitext(testcase_filename)
    output_dir = os.path.dirname(testcase_path)
    if options.output_dir:
      output_dir = options.output_dir
    intermediate_stream = cStringIO.StringIO()
    insert_includes(testcase_path, intermediate_stream, set())
    intermediate_stream.seek(0)
    output_path = os.path.join(output_dir, testcase_root + '.pdf')
    expand_file(intermediate_stream, output_path)
  return 0


if __name__ == '__main__':
  sys.exit(main())
