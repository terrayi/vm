
#define LONG_VERSION 1

#include <stdio.h>
#include "vm_common.h"

int main(int argc, char **argv)
{
	VM vm;
	FILE *fp = 0;
	FILE *fp2 = 0;
	char *code = 0;
	size_t length = 0;
	int i = 0;

	fp = fopen(argv[1], "rb");
	fseek(fp, 0, SEEK_END);
	length = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (!length)
	{
		return 1;
	}

	code = (char*) malloc(length);
	fread(code, 1, length, fp);
	fclose(fp);

	#ifdef LONG_VERSION
	vm_init(&vm);

	if (vm_load(&vm, code, length) != VM_RET_OK)
	{
		return 1;
	}

	vm_run(&vm);

	for (; i < 10; i++)
	{
		printf("Register%d: 0x%08x\n", i, vm.Registers[i].val);
	}

	if (argc >= 3)
	{
		fp2 = fopen(argv[2], "wb");
		fwrite(vm.Code, 1, vm.CodeLength, fp2);
		fclose(fp2);
	}

	vm_uninit(&vm);
	#else
	if (vm_quick_run(&vm, code, length) != VM_RET_OK)
	{
		printf("code run error\n");
		return 1;
	}
	#endif

	free(code);

	return 0;
}

//EOF
