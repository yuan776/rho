#include <stdlib.h>
#include "object.h"
#include "method.h"

Value methobj_make(Object *binder, MethodFunc meth_func)
{
	Method *meth = malloc(sizeof(Method));
	meth->base = OBJ_INIT(&method_class);
	retaino(binder);
	meth->binder = binder;
	meth->method = meth_func;
	return makeobj(meth);
}

static void methobj_free(Value *this)
{
	Method *meth = objvalue(this);
	releaseo(meth->binder);
	meth->base.class->super->del(this);
}

static Value methobj_invoke(Value *this, Value *args, size_t nargs)
{
	Method *meth = objvalue(this);
	return meth->method(&makeobj(meth->binder), args, nargs);
}

struct num_methods meth_num_methods = {
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

	NULL,    /* nonzero */

	NULL,    /* to_int */
	NULL,    /* to_float */
};

struct seq_methods meth_seq_methods = {
	NULL,    /* len */
	NULL,    /* conctat */
	NULL,    /* get */
	NULL,    /* set */
	NULL,    /* contains */
	NULL,    /* iter */
	NULL,    /* iternext */
};

Class method_class = {
	.name = "Method",

	.super = &obj_class,

	.new = NULL,
	.del = methobj_free,

	.eq = NULL,
	.hash = NULL,
	.cmp = NULL,
	.str = NULL,
	.call = methobj_invoke,

	.num_methods = &meth_num_methods,
	.seq_methods  = &meth_seq_methods,

	.members = NULL,
	.methods = NULL
};
