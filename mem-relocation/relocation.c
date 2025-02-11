#include "parser.h"

char *sub_str(char *str){
    int size = strlen(str);
    char *new_str = malloc(size*sizeof(char));
    if(!new_str){
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    int i = 0;
    while(i<size - 1){
        new_str[i] = str[i];
        i++;
    }
    new_str[i] = '\0';
    return new_str;
}

int cnvt_size(char *str){
    int size = strlen(str);
    char ch = str[size - 1];
    int new_size = 0;
    char *sub = NULL;

    if(ch == 'k' || ch == 'K'){
        sub = sub_str(str);
        new_size = atoi(sub) *1024;
    }
    else if(ch == 'm' || ch == 'M'){
        sub = sub_str(str);
        new_size = atoi(sub) *1024*1024;
    }
    else if(ch == 'g' || ch == 'G'){
        sub = sub_str(str);
        new_size = atoi(sub) *1024*1024*1024;
    }
    else new_size = atoi(str);

    if(sub != NULL) free(sub);
    
    return new_size;
}

void error_msg(char *msg){
    printf("Error: %s\n",msg);
}

int main(int argc, char *argv[]) {
   
    int seed = 0;
    char *asize = "1k";
    char *psize = "16k";
    int addr_num = 5;
    char *base = "-1";
    char *limit = "-1";
    bool solve = false;
 
    Option options[] = {
      {'s', "--seed", "0", "the random seed", "int", &seed},
      {'a', "--asize", "1k", "address space size (e.g., 16, 64k, 32m, 1g)", "string", &asize},
      {'p', "--physmem", "16k", "physical memory size (e.g., 16, 64k, 32m, 1g)", "string", &psize},
      {'n', "--addresses", "5", "number of virtual addresses to generate", "int", &addr_num},
      {'b', "--base", "-1", "value of base register", "string", &base},
      {'l', "--limit", "-1", "value of limit register", "string", &limit},
      {'c', "--compute", "false", "compute answers for me", "bool", &solve},
      {'h',"--help","false","show help message and exit"}
    };

    int num_opts = sizeof(options) / sizeof(Option);
    bool help = parser_options(argc, argv, options, num_opts);

    if(help){
      print_help("relocation",options,num_opts);
      return 0;
    }

    srand(seed);
    printf("ARG seed %i\n",seed);
    printf("ARG address space size %s\n",asize);
    printf("ARG phys mem size %s\n\n",psize);

    int new_asize = cnvt_size(asize);
    int new_psize = cnvt_size(psize);

    if(new_asize <= 0){
        error_msg("address space size should be +ve non-zero");
        return 0;
    }
    if(new_psize <= 1) {
        error_msg("physical memory size should be +ve non-zero");
        return 0;
    }
    if(new_asize >= new_psize){
        error_msg("physical memory size should be greater than address space size");
        return 0;
    }

    int new_limit = cnvt_size(limit);
    int new_base = cnvt_size(base);

    if(new_limit == -1) new_limit = (int)(new_asize / 4.0 + (new_asize / 4.0 * (rand()/(RAND_MAX + 1.0))));
    if(new_base == -1){
       int done = 0;
       while(!done){
          new_base = (int)(new_psize*(rand()/(RAND_MAX + 1.0)));
          if(new_base + new_limit < new_psize) done = 1;
       }
    }
    else if(new_base + new_limit > new_psize) {
        error_msg("address space does not fit into physical memory with those base/bounds values.");
        printf("Base + Limit: %i\tPsize: %i",new_base + new_limit,new_psize);
        return 0;
    }

    printf("Base-and-Bounds register information:\n\n");
    printf("  Base   : 0x%08x (decimal %d)\n",new_base,new_base);
    printf("  Limit  : %d\n\n",new_limit);

    printf("Virtual Address Trace\n");
    for(int i=0; i<addr_num; i++){

        int v_addr = (int)(new_asize * (rand()/(RAND_MAX + 1.0)));
        printf("  VA  %i: 0x%08x (decimal %d) -> ",i,v_addr,v_addr);

        if(!solve) printf("PA or segmentation violation?\n");
        else{
            if(v_addr >= new_limit) printf("SEGMENTATION VIOLATION\n");
            else{
                int PA = v_addr + new_base;
                printf("VALID: %08x (dec:%d)\n",PA,PA);
            }
        }
    }

    if(!solve){
        printf("\nFor each virtual address, either write down the physical address it translates to\n");
        printf("OR write down that it is an out-of-bounds address (a segmentation violation). For\n");
        printf("this problem, you should assume a simple virtual address space of a given size.\n");
    }
    printf("\n");
    
   return 0;
}