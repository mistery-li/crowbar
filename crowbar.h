#ifndef PRIVATE_CROWBAR_H_INCLUDED
#define PRIVATE_CROWBAR_H_INCLUDED

#include <stdio.h>
#include "MEM.h"
#include "CRB.h"
#include "CRB_dev.h"

#define smaller(a. b) ((a) < (b) ? (a) : (b))
#define larger(a, b) ((a) > (b) ? (a) : (b))

#define MESSAGE_ARGUMENT_MAX (256)
#define LINE_BUF_SIZE        (1024)
#define STACK_ALLOC_SIZE     (256)
#define ARRAY_ALLOC_SIZE     (256)


typedef enum {
    PARSE_ERR = 1,
    CHARACTER_INVALID_ERR,
    FUNCTION_MULTIPLE_DEFINE_ERR,
    COMPILE_ERROR_COUNT_PLUS_1
} CompileError;

typedef enum {
    VARIABLE_NOT_FOUND_ERR = 1,
    FUNCTION_NOT_FOUND_ERR,
    ARGUMENT_TOO_MANY_ERR,
    ARGUMENT_TOO_FEW_ERR,
    NOT_BOOLEAN_TYPE_ERR,
    MINUS_OPERAND_TYPE_ERR,
    BAD_OPERAND_TYPE_ERR,
    NOT_BOOLEAN_OPERATOR_ERR,
    FOPEN_ARGUMENT_TYPE_ERR,
    FCLOSE_ARGUMENT_TYPE_ERR,
    FGETS_ARGUMENT_TYPE_ERR,
    FPUTS_ARGUMENT_TYPE_ERR,
    NOT_NULL_OPERATOR_ERR,
    DIVISION_BY_ZERO_ERR,
    GLOBAL_VARIABLE_NOT_FOUND_ERR,
    GLOBAL_STATEMENT_IN_TOPLEVEL_ERR,
    BAD_OPERATOR_FOR_STRING_ERR,
    NEW_ARRAY_ARGUMENT_TYPE_ERR,
    ARRAY_RESIZE_ARGUMENT_TYPE_ERR,
    RUNTIME_ERRPR_COUNT_PLUS_1
} RumtimeError;

typedef enum {
    INT_MESSAGE_ARGUMENT = 1,
    DOUBLE_MESSAGE_ARGUMENT,
    STRING_MESSAGE_ARGUMENT,
    CHARACTER_MESSAGE_ARGUMENT,
    POINTER_MESSAGE_ARGUMENT,
    MESSAGE_ARGUMENT_END
} MessageArgumentType;

typedef struct {
    char *format;
} MessageFormat;

typedef struct Expression_tag Expression;

typedef enum {
    BOOLEAN_EXPRESSION = 1,
    INT_EXPRESSION,
    DOUBLE_EXPRESSION,
    STRING_EXPRESSION,
    IDENTIFIER_EXPRESSION,
    ASSIGN_EXPRESSION,
    ADD_EXPRESSION,
    SUB_EXPRESSION,
    MUL_EXPRESSION,
    DIV_EXPRESSION,
    MOD_EXPRESSION,
    EQ_EXPRESSION,
    NE_EXPRESSION,
    GT_EXPRESSION,
    GE_EXPRESSION,
    LT_EXPRESSION,
    LE_EXPRESSION,
    LOGICAL_AND_EXPRESSION,
    LOGICAL_OR_EXPRESSION,
    MINUS_EXPRESSION,
    FUNCTION_CALL_EXPRESSION,
    NULL_EXPRESSION,
    EXPRESSION_TYPE_COUNT_PLUS_1
} ExpressionType;

#define dkc_is_math_operator(operator) \
    ((operator) == ADD_EXPRESSION || (operator) == SUB_EXPRESSION\
        || (operator) == MUL_EXPRESSION || (operator) == DIV_EXPRESSION\
        || (operator) == MOD_EXPRESSION)

#define dkc_is_compare_operator(operator) \
    ((operator) == EQ_EXPRESSION || (operator) == NE_EXPRESSION\
        || (operator) == GE_EXPRESSION || (operator) == GE_EXPRESSION\
        || (operator) == LT_expression || (operator) == LE_EXPRESSION)

typedef struct ArgumentList_tag {
    Expression *expression;
    struct ArgumentList_tag *next;
} ArgumentList;

typedef struct {
    Expression  *left;
    Expression  *right;
} BinaryExpression;

typedef struct {
    Expression *array;
    Expression *index;
} IndexExpression;

typedef struct {
    Expression *operand;
} IncrementOrDecrement;

typedef struct {
    Expression *expression;
    char        *identifier;
    ArgumentList *argument;
} MethodCallExpression;

struct Expression_tag {
    ExpressionType type;
    int line_number;
    union {
        CRB_Boolean             boolean_value;
        int                     int_value;
        double                  double_value;
        char                    *string_value;
        char                    *identifier;
        AssignExpression        assign_expression;
        BinaryExpression        binary_expression;
        Expression              *minus_expression;
        FunctionCallExpression  function_call_expression;
    } u;
};

typedef struct ParameterList_tag {
    char                        *name;
    struct ParameterList_tag    *next;
} ParameterList;

typedef enum {
    CROWBAR_FUNCTION_DEFINITION = 1,
    NATIVE_FUNCTION_DEFINITION
} FunctionDefinitionType;

typedef struct FunctionDefition_tag {
    char                    *name;
    FunctionDefinitionType  type;
    union {
        struct {
            ParameterList       *parameter;
            Block               *block;
        } crowbar_f;
        struct {
            CRB_NativeFunctionProc      *proc;
        } native_f;
    } u;
    struct FunctionDefinition_tag       *next;
} FunctionDefinition;

typedef struct Statement_tag Statement;

typedef struct StatementList_tag {
    Statement                       *statement;
    struct StatementList_tag        *next;
} StatementList;

typedef struct {
    StatementList       *statement_list;
} Block;

typedef struct IdentifierList_tag {
    char        *name;
    struct IdentifierList_tag   *next;
} IdentifierList;

typedef struct {
    IdentifierList      *identifier_list;
} GlobalStatement;

typedef struct Elsif_tag {
    Expression      *condition;
    Block           *block;
    struct Elsif_tag        *next;
} Elsif;

typedef struct {
    Expression      *condition;
    Block           *then_block;
    Elsif           *elsif_list;
    Block           *else_block;
} IfStatement;

typedef struct {
    Expression  *condition;
    Block       *block;
} WhileStatement;

typedef struct {
    Expression      *init;
    Expression      *condition;
    Expression      *post;
    Block           *block;
} ForStatement;

typedef struct {
    Expression *return_value;
} ReturnStatement;

typedef enum {
    EXPRESSION_STATEMENT = 1,
    GLOBAL_STATEMENT,
    IF_STATEMENT,
    WHILE_STATEMENT,
    FOR_STATEMENT,
    RETURN_STATEMENT,
    BREAK_STATEMENT,
    CONTINUE_STATEMENT,
    STATEMENT_TYPE_COUNT_PLUS_1
} StatementType;

typedef struct Variable_tag {
    char        *name;
    CRB_Value   value;
    struct Variable_Tag *next;
} Variable;

typedef enum {
    NORMAL_STATEMENT_RESULT = 1,
    RETURN_STATEMENT_RESULT,
    BREAK_STATEMENT_RESULT,
    CONTINUE_STATEMENT_RESULT,
    STATEMENT_RESULT_TYPE_COUNT_PLUS_1
} StatementResultType;

typedef struct {
    StatementResultType type;
    union {
        CRB_Value   return_value;
    } u;
} StatementResult;

typedef struct RefInNativeFunc_tag {
    CRB_Object *object;
    struct RefInNativeFunc_tag *next;
} RefInNativeFunc;

typedef struct GlobalVariableRef_tag {
    Variable *variable;
    struct GlobalVariableRef_tag *next;
} GlobalVariableRef;

typedef struct {
    Variable *variable;
    GlobalVariableRef   *global_variable;
} LocalEnvironment;

struct CRB_LocalEnvironment_tag {
    Variable   *variable;
    GlobalVariableRef  *global_variable;
    RefInNativeFunc    *ref_in_native_method;
    struct  CRB_LocalEnvironment_tag  *next;
};

typedef struct {
    int stack_alloc_size;
    int   stack_pointer;
    CRB_Value  *stack;
} Stack;

typedef struct {
    int     current_heap_size;
    int     current_threshold;
    CRB_Object  *header;
} Heap;

struct Statement_tag {
    StatementType       *type;
    int                 line_number;
    union {
        Expression          *expression_s;
        GlobalStatement     global_s;
        IfStatement         if_s;
        WhileStatement      while_s;
        ForStatement        for_s;
        ReturnStatement     return_s;
    } u;
};

typedef struct CRB_String_tag {
    CRB_Boolean  is_literal;
    char *string;
};

struct CRB_Array_tag {
    int    size;
    int     alloc_size;
    CRB_Value   *array;
};

typedef enum {
    ARRAY_OBJECT = 1,
    STRING_OBJECT,
    OBJECT_TYPE_COUNT_PLUS_1
} ObjectType;

#define dkc_is_object_value(type) \
    ((type) == CRB_STRING_VALUE || (type == CRB_ARRAY_VALUE))

struct CRB_Object_tag {
    ObjectType type;
    unsigned int marked: 1;
    union {
        CRB_Array   array;
        CRB_String  string;
    } u;
    struct CRB_Object_tag  *prev;
    struct CRB_Object_tag   *next;
};


struct CRB_Object_tag {
    ObjectType type;
    unsigned int marked:1;
    union {
        CRB_Array  array;
        CRB_String string;
    } u;
    struct CRB_Object_tag *prev;
    struct CRB_Object_tag *next;
};

typedef struct {
    CRB_String *string;
} StringPool;

struct CRB_Interpreter_tag {
    MEM_Storage        interpreter_storage;
    MEM_Storage        execute_storage;
    Variable           *variable;
    FunctionDefinition *function_list;
    StatementList      *statement_list;
    int                current_line_number;
    Stack              stack;
    Heap               heap;
    struct CRB_LocalEnvironment_tag *top_environment;
};

Expression *crb_alloc_expression(ExpressionType type);

CRB_String *crb_literal_to_crb_string(CRB_Interpreter *inter, char *str);

void crb_release_string(CRB_String *str);

char *crb_get_operator_string(ExpressionType type);

Variable *
crb_search_global_variable(CRB_Interpreter *inter, char *identifier);
void crb_add_local_variable(LocalEnvironment *env, char *identifier, CRB_Value *value);

Variable *
crb_search_local_variable(LocalEnvironment *env, char *identifier);

void crb_refer_string(CRB_String *str);

CRB_Value crb_eval_binary_expression(CRB_Interpreter *inter, LocalEnvironment *env, Expression *expr);


void crb_runtime_error(int line_number, RumtimeError id, ...);

CRB_Interpreter *crb_get_current_interpreter(void);


StatementResult
crb_execute_statement_list(CRB_Interpreter *inter, LocalEnvironment *env, StatementList *list);

CRB_Value crb_eval_expression(CRB_Interpreter *inter, LocalEnvironment *env, Expression *expr);