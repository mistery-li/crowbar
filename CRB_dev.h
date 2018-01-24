//
// Created by liii on 2018/1/23.
//

#ifndef CROWBAR_CRB_DEV_H
#define CROWBAR_CRB_DEV_H

#endif //CROWBAR_CRB_DEV_H

typedef struct {
    CRB_ValueType  type;
    union {
        CRB_Boolean boolean_value;
        int         int_value;
        double      double_value;
        CRB_String  *string_value;
        CRB_NativePointer  native_pointer;
    } u;
} CRB_Value;