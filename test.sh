#!/usr/bin/env bash

set -euo pipefail

outfile="$(mktemp --suffix=.png)"

runtest() {
  ./deupscale "$1" "$outfile"
  diff="$(magick compare -metric AE "$outfile" "$2" /dev/null 2>&1)"
  if [ "$diff" = "0" ]
  then
    printf "%s: OK\n" "$1"
  else
    printf "%s: FAIL, %d wrong pixels\n" "$1" "$diff"
    file "$outfile"
    file "$2"
  fi
}

runtest tests/haruka_1.png tests/haruka_orig.png
for i in {1..7}; do
  runtest tests/monalisa_$i.png tests/monalisa_orig.png
done

rm "$outfile"
