#!/bin/bash

#myvar='hello world'
#myvar=$1
#myvar="${myvar%.*}" 
#clang -S -emit-llvm "$myvar".c -o ""$myvar".ll"
#opt -mem2reg -S ""$myvar".ll" -o ""$myvar"reg.ll"
#rm -f ""$myvar".ll"

function ergodic(){

for file in ` ls $1`
do

                if [ -d $1"/"$file ] #如果 file存在且是一个目录则为真
                then
                      ergodic $1"/"$file
                elif [[ $file == *.c ]] 
		then
                      local path=$1"/"$file #得到文件的完整的目录
                      local name=$file       #得到文件的名字
			local name1=$2"/"$file".ll1"
                      local name2=$2"/"$file"reg.ll"
                      clang -S -emit-llvm $path -o $name1
		      opt -mem2reg -S $name1 -o $name2

                      #做自己的工作.
               fi

done

find $1  -name "*.ll1"|xargs rm -rf
}

ergodic $1 $2

