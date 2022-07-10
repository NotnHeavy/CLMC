# CLMC
A Little Man's Computer virtual machine in C. Really all you do with this is feed the executor any .lmc files you have on your PC. Do note that stdin/stdout will stick to the ASCII representation of characters. For instance, if the accumulator has the value of "72" and you use "OUT", it will write the character "H" to stdout.

## Little Man's Computer
Little Man's Computer is a very stripped down representation of a computer using the [von Neumann architecture](https://en.wikipedia.org/wiki/Von_Neumann_architecture). The sole purpose of this is to teach people the basics of a modern computer. [This Wikipedia page](https://en.wikipedia.org/wiki/Little_man_computer) has good documentation and lists of all the instructions used with it.
