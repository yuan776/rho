#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "attr.h"
#include "exc.h"
#include "str.h"
#include "object.h"
#include "util.h"
#include "strobject.h"

Value strobj_make(Str value)
{
	StrObject *s = obj_alloc(&str_class);
	s->freeable = value.freeable;
	value.freeable = 0;
	s->str = value;
	return makeobj(s);
}

Value strobj_make_direct(const char *value, const size_t len)
{
	StrObject *s = obj_alloc(&str_class);
	char *copy = rho_malloc(len + 1);
	memcpy(copy, value, len);
	copy[len] = '\0';
	s->str = STR_INIT(copy, len, 0);
	s->freeable = 1;
	return makeobj(s);
}

static bool strobj_eq(Value *this, Value *other)
{
	if (!is_a(other, &str_class)) {
		return false;
	}

	StrObject *s1 = objvalue(this);
	StrObject *s2 = objvalue(other);
	return str_eq(&s1->str, &s2->str);
}

static Value strobj_cmp(Value *this, Value *other)
{
	if (!is_a(other, &str_class)) {
		return makeut();
	}

	StrObject *s1 = objvalue(this);
	StrObject *s2 = objvalue(other);
	return makeint(str_cmp(&s1->str, &s2->str));
}

static int strobj_hash(Value *this)
{
	StrObject *s = objvalue(this);
	return str_hash(&s->str);
}

static bool strobj_nonzero(Value *this)
{
	StrObject *s = objvalue(this);
	return (s->str.len != 0);
}

static void strobj_free(Value *this)
{
	StrObject *s = objvalue(this);
	if (s->freeable) {
		FREE(s->str.value);
	}

	s->base.class->super->del(this);
}

static StrObject *strobj_str(Value *this)
{
	StrObject *s = objvalue(this);
	retaino(s);
	return s;
}

static Value strobj_cat(Value *this, Value *other)
{
	if (!is_a(other, &str_class)) {
		return makeut();
	}

	Str *s1 = &((StrObject *) objvalue(this))->str;
	Str *s2 = &((StrObject *) objvalue(other))->str;

	const size_t len1 = s1->len;
	const size_t len2 = s2->len;
	const size_t len_cat = len1 + len2;

	char *cat = rho_malloc(len_cat + 1);

	for (size_t i = 0; i < len1; i++) {
		cat[i] = s1->value[i];
	}

	for (size_t i = 0; i < len2; i++) {
		cat[i + len1] = s2->value[i];
	}

	cat[len_cat] = '\0';

	return strobj_make(STR_INIT(cat, len_cat, 1));
}

static size_t strobj_len(Value *this)
{
	StrObject *s = objvalue(this);
	return s->str.len;
}

struct num_methods str_num_methods = {
	NULL,    /* plus */
	NULL,    /* minus */
	NULL,    /* abs */

	strobj_cat,    /* add */
	NULL,    /* sub */
	NULL,    /* mul */
	NULL,    /* div */
	NULL,    /* mod */
	NULL,    /* pow */

	NULL,    /* bitnot */
	NULL,    /* bitand */
	NULL,    /* bitor */
	NULL,    /* xor */
	NULL,    /* shiftl */
	NULL,    /* shiftr */

	NULL,    /* iadd */
	NULL,    /* isub */
	NULL,    /* imul */
	NULL,    /* idiv */
	NULL,    /* imod */
	NULL,    /* ipow */

	NULL,    /* ibitand */
	NULL,    /* ibitor */
	NULL,    /* ixor */
	NULL,    /* ishiftl */
	NULL,    /* ishiftr */

	NULL,    /* radd */
	NULL,    /* rsub */
	NULL,    /* rmul */
	NULL,    /* rdiv */
	NULL,    /* rmod */
	NULL,    /* rpow */

	NULL,    /* rbitand */
	NULL,    /* rbitor */
	NULL,    /* rxor */
	NULL,    /* rshiftl */
	NULL,    /* rshiftr */

	strobj_nonzero,    /* nonzero */

	NULL,    /* to_int */
	NULL,    /* to_float */
};

struct seq_methods str_seq_methods = {
	strobj_len,    /* len */
	NULL,    /* get */
	NULL,    /* set */
	NULL,    /* contains */
	NULL,    /* apply */
	NULL,    /* iapply */
};

Class str_class = {
	.base = CLASS_BASE_INIT(),
	.name = "Str",
	.super = &obj_class,

	.instance_size = sizeof(StrObject),

	.init = NULL,
	.del = strobj_free,

	.eq = strobj_eq,
	.hash = strobj_hash,
	.cmp = strobj_cmp,
	.str = strobj_str,
	.call = NULL,

	.num_methods = &str_num_methods,
	.seq_methods  = &str_seq_methods,

	.print = NULL,

	.iter = NULL,
	.iternext = NULL,

	.members = NULL,
	.methods = NULL,

	.attr_get = NULL,
	.attr_set = NULL
};
