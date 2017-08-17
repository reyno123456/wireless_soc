#!/bin/bash
# Author: Minzhao
# Date: 2016-9-23
# Verison: 0.1
# This script is used to cat cpu* binary file into one flash image
# Note: if you run this script and get the following error on linux: 
#       /bin/bash^M bad interpreter:No such file or directory
# you could do like this to convert the file to linux line-ending format:
#       vi joint2flash.sh
#       :set ff=unix and :wq 

# function helptext()
# {
#    echo "[Usage:] "$0" -i cpu0.txt cpu1.txt cpu2.txt -o flash.image"
#    exit
# }

function dec2hex()
{ 
    printf "%x" $1
}

TOP_DIR=

outputtmp=tmp.bin
outputconfig=config.bin
output=cfgdata.bin


Bin=../../Output/Staging/Config

tmpdir=`ls $Bin`

#config bin cat
for element in $tmpdir
do
    cat $Bin/$element >> $outputconfig
    echo $Bin/$element
    lengthTmp=`stat --format=%s $Bin/$element`
    replenishTmp=$((4-$lengthTmp%4))
    if [ $replenishTmp != 4 ]
    then
        for((a=0;a<=(($replenishTmp-1));a++))
        do
                echo -n -e \\x0 >> $outputconfig
        done
    fi
done

headflag=$((16#3553f708))
version=$((16#00000001))
length=`stat --format=%s $outputconfig`

for i in {0..3}
do
        shiftlen=$[ i * 8 ]
        tmp=`echo $headflag $shiftlen | awk '{print rshift($1,$2)}'`
        tmp=`echo $tmp | awk '{print and($1,255)}'`
        tmphex=$(dec2hex $tmp)
        echo -n -e \\x$tmphex >> $outputtmp
done
for i in {0..3}
do
        shiftlen=$[ i * 8 ]
        tmp=`echo $version $shiftlen | awk '{print rshift($1,$2)}'`
        tmp=`echo $tmp | awk '{print and($1,255)}'`
        tmphex=$(dec2hex $tmp)
        echo -n -e \\x$tmphex >> $outputtmp
done
for i in {0..3}
do
        shiftlen=$[ i * 8 ]
        tmp=`echo $length $shiftlen | awk '{print rshift($1,$2)}'`
        tmp=`echo $tmp | awk '{print and($1,255)}'`
        tmphex=$(dec2hex $tmp)
        echo -n -e \\x$tmphex >> $outputtmp
done

#config bin md5 cat
md5=`md5sum $outputconfig | cut -d ' ' -f 1`
echo $md5
for i in {0..15}
do
        shiftlen=$[ i * 2 + 1]
        tmp=`expr substr "$md5" $shiftlen 2`
        echo -n -e \\x$tmp >> $outputtmp
done

cat $outputconfig >> $outputtmp
cat $outputtmp > $output

rm $outputtmp
rm $outputconfig
