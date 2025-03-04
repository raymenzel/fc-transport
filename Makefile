makefile_path := $(realpath $(dir $(realpath $(lastword $(MAKEFILE_LIST)))))

CC ?= gcc
CFLAGS ?= -O0 -g -Wall -Werror -Wextra -pedantic

all: square-wave

square-wave: $(makefile_path)/src/flux_corrected_transport.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o
	rm -f square-wave
