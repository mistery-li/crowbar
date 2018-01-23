//
// Created by liii on 2018/1/23.
//
#include <math.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"

static StatementResult
execute_statement(CRB_Interpreter *inter, LocalEnvironment *env, Statement *statement);

// if语句执行

static StatementResult
execute_while_statement(CRB_Interpreter *inter, LocalEnvironment *env, Statement *statement)
{
    StatementResult result;
    CRB_Value cond;

    result.type = NORMAL_STATEMENT_RESULT;
    for (;;) {
        cond = crb_eval_expression(inter, env, statement->u.while_s.condition);
        if (cond.type != CRB_BOOLEAN_VALUE) {
            crb_runtime_error(statement->u.while_s.condition->line_number, NOT_BOOLEAN_TYPE_ERR, MESSAGE_ARGUMENT_END);
        }
        DBG_assert(cond.type == CRB_BOOLEAN_VALUE, ("cond.type..%d", cond.type));
        if (!cond.u.boolean_value)
            break;
        result = crb_execute_statement_list(inter, env, statement->u.while_s.block->statement_list);
        if (result.type == RETURN_STATEMENT_RESULT) {
            break;
        } else if (result.type == BREAK_STATEMENT_RESULT) {
            result.type = NORMAL_STATEMENT_RESULT;
            break;
        }
    }
    return result;
}

static StatementResult
execute_for_statement(CRB_Interpreter *inter, LocalEnvironment *env, Statement *statement)
{
    StatementResult result;
    CRB_Value cond;

    result.type = NORMAL_STATEMENT_RESULT;
    if (statement->u.for_s.init) {
        crb_eval_expression(inter, env, statement->u.for_s.init);
    }
    for (;;) {
        if (statement->u.for_s.condition) {
            cond = crb_eval_expression(inter, env, statement->u.for_s.condition);
            if (cond.type != CRB_BOOLEAN_VALUE) {
                crb_runtime_error(statement->u.for_s.condition->line_number, NOT_BOOLEAN_OPERATOR_ERR, MESSAGE_ARGUMENT_END);
            }
            DBG_ASSERT(cond.type == CRB_BOOLEAN_VALUE, ("cond.type..%d", cond.type));
            if (!cond.u.boolean_value)
                break;
        }
        result = crb_execute_statement_list(inter, env, statement->u.for_s.block->statement_list);
        if (result.type == RETURN_STATEMENT_RESULT) {
            break;
        } else if (result.type == BREAK_STATEMENT_RESULT) {
            result.type = NORMAL_STATEMENT_RESULT;
            break;
        }
        if (statement->u.for_s.post) {
            crb_eval_expression(itner, env, statement->u.for_s.post);
        }
    }
    return result;
}

static StatementResult
execute_return_statement(CRB_Interpreter *inter, LocalEnvironment *env, Statement *statement)
{
    StatementResult result;
    result.type = RETURN_STATEMENT_RESULT;
    if (statement->u.return_s.return_value) {
        result.u.return_value = crb_eval_expression(inter, env, statement->u.return_s.return_value);
    } else {
        result.u.return_value.type = CRB_NULL_VALUE;
    }

    return result;
}

static StatementResult
execute_break_statement(CRB_Interpreter *inter, LocalEnvironment *env, Statement *statement)
{
    StatementResult result;
    result.type = BREAK_STATEMENT_RESULT;
    return result;
}

static StatementResult
execute_continue_statement(CRB_Interpreter *inter, LocalEnvironment *env, Statement *statement)
{
    StatementResult result;
    result.type = CONTINUE_STATEMENT_RESULT;
    return result;
}

static StatementResult
execute_statement(CRB_Interpreter *inter, LocalEnvironment *env, Statement *statement)
{
    StatementResult result;

    result.type = NORMAL_STATEMENT_RESULT;

    switch (statement->type) {
        case EXPRESSION_STATEMENT:
            result = execute_expression_statement(inter, env, statement);
            break;
        case GLOBAL_STATEMENT:
            result = execute_global_statement(inter, env, statement);
            break;
        case IF_STATEMENT:
            result = execute_if_statement(inter, env, statement);
            break;
        case WHILE_STATEMENT:
            result = execute_while_statement(inter, env, statement);
        case FOR_STATEMENT:
            result = execute_for_statement(inter, env, statement);
            break;
        case RETURN_STATEMENT:
            result = execute_return_statement(inter, env, statement);
            break;
        case BREAK_STATEMENT:
            result = execute_break_statement(inter, env, statement);
            break;
        case CONTINUE_STATEMENT:
            result = execute_continue_statement(inter, env, statement);
            break;
        case STATEMENT_TYPE_COUNT_PLUS_1:
        default:
            DBG_PANIC(("bad case...%d", statement->type));
    }

    return result;
}
