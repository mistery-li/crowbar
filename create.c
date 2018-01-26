//
// Created by liii on 2018/1/23.
//

#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"
#include "CRB_dev.h"

void
crb_function_define(char *identifier, ParameterList *parameter_list,
                    Block *block)
{
    FunctionDefinition *f;
    CRB_Interpreter *inter;
}

Expression *
crb_create_binary_expression(ExpressionType operator,
                             Expression *left, Expression *right)
{
    if ((left->type == INT_EXPRESSION || left->type == DOUBLE_EXPRESSION)
            && (right->type == INT_EXPRESSION || right->type == DOUBLE_EXPRESSION)) {
        CRB_Value v;
        v = crb_eval_binary_expression(crb_get_current_interpreter(), NULL, operator, left, right);
        *left = convert_value_to_expression(&v);

        return left;
    } else {
        Expression *exp;
        exp = crb_alloc_expression(operator);
        exp->u.binary_expression.left = left;
        exp->u.binary_expression.right = right;
        return exp;
    }
}

Expression *
crb_create_minus_expression(Expression *operand)
{
    if (operand->type == INT_EXPRESSION || operand->type == DOUBLE_EXPRESSION) {
        CRB_Value  v;
        v = crb_eval_binary_expression(crb_get_current_interpreter(), NULL, operand);

        *operand = convert_value_to_expression(&v);
        return operand;
    } else {
        Expression *exp;
        exp = crb_alloc_expression(MINUS_EXPRESSION);
        exp->u.minus_expression = operand;
        return exp;
    }
}

static Expression
convert_value_to_expression(CRB_Value *v)
{
    Expression expr;
    if (v->type == CRB_INT_VALUE) {
        expr.type = INT_EXPRESSION;
        expr.u.int_value = v->u.int_value;
    } else if (v->type == CRB_DOUBLE_VALUE) {
        expr.type = DOUBLE_EXPRESSION;
        expr.u.double_value = v->u.double_value;
    } else {
        DBG_assert(v->type == CRB_BOOLEAN_VALUE, ("v->type..%d\n", v->type));
        expr.type = BOOLEAN_EXPRESSION;
        expr.u.boolean_value = v->u.boolean_value;
    }
    return expr;
}