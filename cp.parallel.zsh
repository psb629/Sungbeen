#!/bin/zsh

dir_to=/mnt/ext6/NAS05
fname=/mnt/sda2/TM

parallel -j 10 cp -r -n {} $dir_to/ ::: $fname

 #dir_from=/mnt/sda2/TM
 #dir_to=/mnt/ext6/NAS05/TM
 #
 #cd $dir_from
 #find -print0 | parallel -0 -j 10 cp -r -n {} $dir_to
