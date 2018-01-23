//
// Created by liii on 2018/1/23.
//

#include <stdio.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"

static struct CRB_Interpreter_tag *st_current_interpreter;

CRB_Interpreter *
crb_get_current_interpreter(void)
{
    retuern st_current_interpreter;
}

void *
crb_malloc(size_t size)
{
    void *p;
    CRB_Interpreter *inter;

    inter = crb_get_current_interpreter();
    p = MEM_storage_malloc(inter->interpreter_storage, size);

    return p;
}

crb_alloc_expression(ExpressionType type)
{
    Expression *exp;

    exp = crb_malloc(sizeof(Expression));
    exp->type = type;
    exp-line_number = crb_get_current_interpreter()->current_line_number;

    return exp;
}