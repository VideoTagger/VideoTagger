#!/bin/bash

cd $(dirname $0)/../
chmod +x tools/premake/linux/premake5
./tools/premake/linux/premake5 gmake2
read -rsn1 -p "Press any key to continue . . . "