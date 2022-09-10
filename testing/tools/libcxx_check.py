#!/usr/bin/env python3
# Copyright 2022 The PDFium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Verifies libcxx_revision entries are in sync.

DEPS and buildtools/deps_revisions.gni both have libcxx_revision entries.
Check that they are in sync.
"""

import re
import sys


def _ExtractRevisionFromFile(path, regex):
  """Gets the revision by reading path and searching the lines using regex."""
  data = open(path, 'rb').read().splitlines()
  revision = None
  for line in data:
    match = regex.match(line)
    if not match:
      continue
    if revision:
      return None
    revision = match.group(1)
  return revision


def _GetDepsLibcxxRevision(deps_path):
  """Gets the libcxx_revision from DEPS."""
  regex = re.compile(b"^  'libcxx_revision': '(.*)',$")
  return _ExtractRevisionFromFile(deps_path, regex)


def _GetBuildtoolsLibcxxRevision(buildtools_deps_path):
  """Gets the libcxx_revision from buildtools/deps_revisions.gni."""
  regex = re.compile(b'^  libcxx_revision = "(.*)"$')
  return _ExtractRevisionFromFile(buildtools_deps_path, regex)


def main():
  if len(sys.argv) != 3:
    print('Wrong number of arguments')
    return 0

  deps_path = sys.argv[1]
  deps_revision = _GetDepsLibcxxRevision(deps_path)
  if not deps_revision:
    print('Cannot parse', deps_path)
    return 0

  buildtools_deps_path = sys.argv[2]
  buildtools_revision = _GetBuildtoolsLibcxxRevision(buildtools_deps_path)
  if not buildtools_revision:
    print('Cannot parse', buildtools_deps_path)
    return 0

  if deps_revision != buildtools_revision:
    print('libcxx_revision mismatch between %s and %s: %s vs. %s' %
          (deps_path, buildtools_deps_path, deps_revision, buildtools_revision))
  return 0


if __name__ == '__main__':
  sys.exit(main())
