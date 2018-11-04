#include "js1.h"

/**
 * Fills token type and boundaries.
 */
static int js1_fill_token(enum js1type type, struct js1_parser *p,
			  int start, int end)
{
	struct js1token *token = p->toknext++;

	if (p->toknext > p->tokend)
		return JS1_ERROR_NOMEM;

	token->type = type;
	token->start = start;
	token->end = end;
	token->size = 0;
	token->parent = p->toksuper;
	if (p->toksuper)
		p->toksuper->size++;

	return 0;
}

/**
 * Fills next available token with JSON primitive.
 */
static int js1_parse_primitive(struct js1_parser *p, const char *js, size_t len)
{
	int next = p->pos;

	for (; next < len && js[next] != '\0'; next++) {
		switch (js[next]) {
		case '\t' : case '\r' : case '\n' : case ' ' :
		case ','  : case ']'  : case '}' :
			if (js1_fill_token(JS1_PRIMITIVE, p, p->pos, next))
				return JS1_ERROR_NOMEM;

			p->pos = next - 1;
			return 0;
		}
		if (js[next] < 32 || js[next] >= 127) {
			return JS1_ERROR_INVAL;
		}
	}
	return JS1_ERROR_PART;
}

/**
 * Fills next token with JSON string.
 */
static int js1_parse_string(struct js1_parser *p, const char *js, size_t len)
{
	/* Skip starting quote */
	for (int next = p->pos + 1; next < len && js[next] != '\0'; next++) {
		char c = js[next];

		/* Quote: end of string */
		if (c == '\"') {
			if (js1_fill_token(JS1_STRING, p, p->pos+1, next))
				return JS1_ERROR_NOMEM;
			p->pos = next;
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
 * Parse JSON string and fill tokens.
 */
int js1_parse(struct js1_parser *p, const char *js, size_t len)
{
	int r;
	if (!p->tokens)
		return JS1_ERROR_NOMEM;

	for (; p->pos < len && js[p->pos] != '\0'; p->pos++) {
		char c;
		enum js1type type;
		struct js1token *token;

		c = js[p->pos];
		switch (c) {
		case '{': case '[':
			if (js1_fill_token((c == '{' ? JS1_OBJECT : JS1_ARRAY), p, p->pos, -1))
				return JS1_ERROR_NOMEM;
			p->toksuper = p->toknext - 1;
			break;
		case '}': case ']':
			type = (c == '}' ? JS1_OBJECT : JS1_ARRAY);
			if (p->toknext < p->tokens + 1) {
				return JS1_ERROR_INVAL;
			}
			token = p->toknext - 1;
			for (;;) {
				if (token->start != -1 && token->end == -1) {
					if (token->type != type) {
						return JS1_ERROR_INVAL;
					}
					token->end = p->pos + 1;
					p->toksuper = token->parent;
					break;
				}
				if (token->parent == NULL) {
					if(token->type != type || !p->toksuper) {
						return JS1_ERROR_INVAL;
					}
					break;
				}
				token = token->parent;
			}
			break;
		case '\"':
			r = js1_parse_string(p, js, len);
			if (r < 0) return r;
			break;
		case '\t' : case '\r' : case '\n' : case ' ':
			break;
		case ':':
			p->toksuper = p->toknext - 1;
			break;
		case ',':
			if (p->toksuper &&
			    p->toksuper->type != JS1_ARRAY &&
			    p->toksuper->type != JS1_OBJECT) {
				p->toksuper = p->toksuper->parent;
			}
			break;
		case '-': case '0': case '1' : case '2': case '3' : case '4':
		case '5': case '6': case '7' : case '8': case '9':
		case 't': case 'f': case 'n' :
			/* And they must not be keys of the object */
			if (p->toksuper) {
				struct js1token *t = p->toksuper;
				if (t->type == JS1_OBJECT ||
						(t->type == JS1_STRING && t->size != 0)) {
					return JS1_ERROR_INVAL;
				}
			}
			r = js1_parse_primitive(p, js, len);
			if (r < 0) return r;
			break;
		default:
			return JS1_ERROR_INVAL;
		}
	}

	for (struct js1token *token = p->toknext - 1; token >= p->tokens; token--) {
		/* Unmatched opened object or array */
		if (token->start != -1 && token->end == -1) {
			return JS1_ERROR_PART;
		}
	}

	return p->toknext - p->tokens;
}

/**
 * Creates a new js1_parser based over a given  buffer with an array of tokens
 * available.
 */
void js1_init(struct js1_parser *p, struct js1token *tokens, size_t num_tokens)
{
	p->pos = 0;
	p->tokens = tokens;
	p->tokend = tokens + num_tokens;
	p->toknext = tokens;
	p->toksuper = NULL;
}

