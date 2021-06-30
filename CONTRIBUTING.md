# CONTRIBUTING
In general, we follow the
[Chromium Contributing](https://chromium.googlesource.com/chromium/src/+/main/docs/contributing.md)
guidelines in PDFium. The code review process, and the build tools are all very
similar to Chromium. The PDFium
[README](https://pdfium.googlesource.com/pdfium/+/refs/heads/main/README.md)
outlines specific build and test information for PDFium.

This document focuses on how the PDFium project operates and how we’d like it
to operate in the future. This is a living document, please file bugs if you
think there are changes/updates which could be put in place to make it easier
to contribute to PDFium.

## Communication
When writing a new feature or fixing an existing bug, get a second opinion
before investing effort in coding. Coordinating up front makes it much easier
to avoid frustration later on.

If it‘s a new feature, or updating existing code, first propose it to the
[mailing list](https://groups.google.com/forum/#!forum/pdfium).

 * If a change needs further context outside the CL, it should be tracked in
   the [bug system](https://bugs.chromium.org/p/pdfium). Bugs are the right
   place for long histories, discussion and debate, attaching screenshots, and
   linking to other associated bugs. Bugs are unnecessary for changes isolated
   enough to not need any of these.
 * If the work being implemented is especially complex or large a design
   document may be warranted. The document should be linked to the filled bug
   and be set to publicly viewable.
 * If there isn't a bug and there should be one, please file a new bug.
 * Just because there is a bug in the bug system doesn't necessarily mean that
   a patch will be accepted.

## Public APIs
The public API of PDFium has grown over time. There are multiple mechanisms in
place to support this growth from the stability requirements to the versioning
fields. Along with those there are several other factors to be considered when
adding public APIs.

 * _Consistency_. We try to keep the APIs consistent with each other, this
   includes things like naming, parameter ordering and how parameters are
   handled.
 * _Generality_. PDFium is used in several places outside the browser. This
   could be server side, or in user applications. APIs should be designed to
   work in the general case, or such that they can be expanded to the general
   case if possible.
 * _Documentation_. All public APIs should be documented to include information
   on ownership of passed parameters, valid values being provided, error
   conditions and return values.
 * _Differentiate error conditions_. If at all possible, it should be possible
   to tell the difference between a valid failure and an error.
 * _Avoid global state_. APIs should receive the objects to be operated on
   instead of assuming they exist in a global context.

### Stability
There are a lot of consumers of PDFium outside of Chromium. These include
LibreOffice, Android and offline conversion tooling. As such, a lot of care is
taken around the code in the
[public](https://pdfium.googlesource.com/pdfium/+/refs/heads/main/public/)
folder. When planning on changing the public API, the change should be preceded
by a bug being created and an email to the mailing list to gather feedback from
other PDFium embedders.

The only stability guarantees that PDFium provides are around the APIs in the
public folder. Any other interface in the system can be changed without notice.
If there are features needed which are not exposed through the public headers
you'll need to file a bug to get it added to the public APIs.

#### Experimental
All APIs start as Experimental. The experimental status is a documentation tag
which is added to the API, the first line of the API documentation should be
`// Experimental API.`

Experimental APIs may be changed or removed entirely without formal notice to
the community.

#### Stable
APIs eventually graduate to stable. This is done by removing the
`// Experimental API.` marker in the documentation. We endeavor to not change
stable APIs without notice to the community.

NOTE, the process of migrating from experimental to stable isn’t well defined
at this point. We have experimental APIs which have been that way for multiple
years. We should work to better define how this transition happens.

#### Deprecated
If the API is retired, it is marked as deprecated and will eventually be removed.
API deprecation should, ideally, come with a better replacement API and have a
6-12 months deprecation period.  The pending removal should be recorded in the
documentation comment for the API and should also be recorded in the README with
the target removal timeframe. All deprecations should have an associated bug
attached to them.

### Versioning
In order to allow the public API to expand there are `version` fields in some
structures. When the versioned structures are expanded those version fields
need to be incremented to cover the new additions. The code then needs to guard
against the structure being received having the required version number in
order to validate the new additions are available.

## Trybot Access
Changes must pass the try bots before they are merged into the repo. For your
first few CLs the try bots will need to be triggered by a committer. After
you've submitted 2-3 CLs you can request try bot access by emailing one of the
OWNERS and requesting try bot access. This will allow you to trigger the bots
on your own changes without needing a committer.

## Committers
All changes committed to PDFium must be reviewed by a committer. Committers
have done significant work in the PDFium code base and have a good overall
understanding of the system.

Contributors can become committers as they exhibit a strong understanding
of the code base. There is a general requirement for ~10 non-trivial CLs to be
written by the contributor before being considered for committership. The
contributor is then nominated by an existing committer and if the nomination is
accepted by two other committers they receive committer status.

## OWNERS
The OWNERS files list long time committers to the project and have a broad
understanding of the code base and how the various pieces interact. In the
event of a code review stalling with a committer, the OWNERS are the first line
of escalation. The OWNERS files inherit up the tree, so an OWNER in a top-level
folder has OWNERS in the folders subdirectories.

There are a limited number of OWNERS files in PDFium at this time due to the
inherent interconnectedness of the code. We are hoping to expand the number of
OWNERS files to make them more targeted as the code quality progresses.

Committers can be added to OWNERS files when they exhibit a strong
understanding of the PDFium code base. This typically involves a combination of
significant CLs, code review on other contributor CLs, and working with the
other OWNERs to work through design and development considerations for the code.
An OWNER must be committed to upholding the principles for the long term health
of the project, take on a responsibility for reviewing future work, and
mentor new contributors. Once you are a committer, you should feel free to reach
out to the OWNERS who have reviewed your patches to ask what else they’d like to
see from you to be comfortable nominating you as an OWNER. Once nominated,
OWNERS are added or removed by rough consensus of the existing OWNERS.

## Escalations
There are times when reviews stall due to differences between reviewers,
developers and OWNERS. If this happens, please escalate the situation to one of
the people in the top-level OWNERS file (or another of the owners if already
discussing with a top-level owner). If the disagreement has moved up through
all the OWNERS files in the PDFium repo, the escalation should then proceed to
the Chromium
[ENG_REVIEW_OWNERS](https://chromium.googlesource.com/chromium/src/+/refs/heads/main/ENG_REVIEW_OWNERS)
as the final deciders.

The
[Standard of Code Review](https://google.github.io/eng-practices/review/reviewer/standard.html)
document has some good guidance on resolving conflicts during code review.

## CLA
All contributors must complete the Google contributor license agreement. For
individual contributors, please complete the
[Individual Contributor License Agreement](https://cla.developers.google.com/about/google-individual?csw=1)
online. Corporate contributors must fill out the
[Corporate Contributor License Agreement](https://cla.developers.google.com/about/google-corporate?csw=1)
and send it to us as described on that page.

Your first CL should add yourself to the
[AUTHORS](https://pdfium.googlesource.com/pdfium/+/refs/heads/main/AUTHORS)
file (unless you’re covered by one of the blanket entries).

### External contributor checklist for reviewers
Before LGTMing a change, ensure that the contribution can be accepted:
 * Definition: The "author" is the email address that owns the code review
   request on
   [https://pdfium-review.googlesource.com](https://pdfium-review.googlesource.com/)
 * Ensure the author is already listed in
   [AUTHORS](https://pdfium.googlesource.com/pdfium/+/refs/heads/main/AUTHORS).
   In some cases, the author's company might have a wildcard rule
   (e.g. \*@google.com).
 * If the author or their company is not listed, the CL should include a new
   AUTHORS entry.
   * Ensure the new entry is reviewed by a reviewer who works for Google.
   * Contributor License Agreement can be verified by Googlers at
     [http://go/cla](http://go/cla)
   * If there is a corporate CLA for the author‘s company, it must list the
     person explicitly (or the list of authorized contributors must say
     something like "All employees"). If the author is not on their company’s
     roster, do not accept the change.

## Legacy Code
The PDFium code base has been around in one form or another for a long time. As
such, there is a lot of legacy hidden in the existing code. There are surprising
interactions and untested corners of the code. We are actively working on
increasing code coverage on the existing code, and especially welcome additions
which move the coverage upwards. All new code should come with tests (either
unit tests or integration tests depending on the feature).

As part of this legacy nature, there is a good chance the code you’re working
with wasn’t designed to do what you need it to do. There are often refactorings
and bug fixes that end up happening along with feature development. Those
fixes/refactorings should be pulled out to their own changes with the
appropriate tests. This will make reviews a lot easier as, currently, it can be
hard to tell if there are far reaching effects of a given change.

There is a lot of existing technical debt that is being paid down in PDFium,
anything we can do here to make future development easier is a great benefit to
the project. This debt means means code reviews can take a bit longer if
research is needed to determine how a feature change will interact with the
rest of the system.
