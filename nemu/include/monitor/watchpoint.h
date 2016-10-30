#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;

	/* TODO: Add more members if necessary */
	char *expr;
	int status;

} WP;

WP *new_wp();
void free_wp(WP *wp);
uint32_t scan_watchpoints();
void dump_wps();
int delete_wp(uint32_t N);

#endif
