#include "cpu/exec/template-start.h"

#define instr add

static void do_execute () {
	DATA_TYPE result = op_dest->val + op_src->val;
	OPERAND_W(op_dest, result);

	/* TODO: Update EFLAGS. */
	cpu.AF = 0;
	cpu.OF = 0;
	cpu.CF = result < (op_dest->val + op_src->val);
	cpu.SF = result & 0x7fffffff;
	cpu.ZF = result == 0 ? 1 : 0;
	cpu.PF = parity(result & 0xff);


	print_asm_template1();
}

make_instr_helper(r2rm)

#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(r)
#endif

#include "cpu/exec/template-end.h"
