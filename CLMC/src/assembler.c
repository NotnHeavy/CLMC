#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "machine.h"
#include "assembler.h"

// DAT is a bit of a special case. It's not an instruction that gets compiled down, 
// unlike the rest, it just reserves the next mailbox with an optional value. 
// Typically this is used with labels.
static const char instructions[11][4] = {"HLT", "ADD", "SUB", "STA", "DAT", "LDA", "BRA", "BRZ", "BRP", "INP", "OUT"};

typedef struct
{
	char label[LMC_LABEL_SIZE + 1];
	uint8_t index;
} lmclabel_t;

typedef struct
{
	char instruction;
	char label[LMC_LABEL_SIZE + 1];
} lmclabelref_t;

static inline int inBounds(lmcassembler_t* assembler)
{
	if (assembler == NULL)
		return 0;

	return assembler->codePointer != assembler->code + assembler->codeLength;
}

static inline char peak(lmcassembler_t* assembler)
{
	if (assembler == NULL)
		return 0;

	return assembler->codePointer + 1 != assembler->code + assembler->codeLength ? *(assembler->codePointer + 1) : 0;
}

static char isInstruction(const char* const string)
{
	if (string == NULL)
		return -1;
	char upper[LMC_LABEL_SIZE + 1] = { 0 };
	size_t index = 0;
	strcpy_s(upper, sizeof(upper), string);
	while (upper[index] != '\0')
	{
		upper[index] = toupper(upper[index]);
		++index;
	}
	for (char i = 0; i < sizeof(instructions) / sizeof(instructions[0]); ++i)
	{
		if (strcmp(upper, instructions[i]) == 0)
			return i;
	}
	return -1;
}

static int isNumber(const char* number, size_t size)
{
	if (number == NULL)
		return 0;
	if (size == 0)
		size = strlen(number);
	for (size_t i = 0; i < size; ++i)
	{
		if (!isdigit(number[i]))
			return 0;
	}
	return 1;
}

static lmctoken_t* getNextToken(lmcassembler_t* assembler)
{
	if (assembler == NULL)
		return NULL;

	lmctoken_t* token = (lmctoken_t*)malloc(sizeof(lmctoken_t));
	if (token == NULL)
		return NULL;

	if (assembler->currentToken != NULL)
	{
		free(assembler->currentToken);
		assembler->currentToken = NULL;
	}

	while (inBounds(assembler))
	{
		char character = *assembler->codePointer;

		// Comments/ignored code.
		if (character == '/' && peak(assembler) == '/')
		{
			while (inBounds(assembler) && *assembler->codePointer != '\n')
				++assembler->codePointer;
			while (inBounds(assembler) && isspace(*assembler->codePointer))
				++assembler->codePointer;
			continue;
		}

		// Skip spaces.
		if (isspace(character) && *assembler->codePointer != '\n')
		{
			while (inBounds(assembler) && isspace(*assembler->codePointer) && *assembler->codePointer != '\n')
				++assembler->codePointer;
			continue;
		}

		// Newlines.
		else if (character == '\n')
		{
			++assembler->codePointer;
			token->token = NEWLINE;
			strcpy_s(token->value, sizeof(token->value), "\n");
			assembler->currentToken = token;
			return token;
		}

		// Identifier or numeric consts.
		else if (isalnum(character) || character == '-')
		{
			char buffer[LMC_LABEL_SIZE + 1] = { 0 };
			int index = 0;
			while (inBounds(assembler) && ((character == '-' && (isdigit(*assembler->codePointer) || *assembler->codePointer == '-')) || isalnum(*assembler->codePointer)) && index < sizeof(buffer) - 1)
			{
				buffer[index] = *assembler->codePointer;
				++index;
				++assembler->codePointer;
			}

			if (isNumber(buffer, (size_t)(index - 1)))
				token->token = NUMERIC;
			else
				token->token = IDENTIFIER;
			strcpy_s(token->value, sizeof(token->value), buffer);
			assembler->currentToken = token;
			return token;
		}
		
		free(token);
		sprintf_s(assembler->errorState, sizeof(assembler->errorState), "Unrecognised token '%c'.", character);
		return NULL;
	}

	assembler->currentToken = token;
	token->token = EoF;
	strcpy_s(token->value, sizeof(token->value), "EOF");
	return token;
}

static int verifyToken(lmcassembler_t* assembler, lmctokens_t token)
{
	if (assembler == NULL || assembler->currentToken == NULL)
		return 1;

	if (assembler->currentToken->token != token)
	{
		sprintf_s(assembler->errorState, sizeof(assembler->errorState), "Unexpected token \"%s\".", assembler->currentToken->value);
		return 1;
	}
	return 0;
}

static int verifyTokenList(lmcassembler_t* assembler, lmctokens_t tokens[], size_t length)
{
	if (assembler == NULL || assembler->currentToken == NULL)
		return 1;

	for (size_t i = 0; i < length; ++i)
	{
		if (assembler->currentToken->token == tokens[i])
			return 0;
	}
	sprintf_s(assembler->errorState, sizeof(assembler->errorState), "Unexpected token \"%s\".", assembler->currentToken->value);
	return 1;
}

void FreeLMCASsembly(lmccode_t* assembly)
{
	if (assembly == NULL)
		return;
	free(assembly->assembly);
	free(assembly);
}

void FreeLMCAssemblerCode(lmcassembler_t* assembler)
{
	if (assembler == NULL)
		return;

	free(assembler->code);
	assembler->code = NULL;
	assembler->codePointer = NULL;
	assembler->codeLength = 0;
}

void CleanLMCAssembler(lmcassembler_t* assembler)
{
	if (assembler == NULL)
		return;

	if (assembler->currentToken != NULL)
	{
		free(assembler->currentToken);
		assembler->currentToken = NULL;
	}

	if (assembler->code != NULL)
		assembler->codePointer = assembler->code;

	memset(assembler->errorState, 0, sizeof(assembler->errorState));
}

void SetLMCAssemblerCode(lmcassembler_t* assembler, char* code, size_t length)
{
	if (assembler == NULL || code == NULL)
		return;

	CleanLMCAssembler(assembler);
	assembler->code = code;
	assembler->codePointer = assembler->code;
	if (length <= 0)
		assembler->codeLength = strlen(code);
	else
		assembler->codeLength = length;
}

lmccode_t* AssembleLMC(lmcassembler_t* assembler)
{
	if (assembler == NULL)
		return NULL;

	lmctokens_t addressTokens[] = { IDENTIFIER, NUMERIC };
	uint8_t index = 0;
	lmctoken_t* token = getNextToken(assembler);
	int16_t* assembly = (int16_t*)malloc(sizeof(int16_t) * LMC_MAILBOX_SIZE);

	size_t labelIndex = 0;
	lmclabel_t* labels = (lmclabel_t*)malloc(sizeof(lmclabel_t) * LMC_MAILBOX_SIZE);
	lmclabelref_t* labelReferences = (lmclabelref_t*)malloc(sizeof(lmclabelref_t) * LMC_MAILBOX_SIZE);

	if (labels == NULL || labelReferences == NULL || assembly == NULL)
		return NULL;
	memset(labels, 0, sizeof(lmclabel_t) * LMC_MAILBOX_SIZE);
	memset(labelReferences, 0, sizeof(lmclabelref_t) * LMC_MAILBOX_SIZE);
	memset(assembly, 0, LMC_MAILBOX_SIZE);

	while (token->token != EoF && assembler->errorState[0] == '\0')
	{
		// Instruction.
		if (verifyToken(assembler, IDENTIFIER))
			goto failure;
		char label[LMC_LABEL_SIZE + 1] = { 0 };
		char instruction = isInstruction(token->value);
		memcpy(label, token->value, sizeof(label));
		token = getNextToken(assembler);
		if (token == NULL)
			goto failure;

		// Was the token a label? Try getting the actual instruction.
		if (instruction == -1)
		{
			if (verifyToken(assembler, IDENTIFIER))
				goto failure;
			char opcode[LMC_LABEL_SIZE + 1] = { 0 };
			memcpy(opcode, token->value, sizeof(opcode));
			instruction = isInstruction(token->value);
			token = getNextToken(assembler);
			if (token == NULL)
				goto failure;

			// Unrecognised instruction.
			if (instruction == -1)
			{
				sprintf_s(assembler->errorState, sizeof(assembler->errorState), "Unrecognised instruction \"%s\"", opcode);
				goto failure;
			}

			// Set up label information.
			labels[labelIndex].index = index;
			memcpy(labels[labelIndex].label, label, sizeof(labels[labelIndex].label));
			++labelIndex;
		}

		// Assemble instruction.
		char mailbox[5] = { 0 };
		memset(mailbox, '0', sizeof(mailbox) - 1);
		if (instruction != 0x04)
			mailbox[1] = (instruction == 10 ? 9 : instruction) + '0';
		switch (instruction)
		{
		case 0x00: // HLT
			break;
		case 0x09: // INP
			mailbox[3] = '1';
			break;
		case 0x0A: // OUT
			mailbox[3] = '2';
			break;
		default: // Handles DAT and the other instructions slightly differently.
		{
			if (instruction != 0x04)
			{
				if (verifyTokenList(assembler, addressTokens, 2))
					goto failure;
			}

			if (token->token == NUMERIC) // Direct address.
			{
				size_t length = strlen(token->value);
				int16_t value = atoi(token->value);
				if (instruction == 0x04)
				{
					if (value < -999 || value > 999)
					{
						strcpy_s(assembler->errorState, sizeof(assembler->errorState), "Mailbox values must be within the range -999 to 999.");
						goto failure;
					}
				}
				else
				{
					if (value < 0 || value > 99)
					{
						strcpy_s(assembler->errorState, sizeof(assembler->errorState), "Mailbox addresses must be within the range 0 to 99.");
						goto failure;
					}
				}
				memcpy(mailbox + sizeof(mailbox) - 1 - length, token->value, length);
				token = getNextToken(assembler);
			}
			else if (token->token == IDENTIFIER) // Using labels?
			{
				labelReferences[index].instruction = instruction;
				memcpy(labelReferences[index].label, token->value, sizeof(labelReferences[index].label));
				token = getNextToken(assembler);
			}

			// Typically the token must be either NUMERIC or IDENTIFIER, however in the case of DAT, there may be no specified value.
			if (token == NULL)
				goto failure;
			break;
		}
		}

		// Write to memory.
		assembly[index] = (int16_t)atoi(mailbox);
		++index;
		if (index + 1 > LMC_MAILBOX_SIZE)
		{
			strcpy_s(assembler->errorState, sizeof(assembler->errorState), "LMC applications can only fit in 100 mailboxes.");
			goto failure;
		}

		// Prepare for next instruction.
		if (token->token == EoF)
			break;
		if (verifyToken(assembler, NEWLINE))
			goto failure;
		token = getNextToken(assembler);
		if (token == NULL)
			goto failure;
	}

	// Set memory address for labels.
	for (uint8_t i = 0; i < LMC_MAILBOX_SIZE; ++i)
	{
		if (labelReferences[i].instruction != HLT)
		{
			// Get label.
			lmclabel_t* label = NULL;
			for (uint8_t v = 0; v < LMC_MAILBOX_SIZE; ++v)
			{
				if (strcmp(labels[v].label, labelReferences[i].label) == 0)
				{
					label = &labels[v];
					break;
				}
			}
			if (label == NULL)
			{
				sprintf_s(assembler->errorState, sizeof(assembler->errorState), "Undefined label \"%s\"", labelReferences[i].label);
				goto failure;
			}

			// Get the index as a string.
			char address[5] = { 0 };
			snprintf(address, sizeof(address), "%d", label->index);
			size_t length = strlen(address);

			// Re-write assembly output.
			char mailbox[5] = { 0 };
			memset(mailbox, '0', sizeof(mailbox) - 1);
			if (labelReferences[i].instruction != 0x04)
				mailbox[1] = labelReferences[i].instruction + '0';
			memcpy(mailbox + sizeof(mailbox) - 1 - length, address, length);
			assembly[i] = (int16_t)atoi(mailbox);
		}
	}

	CleanLMCAssembler(assembler);
	free(labels);
	free(labelReferences);
	lmccode_t* bytecode = (lmccode_t*)malloc(sizeof(lmccode_t));
	if (bytecode == NULL)
		return NULL;
	bytecode->assembly = assembly;
	bytecode->length = index;
	return bytecode;

failure:
	free(assembly);
	free(labels);
	free(labelReferences);
	return NULL;
}

lmcassembler_t* CreateLMCAssembler(char* code, size_t length)
{
	lmcassembler_t* assembler = (lmcassembler_t*)malloc(sizeof(lmcassembler_t));
	if (assembler == NULL)
		return NULL;
	assembler->currentToken = NULL;
	SetLMCAssemblerCode(assembler, code, length);
	return assembler;
}