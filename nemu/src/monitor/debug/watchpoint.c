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
		wp_pool[i].status = 0;
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
	p->status = 1;

	// add to busy list
	p->next = head;
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
		Log("==============%s: %u", p->expr, new_val);
		if (new_val) {
			printf("value of %s changes\n", p->expr);
			res = 1;
		}
	}
	return res;
}

void dump_wps()
{
	WP *p = head;
	printf("watchpoints list:\n");
	for (; p; p = p->next) {
		printf("[%02d] expr: %s\n", p->NO, p->expr);
	}
}

int delete_wp(uint32_t N)
{
	if (N > NR_WP ||
	    wp_pool[N].status == 0) return 1;

	free_wp(&wp_pool[N]);
	return 0;
}
