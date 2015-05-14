
TARGETS+= sin_quick_submit
#TARGETS+= sin_batch_submit

include ../Makefile.defs

ifndef NO_KIWI
CFLAGS += -DUSE_KIWI
endif
CFLAGS += -DUSE_KIWI_SERVER
#CFLAGS += -DUSE_KIWI_SERVER_MHD
LDFLAGS += -L$(KIWI_BASE_DIR) -lkiwi
CFLAGS += -I$(KIWI_BASE_DIR)

#
# you should define TARGETS in your Makefile so that useful target
# in this file works properly.
#     e.g. TARGETS= hoge2 hoge3
#

.PHONY: clean

OS= $(shell uname -s)

CFLAGS += -Werror -Wall
CFLAGS += -I.

ifeq ($(OS),Linux)
#LDFLAGS+= -lpthread
endif

all: $(TARGETS)

$(TARGETS):

#
#.SUFFIXES: .o .c
#
#.c.o:
#	$(CC) $(CFLAGS) -c $<

clean:
	-rm -rf *.dSYM *.o
ifneq ($(TARGETS),)
	-rm -f $(TARGETS)
endif

distclean: clean
	-rm -f Makefile config.cache config.status config.log .depend

make_var_test:
	@echo CFLAGS =$(CFLAGS)
	@echo LDFLAGS=$(LDFLAGS)

