#ifndef __JS1_H_
#define __JS1_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * JSON type identifier. Basic types are:
 * 	o Object
 * 	o Array
 * 	o String
 * 	o Other primitive: number, boolean (true/false) or null
 */
enum js1type {
	JS1_UNDEFINED = 0,
	JS1_OBJECT = 1,
	JS1_ARRAY = 2,
	JS1_STRING = 3,
	JS1_PRIMITIVE = 4
};

enum js1err {
	/* Not enough tokens were provided */
	JS1_ERROR_NOMEM = -1,
	/* Invalid character inside JSON string */
	JS1_ERROR_INVAL = -2,
	/* The string is not a full JSON packet, more bytes expected */
	JS1_ERROR_PART = -3
};

/**
 * JSON token description.
 * type		type (object, array, string etc.)
 * start	start position in JSON data string
 * end		end position in JSON data string
 */
struct js1token {
	enum js1type type;
	const char *start;
	const char *end;
	int size;
	struct js1token *parent;
};

/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string
 */
struct js1_parser {
	const char *js;
	const char *js_end;
	struct js1token *tokens;
	struct js1token *tokend;
	struct js1token *toknext; /* next token to allocate */
	struct js1token *toksuper; /* superior token node, e.g parent object or array */
};

#define is_hex(c) ((c) <= 'f' && (c) >= '0' && \
		   ((c) <= '9' || (c) >= 'a' || ((c) >= 'A' && (c) <= 'F')))
/**
 * Create JSON parser over an array of tokens
 */
void js1_init(struct js1_parser *parser, struct js1token *tokens,
	      size_t num_tokens);

/**
 * Run JSON parser. It parses a JSON data string into and array of tokens, each describing
 * a single JSON object.
 */
int js1_parse(struct js1_parser *parser, const char *js, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* __JS1_H_ */
