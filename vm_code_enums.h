
#ifndef _VM_CODE_ENUMS_H
#define _VM_CODE_ENUMS_H

#ifdef __cplusplus
extern "C" {
#endif

enum
{
	VM_ARG_REGISTER        = 0,
	VM_ARG_REGISTER_OFFSET = 1,
	VM_ARG_CONSTANT        = 2,
	VM_ARG_CONSTANT_OFFSET = 3
};

enum
{
	VM_REG_WHOLE = 0,
	VM_REG_LOW,
	VM_REG_HIGH,
	VM_REG_LOW_LOW,
	VM_REG_LOW_HIGH,
	VM_REG_HIGH_LOW,
	VM_REG_HIGH_HIGH
};

enum
{
	VM_CMP_LESS  = -1,
	VM_CMP_EQUAL = 0,
	VM_CMP_ABOVE = 1
};

enum
{
	VM_CODE_NOP   = 0,
	VM_CODE_MOV   = 1,
	VM_CODE_ADD   = 2,
	VM_CODE_SUB   = 3,
	VM_CODE_MUL   = 4,
	VM_CODE_DIV   = 5,
	VM_CODE_MOD   = 6,
	VM_CODE_AND   = 7,
	VM_CODE_OR    = 8,
	VM_CODE_XOR   = 9,
	VM_CODE_SHL   = 10, // 0x0a
	VM_CODE_SHR   = 11, // 0x0b
	VM_CODE_INC   = 12, // 0x0c
	VM_CODE_DEC   = 13, // 0x0d
	VM_CODE_PUSH  = 14, // 0x0e
	VM_CODE_POP   = 15, // 0x0f
	VM_CODE_PUSHA = 16, // 0x10
	VM_CODE_POPA  = 17, // 0x11
	VM_CODE_CMP   = 18, // 0x12
	VM_CODE_JMP   = 19, // 0x13
	VM_CODE_JL    = 20, // 0x14
	VM_CODE_JA    = 21, // 0x15
	VM_CODE_JE    = 22, // 0x16
	VM_CODE_JNE   = 23, // 0x17
	VM_CODE_JAE   = 24, // 0x18
	VM_CODE_JLE   = 25, // 0x19
	VM_CODE_CALL  = 26, // 0x1a
	VM_CODE_RET   = 27  // 0x1b
};

#ifdef __cplusplus
}
#endif

#endif

//EOF
