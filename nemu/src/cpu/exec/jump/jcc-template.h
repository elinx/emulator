#include "cpu/exec/template-start.h"

#define instr j

static void do_execute()
{
	fprintf(stderr, "2 dest: %u src: %u\n", REG(op_dest->reg), op_src->val);
	switch (ops_decoded.opcode) {
	case JCC_NA:
	/* case JCC_BE: */
		if (cpu.ZF && cpu.CF) {
			cpu.eip += op_src->val;
			if (ops_decoded.is_operand_size_16) {
				cpu.eip &= 0xffff;
			}
		}
		break;
	case JCC_E:
	/* case JCC_Z: */
		if (cpu.ZF) {
			cpu.eip += op_src->val;
			if (ops_decoded.is_operand_size_16) {
				cpu.eip &= 0xffff;
			}
		}
	default:
		break;
	}

	print_asm_template1();
}

make_instr_helper(c)

#include "cpu/exec/template-end.h"
