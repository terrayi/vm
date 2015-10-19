
#ifndef _COMPILER_STRUCTS_H
#define _COMPILER_STRUCTS_H

namespace VM {

struct LabelStruct
{
	uint Address;
	std::vector<uint> Offsets;

	LabelStruct()
	{
	};

	~LabelStruct()
	{
		Offsets.clear();
	};
};

typedef struct LabelStruct LabelType;

struct CodeDefStruct
{
	std::string Mnemonic;
	int Code;
	int HasSourceArgument;
	int HasTargetArgument;
};

typedef struct CodeDefStruct CodeDefType;

struct CodeStruct
{
	int Type;
	byte Opcode;
	uint Address;
	int SourceType;
	byte SourceRegister;
	uint SourceValue;
	std::string SourceLabel;
	int TargetType;
	byte TargetRegister;
	byte FillByte;
	int FillSize;
	std::vector<byte> Data;
};

typedef struct CodeStruct CodeType;

}; //namespace VM

#endif

//EOF
