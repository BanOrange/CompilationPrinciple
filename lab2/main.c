#include <stdio.h>
#include <stdlib.h>
#include "node.h"
#include "semantic.h"
#include "global.h"

extern FILE *yyin;        // 输入文件流指�??
extern int yylex();                                    // 词法分析
extern int yyparse(void);                              // 语法分析
extern void yyrestart(FILE *input_file);               // 重置输入文件
extern void print_tree(struct Node *heads, int depth); // 打印语法树函�??

int errorFlag = 0; //表示当前语法分析是否发生了错�??
int errorLine=0; //记录上一次发生错误的行号，通过对比即可知道是否为新错误
struct Node* head = NULL; // 指向语法树的根节�??
// struct Node* Root = NULL;

int main(int argc,char** argv){
    if(argv <= 1) return 1;
    FILE* f = fopen(argv[1],"r");
    if(!f){
        perror(argv[1]);
        return 1;
    }
    yylineno = 1; //行号指定�??1
    yyrestart(f);
    yyparse();
    //没有错误的情况下再打印语法树
    if(errorFlag != 1){
        // print_tree(head,0);
        initHashTable();
        traverseTree(head);
    }
    //分析完毕
    return 0;
}