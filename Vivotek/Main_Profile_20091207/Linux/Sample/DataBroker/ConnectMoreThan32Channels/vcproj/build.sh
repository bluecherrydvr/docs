#!/bin/sh
ProjectName=ConnectMoreThan32Channels 
echo "make" $ProjectName
make
echo "export LD_LIBRARY_PATH=../../../../../Bin_MainProfile/lib/:/lib/"
export LD_LIBRARY_PATH=../../../../../Bin_MainProfile/lib/:/lib/
echo "Run" $ProjectName
cp Device.ini ../../"$ProjectName"_Build/Linux_Release/bin
cd ../../"$ProjectName"_Build/Linux_Release/bin
./$ProjectName.exe
