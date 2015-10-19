
#include "compiler_includes.h"
#include "compiler_typedefs.h"
#include "compiler_structs.h"
#include "compiler_enums.h"

#ifndef _COMPILER_H
#define _COMPILER_H

namespace VM {

class Compiler
{
protected:
	std::map<std::string, LabelType*> labels;
	std::vector<CodeType*> codes;
	std::vector<byte> sourcecode;
	std::vector<byte> bytecodes;
	int codeOffset;

	int addLabel(std::string label, int address = 0);
	int addLabelOffset(std::string label, uint offset = 0);
	int addBytecode(byte bytecode);
	int addBytecodes(byte* bytecode, uint length = 1);
	int addCode(CodeType *code);

	int __commentLine(int offset);
	int __foundAscii(int offset);
	int __foundDot(int offset);
	int __isMnemonic(char *text);
	int __isRegister(char *text);
	int __isLabel(std::string text);
	int __isConstant(char *text);
	int __convertToInt(char *text);
	int __isMeaningfulCharacter(byte c);
	int __isMeaningfulString(char *text);
	int __getString(int offset, char **text);
	int __tolower(char *text);

public:
	Compiler();
	~Compiler();

	int load(std::ifstream *in);
	int convert(void);
	int write(std::ostream *out);

	int compile(std::ifstream *in, std::ostream *out);
}; //class Compiler

}; //namespace VM

#endif

//EOF
