#!/bin/zsh

dir_to=/mnt/ext6/NAS05
fname=/mnt/sda2/TM

parallel -j 10 cp -r -n {} $dir_to/ ::: $fname
