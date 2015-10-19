
#ifndef _VM_STACK_STRUCT_H
#define _VM_STACK_STRUCT_H

#ifdef __cplusplus
extern "C" {
#endif

struct StackStruct
{
	uint Value;
	struct StackStruct *Next;
};

typedef struct StackStruct StackType;

#ifdef __cplusplus
}
#endif

#endif

//EOF
