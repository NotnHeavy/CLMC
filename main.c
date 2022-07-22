#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assembler.h"
#include "machine.h"

int main()
{
	puts("NotnHeavy's Little Man's Computer virtual machine.");
	lmc_t* machine = CreateLMC();
	lmcassembler_t* assembler = CreateLMCAssembler(NULL, 0);
	char path[4096];
	for (;;)
	{
		// Read input.
		ResetLMC(machine);
		fputs("Input .lmc ASM:\n>>> ", stdout);
		fgets(path, sizeof(path), stdin);
		path[strlen(path) - 1] = '\0';

		// Open file.
		FILE* file = fopen(path, "r");
		if (file == NULL)
		{
			puts("This file does not exist on your computer.");
			continue;
		}
		fseek(file, 0, SEEK_END);
		size_t size = ftell(file);
		char* contents = (char*)malloc(sizeof(char) * size);
		if (contents == NULL)
		{
			puts("Could not malloc file contents buffer.");
			continue;
		}
		fseek(file, 0, SEEK_SET);
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
		putchar('\n');
	}

	return 0;
}
