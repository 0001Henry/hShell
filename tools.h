#ifndef _TOOLS_H
#define _TOOLS_H


#define MAX_PATH_LEN 1024
#define MAX_STR_LEN 1024
#define BUFFER_SIZE 4096

void replace_env_variables(char *input, char *output, int max_size);

char *get_pwd(void);

char *get_prompt(void);

char* get_external_command_path(const char *command);

int delete_file(const char *path);

int delete_directory(const char *dir_path);

void print_file(const char *file_path, int show_line_numbers);

int copy_file(const char *src_path, const char *dest_path, int interactive, int preserve_attrs);

int copy_directory(const char *src_path, const char *dest_path, int interactive, int preserve_attrs);

void my_debug(int i);

#endif