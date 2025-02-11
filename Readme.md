# Overview

For the simulators, the basic idea is simple: each of the simulators below let you both generate problems and obtain solutions for an infinite number of problems. Different random seeds can usually be used to generate different problems; using the `-c` flag computes the answers for you (presumably after you have tried to compute them yourself!).

To compile and run particular file :

```sh
prompt> ./build.sh fileName arguments
```
eg : Here build.sh is bash to automate compilation and execution of file. 'scheduler' is fileName you want to compile 
and execute and (-p,-j,-s) are flags or command line arguments. 

```sh
prompt> ./build.sh scheduler -p FIFO -j 3 -s 100
```