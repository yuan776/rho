#include <stdlib.h>
#include <math.h>
#include "object.h"
#include "util.h"
#include "err.h"
#include "intobject.h"

#define TYPE_ERR_STR(op) "invalid operator types for operator " #op "."

#define INT_BINOP_FUNC_BODY(op) \
	if (isint(other)) { \
		return makeint(intvalue(this) op intvalue(other)); \
	} else if (isfloat(other)) { \
		return makefloat(intvalue(this) op floatvalue(other)); \
	} else { \
		type_error(TYPE_ERR_STR(op)); \
		return SENTINEL; \
	}

#define INT_BINOP_FUNC_BODY_NOFLOAT(op) \
	if (isint(other)) { \
		return makeint(intvalue(this) op intvalue(other)); \
	} else { \
		type_error(TYPE_ERR_STR(op)); \
		return SENTINEL; \
	}

#define INT_IBINOP_FUNC_BODY(op) \
	if (isint(other)) { \
		this->data.i = this->data.i op intvalue(other); \
		return *this; \
	} else if (isfloat(other)) { \
		this->type = VAL_TYPE_FLOAT; \
		this->data.f = this->data.i op floatvalue(other); \
		return *this; \
	} else { \
		type_error(TYPE_ERR_STR(op)); \
		return SENTINEL; \
	}

#define INT_IBINOP_FUNC_BODY_NOFLOAT(op) \
	if (isint(other)) { \
		this->data.i = this->data.i op intvalue(other); \
		return *this; \
	} else { \
		type_error(TYPE_ERR_STR(op)); \
		return SENTINEL; \
	}

static Value int_eq(Value *this, Value *other)
{
	if (isint(other)) {
		return makeint(intvalue(this) == intvalue(other));
	} else if (isfloat(other)) {
		return makeint(intvalue(this) == floatvalue(other));
	} else {
		return makeint(0);
	}
}

static Value int_hash(Value *this)
{
	return makeint(hash_int(intvalue(this)));
}

static Value int_cmp(Value *this, Value *other)
{
	const int x = intvalue(this);
	if (isint(other)) {
		const int y = intvalue(other);
		return makeint((x < y) ? -1 : ((x == y) ? 0 : 1));
	} else if (isfloat(other)) {
		const double y = floatvalue(other);
		return makeint((x < y) ? -1 : ((x == y) ? 0 : 1));
	} else {
		type_error(TYPE_ERR_STR(cmp));
		return SENTINEL;
	}
}

static Value int_plus(Value *this)
{
	return *this;
}

static Value int_minus(Value *this)
{
	return makeint(-intvalue(this));
}

static Value int_abs(Value *this)
{
	return makeint(abs(intvalue(this)));
}

static Value int_add(Value *this, Value *other)
{
	INT_BINOP_FUNC_BODY(+)
}

static Value int_sub(Value *this, Value *other)
{
	INT_BINOP_FUNC_BODY(-)
}

static Value int_mul(Value *this, Value *other)
{
	INT_BINOP_FUNC_BODY(*)
}

static Value int_div(Value *this, Value *other)
{
	INT_BINOP_FUNC_BODY(/)
}

static Value int_mod(Value *this, Value *other)
{
	INT_BINOP_FUNC_BODY_NOFLOAT(%)
}

static Value int_pow(Value *this, Value *other)
{
	if (isint(other)) {
		return makeint(pow(intvalue(this), intvalue(other)));
	} else if (isfloat(other)) {
		return makefloat(pow(intvalue(this), floatvalue(other)));
	} else {
		type_error(TYPE_ERR_STR(**)); \
		return SENTINEL;
	}
}

static Value int_not(Value *this)
{
	return makeint(~intvalue(this));
}

static Value int_and(Value *this, Value *other)
{
	INT_BINOP_FUNC_BODY_NOFLOAT(&)
}

static Value int_or(Value *this, Value *other)
{
	INT_BINOP_FUNC_BODY_NOFLOAT(|)
}

static Value int_xor(Value *this, Value *other)
{
	INT_BINOP_FUNC_BODY_NOFLOAT(^)
}

static Value int_shiftl(Value *this, Value *other)
{
	INT_BINOP_FUNC_BODY_NOFLOAT(<<)
}

static Value int_shiftr(Value *this, Value *other)
{
	INT_BINOP_FUNC_BODY_NOFLOAT(>>)
}

static Value int_iadd(Value *this, Value *other)
{
	INT_IBINOP_FUNC_BODY(+)
}

static Value int_isub(Value *this, Value *other)
{
	INT_IBINOP_FUNC_BODY(-)
}

static Value int_imul(Value *this, Value *other)
{
	INT_IBINOP_FUNC_BODY(*)
}

static Value int_idiv(Value *this, Value *other)
{
	INT_IBINOP_FUNC_BODY(/)
}

static Value int_imod(Value *this, Value *other)
{
	INT_IBINOP_FUNC_BODY_NOFLOAT(%)
}

static Value int_ipow(Value *this, Value *other)
{
	if (isint(other)) {
		this->data.i = pow(this->data.i, intvalue(other));
		return *this;
	} else if (isfloat(other)) {
		this->type = VAL_TYPE_FLOAT;
		this->data.f = pow(this->data.i, floatvalue(other));
		return *this;
	} else {
		type_error(TYPE_ERR_STR(**));
		return SENTINEL;
	}
}

static Value int_iand(Value *this, Value *other)
{
	INT_IBINOP_FUNC_BODY_NOFLOAT(&)
}

static Value int_ior(Value *this, Value *other)
{
	INT_IBINOP_FUNC_BODY_NOFLOAT(|)
}

static Value int_ixor(Value *this, Value *other)
{
	INT_IBINOP_FUNC_BODY_NOFLOAT(^)
}

static Value int_ishiftl(Value *this, Value *other)
{
	INT_IBINOP_FUNC_BODY_NOFLOAT(<<)
}

static Value int_ishiftr(Value *this, Value *other)
{
	INT_IBINOP_FUNC_BODY_NOFLOAT(>>)
}

static Value int_nonzero(Value *this)
{
	return makeint(intvalue(this) != 0);
}

static Value int_to_int(Value *this)
{
	return *this;
}

static Value int_to_float(Value *this)
{
	return makefloat(intvalue(this));
}

struct num_methods int_num_methods = {
	int_plus,    /* plus */
	int_minus,    /* minus */
	int_abs,    /* abs */

	int_add,    /* add */
	int_sub,    /* sub */
	int_mul,    /* mul */
	int_div,    /* div */
	int_mod,    /* mod */
	int_pow,    /* pow */

	int_not,    /* not */
	int_and,    /* and */
	int_or,    /* or */
	int_xor,    /* xor */
	int_shiftl,    /* shiftl */
	int_shiftr,    /* shiftr */

	int_iadd,    /* iadd */
	int_isub,    /* isub */
	int_imul,    /* imul */
	int_idiv,    /* idiv */
	int_imod,    /* imod */
	int_ipow,    /* ipow */

	int_iand,    /* iand */
	int_ior,    /* ior */
	int_ixor,    /* ixor */
	int_ishiftl,    /* ishiftl */
	int_ishiftr,    /* ishiftr */

	int_nonzero,    /* nonzero */

	int_to_int,    /* to_int */
	int_to_float,    /* to_float */
};

Class int_class = {
	.name = "Int",

	.super = &obj_class,

	.new = NULL,
	.del = NULL,

	.eq = int_eq,
	.hash = int_hash,
	.cmp = int_cmp,
	.str = NULL,
	.call = NULL,

	.num_methods = &int_num_methods,
	.seq_methods  = NULL
};
