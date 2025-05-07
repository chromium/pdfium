#!/usr/bin/env python3
# Copyright 2025 The PDFium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Renumbers objects in a .in file consumed by fixup_pdf_template.py

Takes a .in file and renumbers all objects therein so that they are numbered
in the order they appear in in the file.
"""

import argparse
import re
import sys


def renumber(contents):
  OBJECT_PATTERN = r'{{object\s+(\d+)\s+(\d+)}}'

  old_to_new = {
      m.group(1): str(i)
      for i, m in enumerate(re.finditer(OBJECT_PATTERN, contents), start=1)
  }

  def new_id(m):
    return m.group(0).replace(m.group(1), old_to_new[m.group(1)])

  contents = re.sub(OBJECT_PATTERN, new_id, contents)
  contents = re.sub(r'\b(\d+)\s+\d+\s+R\b', new_id, contents)  # Update `n 0 R`.
  return contents


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('file')
  args = parser.parse_args()

  with open(args.file, 'r') as f:
    contents = f.read()
  contents = renumber(contents)
  with open(args.file, 'w') as f:
    f.write(contents)


if __name__ == '__main__':
  sys.exit(main())
