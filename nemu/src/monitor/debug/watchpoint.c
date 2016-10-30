#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP *new_wp()
{
	// get a new wp from free list
	WP *p = free_;
	assert(p);
	free_ = free_->next;

	// add to busy list
	p->next = head->next;
	head = p;

	return p;
}

void free_wp(WP *wp)
{
	wp->next = free_;
	free_ = wp;
}

// scan watchpoints list, if a the value of the watchpoint
// changes stop the emulator and print the information
uint32_t scan_watchpoints()
{
	WP *p = head;
	bool s;
	uint32_t res = 0;
	for (; p; p = p->next) {
		uint32_t new_val = expr(p->expr, &s);
		if (new_val) {
			printf("value of %s changes\n", p->expr);
			res = 1;
		}
	}
	return res;
}
