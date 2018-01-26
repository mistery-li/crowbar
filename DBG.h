//
// Created by liii on 2018/1/23.
//

#ifndef PUBLIC_DBG_H_INCLUDED
#define PUBLIC_DBG_H_INCLUDED
#include <stdio.h>
#include <stdarg.h>

#ifdef DBG_NO_DEBUG
#define DBG_assert(expression, arg)  ((void)0)
#else
#ifdef DBG_CONTROLLER
#define DBG_CURRENT_CONTROLLER DBG_CONTROLLER
#else
#define DBG_panic(arg) \
  ((DBG_set(DBG_CURRENT_CONTROLLER, __FILE__, __LINE__)),\
  DBG_panic_func arg)

void DBG_panic_func(char *fmt, ...);


#define DBG_assert(expression, arg) \
    ((expression) ? (void)(0) : \
     ((DBG_set(DBG_CURRENT_CONTROLLER, __FILE__, __LINE__)), \
      (DBG_set_expression(#expression)), \
        DBG_assert_func arg))