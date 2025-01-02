#!/bin/bash

if ! command -v pkg-config &> /dev/null
then
    echo "Error: pkg-config could not be found"
    exit 1
fi

if ! command -v python3 &> /dev/null
then
    echo "Error: python3 could not be found"
    exit 1
fi

if ! command -v python3-config &> /dev/null
then
    echo "Error: python3-config could not be found"
    exit 1
fi

cd $(dirname $0)/../
python3 scripts/setup.py
if [ $? -ne 0 ]; then
    echo "Error: Setup failed"
    exit 1
fi

chmod +x tools/bin/premake5
./tools/bin/premake5 xcode4
read -rsn1 -p "Press any key to continue . . ."
echo -e
