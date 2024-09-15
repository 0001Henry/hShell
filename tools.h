#ifndef _TOOLS_H
#define _TOOLS_H

#define MAX_PATH 1024

void replace_env_variables(char *input, char *output, int max_size);

char *get_pwd(void);

char *get_prompt(void);

char* get_external_command_path(const char *command);

int delete_file(const char *path);

int delete_directory(const char *dir_path);

#endif