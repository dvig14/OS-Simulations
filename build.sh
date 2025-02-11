#!/bin/bash

build_run(){
    local dir=$1
    local source_file=$2
    local executable=$3
    shift 3

    cd $dir #change directory

    rm -f $executable #remove prv binary file(.exe)

    gcc -I.. -o $executable $source_file #compile code 

   ./$executable "$@" #run
}

# If no argument is provided
if [ -z "$1" ]; then
   echo "Please specify a file to compile and run."
   echo "Usage: ./build.sh <file_name>"
   exit 1
fi

case $1 in
      "scheduler")
       shift 1 # Remove the first argument (scheduler)
       build_run "cpu-schedulers" "scheduler.cpp" "scheduler" "$@"
      ;;
      "mlfq")
       shift 1
       build_run "cpu-mlfq" "mlfq.cpp" "mlfq" "$@"
      ;;
      "relocation")
       shift 1
       build_run "mem-relocation" "relocation.c" "relocation" "$@"
      ;;
      *)
       echo "Unknown file: $1"
      ;;
esac

