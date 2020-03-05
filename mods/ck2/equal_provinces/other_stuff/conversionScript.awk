#!/bin/awk -f
# To get in-place changes run like so:
# awk -i inplace -f .\conversionScript.awk '.\558 - Lower Dniepr.txt'
#
# This script changes province history files to have three baronies
# unless there is a tribal barony, in which case they are left unchanged,
# or there aren't any commented-out baronies to make three total, in which
# case nothing is done.

# NB this script creates bugs in files that have comments after the first column,
# e.g. 'b_longuyon = castle # Longuyon'. TODO: Fix that.
BEGIN {
  FS = " ";
  baronyCount = 0;
  tribal = 0;
  history = 0;
  histBaronies = 0;
}

{
  baronyIndex = index($0, "b_")
  commentIndex = index($0, "#")
  if (index($0, "800.1.1 = {") > 0) {
    history = 1
  }
  if (index($0, "}") > 0 && history > 0) {
    history = 0
  }
  if (baronyIndex > 0 && commentIndex == 0) {
    if (index($0, "= tribal") > 0) {
      tribal = 1
    } else {
      baronyCount = baronyCount + 1
    }
    if (history > 0 && index($0, "b_") > 0) {
      if ((index($0, "= castle") > 0) || (index($0, "= temple") > 0) || (index($0, "= city") > 0)) {
        histBaronies = histBaronies + 1
      }
    }
  }

  if (baronyIndex == 0 || tribal > 0) {
    print $0
    next
  }
  if (baronyCount > 3 && commentIndex == 0 && baronyIndex == 1) {
    print "#", $0
    next
  }
  if (baronyCount < 3 && commentIndex > 0) {
    baronyCount = baronyCount + 1
    print substr($0, commentIndex + 1, length($0))
    next
  }
  print $0
}

ENDFILE {
  if ((histBaronies + baronyCount != 3) && (histBaronies > 0)) {
    print ""
    print "# Candidate for by-hand fix", histBaronies, baronyCount
  }
}
