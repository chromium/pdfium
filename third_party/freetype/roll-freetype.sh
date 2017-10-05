REVIEWERS=`paste -s -d, third_party/freetype/OWNERS` &&
roll-dep -r "${REVIEWERS}" "$@" pdfium/third_party/freetype/src/ &&
FTVERSION=`git -C third_party/freetype/src/ describe --long` &&
FTCOMMIT=`git -C third_party/freetype/src/ rev-parse HEAD` &&
sed -i "s/^Version: .*\$/Version: ${FTVERSION%-*}/" third_party/freetype/README.pdfium &&
sed -i "s/^Revision: .*\$/Revision: ${FTCOMMIT}/" third_party/freetype/README.pdfium &&
git add third_party/freetype/README.pdfium && git commit --quiet --amend --no-edit
