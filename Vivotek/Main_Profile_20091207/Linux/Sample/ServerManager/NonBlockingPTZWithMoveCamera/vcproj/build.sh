#!/bin/sh
ProjectName=NonBlockingPTZWithMoveCamera
echo "make" $ProjectName
make
echo "export LD_LIBRARY_PATH=../../../../../Bin_MainProfile/lib/:/lib/"
export LD_LIBRARY_PATH=../../../../../Bin_MainProfile/lib/:/lib/
echo "Run" $ProjectName
cd ../../"$ProjectName"_Build/Linux_Release/bin
./$ProjectName.exe