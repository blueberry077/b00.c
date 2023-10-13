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

int main(int argc, char ** argv)
{
	if (argc < 3 || argc > 3) {
		printf("Arg Count : %d\n", argc);
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
	freopen(outpfile, "w", stdout);
	
	printf(".globl _main\n");
	printf("jmp _main\n");
	
	while (!eoff) {
		declare();
	}
	
	printf("// as %s.s -o %s.obj\n", argv[2], argv[2]);
	printf("// ld -o %s.exe %s.obj  -L/mingw/lib -luser32 -lkernel32 -lmsvcrt\n", argv[2], argv[2]);
	return 0;
}

void function(char * s)
{
	printf(".text;_%s:\n", s);
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
				printf("%d: Function Definion Error\n", line);
				exit(1);
			}
			function(symbuf);
		} else {
			printf("%d: Function Definion Error\n", line);
			exit(1);
		}
	} else
	if (o == TOKEN_TYPE_IF) { /* If conditions */
		in_cond+=in_loop;
		ifdx_cond++;
		nifdx_cond = ifdx_cond;
	} else
	if (o == TOKEN_TYPE_WHILE) { /* While loop */
		in_loop+=in_cond;
		wdx_cond++;
		printf(".text; .WHILE%d:\n", wdx_cond);
		nwdx_cond = wdx_cond;
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
			printf(".text; popl %%ebx\n");
			printf(".text; popl %%eax\n");
			printf(".text; cmp %%eax, %%ebx\n");
			if (op == '>') printf(".text; jg .IFTHEN%d\n", ifdx_cond);
			if (op == '=') printf(".text; je .IFTHEN%d\n", ifdx_cond);
			if (op == '<') printf(".text; jl .IFTHEN%d\n", ifdx_cond);
			printf(".text; jmp .IFEND%d\n", ifdx_cond);
			printf(".text; .IFTHEN%d:\n", ifdx_cond);
		} else {
			printf("%d: Not Started If-Condition\n", line);
			exit(1);
		}
	} else
	if (o == TOKEN_TYPE_DO) { /* While loop */
		printf(".text; popl %%ebx\n");
		printf(".text; popl %%eax\n");
		printf(".text; cmp %%ebx, %%eax\n");
		if (op == '>') printf(".text; jl .WHILEEND%d\n", wdx_cond);
		if (op == '=') printf(".text; jne .WHILEEND%d\n", wdx_cond);
		if (op == '<') printf(".text; jg .WHILEEND%d\n", wdx_cond);
	} else
	if (o == TOKEN_TYPE_PLUS) { /* Arithmetic */
		printf(".text; popl %%ebx\n");
		printf(".text; popl %%eax\n");
		printf(".text; addl %%ebx, %%eax\n");
		printf(".text; pushl %%eax\n");
	} else
	if (o == TOKEN_TYPE_MINUS) {
		printf(".text; popl %%ebx\n");
		printf(".text; popl %%eax\n");
		printf(".text; subl %%ebx, %%eax\n");
		printf(".text; pushl %%eax\n");
	} else
	if (o == TOKEN_TYPE_MULTIPLY) {
		printf(".text; popl %%ebx\n");
		printf(".text; popl %%eax\n");
		printf(".text; imull %%ebx, %%eax\n");
		printf(".text; pushl %%eax\n");
	} else
	if (o == TOKEN_TYPE_AND) { /* Bitwise */
		printf(".text; popl %%ebx\n");
		printf(".text; popl %%eax\n");
		printf(".text; andl %%ebx, %%eax\n");
		printf(".text; pushl %%eax\n");
	} else
	if (o == TOKEN_TYPE_OR) {
		printf(".text; popl %%ebx\n");
		printf(".text; popl %%eax\n");
		printf(".text; orl %%ebx, %%eax\n");
		printf(".text; pushl %%eax\n");
	} else
	if (o == TOKEN_TYPE_SHL) {
		printf(".text; popl %%ecx\n");
		printf(".text; popl %%eax\n");
		printf(".text; shll %%cl, %%eax\n");
		printf(".text; pushl %%eax\n");
	} else
	if (o == TOKEN_TYPE_SHR) {
		printf(".text; popl %%ecx\n");
		printf(".text; popl %%eax\n");
		printf(".text; shrl %%cl, %%eax\n");
		printf(".text; pushl %%eax\n");
	} else
	if (o == TOKEN_TYPE_PEEK) {
		printf(".text; popl %%edx\n"); /* get the address */
		printf(".text; pushl (%%edx)\n");
	} else
	if (o == TOKEN_TYPE_POKE) {
		printf(".text; popl %%ebx\n"); /* get the value */
		printf(".text; popl %%edx\n"); /* get the address */
		printf(".text; mov %%ebx, (%%edx)\n");
	} else
	if (o == TOKEN_TYPE_RETURN) {
		printf(".text; popl %%eax\n");
	} else
	if (o == TOKEN_TYPE_EXTERN) { /* Extern function */
		if((o = symbol(1)) == TOKEN_TYPE_NAME) {
			printf(".extern %s\n", symbuf);
		} else {
			printf("%d: Extern Function Definition Error\n", line);
			exit(1);
		}
	} else
	if (o == TOKEN_TYPE_SET) { /* Constants */
		if ((o = symbol(1)) != TOKEN_TYPE_NAME) {
			printf("%d: Constant Definition Error\n", line);
			exit(1);
		}
		strcpy(constt[constl].symb, symbuf);
		printf("%s = ", symbuf);
		if ((o = symbol(1)) != TOKEN_TYPE_COLON) {
			printf("%d: Constant Definition Error\n", line);
			exit(1);
		}
		if ((o = symbol(1)) != TOKEN_TYPE_INTEGER) {
			printf("%d: Constant Definition Error\n", line);
			exit(1);
		}
		constt[constl].val = intval;
		printf("%d\n", intval);
		if ((o = symbol(1)) != TOKEN_TYPE_END) {
			printf("%d: Constant Definition Error\n", line);
			exit(1);
		}
		constl++;
	} else
	if (o == TOKEN_TYPE_VAR) { /* Variables */
		if ((o = symbol(1)) == TOKEN_TYPE_NAME) {
			for (int i = 0; i < varl; ++i) {
				if (!strcmp(symbuf, vars[i])) {
					printf("%d: Variable Redefinition\n", line);
					exit(1);
				}
			}
			strcpy(vars[varl], symbuf);
			printf(".data; %s:", symbuf);
			if ((o = symbol(1)) != TOKEN_TYPE_COLON) {
				printf("\n%d: Variable Definion Error\n", line);
				exit(1);
			}
			o = symbol(1);
			if (o == TOKEN_TYPE_INTEGER) {
				var_sizeof = intval;
				printf("%s ", types[var_sizeof]);
			} else
			if (o == TOKEN_TYPE_NAME) {
				for (int i = 0; i < constl; ++i) {
					if (!strcmp(constt[i].symb, symbuf)) {
						printf("%s ", types[constt[i].val]);
						break;
					}
				}
			} else {
				printf("%d: Type should be an integer\n", line);
				exit(1);
			}
			putc('0', stdout);
			o = symbol(1);
			if (o == TOKEN_TYPE_LBRACK) {
				o = symbol(1);
				if (o != TOKEN_TYPE_INTEGER) {
					printf("%d: Array Variable Definion Error\n", line);
					exit(1);
				}
				for (int i = 0; i < intval - 1; ++i) { printf(",0"); }
				if ((o = symbol(1)) != TOKEN_TYPE_RBRACK) {
					printf("%d: Array Variable Definion Error\n", line);
					exit(1);
				}
				o = symbol(1);
			}
			putc('\n', stdout);
			if (o != TOKEN_TYPE_END) {
				printf("%d: Variable Definion Error\n", line);
				exit(1);
			}
			varl++;
		} else {
			printf("%d: Variable Definion Error\n", line);
			exit(1);
		}
	} else
	if (o == TOKEN_TYPE_END) {
		if (in_loop > in_cond) {
			if (in_loop > 0) {
				printf(".text; jmp .WHILE%d\n", nwdx_cond);
				printf(".text; .WHILEEND%d:\n", nwdx_cond);
				in_loop-=in_cond;
				nwdx_cond--;
			}
		} else
		if ((in_cond > in_loop)) {
			if (in_cond > 0) {
				printf(".text; .IFEND%d:\n", nifdx_cond);
				in_cond-=in_loop;
				nifdx_cond--;
			}
		} else
		if (infunc) {
			infunc = 0;
			printf(".text; ret\n");
		} else {
			printf("%d: Nothing to End\n", line);
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
				printf("%d: Symbol too big : '%s'\n", line, symbuf);
				exit(1);
			}
			*sp++ = c; c = getchar();
		}
		if (symbuf[0] == '_' && !t) {
			printf(".text; call %s\n", symbuf);
			printf(".text; pushl %%eax\n");
		} else {
			for (int i = 0; i < keywrdl; ++i) { /* check for keywords */
				if (!strcmp(symbuf, keywrds[i].symb)) {
					return keywrds[i].type;
				}
			}
			for (int i = 0; i < varl; ++i) { /* check for variable name */
				if (!strcmp(symbuf, vars[i])) {
					printf(".text; movl $%s, %%edx\n", symbuf);
					printf(".text; pushl %%edx\n");
					break;
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
			printf(".text; pushl $%d\n", intval);
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
				printf(".text; pushl $-%d\n", intval);
			return TOKEN_TYPE_INTEGER;
		}
		return subseq('-', TOKEN_TYPE_MINUS, TOKEN_TYPE_DECREMENT);
	} else
	if (c == '*') { /* check for * */
		return TOKEN_TYPE_MULTIPLY;
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
		if (subseq('*', 0, 1))
			comm();
		return TOKEN_TYPE_DIVIDE;
	} else { /* check invalid character */
		printf("%d: Unknown Character : '%c'\n", line, c);
		exit(1);
	}
	return TOKEN_TYPE_INVALID;
}

int comm(void)
{
	int c;
loop:
	while (c != '*') {
		c = getchar();
	}
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
		printf("%d: Non-terminated String : '%c'\n", line, c);
		exit(1);
	}
	if (c == '\\') {
		c = getchar();
		if (c == 'n') return '\n';
		if (c == 'r') return '\r';
		if (c == 't') return '\t';
		if (c == '0') return '\0';
		else {
			printf("%d: Unknown Escape Character : '%c'\n", line, c);
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
	printf(".data;L%d:.byte ", sidx);
	while ((c = ch('"')) >= 0) {
		printf("0x%X,", c);
	}
	printf("0x0\n");
	printf(".text; pushl $L%d\n", sidx);
	sidx++;
}