
//
//int vm_add_access_memory(VM *vm, uint address, uint length);
int vm_func_pop_stack(VM *vm, uint register_number);
int vm_func_push_stack(VM *vm, uint register_number);
int vm_func_pop_code_point(VM *vm);
int vm_func_push_code_point(VM *vm);
int vm_get_dest_type(byte code);
int vm_get_src_type(byte code);
uint vm_get_register_value(VM *vm, byte register_code);
uint vm_get_value_at_offset(VM *vm, uint address);
uint vm_get_value_from_code(VM *vm, uint address);
int vm_get_src_value(VM *vm, int relative_offset, uint *value);
int vm_set_register_value(VM *vm, byte register_code, uint value);
int vm_set_value_at_offset(VM *vm, int access, uint address, uint value);

//
int vm_code_nop(VM *vm);
int vm_code_mov(VM *vm);
int vm_code_add(VM *vm);
int vm_code_sub(VM *vm);
int vm_code_mul(VM *vm);
int vm_code_div(VM *vm);
int vm_code_mod(VM *vm);
int vm_code_and(VM *vm);
int vm_code_or(VM *vm);
int vm_code_xor(VM *vm);
int vm_code_shl(VM *vm);
int vm_code_shr(VM *vm);
int vm_code_inc(VM *vm);
int vm_code_dec(VM *vm);
int vm_code_push(VM *vm);
int vm_code_pop(VM *vm);
int vm_code_pusha(VM *vm);
int vm_code_popa(VM *vm);
int vm_code_cmp(VM *vm);
int vm_code_jmp(VM *vm);
int vm_code_jl(VM *vm);
int vm_code_ja(VM *vm);
int vm_code_je(VM *vm);
int vm_code_jne(VM *vm);
int vm_code_jae(VM *vm);
int vm_code_jle(VM *vm);
int vm_code_call(VM *vm);
int vm_code_ret(VM *vm);

//EOF
