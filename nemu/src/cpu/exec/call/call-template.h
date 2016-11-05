#include "cpu/exec/template-start.h"

#define instr call

static void do_execute()
{
	// 1. push(eip), then esp decrease by ##?
	REG(R_ESP) -= DATA_BYTE;                                     // decscrease ESP
	MEM_W(REG(R_ESP), cpu.eip + DATA_BYTE + OPCODE_SIZE);        // save return address

	cpu.eip += op_src->val;                                      // EIP += rel32/rel16
	print_asm("call 0x%x", cpu.eip + DATA_BYTE + OPCODE_SIZE);
}

make_instr_helper(c)

#include "cpu/exec/template-end.h"
