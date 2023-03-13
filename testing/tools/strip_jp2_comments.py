#!/usr/bin/env python3
# Copyright 2023 The PDFium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Strips comments from a JP2 file.

This is a simple filter script to strip comments from a JP2 file, in order to
save a few bytes from the final file size.
"""

import struct
import sys

BOX_HEADER_SIZE = 8
BOX_TAG_JP2C = b'jp2c'

MARKER_SIZE = 2
MARKER_START = 0xff
MARKER_TAG_IGNORE = 0x00
MARKER_TAG_COMMENT = 0x64
MARKER_TAG_FILL = 0xff


def parse_box(buffer, offset):
  """Parses the next box in a JP2 file.

  Args:
    buffer: A buffer containing the JP2 file contents.
    offset: The starting offset into the buffer.

  Returns:
    A tuple (next_offset, tag) where next_offset is the ending offset, and tag
    is the type tag. The box contents will be buffer[offset + 8:next_offset].
  """
  length, tag = struct.unpack_from('>I4s', buffer, offset)
  return offset + length, tag


def parse_marker(buffer, offset):
  """Parses the next marker in a codestream.

  Args:
    buffer: A buffer containing the codestream.
    offset: The starting offset into the buffer.

  Returns:
    A tuple (next_offset, tag) where next_offset is the offset after the marker,
    and tag is the type tag. If no marker was found, next_offset will point to
    the end of the buffer, and tag will be None. A marker is always 2 bytes.
  """
  while True:
    # Search for start of marker.
    next_offset = buffer.find(MARKER_START, offset)
    if next_offset == -1:
      next_offset = len(buffer)
      break
    next_offset += 1

    # Parse marker.
    if next_offset == len(buffer):
      break
    tag = buffer[next_offset]
    if tag == MARKER_TAG_FILL:
      # Possible fill byte, reparse as start of marker.
      continue
    next_offset += 1

    if tag == MARKER_TAG_IGNORE:
      # Not a real marker.
      continue
    return next_offset, tag

  return next_offset


def rewrite_jp2c(buffer):
  rewrite_buffer = bytearray(BOX_HEADER_SIZE)

  offset = 0
  start_offset = offset
  while offset < len(buffer):
    next_offset, marker = parse_marker(buffer, offset)
    if marker == MARKER_TAG_COMMENT:
      # Flush the codestream before the comment.
      rewrite_buffer.extend(buffer[start_offset:next_offset - MARKER_SIZE])

      # Find the next marker, skipping the comment.
      next_offset, marker = parse_marker(buffer, next_offset)
      if marker is not None:
        # Reparse the marker.
        next_offset -= MARKER_SIZE
      start_offset = next_offset
    else:
      # Pass through other markers.
      pass
    offset = next_offset

  # Flush the tail of the codestream.
  rewrite_buffer.extend(buffer[start_offset:])

  struct.pack_into('>I4s', rewrite_buffer, 0, len(rewrite_buffer), BOX_TAG_JP2C)
  return rewrite_buffer


def main(in_file, out_file):
  buffer = in_file.read()

  # Scan through JP2 boxes.
  offset = 0
  while offset < len(buffer):
    next_offset, tag = parse_box(buffer, offset)
    if tag == BOX_TAG_JP2C:
      # Rewrite "jp2c" (codestream) box.
      out_file.write(rewrite_jp2c(buffer[offset + BOX_HEADER_SIZE:next_offset]))
    else:
      # Pass through other boxes.
      out_file.write(buffer[offset:next_offset])
    offset = next_offset

  out_file.flush()


if __name__ == '__main__':
  main(sys.stdin.buffer, sys.stdout.buffer)
