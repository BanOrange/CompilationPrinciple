#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "node.h"
#include "global.h"
#include "semantic.h"

//需要给出节点的名称和行�?
struct Node* create_Node(struct Node* node, char* name, int line) {
    // 分配内存空间
    struct Node* newnode = (struct Node*)malloc(sizeof(struct Node));
    // 设置节点属�?
    newnode->childNumber = 1; // judge�?1表示非终结符
    newnode->child = node; // 子节�?
    newnode->brother = NULL; // 兄弟节点
    newnode->linenumber = line; // 行号
    newnode->endFlag = 0; //默认为非终结�?
    newnode->emptyFlag = 0; //默认没有产生空串
    newnode->int_number = 1; // 整型�?
    strcpy(newnode->shikibetsuko, name); // 名称
    head = newnode; // 将当前节点设置为头节�?
    return newnode; // 返回新节�?
}

// 打印节点信息
void print_node(struct Node* now) {
    // 终结符或者产生式不为空产生式
    if (now->endFlag == 1 || now->emptyFlag == 0) {
        if (!strcmp(now->shikibetsuko, "ID\0")) {
            printf("ID: %s\n", now->char_name); // 标识�?
        } else if (!strcmp(now->shikibetsuko, "TYPE\0")) {
            printf("TYPE: %s\n", now->char_name); // 类型
        } else if (!strcmp(now->shikibetsuko, "INT\0")) {
            printf("INT: %u\n", now->int_number); // 整型�?
        } else if (!strcmp(now->shikibetsuko, "FLOAT\0")) {
            printf("FLOAT: %f\n", now->float_number); // 浮点数�?
        } else {
            printf("%s\n", now->shikibetsuko); //如果是其他类型的节点，那么直接输出标识子即可
        }
    } else {
        printf("%s (%d)\n", now->shikibetsuko, now->linenumber); // 输出非终结符节点信息
    }
}

// 打印语法�?
void print_tree(struct Node* now, int depth) {
    // 终结符和不产生空产生式的节点才可以被打印
    if (now->endFlag == 1 || now->emptyFlag == 0) {
        //根据当前节点所处的深度开始进行缩�?
        for (int i = 0; i < depth; ++i) printf("  ");
        print_node(now);
    }
    //向下递进打印子节点和兄弟节点
    if (now->child != NULL) print_tree(now->child, depth + 1);
    if (now->brother != NULL) print_tree(now->brother, depth);
    return;
}

// 打印错误信息
void printError(char errorType, char* msg) {
    // 打印错误类型和错误消息以及行�?
    fprintf(stderr, "Error type %c at Line %d: %s.\n", errorType, yylineno, msg);
    errorLine = yylineno; 
}

//因为我们已经用PrintError函数对错误进行了处理，所以不需要用系统提供的函�?
void yyerror(const char* s) {}

struct Node* create_Node_lexical(char* name) {
    struct Node* newnode=(struct Node*)malloc(sizeof(struct Node)); 
    newnode->childNumber=0;  // 判别值为0
    newnode->child=NULL; // 子节点为�?
    newnode->brother=NULL; // 兄弟节点为空
    newnode->linenumber = 0; // 行号�?0
    newnode->emptyFlag = 0; // 终结符自然没有空产生式的子节�?
    newnode->endFlag = 1; //为终结符�?
    strcpy(newnode->shikibetsuko,name); // 节点的类别，通过字符形式表达
    return newnode; // 返回创建的节�?
}

int OctalToInt(char* OCTAL){
    int i=1,n=0;
 	while(OCTAL[i]!='\0'){//此处判断不能为‘null，否则会判定错误
 		n=n*8+OCTAL[i]-'0';
 		i++;
 	}
 	// printf("该八进制数转化为十进制数应为�?%d\n",n);
    return n;
}

int HexToInt(char* HEX){
    int i=2,n=0;
 	while(HEX[i]!='\0'){//此处判断不能为‘null，否则会判定错误
 		n=n*16+getIndexOfSigns(HEX[i]);
 		i++;
 	}
 	// printf("该十六进制数转化为十进制数应为：%d\n",n);
    return n;
}

/* 返回ch字符在sign数组中的序号 */
int getIndexOfSigns(char ch) {
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    }
    return -1;
}


void traverseTree(struct Node *root)
{
    if (root == NULL){
        return;
    }
    if (strcmp(root->shikibetsuko, "ExtDefList") == 0)
    {
        ExtDefList(root); 
        return;
    }
    //遍历子节�?
    int number = root->childNumber;
    if(number == 0){
        return;
    }
    struct Node* temp = root->child;
    if (number != 0){
        traverseTree(temp);
        for (int i = 1; i < number; i++){
            temp = temp->brother;
            traverseTree(temp); 
        }
    }
            
}