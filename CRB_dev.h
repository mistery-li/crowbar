//
// Created by liii on 2018/1/23.
//

#ifndef CROWBAR_CRB_DEV_H
#define CROWBAR_CRB_DEV_H

#endif //CROWBAR_CRB_DEV_H

typedef enum {
    CRB_FALSE = 0;
    CRB_TRUE = 1
} CRB_Boolean;

typedef struct {
    char *name;
} CRB_NativePointerInfo;

typedef enum {
    CRB_BOOLEAN_VALUE = 1,
    CRB_INT_VALUE,
    CRB_DOUBLE_VALUE,
    CRB_STRING_VALUE,
    CRB_NATIVE_POINTER_VALUE,
    CRB_NULL_VALUE
} CRB_ValueType;

typedef struct {
    CRB_NativePointerInfo   *info;
    void        *pointer;
} CRB_NativePointer;

typedef struct {
    CRB_ValueType  type;
    union {
        CRB_Boolean boolean_value;
        int         int_value;
        double      double_value;
        CRB_NativePointer  native_pointer;
        CRB_Object   *object;
    } u;
} CRB_Value;

void CRB_add_native_function(CRB_Interpreter *interpreter,
                             char *name, CRB_NativeFunctionProc *proc);

void CRB_add_global_variable(CRB_Interpreter *inter,
                             char *identifier, CRB_Value *value);