#!/bin/bash

if ! command -v pkg-config &> /dev/null
then
    echo "pkg-config could not be found"
    exit 1
fi

cd $(dirname $0)/../
chmod +x tools/premake/macos/premake5
./tools/premake/macos/premake5 xcode4
read -rsn1 -p "Press any key to continue . . ."
echo -e