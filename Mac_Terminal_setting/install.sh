#!/bin/bash

#### zsh ####
 #which zsh
 #chsh -s /usr/bin/zsh
 #chsh -s /bin/zsh

#### Homebrew ####
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
## To update Homebrew, simply run:
 #brew update
 #brew upgrade

#### gcc and Xcode ####
brew install gcc

#### To solve the problem that 'xcrun: error: invalid active developer path' ####
xcode-select --install

#### Git ####
brew install git
git_dir=~/Github
mkdir -p -m 755 $git_dir
cd $git_dir
git clone https://github.com/psb629/Sungbeen.git
 #git clone https://github.com/clmnlab/labs.git
work_dir=$git_dir/Sungbeen
cd $work_dir
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
git remote add origin https://"$git_id":"$git_password"@github.com/psb629/Sungbeen.git
git pull origin master

echo "> * `users`(`ipconfig getifaddr en0`): `date`" >>$work_dir/Mac_Terminal_setting/README.md
git add $work_dir/Mac_Terminal_setting/README.md
git commit -m "ran install.sh"
git push --set-upstream origin master
 #git push -u origin master

#### vim ####
cp $work_dir/Mac_Terminal_setting/.vim ~/.vim
cp $work_dir/Mac_Terminal_setting/.vimrc ~/.vimrc
## type ":source ~/.vimrc", then ":PlugInstall" in any vim"

#### oh-my-zsh ####
## Xcode required
sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"
cat $work_dir/Mac_Terminal_setting/.zshrc >>~/.zshrc
## plugins
zsh=~/.zshrc
cp -n $zsh ~/.zshrc~
### zsh-syntax-highlighting
git clone https://github.com/zsh-users/zsh-syntax-highlighting.git ${ZSH_CUSTOM:-~/.oh-my-zsh/custom}/plugins/zsh-syntax-highlighting
### zsh-autosuggestions
git clone git://github.com/zsh-users/zsh-autosuggestions $ZSH_CUSTOM/plugins/zsh-autosuggestions
### renew
cat $zsh | sed -e "s/plugins=(git/plugins=(git zsh-syntax-highlighting zsh-autosuggestions/" >$zsh
### zsh-syntax-highlighting

#### python 3 ####
brew install python
## python --version -> Python 2.x
## python3 --version -> Python -> 3.x

#### Anaconda ####
## to uninstall anaconda clearly
 #sudo uninstall file:///Applications/Anaconda-Navigator.app/
 #sudo rm -rf /usr/local/anaconda3 /usr/local/Caskroom/anaconda /Users/clmnlab/.conda
 #echo "uninstalling anaconda is completed"
## installation
brew install anaconda
## add anaconda3 folder to our shell path
echo 'export PATH=$PATH:/usr/local/anaconda3/bin' >> ~/.zshrc
source ~/.zshrc
## update pip
pip install --upgrade pip
## make environment
env_name=sampark
conda create --name $env_name python=3
conda info --env
 #conda env list
## remove enviroment
 #conda remove --name $env_name --all
## install packages
 #conda activate $env_name
source activate $env_name
is_env_sampark=`conda info | grep sampark | wc -l`
if [ $is_env_sampark -gt 0 ]; then
	echo "ready to pip install modules at env '${env_name}'"
	pip install -r modules.txt
	## generate a new kernel
	pip install ipykernel
 #	jupyter kernelspec list
	python -m ipykernel install --user --name $env_name --display-name $env_name
 #	jupyter kernelspec uninstall $env_name
	## check the package installing
	pip freeze
else
	echo "pip install modules at env '${env_name}' is failed"
fi

#### XQuartz ####
brew install xquartz

#### afni ####
## XQuartz required
cd ~
update=@update.afni.binaries
pack=macos_10.12_local
curl -O https://afni.nimh.nih.gov/pub/dist/bin/misc/$update
tcsh $update -package $pack -do_extras
cp $HOME/abin/AFNI.afnirc $HOME/.afnirc
## Update AFNI to latest version
 #@update.afni.binaries -d
rm ~/$update

#### Subversion ####
 #brew install subversion

#### cmake ####
 #brew install cmake

#### llvm ####
brew install llvm
echo 'export PATH="/usr/local/opt/llvm/bin:$PATH"' >> ~/.zshrc
 #export LDFLAGS="-L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib"
 #export LDFLAGS="-L/usr/local/opt/llvm/lib"
 #export CPPFLAGS="-I/usr/local/opt/llvm/include"

## checkout LLVM including related sub-projects like Clang ####
 #git clone https://github.com/llvm/llvm-project.git

