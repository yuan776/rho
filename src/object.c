#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "attr.h"
#include "err.h"
#include "util.h"
#include "intobject.h"
#include "floatobject.h"
#include "object.h"

static void obj_init(Value *this, Value *args, size_t nargs)
{
	UNUSED(this);
	UNUSED(args);
	UNUSED(nargs);
}

static int obj_hash(Value *this)
{
	return hash_ptr(objvalue(this));
}

static bool obj_eq(Value *this, Value *other)
{
	if (getclass(other) != &obj_class) {
		return false;
	}
	return objvalue(this) == objvalue(other);
}

static Str *obj_str(Value *this)
{
	char *buf = malloc(32);
	sprintf(buf, "<%s at %p>", getclass(this)->name, objvalue(this));
	Str *str = str_new(buf, strlen(buf));
	str->freeable = 1;
	return str;
}

static bool obj_nonzero(Value *this)
{
	UNUSED(this);
	return true;
}

static void obj_free(Value *this)
{
	free(objvalue(this));
}

struct num_methods obj_num_methods = {
	NULL,    /* plus */
	NULL,    /* minus */
	NULL,    /* abs */

	NULL,    /* add */
	NULL,    /* sub */
	NULL,    /* mul */
	NULL,    /* div */
	NULL,    /* mod */
	NULL,    /* pow */

	NULL,    /* not */
	NULL,    /* and */
	NULL,    /* or */
	NULL,    /* xor */
	NULL,    /* shiftl */
	NULL,    /* shiftr */

	NULL,    /* iadd */
	NULL,    /* isub */
	NULL,    /* imul */
	NULL,    /* idiv */
	NULL,    /* imod */
	NULL,    /* ipow */

	NULL,    /* iand */
	NULL,    /* ior */
	NULL,    /* ixor */
	NULL,    /* ishiftl */
	NULL,    /* ishiftr */

	obj_nonzero,    /* nonzero */

	NULL,    /* to_int */
	NULL,    /* to_float */
};

struct seq_methods obj_seq_methods = {
	NULL,    /* len */
	NULL,    /* get */
	NULL,    /* set */
	NULL,    /* contains */
	NULL,    /* iter */
	NULL,    /* iternext */
};

Class obj_class = {
	.name = "Object",

	.super = NULL,

	.instance_size = sizeof(Object),

	.init = obj_init,
	.del = obj_free,

	.eq = obj_eq,
	.hash = obj_hash,
	.cmp = NULL,
	.str = obj_str,
	.call = NULL,

	.num_methods = &obj_num_methods,
	.seq_methods = &obj_seq_methods,

	.members = NULL,
	.methods = NULL
};

Class *getclass(Value *v)
{
	if (v == NULL) {
		return NULL;
	}

	switch (v->type) {
	case VAL_TYPE_INT:
		return &int_class;
	case VAL_TYPE_FLOAT:
		return &float_class;
	case VAL_TYPE_OBJECT: {
		const Object *o = objvalue(v);
		return o->class;
	case VAL_TYPE_EMPTY:
	case VAL_TYPE_ERROR:
	case VAL_TYPE_UNSUPPORTED_TYPES:
	case VAL_TYPE_DIV_BY_ZERO:
		INTERNAL_ERROR();
		return NULL;
	}
	}
}

/*
 * Generic "is a" -- checks if the given object is
 * an instance of the given class or any of its super-
 * classes. This is subject to change if support for
 * multiple-inheritance is ever added.
 */
bool is_a(Value *v, Class *class)
{
	return class != NULL && ((getclass(v) == class) || is_a(v, class->super));
}

#define MAKE_METHOD_RESOLVER_DIRECT(name, type) \
type resolve_##name(Class *class) { \
	Class *target = class; \
	type op; \
	while (target != NULL && (op = target->name) == NULL) { \
		target = target->super; \
	} \
\
	if (target == NULL) { \
		return NULL; \
	} \
\
	class->name = op; \
	return op; \
}

#define MAKE_METHOD_RESOLVER(name, category, type) \
type resolve_##name(Class *class) { \
	Class *target = class; \
	type op; \
	while (target != NULL && (op = target->category->name) == NULL) { \
		target = target->super; \
	} \
\
	if (target == NULL) { \
		return NULL; \
	} \
\
	class->category->name = op; \
	return op; \
}

MAKE_METHOD_RESOLVER_DIRECT(eq, BoolBinOp)
MAKE_METHOD_RESOLVER_DIRECT(hash, IntUnOp)
MAKE_METHOD_RESOLVER_DIRECT(cmp, BinOp)
MAKE_METHOD_RESOLVER_DIRECT(str, StrUnOp)
MAKE_METHOD_RESOLVER_DIRECT(call, CallFunc)

MAKE_METHOD_RESOLVER(plus, num_methods, UnOp)
MAKE_METHOD_RESOLVER(minus, num_methods, UnOp)
MAKE_METHOD_RESOLVER(abs, num_methods, UnOp)
MAKE_METHOD_RESOLVER(add, num_methods, BinOp)
MAKE_METHOD_RESOLVER(sub, num_methods, BinOp)
MAKE_METHOD_RESOLVER(mul, num_methods, BinOp)
MAKE_METHOD_RESOLVER(div, num_methods, BinOp)
MAKE_METHOD_RESOLVER(mod, num_methods, BinOp)
MAKE_METHOD_RESOLVER(pow, num_methods, BinOp)
MAKE_METHOD_RESOLVER(not, num_methods, UnOp)
MAKE_METHOD_RESOLVER(and, num_methods, BinOp)
MAKE_METHOD_RESOLVER(or, num_methods, BinOp)
MAKE_METHOD_RESOLVER(xor, num_methods, BinOp)
MAKE_METHOD_RESOLVER(shiftl, num_methods, BinOp)
MAKE_METHOD_RESOLVER(shiftr, num_methods, BinOp)
MAKE_METHOD_RESOLVER(iadd, num_methods, BinOp)
MAKE_METHOD_RESOLVER(isub, num_methods, BinOp)
MAKE_METHOD_RESOLVER(imul, num_methods, BinOp)
MAKE_METHOD_RESOLVER(idiv, num_methods, BinOp)
MAKE_METHOD_RESOLVER(imod, num_methods, BinOp)
MAKE_METHOD_RESOLVER(ipow, num_methods, BinOp)
MAKE_METHOD_RESOLVER(iand, num_methods, BinOp)
MAKE_METHOD_RESOLVER(ior, num_methods, BinOp)
MAKE_METHOD_RESOLVER(ixor, num_methods, BinOp)
MAKE_METHOD_RESOLVER(ishiftl, num_methods, BinOp)
MAKE_METHOD_RESOLVER(ishiftr, num_methods, BinOp)
MAKE_METHOD_RESOLVER(nonzero, num_methods, BoolUnOp)
MAKE_METHOD_RESOLVER(to_int, num_methods, UnOp)
MAKE_METHOD_RESOLVER(to_float, num_methods, UnOp)

MAKE_METHOD_RESOLVER(len, seq_methods, LenFunc)
MAKE_METHOD_RESOLVER(get, seq_methods, BinOp)
MAKE_METHOD_RESOLVER(set, seq_methods, SeqSetFunc)
MAKE_METHOD_RESOLVER(contains, seq_methods, BoolBinOp)
MAKE_METHOD_RESOLVER(iter, seq_methods, UnOp)
MAKE_METHOD_RESOLVER(iternext, seq_methods, UnOp)

#undef MAKE_METHOD_RESOLVER_DIRECT
#undef MAKE_METHOD_RESOLVER

Value instantiate(Class *class, Value *args, size_t nargs)
{
	if (class == &int_class) {
		return makeint(0);
	} else if (class == &float_class) {
		return makefloat(0);
	} else {
		if (class->init == NULL) {
			type_error_cannot_instantiate(class);
		}

		Value instance = makeobj(malloc(class->instance_size));
		class->init(&instance, args, nargs);
		return instance;
	}
}

void retaino(Object *o)
{
	++o->refcnt;
}

void releaseo(Object *o)
{
	if (--o->refcnt == 0) {
		destroyo(o);
	}
}

void destroyo(Object *o)
{
	o->class->del(&makeobj(o));
}

void retain(Value *v)
{
	if (v == NULL || v->type != VAL_TYPE_OBJECT) {
		return;
	}

	retaino(objvalue(v));
}


void release(Value *v)
{
	if (v == NULL || v->type != VAL_TYPE_OBJECT) {
		return;
	}

	releaseo(objvalue(v));
}

void destroy(Value *v)
{
	if (v == NULL || v->type != VAL_TYPE_OBJECT) {
		return;
	}

	Object *o = objvalue(v);
	o->class->del(v);
}

void class_init(Class *class)
{
	/* initialize attributes */
	size_t max_size = 0;

	if (class->members != NULL) {
		for (size_t i = 0; class->members[i].name != NULL; i++) {
			++max_size;
		}
	}

	if (class->methods != NULL) {
		for (struct attr_method *m = class->methods; m->name != NULL; m++) {
			++max_size;
		}
	}

	attr_dict_init(&class->attr_dict, max_size);
	attr_dict_register_members(&class->attr_dict, class->members);
	attr_dict_register_methods(&class->attr_dict, class->methods);
}
