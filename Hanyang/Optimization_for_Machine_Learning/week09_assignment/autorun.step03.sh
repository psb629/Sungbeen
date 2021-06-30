#!/bin/zsh

root_dir=/Users/clmn/Desktop/aiplatform_HW2_for_students_updated
step_dir=$root_dir/HW_3
source=opencl_host_HW3.c
kernel=matmul_HW3.cl
modified=opencl_host_HW3.cpp
filled=matmul_HW3.filled.cl
output_file=a.exe

cd $step_dir
## opencl_host_HW3, 커널파일이 바뀌었으므로 바뀐 커널로 불러오기. 
sed -e "s:$kernel:$filled:g" $source >$modified
## C++ compiler
g++ $modified -framework OpenCL -o $output_file #-I$root_dir/common -I$step_dir
## run
./$output_file
