#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "test.h"
#include "testutil.h"

int test_empty(void) {
	check(parse("{}", 1, 1,
				JS1_OBJECT, 0, 2, 0));
	check(parse("[]", 1, 1,
				JS1_ARRAY, 0, 2, 0));
	check(parse("[{},{}]", 3, 3,
				JS1_ARRAY, 0, 7, 2,
				JS1_OBJECT, 1, 3, 0,
				JS1_OBJECT, 4, 6, 0));
	return 0;
}

int test_object(void) {
	check(parse("{\"a\":0}", 3, 3,
				JS1_OBJECT, 0, 7, 1,
				JS1_STRING, "a", 1,
				JS1_PRIMITIVE, "0"));
	check(parse("{\"a\":[]}", 3, 3,
				JS1_OBJECT, 0, 8, 1,
				JS1_STRING, "a", 1,
				JS1_ARRAY, 5, 7, 0));
	check(parse("{\"a\":{},\"b\":{}}", 5, 5,
				JS1_OBJECT, -1, -1, 2,
				JS1_STRING, "a", 1,
				JS1_OBJECT, -1, -1, 0,
				JS1_STRING, "b", 1,
				JS1_OBJECT, -1, -1, 0));
	check(parse("{\n \"Day\": 26,\n \"Month\": 9,\n \"Year\": 12\n }", 7, 7,
				JS1_OBJECT, -1, -1, 3,
				JS1_STRING, "Day", 1,
				JS1_PRIMITIVE, "26",
				JS1_STRING, "Month", 1,
				JS1_PRIMITIVE, "9",
				JS1_STRING, "Year", 1,
				JS1_PRIMITIVE, "12"));
	check(parse("{\"a\": 0, \"b\": \"c\"}", 5, 5,
				JS1_OBJECT, -1, -1, 2,
				JS1_STRING, "a", 1,
				JS1_PRIMITIVE, "0",
				JS1_STRING, "b", 1,
				JS1_STRING, "c", 0));
	check(parse("{\"a\":{\"b\":{\"c\":{}}}}", 7, 7,
				JS1_OBJECT, 0, 20, 1,
				JS1_STRING, "a", 1,
				JS1_OBJECT, 5, 19, 1,
				JS1_STRING, "b", 1,
				JS1_OBJECT, 10, 18, 1,
				JS1_STRING, "c", 1,
				JS1_OBJECT, 15, 17, 0));

#ifdef JS1_STRICT
	check(parse("{\"a\"\n0}", JS1_ERROR_INVAL, 3));
	check(parse("{\"a\", 0}", JS1_ERROR_INVAL, 3));
	check(parse("{\"a\": {2}}", JS1_ERROR_INVAL, 3));
	check(parse("{\"a\": {2: 3}}", JS1_ERROR_INVAL, 3));
	check(parse("{\"a\": {\"a\": 2 3}}", JS1_ERROR_INVAL, 5));
	/* FIXME */
	/*check(parse("{\"a\"}", JS1_ERROR_INVAL, 2));*/
	/*check(parse("{\"a\": 1, \"b\"}", JS1_ERROR_INVAL, 4));*/
	/*check(parse("{\"a\",\"b\":1}", JS1_ERROR_INVAL, 4));*/
	/*check(parse("{\"a\":1,}", JS1_ERROR_INVAL, 4));*/
	/*check(parse("{\"a\":\"b\":\"c\"}", JS1_ERROR_INVAL, 4));*/
	/*check(parse("{,}", JS1_ERROR_INVAL, 4));*/
#endif
	return 0;
}

int test_array(void) {
	/* FIXME */
	/*check(parse("[10}", JS1_ERROR_INVAL, 3));*/
	/*check(parse("[1,,3]", JS1_ERROR_INVAL, 3)*/
	check(parse("[10]", 2, 2,
				JS1_ARRAY, -1, -1, 1,
				JS1_PRIMITIVE, "10"));
	check(parse("{\"a\": 1]", JS1_ERROR_INVAL, 3));
	/* FIXME */
	/*check(parse("[\"a\": 1]", JS1_ERROR_INVAL, 3));*/
	return 0;
}

int test_primitive(void) {
	check(parse("{\"boolVar\" : true }", 3, 3,
				JS1_OBJECT, -1, -1, 1,
				JS1_STRING, "boolVar", 1,
				JS1_PRIMITIVE, "true"));
	check(parse("{\"boolVar\" : false }", 3, 3,
				JS1_OBJECT, -1, -1, 1,
				JS1_STRING, "boolVar", 1,
				JS1_PRIMITIVE, "false"));
	check(parse("{\"nullVar\" : null }", 3, 3,
				JS1_OBJECT, -1, -1, 1,
				JS1_STRING, "nullVar", 1,
				JS1_PRIMITIVE, "null"));
	check(parse("{\"intVar\" : 12}", 3, 3,
				JS1_OBJECT, -1, -1, 1,
				JS1_STRING, "intVar", 1,
				JS1_PRIMITIVE, "12"));
	check(parse("{\"floatVar\" : 12.345}", 3, 3,
				JS1_OBJECT, -1, -1, 1,
				JS1_STRING, "floatVar", 1,
				JS1_PRIMITIVE, "12.345"));
	return 0;
}

int test_string(void) {
	check(parse("{\"strVar\" : \"hello world\"}", 3, 3,
				JS1_OBJECT, -1, -1, 1,
				JS1_STRING, "strVar", 1,
				JS1_STRING, "hello world", 0));
	check(parse("{\"strVar\" : \"escapes: \\/\\r\\n\\t\\b\\f\\\"\\\\\"}", 3, 3,
				JS1_OBJECT, -1, -1, 1,
				JS1_STRING, "strVar", 1,
				JS1_STRING, "escapes: \\/\\r\\n\\t\\b\\f\\\"\\\\", 0));
	check(parse("{\"strVar\": \"\"}", 3, 3,
				JS1_OBJECT, -1, -1, 1,
				JS1_STRING, "strVar", 1,
				JS1_STRING, "", 0));
	check(parse("{\"a\":\"\\uAbcD\"}", 3, 3,
				JS1_OBJECT, -1, -1, 1,
				JS1_STRING, "a", 1,
				JS1_STRING, "\\uAbcD", 0));
	check(parse("{\"a\":\"str\\u0000\"}", 3, 3,
				JS1_OBJECT, -1, -1, 1,
				JS1_STRING, "a", 1,
				JS1_STRING, "str\\u0000", 0));
	check(parse("{\"a\":\"\\uFFFFstr\"}", 3, 3,
				JS1_OBJECT, -1, -1, 1,
				JS1_STRING, "a", 1,
				JS1_STRING, "\\uFFFFstr", 0));
	check(parse("{\"a\":[\"\\u0280\"]}", 4, 4,
				JS1_OBJECT, -1, -1, 1,
				JS1_STRING, "a", 1,
				JS1_ARRAY, -1, -1, 1,
				JS1_STRING, "\\u0280", 0));

	check(parse("{\"a\":\"str\\uFFGFstr\"}", JS1_ERROR_INVAL, 3));
	check(parse("{\"a\":\"str\\u@FfF\"}", JS1_ERROR_INVAL, 3));
	check(parse("{{\"a\":[\"\\u028\"]}", JS1_ERROR_INVAL, 5));
	return 0;
}

int test_partial_string(void) {
	int i;
	int r;
	struct js1_parser p;
	struct js1token tok[5];
	const char *js = "{\"x\": \"va\\\\ue\", \"y\": \"value y\"}";

	for (i = 1; i <= strlen(js); i++) {
		js1_init(&p, tok, sizeof(tok)/sizeof(tok[0]));
		r = js1_parse(&p, js, i);
		if (i == strlen(js)) {
			check(r == 5);
			check(tokeq(js, tok, 5,
					JS1_OBJECT, -1, -1, 2,
					JS1_STRING, "x", 1,
					JS1_STRING, "va\\\\ue", 0,
					JS1_STRING, "y", 1,
					JS1_STRING, "value y", 0));
		} else {
			check(r == JS1_ERROR_PART);
		}
	}
	return 0;
}

int test_partial_array(void) {
#ifdef JS1_STRICT
	int r;
	int i;
	struct js1_parser p;
	struct js1token tok[6];
	const char *js = "[ 1, true, [123, \"hello\"]]";

	for (i = 1; i <= strlen(js); i++) {
		js1_init(&p, tok, sizeof(tok)/sizeof(tok[0]));
		r = js1_parse(&p, js, i);
		if (i == strlen(js)) {
			check(r == 6);
			check(tokeq(js, tok, 6,
					JS1_ARRAY, -1, -1, 3,
					JS1_PRIMITIVE, "1",
					JS1_PRIMITIVE, "true",
					JS1_ARRAY, -1, -1, 2,
					JS1_PRIMITIVE, "123",
					JS1_STRING, "hello", 0));
		} else {
			check(r == JS1_ERROR_PART);
		}
	}
#endif
	return 0;
}

int test_array_nomem(void) {
	int i;
	int r;
	struct js1_parser p;
	struct js1token toksmall[10];
	const char *js;

	js = "  [ 1, true, [123, \"hello\"]]";

	for (i = 0; i < 6; i++) {
		js1_init(&p, toksmall, i);
		memset(toksmall, 0, sizeof(toksmall));
		r = js1_parse(&p, js, strlen(js));
		check(r == JS1_ERROR_NOMEM);

		js1_init(&p, toksmall, 10);
		r = js1_parse(&p, js, strlen(js));
		check(r >= 0);
		check(tokeq(js, toksmall, 6,
					JS1_ARRAY, -1, -1, 3,
					JS1_PRIMITIVE, "1",
					JS1_PRIMITIVE, "true",
					JS1_ARRAY, -1, -1, 2,
					JS1_PRIMITIVE, "123",
					JS1_STRING, "hello", 0));
	}
	return 0;
}

int test_unquoted_keys(void) {
#ifndef JS1_STRICT
	int r;
	struct js1_parser p;
	struct js1token tok[10];
	const char *js;

	js1_init(&p, tok, 10);
	js = "key1: \"value\"\nkey2 : 123";

	r = js1_parse(&p, js, strlen(js));
	check(r >= 0);
	check(tokeq(js, tok, 4,
				JS1_PRIMITIVE, "key1",
				JS1_STRING, "value", 0,
				JS1_PRIMITIVE, "key2",
				JS1_PRIMITIVE, "123"));
#endif
	return 0;
}

int test_issue_22(void) {
	int r;
	struct js1_parser p;
	struct js1token tokens[128];
	const char *js;

	js = "{ \"height\":10, \"layers\":[ { \"data\":[6,6], \"height\":10, "
		"\"name\":\"Calque de Tile 1\", \"opacity\":1, \"type\":\"tilelayer\", "
		"\"visible\":true, \"width\":10, \"x\":0, \"y\":0 }], "
		"\"orientation\":\"orthogonal\", \"properties\": { }, \"tileheight\":32, "
		"\"tilesets\":[ { \"firstgid\":1, \"image\":\"..\\/images\\/tiles.png\", "
		"\"imageheight\":64, \"imagewidth\":160, \"margin\":0, \"name\":\"Tiles\", "
		"\"properties\":{}, \"spacing\":0, \"tileheight\":32, \"tilewidth\":32 }], "
		"\"tilewidth\":32, \"version\":1, \"width\":10 }";
	js1_init(&p, tokens, 128);
	r = js1_parse(&p, js, strlen(js));
	check(r >= 0);
	return 0;
}

int test_issue_27(void) {
	const char *js =
		"{ \"name\" : \"Jack\", \"age\" : 27 } { \"name\" : \"Anna\", ";
	check(parse(js, JS1_ERROR_PART, 8));
	return 0;
}

int test_input_length(void) {
	const char *js;
	int r;
	struct js1_parser p;
	struct js1token tokens[10];

	js = "{\"a\": 0}garbage";

	js1_init(&p, tokens, 10);
	r = js1_parse(&p, js, 8);
	check(r == 3);
	check(tokeq(js, tokens, 3,
				JS1_OBJECT, -1, -1, 1,
				JS1_STRING, "a", 1,
				JS1_PRIMITIVE, "0"));
	return 0;
}

int test_count(void) {
	struct js1_parser p;
	struct js1token tokens[7];
	const char *js;

	js = "{}";
	js1_init(&p, tokens, 1);
	check(js1_parse(&p, js, strlen(js)) == 1);

	js = "[]";
	js1_init(&p, tokens, 1);
	check(js1_parse(&p, js, strlen(js)) == 1);

	js = "[[]]";
	js1_init(&p, tokens, 2);
	check(js1_parse(&p, js, strlen(js)) == 2);

	js = "[[], []]";
	js1_init(&p, tokens, 3);
	check(js1_parse(&p, js, strlen(js)) == 3);

	js = "[[], []]";
	js1_init(&p, tokens, 3);
	check(js1_parse(&p, js, strlen(js)) == 3);

	js = "[[], [[]], [[], []]]";
	js1_init(&p, tokens, 7);
	check(js1_parse(&p, js, strlen(js)) == 7);

	js = "[\"a\", [[], []]]";
	js1_init(&p, tokens, 5);
	check(js1_parse(&p, js, strlen(js)) == 5);

	js = "[[], \"[], [[]]\", [[]]]";
	js1_init(&p, tokens, 5);
	check(js1_parse(&p, js, strlen(js)) == 5);

	js = "[1, 2, 3]";
	js1_init(&p, tokens, 4);
	check(js1_parse(&p, js, strlen(js)) == 4);

	js = "[1, 2, [3, \"a\"], null]";
	js1_init(&p, tokens, 7);
	check(js1_parse(&p, js, strlen(js)) == 7);

	return 0;
}


int test_nonstrict(void) {
#ifndef JS1_STRICT
	const char *js;
	js = "a: 0garbage";
	check(parse(js, 2, 2,
				JS1_PRIMITIVE, "a",
				JS1_PRIMITIVE, "0garbage"));

	js = "Day : 26\nMonth : Sep\n\nYear: 12";
	check(parse(js, 6, 6,
				JS1_PRIMITIVE, "Day",
				JS1_PRIMITIVE, "26",
				JS1_PRIMITIVE, "Month",
				JS1_PRIMITIVE, "Sep",
				JS1_PRIMITIVE, "Year",
				JS1_PRIMITIVE, "12"));

	//nested {s don't cause a parse error.
	js = "\"key {1\": 1234";
	check(parse(js, 2, 2,
		              JS1_STRING, "key {1", 1,
		              JS1_PRIMITIVE, "1234"));


#endif
	return 0;
}

int test_unmatched_brackets(void) {
	const char *js;
	js = "\"key 1\": 1234}";
	check(parse(js, JS1_ERROR_INVAL, 2));
	js = "{\"key 1\": 1234";
	check(parse(js, JS1_ERROR_PART, 3));
	js = "{\"key 1\": 1234}}";
	check(parse(js, JS1_ERROR_INVAL, 3));
	js = "\"key 1\"}: 1234";
	check(parse(js, JS1_ERROR_INVAL, 3));
	js = "{\"key {1\": 1234}";
	check(parse(js, 3, 3,
				JS1_OBJECT, 0, 16, 1,
				JS1_STRING, "key {1", 1,
				JS1_PRIMITIVE, "1234"));
	js = "{{\"key 1\": 1234}";
	check(parse(js, JS1_ERROR_PART, 4));
	return 0;
}

int main(void) {
	test(test_empty, "test for a empty JSON objects/arrays");
	test(test_object, "test for a JSON objects");
	test(test_array, "test for a JSON arrays");
	test(test_primitive, "test primitive JSON data types");
	test(test_string, "test string JSON data types");

	test(test_partial_string, "test partial JSON string parsing");
	test(test_partial_array, "test partial array reading");
	test(test_array_nomem, "test array reading with a smaller number of tokens");
	test(test_unquoted_keys, "test unquoted keys (like in JavaScript)");
	test(test_input_length, "test strings that are not null-terminated");
	test(test_issue_22, "test issue #22");
	test(test_issue_27, "test issue #27");
	test(test_count, "test tokens count estimation");
	test(test_nonstrict, "test for non-strict mode");
	test(test_unmatched_brackets, "test for unmatched brackets");
	printf("\nPASSED: %d\nFAILED: %d\n", test_passed, test_failed);
	return (test_failed > 0);
}
