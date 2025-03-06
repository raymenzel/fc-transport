makefile_path := $(realpath $(dir $(realpath $(lastword $(MAKEFILE_LIST)))))

CC ?= gcc
CFLAGS ?= -O0 -g -Wall -Werror -Wextra -pedantic

all: square-wave

square-wave: $(makefile_path)/src/flux_corrected_transport.c
	$(CC) $(CFLAGS) -o $@ $^

compare_logs.x: $(makefile_path)/test/compare_logs.c
	$(CC) $(CFLAGS) -o $@ $^ -lm

check: all compare_logs.x
	bash test/test.bash

clean:
	rm -f *.o
	rm -f square-wave
	rm -f compare_logs.x
