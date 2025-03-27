#include <stdio.h>
#include <stdlib.h>

extern FILE *yyin;        // �����ļ���ָ��
extern int yylineno;      // ��ǰ�к�
extern int errorFlag;           // �Ƿ�������0��ʾ�޴���1��ʾ�д���
extern struct Node *head; // �﷨����ͷ�ڵ�ָ��
extern int yylex();                                    // �ʷ�����
extern int yyparse(void);                              // �﷨����
extern void yyrestart(FILE *input_file);               // ���������ļ�
extern void print_tree(struct Node *heads, int depth); // ��ӡ�﷨������

int main(int argc,char** argv){
    if(argv <= 1) return 1;
    FILE* f = fopen(argv[1],"r");
    if(!f){
        perror(argv[1]);
        return 1;
    }
    yylineno = 1; //�к�ָ��Ϊ1
    yyrestart(f);
    yyparse();
    //û�д����������ٴ�ӡ�﷨��
    if(errorFlag != 1){
        print_tree(head,0);
    }
    //�������
    return 0;
}