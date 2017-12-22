#include <stdio.h>
#include <string.h>
#include <sys/resource.h>

// #include "run.h"
#include "hexdump.h"
#include "util.h"

query *query_list = 0;
char** alloc_list = 0;
int query_cnt = 0;
int mem_cnt = 0;
int fit_flag = FIRST_FIT;
int print_flag = 1;

void load(char *file) {
    FILE *input;
    int step_flag = 0, i = 0;
    char buffer[256];

    if (!(input = fopen(file, "r"))) {
        printf("Error: Can't open file %s\n", file);
        exit(-1);
    }

    while(fgets(buffer, 256, input) != 0){
        if(step_flag == 0) {

            query_cnt = atoi(buffer);
            query_list = (query*)malloc(sizeof(query) * query_cnt);

            step_flag =1;
        } else {
            query_list[i].type = buffer[0];

            if(query_list[i].type == 'm' || query_list[i].type == 'e') {
                mem_cnt++;
            }

            strncpy(query_list[i].val, buffer + 2, strlen(buffer + 2) - 1);

            i++;
        }
    }
    fclose(input);

    alloc_list = (char**)malloc(sizeof(char*) * mem_cnt);


}

int main(int argc, char *argv[]) {

    int i = 0, j = 0;
    int cnt = 0;
    int target = 0, rsize = 0;
    int tsize = 0;

    /* Error Checking */
    if (argc < 2) {
        printf("Usage: %s [-b or -w] input_file\n", argv[0]);
        exit(1);
    }

    load(argv[argc-1]);


    while(cnt != argc-1){
        if(strcmp(argv[cnt], "-b") == 0){
            fit_flag = BEST_FIT;
        }
        else if(strcmp(argv[cnt], "-w") == 0) {
            fit_flag = WORST_FIT;
        } else if(strcmp(argv[cnt], "-np") == 0) {
            print_flag =0;
        }

        cnt++;
    }
    for(i = 0 ; i < query_cnt ; i++) {
        switch(query_list[i].type) {
            case 'm':
                alloc_list[j] = m_malloc(strlen(query_list[i].val) + 1);
                strcpy(alloc_list[j], query_list[i].val);
                j++;
                break;
            case 'f':
                target = atoi(query_list[i].val);
                m_free(alloc_list[target]);
                break;
            case 'a':
                sscanf(query_list[i].val, "%d %d", &target, &rsize);
                tsize = strlen(alloc_list[target]) + rsize;
                alloc_list[target] = m_realloc(alloc_list[target], tsize);
                break;
            case 'd':
                sscanf(query_list[i].val, "%d %d", &target, &rsize);
                tsize = strlen(alloc_list[target]) - rsize;
                alloc_list[target] = m_realloc(alloc_list[target], tsize);
                *(alloc_list[target] + tsize) = '\0';
                break;
            case 'e':
                alloc_list[j++] = m_malloc(atoi(query_list[i].val));
                break;
        }

        print_block_list();
    }

    return 0;
}

