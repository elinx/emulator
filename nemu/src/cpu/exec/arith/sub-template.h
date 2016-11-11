#include "cpu/exec/template-start.h"

#define instr sub

static void do_execute () {
	DATA_TYPE result = op_dest->val - op_src->val;
	OPERAND_W(op_src, result);

	print_asm_template2();
}

make_instr_helper(r2rm)


#include "cpu/exec/template-end.h"
