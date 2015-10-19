
#ifndef _VM_FUNCTIONS_H
#define _VM_FUNCTIONS_H

#include "vm_vm_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

int vm_init(VM *vm);
int vm_load(VM *vm, byte *code, size_t code_length);
int vm_step(VM *vm);
int vm_run(VM *vm);
int vm_uninit(VM *vm);
int vm_quick_run(VM *vm, byte *code, size_t code_length);

#ifdef __cplusplus
}
#endif

#endif

//EOF
