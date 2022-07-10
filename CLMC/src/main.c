#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assembler.h"
#include "machine.h"

int main()
{
	printf("NotnHeavy's Little Man's Computer virtual machine.\n");
	lmc_t* machine = CreateLMC();
	lmcassembler_t* assembler = CreateLMCAssembler(NULL, 0);
	char path[_MAX_PATH];
	for (;;)
	{
		// Read input.
		ResetLMC(machine);
		printf("Input .lmc ASM:\n>>> ");
		fgets(path, sizeof(path), stdin);
		path[strlen(path) - 1] = '\0';

		// Open file.
		FILE* file;
		if (fopen_s(&file, path, "rb") != 0)
		{
			printf("This file does not exist on your computer.\n");
			continue;
		}
		_fseeki64(file, 0, SEEK_END);
		size_t size = _ftelli64(file);
		char* contents = (char*)malloc(sizeof(char) * size);
		if (contents == NULL)
		{
			printf("Could not malloc file contents buffer.\n");
			continue;
		}
		_fseeki64(file, 0, SEEK_SET);
		fread(contents, size, sizeof(char), file);
		fclose(file);

		// Run VM.
		SetLMCAssemblerCode(assembler, contents, size);
		lmccode_t* code = AssembleLMC(assembler);
		if (code == NULL)
		{
			printf("The assembler has thrown an error: %s\n", assembler->errorState[0] == '\0' ? "An internal error has occurred." : assembler->errorState);
			continue;
		}
		LoadIntoLMC(machine, code->assembly, code->length);
		ExecuteLMC(machine);
		FreeLMCASsembly(code);
		printf("\n");
	}

	return 0;
}