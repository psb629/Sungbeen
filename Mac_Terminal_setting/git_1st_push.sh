#!/bin/zsh

## Define a repository
rep=labs

## Define Git directory (root)
git_dir=~/Github
 #mkdir -p -m 755 $git_dir

## Define work directory
work_dir=$git_dir/$rep
cd $work_dir

## initializing git config
git init
git config --global user.name "psb629"
git config --global user.email "psb629@gmail.com"
git remote remove origin
git_id=psb629

## my_token, if you write this in one line, github will delete this token
aaa=69c34aad789ae9
bbb=72275d2ca4785
ccc=660d8e0d9aac8
git_password=${aaa}${bbb}${ccc} # personal access token
git remote add origin https://"$git_id":"$git_password"@github.com/psb629/$rep.git
git pull origin master

## The 1st push
res=$work_dir/ran_git_1st_push.txt
echo "> * `users`(`ipconfig getifaddr en0`): `date`" >>$res
git add $res
git commit -m "ran git_1st_push.sh"
git push --set-upstream origin master

