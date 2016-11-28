#include "cpu/exec/template-start.h"

#define instr cmp

static void do_execute()
{
	int32_t tmp = op_dest->val - op_src->val;
	cpu.AF = 0;
	cpu.OF = 0;
	cpu.CF = tmp < 0;
	cpu.SF = tmp & 0x7fffffff;
	cpu.ZF = tmp == 0 ? 1 : 0;
	cpu.PF = parity(tmp & 0xff);

	print_asm_template2();
}

make_instr_helper(i2rm)

#include "cpu/exec/template-end.h"
