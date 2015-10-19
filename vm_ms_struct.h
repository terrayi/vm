
#ifndef _VM_MS_STRUCT_H
#define _VM_MS_STRUCT_H

#ifdef __cplusplus
extern "C" {
#endif

struct MemorySection
{
	uint Address;
	uint Length;
	struct MemorySection *Next;
};

typedef struct MemorySection SectionType;

#ifdef __cplusplus
}
#endif

#endif

//EOF
