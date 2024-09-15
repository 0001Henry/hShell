#include "tools.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <dirent.h>

// 查找并替换环境变量
void replace_env_variables(char *input, char *output, int max_size) {
    int i = 0, j = 0;
    
    while (input[i] != '\0' && j < max_size - 1) {
        if (input[i] == '$') {
            // 检测到 '$'，开始提取环境变量名
            i++;
            char env_name[256] = {0}; 
            int env_index = 0;
            
            // while (input[i] != ' ' && input[i] != '\0' && input[i] != '\n' && env_index < 255) {
            while ((input[i] == '_' || (input[i] >= 'A' && input[i] <= 'Z')) && env_index < 255) {
                env_name[env_index++] = input[i++];
            }
            env_name[env_index] = '\0';

            // 获取环境变量的值
            char *env_value = getenv(env_name);
            if (NULL != env_value) {
                // 将环境变量值复制到输出
                int k = 0;
                while (env_value[k] != '\0' && j < max_size - 1) {
                    output[j++] = env_value[k++];
                }
            }
        } else {
            // 直接将普通字符复制到输出
            output[j++] = input[i++];
        }
    }
    output[j] = '\0';  // 确保输出以 null 结尾
}


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
    char path[MAX_PATH_LEN];
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


// 获取系统命令提示符
char *get_prompt(void) {

    char name[] = "$USER@$NAME";
    char new_name[MAX_PATH_LEN/2];
    replace_env_variables(name, new_name, MAX_PATH_LEN/2);

    // 获取当前工作目录
    char *pwd = get_pwd();
    if (!pwd) {
        pwd = strdup("unknown-dir");
    }

    // 拼接用户名、主机名和路径，生成类似shell的提示符字符串
    char prompt[MAX_PATH_LEN];  

    // 用于显示彩色的prompt
    snprintf(prompt, sizeof(prompt), "\001\033[49;32m\002\001\033[1m\002%s\001\033[0m\002:\001\033[49;34m\002\001\033[1m\002%s\001\033[0m\002$ ", new_name, pwd);

    free(pwd);

    return strdup(prompt);
}



/* 获取当前工作目录 */
char* get_pwd(void) {
    char buf[MAX_PATH_LEN];
    // 使用 syscall 调用 SYS_getcwd
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
        char full_path[MAX_PATH_LEN];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, command);

        // 检查文件是否存在且可执行
        if (access(full_path, X_OK) == 0) {
            free(path_copy);  
            return strdup(full_path);  
        }

        dir = colon + 1;  // 继续查找下一个目录
    }

    // 最后一个路径段的检查（没有 `:`）
    if (*dir != '\0') {
        char full_path[MAX_PATH_LEN];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, command);

        if (access(full_path, X_OK) == 0) {
            free(path_copy);
            return strdup(full_path);
        }
    }

    free(path_copy);
    return NULL;  // 未找到命令
}


void print_file(const char *file_path, int show_line_numbers) {

    
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        perror("cat: cannot open file");
        return ;
    }

    char buffer[MAX_STR_LEN];

    int line_number = 1;

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        if (show_line_numbers) {
            // 如果启用了行号显示，则打印行号
            printf("%6d  %s", line_number++, buffer);
        } else {
            printf("%s", buffer);
        }
    }
    printf("\n");
    fclose(file);  
}

// /* 获取当前用户名和主机名 */
// char *get_prompt(void) {

//     // 获取用户名
//     uid_t uid = getuid();
//     struct passwd *pw = getpwuid(uid);
//     char *username = pw ? pw->pw_name : "unknown";

//     // 获取主机名
//     char hostname[256];
//     if (gethostname(hostname, sizeof(hostname)) != 0) {
//         strcpy(hostname, "unknown-host");
//     }

//     // 获取当前工作目录
//     char *cwd = get_pwd();
//     if (!cwd) {
//         cwd = strdup("unknown-dir");
//     }

//     // 拼接用户名、主机名和路径，生成类似shell的提示符字符串
//     char prompt[MAX_PATH_LEN];  

//     // 用于显示彩色的prompt
//     snprintf(prompt, sizeof(prompt), "\001\033[49;32m\002\001\033[1m\002%s@%s\001\033[0m\002:\001\033[49;34m\002\001\033[1m\002%s\001\033[0m\002$ ", username, hostname, cwd);

//     free(cwd);

//     return strdup(prompt);
// }
