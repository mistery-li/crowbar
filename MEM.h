//
// Created by liii on 2018/1/23.
//

#ifndef CROWBAR_MEM_H
#define CROWBAR_MEM_H

#include <stdio.h>
#include <stdlib.h>

typedef struct MEM_Storage_tag *MEM_Storage;
typedef struct MEM_Controller_tag *MEM_Controller;

#endif //CROWBAR_MEM_H
#ifdef MEM_CONTROLLER
#define MEM_CURRENT_CONTROLLER MEM_CONTROLLER
#else
#define MEM_CURRENT_CONTROLLER mem_default_controller
#endif


void MEM_free_func(MEM_Controller controller, void *ptr);
void *MEM_malloc_func(MEM_Controller controller, char *filename, int line, size_t size);

#define MEM_free(ptr) (MEM_free_func(MEM_CURRENT_CONTROLLER, prt))

#define MEM_malloc(size)\
    (MEM_malloc_func(MEM_CURRENT_CONTROLLER,\
                    __FILE__,__LINE__, size))