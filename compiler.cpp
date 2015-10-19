
#include "compiler.h"
#include "compiler_mnemonics.h"
#include "ascii.h"

#ifdef DEBUG
	#define LOG printf
#else
	#define LOG(...) ;
#endif

#define IS_A_TO_Z(c) (c >= ASCII_A && c <= ASCII_Z)
#define IS_a_TO_z(c) (c >= ASCII_a && c <= ASCII_z)
#define IS_HEX(c) ((c >= ASCII_A && c <= ASCII_F) || (c >= ASCII_a && c <= ASCII_f))
#define IS_ALPHA(c) (IS_A_TO_Z(c) || IS_a_TO_z(c))
#define IS_DIGIT(c) (c >= ASCII_0 && c <= ASCII_9)
#define IS_ALNUM(c) (IS_ALPHA(c) || IS_DIGIT(c))
#define IS_WHITESPACE(c) (c == ASCII_CR || c == ASCII_LF || c == ASCII_SPACE || c == ASCII_TAB)
#define IS_SPACE(c) (c == ASCII_SPACE || c == ASCII_TAB)
#define TOLOWER(c) (IS_A_TO_Z(c) ? c + 0x20 : c)
#define CTOH(c) (TOLOWER(c) - 0x57)

namespace VM {

Compiler::Compiler()
{
	codeOffset = 0;
}

Compiler::~Compiler()
{
}

int
Compiler::load(std::ifstream *in)
{
	//typedef std::istream_iterator<byte> iterator;

	int length = -1;
	int offset = 0;
	int ret = 0;
	byte c = 0;

	//LOG("check open state\n");
	if (!in->is_open())
	{
		LOG("file is not opened\n");
		std::cout << "file is not opened" << std::endl;
		return LOAD_OPEN_ERROR;
	}

	//LOG("seek to the end of file\n");
	in->seekg(0, std::ios::end);

	if (in->fail() || in->bad())
	{
		LOG("failed to seek eof\n");
		std::cout << "corrupted or empty file" << std::endl;
		return LOAD_SEEK_ERROR;
	}

	//LOG("getting size of the file\n");
	length = in->tellg();

	if (length == -1)
	{
		LOG("failed\n");
		std::cout << "file might be corrupted" << std::endl;
		return LOAD_TELL_ERROR;
	}

	//LOG("resizing buffer to store content\n");
	sourcecode.resize(length);

	//LOG("seek to the beginning of the file\n");
	in->seekg(0, std::ios::beg);

	if (in->fail() || in->bad())
	{
		LOG("failed\n");
		std::cout << "file might be modified" << std::endl;
		return LOAD_SEEK_ERROR;
	}

	//LOG("read in content of the file\n");
	in->read((char*) &sourcecode[0], length);

	if (in->fail() || in->bad())
	{
		LOG("failed\n");
		return LOAD_READ_ERROR;
	}

	//
	//LOG("analyse content of the file\n");
	LOG("file size: %d (%d)\n", length, (int) sourcecode.size());

	while (offset < length)
	{
		c = sourcecode[offset];
		//printf("0x%02x\n", c);

		switch (c) {
		case ASCII_SHARP:
		{
			//LOG("comment line\n");
			ret = __commentLine(offset);
			offset += ret;

			break;
		}
		case ASCII_LF:
		case ASCII_CR:
		case ASCII_TAB:
		case ASCII_SPACE:
		{
			offset++;
			break;
		}
		case ASCII_DOT:
		{
			//.data and .fill
			//LOG("found dot\n");
			ret = __foundDot(offset);

			if (ret == EXCEPTION)
			{
				return EXCEPTION;
			}

			offset += ret;
			break;
		}
		default:
		{
			if ((c >= ASCII_A && c <= ASCII_Z)
			|| (c >= ASCII_a && c <= ASCII_z))
			{
				//LOG("ascii character found\n");
				ret = __foundAscii(offset);

				if (ret == EXCEPTION)
				{
					return EXCEPTION;
				}

				offset += ret;
			}
			else
			{
				offset++;
				LOG("Unknown.. 0x%02x\n", c);
				return EXCEPTION;
			}
			break;
		}
		}
	}

	return COMPILER_OK;
}

int
Compiler::convert(void)
{
	std::vector<CodeType*>::iterator it;
	std::vector<byte>::iterator biter;
	std::string label;
	int i;

	//fix label offset
	for (it = codes.begin(); it != codes.end(); it++)
	{
		if ((*it)->Type == DATA || (*it)->Type == FILL)
		{
			continue;
		}

		if ((*it)->SourceType != LABEL_OFFSET
		&& (*it)->SourceType != LABEL)
		{
			continue;
		}

		label = (*it)->SourceLabel;

		if (!__isLabel(label))
		{
			LOG("label '%s' not found\n", label.c_str());
			std::cout << "label (" << label << ") is not found"
				<< std::endl;
			return EXCEPTION;
		}

		if ((*it)->SourceType == LABEL_OFFSET)
		{
			(*it)->SourceType = CONSTANT_OFFSET;
		}
		else if ((*it)->SourceType == LABEL)
		{
			(*it)->SourceType = CONSTANT;
		}

		(*it)->SourceValue = labels[label]->Address;
	}

	//now produce bytecodes
	for (it = codes.begin(); it != codes.end(); it++)
	{
		CodeType *code = (*it);

		if (code->Type == DATA)
		{
			//LOG(".Data\n");
			for (biter = code->Data.begin();
				biter != code->Data.end(); biter++)
			{
				addBytecode((*biter));
			}

			continue;
		}
		else if (code->Type == FILL)
		{
			//LOG(".Fill: %d\n", code->FillSize);
			for (i = 0; i < code->FillSize; i++)
			{
				addBytecode(code->FillByte);
			}

			continue;
		}

		addBytecode(code->Opcode);

		if (codeDefs[code->Type].HasTargetArgument)
		{
			addBytecode(code->TargetRegister);
		}

		if (!codeDefs[code->Type].HasSourceArgument)
		{
			continue;
		}

		switch (code->SourceType) {
		case REGISTER:
		case REGISTER_OFFSET:
		{
			addBytecode(code->SourceRegister);
			break;
		}
		case CONSTANT:
		case CONSTANT_OFFSET:
		{
			addBytecodes((byte*) &code->SourceValue, 4);
			break;
		}
		default:
		{
			LOG("Incorrect source argument type\n");
			return EXCEPTION;
		}
		}
	}

	return COMPILER_OK;
}

int
Compiler::write(std::ostream *out)
{
	out->write((char*) &bytecodes[0], bytecodes.size());

	return COMPILER_OK;
}

int
Compiler::compile(std::ifstream *in, std::ostream *out)
{
	int ret = COMPILER_OK;

	LOG("load(in)\n");
	ret = load(in);

	if (ret != COMPILER_OK)
	{
		return ret;
	}

	LOG("convert()\n");
	ret = convert();

	if (ret != COMPILER_OK)
	{
		return ret;
	}

	LOG("write()\n");
	ret = write(out);

	if (ret != COMPILER_OK)
	{
		return ret;
	}

	return COMPILER_OK;
}

int
Compiler::addLabel(std::string label, int address)
{
	LabelType *newLabel = 0;

	if (labels.find(label) == labels.end())
	{
		newLabel = new LabelType;

		labels.insert(
			std::pair<std::string, LabelType*>(label, newLabel));
	}

	labels[label]->Address = address;

	return 1;
}

int
Compiler::addLabelOffset(std::string label, uint offset)
{
	LabelType *newLabel = 0;

	if (labels.find(label) == labels.end())
	{
		newLabel = new LabelType;

		labels.insert(
			std::pair<std::string, LabelType*>(label, newLabel));
	}

	labels[label]->Offsets.push_back(offset);

	return 1;
}

int
Compiler::addBytecode(byte bytecode)
{
	bytecodes.push_back(bytecode);
}

int
Compiler::addBytecodes(byte* bytecode, uint length)
{
	uint i;

	for (i = 0; i < length; i++)
	{
		bytecodes.push_back(bytecode[i]);
	}
}

int
Compiler::addCode(CodeType *code)
{
	codes.push_back(code);
}

int
Compiler::__commentLine(int offset)
{
	#ifdef DEBUG
		char *comment = 0;
		int text_length = 0;
	#endif

	int length = 1;
	byte c = sourcecode[offset + length];

	while (c != ASCII_CR && c != ASCII_LF)
	{
		length++;

		if (offset + length >= sourcecode.size())
		{
			goto comment_ends;
		}

		c = sourcecode[offset + length];
	}

	#ifdef DEBUG
		text_length = length - 1;
	#endif

	while (c == ASCII_CR || c == ASCII_LF)
	{
		length++;

		if (offset + length >= sourcecode.size())
		{
			goto comment_ends;
		}

		c = sourcecode[offset + length];
	}

comment_ends:
	#ifdef DEBUG
		comment = new char[text_length + 1];
		memcpy(comment, &sourcecode[offset + 1], text_length);
		comment[text_length] = 0;
	#endif
	LOG("comment (%d): '%s'\n", text_length, comment);
	#ifdef DEBUG
		delete comment;
	#endif

	return length;
}

int
Compiler::__foundAscii(int offset)
{
	CodeType *codeType = 0;
	char *text = 0;
	int text_length = 0;
	int length = 0;
	int ret = 0;
	byte bytecode = 0;
	byte c = sourcecode[offset];
	bool offsetArgument = false;

	while ((c >= ASCII_A && c <= ASCII_Z)
		|| (c >= ASCII_a && c <= ASCII_z)
		|| (c >= ASCII_0 && c <= ASCII_9)
		|| c == ASCII_UNDERSCORE)
	{
		c = sourcecode[offset + ++length];
	}

	text = new char[length + 1];
	memcpy(text, &sourcecode[offset], length);
	text[length] = 0;
	//LOG("text (%d): '%s'\n", length, text);
	
	if (c == ASCII_COLON)
	{
		//found label
		LOG("label (%d): '%s'\n", length, text);
		//add label
		addLabel(text, codeOffset);
		delete text;
		return length + 1;
	}

	ret = __isMnemonic(text);

	if (ret == -1)
	{
		LOG("Invalid text? (%s)\n", text);
		std::cout << "Invalid: " << text << std::endl;
		return length;
	}

	//found mnemonic
	LOG("mnemonic (%d): '%s'\n", length, text);
	delete text;
	bytecode = codeDefs[ret].Code;
	codeType = new CodeType;
	codeType->Type = ret;
	codeType->Address = codeOffset++;

	if (codeDefs[ret].HasTargetArgument)
	{
		int r = 0;

		//LOG("has target argument\n");
		c = sourcecode[offset + length];

		while (!__isMeaningfulCharacter(c))
		{
			c = sourcecode[offset + ++length];
		}

		codeType->TargetType = REGISTER;

		if (c == ASCII_AT)
		{
			offsetArgument = true;
			length++;
			bytecode |= 0x80;
			codeType->TargetType = REGISTER_OFFSET;
		}

		//LOG("expect a string\n");
		r = __getString(offset + length, &text);

		if (r == 0)
		{
			LOG("target argument expected\n");
			delete codeType;
			std::cout << "target argument was expected"
				<< std::endl;
			return EXCEPTION;
		}

		//LOG("expect a register target argument\n");
		length += r;
		r = __isRegister(text);

		if (r == -1)
		{
			LOG("Invalid target argument\n");
			delete codeType;
			std::cout << "invalid target argument: " << text
				<< std::endl;
			return EXCEPTION;
		}

		LOG("isRegister(%s) = 0x%08x\n", text, r);
		delete text;
		codeType->TargetRegister = (byte) r;

		//LOG("expect a comma\n");
		c = sourcecode[offset + length];
		codeOffset++;

		while (IS_WHITESPACE(c))
		{
			c = sourcecode[offset + ++length];
		}

		if (c != ASCII_COMMA)
		{
			LOG("comma was expected but not exists\n");
			delete codeType;
			std::cout << "comma(,) was expected" << std::endl;
			return EXCEPTION;
		}
	}

	offsetArgument = false;

	if (codeDefs[ret].HasSourceArgument)
	{
		int r = 0;

		//LOG("has source argument\n");
		c = sourcecode[offset + length];

		while (!__isMeaningfulCharacter(c))
		{
			c = sourcecode[offset + ++length];
		}

		if (c == ASCII_AT)
		{
			offsetArgument = true;
			length++;
			bytecode |= 0x20;
		}

		r = __getString(offset + length, &text);

		if (r == 0)
		{
			LOG("source argument expected");
			delete codeType;
			std::cout << "source argument was expected"
				<< std::endl;
			return EXCEPTION;
		}

		length += r;

		if ((r = __isRegister(text)) != -1)
		{
			//LOG("register source argument\n");
			if (offsetArgument)
			{
				codeType->SourceType = REGISTER_OFFSET;
			}
			else
			{
				codeType->SourceType = REGISTER;
			}

			codeType->SourceRegister = r;
			LOG("isRegister(%s) = 0x%08x\n", text, r);
			codeOffset++;
		}
		else if (0 < (r = __isConstant(text)))
		{
			int v;

			v = __convertToInt(text);
			LOG("constant source argument: %d (%s)\n", v, text);
			codeType->SourceType = CONSTANT;

			if (offsetArgument)
			{
				codeType->SourceType = CONSTANT_OFFSET;
			}
			else
			{
				codeType->SourceType = CONSTANT;
			}

			codeType->SourceValue = v;
			codeOffset += 4;
			bytecode |= 0x40;
		}
		else if (__isMeaningfulString(text))
		{
			LOG("assume label source argument: '%s'\n", text);

			if (offsetArgument)
			{
				codeType->SourceType = LABEL_OFFSET;
			}
			else
			{
				codeType->SourceType = LABEL;
			}

			codeType->SourceLabel = std::string(text);
			codeOffset += 4;
			bytecode |= 0x40;
			addLabelOffset(text, codeType->Address);
		}
		else
		{
			LOG("Invalid source argument\n");
			std::cout << "invalid source argument: " << text
				<< std::endl;
			delete text;
			delete codeType;
			return EXCEPTION;
		}

		delete text;
	}

	codeType->Opcode = bytecode;
	addCode(codeType);

	return length;
}

int
Compiler::__foundDot(int offset)
{
	CodeType *code;
	char *text = 0;
	int length = 0;
	int ret = 0;
	byte fb;
	byte c;

	ret = __getString(offset + 1, &text);

	if (ret < 1)
	{
		LOG("command was expected but not found\n");
		std::cout << "invalid dot(.) found" << std::endl;
		return EXCEPTION;
	}
	else if (ret != 4)
	{
		LOG("Unexpected command '%s'\n", text);
		std::cout << "invalid dot(.) found" << std::endl;
		return EXCEPTION;
	}

	__tolower(text);
	length = ret + 1;
	//LOG(".%s\n", text);

	if (strcmp(text, "data") == 0)
	{
		ret = 1;
		code = new CodeType;
		code->Type = DATA;

		while (ret)
		{
			c = sourcecode[offset + length];

			if (c == ASCII_CR || c == ASCII_LF)
			{
				break;
			}

			//LOG("skipping space characters\n");
			while (IS_SPACE(c))
			{
				c = sourcecode[offset + ++length];
			}

			ret = __getString(offset + length, &text);

			if (ret < 1)
			{
				break;
			}
			else if (__isConstant(text))
			{
				fb = (byte) __convertToInt(text);
				length += ret;
			}
			else if (IS_ALPHA(c))
			{
				fb = c;
				length++;
			}
			else
			{
				LOG("Unexpected character: 0x%02x\n", c);
				return EXCEPTION;
			}

			//LOG("adding 0x%02x\n", fb);
			code->Data.push_back(fb);
			codeOffset++;
		}
	}
	else if (strcmp(text, "fill") == 0)
	{
		int fill = 0;

		c = sourcecode[offset + length];
		code = new CodeType;
		code->Type = FILL;
		code->Address = codeOffset;

		while (IS_SPACE(c))
		{
			c = sourcecode[offset + ++length];
		}

		ret = __getString(offset + length, &text);

		if (ret < 1)
		{
			LOG("two arguments was expected but not found\n");
			delete code;
			return EXCEPTION;
		}

		if (!__isConstant(text))
		{
			LOG("a byte to fill was expected but not found\n");
			delete code;
			return EXCEPTION;
		}

		fb = (byte) __convertToInt(text);
		length += ret;
		delete text;
		text = 0;
		c = sourcecode[offset + length];
		//LOG("0x%02x ", fb);

		while (IS_SPACE(c))
		{
			c = sourcecode[offset + ++length];
		}

		ret = __getString(offset + length, &text);

		if (ret < 1)
		{
			LOG("another argument was expected but not found\n");
			delete code;
			return EXCEPTION;
		}

		if (!__isConstant(text))
		{
			LOG("Invalid argument #2\n");
			return EXCEPTION;
		}

		//LOG("%s\n", text);
		fill = __convertToInt(text);

		if (fill < 1)
		{
			LOG("Invalid argument #2\n");
			delete code;
			return EXCEPTION;
		}

		code->FillByte = fb;
		code->FillSize = fill;
		codeOffset += fill;
		//LOG("fill 0x%02x for %d times\n", fb, fill);
		length += ret;
	}
	else
	{
		LOG("Unexpected command '%s'\n", text);
		std::cout << "Unexpected: " << text << std::endl;
		return EXCEPTION;
	}

	addCode(code);
	return length;
}

int
Compiler::__isMnemonic(char *text)
{
	int i;

	//LOG("Compiler::__isMnemonic()\n");

	__tolower(text);

	for (i = 0; i < 28; i++)
	{
		//LOG("%s equals %s ? ", text, codeDefs[i].Mnemonic.c_str());
		if (strcmp(codeDefs[i].Mnemonic.c_str(), text) == 0)
		{
			//LOG("TRUE\n");
			return i;
		}

		//LOG("FALSE\n");
	}

	return -1;
}

int
Compiler::__isRegister(char *text)
{
	byte c;
	int length = strlen(text);
	int reg_type = REG_WHOLE;

	__tolower(text);

	if (length < 4)
	{
		//LOG("unexpected length\n");
		return -1;
	}

	if (memcmp(text, "reg", 3) != 0)
	{
		//LOG("expected 'reg' but mismatch\n");
		return -1;
	}

	if (!IS_DIGIT(text[3]))
	{
		LOG("expected register number but not found\n");
		return -1;
	}

	c = (byte) ((text[3] - 0x30) & 0x0f);

	if (length == 5)
	{
		if (text[4] == 'l')
		{
			//LOG("Register%d L\n", c);
			reg_type = REG_L;
		}
		else if (text[4] == 'h')
		{
			//LOG("Register%d H\n", c);
			reg_type = REG_H;
		}
		else
		{
			LOG("Incorrect register access\n");
			return -1;
		}
	}
	else if (length == 6)
	{
		if (memcmp(&text[4], "ll", 2) == 0)
		{
			//LOG("Register%d LL\n", c);
			reg_type = REG_LL;
		}
		else if (memcmp(&text[4], "lh", 2) == 0)
		{
			//LOG("Register%d LH\n", c);
			reg_type = REG_LH;
		}
		else if (memcmp(&text[4], "hl", 2) == 0)
		{
			//LOG("Register%d HL\n", c);
			reg_type = REG_HL;
		}
		else if (memcmp(&text[4], "hh", 2) == 0)
		{
			//LOG("Register%d HH\n", c);
			reg_type = REG_HH;
		}
		else
		{
			LOG("Incorrect register access\n");
			return -1;
		}
	}
	else if (length != 4)
	{
		return -1;
	}

	if (reg_type != REG_WHOLE)
	{
		c |= ((((byte) reg_type) << 4) & 0xf0);
	}

	return (int) c;
}

int
Compiler::__isLabel(std::string text)
{
	if (labels.find(text) == labels.end())
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int
Compiler::__isConstant(char *text)
{
	int i = 0;
	int length = strlen(text);
	bool hex = false;

	if (length > 1
	&& text[0] == ASCII_MINUS || text[0] == ASCII_X || text[0] == ASCII_x)
	{
		i = 1;
	}

	if (length > 2 && text[0] == ASCII_0
	&& (text[1] == ASCII_X || text[1] == ASCII_x))
	{
		i = 2;
		hex = true;
	}

	for (; i < strlen(text); i++)
	{
		if (hex && IS_HEX(text[i]))
		{
			continue;
		}

		if (!IS_DIGIT(text[i]))
		{
			return 0;
		}
	}

	if (hex)
	{
		return 2;
	}
	else
	{
		return 1;
	}
}

int
Compiler::__convertToInt(char *text)
{
	int i = 0;
	int j = 0;
	int bit = 0;
	int value = 0;
	int length = strlen(text);

	if (__isConstant(text) == 2)
	{
		i = length - 1;

		if (i - 8 > 0)
		{
			j = i - 8;
			LOG("value will be fitted into integer type\n");
		}

		for (; i >= j; i--)
		{
			if (text[i] == ASCII_X || text[i] == ASCII_x
			|| bit > 28)
			{
				break;
			}

			if (IS_HEX(text[i]))
			{
				value |= ((CTOH(text[i]) & 0x0f) << bit);
			}
			else if (text[i] != ASCII_0)
			{
				value |= (((text[i] - 0x30) & 0x0f) << bit);
			}

			//LOG("%c (%02x) => %08x\n", text[i], text[i], value);
			bit += 4;
		}

		return value;
	}
	else
	{
		//LOG("'%s' -> %d\n", text, atoi(text));
		return atoi(text);
	}
}

int
Compiler::__isMeaningfulCharacter(byte c)
{
	if (IS_ALNUM(c) || c == ASCII_AT || c == ASCII_UNDERSCORE
	|| c == ASCII_MINUS)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int
Compiler::__isMeaningfulString(char *text)
{
	int i;

	for (i = 0; i < strlen(text); i++)
	{
		if (!IS_ALNUM(text[i]) && text[i] != ASCII_UNDERSCORE)
		{
			return 0;
		}
	}

	return 1;
}

int
Compiler::__getString(int offset, char **text)
{
	int length = 0;
	byte c = sourcecode[offset];

	while (IS_ALNUM(c) || c == ASCII_UNDERSCORE || c == ASCII_MINUS)
	{
		c = sourcecode[offset + ++length];
	}

	if (length > 0)
	{
		//LOG("there is a string of length %d\n", length);
		*text = new char[length + 1];
		memcpy(*text, &sourcecode[offset], length);
		(*text)[length] = 0;
		//LOG("text (%d): %s\n", length, *text);
	}

	return length;
}

int
Compiler::__tolower(char *text)
{
	int i;

	for (i = 0; i < strlen(text); i++)
	{
		if (IS_A_TO_Z(text[i]))
		{
			text[i] += 0x20;
		}
	}

	return 1;
}

}; //namespace VM

//EOF
