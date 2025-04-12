//用于给semantic.cpp程序进行结构体等的声�??
#ifndef _SEMANTIC_H_ // 如果未定义语义头文件
#define _SEMANTIC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"  

#define HASH_TABLE_SIZE 65536 // 哈希表大�??

#define INT_TYPE 24	 // 整型类型标识
#define FLOAT_TYPE 25 // 浮点型类型标�??

typedef enum Kind_
{
	BASIC = 20,
	ARRAY = 21,
	STRUCTURE = 22,
	FUNCTION = 23 // 类型种类：基本类型、数组、结构体、函�??
} Kind;

//定义类型
typedef struct Type_* Type;
typedef struct FieldList_* FieldList;

typedef struct Type_
{
    Kind kind; //类型
    union{
        int basic; //基本数据类型
        struct {Type elem;int size;} array; //数组
        FieldList structure; //结构体链�??
    } u;

    //函数类型对应的结构体
    struct{
        FieldList params; //参数
        Type funcType; //返回值类�??
        int paramNum;  //参数个数
    } function;

}Type_;

typedef struct FieldList_ {
    char* name;  //域的名字
    Type type; //域的类型
    FieldList tail; //下一个域
    int collision; //记录一下碰撞次�??
}FieldList_;


//下面根据不同的文法符号，进行不同的语义分�??
void ExtDefList(struct Node *root);					 // 外部定义列表
FieldList VarDec(struct Node *root, Type basictype); // 变量声明
Type Specifier(struct Node *root);					 // 类型说明
void CompSt(struct Node *root, Type funcType);		 // 复合语句
void DefList(struct Node *root);				     // 定义列表
void Stmt(struct Node *root, Type funcType);		 // 语句
Type Exp(struct Node *root);						 // 表达�??



//维护符号表的相关函数
unsigned int hash(char* name);  //哈希函数
void initHashTable();           //初始化哈希表
int insert(FieldList f);  //插入一个符�??
int typeEqual(Type type1,Type type2);//判断两个符号类型是否相同
FieldList lookupSymbol(char* name,int function); //在符号表中寻找对应的符号
void AllSymbol(); //展示所有的符号
//注，由於實驗當中不會出現特別大的測試用例且沒有垃圾回收機制，所以不需要從符號表中進行刪除


#endif // SEMANTIC_H