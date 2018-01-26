//
// Created by liii on 2018/1/23.
//

#ifndef CROWBAR_MEM_H
#define CROWBAR_MEM_H

#include <stdio.h>
#include <stdlib.h>

typedef struct MEM_Storage_tag *MEM_Storage;

#endif //CROWBAR_MEM_H


void MEM_free_func(MEM_Controller controller, void *ptr);

#define MEM_free(ptr) (MEM_free_func(MEM_CURRENT_CONTROLLER, prt))