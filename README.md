# blueberry
blueberry compile (b00) is a compiled programming language  
written in ANSI C.
**WORK IN PROGRESS. SOME FEATURES MAY CHANGE!**  
## Prerequisites
You must have, in order to compile:
- [The Gnu Assembler](https://fr.wikipedia.org/wiki/GNU_Assembler)
- [The GNU Linker](https://fr.wikipedia.org/wiki/GNU_linker)
- [The GNU Compiler Collection](https://en.wikipedia.org/wiki/GNU_Compiler_Collection)
## Quickstart
```bash
> gcc -Wall -Wextra -o SOURCE/b00.c b00
```
run this command to compile the compile.  
Or just run the make.bat script.

## Exemple
```bash
> b00 EXEMPLS\hello.blue EXEMPLS\hello
> as EXEMPLES\hello.s -o EXEMPLES\hello.obj
> ld -o EXEMPLES\hello.exe EXEMPLES\hello.obj  -L/mingw/lib -luser32 -lkernel32 -lmsvcrt
> EXEMPLES\hello.exe
```
run this command to compile the hello exemple.  
Or just run the make.bat script.
# .........................
