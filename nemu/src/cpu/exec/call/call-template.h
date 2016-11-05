#include "cpu/exec/template-start.h"

#define instr call

static void do_execute()
{
	// 1. push(eip), then esp decrease by ##?
	REG(R_ESP) -= DATA_BYTE;                            // push EIP
	MEM_W(REG(R_ESP), cpu.eip);
	// 2. eip is assigned by (eip + ##) + rel32/rel16
	cpu.eip += op_src->val;        // EIP = EIP' + rel32/rel16
}

make_instr_helper(i)

#include "cpu/exec/template-end.h"
