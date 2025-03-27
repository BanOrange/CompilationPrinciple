bison -vdty syntax.y

flex lexical.l

gcc -o lexical lex.yy.c y.tab.c

./lexical