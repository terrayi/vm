
#ifndef _VM_STATE_ENUMS_H
#define _VM_STATE_ENUMS_H

#ifdef __cplusplus
extern "C" {
#endif

enum
{
	VM_UNINITIALIZED,
	VM_INITIALIZED,
	VM_LOADED,
	VM_RUNNING,
	VM_CODE_END_REACHED,
	VM_FINISHED,
	VM_UNLOADED
};

#ifdef __cplusplus
}
#endif

#endif

//EOF
