#!/usr/bin/env bash

set -euo pipefail

[ -t 1 ] && {
  red="\e[31m"
  green="\e[32m"
  reset="\e[0m"
} || {
  red=""
  green=""
  reset=""
}
outfile="$(mktemp --suffix=.png)"
fail=""

runtest() {
  ./deupscale "$1" "$outfile"
  set +e
  diff="$(compare -metric AE "$outfile" "$2" /dev/null 2>&1)"
  set -e
  if [ "$diff" = "0" ]
  then
    printf "%s: ${green}OK${reset}\n" "$1"
  else
    printf "%s: ${red}FAIL${reset}, %d wrong pixels\n" "$1" "$diff"
    file "$outfile"
    file "$2"
    fail="1"
  fi
}

runtest tests/haruka_1.png tests/haruka_orig.png
for i in {1..7}; do
  runtest tests/monalisa_$i.png tests/monalisa_orig.png
done

rm "$outfile"
[ -z "$fail" ] || exit 1
