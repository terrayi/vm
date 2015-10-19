
#include <fstream>
#include "compiler.h"

void usage(void);

int main(int argc, char **argv)
{
	VM::Compiler compiler;
	std::ifstream in;
	std::ofstream out;
	int ret = VM::COMPILER_OK;

	if (argc < 3)
	{
		usage();
		return 1;
	}

	std::cout << "open input: " << argv[1] << std::endl;

	in.open(argv[1], std::fstream::in);
	out.open(argv[2], std::fstream::out | std::fstream::binary);

	std::cout << "compiling..." << std::endl;

	ret = compiler.compile(&in, &out);

	//std::cout << "returned from compiling..." << std::endl;

	if (ret != VM::COMPILER_OK)
	{
		std::cout << "compile error" << std::endl;
		return 1;
	}

	return 0;
}

void usage(void)
{
	std::cout
		<< "Usage:" << std::endl
		<< "\tCompiler src dest" << std::endl;
}

//EOF
