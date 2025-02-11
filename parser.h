#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    char short_opt;
    const char *long_opt;
    const char *default_value;
    const char *help;
    const char *type;  // "int" or "string" or "bool"
    void *value;
}Option;

static inline void print_help(char *filename,Option options[], int num_opts){   
    printf("Usage: ./build.sh %s [options]\n\n",filename);
    printf("Options:\n");
    for(int i=0; i<num_opts; i++) printf("  -%c, %s\t%s\n",options[i].short_opt,options[i].long_opt,options[i].help);
}

static inline bool parser_options(int argc, char *argv[], Option options[], int num_opts){

    Option *op_map[26] = {NULL};

    for(int i=0; i<num_opts; i++) op_map[options[i].short_opt - 'a'] = &options[i];

    for(int i=1; i<argc; i++){

        if(argv[i][1] >= 'a' && argv[i][1] <= 'z'){
         
          unsigned int hash = (argv[i][1] - 'a');

          if(op_map[hash]){
            
            if(op_map[hash]->short_opt == 'h') return true;
            else if(op_map[hash]->short_opt == 'c') *((bool *)op_map[hash]->value) = true;
            else if(i+1 < argc && argv[i+1][0] != '-'){ 
               
               if(strcmp(op_map[hash]->type,"int") == 0) *((int *)op_map[hash]->value) = atoi(argv[++i]);   
               else if(strcmp(op_map[hash]->type,"string") == 0)  *((char **)op_map[hash]->value) = argv[++i];
               
            }
         }
        }
    }
    return false;
}

#endif