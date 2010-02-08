#!/bin/sh
ProjectName=DecoderChannel
echo "make" $ProjectName
make
echo "export LD_LIBRARY_PATH=../../../../../Bin_MainProfile/lib/:/lib/"
export LD_LIBRARY_PATH=../../../../../Bin_MainProfile/lib/:/lib/
echo "Run" $ProjectName
cd ../../"$ProjectName"_Build/Linux_Release/bin

echo "Decode file  : testfile_v3.raw "
echo "output format: Bmp24"
echo "output file  : No"
./$ProjectName.out testfile_v3.raw 6 F
