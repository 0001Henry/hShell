#ifndef _built_IN_H
#define _built_IN_H


#define built_IN_CMD_COUNT 8


// extern char *built_in_cmds[];

// extern int (*built_in_list[])(char**);

int my_touch(char** token_list);

int my_rm(char** token_list);

int my_cd(char** token_list);

int my_pwd(char** token_list);

int my_exit(char** token_list);

int my_echo(char** token_list);

int my_type(char** token_list);

int my_env(char** token_list);


#endif