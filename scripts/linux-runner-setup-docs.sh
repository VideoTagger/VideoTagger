#!/bin/bash

pushd "$(dirname "$0")/../docs" || exit

VT_VERSION=$(uv run ../scripts/get_vt_version.py)
if [ -z "$VT_VERSION" ]; then
  echo "Error: version in config.json is empty or not found."
  exit 1
fi
export VT_VERSION

uv run ../scripts/setup_docs.py
if [ $? -ne 0 ]; then
	echo "Error: Failed to set up tools."
	exit 1
fi
popd || exit
exit 0
