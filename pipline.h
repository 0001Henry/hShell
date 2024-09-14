#ifndef _PIPLINE
#define _PIPLINE

// #define CLOSE "\001\033[0m\002"                 // 关闭所有属性
// #define BLOD  "\001\033[1m\002"                 // 强调、加粗、高亮
// #define BEGIN(x,y) "\001\033["#x";"#y"m\002"    // x: 背景，y: 前景


void shell_loop();

char **tokenizer(char *str);

int exe_cmds(char *cmds);

#endif