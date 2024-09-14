#include "utils.h"

/* 获取当前用户名 */
char *getCurrentUsername(void) {
    uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);
    if (pw != NULL) {
        return pw->pw_name;
    } else {
        return NULL;
    }
}

/* 获取主机名 */
char *getCurrentHostname(void) {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        hostname[sizeof(hostname) - 1] = '\0';
        return strdup(hostname);
    } else {
        return NULL;
    }
}

/* 获取当前工作目录 */
char *getCurrentWorkingDirectory(void) {
    char *dir = getcwd(NULL, 0);
    if (dir != NULL) {
        return dir;
    } else {
        perror("getcwd failed");
        return NULL;
    }
}

// int main() {
//     char *username = getCurrentUsername();
//     char *hostname = getCurrentHostname();
//     char *dir = getCurrentWorkingDirectory();

//     if (username != NULL) {
//         printf("Current Username: %s\n", username);
//         free(username); /* 释放内存 */
//     }

//     if (hostname != NULL) {
//         printf("Current Hostname: %s\n", hostname);
//         free(hostname); /* 释放内存 */
//     }

//     if (dir != NULL) {
//         printf("Current Working Directory: %s\n", dir);
//         free(dir); /* 释放内存 */
//     }

//     return 0;
// }