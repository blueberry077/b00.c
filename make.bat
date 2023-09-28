gcc -Wall -Wextra -o b00 SOURCE/b00.c && b00 EXEMPLES/hello > EXEMPLES/hello.s
as EXEMPLES/hello.s -o EXEMPLES/hello.obj
ld -o EXEMPLES/hello.exe EXEMPLES/hello.obZj  -L/mingw/lib -luser32 -lkernel32 -lmsvcrt
EXEMPLES/hello.exe