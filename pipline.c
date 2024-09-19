#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h> 
#include "built_in.h"
#include "pipline.h"
#include "external.h"
#include "tools.h"


int token_num = 0;

extern char *built_in_cmds[];

extern int (*built_in_list[])(char**);

void shell_loop(){
    char *cmds;
    int state = 1;

    while(state){
        char *prompt = get_prompt();

        cmds = readline(prompt);
        free(prompt);

        if (!cmds){
            perror("Allocate failed"); 
            exit(EXIT_FAILURE); 
        }
        
        add_history(cmds);

        char new_cmds[MAX_PATH_LEN];
        replace_env_variables(cmds, new_cmds, MAX_PATH_LEN);

        state = exe_cmds(new_cmds);

        free(cmds);

    }
}


char **tokenizer(char *str){
    int maxsize = 16;
    int idx = 0;

    char **list = (char **)malloc(maxsize*sizeof(char*));
    if(NULL == list){
        perror("Allocate failed"); 
        exit(EXIT_FAILURE); 
    }

    char *token = strtok(str, " ");

    while(NULL != token){
        list[idx++] = token;
        if(idx >= maxsize){
            maxsize += maxsize / 2;
            list = realloc(list, maxsize*sizeof(char*));
            if(NULL == list){
                perror("Realloc failed");
                exit(EXIT_FAILURE); 
            }
        }
        token = strtok(NULL, " ");
    }

    list = realloc(list, (idx+1)*sizeof(char*));
    if(NULL == list){
        perror("Realloc failed");
        exit(EXIT_FAILURE); 
    }

    list[idx] = NULL;
    
    token_num = idx;

    return list;
}


int exe_cmds(char *cmds){
    char** token_list = tokenizer(cmds);
    int state = 1;
    if(NULL == token_list){
        return 1;
    }

    int flag = 0;
    for (int i = 0; i < built_IN_CMD_COUNT; i++){
        // printf("%s\n", built_in_cmds[i]);
        if (strcmp(token_list[0], built_in_cmds[i]) == 0){
            state = (*built_in_list[i])(token_list);
            flag = 1; 
            break;
        }
    }
    if(!flag){
        state = external_process(token_list);
    }
    free(token_list);
    
    return state;
}