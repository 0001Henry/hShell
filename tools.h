#ifndef _TOOLS_H
#define _TOOLS_H

#define MAX_PATH 1024

char *get_pwd(void);

char *get_prompt(void);

char* get_external_command_path(const char *command);

int delete_file(const char *path);

int delete_directory(const char *dir_path);

#endif