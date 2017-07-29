#!/bin/bash
# Author: Minzhao
# Date: 2017-3-13
# Verison: 0.1
# This script is to make sdk and generate *.bin for each application
# you could do like this to convert the file to linux line-ending format:
#       vi deliverSDK.sh
#       :set ff=unix and :wq

# set -x
set -e

tmp=$0
UTILITY_DIR=${tmp%/deliverSDK.sh*}
if [ ! -n $UTILITY_DIR ]
then
	echo "Path is null.	Exit"
	exit -1
fi
cd $UTILITY_DIR
tmp=`pwd`
AR8020SW=$tmp/..
OUTPUT_DIR=$AR8020SW/Output
SDK_DIR=$OUTPUT_DIR/AR8020SDK
#SDK_VERSION="AR8020SDK_""`sed -n '1,1p' $AR8020SW/Document/version`"
SDK_VERSION="AR8020SDK_""`cat  $AR8020SW/Document/version`"
SDK_VERSION_TAR="AR8020SDK_""`cat $AR8020SW/Document/version`"".tar.gz"
if [ ! -n $AR8020SW ]
then
	echo "Path is null.	Exit"
	exit -1
fi

if [[ ! -n $SDK_VERSION ]]; then
	echo "Path is null.	Exit"
	exit -1
else
	mkdir -p $AR8020SW/Output/$SDK_VERSION
	cp $AR8020SW/Application/Example $AR8020SW/Output/$SDK_VERSION -R
	mkdir -p $AR8020SW/Output/AR8020SDK/Example
	cp $AR8020SW/Application/Example $AR8020SW/Output/AR8020SDK -R
fi


#generate SDK
BUILD_DIR=$AR8020SW/Build/ConfigureFiles
 cd $BUILD_DIR
 for element in `ls $1`
 do  
   dir_or_file=$1"/"$element
  if [ -d $dir_or_file ]
  then 
      getdir $dir_or_file
   else
         tmp=${dir_or_file%.mk*}
         tmp=${tmp:7}
         # echo $tmp
         mkdir -p $AR8020SW/Output/$SDK_VERSION/$tmp
         cd ../Project/AR8020
         make configure PROJS=$tmp
         make 
         #generate application
         make sdk
         cp $OUTPUT_DIR/$SDK_VERSION_TAR $AR8020SW/Output/$SDK_VERSION/$tmp
         md5sum $AR8020SW/Output/$SDK_VERSION/$tmp/$SDK_VERSION_TAR > $AR8020SW/Output/$SDK_VERSION/$tmp/$SDK_VERSION_TAR".md5"
         cd $SDK_DIR/Application
         make sky
         mv app.bin $AR8020SW/Output/$SDK_VERSION/$tmp/sky_app.bin
         mv ar8020.bin $AR8020SW/Output/$SDK_VERSION/$tmp/sky_ar8020.bin
         make clean
         make ground
         mv app.bin $AR8020SW/Output/$SDK_VERSION/$tmp/ground_app.bin
         mv ar8020.bin $AR8020SW/Output/$SDK_VERSION/$tmp/ground_ar8020.bin
         make clean
         cd $AR8020SW/Build/Project/AR8020
         rm -rf $AR8020SW/Output/$SDK_VERSION_TAR
         rm -rf $AR8020SW/Output/*.bin
         make clean
         cd $BUILD_DIR
     fi
 done

# #delete SDKDIR
 rm -rf $SDK_DIR

#generate note document
cd $AR8020SW/Output/$SDK_VERSION
for dir in `ls .`
do
	if [[ -d $dir ]]; then
#		# echo $dir
		cd $dir
		touch releaseNote
		echo "1. Files:" > releaseNote
		for file in `ls $1`
		do
			dir_or_file=$1"/"$file
			if [ -d $dir_or_file ]
		    then 
		        getdir $dir_or_file
		    else
           		# if [[ -f $dir_or_file ]]; then
				tmp=${dir_or_file:1}
				echo "		$tmp" >> releaseNote
			fi
		done
		echo "2. Version:" >> releaseNote
		git log -1 > gitlog
		echo "		`sed -n '1,1p' gitlog`" >>releaseNote
		echo "		`sed -n '2,2p' gitlog`" >>releaseNote
		echo "		`sed -n '3,3p' gitlog`" >>releaseNote
		cd ..
	fi
done



