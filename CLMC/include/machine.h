#pragma once
#include <stdint.h>

#define LMC_MAILBOX_SIZE 100
#define LMC_LABEL_SIZE 32 // 32 characters for label, use 1 more for reserve NUL

typedef enum
{
	HLT,	   // 000
	ADD,	   // 1xx
	SUB,	   // 2xx
	STA,	   // 3xx
	DAT,       // does not have a numeric representation
	LDA,       // 5xx
	BRA,	   // 6xx
	BRZ,       // 7xx
	BRP,       // 8xx
	INP = 901, // 901
	OUT		   // 902
} opcode_t;

typedef struct
{
	uint8_t pc;
	opcode_t ir;
	int16_t ar;
	int16_t mailboxes[LMC_MAILBOX_SIZE];
	int16_t accumulator;
} lmc_t;

void ResetLMC(lmc_t* machine);
void LoadIntoLMC(lmc_t* machine, int16_t buffer[], size_t length);
int ExecuteLMC(lmc_t* machine);
lmc_t* CreateLMC(void);