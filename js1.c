#include "js1.h"

/**
 * Fills token type and boundaries.
 */
static int js1_fill_token(enum js1type type, struct js1_parser *parser,
			  int start, int end)
{
	struct js1token *token = parser->toknext++;

	if (parser->toknext > parser->tokend)
		return JS1_ERROR_NOMEM;

	token->type = type;
	token->start = start;
	token->end = end;
	token->size = 0;
	token->parent = parser->toksuper;
	if (parser->toksuper)
		parser->toksuper->size++;

	return 0;
}

/**
 * Fills next available token with JSON primitive.
 */
static int js1_parse_primitive(struct js1_parser *parser, const char *js, size_t len)
{
	int next = parser->pos;

	for (; next < len && js[next] != '\0'; next++) {
		switch (js[next]) {
		case '\t' : case '\r' : case '\n' : case ' ' :
		case ','  : case ']'  : case '}' :
			goto found;
		}
		if (js[next] < 32 || js[next] >= 127) {
			return JS1_ERROR_INVAL;
		}
	}
	return JS1_ERROR_PART;

found:
	if (js1_fill_token(JS1_PRIMITIVE, parser, parser->pos, next))
		return JS1_ERROR_NOMEM;
	parser->pos = next;
	parser->pos--;
	return 0;
}

/**
 * Fills next token with JSON string.
 */
static int js1_parse_string(struct js1_parser *parser, const char *js, size_t len)
{
	/* Skip starting quote */
	for (int next = parser->pos + 1; next < len && js[next] != '\0'; next++) {
		char c = js[next];

		/* Quote: end of string */
		if (c == '\"') {
			if (js1_fill_token(JS1_STRING, parser, parser->pos+1, next))
				return JS1_ERROR_NOMEM;
			parser->pos = next;
			return 0;
		}

		/* Backslash: Quoted symbol expected */
		if (c == '\\' && ++next < len) {
			switch (js[next]) {
			/* Allowed escaped symbols */
			case '\"': case '/' : case '\\' : case 'b' :
			case 'f' : case 'r' : case 'n'  : case 't' :
				break;
			/* Allows escaped symbol \uXXXX */
			case 'u':
				if (next + 4 > len)
					break;
				for(int i = 0; i < 4 && js[++next] != '\0'; i++) {
					if(!is_hex(js[next]))
						return JS1_ERROR_INVAL;
				}
				break;
			/* Unexpected symbol */
			default:
				return JS1_ERROR_INVAL;
			}
		}
	}
	return JS1_ERROR_PART;
}

/**
 * Parse JSON string and fill parser->tokens.
 */
int js1_parse(struct js1_parser *parser, const char *js, size_t len)
{
	int r;
	if (!parser->tokens)
		return JS1_ERROR_NOMEM;

	for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
		char c;
		enum js1type type;
		struct js1token *token;

		c = js[parser->pos];
		switch (c) {
		case '{': case '[':
			if (js1_fill_token((c == '{' ? JS1_OBJECT : JS1_ARRAY), parser, parser->pos, -1))
				return JS1_ERROR_NOMEM;
			parser->toksuper = parser->toknext - 1;
			break;
		case '}': case ']':
			type = (c == '}' ? JS1_OBJECT : JS1_ARRAY);
			if (parser->toknext < parser->tokens + 1) {
				return JS1_ERROR_INVAL;
			}
			token = parser->toknext - 1;
			for (;;) {
				if (token->start != -1 && token->end == -1) {
					if (token->type != type) {
						return JS1_ERROR_INVAL;
					}
					token->end = parser->pos + 1;
					parser->toksuper = token->parent;
					break;
				}
				if (token->parent == NULL) {
					if(token->type != type || !parser->toksuper) {
						return JS1_ERROR_INVAL;
					}
					break;
				}
				token = token->parent;
			}
			break;
		case '\"':
			r = js1_parse_string(parser, js, len);
			if (r < 0) return r;
			break;
		case '\t' : case '\r' : case '\n' : case ' ':
			break;
		case ':':
			parser->toksuper = parser->toknext - 1;
			break;
		case ',':
			if (parser->toksuper &&
			    parser->toksuper->type != JS1_ARRAY &&
			    parser->toksuper->type != JS1_OBJECT) {
				parser->toksuper = parser->toksuper->parent;
			}
			break;
		case '-': case '0': case '1' : case '2': case '3' : case '4':
		case '5': case '6': case '7' : case '8': case '9':
		case 't': case 'f': case 'n' :
			/* And they must not be keys of the object */
			if (parser->toksuper) {
				struct js1token *t = parser->toksuper;
				if (t->type == JS1_OBJECT ||
						(t->type == JS1_STRING && t->size != 0)) {
					return JS1_ERROR_INVAL;
				}
			}
			r = js1_parse_primitive(parser, js, len);
			if (r < 0) return r;
			break;
		default:
			return JS1_ERROR_INVAL;
		}
	}

	for (struct js1token *token = parser->toknext - 1; token >= parser->tokens; token--) {
		/* Unmatched opened object or array */
		if (token->start != -1 && token->end == -1) {
			return JS1_ERROR_PART;
		}
	}

	return parser->toknext - parser->tokens;
}

/**
 * Creates a new parser based over a given  buffer with an array of tokens
 * available.
 */
void js1_init(struct js1_parser *parser, struct js1token *tokens, size_t num_tokens)
{
	parser->pos = 0;
	parser->tokens = tokens;
	parser->tokend = tokens + num_tokens;
	parser->toknext = tokens;
	parser->toksuper = NULL;
}

