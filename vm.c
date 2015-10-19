
#include "vm_includes.h"
#include "vm_typedefs.h"
//#include "vm_ms_struct.h"
#include "vm_stack_struct.h"
#include "vm_ret_enums.h"
#include "vm_state_enums.h"
#include "vm_code_enums.h"
#include "vm_functions.h"
#include "vm.h"
#include "vm_error_defines.h"

typedef int (*vm_func)(VM*);

#define GET_REG_ACCESS(c) ((c & 0xf0) >> 4)
#define GET_REG_NUM(c) (c & 0x0f)

#ifdef DEBUG
	#include <stdio.h>
	#define LOG printf

	char* __reg_access[] = {
			"\0",
			"L\0",
			"H\0",
			"LL\0",
			"LH\0",
			"HL\0",
			"HH\0"
		};
#else
	#define LOG(...) ;
#endif

#define VM_CODE_LOCAL_VARIABLES \
	byte register_code = 0; \
	int dest_type = vm_get_dest_type(vm->Code[vm->CodePoint]); \
	uint src_value = 0; \
	uint dest_value = 0; \
	int ret = -1;

#define VM_ARG_REGISTER_LOG \
	LOG("reg%d%s\n", \
		GET_REG_NUM(register_code), \
		__reg_access[GET_REG_ACCESS(register_code)]);

#define VM_ARG_REGISTER_OFFSET_LOG \
	LOG("@reg%d%s (@0x%08x)\n", \
		GET_REG_NUM(register_code), \
		__reg_access[GET_REG_ACCESS(register_code)], \
		dest_value);

#define VM_CODE_INCORRECT_ARG_CODE \
	default: \
		LOG("[incorrect code]\n"); \
		return VM_RET_EXCEPTION;

#define VM_CODE_GET_SRC_VALUE(a) \
	ret = vm_get_src_value(vm, (a), &src_value); \
	if (ret < 1) \
	{ \
		return VM_RET_EXCEPTION; \
	} \
	ret += (a);

#define VM_CODE_BEFORE_SWITCH LOG("\ttarget: "); \
	register_code = vm->Code[vm->CodePoint + 1];

#define VM_CODE_GET_DEST_VALUE \
	dest_value = vm_get_register_value(vm, register_code);

#define CURRENT_REG vm->Registers[register_number]
#define VM_CODE_SET_REGISTER_VALUE(v) \
	vm_set_register_value(vm, register_code, (v));

#define OFFSET(v) ((uint*) &(v))[0]

#define VM_CODE_COMMON_PREPOST1(t) \
	VM_CODE_LOCAL_VARIABLES \
	LOG(t); \
	VM_CODE_GET_SRC_VALUE(2) \
	VM_CODE_BEFORE_SWITCH \
	VM_CODE_GET_DEST_VALUE

#define VM_CODE_COMMON_PREPOST2(t) \
	uint src_value = 0; \
	int ret = 1; \
	LOG(t); \
	VM_CODE_GET_SRC_VALUE(1);

#define VM_CODE_COMMON_PREPOST3(t) \
	int dest_type = vm_get_dest_type(vm->Code[vm->CodePoint]); \
	int register_code = 0; \
	uint dest_value = 0; \
	LOG(t); \
	VM_CODE_BEFORE_SWITCH \
	VM_CODE_GET_DEST_VALUE

#define VM_ARG_REGISTER_CASE(v) \
	case VM_ARG_REGISTER: \
		VM_CODE_SET_REGISTER_VALUE((v)); \
		VM_ARG_REGISTER_LOG \
		break;

vm_func code_table[] = {
	vm_code_nop,
	vm_code_mov,
	vm_code_add,
	vm_code_sub,
	vm_code_mul,
	vm_code_div,
	vm_code_mod,
	vm_code_and,
	vm_code_or,
	vm_code_xor,
	vm_code_shl,
	vm_code_shr,
	vm_code_inc,
	vm_code_dec,
	vm_code_push,
	vm_code_pop,
	vm_code_pusha,
	vm_code_popa,
	vm_code_cmp,
	vm_code_jmp,
	vm_code_jl,
	vm_code_ja,
	vm_code_je,
	vm_code_jne,
	vm_code_jae,
	vm_code_jle,
	vm_code_call,
	vm_code_ret
};

int vm_init(VM *vm)
{
	int i;

	for (i = 0; i < NUMBER_OF_REGISTERS; i++)
	{
		vm->Registers[i].val = 0;
	}

	vm->CmpRet.val = 0;
	vm->CodePoint = 0;
	vm->Code = 0;
	vm->CodeLength = 0;
	vm->State = VM_INITIALIZED;
	vm->Stack = 0;
	vm->RetAddStack = 0;
	//vm->AccessibleMemory = 0;

	return VM_RET_OK;
}

int vm_load(VM *vm, byte *code, size_t code_length)
{
	if (vm->State != VM_INITIALIZED && vm->State != VM_UNLOADED)
	{
		return VM_RET_EXCEPTION;
	}

	if (!code_length)
	{
		return VM_RET_EXCEPTION;
	}

	vm->Code = (byte*) malloc(code_length);
	memcpy(vm->Code, code, code_length);
	vm->CodeLength = code_length;
	vm->State = VM_LOADED;

	return VM_RET_OK;
}

int vm_step(VM *vm)
{
	byte cmd = 0;
	int inc = 0;

	if (vm->State != VM_RUNNING)
	{
		LOG("vm is not running\n");
		return VM_RET_EXCEPTION;
	}

	if (vm->CodePoint >= vm->CodeLength)
	{
		vm->State = VM_CODE_END_REACHED;
		LOG("end of code reached\n");
		return VM_RET_OK;
	}

	cmd = vm->Code[vm->CodePoint] & 0x1f;
	inc = code_table[cmd](vm);

	if (inc < 0)
	{
		LOG("error execution\n");
		return VM_RET_EXCEPTION;
	}

	vm->CodePoint += (uint) inc;

	return VM_RET_OK;
}

int vm_run(VM *vm)
{
	int ret = 0;

	if (vm->State != VM_LOADED)
	{
		return VM_RET_ERROR;
	}

	vm->State = VM_RUNNING;

	while (vm->State == VM_RUNNING)
	{
		ret = vm_step(vm);

		if (ret != VM_RET_OK)
		{
			return ret;
		}
	}

	return VM_RET_OK;
}

int vm_uninit(VM *vm)
{
	//SectionType *st;
	StackType *stack = 0;

	if (vm->CodeLength && vm->Code)
	{
		free(vm->Code);
	}

/*
	while (vm->AccessibleMemory)
	{
		st = vm->AccessibleMemory;
		vm->AccessibleMemory = vm->AccessibleMemory->Next;
		free(st);
	}
*/
	//clear vm->Stack
	while (vm->Stack)
	{
		stack = vm->Stack;
		vm->Stack = vm->Stack->Next;
		free(stack);
	}

	//clear vm->RetAddStack
	while (vm->RetAddStack)
	{
		stack = vm->RetAddStack;
		vm->RetAddStack = vm->RetAddStack->Next;
		free(stack);
	}

	vm->State = VM_UNINITIALIZED;

	return VM_RET_OK;
}

int vm_quick_run(VM *vm, byte *code, size_t code_length)
{
	int ret = VM_RET_ERROR;

	vm_init(vm);

	ret = vm_load(vm, code, code_length);

	if (ret != VM_RET_OK)
	{
		return ret;
	}

	ret = vm_run(vm);

	if (ret != VM_RET_OK)
	{
		return ret;
	}

	vm_uninit(vm);

	return VM_RET_OK;
}

/*
int vm_add_access_memory(VM *vm, uint address, uint length)
{
	SectionType *st;

	st = (SectionType*) malloc(sizeof(SectionType));
	st->Address = address;
	st->Length = length;
	st->Next = vm->AccessibleMemory;
	vm->AccessibleMemory = st;

	return VM_RET_OK;
}
*/

int vm_func_pop_stack(VM *vm, uint register_number)
{
	StackType *old = 0;

	if (register_number >= NUMBER_OF_REGISTERS)
	{
		return VM_RET_ERROR;
	}

	if (!vm->Stack)
	{
		return VM_RET_ERROR;
	}

	vm->Registers[register_number].val = vm->Stack->Value;
	old = vm->Stack;
	free(old);
	vm->Stack = vm->Stack->Next;

	return VM_RET_OK;
}

int vm_func_push_stack(VM *vm, uint register_number)
{
	StackType *stack;

	if (register_number >= NUMBER_OF_REGISTERS)
	{
		return VM_RET_ERROR;
	}

	stack = (StackType*) malloc(sizeof(StackType));
	stack->Value = vm->Registers[register_number].val;
	stack->Next = vm->Stack;
	vm->Stack = stack;

	return VM_RET_OK;
}

int vm_func_pop_code_point(VM *vm)
{
	StackType *old = 0;

	if (!vm->RetAddStack)
	{
		return VM_RET_ERROR;
	}

	vm->CodePoint = vm->RetAddStack->Value;
	old = vm->RetAddStack;
	free(old);
	vm->RetAddStack = vm->RetAddStack->Next;

	return VM_RET_OK;
}

int vm_func_push_code_point(VM *vm)
{
	StackType *stack;

	stack = (StackType*) malloc(sizeof(StackType));
	stack->Value = vm->CodePoint;
	stack->Next = vm->RetAddStack;
	vm->RetAddStack = stack;

	return VM_RET_OK;
}

int vm_get_dest_type(byte code)
{
	code = (code & 0x80) >> 7;

	return (int) code;
}

int vm_get_src_type(byte code)
{
	code = (code & 0x60) >> 5;

	return (int) code;
}

uint vm_get_register_value(VM *vm, byte register_code)
{
	uint value = 0;
	int register_access = GET_REG_ACCESS(register_code);
	int register_number = GET_REG_NUM(register_code);

	if (register_number >= NUMBER_OF_REGISTERS)
	{
		return VM_RET_EXCEPTION;
	}

	value = vm->Registers[register_number].val;

	switch (register_access) {
	case VM_REG_WHOLE:
		break;

	case VM_REG_LOW:
		value = (value & 0x0000ffff);
		break;

	case VM_REG_HIGH:
		value = (value & 0xffff0000) >> 16;
		break;

	case VM_REG_LOW_LOW:
		value = (value & 0x000000ff);
		break;

	case VM_REG_LOW_HIGH:
		value = (value & 0x0000ff00) >> 8;
		break;

	case VM_REG_HIGH_LOW:
		value = (value & 0x00ff0000) >> 16;
		break;

	case VM_REG_HIGH_HIGH:
		value = (value & 0xff000000) >> 24;
		break;
	}

	return value;
}

uint vm_get_value_at_offset(VM *vm, uint address)
{
	if (address >= 0 && address < vm->CodeLength)
	{
		return ((uint*)(vm->Code + address))[0];
	}
	else
	{
		return OFFSET(address);
	}
}

uint vm_get_value_from_code(VM *vm, uint address)
{
	return ((uint*)(vm->Code + address))[0];
}

int vm_get_src_value(VM *vm, int relative_offset, uint *value)
{
	int register_code = 0;
	int src_type = vm_get_src_type(vm->Code[vm->CodePoint]);
	int point = vm->CodePoint + relative_offset;
	int ret = 0;

	LOG("\tsource: ");

	switch (src_type) {
	case VM_ARG_REGISTER:
		register_code = vm->Code[point];
		*value = vm_get_register_value(vm, register_code);
		LOG("reg%d%s (0x%08x)\n",
			GET_REG_NUM(register_code),
			__reg_access[GET_REG_ACCESS(register_code)],
			*value);
		ret = 1;
		break;

	case VM_ARG_REGISTER_OFFSET:
		register_code = vm->Code[point];
		*value = vm_get_register_value(vm, register_code);
		*value = vm_get_value_at_offset(vm, *value);
		LOG("@reg%d%s (0x%08x)\n",
			GET_REG_NUM(register_code),
			__reg_access[GET_REG_ACCESS(register_code)],
			*value);
		ret = 1;
		break;

	case VM_ARG_CONSTANT:
		*value = vm_get_value_from_code(vm, point);
		LOG("0x%08x\n", *value);
		ret = 4;
		break;

	case VM_ARG_CONSTANT_OFFSET:
		*value = vm_get_value_from_code(vm, point);
		LOG("@0x%08x", *value);
		*value = vm_get_value_at_offset(vm, *value);
		LOG(" (0x%08x)\n", *value);
		ret = 4;
		break;

	default:
		LOG("[incorrect code]\n");
		return VM_RET_EXCEPTION;
	}

	return ret;
}

int vm_set_register_value(VM *vm, byte register_code, uint value)
{
	int register_access = GET_REG_ACCESS(register_code);
	int register_number = GET_REG_NUM(register_code);

	if (register_number >= NUMBER_OF_REGISTERS)
	{
		return VM_RET_EXCEPTION;
	}

	switch (register_access) {
	case VM_REG_WHOLE:
		CURRENT_REG.val = value;
		break;

	case VM_REG_LOW:
		CURRENT_REG.words.l = (ushort) value;
		break;

	case VM_REG_HIGH:
		CURRENT_REG.words.h = (ushort) value;
		break;

	case VM_REG_LOW_LOW:
		CURRENT_REG.bytes.ll = (byte) value;
		break;

	case VM_REG_LOW_HIGH:
		CURRENT_REG.bytes.lh = (byte) value;
		break;

	case VM_REG_HIGH_LOW:
		CURRENT_REG.bytes.hl = (byte) value;
		break;

	case VM_REG_HIGH_HIGH:
		CURRENT_REG.bytes.hh = (byte) value;
		break;

	default:
		return VM_RET_EXCEPTION;
	}

	return VM_RET_OK;
}

int vm_set_value_at_offset(VM *vm, int access, uint address, uint value)
{
	int internal;

	internal = (address >= 0 && address < vm->CodeLength) ? 1 : 0;

	switch (access) {
	case VM_REG_WHOLE:
		if (internal)
		{
			((uint*)(vm->Code + address))[0] = value;
		}
		else
		{
			OFFSET(address) = value;
		}
		break;

	case VM_REG_LOW:
		if (internal)
		{
			((ushort*)(vm->Code + address))[1] = (ushort) value;
		}
		else
		{
			((ushort*)&address)[1] = (ushort) value;
		}
		break;

	case VM_REG_HIGH:
		if (internal)
		{
			((ushort*)(vm->Code + address))[0] = (ushort) value;
		}
		else
		{
			((ushort*)&address)[0] = (ushort) value;
		}
		break;

	case VM_REG_LOW_LOW:
		if (internal)
		{
			((byte*)(vm->Code + address))[3] = (byte) value;
		}
		else
		{
			((byte*)&address)[3] = (byte) value;
		}
		break;

	case VM_REG_LOW_HIGH:
		if (internal)
		{
			((byte*)(vm->Code + address))[2] = (byte) value;
		}
		else
		{
			((byte*)&address)[2] = (byte) value;
		}
		break;

	case VM_REG_HIGH_LOW:
		if (internal)
		{
			((byte*)(vm->Code + address))[1] = (byte) value;
		}
		else
		{
			((byte*)&address)[1] = (byte) value;
		}
		break;

	case VM_REG_HIGH_HIGH:
		if (internal)
		{
			((byte*)(vm->Code + address))[0] = (byte) value;
		}
		else
		{
			((byte*)&address)[0] = (byte) value;
		}
		break;
	}

	return VM_RET_OK;
}

//
int vm_code_nop(VM *vm)
{
	LOG("nop\n");
	return 1;
}

int vm_code_mov(VM *vm)
{
	int access = 0;

	VM_CODE_LOCAL_VARIABLES
	LOG("mov\n");
	VM_CODE_GET_SRC_VALUE(2)
	VM_CODE_BEFORE_SWITCH

	switch (dest_type) {
	VM_ARG_REGISTER_CASE(src_value)

	case VM_ARG_REGISTER_OFFSET:
		access = GET_REG_ACCESS(vm->Code[vm->CodePoint + 1]);
		VM_CODE_GET_DEST_VALUE
		VM_ARG_REGISTER_OFFSET_LOG
		vm_set_value_at_offset(vm, access, dest_value, src_value);
		break;

	VM_CODE_INCORRECT_ARG_CODE
	}

	return ret;
}

int vm_code_add(VM *vm)
{
	VM_CODE_COMMON_PREPOST1("add\n")

	switch (dest_type) {
	VM_ARG_REGISTER_CASE(dest_value + src_value)

	case VM_ARG_REGISTER_OFFSET:
		VM_ARG_REGISTER_OFFSET_LOG
		OFFSET(dest_value) += src_value;
		break;

	VM_CODE_INCORRECT_ARG_CODE
	}

	return ret;
}

int vm_code_sub(VM *vm)
{
	VM_CODE_COMMON_PREPOST1("sub\n");

	switch (dest_type) {
	VM_ARG_REGISTER_CASE(dest_value - src_value);

	case VM_ARG_REGISTER_OFFSET:
		VM_ARG_REGISTER_OFFSET_LOG
		OFFSET(dest_value) -= src_value;
		break;

	VM_CODE_INCORRECT_ARG_CODE
	}

	return ret;
}

int vm_code_mul(VM *vm)
{
	VM_CODE_COMMON_PREPOST1("mul\n");

	switch (dest_type) {
	VM_ARG_REGISTER_CASE(dest_value * src_value)

	case VM_ARG_REGISTER_OFFSET:
		VM_ARG_REGISTER_OFFSET_LOG
		OFFSET(dest_value) *= src_value;
		break;

	VM_CODE_INCORRECT_ARG_CODE
	}

	return ret;
}

int vm_code_div(VM *vm)
{
	VM_CODE_COMMON_PREPOST1("div\n");

	switch (dest_type) {
	VM_ARG_REGISTER_CASE(dest_value / src_value)

	case VM_ARG_REGISTER_OFFSET:
		VM_ARG_REGISTER_OFFSET_LOG
		OFFSET(dest_value) /= src_value;
		break;

	VM_CODE_INCORRECT_ARG_CODE
	}

	return ret;
}

int vm_code_mod(VM *vm)
{
	VM_CODE_COMMON_PREPOST1("mod\n");

	switch (dest_type) {
	VM_ARG_REGISTER_CASE(dest_value % src_value)

	case VM_ARG_REGISTER_OFFSET:
		VM_ARG_REGISTER_OFFSET_LOG
		OFFSET(dest_value) %= src_value;
		break;

	VM_CODE_INCORRECT_ARG_CODE
	}

	return ret;
}

int vm_code_and(VM *vm)
{
	VM_CODE_COMMON_PREPOST1("and\n");

	switch (dest_type) {
	VM_ARG_REGISTER_CASE(dest_value & src_value)

	case VM_ARG_REGISTER_OFFSET:
		VM_ARG_REGISTER_OFFSET_LOG
		OFFSET(dest_value) &= src_value;
		break;

	VM_CODE_INCORRECT_ARG_CODE
	}

	return ret;
}

int vm_code_or(VM *vm)
{
	VM_CODE_COMMON_PREPOST1("or\n");

	switch (dest_type) {
	VM_ARG_REGISTER_CASE(dest_value | src_value)

	case VM_ARG_REGISTER_OFFSET:
		VM_ARG_REGISTER_OFFSET_LOG
		OFFSET(dest_value) |= src_value;
		break;

	VM_CODE_INCORRECT_ARG_CODE
	}

	return ret;
}

int vm_code_xor(VM *vm)
{
	VM_CODE_COMMON_PREPOST1("xor\n");

	switch (dest_type) {
	VM_ARG_REGISTER_CASE(dest_value ^ src_value)

	case VM_ARG_REGISTER_OFFSET:
		VM_ARG_REGISTER_OFFSET_LOG
		OFFSET(dest_value) ^= src_value;
		break;

	VM_CODE_INCORRECT_ARG_CODE
	}

	return ret;
}

int vm_code_shl(VM *vm)
{
	VM_CODE_COMMON_PREPOST1("shl\n");

	switch (dest_type) {
	VM_ARG_REGISTER_CASE(dest_value << src_value)

	case VM_ARG_REGISTER_OFFSET:
		VM_ARG_REGISTER_OFFSET_LOG
		OFFSET(dest_value) <<= src_value;
		break;

	VM_CODE_INCORRECT_ARG_CODE
	}

	return ret;
}

int vm_code_shr(VM *vm)
{
	VM_CODE_COMMON_PREPOST1("shr\n");

	switch (dest_type) {
	VM_ARG_REGISTER_CASE(dest_value >> src_value)

	case VM_ARG_REGISTER_OFFSET:
		VM_ARG_REGISTER_OFFSET_LOG
		OFFSET(dest_value) >>= src_value;
		break;

	VM_CODE_INCORRECT_ARG_CODE
	}

	return ret;
}

int vm_code_inc(VM *vm)
{
	VM_CODE_COMMON_PREPOST3("inc\n")

	switch (dest_type) {
	VM_ARG_REGISTER_CASE(dest_value + 1)

	case VM_ARG_REGISTER_OFFSET:
		VM_ARG_REGISTER_OFFSET_LOG
		OFFSET(dest_value) += 1;
		break;

	VM_CODE_INCORRECT_ARG_CODE
	}

	return 2;
}

int vm_code_dec(VM *vm)
{
	VM_CODE_COMMON_PREPOST3("dec\n")

	switch (dest_type) {
	VM_ARG_REGISTER_CASE(dest_value - 1)

	case VM_ARG_REGISTER_OFFSET:
		VM_ARG_REGISTER_OFFSET_LOG
		OFFSET(dest_value) -= 1;
		break;

	VM_CODE_INCORRECT_ARG_CODE
	}

	return 2;
}

int vm_code_pop(VM *vm)
{
	byte register_number = GET_REG_NUM(vm->Code[vm->CodePoint + 1]);
	int ret = 0;

	LOG("pop reg%d\n", register_number);

	ret = vm_func_pop_stack(vm, register_number);

	if (ret != VM_RET_OK)
	{
		LOG(VM_ERROR_EXE_INCOMPLETE);
		return VM_RET_EXCEPTION;
	}

	return 2;
}

int vm_code_push(VM *vm)
{
	byte register_number = GET_REG_NUM(vm->Code[vm->CodePoint + 1]);
	int ret = 0;

	LOG("push reg%d\n", register_number);

	ret = vm_func_push_stack(vm, register_number);

	if (ret != VM_RET_OK)
	{
		LOG(VM_ERROR_EXE_INCOMPLETE);
		return VM_RET_EXCEPTION;
	}

	return 2;
}

int vm_code_pusha(VM *vm)
{
	int i;
	int ret = 0;

	LOG("pusha\n");

	for (i = 0; i < NUMBER_OF_REGISTERS; i++)
	{
		ret = vm_func_push_stack(vm, i);

		if (ret != VM_RET_OK)
		{
			LOG(VM_ERROR_EXE_INCOMPLETE);
			return ret;
		}
	}

	return 1;
}

int vm_code_popa(VM *vm)
{
	int i;
	int ret = 0;

	LOG("popa\n");

	for (i = NUMBER_OF_REGISTERS - 1; i >= 0; i--)
	{
		ret = vm_func_pop_stack(vm, i);

		if (ret != VM_RET_OK)
		{
			LOG(VM_ERROR_EXE_INCOMPLETE);
			return ret;
		}
	}

	return 1;
}

int vm_code_cmp(VM *vm)
{
	VM_CODE_COMMON_PREPOST1("cmp\n");

	switch (dest_type) {
	case VM_ARG_REGISTER:
		VM_ARG_REGISTER_LOG
		break;

	case VM_ARG_REGISTER_OFFSET:
		VM_ARG_REGISTER_OFFSET_LOG
		dest_value = OFFSET(dest_value);
		break;

	VM_CODE_INCORRECT_ARG_CODE
	}

	if (dest_value == src_value)
	{
		vm->CmpRet.val = VM_CMP_EQUAL;
	}
	else if (dest_value < src_value)
	{
		vm->CmpRet.val = VM_CMP_LESS;
	}
	else if (dest_value > src_value)
	{
		vm->CmpRet.val = VM_CMP_ABOVE;
	}

	return ret;
}

int vm_code_jmp(VM *vm)
{
	VM_CODE_COMMON_PREPOST2("jmp\n")
	vm->CodePoint = src_value;

	return ret;
}

int vm_code_jl(VM *vm)
{
	VM_CODE_COMMON_PREPOST2("jl\n")

	if (vm->CmpRet.val == VM_CMP_LESS)
	{
		vm->CodePoint = src_value;
	}

	return ret;
}

int vm_code_ja(VM *vm)
{
	VM_CODE_COMMON_PREPOST2("ja\n")

	if (vm->CmpRet.val == VM_CMP_ABOVE)
	{
		vm->CodePoint = src_value;
	}

	return ret;
}

int vm_code_je(VM *vm)
{
	VM_CODE_COMMON_PREPOST2("je\n")

	if (vm->CmpRet.val == VM_CMP_EQUAL)
	{
		vm->CodePoint = src_value;
	}

	return ret;
}

int vm_code_jne(VM *vm)
{
	VM_CODE_COMMON_PREPOST2("jne\n")

	if (vm->CmpRet.val != VM_CMP_EQUAL)
	{
		vm->CodePoint = src_value;
	}

	return ret;
}

int vm_code_jae(VM *vm)
{
	VM_CODE_COMMON_PREPOST2("jae\n")

	if (vm->CmpRet.val >= VM_CMP_EQUAL)
	{
		vm->CodePoint = src_value;
	}

	return ret;
}

int vm_code_jle(VM *vm)
{
	VM_CODE_COMMON_PREPOST2("jle\n")

	if (vm->CmpRet.val <= VM_CMP_EQUAL)
	{
		vm->CodePoint = src_value;
	}

	return ret;
}

int vm_code_call(VM *vm)
{
	VM_CODE_COMMON_PREPOST2("call\n")
	vm_func_push_code_point(vm);
	vm->CodePoint = src_value;
 
	return 0;
}

int vm_code_ret(VM *vm)
{
	LOG("ret\n");
	vm_func_pop_code_point(vm);

	return 0;
}

//EOF
