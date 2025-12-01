#!/bin/bash

folder='/home/script/'
files=`ls /home/script/| grep -E .c$` 



for i in $files
do
	 newName=`echo $folder$i | cut -d. -f1`
	`gcc -o $newName'.o' $folder$i`
done
