#include "external.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int external_process(char** token_list) {
    pid_t pid, wpid;
    int status; 

    // 创建子进程
    pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 0; 
    }
    if (pid == 0) {
        // 在子进程中执行命令
        if (execvp(token_list[0], token_list) == -1) {
            // perror("execvp failed");
            printf("%s: command not found\n", token_list[0]);
            exit(EXIT_FAILURE); 
        }
    } 
    else {
        // 在父进程中等待子进程完成
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
            if (wpid == -1) {
                perror("waitpid failed");
                return 0;
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1; 
}
