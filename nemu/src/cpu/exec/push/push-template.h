#include "cpu/exec/template-start.h"

#define instr push

static void do_execute()
{
	// 1. push(eip), then esp decrease by ##?
	REG(R_ESP) -= DATA_BYTE;                      // decscrease ESP
	MEM_W(REG(R_ESP), op_src->val);
	print_asm_template1();
}

make_instr_helper(r)

#include "cpu/exec/template-end.h"
