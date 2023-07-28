#!/usr/bin/env python3
# Copyright 2023 The PDFium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Tool for converting GN runtime_deps to CAS archive paths."""

import argparse
from collections import deque
import filecmp
import json
import logging
from pathlib import Path
import os

EXCLUDE_DIRS = {
    '.git',
    '__pycache__',
}


def parse_runtime_deps(runtime_deps):
  """Parses GN's `runtime_deps` format."""
  with runtime_deps:
    return [line.rstrip() for line in runtime_deps]


def resolve_paths(root, initial_paths):
  """Converts paths to CAS archive paths format."""
  absolute_root = os.path.abspath(root)

  resolved_paths = []
  unvisited_paths = deque(map(Path, initial_paths))
  while unvisited_paths:
    path = unvisited_paths.popleft()

    if not path.exists():
      logging.warning('"%(path)s" does not exist', {'path': path})
      continue

    if path.is_dir():
      # Expand specific children if any are excluded.
      child_paths = expand_dir(path)
      if child_paths:
        unvisited_paths.extendleft(child_paths)
        continue

    resolved_paths.append(os.path.relpath(path, start=absolute_root))

  resolved_paths.sort()
  return [[absolute_root, path] for path in resolved_paths]


def expand_dir(path):
  """Explicitly expands directory if any children are excluded."""
  expand = False
  expanded_paths = []

  for child_path in path.iterdir():
    if child_path.name in EXCLUDE_DIRS and path.is_dir():
      expand = True
      continue
    expanded_paths.append(child_path)

  return expanded_paths if expand else []


def replace_output(resolved, output_path):
  """Atomically replaces the output with the resolved JSON if changed."""
  new_output_path = output_path + '.new'
  try:
    with open(new_output_path, 'w', encoding='ascii') as new_output:
      json.dump(resolved, new_output)

    if (os.path.exists(output_path) and
        filecmp.cmp(new_output_path, output_path, shallow=False)):
      return

    os.replace(new_output_path, output_path)
    new_output_path = None
  finally:
    if new_output_path:
      os.remove(new_output_path)


def main():
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument('--root')
  parser.add_argument(
      'runtime_deps',
      help='runtime_deps written by GN',
      type=argparse.FileType('r', encoding='utf_8'),
      metavar='input.runtime_deps')
  parser.add_argument(
      'output_json',
      help='CAS archive paths in JSON format',
      metavar='output.json')
  args = parser.parse_args()

  runtime_deps = parse_runtime_deps(args.runtime_deps)
  resolved_paths = resolve_paths(args.root, runtime_deps)
  replace_output(resolved_paths, args.output_json)


if __name__ == '__main__':
  main()
