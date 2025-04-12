%{
# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include "lex.yy.c"
# include "node.h"
# include "global.h" 

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
        $$ -> childNumber = 1;
    }else{
        $$ = create_Node($1,"Program\0",@1.first_line);
        $$ -> childNumber = 1;
    }
};

ExtDefList:{
    // $$ = NULL;
    //空产生式，生成一个空的节点，同时将childNumber设为0
    $$ = create_Node(NULL,"ExtDefList\0",yylineno);
    $$ -> childNumber = 0;
    $$ -> emptyFlag= 1;
} | ExtDef ExtDefList{
    //生成两个非终结符号，子节点数量为2
    $$ = create_Node($1,"ExtDefList\0",@1.first_line);
    $$ -> childNumber = 2;
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
    $$ -> childNumber = 1;
} | StructSpecifier{
    $$ = create_Node($1,"Specifier\0",@1.first_line);
    $$ -> childNumber = 1; 
}

StructSpecifier: STRUCT OptTag LC DefList RC{
    $$ = create_Node($1,"StructSpecifier\0",@1.first_line); 
    $$ -> childNumber = 5; 
    $1 -> brother=$2; 
    $2 -> brother=$3; 
    $3 -> brother=$4;
    $4 -> brother=$5; 
} | STRUCT Tag{
    $$ = create_Node($1,"StructSpecifier\0",@1.first_line); 
    $$ -> childNumber = 2; 
    $1 -> brother=$2;
}

OptTag: {
    // $$ = NULL;
    $$ = create_Node(NULL,"OptTag\0",0); 
    $$ -> childNumber = 0;
    $$ -> emptyFlag = 1;   //表示该节点只有空产生式的子节点
} | ID{
    $$ = create_Node($1,"OptTag\0",@1.first_line); 
    $$ -> childNumber = 1; 
}

Tag:ID{
    $$ = create_Node($1,"Tag\0",@1.first_line); 
    $$ -> childNumber = 1; 
}

//Declaractors
VarDec:ID{
    $$ = create_Node($1,"VarDec\0",@1.first_line);
    $$ -> childNumber = 1; 
} | VarDec LB INT RB{
    $$ = create_Node($1,"VarDec\0",@1.first_line); 
    $$ -> childNumber = 4; 
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
    $$ -> childNumber = 4; 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
    $3 -> brother = $4; 
} | ID LP RP{
    $$ = create_Node($1,"FunDec\0",@1.first_line); 
    $$ -> childNumber = 3; 
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
    $$ -> childNumber = 3; 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
} | ParamDec{
    $$ = create_Node($1,"VarList\0",@1.first_line); 
    $$ -> childNumber = 1; 
}

ParamDec:Specifier VarDec{
    $$ = create_Node($1,"ParamDec\0",@1.first_line); 
    $$ -> childNumber = 2; 
    $1 -> brother = $2; 
}


CompSt:LC DefList StmtList RC{
    $$ = create_Node($1,"CompSt\0",@1.first_line);
    $$ -> childNumber = 4;  
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
    // $$ = NULL;
    $$ = create_Node(NULL,"StmtList\0",0); 
    $$ -> childNumber = 0;
    $$ -> emptyFlag=1; 
} | Stmt StmtList{
    $$ = create_Node($1,"StmtList\0",@1.first_line); 
    $$ -> childNumber = 2;
    $1 -> brother = $2; 
}


Stmt:Exp SEMI{
    $$ = create_Node($1,"Stmt\0",@1.first_line); 
    $$ -> childNumber = 2; 
    $1 -> brother = $2; 
} | CompSt{
    $$ = create_Node($1,"Stmt\0",@1.first_line); 
    $$ -> childNumber = 1; 
} | RETURN Exp SEMI{
    $$ = create_Node($1,"Stmt\0",@1.first_line); 
    $$ -> childNumber = 3; 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
} | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE{
    $$ = create_Node($1,"Stmt\0",@1.first_line); 
    $$ -> childNumber = 5; 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
    $3 -> brother = $4; 
    $4 -> brother = $5; 
} | IF LP Exp RP Stmt ELSE Stmt{
    $$ = create_Node($1,"Stmt\0",@1.first_line); 
    $$ -> childNumber = 7; 
    $1 -> brother=$2; 
    $2 -> brother=$3; 
    $3 -> brother=$4; 
    $4 -> brother=$5; 
    $5 -> brother=$6; 
    $6 -> brother=$7; 
} | WHILE LP Exp RP Stmt{
    $$ = create_Node($1,"Stmt\0",@1.first_line); 
    $$ -> childNumber = 5; 
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
    // $$ = NULL;
    $$ = create_Node(NULL,"DefList\0",0); 
    $$ -> childNumber = 0; 
    $$ -> emptyFlag = 1; 
} | Def DefList{
    $$ = create_Node($1,"DefList\0",@1.first_line); 
    $$ -> childNumber =2; 
    $1 -> brother = $2; 
}

Def:Specifier DecList SEMI{
    $$ = create_Node($1,"Def\0",@1.first_line); 
    $$ -> childNumber = 3; 
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
    $$ -> childNumber = 1; 
} | Dec COMMA DecList{
    $$ = create_Node($1,"DecList\0",@1.first_line); 
    $$ -> childNumber = 3; 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
}

Dec:VarDec{
    $$ = create_Node($1,"Dec\0",@1.first_line);
    $$ -> childNumber = 1;  
} | VarDec ASSIGNOP Exp{
    $$ = create_Node($1,"Dec\0",@1.first_line); 
    $$ -> childNumber = 3; 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
}

Exp:Exp ASSIGNOP Exp{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $$ -> childNumber = 3; 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
} | Exp AND Exp{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $$ -> childNumber = 3; 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
} | Exp OR Exp{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $$ -> childNumber = 3; 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
} | Exp RELOP Exp{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $$ -> childNumber = 3; 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
} | Exp PLUS Exp{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $$ -> childNumber = 3; 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
} | Exp MINUS Exp{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $$ -> childNumber = 3; 
    $1 -> brother=$2; 
    $2 -> brother=$3; 
} | Exp STAR Exp{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $$ -> childNumber = 3; 
    $1 -> brother=$2; 
    $2 -> brother=$3; 
} | Exp DIV Exp{
    $$ = create_Node($1,"Exp\0",@1.first_line);
    $$ -> childNumber = 3; 
    $1 -> brother=$2; 
    $2 -> brother=$3; 
} | LP Exp RP{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $$ -> childNumber = 3; 
    $1 -> brother=$2; 
    $2 -> brother=$3; 
} | MINUS Exp %prec LOWER_THAN_NOT{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $$ -> childNumber = 2; 
    $1 -> brother = $2; 
} | NOT Exp{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $$ -> childNumber = 2; 
    $1 -> brother = $2; 
} | ID LP Args RP{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $$ -> childNumber = 4; 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
    $3 -> brother = $4; 
} | ID LP RP{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $$ -> childNumber = 3; 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
} | Exp LB Exp RB{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $$ -> childNumber = 4; 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
    $3 -> brother = $4; 
} | Exp DOT ID{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $$ -> childNumber = 3; 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
} | ID{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $$ -> childNumber = 1; 
} | INT{
    $$ = create_Node($1,"Exp\0",@1.first_line); 
    $$ -> childNumber = 1; 
} | FLOAT{
    $$ = create_Node($1,"Exp\0",@1.first_line);
    $$ -> childNumber = 1;  
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
    $$ -> childNumber = 3; 
    $1 -> brother = $2; 
    $2 -> brother = $3; 
} | Exp{
    $$ = create_Node($1,"Args\0",@1.first_line); 
    $$ -> childNumber = 1; 
}


%%


