
#ifndef _COMPILER_MNEMONIC_H
#define _COMPILER_MNEMONIC_H

namespace VM {

CodeDefType codeDefs[] = {
	{"nop", NOP, 0, 0},
	{"mov", MOV, 1, 1},
	{"add", ADD, 1, 1},
	{"sub", SUB, 1, 1},
	{"mul", MUL, 1, 1},
	{"div", DIV, 1, 1},
	{"mod", MOD, 1, 1},
	{"and", AND, 1, 1},
	{"or", OR, 1, 1},
	{"xor", XOR, 1, 1},
	{"shl", SHL, 1, 1},
	{"shr", SHR, 1, 1},
	{"inc", INC, 1, 0},
	{"dec", DEC, 1, 0},
	{"pop", POP, 1, 0},
	{"push", PUSH, 1, 0},
	{"popa", POPA, 0, 0},
	{"pusha", PUSHA, 0, 0},
	{"cmp", CMP, 1, 1},
	{"jmp", JMP, 1, 0},
	{"jl", JL, 1, 0},
	{"ja", JA, 1, 0},
	{"je", JE, 1, 0},
	{"jne", JNE, 1, 0},
	{"jae", JAE, 1, 0},
	{"jle", JLE, 1, 0},
	{"call", CALL, 1, 0},
	{"ret", RET, 0, 0}
};

}; //namespace

#endif

//EOF
