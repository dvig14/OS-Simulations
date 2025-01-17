#!/bin/bash

build_run(){
    local dir=$1
    local source_file=$2
    local executable=$3
    shift 3

    cd $dir

    if [ -f "$executable" ];then
      if [ "$source_file" -nt "$executable" ];then # check timestamp (mtime) Modification time
        
        # Clean up previous build (delete the old executable)
        rm -f $executable

        # Compile the scheduler.cpp file with cxxopts.hpp from the parent directory
        g++ -I.. -o $executable $source_file
      else 
        echo "Executable is up-to-date. No need to recompile."
      fi
    else 
      echo "First compilation"
      gcc -I.. -o $executable $source_file
    fi

   #run
    ./$executable "$@"
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
       build_run "schedulers" "scheduler.cpp" "scheduler" "$@"
      ;;
      "mlfq")
       shift 1
       build_run "mlfq" "mlfq.cpp" "mlfq" "$@"
      ;;
      *)
       echo "Unknown file: $1"
      ;;
esac

