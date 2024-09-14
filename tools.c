#include "tools.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <dirent.h>

// 删除文件函数，使用 SYS_unlink 系统调用
int delete_file(const char *path) {
    if (syscall(SYS_unlink, path) == 0) {
        printf("Deleted file: %s\n", path);
        return 1;
    } 
    else {
        perror("Failed to delete file");
        return 0;
    }
}

// 递归删除目录，使用 SYS_rmdir 系统调用
int delete_directory(const char *dir_path) {
    DIR *dir = opendir(dir_path);
    if (NULL == dir) {
        perror("opendir failed");
        return 0;
    }

    struct dirent *entry;
    char path[MAX_PATH];
    struct stat statbuf;

    // 读取目录内容
    while ((entry = readdir(dir)) != NULL) {
        // 跳过 "." 和 ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        if (stat(path, &statbuf) == 0) {
            // 如果是目录，递归删除
            if (S_ISDIR(statbuf.st_mode)) {
                if (delete_directory(path) == -1) {
                    closedir(dir);
                    return 0;
                }
            } else {
                // 否则删除文件
                if (syscall(SYS_unlink, path) == -1) {
                    perror("Failed to delete file");
                    closedir(dir);
                    return 0;
                }
            }
        }
    }

    closedir(dir);

    // 最后删除空目录
    if (syscall(SYS_rmdir, dir_path) == 0) {
        printf("Deleted directory: %s\n", dir_path);
        return 1;
    } else {
        perror("Failed to delete directory");
        return 0;
    }
}


/* 获取当前用户名和主机名 */
char *get_prompt(void) {

    // 获取用户名
    uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);
    char *username = pw ? pw->pw_name : "unknown";

    // 获取主机名
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        strcpy(hostname, "unknown-host");
    }

    // 获取当前工作目录
    char *cwd = get_pwd();
    if (!cwd) {
        cwd = strdup("unknown-dir");
    }

    // 拼接用户名、主机名和路径，生成类似shell的提示符字符串
    char prompt[MAX_PATH];  

    // 用于显示彩色的prompt
    snprintf(prompt, sizeof(prompt), "\001\033[49;32m\002\001\033[1m\002%s@%s\001\033[0m\002:\001\033[49;34m\002\001\033[1m\002%s\001\033[0m\002$ ", username, hostname, cwd);

    free(cwd);

    return strdup(prompt);
}


/* 获取当前工作目录 */
char* get_pwd(void) {
    char buf[MAX_PATH];
    if (-1 != syscall(SYS_getcwd, buf, sizeof(buf))) {
        return strdup(buf); 
    } 
    else {
        perror("syscall SYS_getcwd failed");
        return NULL;
    }
}



// 在 PATH 目录中查找可执行文件，不进行递归
char* get_external_command_path(const char *command) {
    char *path = getenv("PATH");
    if (path == NULL) {
        return NULL;  // PATH 未设置
    }

    // 创建一个副本，以避免破坏原始 PATH
    char *path_copy = strdup(path);
    if (path_copy == NULL) {
        perror("strdup failed");
        return NULL;
    }

    char *dir = path_copy;
    char *colon;

    // 循环遍历每个路径
    while ((colon = strchr(dir, ':')) != NULL) {
        *colon = '\0';  // 临时将 ':' 替换为 '\0'，方便构造目录

        // 构造完整路径
        char fullpath[MAX_PATH];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, command);

        // 检查文件是否存在且可执行
        if (access(fullpath, X_OK) == 0) {
            free(path_copy);  
            return strdup(fullpath);  
        }

        dir = colon + 1;  // 继续查找下一个目录
    }

    // 最后一个路径段的检查（没有 `:`）
    if (*dir != '\0') {
        char fullpath[MAX_PATH];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, command);

        if (access(fullpath, X_OK) == 0) {
            free(path_copy);
            return strdup(fullpath);
        }
    }

    free(path_copy);
    return NULL;  // 未找到命令
}
