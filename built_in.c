#include "built_in.h"
#include "tools.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/stat.h>

char *built_in_cmds[] = {
    "touch",
    "cd",
    "pwd",
    "exit",
    "echo",
    "type",
    "env",
    "rm"
};

int (*built_in_list[])(char**) = {
    &my_touch,
    &my_cd,
    &my_pwd,
    &my_exit,
    &my_echo,
    &my_type,
    &my_env,
    &my_rm
};

extern int token_num;


int my_touch(char** token_list){

}

int my_rm(char** token_list){
    if(NULL == token_list[1]){
        printf("Please enter the correct path!\n");
        return 1;
    }
    struct stat path_stat;
    if (stat(token_list[1], &path_stat) != 0) {
        perror("stat failed");
        return 1;
    }

    if (S_ISDIR(path_stat.st_mode)) {
        if(delete_directory(token_list[1]) != 1){
            printf("Failed to remove %s\n", token_list[1]);
        }
    } else {
        if(delete_file(token_list[1]) != 1){
            printf("Failed to remove %s\n", token_list[1]);
        }
    }
    return 1;
}


int my_cd(char** token_list){
    if(NULL == token_list[1]){
        printf("Please enter the correct directory\n");
    }
    else{
        if(chdir(token_list[1]) != 0){
            perror("Failed to change the dictionary");
        }
    }
    return 1;
}


int my_pwd(char** token_list) {
    char * pwd = get_pwd();
    if(NULL != pwd){
        printf("Current working directory: %s\n", pwd);
        free(pwd);
    }
    return 1;
}


int my_exit(char** token_list){
    printf("---------------------Welcome back!----------------------\n");
    return 0;
}


int my_echo(char** token_list){
    if (NULL == token_list[1]){
        printf("Please enter the correct output\n");
    }
    else{
        for (int i = 1; NULL != token_list[i]; i++)
        {
            printf("%s ", token_list[i]);
        }
        printf("\n");
    }
    return 1;
}

int my_type(char** token_list){
    if (NULL == token_list[1]) return 1;

    if(token_num != 2){
        printf("Too many parameters!\n");
        return 1;
    }
    
    for (int i = 0; i < built_IN_CMD_COUNT; i++){
        if (strcmp(token_list[1], built_in_cmds[i]) == 0){
            fprintf(stdout, "%s is a shell builtin\n", token_list[1]);
            return 1;
        }
    }

    char *fullpath = get_external_command_path(token_list[1]);
    if(NULL != fullpath){
        printf("%s is found at %s\n", token_list[1], fullpath);
        free(fullpath);
    }
    else{
        printf("%s: not found\n", token_list[1]);
    }
    return 1;
}


int my_env(char** token_list){
    extern char **environ;

    if (NULL == token_list[1])
    {
        for (int i = 0; environ[i]; i++)
            fprintf(stdout, "%s\n", environ[i]);
    }
    return 1;
}