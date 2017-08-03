# Copyright 2017 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Classes for dealing with git."""

import subprocess


class GitHelper(object):
  """Issues git commands. Stateful."""

  def __init__(self):
    self.stashed = 0

  def Checkout(self, branch):
    """Checks out a branch."""
    subprocess.check_output(['git', 'checkout', branch])

  def StashPush(self):
    """Stashes uncommitted changes."""
    output = subprocess.check_output(['git', 'stash', '--include-untracked'])
    if 'No local changes to save' in output:
      return False

    self.stashed += 1
    return True

  def StashPopAll(self):
    """Pops as many changes as this instance stashed."""
    while self.stashed > 0:
      subprocess.check_output(['git', 'stash', 'pop'])
      self.stashed -= 1

  def GetCurrentBranchName(self):
    """Returns a string with the current branch name."""
    return subprocess.check_output(
        ['git', 'rev-parse', '--abbrev-ref', 'HEAD']).strip()

  def BranchExists(self, branch_name):
    """Return whether a branch with the given name exists."""
    try:
      subprocess.check_output(['git', 'rev-parse', '--verify',
                               branch_name])
      return True
    except subprocess.CalledProcessError:
      return False

  def CloneLocal(self, source_repo, new_repo):
    subprocess.check_call(['git', 'clone', source_repo, new_repo])
