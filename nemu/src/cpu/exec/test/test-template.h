#include "cpu/exec/template-start.h"

#define instr test

static void do_execute()
{
	uint32_t tmp = op_src->reg & op_dest->reg;
	cpu.OF = 0;
	cpu.CF = 0;
	cpu.SF = tmp & 0x7fffffff;
	cpu.ZF = tmp == 0 ? 1 : 0;
	cpu.PF = parity(tmp & 0xff);
	print_asm_template2();
}

make_instr_helper(rm2r)

#include "cpu/exec/template-end.h"
