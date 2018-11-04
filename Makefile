# You can put your build options here
-include config.mk

all: libjs1.a

libjs1.a: js1.o
	$(AR) rc $@ $^

%.o: %.c js1.h
	$(CC) -c $(CFLAGS) $< -o $@

test: CFLAGS += -Werror -Wall
test: test_default
test_default: test/tests.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o test/$@
	./test/$@

js1_test.o: js1_test.c libjs1.a

simple_example: example/simple.o libjs1.a
	$(CC) $(LDFLAGS) $^ -o $@

jsondump: example/jsondump.o libjs1.a
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	rm -f *.o example/*.o
	rm -f *.a *.so
	rm -f simple_example
	rm -f jsondump

.PHONY: all clean test

