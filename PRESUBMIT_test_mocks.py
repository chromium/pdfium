# Copyright 2020 The PDFium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re


class MockInputApi(object):
  """Mock class for the InputApi class.

  This class can be used for unittests for presubmit by initializing the files
  attribute as the list of changed files.
  """

  def __init__(self):
    self.files = []
    self.re = re

  def AffectedFiles(self, file_filter=None, include_deletes=False):
    # pylint: disable=unused-argument
    return self.files


class MockOutputApi(object):
  """Mock class for the OutputApi class.

  An instance of this class can be passed to presubmit unittests for outputting
  various types of results.
  """

  class PresubmitResult(object):

    def __init__(self, message, items=None, long_text=''):
      self.message = message
      self.items = items
      self.long_text = long_text

    def __repr__(self):
      return self.message

  class PresubmitError(PresubmitResult):

    def __init__(self, message, items=None, long_text=''):
      MockOutputApi.PresubmitResult.__init__(self, message, items, long_text)
      self.type = 'error'

  class PresubmitPromptWarning(PresubmitResult):

    def __init__(self, message, items=None, long_text=''):
      MockOutputApi.PresubmitResult.__init__(self, message, items, long_text)
      self.type = 'warning'


class MockFile(object):
  """Mock class for the File class.

  This class can be used to form the mock list of changed files in
  MockInputApi for presubmit unittests.
  """

  def __init__(self,
               local_path,
               new_contents=None,
               old_contents=None,
               action='A'):
    self._local_path = local_path
    if new_contents is None:
      new_contents = []
    self._new_contents = new_contents
    self._changed_contents = [(i + 1, l) for i, l in enumerate(new_contents)]
    self._action = action
    self._old_contents = old_contents

  def ChangedContents(self):
    return self._changed_contents

  def LocalPath(self):
    return self._local_path
