JS1
====

js1 (pronounced like 'jay swan') is a minimalistic JSON parser in C. It started as
a fork of jsmn (https://github.com/zserge/jsmn) with an eye toward being a complete
and easy to use parser rather than just a tokenizer.

You can find more information about JSON format at [json.org][1]

Library sources are available at https://github.com/d4ddi0/js1

and extract any value by its name. js1 proves that checking the correctness of
every JSON packet or allocating temporary objects to store parsed JSON fields
often is an overkill. 

JSON format itself is extremely simple, so why should we complicate it?

Like jsmn, js1 is designed to be **robust** (it should work fine even with erroneous
data), **fast** (it should parse data on the fly), **portable** (no superfluous
dependencies or non-standard C extensions). And of course, **simplicity** is a
key feature - simple code style, simple algorithm, simple integration into
other projects.

Features
--------

* compatible with C89
* no dependencies (even libc!)
* highly portable (tested on x86/amd64, ARM, AVR)
* about 200 lines of code
* extremely small code footprint
* API contains only 2 functions
* no dynamic memory allocation
* incremental single-pass parsing
* library code is covered with unit-tests

Design
------

The rudimentary js1 object is a **token**. Let's consider a JSON string:

	'{ "name" : "Jack", "age" : 27 }'

It holds the following tokens:

* Object: `{ "name" : "Jack", "age" : 27}` (the whole object)
* Strings: `"name"`, `"Jack"`, `"age"` (keys and some values)
* Number: `27`

In js1, tokens do not hold any data, but point to token boundaries in JSON
string instead. In the example above js1 will create tokens like: Object
[0..31], String [3..7], String [12..16], String [20..23], Number [27..29].

Every js1 token has a type, which indicates the type of corresponding JSON
token. js1 supports the following token types:

* Object - a container of key-value pairs, e.g.:
	`{ "foo":"bar", "x":0.3 }`
* Array - a sequence of values, e.g.:
	`[ 1, 2, 3 ]`
* String - a quoted sequence of chars, e.g.: `"foo"`
* Primitive - a number, a boolean (`true`, `false`) or `null`

Besides start/end positions, js1 tokens for complex types (like arrays
or objects) also contain a number of child items, so you can easily follow
object hierarchy.

This approach provides enough information for parsing any JSON data and makes
it possible to use zero-copy techniques.

Install
-------

To clone the repository you should have Git installed. Just run:

	$ git clone https://github.com/d4ddi0/js1

Repository layout is simple: js1.c and js1.h are library files, tests are in
the js1\_test.c, you will also find README, LICENSE and Makefile files inside.

To build the library, run `make`. It is also recommended to run `make test`.
Let me know, if some tests fail.

If build was successful, you should get a `libjs1.a` library.
The header file you should include is called `"js1.h"`.

API
---

Token types are described by `js1type_t`:

	typedef enum {
		JS1_UNDEFINED = 0,
		JS1_OBJECT = 1,
		JS1_ARRAY = 2,
		JS1_STRING = 3,
		JS1_PRIMITIVE = 4
	} js1type_t;

**Note:** Unlike JSON data types, primitive tokens are not divided into
numbers, booleans and null, because one can easily tell the type using the
first character:

* <code>'t', 'f'</code> - boolean 
* <code>'n'</code> - null
* <code>'-', '0'..'9'</code> - number

Token is an object of `js1tok_t` type:

	typedef struct {
		js1type_t type; // Token type
		int start;       // Token start position
		int end;         // Token end position
		int size;        // Number of child (nested) tokens
	} js1tok_t;

**Note:** string tokens point to the first character after
the opening quote and the previous symbol before final quote. This was made 
to simplify string extraction from JSON data.

All job is done by `js1_parser` object. You can initialize a new parser using:

	js1_parser parser;
	js1tok_t tokens[10];

	js1_init(&parser);

	// js - pointer to JSON string
	// tokens - an array of tokens available
	// 10 - number of tokens available
	js1_parse(&parser, js, strlen(js), tokens, 10);

This will create a parser, and then it tries to parse up to 10 JSON tokens from
the `js` string.

A non-negative return value of `js1_parse` is the number of tokens actually
used by the parser.
Passing NULL instead of the tokens array would not store parsing results, but
instead the function will return the value of tokens needed to parse the given
string. This can be useful if you don't know yet how many tokens to allocate.

If something goes wrong, you will get an error. Error will be one of these:

* `JS1_ERROR_INVAL` - bad token, JSON string is corrupted
* `JS1_ERROR_NOMEM` - not enough tokens, JSON string is too large
* `JS1_ERROR_PART` - JSON string is too short, expecting more JSON data

If you get `JS1_ERROR_NOMEM`, you can re-allocate more tokens and call
`js1_parse` once more.  If you read json data from the stream, you can
periodically call `js1_parse` and check if return value is `JS1_ERROR_PART`.
You will get this error until you reach the end of JSON data.

Other info
----------

This software is distributed under [MIT license](http://www.opensource.org/licenses/mit-license.php),
 so feel free to integrate it in your commercial products.

[1]: http://www.json.org/
