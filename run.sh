#!/bin/bash

#Clean up
rm -f list*.txt

for file in BEC_SM*.nc
do
 gdalinfo $(gdalinfo $file | grep SUBDATASET_1_NAME | sed 's/\ \ SUBDATASET_1_NAME=//')
 echo $(gdalinfo $file | grep SUBDATASET_1_NAME | sed 's/\ \ SUBDATASET_1_NAME=//') > list1.txt
 echo $(gdalinfo $file | grep SUBDATASET_2_NAME | sed 's/\ \ SUBDATASET_2_NAME=//') > list2.txt
 echo $(gdalinfo $file | grep SUBDATASET_3_NAME | sed 's/\ \ SUBDATASET_3_NAME=//') > list3.txt
done

make clean
make
./becsmos $(cat list1.txt) $(cat list2.txt)
