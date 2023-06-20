<h1>JsonFS</h1>

<h2>Project description</h2>
This project is about fuse program via Json. 
<div></div>
FUSE stands for "Filesystem in Userspace." It is a software interface that allows developers to create custom file systems in user space rather than in the kernel. Typically, file systems are implemented as part of the operating system's kernel, but FUSE provides a way to create file systems as user-level programs.
<div></div>
Espeacially, I made FUSE for Json. I used cJSON header file and json.h. In this project, The most difficult problem is mount part because other function was working well but mount didn't work. The other reason mount was difficult was about system setting and library error. I resolved this problem with both cJSON.h and json.h.

<h2>How to run the code</h2>
You can run the code with build.sh and run.sh.
<div></div>
If build.sh, run.sh, and umount.sh is not executable mode, you need to command "chmod +x [three .sh file]"
<div></div>

`1. command ./build.sh`
` 2. command ./run.sh`

<h2>Information</h2>
1. This project is based on branch hw5 of https://github.com/hongshin/OperatingSystem.git. <div></div>
2. cJSON.h and cJSON.c are based on https://github.com/DaveGamble/cJSON.git.
