%{
# include <stdio.h>
# include <string.h>
# include <stdlib.h>

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

extern int yylineno;
extern char* yytext;
extern int yylex();
void yyerror(const char* s); // 错误处理函数,暂时不需要，但是为了通过编译还是要加上

int errorFlag = 0; //表示当前语法分析是否发生了错误
int errorLine=0; //记录上一次发生错误的行号，通过对比即可知道是否为新错误

#define YYSTYPE struct Node* // 定义 YYSTYPE 类型，词法分析的返回值

struct Node* head=NULL; // 指向语法树的根节点
struct Node* create_Node(struct Node* node,char* name,int line); //创建节点的函数
void print_tree(struct Node* now,int depth); // 打印语法树
void printError(char errorType, char* msg); // 打印错误信息
%}

//下面开始进行token和产生式的构建
%token SEMI
%token COMMA
%token ASSIGNOP
%token RELOP
%token PLUS
%token MINUS
%token STAR
%token DIV
%token AND
%token OR
%token NOT
%token DOT
%token LP
%token RP
%token LB
%token RB
%token LC
%token RC
%token STRUCT
%token RETURN
%token IF
%token ELSE
%token WHILE
%token TYPE
%token INT
%token FLOAT
%token ID


//此处定义一下token的结合性
%right ASSIGNOP
%left RELOP
%left AND OR  //和 或 左结合
%left PLUS MINUS STAR DIV //加减乘除都是左结合
%right LOWER_THAN_NOT NOT // 非操作符 右结合
%right DOT LP LB RP RB // 点和括号 右结合
%nonassoc LOWER_THAN_ELSE // 比 else 小的操作符
%nonassoc ELSE // else 操作符 不关联

%%

//下面开始按照产生式来写对应的处理和分析树的构建

//High-level Definitions
Program: ExtDefList{
    //检查是否为空串产生语法单元，是的话继续保持，反之更新行号
    if($1->emptyFlag == 1){  
        $$ = create_Node($1,"Program\0",$1->emptyFlag);  //不需要打印了
    }else{
        $$ = create_Node($1,"Program\0",@1.first_line);
    }
};

ExtDefList:{
    //空产生式，生成一个空的节点，同时将childNumber设为0
    $$ = create_Node(NULL,"ExtDefList\0",yylineno);
    $$ -> emptyFlag= 1;
} | ExtDef ExtDefList{
    //生成两个非终结符号，子节点数量为2
    $$ = create_Node($1,"ExtDefList\0",@1.first_line);

    //构建兄弟关系
    $1 -> brother = $2;
}

ExtDef:Specifier ExtDecList SEMI{
    $$ = create_Node($1,"ExtDef\0",@1.first_line);
    $$ -> childNumber = 3;
    $1 -> brother = $2;
    $2 -> brother = $3;
} | Specifier SEMI{
    $$ = create_Node($1,"ExtDef\0",@1.first_line);
    $$ -> childNumber = 2;
    $1 -> brother = $2;
} | Specifier FunDec CompSt{
    $$ = create_Node($1,"ExtDef\0",@1.first_line);
    $$ -> childNumber = 3;
    $1 -> brother = $2;
    $2 -> brother = $3;
} | Specifier error SEMI{
    errorFlag = 1; 
    char msg[32]; 
    sprintf(msg, "Syntax error, near \'%c\'", yytext[0]); 
    if (errorLine != yylineno) {
        printError('B', msg); 
    }   
} | Specifier error{  //用error指代错误，在SEMI等特殊位置附近
    errorFlag = 1; //发生了错误
    if(errorLine != yylineno){
        printError('B',"first Missing \";\"");
    }
} 

ExtDecList:VarDec{
    $$ = create_Node($1,"ExtDecList",@1.first_line);
    $$ -> childNumber = 1;
} | VarDec COMMA ExtDecList{
    $$ = create_Node($1,"ExtDecList",@1.first_line);
    $$ -> childNumber = 3;
    $1 -> brother = $2;
    $2 -> brother = $3;
} | VarDec error ExtDecList{
    errorFlag = 1;
    if(errorLine != yylineno){
        printError('B',"text");
    }
}


//Specifiers
Specifier: TYPE{
    $$ = create_Node($1,"Specifier\0",@1.first_line); 
} | StructSpecifier{
    $$ = create_Node($1,"Specifier\0",@1.first_line); 
}

StructSpecifier: STRUCT OptTag LC DefList RC{
    $$ = create_Node($1,"StructSpecifier\0",@1.first_line); 
    $1 -> brother=$2; 
    $2 -> brother=$3; 
    $3 -> brother=$4;
    $4 -> brother=$5; 
} | STRUCT Tag{
    $$ = create_Node($1,"StructSpecifier\0",@1.first_line); 
    $1 -> brother=$2;
}

OptTag: {
    $$ = create_Node(NULL,"OptTag\0",0); 
    $$ -> emptyFlag = 1;   //表示该节点只有空产生式的子节点
} | ID{
    $$ = create_Node($1,"OptTag\0",@1.first_line); 
}

Tag:ID{
    $$ = create_Node($1,"Tag\0",@1.first_line); 
}

//Declaractors
VarDec:ID{
    $$ = create_Node($1,"VarDec\0",@1.first_line);
} | VarDec LB INT RB{
    $$ = create_Node($1,"VarDec\0",@1.first_line); 
    $1 -> brother = $2;
    $2 -> brother = $3; 
    $3 -> brother = $4; 
} | VarDec LB error RB {
    errorFlag = 1; 
    char msg[32]; 
    sprintf(msg, "Syntax error, near \'%c\'", yytext[0]); 
    if (errorLine != yylineno) {
        printError('B', msg); 
    }    
}

FunDec:ID LP VarList RP{
    $$ = create_Node($1,"FunDec\0",@1.first_line); 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
    $3 -> brother = $4; 
} | ID LP RP{
    $$ = create_Node($1,"FunDec\0",@1.first_line); 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
} | ID LP error RP {
    errorFlag = 1; 
    if (errorLine != yylineno){
        printError('B', "Syntax error between ()"); 
    }
        
}

VarList:ParamDec COMMA VarList{
    $$ = create_Node($1,"VarList\0",@1.first_line);
    $1 -> brother = $2; 
    $2 -> brother = $3; 
} | ParamDec{
    $$ = create_Node($1,"VarList\0",@1.first_line); 
}

ParamDec:Specifier VarDec{
    $$ = create_Node($1,"ParamDec\0",@1.first_line); 
    $1 -> brother = $2; 
}


CompSt:LC DefList StmtList RC{
    $$ = create_Node($1,"CompSt\0",@1.first_line); 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
    $3 -> brother = $4; 
} 
// | LC error StmtList RC{
//     errorFlag = 1; 
//     if (errorLine != yylineno){
//         printError('B', "Syntax error between {}"); 
//     }
// }

StmtList: {
    $$ = create_Node(NULL,"StmtList\0",0); 
    $$ -> emptyFlag=1; 
} | Stmt StmtList{
    $$ = create_Node($1,"StmtList\0",@1.first_line); 
    $1 -> brother = $2; 
}


Stmt:Exp SEMI{
    $$ = create_Node($1,"Stmt\0",@1.first_line); 
    $1 -> brother = $2; 
} | CompSt{
    $$ = create_Node($1,"Stmt\0",@1.first_line); 
} | RETURN Exp SEMI{
    $$ = create_Node($1,"Stmt\0",@1.first_line); 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
} | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE{
    $$ = create_Node($1,"Stmt\0",@1.first_line); 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
    $3 -> brother = $4; 
    $4 -> brother = $5; 
} | IF LP Exp RP Stmt ELSE Stmt{
    $$ = create_Node($1,"Stmt\0",@1.first_line); 
    $1 -> brother=$2; 
    $2 -> brother=$3; 
    $3 -> brother=$4; 
    $4 -> brother=$5; 
    $5 -> brother=$6; 
    $6 -> brother=$7; 
} | WHILE LP Exp RP Stmt{
    $$ = create_Node($1,"Stmt\0",@1.first_line); 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
    $3 -> brother = $4; 
    $4 -> brother = $5; 
} | Exp error {
    errorFlag = 1; 
    if (errorLine != yylineno){
        printError('B', "second Missing \";\""); 
    }
} 
// | WHILE LP error RP{
//     errorFlag = 1; 
//     if (errorLine != yylineno){
//         printError('B', "Syntax error between ()"); 
//     }
// } | IF LP error RP{
//     errorFlag = 1; 
//     if (errorLine != yylineno){
//         printError('B', "Syntax error between ()"); 
//     }
// } | RETURN Exp error{
//     errorFlag = 1; 
//     if (errorLine != yylineno){
//         printError('B', "Missing \";\""); 
//     }
// }

DefList: {
    $$ = create_Node(NULL,"DefList\0",0); 
    $$ -> emptyFlag = 1; 
} | Def DefList{
    $$ = create_Node($1,"DefList\0",@1.first_line); 
    $1 -> brother = $2; 
}

Def:Specifier DecList SEMI{
    $$ = create_Node($1,"Def\0",@1.first_line); 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
} | Specifier error SEMI{
    errorFlag = 1; 
    char msg[32]; 
    sprintf(msg, "Syntax error, near \'%c\'", yytext[0]); 
    if (errorLine != yylineno){
       printError('B', msg); 
    } 
} | Specifier DecList error {
    errorFlag = 1; 
    if(errorLine != yylineno){
        printError('B',"fourth Missing \";\"");
    }
}

DecList:Dec{
    $$ = create_Node($1,"DecList\0",@1.first_line); 
} | Dec COMMA DecList{
    $$ = create_Node($1,"DecList\0",@1.first_line); 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
}

Dec:VarDec{
    $$ = create_Node($1,"Dec\0",@1.first_line); 
} | VarDec ASSIGNOP Exp{
    $$ = create_Node($1,"Dec\0",@1.first_line); 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
}

Exp:Exp ASSIGNOP Exp{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
} | Exp AND Exp{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
} | Exp OR Exp{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
} | Exp RELOP Exp{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
} | Exp PLUS Exp{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
} | Exp MINUS Exp{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $1 -> brother=$2; 
    $2 -> brother=$3; 
} | Exp STAR Exp{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $1 -> brother=$2; 
    $2 -> brother=$3; 
} | Exp DIV Exp{
    $$ = create_Node($1,"Exp\0",@1.first_line);
    $1 -> brother=$2; 
    $2 -> brother=$3; 
} | LP Exp RP{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $1 -> brother=$2; 
    $2 -> brother=$3; 
} | MINUS Exp %prec LOWER_THAN_NOT{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $1 -> brother = $2; 
} | NOT Exp{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $1 -> brother = $2; 
} | ID LP Args RP{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
    $3 -> brother = $4; 
} | ID LP RP{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
} | Exp LB Exp RB{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
    $3 -> brother = $4; 
} | Exp DOT ID{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
} | ID{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
} | INT{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
} | FLOAT{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
} | Exp LB error RB {
    errorFlag = 1; 
    if (errorLine != yylineno)
        printError('B', "Syntax error between \"[]\""); 
} | Exp ASSIGNOP error {
    errorFlag = 1; 
    if (errorLine != yylineno) 
        printError('B', "Syntax error in Exp"); 
} | LP error RP {
    errorFlag = 1; 
    if (errorLine != yylineno) 
        printError('B', "Syntax error in Exp"); 
} | ID LP error RP{
    errorFlag = 1; 
    if (errorLine != yylineno) 
        printError('B', "Syntax error in Exp"); 
} | Exp LB Exp error RB{
    errorFlag = 1; 
    if (errorLine != yylineno) 
        printError('B', "fifth Missing \"]\"");
};

Args:Exp COMMA Args{
    $$ = create_Node($1,"Arg\0",@1.first_line); 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
} | Exp{
    $$ = create_Node($1,"Args\0",@1.first_line); 
}


%%
#include "lex.yy.c"

//需要给出节点的名称和行号
struct Node* create_Node(struct Node* node, char* name, int line) {
    // 分配内存空间
    struct Node* newnode = (struct Node*)malloc(sizeof(struct Node));
    // 设置节点属性
    newnode->childNumber = 1; // judge为1表示非终结符
    newnode->child = node; // 子节点
    newnode->brother = NULL; // 兄弟节点
    newnode->linenumber = line; // 行号
    newnode->endFlag = 0; //默认为非终结符
    newnode->emptyFlag = 0; //默认没有产生空串
    newnode->int_number = 1; // 整型值
    strcpy(newnode->shikibetsuko, name); // 名称
    head = newnode; // 将当前节点设置为头节点
    return newnode; // 返回新节点
}

// 打印节点信息
void print_node(struct Node* now) {
    // 终结符或者产生式不为空产生式
    if (now->endFlag == 1 || now->emptyFlag == 0) {
        if (!strcmp(now->shikibetsuko, "ID\0")) {
            printf("ID: %s\n", now->char_name); // 标识符
        } else if (!strcmp(now->shikibetsuko, "TYPE\0")) {
            printf("TYPE: %s\n", now->char_name); // 类型
        } else if (!strcmp(now->shikibetsuko, "INT\0")) {
            printf("INT: %u\n", now->int_number); // 整型值
        } else if (!strcmp(now->shikibetsuko, "FLOAT\0")) {
            printf("FLOAT: %f\n", now->float_number); // 浮点数值
        } else {
            printf("%s\n", now->shikibetsuko); //如果是其他类型的节点，那么直接输出标识子即可
        }
    } else {
        printf("%s (%d)\n", now->shikibetsuko, now->linenumber); // 输出非终结符节点信息
    }
}

// 打印语法树
void print_tree(struct Node* now, int depth) {
    // 终结符和不产生空产生式的节点才可以被打印
    if (now->endFlag == 1 || now->emptyFlag == 0) {
        //根据当前节点所处的深度开始进行缩进
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
    // 打印错误类型和错误消息以及行号
    fprintf(stderr, "Error type %c at Line %d: %s.\n", errorType, yylineno, msg);
    errorLine = yylineno; 
}

//因为我们已经用PrintError函数对错误进行了处理，所以不需要用系统提供的函数
void yyerror(const char* s) {}

