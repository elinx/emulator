#include "cpu/exec/template-start.h"

#define instr sub

static void do_execute () {
	DATA_TYPE result = REG(op_dest->reg) - op_src->val;
	fprintf(stderr, "1 dest: %u src: %u", op_dest->val, op_src->val);
	OPERAND_W(op_dest, result);
	fprintf(stderr, "2 dest: %u src: %u", op_dest->val, op_src->val);

	print_asm_template2();
}

make_instr_helper(i2rm)
make_instr_helper(r2rm)


#include "cpu/exec/template-end.h"
