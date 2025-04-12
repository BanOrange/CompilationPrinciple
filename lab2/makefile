# 定义编译器为gcc
CC = gcc
# 定义词法分析器生成工具flex
FLEX = flex
# 定义语法分析器生成工具bison
BISON = bison
# 设置C编译器参数使用C99标准
CFLAGS = -std=c99

# 查找当前目录及子目录下所有.c文件
CFILES = $(shell find ./ -name "*.c")
# 将.c文件列表转换为对应的.o目标文件列表
OBJS = $(CFILES:.c=.o)
# 查找当前目录及子目录下所有.l（flex）文件
LFILE = $(shell find ./ -name "*.l")
# 查找当前目录及子目录下所有.y（bison）文件
YFILE = $(shell find ./ -name "*.y")
# 转换.l文件路径为生成的lex.yy.c文件路径（词法分析代码）
LFC = $(shell find ./ -name "*.l" | sed s/[^/]*\\.l/lex.yy.c/)
# 转换.y文件路径为生成的syntax.tab.c文件路径（语法分析代码）
YFC = $(shell find ./ -name "*.y" | sed s/[^/]*\\.y/syntax.tab.c/)
# 将lex.yy.c转换为对应的.o文件路径
LFO = $(LFC:.c=.o)
# 将syntax.tab.c转换为对应的.o文件路径
YFO = $(YFC:.c=.o)

# 最终构建目标：解析器可执行文件
parser: syntax $(filter-out $(LFO),$(OBJS))
	# 链接所有.o文件（排除词法分析.o）生成可执行文件，链接flex库
	$(CC) -o parser $(filter-out $(LFO),$(OBJS)) -lfl

# 生成语法分析相关目标文件
syntax: lexical syntax-c
	# 编译生成的syntax.tab.c为.o文件
	$(CC) -c $(YFC) -o $(YFO)

# 生成词法分析代码
lexical: $(LFILE)
	# 使用flex工具生成lex.yy.c词法分析器代码
	$(FLEX) -o $(LFC) $(LFILE)

# 生成语法分析代码
syntax-c: $(YFILE)
	# 使用bison工具生成语法分析器代码和头文件（-d），生成语法分析报告（-v）
	$(BISON) -o $(YFC) -d -v $(YFILE)

# 自动包含所有.o文件对应的依赖文件(.d)
-include $(patsubst %.o, %.d, $(OBJS))

# 定义伪目标（不生成实际文件）
.PHONY: clean test

# 测试目标：运行解析器测试多个测试用例
test:
	./parser ./test/test1.cmm
	./parser ./test/test2.cmm
	./parser ./test/test3.cmm
	./parser ./test/test4.cmm
	./parser ./test/test5.cmm
	./parser ./test/test6.cmm
	./parser ./test/test7.cmm
	./parser ./test/test8.cmm
	./parser ./test/test9.cmm
	./parser ./test/test10.cmm
	./parser ./test/test11.cmm
	./parser ./test/test12.cmm
	./parser ./test/test13.cmm
	./parser ./test/test14.cmm
	./parser ./test/test15.cmm
	./parser ./test/test16.cmm
	./parser ./test/test17.cmm

# 清理生成的文件
clean:
	rm -f parser lex.yy.c syntax.tab.c syntax.tab.h syntax.output
	rm -f $(OBJS) $(OBJS:.o=.d)     # 删除所有.o和.d文件
	rm -f $(LFC) $(YFC) $(YFC:.c=.h) # 删除生成的词法/语法分析文件
	rm -f *~                         # 删除编辑器临时文件