//
// Created by liii on 2018/2/4.
//

#include <stdio.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "CRB_dev.h"
#include "crowbar.h"

#define NATIVE_LIB_NAME "crowbar.lang.file"

CRB_Value
new_array_sub(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int arg_count, CRB_Value *args, int arg_idx)
{
    CRB_Value ret;
    int size;
    int i;

    if (args[arg_idx].type != CRB_INT_VALUE) {
        crb_runtime_error(0, NEW_ARRAY_ARGUMENT_TYPE_ERR, MESSAGE_ARGUMENT_END);
    }

    size = args[arg_idx].u.int_value;

    ret.type = CRB_ARRAY_VALUE;
    ret.u.object = CRB_create_array(inter, env, size);

    if (arg_idx == arg_count-1) {
        for (i = 0; i < size; i++) {
            ret.u.object->u.array.array[i].type = CRB_NULL_VALUE;
        }
    } else {
        for (i = 0; i < size; i++) {
            ret.u.object->u.array.array[i]
                    = new_array_sub(inter, env, arg_count, args, arg_idx+1);
        }
    }
    return ret;
}

CRB_Value
crb_nv_new_array_proc(CRB_Interpreter *interpreter, CRB_LocalEnvironment *env, int arg_count, CRB_Value *args)
{
    CRB_Value value;

    if (arg_count < 1) {
        crb_runtime_error(0, ARGUMENT_TOO_FEW_ERR, MESSAGE_ARGUMENT_END);
    }

    value = new_array_sub(interpreter, env, arg_count, args, 0);

    return value;
}