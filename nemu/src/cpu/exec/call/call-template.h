#include "cpu/exec/template-start.h"

#define instr call

static void do_execute()
{
	// 1. push(eip), then esp decrease by ##?
	REG(R_ESP) -= DATA_BYTE;                    // push EIP
	MEM_W(REG(R_ESP), cpu.eip);
	cpu.eip += op_src->val;                     // EIP += rel32/rel16
	print_asm_template1();
}

make_instr_helper(c)

#include "cpu/exec/template-end.h"
