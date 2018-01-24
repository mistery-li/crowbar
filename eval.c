//
// Created by liii on 2018/1/24.
//

#include <math.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"
#include "CRB_dev.h"



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
eval_expression(CRB_Interpreter *inter, LocalEnvironment *env, Expression *expr)
{
    CRB_Value v;
    switch (expr->type) {
        case BOOLEAN_EXPRESSION:
            v = eval_boolean_expression(expr->u.boolean_value);
            break;
        case INT_EXPRESSION:
            v = eval_int_expression(expr->u.int_value);
            break;
        case DOUBLE_EXPRESSION:
            v = eval_double_expression(expr->u.double_value);
            break;
        case STRING_EXPRESSION:
            v = eval_string_expression(inter, expr->u.string_value);
            break;
        case IDENTIFIER_EXPRESSION:
            v = eval_identifier_expression(inter, env, expr);
            break;
        case ASSIGN_EXPRESSION:
            v = eval_assign_expression(inter, env, expr->u.assign_expression.variable, expr->u.assign_expression.operand);
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
            v = crb_eval_binary_expression(inter, env, expr->type, expr->u.binary_expression.left, expr->u.binary_expression.right);
            break;
        case LOGICAL_AND_EXPRESSION:
        case LOGICAL_OR_EXPRESSION:
            v = eval_logical_and_pr_expression(inter, env, expr->type, expr->u.binary_expression.left, expr->u.binary_expression.right);
            break;
        case MINUS_EXPRESSION:
            v = crb_eval_minus_expression(inter, env, expr->u.minus_expression);
            break;
        case FUNCTION_CALL_EXPRESSION:
            v = eval_function_call_expression(inter, env, expr);
            break;
        case NULL_EXPRESSION:
            v = eval_null_expression();
            break;
        case EXPRESSION_TYPE_COUNT_PLUS_1:
        default:
            DBG_panic(("bad case. type..%d\n", expr->type));
    }
    return v;
}

CRB_Value
crb_eval_binary_expression(CRB_Interpreter *inter, LocalEnvironment *env, ExpressionType operator, Expression *left, Expression *right)
{
    CRB_Value   left_val;
    CRB_Value   right_val;
    CRB_Value   result;

    left_val = eval_expression(inter, env, left);
    right_val = eval_expression(inter, env, right);

    if (left_val.type == CRB_INT_VALUE && right_val.type == CRB_INT_VALUE) {
        eval_binary_int(inter, operator, left_val.u.int_value, right_val.u.int_value, &result, left->line_number);
    } else if (left_val.type == CRB_DOUBLE_VALUE
            && right_val.type == CRB_DOUBLE_VALUE) {
        eval_binary_double(inter, operator, left_val.u.double_value, right_val.u.double_value,
                            &result, left->line_number);
    } else if (left_val.type == CRN_INT_VALUE && right_val.type == CRB_DOUBLE_VALUE) {
        left_val.u.double_value = left_val.u.int_value;
        eval_binary_double(inter, operator,
                            left_val.u.double_value, right_val.u.double_value, &result, left->line_number);
    } else if (left_val.type == CRB_DOUBLE_VALUE && right_val.type == CRB_INT_VALUE) {
        right_val.u.double_value = right_val.u.int_value;
        eval_binary_double(inter, operator,
                            left_val.u.double_value, right_val.u.double_value,
                            &result, left->line_number);
    } else if (left_val.type == CRB_BOOLEAN_VALUE && right_val.type == CRB_BOOLEAN_VALUE) {
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value = eval_binary_boolean(inter, operator, left_val.u.boolean_value, right_val.u.boolean_value,
                                                    left->line_number);
    } else if (left_val.type == CRB_STRING_VALUE && operator == ADD_EXPRESSION) {
        char buf[LINE_BUF_SIZE];
        CRB_String *right_str;

        if (right_val.type == CRB_INT_VALUE) {
            sprintf(buf, "%d", right_val.u.int_value);
            right_str = crb_create_crowbar_string(inter, MEM_strdup(buf));
        } else if (right_val.type == CRB_DOUBLE_VALUE) {
            sprintf(buf, "%f", right_val.u.double_value);
            right_str = crb_create_crowbar_string(inter, MEM_strdup(buf));
        } else if (right_val.type == CRB_BOOLEAN_VALUE) {
            if (right_val.u.boolean_value) {
                right_str = crb_create_crowbar_string(inter, MEM_strdup("true"));
            } else {
                right_str = crb_create_crowbar_string(inter, MEM_strdup("false"));
            }
        } else if (right_val.type == CRB_STRING_VALUE) {
            right_str = right_val.u.string_value;
        } else if (right_val.type == CRB_NATIVE_POINTER_VALUE) {
            sprintf(buf, "(%s:%p)", right_val.u.native_pointer.info->name, right_val.u.native_pointer.pointer);
            right_str = crb_create_crowbar_string(inter, MEM_strdup("null"));
        }
        result.type = CRB_STRING_VALUE;
        result.u.string_value = chain_string(inter, left_val.u.string_value, right_str);
    } else if (left_val.type == CRB_STRING_VALUE && right_val.type == CRB_STRING_VALUE) {
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value = eval_compare_string(operator, &left_val, &right_val, left->line_number);
    } else if (left_val.type == CRB_NULL_VALUE || right_val.type == CRB_NULL_VALUE) {
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value = eval_binary_null(inter, operator, &left_val, &right_val, left->line_number);
    } else {
        char *op_str = crb_get_operator_string(operator);
        crb_runtime_error(left->line_number, BAD_OPERAND_TYPE_ERR, STRING_MESSAGE_ARGUMENT, "operator", op_str, MESSAGE_ARGUMENT_END);
    }
    return result;
}

static void
eval_binary_int()
{
    
}