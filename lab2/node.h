#ifndef _NODE_H_ // 如果未定义语义头文件
#define _NODE_H_


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// 定义了一个结构体 Node,表示语法树中的节点
struct Node {
    struct Node* child;    // 指向第一个子节点的指针
    struct Node* brother;  // 指向后一个兄弟节点的指针，这样就可以连接起来所有的节点
    int linenumber;        // 节点所在的行号
    char shikibetsuko[32]; // 以字符形式表示的节点类型
    short endFlag;  //判断是否为终结符
    short emptyFlag; // 判断是否为语法单元并且产生空串，如果是的话那么就设置为1，反之设置为0
    int childNumber; //表示子节点的数量
    union {     //表示不同节点类型的值
        char char_name[32]; // 字符串类型
        int int_number;     // 整数类型
        float float_number; // 浮点数类型
    };
};
typedef struct Node Node;

extern int yylineno;
extern char* yytext;
extern int yylex();
void yyerror(const char* s); // 错误处理函数,暂时不需要，但是为了通过编译还是要加上



#define YYSTYPE struct Node* // 定义 YYSTYPE 类型，词法分析的返回值


struct Node* create_Node(struct Node* node,char* name,int line); //创建节点的函数
void print_tree(struct Node* now,int depth); // 打印语法树
void printError(char errorType, char* msg); // 打印错误信息

struct Node* create_Node_lexical(char* name);
//声明函数
int OctalToInt(char* OCTAL);
int HexToInt(char* HEX);
int getIndexOfSigns(char ch);

void traverseTree(struct Node *root);
#endif // NODE_H