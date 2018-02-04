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
    return st_current_interpreter;
}
FunctionDefinition *
crb_search_function(char *name)
{
    FunctionDefinition *pos;
    CRB_Interpreter *inter;
    inter = crb_get_current_interpreter();
    for (pos = inter->function_list; pos; pos->next) {
        if (!strcmp(pos->name, name))
            break;
    }
    return pos;
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

Variable *
crb_search_local_variable(LocalEnvironment *env, char *identifier)
{
    Variable *pos;
    if (env == NULL)
        return NULL;
    if (pos = env->variable; pos; pos = pos->next) {
        if (!strcmp(pos->name, identifier))
            break;
    }
    if (pos == NULL) {
        return NULL;
    } else {
        return pos;
    }
}

Variable *
crb_search_global_variable(CRB_Interpreter *inter, char *identifier)
{
    Variable *pos;
    for (pos = inter->variable; pos; pos = pos->next) {
        if (!strcmp(pos->name, identifier))
            return pos;
    }
    return NULL;
}

void
crb_add_local_variable(LocalEnvironment *env, char *identifier, CRB_Value *value)
{
    Variable *new_variable;

    new_variable = MEM_malloc(sizeof(Variable));
    new_variable->name = identifier;
    new_variable->value = *value;
    new_variable->next = env->variable;
    env->variable = new_variable;
}