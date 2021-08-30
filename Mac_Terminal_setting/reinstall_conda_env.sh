#!/bin/zsh

# ================================
## target enviroment to refresh
env_name=sampark
# ================================
## remove
conda deactivate
conda env remove -n $env_name
jupyter kernelspec uninstall $env_name
# ================================
conda info --env
jupyter kernelspec list
## create
conda create --name $env_name python=3
source activate $env_name

is_env_sampark=`conda info | grep sampark | wc -l`
if [ $is_env_sampark -gt 0 ]; then
	echo "ready to pip install modules at env '${env_name}'"
	pip install -r modules.txt
	## generate a new kernel
	pip install ipykernel
	python -m ipykernel install --user --name $env_name --display-name $env_name
	## check the package installing
	pip freeze
else
	echo "pip install modules at env '${env_name}' is failed"
fi
