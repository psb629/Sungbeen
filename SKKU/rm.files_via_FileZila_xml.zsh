#!/bin/zsh

grep "<LocalFile>" FileZilla.xml >list.txt

sed -i 's/\s\+<LocalFile>//g' list.txt
sed -i 's./Volumes..g' list.txt
sed -i 's.</LocalFile>..g' list.txt

xargs rm -r <list.txt

rm list.txt
