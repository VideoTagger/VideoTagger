#!/bin/bash

cd $(dirname $0)/../
./tools/premake/linux\premake5 gmake2
cd -
read -p "Press enter to continue"