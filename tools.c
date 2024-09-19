#include "tools.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

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
    while ((entry = readdir(dir)) != NULL) {// 读取目录内容
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {// 跳过 "." 和 ".."
            continue;
        }
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);
        if (stat(path, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {// 如果是目录，递归删除
                if (delete_directory(path) == -1) {
                    closedir(dir);
                    return 0;
                }
            } else {

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

// 递归复制目录
int copy_directory(const char *src_path, const char *dest_path, int interactive, int preserve_attrs) {
    DIR *dir = opendir(src_path);
    if (dir == NULL) {
        perror("Failed to open directory");
        return 1;
    }

    // 创建目标目录 7:rwx  5:r-x  EEXIST:File Exists
    if (mkdir(dest_path, 0755) != 0 && errno != EEXIST) {
        perror("Failed to create directory");
        closedir(dir);
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // 跳过 `.` 和 `..`
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        char src_item_path[MAX_PATH_LEN];
        char dest_item_path[MAX_PATH_LEN];
        snprintf(src_item_path, sizeof(src_item_path), "%s/%s", src_path, entry->d_name);
        snprintf(dest_item_path, sizeof(dest_item_path), "%s/%s", dest_path, entry->d_name);

        struct stat st;
        if (stat(src_item_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                // 递归复制目录
                copy_directory(src_item_path, dest_item_path, interactive, preserve_attrs);
            } else {
                // 复制文件
                copy_file(src_item_path, dest_item_path, interactive, preserve_attrs);
            }
        }
    }
    closedir(dir);
    return 1;
}

// 复制文件，并支持交互确认和保留属性
int copy_file(const char *src_path, const char *dest_path, int interactive, int preserve_attrs) {
    // 交互式确认
    if (interactive && access(dest_path, F_OK) == 0) {
        // my_debug(1);
        printf("overwrite '%s'? (y/n): ", dest_path);
        char answer = getchar();
        if (answer != 'y' && answer != 'Y') {
            printf("Skipping '%s'\n", dest_path);
            return 1;
        }
    }

    // 打开源文件
    int src_fd = open(src_path, O_RDONLY);
    if (src_fd == -1) {
        perror("Failed to open source file");
        return 1;
    }

    // 打开或创建目标文件
    int dest_fd = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd == -1) {
        perror("Failed to open or create target file");
        close(src_fd);
        return 1;
    }

    // 读取并写入文件内容
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(src_fd, buffer, BUFFER_SIZE)) > 0) {
        if (write(dest_fd, buffer, bytes_read) != bytes_read) {
            perror("Failed to write to target file");
            close(src_fd);
            close(dest_fd);
            return 1;
        }
    }

    if (bytes_read == -1) {
        perror("Failed to read from source file");
    }

    // 保留文件属性
    if (preserve_attrs) {
        struct stat st;
        if (stat(src_path, &st) == 0) {
            // 设置权限
            if (chmod(dest_path, st.st_mode) != 0) {
                perror("Failed to set permissions");
            }
            // 设置所有者和组
            if (chown(dest_path, st.st_uid, st.st_gid) != 0) {
                perror("Failed to set owner/group");
            }
        } else {
            perror("Failed to get source file attributes");
        }
    }

    close(src_fd);
    close(dest_fd);

    return 1;
}


// 获取系统命令提示符
char *get_prompt(void) {

    char name[] = "$USER@$NAME";
    char new_name[MAX_PATH_LEN/4];
    replace_env_variables(name, new_name, MAX_PATH_LEN/4);

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


void my_debug(int i){
    printf("my_debug: %d\n", i);
}