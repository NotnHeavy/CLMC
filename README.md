# CLMC
A Little Man's Computer virtual machine in C. Really all you do with this is feed the executor any .lmc files you have on your PC.

## stdio
INP/OUT work just like the standard implementation: INP allows you to input a number between -999 and 999 to store into the accumulator, while OUT outputs the numeric representation. However, there also exists IPC and OPC (backwards compatible with Peter Higginson's implemenation), which are non-standard. IPC/OPC are essentially CLMC's equivalents of getchar()/putchar().

## Little Man's Computer
Little Man's Computer is a very stripped down representation of a computer using the [von Neumann architecture](https://en.wikipedia.org/wiki/Von_Neumann_architecture). The sole purpose of this is to teach people the basics of a modern computer. [This Wikipedia page](https://en.wikipedia.org/wiki/Little_man_computer) has good documentation and lists of all the instructions used with it.
