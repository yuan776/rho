#ifndef LEXER_H
#define LEXER_H

#include <stdlib.h>
#include <stdbool.h>

typedef enum {
	TOK_NONE,           // indicates absent or unknown type

	/* literals */
	TOK_INT,            // int literal
	TOK_FLOAT,          // float literal
	TOK_STR,            // quoted string: ".*" (escapes are allowed too)
	TOK_IDENT,          // [a-zA-Z_][a-zA-Z_0-9]*

	/* operators */
	TOK_OPS_START,  /* marker */
	TOK_ASSIGN,
	TOK_PLUS,
	TOK_MINUS,
	TOK_MUL,
	TOK_DIV,
	TOK_MOD,
	TOK_POW,
	TOK_BITAND,
	TOK_BITOR,
	TOK_XOR,
	TOK_BITNOT,
	TOK_SHIFTL,
	TOK_SHIFTR,
	TOK_AND,
	TOK_OR,
	TOK_NOT,
	TOK_EQUAL,
	TOK_NOTEQ,
	TOK_LT,
	TOK_GT,
	TOK_LE,
	TOK_GE,
	TOK_OPS_END,    /* marker */

	TOK_PAREN_OPEN,     // literal: (
	TOK_PAREN_CLOSE,    // literal: )
	TOK_BRACKET_OPEN,   // literal: {
	TOK_BRACKET_CLOSE,  // literal: }

	/* keywords */
	TOK_PRINT,
	TOK_IF,
	TOK_WHILE,
	TOK_DEF,

	/* statement terminators */
	TOK_SEMICOLON,
	TOK_NEWLINE,
	TOK_EOF             // should always be the last token
} TokType;

static inline bool is_op(TokType type)
{
	return ((TOK_OPS_START < type) && (type < TOK_OPS_END));
}

typedef struct {
	const char *value;    // not null-terminated
	size_t length;
	TokType type;
	unsigned int lineno;  // 1-based line number
} Token;

void print_token(Token *tok);

typedef struct {
	/* source code to parse */
	const char *code;

	/* end of source code (last char) */
	const char *end;

	/* where we are in the string */
	char *pos;

	/* increases to consume token */
	unsigned int mark;

	/* tokens that have been read */
	Token *tokens;
	size_t tok_count;
	size_t tok_capacity;

	/* the "peek-token" is somewhat complicated to
	   compute, so we cache it */
	Token *peek;

	/* where we are in the tokens array */
	size_t tok_pos;

	/* the line number/position we are currently on */
	unsigned int lineno;

	/* name of the file out of which the source was read */
	const char *name;
} Lexer;

Lexer *lex_new(char *str, const size_t length, const char *name);
Token *lex_next_token(Lexer *lex);
Token *lex_next_token_direct(Lexer *lex);
Token *lex_peek_token(Lexer *lex);
Token *lex_peek_token_direct(Lexer *lex);
bool lex_has_next(Lexer *lex);
void lex_free(Lexer *lex);

const char *type_to_str(TokType type);

#endif /* LEXER_H */
