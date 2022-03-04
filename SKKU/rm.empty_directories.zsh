#!/bin/zsh

dir_root=/clmnlab/JS/TM/topup_data

find $dir_root -tpye f -name ".DS_Store" -delete
## Synology
 #find $dir_root -type d -name "@eaDir" -delete
find $dir_root -type d -empty -delete
