##
## Copyright 2023, Neutrality.
##
## SPDX-License-Identifier: BSD-2-Clause
##

BIN	:= machinedump
SRCS	:= $(wildcard src/*.[Sc])
HDRS	:= $(wildcard include/*.h)
OBJS	:= $(patsubst %.c,%.o,$(filter %.c,$(SRCS))) \
	   $(patsubst %.S,%.o,$(filter %.S,$(SRCS)))
LDS	:= linker.ld

CFLAGS	:= -O2 -Wall -Werror -ffreestanding -m32 -Iinclude
LDFLAGS	:= $(CFLAGS) -static -nostdlib -z noexecstack

all:	$(BIN)

$(BIN):	$(OBJS) $(LDS)
	$(CC) $(LDFLAGS) -T $(LDS) -o $@ $(OBJS)

%.o:	%.c $(HDRS)
	$(CC) $(CFLAGS) -c -o $@ $<

%.o:	%.S $(HDRS)
	$(CC) $(CFLAGS) -D__ASM__ -c -o $@ $<

clean:
	$(RM) $(BIN) $(OBJS)

.PHONY:	all clean
