#ifndef _UTILS_H
#define _UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

/* 获取当前用户名 */
char *getCurrentUsername(void);

/* 获取主机名 */
char *getCurrentHostname(void);

/* 获取当前工作目录 */
char *getCurrentWorkingDirectory(void);

#endif