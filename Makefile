# CC =
# CFLAGS =

all: clmc
clmc: assembler.c assembler.h machine.c machine.h main.c
	$(CC) $(CFLAGS) $(LDFLAGS) assembler.c machine.c main.c -o clmc

clean:
	rm clmc
