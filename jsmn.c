#include "jsmn.h"

/**
 * Fills token type and boundaries.
 */
static int jsmn_fill_token(jsmntype_t type,
			    jsmn_parser *parser, int start, int end) {
	jsmntok_t *token = parser->toknext++;
	if (parser->toknext > parser->tokend)
		return JSMN_ERROR_NOMEM;

	token->type = type;
	token->start = start;
	token->end = end;
	token->size = 0;
#ifdef JSMN_PARENT_LINKS
	token->parent = parser->toksuper;
#endif
	if (parser->toksuper)
		parser->toksuper->size++;

	return 0;
}

/**
 * Fills next available token with JSON primitive.
 */
static int jsmn_parse_primitive(jsmn_parser *parser, const char *js, size_t len)
{
	int next = parser->pos;

	for (; next < len && js[next] != '\0'; next++) {
		switch (js[next]) {
#ifndef JSMN_STRICT
			/* In strict mode primitive must be followed by "," or "}" or "]" */
			case ':':
#endif
			case '\t' : case '\r' : case '\n' : case ' ' :
			case ','  : case ']'  : case '}' :
				goto found;
		}
		if (js[next] < 32 || js[next] >= 127) {
			return JSMN_ERROR_INVAL;
		}
	}
#ifdef JSMN_STRICT
	/* In strict mode primitive must be followed by a comma/object/array */
	return JSMN_ERROR_PART;
#endif

found:
	if (jsmn_fill_token(JSMN_PRIMITIVE, parser, parser->pos, next))
		return JSMN_ERROR_NOMEM;
	parser->pos = next;
	parser->pos--;
	return 0;
}

/**
 * Fills next token with JSON string.
 */
static int jsmn_parse_string(jsmn_parser *parser, const char *js, size_t len)
{
	/* Skip starting quote */
	for (int next = parser->pos + 1; next < len && js[next] != '\0'; next++) {
		char c = js[next];

		/* Quote: end of string */
		if (c == '\"') {
			if (jsmn_fill_token(JSMN_STRING, parser, parser->pos+1, next))
				return JSMN_ERROR_NOMEM;
			parser->pos = next;
			return 0;
		}

		/* Backslash: Quoted symbol expected */
		if (c == '\\' && ++next < len) {
			int i;
			switch (js[next]) {
				/* Allowed escaped symbols */
				case '\"': case '/' : case '\\' : case 'b' :
				case 'f' : case 'r' : case 'n'  : case 't' :
					break;
				/* Allows escaped symbol \uXXXX */
				case 'u':
					for(i = 0; i < 4 && ++next < len && js[next] != '\0'; i++) {
						/* If it isn't a hex character we have an error */
						if(!((js[next] >= 48 && js[next] <= 57) || /* 0-9 */
									(js[next] >= 65 && js[next] <= 70) || /* A-F */
									(js[next] >= 97 && js[next] <= 102))) { /* a-f */
							return JSMN_ERROR_INVAL;
						}
					}
					break;
				/* Unexpected symbol */
				default:
					return JSMN_ERROR_INVAL;
			}
		}
	}
	return JSMN_ERROR_PART;
}

/**
 * Parse JSON string and fill parser->tokens.
 */
int jsmn_parse(jsmn_parser *parser, const char *js, size_t len,
		jsmntok_t *tokens, unsigned int num_tokens) {
	int r;
	if (!tokens)
		return JSMN_ERROR_NOMEM;
	parser->tokens = tokens;
	parser->tokend = tokens + num_tokens;
	parser->toknext = tokens;

	for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
		char c;
		jsmntype_t type;
		jsmntok_t *token;

		c = js[parser->pos];
		switch (c) {
			case '{': case '[':
				if (jsmn_fill_token((c == '{' ? JSMN_OBJECT : JSMN_ARRAY), parser, parser->pos, -1))
					return JSMN_ERROR_NOMEM;
				parser->toksuper = parser->toknext - 1;
				break;
			case '}': case ']':
				type = (c == '}' ? JSMN_OBJECT : JSMN_ARRAY);
#ifdef JSMN_PARENT_LINKS
				if (parser->toknext < parser->tokens + 1) {
					return JSMN_ERROR_INVAL;
				}
				token = parser->toknext - 1;
				for (;;) {
					if (token->start != -1 && token->end == -1) {
						if (token->type != type) {
							return JSMN_ERROR_INVAL;
						}
						token->end = parser->pos + 1;
						parser->toksuper = token->parent;
						break;
					}
					if (token->parent == NULL) {
						if(token->type != type || !parser->toksuper) {
							return JSMN_ERROR_INVAL;
						}
						break;
					}
					token = token->parent;
				}
#else
				for (token = parser->toknext - 1; token >= parser->tokens; token--) {
					if (token->start != -1 && token->end == -1) {
						if (token->type != type) {
							return JSMN_ERROR_INVAL;
						}
						parser->toksuper = NULL;
						token->end = parser->pos + 1;
						break;
					}
				}
				/* Error if unmatched closing bracket */
				if (token < parser->tokens)
					return JSMN_ERROR_INVAL;
				for (; token >= parser->tokens; token--) {
					if (token->start != -1 && token->end == -1) {
						parser->toksuper = token;
						break;
					}
				}
#endif
				break;
			case '\"':
				r = jsmn_parse_string(parser, js, len);
				if (r < 0) return r;
				break;
			case '\t' : case '\r' : case '\n' : case ' ':
				break;
			case ':':
				parser->toksuper = parser->toknext - 1;
				break;
			case ',':
				if (parser->toksuper &&
				    parser->toksuper->type != JSMN_ARRAY &&
				    parser->toksuper->type != JSMN_OBJECT) {
#ifdef JSMN_PARENT_LINKS
					parser->toksuper = parser->toksuper->parent;
#else
					for (token = parser->toknext - 1; token >= parser->tokens; token--) {
						if (token->type == JSMN_ARRAY || token->type == JSMN_OBJECT) {
							if (token->start != -1 && token->end == -1) {
								parser->toksuper = token;
								break;
							}
						}
					}
#endif
				}
				break;
#ifdef JSMN_STRICT
			/* In strict mode primitives are: numbers and booleans */
			case '-': case '0': case '1' : case '2': case '3' : case '4':
			case '5': case '6': case '7' : case '8': case '9':
			case 't': case 'f': case 'n' :
				/* And they must not be keys of the object */
				if (parser->toksuper) {
					jsmntok_t *t = parser->toksuper;
					if (t->type == JSMN_OBJECT ||
							(t->type == JSMN_STRING && t->size != 0)) {
						return JSMN_ERROR_INVAL;
					}
				}
#else
			/* In non-strict mode every unquoted value is a primitive */
			default:
#endif
				r = jsmn_parse_primitive(parser, js, len);
				if (r < 0) return r;
				break;

#ifdef JSMN_STRICT
			/* Unexpected char in strict mode */
			default:
				return JSMN_ERROR_INVAL;
#endif
		}
	}

	for (jsmntok_t *token = parser->toknext - 1; token >= parser->tokens; token--) {
		/* Unmatched opened object or array */
		if (token->start != -1 && token->end == -1) {
			return JSMN_ERROR_PART;
		}
	}

	return parser->toknext - parser->tokens;
}

/**
 * Creates a new parser based over a given  buffer with an array of tokens
 * available.
 */
void jsmn_init(jsmn_parser *parser) {
	parser->pos = 0;
	parser->toknext = NULL;
	parser->toksuper = NULL;
}

