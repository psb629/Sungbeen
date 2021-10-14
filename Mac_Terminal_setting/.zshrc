## ================================================= ##
## show $SHELL
echo $SHELL

## set zsh to be the default shell
 #chsh -s $(which zsh)

## Set name of the theme to load --- if set to "random", it will
## load a random theme each time oh-my-zsh is loaded, in which case,
## to know which specific one was loaded, run: echo $RANDOM_THEME
## See https://github.com/ohmyzsh/ohmyzsh/wiki/Themes
 #ZSH_THEME="amuse"

## Which plugins would you like to load?
## Standard plugins can be found in $ZSH/plugins/
## Custom plugins may be added to $ZSH_CUSTOM/plugins/
## Example format: plugins=(rails git textmate ruby lighthouse)
## Add wisely, as too many plugins slow down shell startup.
 #plugins=(git zsh-syntax-highlighting zsh-autosuggestions)

## aliases go here 
alias rm='rm -i'
alias cp='cp -i'
alias mv='mv -i'
alias la='ls -al'
alias ll='ls -l -tr'
alias cl='clear'
alias cnt='echo "# of directories : "; ll | grep ^d | wc -l ; echo "# of files : "; ll | grep ^- | wc -l'
alias dush='du -sh ./*'
alias df='df -h'
alias afni='afni /usr/local/afni/abin/MNI152_T1_2009c+tlrc'
alias mount_GoogleDrive='google-drive-ocamlfuse ~/GoogleDrive'
alias unmount_GoogleDrive='fusermount -u ~/GoogleDrive'
## ================================================= ##
