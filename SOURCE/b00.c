#include <io.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

enum TOKEN_TYPE {
	TOKEN_TYPE_INVALID,
	
	TOKEN_TYPE_NAME,
	TOKEN_TYPE_CHAR,
	TOKEN_TYPE_STRING,
	TOKEN_TYPE_INTEGER,
	
	TOKEN_TYPE_EXTERN,		// extern
	TOKEN_TYPE_TYPEDEF,		// typedef
	TOKEN_TYPE_RETURN,		// return
	TOKEN_TYPE_FUNCTION,	// function
	TOKEN_TYPE_BEGIN,		// begin
	TOKEN_TYPE_SET,			// set
	TOKEN_TYPE_END,			// end
	
	TOKEN_TYPE_IF,			// if
	TOKEN_TYPE_THEN,		// then
	TOKEN_TYPE_ELSE,		// else
	TOKEN_TYPE_WHILE,		// while
	TOKEN_TYPE_DO,			// do
	
	TOKEN_TYPE_VAR,			// var
	
	TOKEN_TYPE_PLUS,		// +
	TOKEN_TYPE_MINUS,		// -
	TOKEN_TYPE_MULTIPLY,	// *
	TOKEN_TYPE_DIVIDE,		// /
	TOKEN_TYPE_MODULO,		// %
	
	TOKEN_TYPE_AND,			// and
	TOKEN_TYPE_NOT,			// not
	TOKEN_TYPE_OR,			// or
	TOKEN_TYPE_XOR,			// xor
	
	TOKEN_TYPE_SHL,			// shl
	TOKEN_TYPE_SHR,			// shr
	
	TOKEN_TYPE_ADDR_OF,		// &
	TOKEN_TYPE_INST_OF,		// #
	TOKEN_TYPE_PEEK,		// @
	TOKEN_TYPE_POKE,		// !
	TOKEN_TYPE_COLON,		// :
	
	TOKEN_TYPE_EQUAL,		// equal
	TOKEN_TYPE_LESS,		// less
	TOKEN_TYPE_GREATER,		// greater
	
	TOKEN_TYPE_INCREMENT,	// ++
	TOKEN_TYPE_DECREMENT,	// --
	
	TOKEN_TYPE_LPAREN,		// (
	TOKEN_TYPE_RPAREN,		// )
	TOKEN_TYPE_LBRACK,		// [
	TOKEN_TYPE_RBRACK,		// ]
	
	TOKEN_TYPE_EOF,
};

struct Keyword {
	char symb[16];
	int type;
};

struct Const {
	char symb[16];
	int val;
};

int ch(int);
void key(char *, int);
int symbol(int);
int subseq(char, int, int);
int comm(void);
void getstr(void);
void declare(void);

#define fsystem(buf, cmd, ...) sprintf(buf, cmd, __VA_ARGS__); \
							   system(buf);

#define error(msg, ...) { printf(msg, __VA_ARGS__); \
		 				exit(1); }

char * types[17] = {
	".unkn",		// 0
	".byte",		// 1
	".word",		// 2
	".unkn",		// 3
	".long",		// ".short"  4
	".unkn",		// 5
	".unkn",		// 6
	".unkn",		// 7
	".quad",		// 8
	".unkn",		// 9
	".ten",			// 10
	".unkn",		// 11
	".unkn",		// 12
	".unkn",		// 13
	".unkn",		// 14
	".unkn",		// 15
	".octo",		// 16
};

struct Keyword keywrds[26];
struct Const constt[32];
char vars[32][16];
char symbuf[16];
int varl = 0;
int constl = 0;
int keywrdl = 0;
int namesiz = 16;
int intval = 0;
int op = 0;
int sidx = 0;
int eoff = 0;
int line = 1;
int infunc = 0;
int invars = 0;
int in_cond = 1;
int in_loop = 1;
int wdx_cond = 0;
int nwdx_cond = 0;
int ifdx_cond = 0;
int nifdx_cond = 0;
int nested[8] = {};
int nidx = 0;
FILE *outp;

int main(int argc, char ** argv)
{
	if (argc < 3 || argc > 3) {
		fprintf(outp, "Arg Count : %d\n", argc);
		exit(1);
	}
	freopen(argv[1], "r", stdin);

	key("extern", TOKEN_TYPE_EXTERN);
	key("typedef", TOKEN_TYPE_TYPEDEF);
	key("return", TOKEN_TYPE_RETURN);
	key("function", TOKEN_TYPE_FUNCTION);
	key("begin", TOKEN_TYPE_BEGIN);
	key("set", TOKEN_TYPE_SET);
	key("end", TOKEN_TYPE_END);
	key("var", TOKEN_TYPE_VAR);
	
	key("if", TOKEN_TYPE_IF);
	key("then", TOKEN_TYPE_THEN);
	key("else", TOKEN_TYPE_ELSE);
	
	key("while", TOKEN_TYPE_WHILE);
	key("do", TOKEN_TYPE_DO);
	
	key("less", TOKEN_TYPE_LESS);
	key("equal", TOKEN_TYPE_EQUAL);
	key("greater", TOKEN_TYPE_GREATER);
	
	key("and", TOKEN_TYPE_AND);
	key("not", TOKEN_TYPE_NOT);
	key("or", TOKEN_TYPE_OR);
	key("xor", TOKEN_TYPE_XOR);
	
	key("shl", TOKEN_TYPE_SHL);
	key("shr", TOKEN_TYPE_SHR);

	char outpfile[100];
	sprintf(outpfile, "%s.s", argv[2]);
	outp = fopen(outpfile, "w");
	
	fprintf(outp, ".globl _main\n");
	fprintf(outp, "jmp _main\n");
	
	while (!eoff) {
		declare();
	}
	
	fclose(outp);
	
	char command[256] = {};
	fsystem(command, "as %s.s -o %s.obj", argv[2], argv[2]);
	fsystem(command, "ld -o %s.exe %s.obj  -L/mingw/lib -luser32 -lkernel32 -lmsvcrt -lm", argv[2], argv[2]);
	fsystem(command, "del %s.obj del %s.s", argv[2], argv[2]);
	
	printf(":: as %s.s -o %s.obj\n", argv[2], argv[2]);
	printf(":: ld -o %s.exe %s.obj  -L/mingw/lib -luser32 -lkernel32 -lmsvcrt -lm\n", argv[2], argv[2]);
	printf(":: del %s.exe && del %s.obj\n", argv[2], argv[2]);
	return 0;
}

void function(char * s)
{
	fprintf(outp, ".text;_%s:\n", s);
}

void declare(void)
{
	int o, var_sizeof;
	if ((o=symbol(0)) == TOKEN_TYPE_EOF)
		return;

	if (o == TOKEN_TYPE_FUNCTION) { /* Functions */
		if (!infunc) {
			infunc = 1;
			if((o = symbol(1)) != TOKEN_TYPE_NAME) {
				error("%d: Function Definion Error\n", line);
				exit(1);
			}
			function(symbuf);
		} else {
			error("%d: Function Definion Error\n", line);
			exit(1);
		}
	} else
	if (o == TOKEN_TYPE_IF) { /* If conditions */
		in_cond+=in_loop;
		ifdx_cond++;
		nifdx_cond = ifdx_cond;
		nested[nidx] = ifdx_cond;
		nidx++;
	} else
	if (o == TOKEN_TYPE_WHILE) { /* While loop */
		in_loop+=in_cond;
		wdx_cond++;
		fprintf(outp, ".text; .WHILE%d:\n", wdx_cond);
		nwdx_cond = wdx_cond;
		nested[nidx] = wdx_cond;
		nidx++;
	} else
	if (o == TOKEN_TYPE_GREATER) {
		op = '>';
	} else
	if (o == TOKEN_TYPE_EQUAL) {
		op = '=';
	} else
	if (o == TOKEN_TYPE_LESS) {
		op = '<';
	} else
	if (o == TOKEN_TYPE_THEN) { /* If conditions */
		if (ifdx_cond) {
			fprintf(outp, ".text; popl %%ebx\n");
			fprintf(outp, ".text; popl %%eax\n");
			fprintf(outp, ".text; cmp %%eax, %%ebx\n");
			if (op == '>') fprintf(outp, ".text; jg .IFTHEN%d\n", ifdx_cond);
			if (op == '=') fprintf(outp, ".text; je .IFTHEN%d\n", ifdx_cond);
			if (op == '<') fprintf(outp, ".text; jl .IFTHEN%d\n", ifdx_cond);
			fprintf(outp, ".text; jmp .IFEND%d\n", ifdx_cond);
			fprintf(outp, ".text; .IFTHEN%d:\n", ifdx_cond);
		} else {
			error("%d: Not Started If-Condition\n", line);
			exit(1);
		}
	} else
	if (o == TOKEN_TYPE_DO) { /* While loop */
		fprintf(outp, ".text; popl %%ebx\n");
		fprintf(outp, ".text; popl %%eax\n");
		fprintf(outp, ".text; cmp %%ebx, %%eax\n");
		if (op == '>') fprintf(outp, ".text; jl .WHILEEND%d\n", wdx_cond);
		if (op == '=') fprintf(outp, ".text; jne .WHILEEND%d\n", wdx_cond);
		if (op == '<') fprintf(outp, ".text; jg .WHILEEND%d\n", wdx_cond);
	} else
	if (o == TOKEN_TYPE_PLUS) { /* Arithmetic */
		fprintf(outp, ".text; popl %%ebx\n");
		fprintf(outp, ".text; popl %%eax\n");
		fprintf(outp, ".text; addl %%ebx, %%eax\n");
		fprintf(outp, ".text; pushl %%eax\n");
	} else
	if (o == TOKEN_TYPE_MINUS) {
		fprintf(outp, ".text; popl %%ebx\n");
		fprintf(outp, ".text; popl %%eax\n");
		fprintf(outp, ".text; subl %%ebx, %%eax\n");
		fprintf(outp, ".text; pushl %%eax\n");
	} else
	if (o == TOKEN_TYPE_MULTIPLY) {
		fprintf(outp, ".text; popl %%ebx\n");
		fprintf(outp, ".text; popl %%eax\n");
		fprintf(outp, ".text; imull %%ebx, %%eax\n");
		fprintf(outp, ".text; pushl %%eax\n");
	} else
	if (o == TOKEN_TYPE_DIVIDE) {
		fprintf(outp, ".text; xorl %%edx, %%edx\n");
		fprintf(outp, ".text; popl %%ebx\n");
		fprintf(outp, ".text; popl %%eax\n");
		fprintf(outp, ".text; idivl %%ebx\n");
		fprintf(outp, ".text; pushl %%eax\n");
	} else
	if (o == TOKEN_TYPE_MODULO) {
		fprintf(outp, ".text; xorl %%edx, %%edx\n");
		fprintf(outp, ".text; popl %%ebx\n");
		fprintf(outp, ".text; popl %%eax\n");
		fprintf(outp, ".text; idivl %%ebx\n");
		fprintf(outp, ".text; pushl %%edx\n");
	} else
	if (o == TOKEN_TYPE_AND) { /* Bitwise */
		fprintf(outp, ".text; popl %%ebx\n");
		fprintf(outp, ".text; popl %%eax\n");
		fprintf(outp, ".text; andl %%ebx, %%eax\n");
		fprintf(outp, ".text; pushl %%eax\n");
	} else
	if (o == TOKEN_TYPE_NOT) {
		fprintf(outp, ".text; popl %%eax\n");
		fprintf(outp, ".text; notl %%eax\n");
		fprintf(outp, ".text; pushl %%eax\n");
	} else
	if (o == TOKEN_TYPE_OR) {
		fprintf(outp, ".text; popl %%ebx\n");
		fprintf(outp, ".text; popl %%eax\n");
		fprintf(outp, ".text; orl %%ebx, %%eax\n");
		fprintf(outp, ".text; pushl %%eax\n");
	} else
	if (o == TOKEN_TYPE_SHL) {
		fprintf(outp, ".text; popl %%ecx\n");
		fprintf(outp, ".text; popl %%eax\n");
		fprintf(outp, ".text; shll %%cl, %%eax\n");
		fprintf(outp, ".text; pushl %%eax\n");
	} else
	if (o == TOKEN_TYPE_SHR) {
		fprintf(outp, ".text; popl %%ecx\n");
		fprintf(outp, ".text; popl %%eax\n");
		fprintf(outp, ".text; shrl %%cl, %%eax\n");
		fprintf(outp, ".text; pushl %%eax\n");
	} else
	if (o == TOKEN_TYPE_PEEK) {
		fprintf(outp, ".text; popl %%edx\n"); /* get the address */
		fprintf(outp, ".text; pushl (%%edx)\n");
	} else
	if (o == TOKEN_TYPE_POKE) {
		fprintf(outp, ".text; popl %%ebx\n"); /* get the value */
		fprintf(outp, ".text; popl %%edx\n"); /* get the address */
		fprintf(outp, ".text; mov %%ebx, (%%edx)\n");
	} else
	if (o == TOKEN_TYPE_RETURN) {
		fprintf(outp, ".text; popl %%eax\n");
	} else
	if (o == TOKEN_TYPE_EXTERN) { /* Extern function */
		if((o = symbol(1)) == TOKEN_TYPE_NAME) {
			fprintf(outp, ".extern %s\n", symbuf);
		} else {
			error("%d: Extern Function Definition Error\n", line);
			exit(1);
		}
	} else
	if (o == TOKEN_TYPE_SET) { /* Constants */
		if ((o = symbol(1)) != TOKEN_TYPE_NAME) {
			error("%d: Constant Definition Error\n", line);
			exit(1);
		}
		strcpy(constt[constl].symb, symbuf);
		fprintf(outp, "%s = ", symbuf);
		if ((o = symbol(1)) != TOKEN_TYPE_COLON) {
			error("%d: Constant Definition Error\n", line);
			exit(1);
		}
		if ((o = symbol(1)) != TOKEN_TYPE_INTEGER) {
			error("%d: Constant Definition Error\n", line);
			exit(1);
		}
		constt[constl].val = intval;
		fprintf(outp, "%d\n", intval);
		if ((o = symbol(1)) != TOKEN_TYPE_END) {
			error("%d: Constant Definition Error\n", line);
			exit(1);
		}
		constl++;
	} else
	if (o == TOKEN_TYPE_VAR) { /* Variables */
		if ((o = symbol(1)) == TOKEN_TYPE_NAME) {
			for (int i = 0; i < varl; ++i) {
				if (!strcmp(symbuf, vars[i])) {
					error("%d: Variable Redefinition\n", line);
					exit(1);
				}
			}
			strcpy(vars[varl], symbuf);
			fprintf(outp, ".data; %s:", symbuf);
			if ((o = symbol(1)) != TOKEN_TYPE_COLON) {
				error("\n%d: Variable Definion Error\n", line);
				exit(1);
			}
			o = symbol(1);
			if (o == TOKEN_TYPE_INTEGER) {
				var_sizeof = intval;
				fprintf(outp, "%s ", types[var_sizeof]);
			} else
			if (o == TOKEN_TYPE_NAME) {
				for (int i = 0; i < constl; ++i) {
					if (!strcmp(constt[i].symb, symbuf)) {
						fprintf(outp, "%s ", types[constt[i].val]);
						break;
					}
				}
			} else {
				error("%d: Type should be an integer\n", line);
				exit(1);
			}
			putc('0', outp);
			o = symbol(1);
			if (o == TOKEN_TYPE_LBRACK) {
				o = symbol(1);
				if (o == TOKEN_TYPE_NAME) {
					for (int i = 0; i < constl; ++i) {
						if (!strcmp(symbuf, constt[i].symb)) {
							intval = constt[i].val;
							break;
						}
					}
				} else
				if (o != TOKEN_TYPE_INTEGER) {
					error("%d: Array Variable Definion Error\n", line);
					exit(1);
				}
				for (int i = 0; i < intval - 1; ++i) { fprintf(outp, ",0"); }
				if ((o = symbol(1)) != TOKEN_TYPE_RBRACK) {
					error("%d: Array Variable Definion Error\n", line);
					exit(1);
				}
				o = symbol(1);
			}
			putc('\n', outp);
			if (o != TOKEN_TYPE_END) {
				error("%d: Variable Definion Error\n", line);
				exit(1);
			}
			varl++;
		} else {
			error("%d: Variable Definion Error\n", line);
			exit(1);
		}
	} else
	if (o == TOKEN_TYPE_END) {
		if (in_loop > in_cond) {
			if (in_loop > 0) {
				nidx--;
				fprintf(outp, ".text; jmp .WHILE%d\n", nested[nidx]);
				fprintf(outp, ".text; .WHILEEND%d:\n", nested[nidx]);
				in_loop-=in_cond;
				nwdx_cond--;
			}
		} else
		if ((in_cond > in_loop)) {
			if (in_cond > 0) {
				nidx--;
				fprintf(outp, ".text; .IFEND%d:\n", nested[nidx]);
				in_cond-=in_loop;
				nifdx_cond--;
			}
		} else
		if (infunc) {
			infunc = 0;
			fprintf(outp, ".text; ret\n");
		} else {
			error("%d: Nothing to End\n", line);
			exit(1);
		}
	}
}

void key(char * k, int t)
{
	strcpy(keywrds[keywrdl].symb, k);
	keywrds[keywrdl].type = t;
	keywrdl++;
}

int symbol(int t)
{
	int c = 0; char *sp;
	memset(symbuf, '\0', namesiz);
	
	c = getchar();
	while (isspace(c)) { /* check for whitespace */
		if (c == '\n')
			line++;
		c = getchar();
	}
	if (c == (-1)) { /* check for eof */
		eoff++;
		return TOKEN_TYPE_EOF;
	} else
	if (isalpha(c) || c == '_') { /* check for name */
		sp = symbuf;
		while (isalnum(c) || c == '_' || c == '@') {
			if (sp - symbuf > namesiz) {
				error("%d: Symbol too big : '%s'\n", line, symbuf);
				exit(1);
			}
			*sp++ = c; c = getchar();
		}
		if (symbuf[0] == '_' && !t) {
			fprintf(outp, ".text; call %s\n", symbuf);
			fprintf(outp, ".text; pushl %%eax\n");
		} else {
			for (int i = 0; i < keywrdl; ++i) { /* check for keywords */
				if (!strcmp(symbuf, keywrds[i].symb)) {
					return keywrds[i].type;
				}
			}
			for (int i = 0; i < varl; ++i) { /* check for variable name */
				if (!strcmp(symbuf, vars[i])) {
					fprintf(outp, ".text; movl $%s, %%edx\n", symbuf);
					fprintf(outp, ".text; pushl %%edx\n");
					break;
				}
			}
			if (!t) {
				for (int i = 0; i < constl; ++i) { /* check for constant */
					if (!strcmp(symbuf, constt[i].symb)) {
						fprintf(outp, ".text; pushl $%d\n", constt[i].val);
						break;
					}
				}
			}
		}
		return TOKEN_TYPE_NAME;
	} else
	if (isdigit(c)) { /* check for number */
		intval = 0;
		int b = 10;
		while (isdigit(c)) {
			intval = (intval * b) + (c - '0');
			c = getchar();
		}
		if (!t)
			fprintf(outp, ".text; pushl $%d\n", intval);
		return TOKEN_TYPE_INTEGER;
	} else
	if (c == '"') { /* check for string */
		getstr();
		return TOKEN_TYPE_STRING;
	} else
	if (c == '+') { /* check for + */
		return subseq('+', TOKEN_TYPE_PLUS, TOKEN_TYPE_INCREMENT);
	} else
	if (c == '-') { /* check for - */
		c = getchar();
		if (isdigit(c)) {
			intval = 0;
			int b = 10;
			while (isdigit(c)) {
				intval = (intval * b) + (c - '0');
				c = getchar();
			}
			if (!t)
				fprintf(outp, ".text; pushl $-%d\n", intval);
			return TOKEN_TYPE_INTEGER;
		}
		return (c == '-') ? TOKEN_TYPE_DECREMENT : TOKEN_TYPE_MINUS;
	} else
	if (c == '*') { /* check for * */
		return TOKEN_TYPE_MULTIPLY;
	} else
	if (c == '%') { /* check for % */
		return TOKEN_TYPE_MODULO;
	} else
	if (c == '/') { /* check for / */
		return TOKEN_TYPE_DIVIDE;
	} else
	if (c == ':') { /* check for : */
		return TOKEN_TYPE_COLON;
	} else
	if (c == '!') { /* check for poke */
		return TOKEN_TYPE_POKE;
	} else
	if (c == '@') { /* check for peek */
		return TOKEN_TYPE_PEEK;
	} else
	if (c == '[') { /* check for brackets */
		return TOKEN_TYPE_LBRACK;
	} else
	if (c == ']') { /* check for brackets */
		return TOKEN_TYPE_RBRACK;
	} else
	if (c == '/') { /* check for comment */
		if (subseq('/', 0, 1)) {
			c = getchar();
			while (c != '\n') {
				c = getchar();
			}
		}
		return TOKEN_TYPE_DIVIDE;
	} else { /* check invalid character */
		error("%d: Unknown Character : '%c'\n", line, c);
		exit(1);
	}
	return TOKEN_TYPE_INVALID;
}

int comm(void)
{
	int c;
loop:
	do {
		c = getchar();
	} while (c != '*'); 
	
	if (subseq('/', 0, 1)) {
		return 0;
	} else {
		goto loop;
	}
	return 0;
}

int ch(int a)
{
	char c;
	if ((c = getchar()) == a) {
		return (-1);
	}
	if (c == '\n' || c == 0) {
		error("%d: Non-terminated String : '%c'\n", line, c);
		exit(1);
	}
	if (c == '\\') {
		c = getchar();
		if (c == 'n') return '\n';
		if (c == 'r') return '\r';
		if (c == 't') return '\t';
		if (c == '0') return '\0';
		else {
			error("%d: Unknown Escape Character : '%c'\n", line, c);
			exit(1);
		}
	}
	return c;
}

int subseq(char c, int a, int b)
{
	char d = getchar();
	if (d == c)	return b;
	return a;
}

void getstr(void)
{
	int c;
	fprintf(outp, ".data;L%d:.byte ", sidx);
	while ((c = ch('"')) >= 0) {
		fprintf(outp, "0x%X,", c);
	}
	fprintf(outp, "0x0\n");
	fprintf(outp, ".text; pushl $L%d\n", sidx);
	sidx++;
}