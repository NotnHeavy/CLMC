#pragma once
#include <stdint.h>
#include "machine.h"

typedef enum
{
	IDENTIFIER,
	NUMERIC,
	NEWLINE,
	EoF
} lmctokens_t;

typedef struct
{
	lmctokens_t token;
	char value[LMC_LABEL_SIZE + 1]; // only to fit instructions and numbers ranging -999 to 999.
} lmctoken_t;

typedef struct
{
	lmctoken_t* currentToken;
	size_t codeLength;
	char* code;
	char* codePointer;
	char errorState[256];
} lmcassembler_t;

typedef struct
{
	int16_t* assembly;
	size_t length;
} lmccode_t;

void FreeLMCASsembly(lmccode_t* assembly);

void FreeLMCAssemblerCode(lmcassembler_t* assembler);
void CleanLMCAssembler(lmcassembler_t* assembler);
void SetLMCAssemblerCode(lmcassembler_t* assembler, char* code, size_t length);
lmccode_t* AssembleLMC(lmcassembler_t* assembler);
lmcassembler_t* CreateLMCAssembler(char* code, size_t length);