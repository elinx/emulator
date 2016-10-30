#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

#define REG_PATTERN "\\$(eax|ebx|ecx|edx|esp|ebp|esi|edi)"
enum {
	NOTYPE = 256, EQ, EREG,

	/* TODO: Add more token types */
	DECIMAL, HEX, NEG, DEREF,
	GT, GTE, LT, LTE,
	NEQ, AND, OR, NOT
};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */
	{REG_PATTERN,      EREG},
	{"\\(",            '('},
	{"\\)",            ')'},
	{"0x[0-9]*",       HEX},
	{"[0-9][0-9]*",    DECIMAL},
	{"/",              '/'},
	{"\\*",            '*'},
	{"\\-",            '-'},
	{" +",	           NOTYPE},	// spaces
	{"\\+",            '+'},	// plus
	{"==",             EQ},   	// equal
	{"!=",             NEQ},        // not equal
	{"&&",             AND},        // logic and
	{"\\|\\|",         OR},         // logic or
	{">=",             GTE},        // >=
	{">",              GT},         // >
	{"<=",             LTE},        // <=
	{"<",              LT},         // <
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
				case EREG:
				case EQ:
				case NEQ:
				case AND:
				case OR:
				case DECIMAL:
				case HEX:
				case GTE:
				case LTE:
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
				case GT:
				case LT:
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

#define DUMP_TOKENS
#ifdef DUMP_TOKENS
static void dump_tokens(void)
{
	int i = 0;
	printf("%d tokens:\n", nr_token);
	for (; i < nr_token; ++i) {
		printf("[%02d] (%d, %s)\n", i, tokens[i].type, tokens[i].str);
	}
}
#endif

static bool is_parentheses_balance(uint32_t start, uint32_t end)
{
	int cnt = 0;

	if ((tokens[start++].type == '(') && (tokens[end--].type == ')')) {
		for(; start < end; ++start) {
			if (tokens[start].type == '(') cnt++;
			if (tokens[start].type == ')') cnt--;
			if (cnt < 0) goto out;
		}
		return cnt == 0;
	}
out:
	return false;
}

static uint32_t dominator(uint32_t start, uint32_t end)
{
	uint32_t d = start, low = 0, fo = 0;
	for (; start <= end; ++start) {
		if (tokens[start].type == '(')
			while (tokens[start++].type != ')');
				/* printf("start: %d\n", start); */
		switch (tokens[start].type) {
		case EQ:
		case NEQ:
		case AND:
		case OR:
		case GT:
		case GTE:
		case LT:
		case LTE:
			d = start;
			fo = 1;	/* first order donimator */
			break;
		case '+':
		case '-':
			if (fo == 0) { /* dominates when logical operator don't */
				d = start;
				low = 1;
			}
			break;
		case '*':
		case '/':
			// */- dominates only when +/- don't
			if (low == 0) d = start;
			break;
		default:
			break;
		}
	}
	return d;
}

static uint32_t read_reg(const char *reg)
{
	uint32_t i = 0;
	const char *reg_name = regsl[i];

	while (reg_name) {
		if (strcmp(reg, reg_name) == 0) {
			return reg_l(i);
		}
		++i;
		reg_name = regsl[i];
	}
	return 0;
}

static int32_t eval(uint32_t start, uint32_t end, bool *success)
{
	Log("start: %d, end: %d", start, end);
	if (start > end) {
		*success = false;
		return 0;
	} else if (start == end) {
		*success = true;
		return strtol(tokens[start].str, 0, 0);
	} else if (tokens[start].type == EREG) {
		*success = true;
		return read_reg(tokens[start++].str + 1); /* remove the prefix '$' */
	} else if (tokens[start].type == NEG) { /* process negtive number */
		*success = true;
		return -strtol(tokens[++start].str, 0, 0);
	} else if (tokens[start].type == DEREF) { /* process de-reference */
		*success = true;
		return swaddr_read(strtol(tokens[++start].str, 0, 0), 1);
	} else if (is_parentheses_balance(start, end)) {
		return eval(start + 1, end - 1, success);
	} else {
		uint32_t dom = dominator(start, end);
		Log("dominator: %d", dom);

		uint32_t l = eval(start, dom - 1, success);
		uint32_t r = eval(dom + 1, end, success);

		Log("l: %d, r: %d", l, r);
		switch (tokens[dom].type) {
		case '+': return l + r;
		case '-': return l - r;
		case '*': return l * r;
		case '/': return l / r;
		case EQ:  return l == r;
		case NEQ: return l != r;
		case AND: return l && r;
		case OR:  return l || r;
		case GT:  return l > r;
		case GTE: return l >= r;
		case LT:  return l < r;
		case LTE: return l <= r;
		default:
			assert(0);
		}
	}
}

static inline bool is_neg_number(uint32_t i)
{
	if (tokens[i].type == '-') {
		if (i == 0) {
			return true;
		} else if (tokens[i - 1].type != DECIMAL &&
			   tokens[i - 1].type != HEX) {
			return true;
		}
	}
	return false;
}

static inline bool is_deref(uint32_t i)
{
	if (tokens[i].type == '*') {
		if (i == 0) {
			return true;
		} else if (tokens[i - 1].type != DECIMAL &&
			   tokens[i - 1].type != HEX) {
			return true;
		}
	}
	return false;
}

static void pre_process_tokens()
{
	int i = 0;
	for (; i < nr_token; ++i) {
		if (is_neg_number(i)) tokens[i].type = NEG;
		else if(is_deref(i)) tokens[i].type = DEREF;
	}

}

/* test cases:
 * input:
 *         p 1
 * output:
 *         1
 *
 * input:
 *         p 1 + 2
 * output:
 *         3
 *
 * input:
 *         p 1 + 2 * 3
 * output:
 *         7
 *
 * intput:
 *         p 7 - 6
 * output:
 *         1
 *
 * input:
 *         p 7 - (2 + 3)
 * output:
 *         2
 *
 * input:
 *         p 7 - 2 * 3
 * output:
 *         1
 */
uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
#ifdef DUMP_TOKENS
	dump_tokens();
#endif
	pre_process_tokens();
	return eval(0, nr_token - 1, success);
}

