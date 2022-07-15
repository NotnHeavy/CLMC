#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "machine.h"

static inline void overflow(int16_t* value)
{
	if (*value > 999)
		*value -= 1999;
	else if (*value < -999)
		*value += 1999;
}

void ResetLMC(lmc_t* machine)
{
	if (machine == NULL)
		return;

	machine->accumulator = 0;
	for (size_t i = 0; i < sizeof(machine->mailboxes) / sizeof(machine->mailboxes[0]); ++i)
		machine->mailboxes[i] = 0;
	machine->pc = 0;
	machine->ir = 0;
	machine->ar = 0;
}

void LoadIntoLMC(lmc_t* machine, int16_t buffer[], size_t length)
{
	if (machine == NULL)
		return;

	for (size_t i = 0; i < length && i < sizeof(machine->mailboxes); ++i)
	{
		machine->mailboxes[i] = buffer[i];
		overflow(&machine->mailboxes[i]);
	}
}

int ExecuteLMC(lmc_t* machine)
{
	if (machine == NULL)
		return 1;

	for (;;)
	{
		// FETCH
		machine->ar = machine->mailboxes[machine->pc];

		// DECODE
		machine->ir = machine->ar / 100;
		machine->ar = machine->ar - machine->ir * 100;
		if (machine->ir == 0x09)
			machine->ir = 900 + machine->ar;

		// EXECUTE
		switch (machine->ir)
		{
		// Standard instructions.
		case HLT:
			return 0;
		case ADD:
			machine->accumulator += machine->mailboxes[machine->ar];
			overflow(&machine->accumulator);
			break;
		case SUB:
			machine->accumulator -= machine->mailboxes[machine->ar];
			overflow(&machine->accumulator);
			break;
		case STA:
			machine->mailboxes[machine->ar] = machine->accumulator;
			break;
		case LDA:
			machine->accumulator = machine->mailboxes[machine->ar];
			break;
		case BRA:
			machine->pc = (uint8_t)machine->ar;
			continue;
		case BRZ:
			if (!machine->accumulator)
			{
				machine->pc = (uint8_t)machine->ar;
				continue;
			}
			break;
		case BRP:
			if (machine->accumulator >= 0)
			{
				machine->pc = (uint8_t)machine->ar;
				continue;
			}
			break;
		case INP:
		{
			char buffer[6] = { 0 }; // -999 to 999, with enough space for \n and \0.
			fgets(buffer, sizeof(buffer), stdin);
			buffer[strlen(buffer) - 1] = '\0';
			machine->accumulator = atoi(buffer);
			break;
		}
		case OUT:
			printf("%d", machine->accumulator);
			break;

		// Specific to CLMC.
		case IPC:
			machine->accumulator = getchar();
			if (machine->accumulator != '\n')
			{
				int clear = 0;
				while (clear = getchar() != '\n' && clear != EOF) { }
			}
			break;
		case OTC:
			putchar(machine->accumulator);
			break;
		}

		// REPEAT
		++machine->pc;
	}
}

lmc_t* CreateLMC(void)
{
	lmc_t* machine = (lmc_t*)malloc(sizeof(lmc_t));
	ResetLMC(machine);
	return machine;
}