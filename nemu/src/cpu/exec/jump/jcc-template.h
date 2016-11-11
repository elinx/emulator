#include "cpu/exec/template-start.h"

#define instr jcc

static void do_execute()
{
	if (cpu.ZF) {
		cpu.eip += op_src->val;
		if (ops_decoded.is_operand_size_16) {
			cpu.eip &= 0xffff;
		}
	}
	print_asm_template1();
}

make_instr_helper(c)

#include "cpu/exec/template-end.h"