#ifndef _PIPLINE
#define _PIPLINE

void shell_loop();

char **tokenizer(char *str);

int exe_cmds(char *cmds);

#endif