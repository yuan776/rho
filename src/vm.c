#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include "compiler.h"
#include "opcodes.h"
#include "str.h"
#include "strobject.h"
#include "err.h"
#include "code.h"
#include "util.h"
#include "vm.h"

VM *vm_new(void)
{
	VM *vm = malloc(sizeof(VM));
	return vm;
}

void vm_free(VM *vm)
{
	free(vm);
}

static void vm_pushframe(
	VM *vm,
	const char *name,
	CodeObject *co,
	size_t nglobal
);

/* pushes top-level frame */
static void vm_push_module_frame(VM *vm, Code *code);

static void vm_popframe(VM *vm);

static void vm_pushframe(VM *vm,
                         const char *name,
                         CodeObject *co,
                         size_t nglobal)
{
	/* XXX: maintain a bytecode address to Frame
	 * dictionary to avoid creating frames anew on
	 * each function call.
	 */
	nglobal = 0;  // XXX avoid unused warning
	//const size_t nconst = constants.length;
	//const size_t nlocal = local_symtab.length;

	Frame *frame = malloc(sizeof(Frame));
	frame->name = name;
	frame->co = co;
	//frame->constants = realloc(constants.array, sizeof(Value) * (nconst + nlocal + DEFAULT_VSTACK_DEPTH));
	frame->globals = NULL;  /* for now */
	frame->locals = calloc(co->names.length, sizeof(Value));
	//frame->local_symtab = local_symtab;
	frame->valuestack = malloc(DEFAULT_VSTACK_DEPTH * sizeof(Value));
	frame->stack_capacity = DEFAULT_VSTACK_DEPTH;

	frame->prev = vm->callstack;
	vm->callstack = frame;
}

static void vm_popframe(VM *vm)
{
	Frame *frame = vm->callstack;
	vm->callstack = frame->prev;
	//free(frame->local_symtab.array);

	//for (Value *v = frame->constants; v < frame->locals; ++v) {
		//destroy(v);
	//}

	/*
	for (Value *v = frame->locals; v < frame->valuestack; ++v) {
		decref(v);
	}
	*/

	free(frame);
}

static void eval_frame(VM *vm);
static void print(Value *v);

// TODO: move this out of vm.c
void execute(FILE *compiled)
{
	fseek(compiled, 0L, SEEK_END);
	const size_t code_size = ftell(compiled);
	fseek(compiled, 0L, SEEK_SET);

	VM *vm = vm_new();

	Code code_0;
	code_init(&code_0, code_size);
	fread(code_0.bc, sizeof(char), code_size, compiled);
	code_0.size += code_size;

	Code code = code_0;

	/* verify magic code */
	for (size_t i = 0; i < magic_size; i++) {
		if (code_fwd(&code) != magic[i]) {
			fatal_error("verification error");
		}
	}

	vm_push_module_frame(vm, &code);
	eval_frame(vm);
	vm_popframe(vm);

	vm_free(vm);
	code_dealloc(&code_0);
}

/*
 * Assumes the symbol table and constant table
 * have not yet been read.
 */
static void vm_push_module_frame(VM *vm, Code *code)
{
	CodeObject *co = codeobj_make(code);

	vm_pushframe(
		vm,
		"<module>",
		co,
		0
	);
}

/* utility function for pushing onto value stack */
static void frame_vstack_push(Frame *frame, Value v, Value **stack)
{
	Value *stack_ptr = *stack;
	if ((size_t)(stack_ptr - frame->valuestack) == frame->stack_capacity) {
		assert(0);
		// TODO: this results in invalid pointers
		//const size_t new_capacity = (((stack_ptr - frame->constants) * 3)/2 + 1) * sizeof(Value);
		//frame->constants = realloc(frame->constants, new_capacity);
		//frame->stack_capacity = new_capacity;
	}

	**stack = v;
	++*stack;
}

static void eval_frame(VM *vm)
{
#define STACK_POP() (--stack)
#define STACK_PEEK() (&stack[-1])
#define STACK_PUSH(v) (frame_vstack_push(frame, (v), &stack))

	Frame *frame = vm->callstack;

	/* position in the bytecode */
	unsigned int pos = 0;

	struct str_array symbols = frame->co->names;
	Value *constants = frame->co->consts.array;
	byte *bc = frame->co->bc;
	Value *stack = frame->valuestack;

	while (true) {
		const byte opcode = bc[pos++];

		switch (opcode) {
		case INS_NOP:
			break;
		case INS_ILOAD: {
			const int val = read_int(bc + pos);
			Value intObj = {VAL_TYPE_INT, .data = {.i = val}};
			STACK_PUSH(intObj);
			pos += INT_SIZE;
			break;
		}
		case INS_FLOAD: {
			const double val = read_double(bc + pos);
			Value floatObj = {VAL_TYPE_FLOAT, .data = {.f = val}};
			STACK_PUSH(floatObj);
			pos += DOUBLE_SIZE;
			break;
		}
		case INS_LOAD_CONST: {
			const unsigned int val = bc[pos++];
			STACK_PUSH(constants[val]);
			break;
		}
		case INS_ADD: {
			Value *v1 = STACK_POP();

			if (v1->type == VAL_TYPE_OBJECT) {
				Value *v2 = STACK_POP();
				Object *o = v2->data.o;
				STACK_PUSH(resolve_add(o->class)(v2, v1));
				break;
			}

			Value *v2 = STACK_PEEK();

			int t1 = v1->type;
			int t2 = v2->type;

			if (t1 == VAL_TYPE_INT) {
				if (t2 == VAL_TYPE_INT) {
					v2->data.i += v1->data.i;
				}
				else if (t2 == VAL_TYPE_FLOAT) {
					v2->data.f += v1->data.i;
				}
			}
			else if (t1 == VAL_TYPE_FLOAT) {
				if (t2 == VAL_TYPE_INT) {
					v2->type = VAL_TYPE_FLOAT;
					v2->data.f = v2->data.i + v1->data.f;
				}
				else if (t2 == VAL_TYPE_FLOAT) {
					v2->data.f += v1->data.f;
				}
			}

			break;
		}
		case INS_SUB: {
			Value *v1 = STACK_POP();
			Value *v2 = STACK_PEEK();

			int t1 = v1->type;
			int t2 = v2->type;

			if (t1 == VAL_TYPE_INT) {
				if (t2 == VAL_TYPE_INT) {
					v2->data.i -= v1->data.i;
				}
				else if (t2 == VAL_TYPE_FLOAT) {
					v2->data.f -= v1->data.i;
				}
			}
			else if (t1 == VAL_TYPE_FLOAT) {
				if (t2 == VAL_TYPE_INT) {
					v2->type = VAL_TYPE_FLOAT;
					v2->data.f = v2->data.i - v1->data.f;
				}
				else if (t2 == VAL_TYPE_FLOAT) {
					v2->data.f -= v1->data.f;
				}
			}

			break;
		}
		case INS_MUL: {
			Value *v1 = STACK_POP();
			Value *v2 = STACK_PEEK();

			int t1 = v1->type;
			int t2 = v2->type;

			if (t1 == VAL_TYPE_INT) {
				if (t2 == VAL_TYPE_INT) {
					v2->data.i *= v1->data.i;
				}
				else if (t2 == VAL_TYPE_FLOAT) {
					v2->data.f *= v1->data.i;
				}
			}
			else if (t1 == VAL_TYPE_FLOAT) {
				if (t2 == VAL_TYPE_INT) {
					v2->type = VAL_TYPE_FLOAT;
					v2->data.f = v2->data.i * v1->data.f;
				}
				else if (t2 == VAL_TYPE_FLOAT) {
					v2->data.f *= v1->data.f;
				}
			}

			break;
		}
		case INS_DIV: {
			Value *v1 = STACK_POP();
			Value *v2 = STACK_PEEK();

			int t1 = v1->type;
			int t2 = v2->type;

			if (t1 == VAL_TYPE_INT) {
				if (t2 == VAL_TYPE_INT) {
					v2->data.i /= v1->data.i;
				}
				else if (t2 == VAL_TYPE_FLOAT) {
					v2->data.f /= v1->data.i;
				}
			}
			else if (t1 == VAL_TYPE_FLOAT) {
				if (t2 == VAL_TYPE_INT) {
					v2->type = VAL_TYPE_FLOAT;
					v2->data.f = v2->data.i / v1->data.f;
				}
				else if (t2 == VAL_TYPE_FLOAT) {
					v2->data.f /= v1->data.f;
				}
			}

			break;
		}
		case INS_MOD: {
			Value *v1 = STACK_POP();
			Value *v2 = STACK_PEEK();

			if (v1->type == VAL_TYPE_INT && v2->type == VAL_TYPE_INT) {
				v2->data.i %= v1->data.i;
			}

			break;
		}
		case INS_POW: {
			Value *v1 = STACK_POP();
			Value *v2 = STACK_PEEK();

			int t1 = v1->type;
			int t2 = v2->type;

			if (t1 == VAL_TYPE_INT) {
				if (t2 == VAL_TYPE_INT) {
					v2->data.i = pow(v2->data.i, v1->data.i);
				}
				else if (t2 == VAL_TYPE_FLOAT) {
					v2->data.f = pow(v2->data.f, v1->data.i);
				}
			}
			else if (t1 == VAL_TYPE_FLOAT) {
				if (t2 == VAL_TYPE_INT) {
					v2->type = VAL_TYPE_FLOAT;
					v2->data.f = pow(v2->data.i, v1->data.f);
				}
				else if (t2 == VAL_TYPE_FLOAT) {
					v2->data.f = pow(v2->data.f, v1->data.f);
				}
			}

			break;
		}
		case INS_UPLUS: {
			/* NOP */
			break;
		}
		case INS_UMIN: {
			Value *v = STACK_PEEK();

			switch (v->type) {
			case VAL_TYPE_EMPTY:
				INTERNAL_ERROR();
			case VAL_TYPE_INT:
				v->data.i = -v->data.i;
				break;
			case VAL_TYPE_FLOAT:
				v->data.f = -v->data.f;
				break;
			case VAL_TYPE_OBJECT:
				/* TODO */
				break;
			}
			break;
		}
		case INS_STORE: {
			Value *v = STACK_POP();
			const unsigned int id = read_int(bc + pos);
			frame->locals[id] = *v;
			incref(v);
			pos += INT_SIZE;
			break;
		}
		case INS_LOAD: {
			const unsigned int id = read_int(bc + pos);

			if (frame->locals[id].type == VAL_TYPE_EMPTY) {
				runtime_error(RT_ERR_UNBOUND_VAR, symbols.array[id].str);
			}

			STACK_PUSH(frame->locals[id]);
			pos += INT_SIZE;
			break;
		}
		case INS_PRINT: {
			print(STACK_POP());
			break;
		}
		case INS_JMP: {
			const unsigned int jmp = bc[pos++];
			pos += jmp;
			break;
		}
		case INS_JMP_BACK: {
			const unsigned int jmp = bc[pos++];
			pos -= jmp;
			break;
		}
		case INS_JMP_IF_TRUE: {
			Value *v = STACK_POP();
			const unsigned int jmp = bc[pos++];
			if (v->data.i != 0) {
				pos += jmp;
			}
			break;
		}
		case INS_JMP_IF_FALSE: {
			Value *v = STACK_POP();
			const unsigned int jmp = bc[pos++];
			if (v->data.i == 0) {
				pos += jmp;
			}
			break;
		}
		case INS_JMP_BACK_IF_TRUE: {
			Value *v = STACK_POP();
			const unsigned int jmp = bc[pos++];
			if (v->data.i != 0) {
				pos -= jmp;
			}
			break;
		}
		case INS_JMP_BACK_IF_FALSE: {
			Value *v = STACK_POP();
			const unsigned int jmp = bc[pos++];
			if (v->data.i == 0) {
				pos -= jmp;
			}
			break;
		}
		case INS_CALL: {
			// TODO
			//#define DYN_ARG_ARRAY_THRESHOLD 16
			//const unsigned int nargs = code[pos++];
			//Value args[DYN_ARG_ARRAY_THRESHOLD]
			Value *v = STACK_POP();
			CodeObject *co = v->data.o;
			vm_pushframe(vm, "test", co, 10);
			eval_frame(vm);
			vm_popframe(vm);
			break;
		}
		case INS_RETURN: {
			return;  // TODO: proper return statements
		}
		default: {
			UNEXP_BYTE(opcode);
			break;
		}
		}
	}

#undef STACK_POP
#undef STACK_PEEK
#undef STACK_PUSH
}

static void print(Value *v)
{
	switch (v->type) {
	case VAL_TYPE_EMPTY:
		fatal_error("unexpected value type VAL_TYPE_EMPTY");
		break;
	case VAL_TYPE_INT:
		printf("%d\n", v->data.i);
		break;
	case VAL_TYPE_FLOAT:
		printf("%f\n", v->data.f);
		break;
	case VAL_TYPE_OBJECT: {
		Object *o = v->data.o;
		printf("%s\n", ((StrObject *) o->class->str(v).data.o)->str.value);
		break;
	}
	}
}
