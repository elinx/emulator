#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, EQ,

	/* TODO: Add more token types */
	DECIMAL, HEX

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */
	{"\\(",            '('},
	{"\\)",            ')'},
	{"[1-9][0-9]*",    DECIMAL},
	{"0x[0-9]*",       HEX},
	{"/",              '/'},
	{"\\*",            '*'},
	{"\\-",            '-'},
	{" +",	           NOTYPE},	// spaces
	{"\\+",            '+'},	// plus
	{"==",             EQ}   	// equal
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
				case NOTYPE: break;
				case DECIMAL:
				case HEX:
					tokens[nr_token].type = rules[i].token_type;
					strncpy(tokens[nr_token].str, substr_start, substr_len);
					nr_token += 1;
					break;
				case ')':
				case '(':
				case '+':
				case '-':
				case '*':
				case '/':
					tokens[nr_token].type = rules[i].token_type;
					*tokens[nr_token].str = rules[i].token_type;
					nr_token += 1;
					break;
				default: panic("please implement me");
				}

				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}

static void dump_tokens(void)
{
	int i = 0;
	printf("%d tokens:\n", nr_token);
	for (; i < nr_token; ++i) {
		printf("[%02d] (%d, %s)\n", i, tokens[i].type, tokens[i].str);
	}
}

static bool is_parentheses_balance(uint32_t start, uint32_t end)
{
	int cnt = 0;

	if (tokens[start].type != '(') return false;

	for(; start <= end; ++start) {
		if (tokens[start].type == '(') cnt++;
		if (tokens[start].type == ')') cnt--;
	}
	return cnt == 0;
}

static uint32_t dominator(uint32_t start, uint32_t end)
{
	uint32_t d = start;
	for (; start <= end; ++start) {
		switch (tokens[start].type) {
		case '+':
		case '-':
			d = start;
			break;
		case '*':
		case '/':
			if (d != '*' && d != '/') d = start;
			break;
		default:
			break;
		}
	}
	return d;
}

static uint32_t eval(uint32_t start, uint32_t end, bool *success)
{
	if (start > end) {
		Log("start: %d, end: %d\n", start, end);
		*success = false;
		return 0;
	} else if (start == end) {
		Log("start: %d, end: %d\n", start, end);
		*success = true;
		return strtoul(tokens[start].str, 0, 0);
	} else if (is_parentheses_balance(start, end)) {
		Log("start: %d, end: %d\n", start, end);
		return eval(start + 1, end - 1, success);
	} else {
		uint32_t dom = dominator(start, end);
		Log("dominator: %d\n", dom);

		uint32_t l = eval(start, dom - 1, success);
		uint32_t r = eval(dom, end - 1, success);

		Log("l: %d, r: %d\n", l, r);
		switch (tokens[dom].type) {
		case '+': return l + r;
		case '-': return l - r;
		case '*': return l * r;
		case '/': return l / r;
		default:
			assert(0);
		}
	}
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	dump_tokens();

	return eval(0, nr_token - 1, success);
}

