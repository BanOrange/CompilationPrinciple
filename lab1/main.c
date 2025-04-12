#include <stdio.h>
#include <stdlib.h>

extern FILE *yyin;        // 输入文件流指针
extern int yylineno;      // 当前行号
extern int errorFlag;           // 是否发生错误，0表示无错误，1表示有错误
extern struct Node *head; // 语法树的头节点指针
extern int yylex();                                    // 词法分析
extern int yyparse(void);                              // 语法分析
extern void yyrestart(FILE *input_file);               // 重置输入文件
extern void print_tree(struct Node *heads, int depth); // 打印语法树函数

int main(int argc,char** argv){
    if(argv <= 1) return 1;
    FILE* f = fopen(argv[1],"r");
    if(!f){
        perror(argv[1]);
        return 1;
    }
    yylineno = 1; //行号指定为1
    yyrestart(f);
    yyparse();
    //没有错误的情况下再打印语法树
    if(errorFlag != 1){
        print_tree(head,0);
    }
    //分析完毕
    return 0;
}