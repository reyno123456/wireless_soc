#!/bin/bash
# Author: Minzhao
# Date: 2017-1-6
# Verison: 0.1
# This script is modify the related path arguments for sample code
# you could do like this to convert the file to linux line-ending format:
#       vi modifyMKpath.sh
#       :set ff=unix and :wq
# you should run this script under path AR8020SW/* 


baseDir="."
addIndex=1
addCpFlag="true"
gitOld=$baseDir/AR8020SWOLD/
nowDIR=`pwd`
compare=`whoami`_`date +"%F_%H_%M"`
compOld=${baseDir}/${compare}/Old/
mkdir -p $compOld
compNew=${baseDir}/${compare}/New/
mkdir -p $compNew

gitVersion=`uname -a`
if [[ $gitVersion =~ "Linux" ]]; then
        echo "Now working in Linux Git..."
        workFlag="linux"
        desDIR=/LnxShare/Software/CodeReview
        num=13

else
        echo "Now working in Git Bash..."
        workFlag="bash"
        desDIR=//file/LnxShare/Software/CodeReview/
        num=12

fi

git status > tempf1
git clone -b develop git@git.artosyn.com:jingliu/AR8020SW.git -q $gitOld

if [[ $workFlag == "bash" ]]; then
	for line in $(cat tempf1)
	do
		if [ $line == "modified:" ];then
			flag="modify"
			continue
		elif [ $line == "deleted:" ];then
			flag="delete"
			continue
		elif [ $line == "Untracked" ];then
			flag="add"
			continue
		else
			if [[ $flag == "add" ]]; then
				if [[ $addIndex -le $num ]]; then
					let addIndex+=1
					continue
				fi
			fi
		fi

		# echo $flag
		if [[ $flag = "modify" ]];then
			#copy the new files
			filenameNew=$(dirname $line)
			mkdir -p $compNew$filenameNew
			cp -r $line $compNew$filenameNew
			
			#copy the old files
		    filenameOld=$filenameNew
			mkdir -p $compOld$filenameOld
			cp -r $gitOld$line $compOld$filenameOld
		elif [[ $flag = "add" ]];then
			#copy the new files
			if [[ $line == "no" ]]; then
				addCpFlag="false"
			fi

			if [[ $addCpFlag == "true" ]]; then
				if [[ $line == "tempf1" ]]; then
					continue
				fi
				filenameNew=$(dirname $line)
			    mkdir -p $compNew$filenameNew
				cp -r $line $compNew$filenameNew
			fi
		elif [[ $flag = "delete" ]];then
			#copy the old files
			filenameNew=$(dirname $line)
		    filenameOld=$filenameNew
			mkdir -p $compOld$filenameOld
			cp -r $gitOld$line $compOld$filenameOld
		else 
			flag="none"
		fi
	done
else
	for line in $(cat tempf1)
	do
		if [ $line == "modified:" ];then
			flag="modify"
			continue
		elif [ $line == "deleted:" ];then
			flag="delete"
			continue
		elif [ $line == "Untracked" ];then
			flag="add"
			continue
		else
			if [[ $flag == "add" ]]; then
				if [[ $addIndex -le $num ]]; then
					let addIndex+=1
					continue
				fi
			fi
		fi

		# echo $flag
		if [[ $flag = "modify" ]];then
			#copy the new files
			if [[ $line != "#" ]]; then
				filenameNew=$(dirname $line)
				mkdir -p $compNew$filenameNew
				cp -r $line $compNew$filenameNew
				
				#copy the old files
			    filenameOld=$filenameNew
				mkdir -p $compOld$filenameOld
				cp -r $gitOld$line $compOld$filenameOld
			fi

		elif [[ $flag = "add" ]];then
			#copy the new files
                        if [[ -e $line ]] ; then
				if [[ $line == "no" ]]; then
					addCpFlag="false"
				fi

				if [[ $addCpFlag == "true" ]]; then
					if [[ $line == "tempf1" ]]; then
						continue
					fi
					filenameNew=$(dirname $line)
				    mkdir -p $compNew$filenameNew
					cp -r $line $compNew$filenameNew
				fi
			fi
		elif [[ $flag = "delete" ]];then
			#copy the old files
			if [[ $line != "#" ]]; then
				filenameNew=$(dirname $line)
			    filenameOld=$filenameNew
				mkdir -p $compOld$filenameOld
				cp -r $gitOld$line $compOld$filenameOld
			fi
		else 
			flag="none"
		fi
	done
fi

rm -rf tempf1
rm -rf $gitOld
cp -rf $baseDir/${compare}/ $desDIR
rm -rf $baseDir/${compare}/
echo "Create the diff folder done!"
