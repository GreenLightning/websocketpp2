#!/bin/sh
# Assemble the project website: the landing page, the Doxygen manual and
# reference under docs/, and the browser client example.
#
#   website/build.sh [output directory, default _site]
set -eu

root=$(cd "$(dirname "$0")/.." && pwd)
out="${1:-_site}"
case "$out" in
    /*) ;;
    *) out="$PWD/$out" ;;
esac

mkdir -p "$out/browser_client"
cd "$root"

# Settings after the Doxyfile override the ones in it.
{
    cat Doxyfile
    echo "OUTPUT_DIRECTORY = \"$out\""
    echo "HTML_OUTPUT = docs"
    echo "GENERATE_LATEX = NO"
    echo "EXCLUDE = build website"
} | doxygen -

cp website/index.html "$out/index.html"
cp website/style.css website/mark.svg "$out/"
cp examples/browser_client/index.html "$out/browser_client/index.html"
# The site is plain HTML, so Jekyll must not process it.
touch "$out/.nojekyll"
