#!/usr/bin/env python3
# Copyright 2025 The PDFium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
Convert PDF files to .in format on a best-effort basis.

Requires `mutool` on PATH.
"""

import argparse
import re
import subprocess
import sys
import tempfile


class PDFParser:

  def __init__(self, pdf_data):
    self.data = pdf_data
    self.objects = {}

  def parse(self):
    # Find all objects in the PDF
    obj_pattern = rb'(\d+)\s+(\d+)\s+obj\s*(.*?)\s*endobj'
    matches = re.finditer(obj_pattern, self.data, re.DOTALL)

    for match in matches:
      obj_num = int(match.group(1))
      gen_num = int(match.group(2))
      obj_content = match.group(3)

      self.objects[(obj_num, gen_num)] = obj_content

    return self.objects

  def format_object_content(self, content):
    """Format object content, handling dictionaries and streams."""
    content_str = content.decode('latin-1', errors='replace')

    # Check if this is a stream object
    if 'stream' in content_str and 'endstream' in content_str:
      # Split into dictionary and stream parts
      stream_start = content_str.find('stream')
      dict_part = content_str[:stream_start].strip()
      stream_part = content_str[stream_start:]

      # Replace /Length with {{streamlen}}
      dict_part = re.sub(r'/Length\s+\d+', '{{streamlen}}', dict_part)

      # Extract actual stream data
      stream_match = re.search(r'stream\s*(.*?)\s*endstream', stream_part,
                               re.DOTALL)
      if stream_match:
        stream_data = stream_match.group(1).strip()
        return f"{dict_part}\nstream\n{stream_data}\nendstream"

    # Regular dictionary object
    content_str = re.sub(r'/Length\s+\d+', '{{streamlen}}', content_str)
    return content_str.strip()


def pdf_to_in(pdf_path, output_path=None):
  with tempfile.NamedTemporaryFile(
      mode='wb', suffix='.pdf', delete=False) as tmp_file:
    tmp_path = tmp_file.name

    # Run mutool clean -a
    result = subprocess.run(['mutool', 'clean', '-a', pdf_path, tmp_path],
                            capture_output=True,
                            check=False,
                            text=True)

    if result.returncode != 0:
      raise RuntimeError(f"mutool failed: {result.stderr}")

    # Read the cleaned PDF
    with open(tmp_path, 'rb') as f:
      pdf_data = f.read()

  # Parse PDF
  parser = PDFParser(pdf_data)
  objects = parser.parse()

  # Build output
  output_lines = ['{{header}}', '']

  # Sort objects by number
  sorted_objects = sorted(objects.keys(), key=lambda x: (x[0], x[1]))

  # Add objects
  for obj_num, gen_num in sorted_objects:
    obj_content = objects[(obj_num, gen_num)]
    formatted_content = parser.format_object_content(obj_content)

    output_lines.append(f'{{{{object {obj_num} {gen_num}}}}} <<')
    output_lines.append(formatted_content.removeprefix('<<\n'))
    output_lines.append('endobj')
    output_lines.append('')

  output_lines.append('{{xref}}')
  output_lines.append('{{trailer}}')
  output_lines.append('{{startxref}}')
  output_lines.append('%%EOF')
  output_text = '\n'.join(output_lines)

  if output_path:
    with open(output_path, 'w', encoding='utf-8') as f:
      f.write(output_text + '\n')
    print(f"Converted PDF written to: {output_path}")
  else:
    print(output_text)

  return output_text


def main():
  parser = argparse.ArgumentParser(
      description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
  parser.add_argument('input')
  parser.add_argument('-o', '--output')
  args = parser.parse_args()
  pdf_to_in(args.input, args.output)


if __name__ == '__main__':
  sys.exit(main())
