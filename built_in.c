#include "built_in.h"
#include "tools.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
// #include <linux/time.h>

char *built_in_cmds[] = {
    "touch",
    "rm",
    "cd",
    "pwd",
    "exit",
    "echo",
    "type",
    "env"
};

int (*built_in_list[])(char**) = {
    &my_touch,
    &my_rm,
    &my_cd,
    &my_pwd,
    &my_exit,
    &my_echo,
    &my_type,
    &my_env

};

extern int token_num;


int my_touch(char** token_list){
    if(NULL == token_list[1]){
        printf("Please enter the correct path!\n");
        return 1;
    }

    // 支持创建多个文件 
    int idx = 1;
    while(NULL != token_list[idx]){
        // 使用 SYS_open 系统调用来打开或创建文件
        int fd = syscall(SYS_open, token_list[idx], O_WRONLY | O_CREAT, 0644);
        if (-1 == fd) {
            perror("syscall SYS_open failed");
            return 1;
        }
        
        syscall(SYS_close, fd);

        // 使用 SYS_utimensat 系统调用来更新文件的时间戳
        struct timespec times[2];
        times[0].tv_sec = time(NULL);   
        times[0].tv_nsec = 0;           
        times[1].tv_sec = time(NULL);   // 修改时间
        times[1].tv_nsec = 0;           // 纳秒部分为 0

        // 使用 AT_FDCWD 表示当前工作目录
        if (syscall(SYS_utimensat, AT_FDCWD, token_list[idx], times, 0) == -1) {
            perror("syscall SYS_utimensat failed");
            return 1;
        }

        idx++;
    }
    return 1;
}

int my_rm(char** token_list){
    if(NULL == token_list[1]){
        printf("Please enter the correct path!\n");
        return 1;
    }else if(token_num != 2){
        printf("Too many parameters!\n");
        return 1;
    }

    struct stat path_stat;
    if (stat(token_list[1], &path_stat) != 0) {
        perror("stat failed");
        return 1;
    }

    if (S_ISDIR(path_stat.st_mode)) {
        // 若为文件夹
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
    if (NULL == token_list[1]) {
        printf("Please enter the correct directory\n");
    } else if (token_num != 2) {
        printf("Too many parameters!\n");
    } else {
        // 使用 syscall 调用 SYS_chdir 实现目录切换
        if (syscall(SYS_chdir, token_list[1]) != 0) {
            perror("Failed to change the directory");
        }
    }
    return 1;
}


int my_pwd(char** token_list) {
    char * pwd = get_pwd();
    if(NULL != pwd){
        printf("Present working directory: %s\n", pwd);
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
    if (NULL == token_list[1]){
        printf("Please enter the correct command name!\n");
        return 1;
    }
    else if(token_num != 2){
        printf("Too many parameters!\n");
        return 1;
    }
    
    for (int i = 0; i < built_IN_CMD_COUNT; i++){
        if (strcmp(token_list[1], built_in_cmds[i]) == 0){
            fprintf(stdout, "%s is a shell builtin\n", token_list[1]);
            return 1;
        }
    }

    char *full_path = get_external_command_path(token_list[1]);
    if(NULL != full_path){
        printf("%s is found at %s\n", token_list[1], full_path);
        free(full_path);
    }
    else{
        printf("%s: not found\n", token_list[1]);
    }
    return 1;
}


int my_env(char** token_list){
    extern char **environ;
    if (NULL == token_list[1]){
        for (int i = 0; environ[i]; i++)
            printf("%s\n", environ[i]);
    }
    return 1;
}