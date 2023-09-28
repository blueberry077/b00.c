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
	TOKEN_TYPE_FUNCTION,	// function
	TOKEN_TYPE_BEGIN,		// begin
	TOKEN_TYPE_END,			// end
	
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
	
	TOKEN_TYPE_ADDR_OF,		// &
	TOKEN_TYPE_INST_OF,		// #
	TOKEN_TYPE_PEEK,		// *
	TOKEN_TYPE_POKE,		// !
	TOKEN_TYPE_COLON,		// :
	
	TOKEN_TYPE_INCREMENT,	// ++
	TOKEN_TYPE_DECREMENT,	// --
	
	TOKEN_TYPE_LPAREN,		// (
	TOKEN_TYPE_RPAREN,		// )
	
	TOKEN_TYPE_EOF,
};

struct Keyword {
	char symb[16];
	int type;
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
char vars[32][16];
char symbuf[16];
int varl = 0;
int keywrdl = 0;
int namesiz = 16;
int intval = 0;
int sidx = 0;
int eof = 0;
int line = 1;
int infunc = 0;
int invars = 0;

int main(int argc, char ** argv)
{
	if (argc < 2 || argc > 2) {
		printf("Arg Count : %d\n", argc);
		exit(1);
	}
	freopen(argv[1], "r", stdin);

	key("extern", TOKEN_TYPE_EXTERN);
	key("typedef", TOKEN_TYPE_TYPEDEF);
	key("function", TOKEN_TYPE_FUNCTION);
	key("begin", TOKEN_TYPE_BEGIN);
	key("end", TOKEN_TYPE_END);
	key("var", TOKEN_TYPE_VAR);
	
	key("and", TOKEN_TYPE_AND);
	key("not", TOKEN_TYPE_NOT);
	key("or", TOKEN_TYPE_OR);
	key("xor", TOKEN_TYPE_XOR);

	printf(".globl _main\n");
	printf("jmp _main\n");
	while (!eof) {
		declare();
	}
	
	return 0;
}

void function(char * s)
{
	printf(".text;_%s:\n", s);
}

void declare(void)
{
	int o;
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
	if (o == TOKEN_TYPE_PEEK) {
		printf(".text; popl %%edx\n"); /* get the address */
		printf(".text; pushl (%%edx)\n");
	} else
	if (o == TOKEN_TYPE_POKE) {
		printf(".text; popl %%ebx\n"); /* get the value */
		printf(".text; popl %%edx\n"); /* get the address */
		printf(".text; mov %%ebx, (%%edx)\n");
	} else
	if (o == TOKEN_TYPE_EXTERN) { /* Extern function */
		if((o = symbol(1)) == TOKEN_TYPE_NAME) {
			printf(".extern %s\n", symbuf);
		} else {
			printf("%d: Extern Function Definition Error\n", line);
			exit(1);
		}
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
				printf("\n%d\n", intval);
				printf("\n%d: Variable Definion Error\n", line);
				exit(1);
			}
			if ((o = symbol(1)) == TOKEN_TYPE_INTEGER) {
				invars = 1;
				printf("%s ", types[intval]);
				varl++;
			} else {
				printf("%d: Type should be an integer\n", line);
				exit(1);
			}
			printf("0\n");
			if ((o = symbol(1)) != TOKEN_TYPE_END) {
				printf("%d: Variable Definion Error\n", line);
				exit(1);
			}
		} else {
			printf("%d: Variable Definion Error\n", line);
			exit(1);
		}
	} else
	if (o == TOKEN_TYPE_END) {
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
		eof++;
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
	if (c == ':') { /* check for : */
		return TOKEN_TYPE_COLON;
	} else
	if (c == '!') { /* check for poke */
		return TOKEN_TYPE_POKE;
	} else
	if (c == '*') { /* check for peek */
		return TOKEN_TYPE_PEEK;
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