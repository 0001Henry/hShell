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
    "env",
    "export",
    "unset",
    "cat",
    "cp"
};

int (*built_in_list[])(char**) = {
    &my_touch,
    &my_rm,
    &my_cd,
    &my_pwd,
    &my_exit,
    &my_echo,
    &my_type,
    &my_env,
    &my_export,
    &my_unset,
    &my_cat,
    &my_cp
};

extern int token_num;


int my_cp(char **token_list) {
    // my_debug(0);
    if (token_num < 3) {
        printf("Usage: cp [-i] [-p] source target\n");
        return 1;
    }

    int interactive = 0;
    int preserve_attrs = 0;
    int start_index = 1;

    // 处理选项
    if (strcmp(token_list[1], "-i") == 0) {
        interactive = 1;
        start_index++;
    }
    if (strcmp(token_list[1], "-p") == 0 || (interactive && strcmp(token_list[2], "-p") == 0)) {
        preserve_attrs = 1;
        start_index++;
    }

    const char *src_path = token_list[start_index];
    const char *dest_path = token_list[start_index + 1];

    struct stat st;
    if (stat(src_path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            // 复制目录
            return copy_directory(src_path, dest_path, interactive, preserve_attrs);
        } else {
            // 复制文件
            return copy_file(src_path, dest_path, interactive, preserve_attrs);
        }
    } else {
        perror("Failed to get file status");
    }
    return 1;
}


int my_export(char **token_list) {
    if (token_num != 2) {
        printf("Usage: export VAR=value\n");
        return 1;
    }

    // 提取环境变量的名称和值
    char *var = strtok(token_list[1], "=");
    char *value = strtok(NULL, "=");

    if (var == NULL || value == NULL) {
        printf("Invalid format. Usage: export VAR=value\n");
    }else{
        // 设置环境变量
        if (setenv(var, value, 1) != 0) {
            perror("Failed to set environment variable");
        }
    }
    return 1;
}

int my_unset(char **token_list) {
    if (token_num != 2) {
        printf("Usage: unset VAR\n");
        return 1;
    }

    char *var = token_list[1];

    if (unsetenv(var) != 0) {
        perror("Failed to unset environment variable");
    }

    return 1;
}


int my_touch(char** token_list){
    if (token_num < 2) {
        printf("Usage: touch [file1] [file2]...\n");
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
    if (token_num < 2) {
        printf("Usage: rm [file/directory]...\n");
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
    if (NULL == token_list[1] || strcmp(token_list[1],"~") == 0) {
        // printf("Please enter the correct directory\n");
        char *home_path = getenv("HOME");
        if (home_path == NULL) {
            printf("Failed to find HOME\n");
            return 1;  
        }
        else if (syscall(SYS_chdir, home_path) != 0) {
            perror("Failed to change the directory");
        }     
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
        // printf("Please enter the correct output\n");
        printf("\n");
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
    if(token_num != 2){
        printf("Usage: type cmd_name\n");
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


int my_cat(char **token_list){
    if(token_num != 2){
        printf("Usage: cat [-n] file\n");
        return 1;
    }
    
    int show_line_numbers = 0;  
    int start_index = 1;       

    // 检查是否提供了选项
    if (strcmp(token_list[1], "-n") == 0) {
        show_line_numbers = 1;  
        start_index = 2;       
    }

    // 逐个处理命令行参数中的文件
    for (int i = start_index; i < token_num; i++) {
        print_file(token_list[i], show_line_numbers);
    }
}
