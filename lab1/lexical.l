%option yylineno

%{
# include <stdio.h>
# include <stdlib.h>
# include "syntax.tab.h"
int yycolumn = 1; // 设置yycolumn 初始值为 1


#define YY_USER_ACTION\
    yylloc.first_line=yylloc.last_line=yylineno;\
    yylloc.first_column=yycolumn;\
    yylloc.last_column=yycolumn+yyleng-1;\
    yycolumn+=yyleng;  // 实验指导书的定义宏，更新行列信息


struct Node* create_Node_lexical(char* name) {
    struct Node* newnode=(struct Node*)malloc(sizeof(struct Node)); 
    newnode->childNumber=0;  // 判别值为0
    newnode->child=NULL; // 子节点为空
    newnode->brother=NULL; // 兄弟节点为空
    newnode->linenumber = 0; // 行号为0
    newnode->emptyFlag = 0; // 终结符自然没有空产生式的子节点
    newnode->endFlag = 1; //为终结符号
    strcpy(newnode->shikibetsuko,name); // 节点的类别，通过字符形式表达
    return newnode; // 返回创建的节点
}
//声明函数
int OctalToInt(char* OCTAL);
int HexToInt(char* HEX);
int getIndexOfSigns(char ch);


// 暂时停用的正则表达式，会识别09
// FLOAT [-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?
%}

IGNORE [\r \t] 
CHANGELINE [\n] 
INT [1-9][0-9]{0,31}|0  
ID [_a-zA-Z][_0-9A-Za-z]*
RELOP >|<|>=|<=|==|!=
TYPE int|float


FLOAT [-+]?0|([1-9][0-9]*)\.?[0-9]+([eE][-+]?[0-9]+)?
OCTAL 0[0-7]{1,31}
HEX 0x[0-9A-Fa-f]{0,4}

SIGN \*\/
COMMENT \/\/.+\n?
COMMENTS \/\*([^*]|\*+[^\*\/])*\*+\/

%%
{COMMENT} {
    //遇到单行注释不进行处理
}

{COMMENTS} {
    //遇到多行注释同样不进行处理
    //且不考虑多行注释和单行注释嵌套的问题
}

";" {
        yylval = create_Node_lexical("SEMI\0"); //创建出来新的节点，并存到yylval
        return SEMI; //给语法分析器返回SEMI标志
    }

"," {
        yylval = create_Node_lexical("COMMA\0"); 
        return COMMA; 
    }

"=" {
        yylval = create_Node_lexical("ASSIGNOP\0"); 
        return ASSIGNOP; 
    }

{RELOP} {
        yylval = create_Node_lexical("RELOP\0"); 
        return RELOP; 
    }

"+" {
    yylval = create_Node_lexical("PLUS\0"); 
    return PLUS; 
}

"-" {
    yylval = create_Node_lexical("MINUS\0"); 
    return MINUS; 
}

"*" {
    yylval = create_Node_lexical("STAR\0"); 
    return STAR; 
}

"/" {
    yylval = create_Node_lexical("DIV\0"); 
    return DIV; 
}

"&&" {
    yylval = create_Node_lexical("AND\0"); 
    return AND; 
}

"||" {
    yylval = create_Node_lexical("OR\0"); 
    return OR; 
}

"." {
    yylval = create_Node_lexical("DOT\0"); 
    return DOT; 
}

"!" {
    yylval = create_Node_lexical("NOT\0"); 
    return NOT; 
}

{TYPE} {
    yylval = create_Node_lexical("TYPE\0"); 
    //对于type而言，识别到的类型要放到节点里面
    strcpy(yylval->char_name, yytext); 
    return TYPE; 
}

"(" {
    yylval = create_Node_lexical("LP\0"); 
    return LP; 
}

")" {
    yylval = create_Node_lexical("RP\0"); 
    return RP; 
}

"[" {
    yylval = create_Node_lexical("LB\0"); 
    return LB; 
}

"]" {
    yylval = create_Node_lexical("RB\0"); 
    return RB; 
}

"{" {
    yylval = create_Node_lexical("LC\0"); 
    return LC; 
}

"}" {
    yylval = create_Node_lexical("RC\0"); 
    return RC; 
}


"struct" {
    yylval = create_Node_lexical("STRUCT\0"); 
    return STRUCT; 
}

"return" {
    yylval = create_Node_lexical("RETURN\0"); 
    return RETURN; 
}

"if" {
    yylval = create_Node_lexical("IF\0"); 
    return IF; 
}

"else" {
    yylval = create_Node_lexical("ELSE\0"); 
    return ELSE; 
}

"while" {
    yylval = create_Node_lexical("WHILE\0"); 
    return WHILE; 
}


{IGNORE} {} //空白字符直接跳过

{CHANGELINE} {yycolumn=1;} // 遇到换行符将col号调整为1

{INT} {
    yylval = create_Node_lexical("INT\0"); 
    //识别到的数字要放到节点当中
    yylval -> int_number = atoi(yytext);
    return INT; 
}

{OCTAL} {
    //八进制的数字一样要创建int
    yylval = create_Node_lexical("INT\0"); 
    //识别到的数字转化为正常十进制然后放到节点当中
    yylval -> int_number = OctalToInt(yytext);
    return INT; 
}

{HEX} {
    //八进制的数字一样要创建int
    yylval = create_Node_lexical("INT\0"); 
    //识别到的数字转化为正常十进制然后放到节点当中
    yylval -> int_number = HexToInt(yytext);
    return INT; 
}

{FLOAT} {
    yylval = create_Node_lexical("FLOAT\0"); 
    // printf("检测到浮点数,%s",yytext);
    yylval -> float_number = atof(yytext);
    return FLOAT; 
}

{ID} {
    yylval = create_Node_lexical("ID\0"); 
    strcpy(yylval->char_name,yytext);
    return ID; 
}

. {
    errorFlag = 1; //发现词法分析错误
    char msg[32]; // 定义错误信息缓冲区
    sprintf(msg, "Mysterious character \"%s\"", yytext); // 格式化错误消息
    printError('A', msg); // 打印错误消息
}
%%

int OctalToInt(char* OCTAL){
    int i=1,n=0;
 	while(OCTAL[i]!='\0'){//此处判断不能为‘null，否则会判定错误
 		n=n*8+OCTAL[i]-'0';
 		i++;
 	}
 	// printf("该八进制数转化为十进制数应为：%d\n",n);
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