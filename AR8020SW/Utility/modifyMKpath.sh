#!/bin/bash
# Author: Minzhao
# Date: 2016-12-15
# Verison: 0.1
# This script is modify the related path arguments for sample code
# you could do like this to convert the file to linux line-ending format:
#       vi modifyMKpath.sh
#       :set ff=unix and :wq 

SDK_DIR=../../../Output/AR8020SDK
echo $SDK_DIR
#modify the sample top makefile
cd $SDK_DIR/Application
sed -i '/^TOP_DIR ?= /cTOP_DIR ?= ..'  ./Makefile
sed -i '/^OUTPUT_DIR ?= /cOUTPUT_DIR ?= $(TOP_DIR)'  ./Makefile
sed -i '/^OUTPUT_HEADER_STAGING_DIR ?= /cOUTPUT_HEADER_STAGING_DIR ?= $(OUTPUT_DIR)/Inc'  ./Makefile
sed -i '/^OUTPUT_LIB_STAGING_DIR ?= /cOUTPUT_LIB_STAGING_DIR ?= $(OUTPUT_DIR)/Lib'  ./Makefile
sed -i '/cp \*\.bin/d' ./Makefile



#modify the cpu0 sample top makefile
sed -i '/^TOP_DIR ?= /cTOP_DIR ?= ../..'  ./cpu0/Makefile
sed -i '/^OUTPUT_DIR ?= /cOUTPUT_DIR ?= $(TOP_DIR)'  ./cpu0/Makefile
sed -i '/^OUTPUT_HEADER_STAGING_DIR ?= /cOUTPUT_HEADER_STAGING_DIR ?= $(OUTPUT_DIR)/Inc'  ./cpu0/Makefile
sed -i '/^OUTPUT_LIB_STAGING_DIR ?= /cOUTPUT_LIB_STAGING_DIR ?= $(OUTPUT_DIR)/Lib'  ./cpu0/Makefile

#modify the cpu1 sample top makefile
sed -i '/^TOP_DIR ?= /cTOP_DIR ?= ../..'  ./cpu1/Makefile
sed -i '/^OUTPUT_DIR ?= /cOUTPUT_DIR ?= $(TOP_DIR)'  ./cpu1/Makefile
sed -i '/^OUTPUT_HEADER_STAGING_DIR ?= /cOUTPUT_HEADER_STAGING_DIR ?= $(OUTPUT_DIR)/Inc'  ./cpu1/Makefile
sed -i '/^OUTPUT_LIB_STAGING_DIR ?= /cOUTPUT_LIB_STAGING_DIR ?= $(OUTPUT_DIR)/Lib'  ./cpu1/Makefile

#modify the cpu2 sample top makefile
sed -i '/^TOP_DIR ?= /cTOP_DIR ?= ../..'  ./cpu2/Makefile
sed -i '/^OUTPUT_DIR ?= /cOUTPUT_DIR ?= $(TOP_DIR)'  ./cpu2/Makefile
sed -i '/^OUTPUT_HEADER_STAGING_DIR ?= /cOUTPUT_HEADER_STAGING_DIR ?= $(OUTPUT_DIR)/Inc'  ./cpu2/Makefile
sed -i '/^OUTPUT_LIB_STAGING_DIR ?= /cOUTPUT_LIB_STAGING_DIR ?= $(OUTPUT_DIR)/Lib'  ./cpu2/Makefile

#modify the joint2flash.sh
cd $SDK_DIR/Utility
echo `pwd`
sed -i '/^bootload=/cbootload=../Lib/ar8020_boot.bin'  ./joint2flash.sh
sed -i '/^upgrade=/cupgrade=../Lib/ar8020_upgrade.bin'  ./joint2flash.sh
sed -i '/^cpu0=/ccpu0=../Lib/ar8020_cpu0.bin'  ./joint2flash.sh
sed -i '/^cpu1=/ccpu1=../Lib/ar8020_cpu1.bin'  ./joint2flash.sh
sed -i '/^cpu2=/ccpu2=../Lib/ar8020_cpu2.bin'  ./joint2flash.sh
sed -i '/^ve=/cve=../Utility/imageinfo'  ./joint2flash.sh
sed -i '/^Bin=/cBin=../Bin'  ./joint2flash.sh

