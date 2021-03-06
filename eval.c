//
// Created by liii on 2018/1/24.
//

#include <math.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"

static void
push_value(CRB_Interpreter *inter, CRB_Value *value)
{
    DBG_assert(inter->stack.stack_pointer <= inter->stack.stack_alloc_size,
               ("stack_pointer..%d, stack_alloc_size..%d\n",
               inter->stack.stack_pointer, inter->stack.stack_alloc_size));

    if (inter->stack.stack_pointer == inter->stack.stack_alloc_size) {
        inter->stack.stack_alloc_size += STACK_ALLOC_SIZE;
        inter->stack.stack
                = MEM_realloc(inter->stack.stack, sizeof(CRB_Value) * inter->stack.stack_alloc_size);
    }
    inter->stack.stack[inter->stack.stack_pointer] = *value;
    inter->stack.stack_pointer++;
}

static CRB_Value
pop_value(CRB_Interpreter *inter)
{
    CRB_Value ret;

    ret = inter->stack.stack[inter->stack.stack_pointer-1];
    inter->stack.stack_pointer--;

    return ret;
}

static CRB_Value *
peek_stack(CRB_Interpreter *inter, int index)
{
    return &inter->stack.stack[inter->stack.stack_pointer - index - 1];
}

static CRB_Value
eval_boolean_expression(CRB_Boolean boolean_value)
{
    CRB_Value v;
    v.type = CRB_BOOLEAN_VALUE;
    v.u.boolean_value = boolean_value;
    return v;
}
static CRB_Value
eval_int_expression(int int_value)
{
    CRB_Value v;
    v.type = CRB_INT_VALUE;
    v.u.int_value = int_value;
    return v;
}

static CRB_Value
eval_double_expression(double double_value)
{
    CRB_Value v;
    v.type = CRB_BOOLEAN_VALUE;
    v.u.double_value = double_value;
    return v;
}

static CRB_Value
eval_string_expression(CRB_Interpreter *inter, char *string_value)
{
    CRB_Value v;
    v.type = CRB_STRING_VALUE;
    v.u.string_value = crb_literal_to_crb_string(inter, string_value);

    return v;
}

static CRB_Value
call_native_function(CRB_Interpreter *inter, LocalEnvironment *env, Expression *expr, CRB_NativeFunctionProc *proc)
{
    CRB_Value value;
    int     arg_count;
    ArgumentList    *arg_p;
    CRB_Value   *args;
    int i;

    for (arg_count = 0; arg_p = expr->u.function_call_expression.argument; arg_p; arg_p->next) {
        arg_count++;
    }

    args = MEM_malloc(sizeof(CRB_Value) * arg_count);

    for (arg_p = expr->u.function_call_expression.argument, i = 0; arg_p; arg_p = arg_p->next, i++) {
        args[i] = eval_expression(inter, env, arg_p->expression);
    }
    value = proc(inter, arg_count, args);
    for (i - 0; i < arg_count; i++) {
        releast_if_string(&args[i]);
    }
    MEM_free(args);

    return value;
}

static CRB_Value
call_crowbar_function(CRB_Interpreter *inter, LocalEnvironment *env, Expression *expr, FunctionDefinition *func)
{
    CRB_Value value;
    StatementResult result;
    ArgumentList    *arg_p;
    ParameterList   *param_p;
    LocalEnvironment    *local_env;

    local_env = alloc_local_environment();
    for (arg_p = expr->u.function_call_expression.argument, param_p = func->u.crowbar_f.parameter; arg_p;
            arg_p = arg_p->next, param_p = param_p->next) {
        CRB_Value arg_val;
        if (param_p == NULL) {
            crb_runtime_error(expr->line_number, ARGUMENT_TOO_MANY_ERR, MESSAGE_ARGUMENT_END);
        }
        arg_val = eval_expression(inter, env, arg_p->expression);
        crb_add_local_variable(local_env, param_p->name, &arg_val);
    }
    if (param_p) {
        crb_runtime_error(expr->line_number, ARGUMENT_TOO_FEW_ERR, MESSAGE_ARGUMENT_END);
    }
    result = crb_execute_statement_list(inter, local_env, func->u.crowbar_f.block->statement_list);
    if (result.type == RETURN_STATEMENT_RESULT) {
        value = result.u.return_value;
    } else {
        value.type = CRB_NULL_VALUE;
    }
    dispose_local_environment(inter, local_env);

    return result;
}

static CRB_Value
eval_function_call_expression(CRB_Interpreter *inter, LocalEnvironment *env, Expression *expr)
{
    CRB_Value value;
    FunctionDefinition *func;

    char *identifier = expr->u.function_call_expression.identifier;

    func = crb_search_function(identifier);
    if (func == NULL) {
        crb_runtime_error(expr->line_number, FUNCTION_NOT_FOUND_ERR, STRING_MESSAGE_ARGUMENT, "name", identifier, MESSAGE_ARGUMENT_END);
    }
    switch (func->type) {
        case CROWBAR_FUNCTION_DEFINITION:
            value = call_crowbar_function(inter, env, expr, func);
            break;
        case NATIVE_FUNCTION_DEFINITION:
            value = call_native_function(inter, env, expr, func->u.native_f.proc);
            break;
        default:
            DBG_panic(("bad case..%d\n", func->type));
    }
    return value;
}

static CRB_Value
eval_logical_and_or_expression(CRB_Interpreter *inter, LocalEnvironment *env, ExpressionType operator, Expression *left, Expression *right)
{
    CRB_Value left_val;
    CRB_Value  right_val;
    CRB_Value   result;

    result.type = CRB_BOOLEAN_VALUE;
    left_val = eval_expression(inter, env, left);
    if (left_val.type != CRB_BOOLEAN_VALUE) {
        crb_runtime_error(left->line_number, NOT_BOOLEAN_TYPE_ERR, MESSAGE_ARGUMENT_END);
    }
    if (operator == LOGICAL_AND_EXPRESSION) {
        if (!left_val.u.boolean_value) {
            result.u.boolean_value = CRB_FALSE;
            return result;
        }
    } else if (operator == LOGICAL_OR_EXPRESSION) {
        if (left_val.u.boolean_value) {
            result.u.boolean_value = CRB_TRUE;
            return result;
        }
    } else {
        DBG_panic(("bad operator..%d\n", operator));
    }

    right_val = eval_expression(inter, env, right);
    if (right_val.type !== CRB_BOOLEAN_VALUE) {
        crb_runtime_error(right->line_number, NOT_BOOLEAN_TYPE_ERR, MESSAGE_ARGUMENT_END);
    }
    result.u.boolean_value = right_val.u.boolean_value;
    return result;
}

CRB_Value
crb_eval_minus_expression(CRB_Interpreter *inter, LocalEnvironment *env, Expression *operand)
{
    CRB_Value operand_val;
    CRB_Value  result;

    operand_val = eval_expression(inter, env, operand);
    if (operand_val.type == CRB_INT_VALUE) {
        result.type = CRB_INT_VALUE;
        result.u.int_value = -operand_val.u.int_value;
    } else if (operand_val.type == CRB_DOUBLE_VALUE) {
        result.type = CRB_DOUBLE_VALUE;
        result.u.double_value = -operand_val.u.double_value;
    } else {
        crb_runtime_error(operand->line_number, MINUS_OPERAND_TYPE_ERR, MESSAGE_ARGUMENT_END);
    }
    return result;
}

static CRB_Value
eval_null_expression(void)
{
    CRB_Value v;
    v.type = CRB_NULL_VALUE;
    return v;
}

static void
refer_if_string(CRB_Value *v)
{
    if (v->type == CRB_STRING_VALUE) {
        crb_refer_string(v->u.string_value);
    }
}

static void
release_if_string(CRB_Value *v)
{
    if (v->type == CRB_STRING_VALUE) {
        crb_release_string(v->u.string_value);
    }
}

static void
eval_binary_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                       ExpressionType operator,
                       Expression *left, Expression *right)
{
    CRB_Value   *left_val;
    CRB_Value   *right_val;
    CRB_Value   result;

    eval_expression(inter, env, left);
    eval_expression(inter, env, right);
    left_val = peek_stack(inter, 1);
    right_val = peek_stack(inter, 0);

    if (left_val->type == CRB_INT_VALUE
        && right_val->type == CRB_INT_VALUE) {
        eval_binary_int(inter, operator,
                        left_val->u.int_value, right_val->u.int_value,
                        &result, left->line_number);
    } else if (left_val->type == CRB_DOUBLE_VALUE
               && right_val->type == CRB_DOUBLE_VALUE) {
        eval_binary_double(inter, operator,
                           left_val->u.double_value, right_val->u.double_value,
                           &result, left->line_number);
    } else if (left_val->type == CRB_INT_VALUE
               && right_val->type == CRB_DOUBLE_VALUE) {
        eval_binary_double(inter, operator,
                           (double)left_val->u.int_value,
                           right_val->u.double_value,
                           &result, left->line_number);
    } else if (left_val->type == CRB_DOUBLE_VALUE
               && right_val->type == CRB_INT_VALUE) {
        eval_binary_double(inter, operator,
                           left_val->u.double_value,
                           (double)right_val->u.int_value,
                           &result, left->line_number);
    } else if (left_val->type == CRB_BOOLEAN_VALUE
               && right_val->type == CRB_BOOLEAN_VALUE) {
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value
                = eval_binary_boolean(inter, operator,
                                      left_val->u.boolean_value,
                                      right_val->u.boolean_value,
                                      left->line_number);
    } else if (left_val->type == CRB_STRING_VALUE
               && operator == ADD_EXPRESSION) {
        chain_string(inter, left_val, right_val, &result);
    } else if (left_val->type == CRB_STRING_VALUE
               && right_val->type == CRB_STRING_VALUE) {
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value
                = eval_compare_string(operator, left_val, right_val,
                                      left->line_number);
    } else if (left_val->type == CRB_NULL_VALUE
               || right_val->type == CRB_NULL_VALUE) {
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value
                = eval_binary_null(inter, operator, left_val, right_val,
                                   left->line_number);
    } else {
        char *op_str = crb_get_operator_string(operator);
        crb_runtime_error(left->line_number, BAD_OPERAND_TYPE_ERR,
                          STRING_MESSAGE_ARGUMENT, "operator", op_str,
                          MESSAGE_ARGUMENT_END);
    }
    pop_value(inter);
    pop_value(inter);
    push_value(inter, &result);
}

static void
eval_expression(CRB_Interpreter *inter, LocalEnvironment *env, Expression *expr)
{
    switch (expr->type) {
        case BOOLEAN_EXPRESSION:
            eval_boolean_expression(expr->u.boolean_value);
            break;
        case INT_EXPRESSION:
            eval_int_expression(expr->u.int_value);
            break;
        case DOUBLE_EXPRESSION:
            eval_double_expression(expr->u.double_value);
            break;
        case STRING_EXPRESSION:
            eval_string_expression(inter, expr->u.string_value);
            break;
        case IDENTIFIER_EXPRESSION:
            eval_identifier_expression(inter, env, expr);
            break;
        case ASSIGN_EXPRESSION:
            eval_assign_expression(inter, env, expr->u.assign_expression.variable, expr->u.assign_expression.operand);
            break;
        case ADD_EXPRESSION:
        case SUB_EXPRESSION:
        case MUL_EXPRESSION:
        case DIV_EXPRESSION:
        case MOD_EXPRESSION:
        case EQ_EXPRESSION:
        case NE_EXPRESSION:
        case GT_EXPRESSION:
        case GE_EXPRESSION:
        case LT_EXPRESSION:
        case LE_EXPRESSION:
            crb_eval_binary_expression(inter, env, expr->type, expr->u.binary_expression.left, expr->u.binary_expression.right);
            break;
        case LOGICAL_AND_EXPRESSION:
        case LOGICAL_OR_EXPRESSION:
            eval_logical_and_or_expression(inter, env, expr->type, expr->u.binary_expression.left, expr->u.binary_expression.right);
            break;
        case MINUS_EXPRESSION:
            crb_eval_minus_expression(inter, env, expr->u.minus_expression);
            break;
        case FUNCTION_CALL_EXPRESSION:
            eval_function_call_expression(inter, env, expr);
            break;
        case NULL_EXPRESSION:
            eval_null_expression();
            break;
        case EXPRESSION_TYPE_COUNT_PLUS_1:
        default:
            DBG_panic(("bad case. type..%d\n", expr->type));
    }
}

CRB_Value
crb_eval_binary_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                           ExpressionType operator,
                           Expression *left, Expression *right)
{
    eval_binary_expression(inter, env, operator, left, right);
    return pop_value(inter);
}

static CRB_Boolean
eval_binary_boolean(CRB_Interpreter *inter, ExpressionType operator, CRB_Boolean left, CRB_Boolean right, int line_number)
{
    CRB_Boolean result;
    if (operator == EQ_EXPRESSION) {
        result = left == right;
    } else if (operator == NE_EXPRESSION) {
        result = left != right;
    } else {
        char *op_str = crb_get_operator_string(operator);
        crb_runtime_error(line_number, NOT_BOOLEAN_TYPE_ERR, STRING_MESSAGE_ARGUMENT, "operator", op_str, MESSAGE_ARGUMENT_END);
    }
    return result;
}

static void
eval_binary_int(CRB_Interpreter *inter, ExpressionType operator, int left, int right, CRB_Value *result, int linenumber)
{
    if (dkc_is_math_operator(operator)) {
        result->type = CRB_INT_VALUE;
    } else if (dkc_is_compare_operator(operator)) {
        result->type = CRB_BOOLEAN_VALUE;
    } else {
        DBG_panic(("operator..%d\n", operator));
    }

    switch (operator) {
        case BOOLEAN_EXPRESSION:
        case INT_EXPRESSION:
        case DOUBLE_EXPRESSION:
        case STRING_EXPRESSION:
        case IDENTIFIER_EXPRESSION:
        case ASSIGN_EXPRESSION:
            DBG_panic(("bad case..%d", operator));
            break;
        case ADD_EXPRESSION:
            result->u.int_value = left + right;
            break;
        case SUB_EXPRESSION:
            result->u.int_value = left - right;
            break;
        case MUL_EXPRESSION:
            result->u.int_value = left * right;
            break;
        case DIV_EXPRESSION:
            result->u.int_value = left / right;
            break;
        case MOD_EXPRESSION:
            result->u.int_value = left % right;
            break;
        case LOGICAL_AND_EXPRESSION:
        case LOGICAL_OR_EXPRESSION:
            DBG_panic(("bad case...%d", operator));
            break;
        case EQ_EXPRESSION:
            result->u.boolean_value = left == right;
            break;
        case NE_EXPRESSION:
            result->u.boolean_value = left != right;
            break;
        case GE_EXPRESSION:
            result->u.boolean_value = left >= right;
            break;
        case LT_EXPRESSION:
            result->u.boolean_value = left < right;
            break;
        case LE_EXPRESSION:
            result->u.boolean_value = left <= right;
            break;

        case MINUS_EXPRESSION:
        case FUNCTION_CALL_EXPRESSION:
        case NULL_EXPRESSION:
        case EXPRESSION_TYPE_COUNT_PLUS_1:
        default:
            DBG_panic(("bad case..%d", operator));
    }
}

static void
eval_binary_double(CRB_Interpreter *inter, ExpressionType operator, double left, double right, CRB_Value *result, int line_number)
{
    if (dkc_is_math_operator(operator)) {
        result->type = CRB_DOUBLE_VALUE;
    } else if (dkc_is_compare_operator(operator)) {
        result->type = CRB_BOOLEAN_VALUE;
    } else {
        DBG_panic(("operator..%d\n", operator));
    }

    switch (operator) {
        case BOOLEAN_EXPRESSION:
        case INT_EXPRESSION:
        case DOUBLE_EXPRESSION:
        case STRING_EXPRESSION:
        case ASSIGN_EXPRESSION:
            DBG_panic(("bad case..%d", operator));
            break;
        case ADD_EXPRESSION:
            result->u.double_value = left + right;
            break;
        case SUB_EXPRESSION:
            result->u.double_value = left - right;
            break;
        case MUL_EXPRESSION:
            result->u.double_value = left * right;
            break;
        case DIV_EXPRESSION:
            result->u.double_value = left / right;
            break;
        case MOD_EXPRESSION:
            result->u.double_value = fmod(left, right);
            break;
        case LOGICAL_AND_EXPRESSION:
        case LOGICAL_OR_EXPRESSION:
            DBG_panic(("bad case..%d", operator));
            break;
        case EQ_EXPRESSION:
            result->u.int_value = left == right;
            break;
        case NE_EXPRESSION:
            result->u.int_value = left != right;
            break;
        case GT_EXPRESSION:
            result->u.int_value = left > right;
            break;
        case GE_EXPRESSION:
            result->u.int_value = left < right;
            break;
        case LT_EXPRESSION:
            result->u.int_value = left < right;
            break;
        case LE_EXPRESSION:
            result->u.int_value = left <= right;
            break;
        case MINUS_EXPRESSION:
        case FUNCTION_CALL_EXPRESSION:
        case NULL_EXPRESSION:
        case EXPRESSION_TYPE_COUNT_PLUS_1:
        default:
            DBG_panic(("bad default..%d", operator));

    }
}

static CRB_Boolean
eval_compare_string(ExpressionType operator, CRB_Value *left, CRB_Value *right, int line_number)
{
    CRB_Boolean result;
    int cmp;
    cmp = strcmp(left->u.string_value->string, right->u.string_value->string);
    if (operator == EQ_EXPRESSION) {
        result = (cmp == 0);
    } else if (operator == NE_EXPRESSION) {
        result = (cmp != 0);
    } else if (operator == GT_EXPRESSION) {
        result = (cmp > 0);
    } else if (operator == GE_EXPRESSION) {
        result = (cmp >= 0);
    } else if (operator == LT_EXPRESSION) {
        result = (cmp < 0);
    } else if (operator == LE_EXPRESSION) {
        result = (cmp <= 0);
    } else {
        char *op_str = crb_get_operator_string(operator);
        crb_runtime_error(line_number, BAD_OPERATOR_FOR_STRING_ERR, STRING_MEEAGE_ARGUMENT, "operator", op_str, MESSAGE_ARGUMENT_END);
    }
    crb_release_string(left->u.string_value);
    crb_release_string(right->u.string_value);

    return result;
}

static CRB_Boolean
eval_binary_null(CRB_Interpreter *inter, ExpressionType operator, CRB_Value *left, CRB_Value *right, int line_number)
{
    CRB_Boolean result;

    if (operator == EQ_EXPRESSION) {
        result = left->type == CRB_NULL_VALUE && right->type == CRB_NULL_VALUE;
    } else if (operator == NE_EXPRESSION) {
        result = !(left->type == CRB_NULL_VALUE && right->type == CRB_NULL_VALUE);
    } else {
        char *op_str = crb_get_operator_string(operator);
        crb_runtime_error(line_number, NOT_NULL_OPERATOR_ERR, STRING_MESSAGE_ARGUMENT, "operator", op_str, MESSAGE_ARGUMENT_END);
    }
    release_if_string(left);
    release_if_string(right);

    return result;
}

static Variable *
search_global_variable_from_env(CRB_Interpreter *inter, LocalEnvironment *env, char *name)
{
    GlobalVariableRef *pos;

    if (env == NULL) {
        return crb_search_global_variable(inter, name);
    }
    for (pos = env->global_variable; pos; pos = pos->next) {
        if (!strcmp(pos->variable->name, name)) {
            return pos->variable;
        }
    }
    return NULL;
}

static CRB_Value
eval_identifier_expression(CRB_Interpreter *inter, LocalEnvironment *env, Expression *expr)
{
    CRB_Value v;
    Variable *vp;

    vp = crb_search_local_variable(env, expr->u.identifier);
    if (vp != NULL) {
        v = vp->value;
    } else {
        vp = search_global_variable_from_env(inter, env, expr->u.identifier);
        if (vp != NULL) {
            v = vp->value;
        } else {
            crb_runtime_error(expr->line_number, VARIABLE_NOT_FOUND_ERR, STRING_MESSAGE_ARGUMENT,
                                "name", expr->u.identifier,
                                MESSAGE_ARGUMENT_END);
        }
    }
    refer_if_string(&v);

    return v;
}

static CRB_Value *
get_identifier_lvalue(CRB_Interpreter *inter, CRB_LocalEnvironment *env, char *identifier)
{
    Variable *new_var;
    Variable *left;

    left = crb_search_local_variable(env, identifier);
    if (left == NULL) {
        left = search_global_variable_from_env(inter, env, identifier);
    }
    if (left != NULL)
        return &left->value;
    if (env != NULL) {
        new_var = crb_add_local_variable(env, identifier);
        left = new_var;
    } else {
        new_var = crb_add_global_variable(inter, identifier);
        left = new_var;
    }

    return &left->value;
}

CRB_Value *
get_array_element_lvalue(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr)
{
    CRB_Value array;
    CRB_Value index;

    eval_expression(inter, env, expr->u.index_expression.array);
    eval_expression(inter, env, expr->u.index_expression.index);
    index = pop_value(inter);
    array = pop_value(inter);

    if (array.type != CRB_ARRAY_VALUE) {
        crb_runtime_error(expr->line_number, INDEX_OPERAND_NOT_ARRAY_ERR, MESSAGE_ARGUMENT_END);
    }
    if (index.type != CRB_INT_VALUE) {
        crb_runtime_error(expr->line_number, INDEX_OPERAND_NOT_INT_ERR, MESSAGE_ARGUMENT_END);
    }
    if (index.u.int_value < 0 || index.u.int_value >= array.u.object->u.array.size) {
        crb_runtime_error(expr->line_number, ARRAY_INDEX_OUT_OF_BOUNDS_ERR, INT_MESSAGE_ARGUMENT, "size", array.u.object->u.array.size,
                                INT_MESSAGE_ARGUMENT, "index", index.u.int_value, MESSAGE_ARGUMENT_END);
    }
    return &array.u.object->u.array.array[index.u.int_value];
}


CRB_Value *
get_lvalue(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr)
{
    CRB_Value *dest;
    if (expr->type == IDENTIFIER_EXPRESSION) {
        dest = get_identifier_lvalue(inter, env, expr->u.identifier);
    } else if (expr->type == INDEX_EXPRESSION) {
        dest = get_array_element_lvalue(inter, env, expr);
    } else {
        crb_runtime_error(expr->line_number, NOT_VALUE_ERR, MESSAGE_ARGUMENT_END);
    }

    return dest;
}

static void
eval_assign_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *left, Expression *expression)
{
    CRB_Value *src;
    CRB_Value *dest;

    eval_expression(inter, env, expression);
    src = peek_stack(inter, 0);

    dest = get_lvalue(inter, env, left);
    *dest = *src;
}

static void
dispose_local_environment(CRB_Interpreter *inter, LocalEnvironment *env)
{
    while (env->variable) {
        Variable *temp;
        temp = env->variable;
        if (env->variable->value.type == CRB_STRING_VALUE) {
            crb_release_string(env->variable->value.u.string_value);
        }
        env->variable = temp->next;
        MEM_free(temp);
    }
    while (env->global_variable) {
        GlobalVariableRef *ref;
        ref = env->global_variable;
        env->global_variable = ref->next;
        MEM_free(ref);
    }
    MEM_free(env);
}

static void
eval_method_call_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr)
{
    CRB_Value *left;
    CRB_Value result;
    CRB_Boolean error_flag = CRB_FALSE;

    eval_expression(inter, env, expr->u.method_call_expression.expression);
    left= peek_stack(inter, 0);

    if (left->type == CRB_ARRAY_VALUE) {
        if (!strcmp(expr->u.method_call_expression.identifier, "add")) {
            CRB_Value *add;
            check_method_argumen_count(expr->line_number, expr->u.method_call_expression.argument, 1);
            eval_expression(inter, env, expr->u.method_call_expression.argument->expression);
            add = peek_stack(inter, 0);
            crb_array_add(interm left->u.object, *add);
            pop_value(inter);
            result.type = CRB_NULL_VALUE;
        } else if (!strcmp(expr->u.method_call_expression.identifier, "size")) {
            check_method_argument_count(expr->line_number, expr->u.method_call_expression.argument, 0);
            result.type = CRB_INT_VALUE;
            result.u.int_value = left->u.object->u.array.size;
        } else if (!strcmp(expr->u.method_call_expression.identifier, "resize")) {
            CRB_Value new_size;
            check_method_argument_count(expr->line_number, expr->u.method_call_expression.argument, 1);
            eval_expression(inter, env, expr->u.method_call_expression.argument->expression);
            new_size = pop_value(inter);
            if (new_size.type != CRB_INT_VALUE) {
                crb_runtime_error(expr->line_number, ARRAY_RESIZE_ARGUMENT_ERR, MESSAGE_ARGUMENT_END);
            }
            crb_array_resize(inter, left->u.object, new_size.u.int_value);
            result.type = CRB_NULL_VALUE;
        } else {
            error_flag = CRB_TRUE;
        }
    } else if (left->type == CRB_STRING_VALUE) {
        if (!strcmp(expr->u.method_call_expression.identifier, "length")) {
            check_method_argument_count(expr->line_number, expr->u.method_call_expression.argument, 0);
            result.type = CRB_INT_VALUE;
            result.u.int_value = strlen(left->u.object->u.string.string.string);
        } else {
            error_flag = CRB_TRUE;
        }
    } else {
        error_flag = CRB_TRUE;
    }
    if (error_flag) {
        crb_runtime_error(expr->line_number, NO_SUCH_METHOD_ERR, STRING_MESSAGE_ARGUMENT, "method_name", expr->u.method_call_expression.identifier, MESSAGE_ARGUMENT_END);
    }
    pop_value(inter);
    push_value(inter, &result);
}

static void
eval_index_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr)
{
    CRB_Value *left;
    left = get_array_element_lvalue(inter, env, expr);

    push_value(inter, left);
}
