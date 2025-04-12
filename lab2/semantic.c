#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// 实现semantic.h中声明好的函数

// 采用哈希表作为符号表
FieldList hashTable[HASH_TABLE_SIZE];

/*
下面為哈希符號表相關的函數，用於維護符號表
*/
// hash函数，来自于实验指导书
unsigned int hash(char *name)
{
    unsigned int val = 0, i;
    for (; *name; ++name)
    {                             // 遍历字符串名字
        val = (val << 2) + *name; // 更新哈希值
        if (i = val & ~0x3fff)
        {                                     // 如果存在高位溢出
            val = (val ^ (i >> 12)) & 0x3fff; // 则进行一次异或操作
        }
        return val % HASH_TABLE_SIZE; // 返回哈希值
    }
}

// 初始化哈希符号表
void initHashTable()
{
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        hashTable[i] = NULL; // 初始为空，等待后续加入
    }
}

// 插入符号的函数
int insert(FieldList f)
{
    if (f == NULL)
    {
        return 0; // 如果为空就直接返回
    }
    if (f->name == NULL)
    {
        return 0; // 没有名字也直接返回，不然插入之后也无法再次寻找出来
    }

    f->collision = 0; // 碰撞次数设为0
    unsigned int key; // 哈希值
    if (f->type->kind == 23)
    { // 对应于函数类型,额外增加一些扰动
        key = hash(1949 + f->name);
    }
    else
    {
        key = hash(f->name);
    }

    if (hashTable[key] == NULL)
    { // 为空直接插入即可
        hashTable[key] = f;
        return 1;
    }
    // 碰撞需要进行处理
    int oldKey = key;
    while (true)
    {
        key++;
        // 转了一圈仍未找到
        if (key == oldKey)
        {
            break;
        }
        key = key % HASH_TABLE_SIZE;     // 不断循环哈希表
        f->collision = f->collision + 1; // 碰撞次数加一,后面可以用来快速寻址
        if (hashTable[key] == NULL)
        {
            hashTable[key] = f;
            return 1;
        }
    }

    printf("Wrong!The hashtable is full!\n");
    return 0; // 找不到位置插入符号
}

int typeEqual(Type type1, Type type2)
{
    if ((type1 == NULL) || (type2 == NULL))
    {
        return 0;
    }

    if (type1->kind != type2->kind)
    {
        return 0;
    }
    else
    {
        switch (type1->kind)
        {
        case BASIC:
        {
            if (type1->u.basic == type2->u.basic)
            {
                return 1;
            }
            else
            {
                return 0;
            }
            break;
        }
        break;
        case ARRAY:
        {
            // 需要比较数组的元素类型
            if (typeEqual(type1->u.array.elem, type2->u.array.elem) == 1)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
        break;
        case STRUCTURE:
        {
            // 对比两个类型中的结构体属性
            FieldList field1 = type1->u.structure;
            FieldList field2 = type2->u.structure;
            if (field1 != NULL && field2 != NULL)
            {
                while (field1 != NULL && field2 != NULL)
                {
                    // 发现不同直接传递即可
                    if (typeEqual(field1->type, field2->type) == 0)
                    {
                        return 0;
                    }
                    // 向后比较
                    field1 = field1->tail;
                    field2 = field2->tail;
                }

                // 同时出现NULL也算是相等，返回1
                if (field1 == NULL && field2 == NULL)
                {
                    return 1;
                }
            }
            return 0;
        }
        break;
        case FUNCTION:
        {
            if (type1->function.paramNum != type2->function.paramNum)
            {
                return 0;
            }
            FieldList param1 = type1->function.params;
            FieldList param2 = type2->function.params;
            for (int i = 0; i < type1->function.paramNum; i++)
            {
                if (typeEqual(param1->type, param2->type) == 0)
                {
                    return 0;
                }
                param1 = param1->tail;
                param2 = param2->tail;
            }
            return 1;
        }
        break;
        default:
            return 0;
            break;
        }
    }
}

// 尋找符號,第二個參數的意思是是否為函數
FieldList lookupSymbol(char *name, int function)
{
    if (name == NULL)
    {
        return NULL;
    }
    unsigned int key;
    if (function)
    {
        key = hash(1949 + name);
    }
    else
    {
        key = hash(name);
    }

    FieldList list = hashTable[key];
    while (list != NULL)
    {
        if (strcmp(name, list->name) == 0)
        {
            if ((function == 1) && (list->type->kind == FUNCTION))
            {
                return list;
            }
            if ((function == 0) && (list->type->kind != FUNCTION))
            {
                return list;
            }
        }

        // 找不到可能是發生碰撞了，這個時候需要遍歷
        key++;
        key = key % HASH_TABLE_SIZE;
        list = hashTable[key];
    }
    return NULL;
}

// 遍歷整個哈希符號表
void AllSymbol()
{
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        if (hashTable[i] != NULL)
        {
            printf("symbolName: %s,symbolKind:%d\n", hashTable[i]->name, hashTable[i]->type->kind);
        }
    }
}

/*
下面開始處理相關的文法符號，SDT
*/

// 变量声明的语法树节点
// 可能产生一个变量名或者一个多维的变量数组
FieldList VarDec(struct Node *root, Type basictype)
{
    printf("进入到VarDec\n");

    struct Node *temp = root;
    int i = 0;
    while (strcmp(temp->child->shikibetsuko, "ID") != 0)
    {
        // 不断向下移动，直到找到第一个ID节点
        temp = temp->child;
        // 记录深度，这样就能够知道数组的维数
        i++;
    }
    char *name = temp->child->char_name;
    FieldList field = (FieldList)malloc(sizeof(FieldList_));
    field->name = name;
    // 根节点为ID节点，说明非数组
    if (strcmp(root->child->shikibetsuko, "ID") == 0)
    {
        field->type = basictype;
        return field;
    }

    // 支持多维数组
    struct Node *tempForArray = root;
    Type arrayType = basictype;
    while (i > 0)
    {
        Type var = (Type)malloc(sizeof(Type_));
        var->kind = ARRAY;
        var->u.array.elem = arrayType;
        // VarDec -> LB INT RB
        var->u.array.size = tempForArray->child->brother->brother->int_number;

        field->type = var;

        i--;
        tempForArray = tempForArray->child;
        arrayType = var;
    }

    return field;
}

// 类型声明的语法树节点,支持基本类型和结构体类型
Type Specifier(struct Node *root)
{
    printf("进入到Spevifier\n");
    // Specifier->TYPE
    // 注意默认只有浮点数和整数类型两种基本类型
    Type specifier = (Type)malloc(sizeof(struct Type_));
    if (strcmp(root->child->shikibetsuko, "TYPE") == 0)
    {
        specifier->kind = BASIC;
        if (strcmp(root->child->char_name, "int") == 0)
        {
            specifier->u.basic = INT_TYPE;
        }
        else
        {
            specifier->u.basic = FLOAT_TYPE;
        }

        return specifier;
    }

    // 结构体定义
    /* Specifier->StructSpecifier
       StructSpecifier → STRUCT OptTag LC DefList RC | STRUCT Tag
       OptTag → ID | e
       Tag → ID
    */
    if (strcmp(root->child->shikibetsuko, "TYPE") != 0)
    {
        specifier->kind = STRUCTURE;
        if (root->child->childNumber == 2)
        {
            // 存在tag节点
            char *name = root->child->child->brother->child->char_name;
            FieldList field = lookupSymbol(name, 0);
            if (field == NULL)
            {
                printf("Error type 17 at Line %d: Undefined structure \"%s\"\n", root->linenumber, name);
            }
            else if (field->type != NULL) // 查到了就直接返回
            {
                return field->type;
            }
            specifier->u.structure = NULL; // 没查到那么就需要置空，
                                           //  此时已经报错，但是程序依然继续执行
            return specifier;
        }
        else // 第一个子式，即不存在Tag
        {
            /*
            会产生DefList
            DefList → Def DefList | e
            Def →Specifier DecList  SEMI
            DecList → Dec | Dec COMMA DecList
            Dec → VarDec | VarDec ASSIGNOP Exp
            */
            // 先产生Def，然后产生Dec
            struct Node *DefList = root->child->child->brother->brother->brother;
            specifier->u.structure = NULL;

            // 顺着DefList继续向下
            while (DefList->emptyFlag != 1)
            {
                printf("处理DefList\n");
                struct Node *Def = DefList->child;
                Type basicType = Specifier(Def->child);
                struct Node *DecList = Def->child->brother;
                while (DecList->childNumber == 3)
                { // 递归生成
                    FieldList field = VarDec(DecList->child->child, basicType);
                    // TODO:此处检查Dec是否直接走了VarDec ASSIGNOP Exp
                    // 正在声明结构体，这样做显然直接报错
                    if (DecList->child->childNumber != 1)
                    {
                        printf("Error type 15 at Line %d: Variable %s in struct is initialized.\n", Def->linenumber, field->name); // 输出错误信息
                    }
                    FieldList temp = specifier->u.structure;
                    // 第二次进入循环
                    while (temp != NULL)
                    {
                        // 检查当前字段是否和前面的字段重名
                        if (strcmp(temp->name, field->name) == 0)
                        {
                            printf("Error type 15 at Line %d: Redefined field \"%s\".\n", Def->linenumber, field->name); // 输出错误信息
                            break;
                        }
                        temp = temp->tail;
                    }
                    // 第一次进入循环
                    if (temp == NULL)
                    {
                        // 该结构体已经声明则报错
                        if (lookupSymbol(field->name, 0) != NULL)
                        {
                            printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", Def->linenumber, field->name); // 输出错误信息
                        }
                        else
                        {
                            insert(field);
                            // 头插法更新结构体字段列表
                            field->tail = specifier->u.structure;
                            specifier->u.structure = field;
                        }
                    }
                    DecList = DecList->child->brother->brother; // 换到下一个DecList节点
                }
                // 长度不为3
                FieldList field = VarDec(DecList->child->child, basicType);
                // DecList只有3或者1长度的右子式
                if (DecList->child->childNumber != 1)
                {
                    printf("Error type 15 at Line %d: Variable \"%s\" in struct is initialized.\n", Def->linenumber, field->name); // 输出错误信息
                }
                // 下面开始解析DecList → Dec
                FieldList temp = specifier->u.structure;
                while (temp != NULL)
                {
                    if (strcmp(temp->name, field->name) == 0)
                    {
                        printf("Error type 15 at Line %d: Redefined field \"%s\".\n", Def->linenumber, field->name); // 输出错误信息
                        break;
                    }
                    temp = temp->tail;
                }
                if (temp == NULL)
                {
                    if (lookupSymbol(field->name, 0) != NULL)
                    {
                        printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", Def->linenumber, field->name); // 输出错误信息
                    }
                    else
                    {
                        insert(field);
                        // 头插法更新结构体字段列表
                        field->tail = specifier->u.structure;
                        specifier->u.structure = field;
                    }
                }
                // 至此DecList节点已经处理完毕，切换DefList
                DefList = DefList->child->brother; // 获取下一个DefList节点
            }

            // DefList处理完毕，开始处理optTag
            if (root->child->child->brother->emptyFlag != 1)
            {
                FieldList field = (FieldList)malloc(sizeof(FieldList_));
                field->type = specifier;
                char *name = root->child->child->brother->child->char_name;
                field->name = name;
                if (lookupSymbol(field->name, 0) != NULL)
                {
                    printf("Error type 16 at Line %d: Duplicated name \"%s\".\n", root->linenumber, field->name); // 输出错误信息
                }
                else
                {
                    insert(field);
                }
            }
            return specifier; // 返回结构体类型指针
        }
    }
}

// 遍历外部声明列表，处理全局变量和函数定义，需要插入符号表等
void ExtDefList(struct Node *root)
{
    printf("进入ExtDefList\n");
    // ExtDefList-> e | ExtDef ExtDefList
    // ExtDef -> Specifier ExtDecList SEMI | Specifier SEMI | Specifier FunDec CompSt
    printf("进入到函数ExtDefList()\n");
    struct Node *ExtDefList = root; // 指向头节点，也就是ExtDefList节点
    while (ExtDefList->emptyFlag != 1 && ExtDefList->childNumber != 0)
    { // 不为空产生式
        struct Node *ExtDef = ExtDefList->child;
        Type basicType = Specifier(ExtDef->child);
        // // ExtDecList -> VarDec | VarDec COMMA ExtDecList
        // // 开始处理ExtDecList节点
        if (strcmp(ExtDef->child->brother->shikibetsuko, "ExtDecList") == 0)
        {
            // temp 对应 ExtDecList节点
            struct Node *temp = ExtDef->child->brother;
            FieldList field;
            // 对应VarDec COMMA ExtDecList
            while (temp->childNumber == 3)
            {
                field = VarDec(temp->child, basicType);
                if (lookupSymbol(field->name, 0) != NULL)
                {
                    printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", ExtDef->linenumber, field->name); // 输出错误信息
                }
                else
                {
                    insert(field);
                }
                temp = temp->child->brother->brother; // 移动到下一个ExtDecList
            }
            // 不递归了，最后一次
            field = VarDec(temp->child, basicType);
            if (lookupSymbol(field->name, 0) != NULL)
            {
                printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", ExtDef->linenumber, field->name); // 输出错误信息
            }
            else
            {
                insert(field);
            }
        }
        // // 开始处理遇到Specifier FunDec CompSt
        // /*
        // FunDec → ID LP VarList RP | ID LP RP
        // VarList → ParamDec COMMA VarList | ParamDec
        // ParamDec → Specifier VarDec
        // */
        else if (strcmp(ExtDef->child->brother->shikibetsuko, "FunDec") == 0)
        {
            FieldList field = (FieldList)malloc(sizeof(FieldList_));
            // 对应到FunDec节点的第一个孩子ID
            field->name = ExtDef->child->brother->child->char_name;
            Type type = (Type)malloc(sizeof(Type_));
            type->kind = FUNCTION;
            type->function.funcType = basicType;
            type->function.paramNum = 0;
            type->function.params = NULL;
            // 处理FunDec生成的VarList节点
            if (strcmp(ExtDef->child->brother->child->brother->brother->shikibetsuko, "VarList") == 0)
            {
                struct Node *VarList = ExtDef->child->brother->child->brother->brother; // 获取参数列表节点
                // 不等于1，则为VarList → ParamDec COMMA VarList
                while (VarList->childNumber != 1)
                {                                                                           // 循环处理每个参数
                    Type tempType = Specifier(VarList->child->child);                       // 获取参数类型
                    FieldList tempField = VarDec(VarList->child->child->brother, tempType); // 获取参数字段列表
                    if (lookupSymbol(tempField->name, 0) != NULL)
                    {                                                                                                         // 如果在符号表中找到了同名的变量
                        printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", ExtDef->linenumber, tempField->name); // 输出错误信息
                    }
                    else
                    {
                        insert(tempField);
                    }
                    // 利用头插法，把参数插入到函数的参数列表中
                    type->function.paramNum++;                  // 参数数量加一
                    tempField->tail = type->function.params;    // 将参数字段列表加入到参数列表中
                    type->function.params = tempField;          // 更新参数列表
                    VarList = VarList->child->brother->brother; // 移动到下一个参数
                }
                // 退出递归，剩下一次
                Type tempType = Specifier(VarList->child->child);                       // 获取最后一个参数的类型
                FieldList tempField = VarDec(VarList->child->child->brother, tempType); // 获取最后一个参数的字段列表
                if (lookupSymbol(tempField->name, 0) != NULL)
                {                                                                                                         // 如果在符号表中找到了同名的变量
                    printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", ExtDef->linenumber, tempField->name); // 输出错误信息
                }
                else
                {
                    insert(tempField);
                } // 在符号表中插入字段列表
                // 利用头插法，把参数插入到函数的参数列表中
                type->function.paramNum++;               // 参数数量加一
                tempField->tail = type->function.params; // 将参数字段列表加入到参数列表中
                type->function.params = tempField;       // 更新参数列表
            }
            // 处理完VarList
            // 此时应该剩下一个函数
            field->type = type;
            if (lookupSymbol(field->name, 1) != NULL)
            {
                printf("Error type 4 at Line %d: Redefined function \"%s\".\n", ExtDef->linenumber, field->name); // 输出错误信息
            }
            else
            {
                insert(field);
            }
            // 最后处理一下CompSt即可
            CompSt(ExtDef->child->brother->brother, basicType);
        }
        else
        {
            // 其他情况不必处理了
        }

        // 仍在最外层的ExtDefList之中，需要递归到下一个中
        if (ExtDefList->child->brother->emptyFlag == 1)
        {
            return;
        }

        ExtDefList = ExtDefList->child->brother;
        if (ExtDefList->emptyFlag == 1)
        {
            printf("Error: ExtDefList is NULL\n");
            return;
        }
    }
    return;
}

// 处理函数的内部语句,最简单的一集
void CompSt(struct Node *root, Type funcType)
{
    // CompSt->LC DefList StmtList RC
    // StmtList-> e | Stmt StmtList
    printf("进入CompSt\n");
    struct Node *compSt = root;
    DefList(compSt->child->brother);
    struct Node *stmtList = compSt->child->brother->brother;
    while (stmtList->emptyFlag != 1)
    {
        struct Node *stmt = stmtList->child;
        Stmt(stmt, funcType);
        stmtList = stmtList->child->brother;
        
    }
}

// 处理局部变量定义
void DefList(struct Node *root)
{
    printf("进入到DefList\n");
    // DefList-> e | Def DefList
    /*
    Def →Specifier DecList  SEMI
    DecList → Dec | Dec COMMA DecList
    Dec → VarDec | VarDec ASSIGNOP Exp
    */
    struct Node *DefList = root;
    while (DefList != NULL && DefList->emptyFlag != 1)
    {
        struct Node *Def = DefList->child;
        Type basicType = Specifier(Def->child);
        struct Node *DecList = Def->child->brother;
        // 对应Dec COMMA DecList
        while (DecList->emptyFlag != 1 && DecList->childNumber == 3)
        {
            FieldList field = VarDec(DecList->child->child, basicType);
            if (lookupSymbol(field->name, 0) != NULL)
            {
                printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", DecList->linenumber, field->name); // 输出错误信息
            }
            else
            {
                insert(field);
            }
            // 切换到下一个DecList
            DecList = DecList->child->brother->brother;
        }
        // DecList → Dec，最后一次
        FieldList field = VarDec(DecList->child->child, basicType);
        if (lookupSymbol(field->name, 0) != NULL)
        {
            printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", DecList->linenumber, field->name); // 输出错误信息
        }
        else
        {
            insert(field);
        }

        // 仍在DefList的循环之中，切换到下一个或者退出
        if (DefList->child->brother->emptyFlag == 1)
        {
            return;
        }
        printf("进入到第二个DefList");
        DefList = DefList->child->brother;
    }
}

// 处理语句,例如IF语句等等
void Stmt(struct Node *root, Type funcType)
{
    printf("进入到Stmt函数\n");
    // Stmt -> Exp SEMI  | CompSt  | RETURN Exp SEMI | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE | IF LP Exp RP Stmt ELSE Stmt | WHILE LP Exp RP Stmt
    struct Node *stmt = root;
    if (strcmp(stmt->child->shikibetsuko, "RETURN") == 0)                                  // RETURN Exp SEMI
    {                                                                                           // 如果是返回语句
        Type returnType = Exp(stmt->child->brother);                                            // 获取返回值类型
        if (typeEqual(funcType, returnType) == 0)                                               // 如果返回值类型与函数返回类型不匹配
            printf("Error type 8 at Line %d: Type mismatched for return.\n", stmt->linenumber); // 输出错误信息
    }
    else if (strcmp(stmt->child->shikibetsuko, "Exp") == 0) // Exp SEMI
    {
        Exp(stmt->child);
    }
    else if (strcmp(stmt->child->shikibetsuko, "CompSt") == 0) // CompSt
    {
        CompSt(stmt->child, funcType);
    }

    else if (strcmp(stmt->child->shikibetsuko, "WHILE") == 0) // WHILE LP Exp RP stmt
    {
        Type type = Exp(stmt->child->brother->brother);
        if (!((type->kind == BASIC) && (type->u.basic == INT_TYPE)))
        {
            // WHILE语句中的判断条件必须为INT类型
            printf("Error type 5 at Line %d: Only type INT could be used for judgement.\n", stmt->linenumber);
        }
        Stmt(stmt->child->brother->brother->brother->brother, funcType);
    }
    else if (stmt->childNumber < 6)
    {                                                   // 如果是单独的IF语句
        Type type = Exp(stmt->child->brother->brother); // 获取判断条件的类型
        if (type != NULL){
            if (!((type->kind == BASIC) && (type->u.basic == INT_TYPE)))
            { // 如果判断条件不是INT类型
                printf("Error type 5 at Line %d: Only type INT could be used for judgement.\n", stmt->linenumber);
            } // 输出错误信息
        }
            
        Stmt(stmt->child->brother->brother->brother->brother, funcType);
    }
    else
    { // 带有ELSE的IF语句
        Type type = Exp(stmt->child->brother->brother);
        if (!((type->kind == BASIC) && (type->u.basic == INT_TYPE))){                                       // 如果判断条件不是INT类型
            printf("Error type 5 at Line %d: Only type INT could be used for judgement.\n", stmt->linenumber); // 输出错误信息
        }
            // 处理IF语句块
        Stmt(stmt->child->brother->brother->brother->brother, funcType);
        // 处理ELSE语句块
        Stmt(stmt->child->brother->brother->brother->brother->brother->brother, funcType);
    }
}

// 处理表达式,获取返回的类型
Type Exp(struct Node *root)
{
    printf("进入到Exp函数\n");
    // Exp->Exp ASSIGNOP Exp | Exp AND Exp | Exp OR Exp | Exp RELOP Exp
    // Exp->Exp PLUS Exp | Exp MINUS Exp | Exp STAR Exp | Exp DIV Exp | LP Exp RP
    // Exp->MINUS Exp %prec LOWER_THAN_NOT | NOT Exp | ID LP Args RP | ID LP RP
    // Exp->Exp LB Exp RB | Exp DOT ID | ID | INT | FLOAT

    if (root->emptyFlag == 1)
    {
        return NULL;
    }

    // 接下来开始分类进行处理
    //  Exp->ID
    if ((strcmp(root->child->shikibetsuko, "ID") == 0) && (root->childNumber == 1))
    {
        printf("flag2\n");                                        // 如果是单个ID节点
        FieldList field = lookupSymbol(root->child->char_name, 0); // 查找符号表中的变量
        if (field != NULL){
            return field->type; // 返回变量的类型
        }   
        else
        {
            printf("Error type 1 at Line %d: Undefined variable \"%s\".\n", root->linenumber, root->child->char_name); // 输出错误信息
            return NULL;
        }
    }

    // Exp->INT
    else if (strcmp(root->child->shikibetsuko, "INT") == 0)
    {
        printf("flag3\n");                       // 如果是整数节点
        Type type = (Type)malloc(sizeof(Type_)); // 分配内存
        type->kind = BASIC;
        type->u.basic = INT_TYPE;
        return type; // 返回整数类型
    }

    // Exp->FLOAT
    else if (strcmp(root->child->shikibetsuko, "FLOAT") == 0)
    {
        printf("flag4\n");                       // 如果是浮点数节点
        Type type = (Type)malloc(sizeof(Type_)); // 分配内存
        type->kind = BASIC;
        type->u.basic = FLOAT_TYPE;
        return type; // 返回浮点数类型
    }
    // Exp->MINUS Exp | NOT Exp | LP Exp RP
    else if ((strcmp(root->child->shikibetsuko, "LP") == 0) || (strcmp(root->child->shikibetsuko, "MINUS") == 0) || (strcmp(root->child->shikibetsuko, "NOT") == 0))
    {
        printf("flag5\n");
        return Exp(root->child->brother); // 如果是括号、负号或非运算符，则递归处理子表达式
    }

    // Exp STAR Exp
    // Exp DIV Exp
    // Exp PLUS Exp
    // Exp MINUS Exp
    else if ((strcmp(root->child->brother->shikibetsuko, "PLUS") == 0) || (strcmp(root->child->brother->shikibetsuko, "MINUS") == 0) || (strcmp(root->child->brother->shikibetsuko, "STAR") == 0) || (strcmp(root->child->brother->shikibetsuko, "DIV") == 0))
    {
        printf("flag6\n");
        Type type1 = Exp(root->child);                   // 获取左操作数的类型
        Type type2 = Exp(root->child->brother->brother); // 获取右操作数的类型
        if (typeEqual(type1, type2) == 0)
        { // 如果左右操作数类型不匹配
            if ((type1 != NULL) && (type2 != NULL))
                printf("Error type 7 at Line %d: Type mismatched for operands.\n", root->linenumber); // 输出错误信息
            return NULL;
        }
        else
            return type1; // 返回左操作数的类型
    }
    // Exp RELOP Exp
    // Exp AND Exp
    // Exp OR Exp
    else if ((strcmp(root->child->brother->shikibetsuko, "AND") == 0) || (strcmp(root->child->brother->shikibetsuko, "OR") == 0) || (strcmp(root->child->brother->shikibetsuko, "RELOP") == 0))
    {
        printf("flag1\n");
        Type type1 = Exp(root->child);                   // 获取左操作数
        Type type2 = Exp(root->child->brother->brother); // 获取右操作数
        if (typeEqual(type1, type2) == 0)
        { // 如果左右操作数类型不匹配
            if ((type1 != NULL) && (type2 != NULL))
                printf("Error type 7 at Line %d: Type mismatched for operands.\n", root->linenumber); // 输出错误信息
            return NULL;
        }
        else
        {
            Type type = (Type)malloc(sizeof(Type_)); // 分配内存
            type->kind = BASIC;
            type->u.basic = INT_TYPE;
            return type; // 返回类型
        }
    }
    // Exp = Exp，类型赋值
    else if (strcmp(root->child->brother->shikibetsuko, "ASSIGNOP") == 0)
    {
        printf("flag7\n");
        // 左边必须是变量，这时候就根据不同长度来进行分类
        // ID
        if (root->child->childNumber == 1)
        {
            if (!(strcmp(root->child->child->shikibetsuko, "ID") == 0))
            {
                printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", root->linenumber); // 输出错误信息
                return NULL;
            }
        }
        // Exp DOT ID
        else if (root->child->childNumber == 3)
        {
            if (!((strcmp(root->child->child->shikibetsuko, "Exp") == 0) && (strcmp(root->child->child->brother->shikibetsuko, "DOT") == 0) && (strcmp(root->child->child->brother->brother->shikibetsuko, "ID") == 0)))
            {
                printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", root->linenumber); // 输出错误信息
                return NULL;
            }
        }
        // Exp LB Exp RB
        else if (root->child->childNumber == 4)
        {
            if (!((strcmp(root->child->child->shikibetsuko, "Exp") == 0) && (strcmp(root->child->child->brother->shikibetsuko, "LB") == 0) && (strcmp(root->child->child->brother->brother->shikibetsuko, "Exp") == 0) && (strcmp(root->child->child->brother->brother->brother->shikibetsuko, "RB") == 0)))
            {
                printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", root->linenumber); // 输出错误信息
                return NULL;
            }
        }
        Type type1 = Exp(root->child); // 获取左操作数的类型
        Type type2 = Exp(root->child->brother->brother); // 获取右操作数的类型
        if (typeEqual(type1, type2) == 0)
        { // 如果左右操作数类型不匹配
            if ((type1 != NULL) && (type2 != NULL))
                printf("Error type 5 at Line %d: Type mismatched for assignment.\n", root->linenumber); // 输出错误信息
            return NULL;
        }
        else{
            return type1; // 返回左操作数的类型
        }
    }

    // Exp -> ID LP RP | ID LP Args RP
    else if (strcmp(root->child->shikibetsuko, "ID") == 0)
    {
        printf("flag8\n");                                         // 如果是ID节点
        FieldList field = lookupSymbol(root->child->char_name, 1); // 查找符号表中的函数
        if (field == NULL)
        {
            FieldList field2 = lookupSymbol(root->child->char_name, 0);
            if (field2 != NULL)
                printf("Error type 11 at Line %d: \"%s\" is not a function.\n", root->linenumber, root->child->char_name); // 输出错误信息
            else
                printf("Error type 2 at Line %d: Undefined function \"%s\".\n", root->linenumber, root->child->char_name); // 输出错误信息
            return NULL;
        }
        Type definedType = field->type;

        Type type = (Type)malloc(sizeof(Type_)); // 分配内存
        type->kind = FUNCTION;
        type->function.paramNum = 0;
        type->function.params = NULL;
        if (strcmp(root->child->brother->brother->shikibetsuko, "RP") != 0)
        { // 如果有参数
            struct Node *temp = root->child->brother->brother;
            while (temp->childNumber != 1)
            {                                                                // 遍历参数列表
                Type tempType = Exp(temp->child);                            // 获取参数类型
                FieldList tempField = (FieldList)malloc(sizeof(FieldList_)); // 分配内存
                tempField->name = "no";
                tempField->type = tempType;
                type->function.paramNum++;
                tempField->tail = type->function.params;
                type->function.params = tempField;

                temp = temp->child->brother->brother;
            }
            Type tempType = Exp(temp->child);                            // 获取最后一个参数的类型
            FieldList tempField = (FieldList)malloc(sizeof(FieldList_)); // 分配内存
            tempField->name = "no";                                      // 仅用于临时比较
            tempField->type = tempType;
            type->function.paramNum++;
            tempField->tail = type->function.params;
            type->function.params = tempField;
        }
        if (typeEqual(type, definedType) == 0)
        {
            printf("Error type 9 at Line %d: Params wrong in function \"%s\".\n", root->linenumber, root->child->char_name); // 输出错误信息
            return NULL;
        }
        else
            return definedType->function.funcType; // 返回函数返回值类型
    }

    // Exp DOT ID 结构体成员访问
    else if (strcmp(root->child->brother->shikibetsuko, "DOT") == 0)
    {
        printf("flag9\n");             // 如果是结构体成员访问节点
        Type type1 = Exp(root->child); // 获取结构体变量的类型
        if (type1->kind != STRUCTURE)
        {
            struct Node *temp = root->child;
            char *s;

            switch (temp->childNumber)
            {
            case 1:
            {
                if (strcmp(temp->child->shikibetsuko, "ID") == 0)
                    s = temp->child->char_name; // 若结构体变量是一个标识符，则将其文本内容赋值给变量 s
            }
            break;
            case 3:
            {
                if (strcmp(temp->child->brother->brother->shikibetsuko, "ID") == 0)
                    s = temp->child->char_name; // 若结构体变量是通过指针访问，则将其文本内容赋值给变量 s
            }
            break;
            case 4:
            {
                if (strcmp(temp->child->shikibetsuko, "Exp") == 0)
                    if (strcmp(temp->child->child->shikibetsuko, "ID") == 0)
                        s = temp->child->child->char_name; // 若结构体变量是通过结构体成员访问，则将其文本内容赋值给变量 s
            }
            break;
            default:
                s = "error"; // 若无法确定结构体变量类型，则将 s 设置为 "error"
                break;
            }
            if (lookupSymbol(s, 0) != NULL)
                printf("Error type 13 at Line %d: Illegal use of \".\".\n", root->linenumber); // 输出错误信息
            return NULL;
        }
        char *s = root->child->brother->brother->char_name; // 获取成员名
        FieldList temp = type1->u.structure;                // 遍历结构体成员列表
        while (temp != NULL)
        {
            if (strcmp(temp->name, s) == 0)
                return temp->type; // 返回成员类型

            temp = temp->tail;
        }
        printf("Error type 14 at Line %d: Non-existent field \"%s\".\n", root->linenumber, root->child->brother->brother->char_name); // 输出错误信息
        return NULL;
    }

    // Exp -> Exp LB Exp RB 数组访问
    else if (strcmp(root->child->brother->shikibetsuko, "LB") == 0)
    {
        printf("flag10\n");            // 如果是数组访问节点
        Type type1 = Exp(root->child); // 获取数组变量的类型
        if (type1->kind != ARRAY)
        {
            struct Node *temp = root->child;
            char *s;
            switch (temp->childNumber)
            {
            case 1:
            {
                if (strcmp(temp->child->shikibetsuko, "ID") == 0) // 标识符
                    s = temp->child->char_name;
            }
            break;
            case 3:
            {
                if (strcmp(temp->child->brother->brother->shikibetsuko, "ID") == 0) // 指针访问
                    s = temp->child->char_name;
            }
            break;
            case 4:
            {
                if (strcmp(temp->child->shikibetsuko, "Exp") == 0)
                    if (strcmp(temp->child->child->shikibetsuko, "ID") == 0) // 数组成员访问
                        s = temp->child->child->char_name;
            }
            break;
            default:
                s = "error";
                break;
            }
            if (lookupSymbol(s, 0) != NULL)
                printf("Error type 10 at Line %d: \"%s\" is not an array.\n", root->linenumber, s); // 输出错误信息
            return NULL;
        }
        Type temp = Exp(root->child->brother->brother); // 获取数组索引的类型
        if (temp->kind != BASIC)
        {
            printf("Error type 12 at Line %d: there is not a integer between \"[\" and \"]\".\n", root->linenumber); // 输出错误信息
            return NULL;
        }
        else if (temp->u.basic == FLOAT_TYPE)
        {
            printf("Error type 12 at Line %d: there is not a integer between \"[\" and \"]\".\n", root->linenumber); // 输出错误信息
            return NULL;
        }
        // 无错误，返回数组元素类型
        return type1->u.array.elem;
    }

    else
    {
        printf("flag11\n");
        printf("in\n"); // 输出错误信息
        return NULL;    // 返回空指针
    }
}
