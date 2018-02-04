cmake_minimum_required(VERSION 3.9)
project(crowbar C)

set(CMAKE_C_STANDARD 99)

add_executable(crowbar main.c main.c crowbar.h main.c crowbar.h error_message.c MEM.h CRB.h CRB_dev.h create.c DBG.h util.c execute.c eval.c string_pool.c heap.c native.c)