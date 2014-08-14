#ifndef OPCODE_H
#define OPCODE_H

typedef enum {
	INS_NOP = 0x30,
	INS_ILOAD,
	INS_FLOAD,
	INS_LOAD_CONST,
	INS_ADD,
	INS_SUB,
	INS_MUL,
	INS_DIV,
	INS_MOD,
	INS_POW,
	INS_UPLUS,
	INS_UMIN,
	INS_STORE,
	INS_LOAD,
	INS_PRINT,
	INS_JMP,
	INS_JMP_BACK,
	INS_JMP_IF_TRUE,
	INS_JMP_IF_FALSE,
	INS_JMP_BACK_IF_TRUE,
	INS_JMP_BACK_IF_FALSE,
	INS_CALL,
	INS_RETURN
} Opcode;

typedef enum {
	ST_ENTRY_BEGIN = 0x10,
	ST_ENTRY_END
} STCode;

typedef enum {
	CT_ENTRY_BEGIN = 0x20,
	CT_ENTRY_INT,
	CT_ENTRY_FLOAT,
	CT_ENTRY_STRING,
	CT_ENTRY_CODEOBJ,
	CT_ENTRY_END
} CTCode;

#endif /* OPCODE_H */
