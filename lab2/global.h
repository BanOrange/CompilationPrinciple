// global.h
#ifndef GLOBAL_H
#define GLOBAL_H

//本文件用于将一些全局变量统合在一起，防止重复声明导致编译失败
extern int errorFlag;
extern int errorLine;
extern struct Node* head;

#endif // GLOBAL_H