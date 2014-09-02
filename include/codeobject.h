#ifndef CODEOBJECT_H
#define CODEOBJECT_H

#include "code.h"
#include "object.h"
#include "util.h"

extern Class co_class;

typedef struct {
	Object base;

	/* name of this code object */
	const char *name;

	/* code segment */
	byte *bc;

	/* number of arguments */
	unsigned char argcount;

	/* enumerated variable names */
	struct str_array names;

	/* enumerated attributes used */
	struct str_array attrs;

	/* enumerated constants */
	struct value_array consts;
} CodeObject;

CodeObject *codeobj_make(Code *code, const char *name, unsigned int argcount);

#endif /* CODEOBJECT_H */
