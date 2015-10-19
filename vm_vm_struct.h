
#ifndef _VM_VM_STRUCT_H
#define _VM_VM_STRUCT_H

#include "vm_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

struct VirtualMachineStruct
{
	int State;
	reg_t Registers[NUMBER_OF_REGISTERS];
	reg_t CmpRet;
	uint CodePoint;
	byte *Code;
	size_t CodeLength;
	struct StackStruct *Stack;
	struct StackStruct *RetAddStack;
	//struct MemorySection *AccessibleMemory;
};

typedef struct VirtualMachineStruct VM;

#ifdef __cplusplus
}
#endif

#endif

//EOF
