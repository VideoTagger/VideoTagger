#!/bin/bash

pushd "$(dirname "$0")/../docs" || exit

VT_VERSION=""
if [ -f "../VERSION" ]; then
  VT_VERSION=$(cat ../VERSION)
  VT_VERSION=${VT_VERSION// /}
  VT_VERSION=${VT_VERSION//	/}
fi

if [ -z "$VT_VERSION" ]; then
  echo "Error: VERSION file is empty or not found."
  exit 1
fi

if [ ! -d "src" ]; then
  mkdir -p src
fi

# Inject table of contents into README
cat ../README.md > src/README.autogen.md
echo >> src/README.autogen.md
echo "[TOC]" >> src/README.autogen.md
echo >> src/README.autogen.md

python3 ../scripts/setup_docs.py
python3 ../scripts/setup_docs_version_selector.py

chmod +x ../tools/bin/doxygen
../tools/bin/doxygen Doxyfile

popd || exit

read -rsn1 -p "Press any key to continue . . ."
echo -e
exit 0
