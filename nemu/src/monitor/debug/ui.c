#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

static int cmd_step(char *args) {
	uint32_t s = args ? strtoul(args, 0, 0) : 1;
	cpu_exec(s);
	return 0;
}

static void dump_regs() {
	int i;
	for(i = R_EAX; i <= R_EDI; i ++) {
		printf("%s: 0x%08x\n", regsl[i], cpu.gpr[i]._32);
	}
	printf("eip: 0x%08x\n", cpu.eip);
	printf("eflags: 0x%08x\n", cpu.eflags);
	printf("   CF: %d\n", cpu.CF);
	printf("   PF: %d\n", cpu.PF);
	printf("   AF: %d\n", cpu.AF);
	printf("   ZF: %d\n", cpu.ZF);
	printf("   SF: %d\n", cpu.SF);
	printf("   TF: %d\n", cpu.TF);
	printf("   IF: %d\n", cpu.IF);
	printf("   DF: %d\n", cpu.DF);
	printf("   OF: %d\n", cpu.OF);
}

static int cmd_info(char *args) {
	switch (*args) {
	case 'r': dump_regs(); return 0;
	case 'w': dump_wps(); return 0;
	default: return 1;
	}
}

static int cmd_dump_mem(char *args) {
	unsigned int addr, len, i;

	sscanf(args, "%d 0x%x", &len, &addr);
	printf("dump memory start addr: 0x%08x len: %d\n", addr, len);
	for (i = 0; i < len; ++i) {
		if (!(i & 0xf)) printf("\n0x%08x: ", addr + i * 16);
		printf("0x%02x ", *(unsigned char *)hwa_to_va(addr + i));
	}
	printf("\n");

	return 0;
}

static int cmd_print(char *args) {
	int res;
	bool success;

	/*
         * assert(expr("1 + 2 * 3", &success) == 7);
	 * assert(success == true);
	 * assert(expr("7 - 2 * 3", &success) == 1);
	 * assert(success == true);
	 * assert(expr("(1+2) * (3+4)", &success) == 21);
	 * assert(success == true);
	 * assert(expr("-1 + 2", &success) == 1);
         */

	res = expr(args, &success);
	printf("0x%08x\n", res);

	return 0;
}

static int cmd_watch(char *args) {
	WP *wp = new_wp();
	assert(wp);
	wp->expr = strdup(args);

	return 0;
}

static int cmd_del_watch(char *args) {
	uint32_t nr = strtoul(args, 0, 0);
	return delete_wp(nr);
}

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },

	/* TODO: Add more commands */
	{ "si", "Single step: si [N]", cmd_step},
	{ "info", "dump informations with option: r(registers)", cmd_info},
	{ "x", "dump memory: x length addr", cmd_dump_mem},
	{ "p", "print expressions", cmd_print},
	{ "w", "watch expressions", cmd_watch},
	{ "d", "delete watch expressions: d N", cmd_del_watch},

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
